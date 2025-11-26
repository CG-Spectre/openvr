# Indirect-Ray + ML Optimization Plan

## Current pipeline notes
- `render/camera.cpp` orchestrates GPU rendering: it serializes scene geometry and BVH data, sets OpenCL kernel args (textures, lights, motion), and launches `renderPixel` to fill the PBO-backed texture for display. Indirect lighting buffer hooks already exist via `irlighting` and `renderILResultBuffer`.【F:render/camera.cpp†L61-L210】【F:render/camera.cpp†L211-L332】
- The camera records per-ray metadata with `indirectLightingResult` (radiance, hit position, sample direction, albedo/normal/reflect vectors) intended for offline logging/training.【F:render/camera.h†L20-L46】
- Training code in `training/train.py` uses those samples (saved as JSON) to learn a `SampleDirectionNet` that predicts a normalized 3D direction from nine inputs: albedo, intersection position (x/y/z), surface normal (x/y/z), incoming radiance, and shininess. The output is a unit vector for the next sample direction.【F:training/train.py†L1-L82】
- Pre-exported weights exist in `render/GPU/policy_weights.h` and related headers; OpenCL buffers for those weights are already prepared during `camera::init`, suggesting the runtime can run an on-GPU MLP for sampling guidance.【F:render/camera.cpp†L33-L117】

## Inputs to feed the ML sampler
Use the same nine-feature vector that training already expects to keep compatibility with the existing weight headers:
1. Material albedo (grayscale or luminance scalar).
2. Hit position: world-space intersection `x`, `y`, `z`.
3. Surface normal: `nx`, `ny`, `nz` (ensure normalized, outward-facing).
4. Incoming radiance estimate for the first-bounce hit.
5. Specular/roughness proxy: current "shininess" value used by BRDF in the kernel.

If roughness is not already available, derive it from texture flags (`textureRotation` etc.) or augment `SerializedObject`/material metadata to carry a scalar roughness so the nine inputs stay consistent.

## Step-by-step implementation plan
1. **Instrument the renderer to log training samples**
   - In the OpenCL `renderPixel` kernel (see the kernel source under `render/GPU`), capture per-pixel first-bounce data (radiance, intersection position, normal, albedo/roughness, chosen sample direction) and write it into `renderILResultBuffer` for a small set of pixels per frame to avoid stalls.
   - On the CPU, extend `camera::render` to append those results into `allILResults` and periodically flush to JSON (e.g., in `camera::stop`) to regenerate `samples.json` for training.【F:render/camera.cpp†L211-L332】

2. **Train or fine-tune the direction sampler**
   - Use `training/train.py` with the new dataset; keep the 9→64→64→3 MLP architecture so existing inference code can reuse the exported flat weight arrays. Confirm normalization of targets to unit vectors during training.【F:training/train.py†L1-L82】
   - Export updated weights to header form (matching `policy_weights.h` layout) and drop them in `render/GPU/`. Keep the same array names (W0/W1/W2/b0/b1/b2) so `camera::init` continues to populate buffers without code changes.【F:render/camera.cpp†L33-L117】

3. **Add ML-guided indirect ray sampling at runtime**
   - In the OpenCL kernel, branch after the primary hit: form the 9D feature vector from per-hit data, run the small MLP using the uploaded weights (two ReLU hidden layers, output normalized), and treat the result as the next-bounce direction. Fall back to cosine-weighted hemisphere sampling if the network outputs NaNs or the hit lacks valid inputs.
   - Reuse `raysInBuffer`/`renderILResultBuffer` to pass debugging rays back to the CPU when `useReflectDir` is enabled for inspection tools (already wired via `pipe_server`).【F:render/camera.cpp†L117-L188】【F:render/camera.cpp†L211-L332】

4. **Optimize sampling and accumulation**
   - Importance-sample indirect rays using the MLP direction as the primary candidate; optionally generate a secondary stratified sample to maintain variance estimates and avoid bias.
   - Accumulate indirect radiance into `irlighting` and blend with direct lighting in the kernel to keep the CPU-side code unchanged.
   - Use per-pixel reservoirs or running averages in the kernel to stabilize indirect lighting across frames (temporal reuse) without growing CPU buffers.

5. **Validation and debugging hooks**
   - Add a debug mode (toggle via key or CLI flag) that writes out a small CSV/JSON of MLP inputs/outputs and resulting radiance for a fixed pixel set each frame to validate that the network produces sensible directions.
   - Render an on-screen heatmap (using existing PBO) of indirect light contribution or sampling probability to diagnose bias/noise before shipping changes.

## Deliverables checklist
- Updated OpenCL kernel implementing MLP-guided indirect ray sampling and accumulation.
- Logging path that produces refreshed training data (`samples.json`) and a script to convert it to header weights.
- Documentation in `README` or developer notes summarizing how to retrain the sampler and swap weights.
- Basic verification: side-by-side renders with and without the ML sampler, plus noise convergence plots if possible.
