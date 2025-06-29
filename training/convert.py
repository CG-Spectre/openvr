import torch
import numpy as np
from stable_baselines3 import PPO

# Load the trained model
model = PPO.load("direction_policy.zip")

# Get the full policy network
policy_net = model.policy.mlp_extractor.policy_net
action_net = model.policy.action_net  # Output layer

# Gather all linear layers in order: FC1, FC2, Output
layers = [layer for layer in policy_net if isinstance(layer, torch.nn.Linear)]
layers.append(action_net)

# Open header file
with open("policy_weights5.h", "w") as f:
    f.write("// Auto-generated neural network weights for OpenCL\n")
    f.write("// Model architecture: 12 -> 64 -> 64 -> 3\n\n")

    for i, layer in enumerate(layers):
        W = layer.weight.detach().cpu().numpy()
        b = layer.bias.detach().cpu().numpy()

        flat_W = W.T.flatten()
        flat_b = b.flatten()

        # Write weight matrix
        w_name = f"W{i}"
        f.write(f"// Shape: {W.shape}\n")
        f.write(f"const float {w_name}[{flat_W.size}] = {{\n")
        for j, val in enumerate(flat_W):
            f.write(f"  {val:.8f},")
            if (j + 1) % 8 == 0:
                f.write("\n")
        f.write("\n};\n\n")

        # Write bias vector
        b_name = f"b{i}"
        f.write(f"// Shape: {b.shape}\n")
        f.write(f"const float {b_name}[{flat_b.size}] = {{\n")
        for j, val in enumerate(flat_b):
            f.write(f"  {val:.8f},")
            if (j + 1) % 8 == 0:
                f.write("\n")
        f.write("\n};\n\n")