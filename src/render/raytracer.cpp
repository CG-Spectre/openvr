#include "raytracer.h"

void RayTracingRenderer::render(Scene& scene, camera& cam, Vector3d translationalVelocity,
                                Vector3d rotationalVelocity) {
    cam.setScene(&scene);
    cam.render(nullptr, translationalVelocity, rotationalVelocity);
}
