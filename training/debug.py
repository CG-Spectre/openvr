import numpy as np
import torch
from stable_baselines3 import PPO

# Load model
model = PPO.load("direction_policy.zip")

# Prepare input
input_np = np.array([1.0, 0.004376, 0.001733, 3.5, 0, 0, 1, 0, 1, -0.00125, -0.000495, -0.99999], dtype=np.float32)
x = input_np.copy()

# Extract layers
policy_net = model.policy.mlp_extractor.policy_net
action_net = model.policy.action_net
layers = [layer for layer in policy_net if isinstance(layer, torch.nn.Linear)]
layers.append(action_net)

# Convert all weights and biases to NumPy arrays (transposed)
W0 = layers[0].weight.detach().cpu().numpy().T  # shape [9, 64]
b0 = layers[0].bias.detach().cpu().numpy()      # shape [64]
W1 = layers[1].weight.detach().cpu().numpy().T  # shape [64, 64]
b1 = layers[1].bias.detach().cpu().numpy()
W2 = layers[2].weight.detach().cpu().numpy().T  # shape [64, 3]
b2 = layers[2].bias.detach().cpu().numpy()

# Forward pass
def relu(x):
    return np.maximum(0, x)

# FC1
h1 = relu(np.dot(x, W0) + b0)
print("h1:", h1.round(6))

# FC2
h2 = relu(np.dot(h1, W1) + b1)
print("h2:", h2.round(6))

# Output
out = np.dot(h2, W2) + b2
print("Output before tanh:", out.round(6))
out_tanh = np.tanh(out)
print("Output after tanh:", out_tanh.round(6))

# Normalized output
normed = out_tanh / (np.linalg.norm(out_tanh) + 1e-6)
print("Normalized final output:", normed.round(6))