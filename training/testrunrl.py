import torch
import numpy as np
from stable_baselines3 import PPO

model = PPO.load("direction_policy.zip")
observation = torch.tensor([[
    1.0,  # albedo
    0.004376,  # interX
    0.001733,
    3.5,
    0,
    0,
    1,
    0.0,  # radiance (ignored)
    1.0  # shininess
]], dtype=torch.float32, device=model.device)
data, _ = model.predict(observation.cpu(), deterministic=True)
#norm = np.linalg.norm(data)
#print(data/norm)

# Forward pass
with torch.no_grad():
    latent, _ = model.policy.mlp_extractor(observation)
    logits = model.policy.action_net(latent)
    squashed = torch.tanh(logits)

print("Logits (pre-tanh):", logits.cpu().numpy()/np.linalg.norm(logits.cpu().numpy()))
print("Tanh output:", squashed.cpu().numpy()/np.linalg.norm(squashed.cpu().numpy()))
print("Predicted (from .predict):", data/np.linalg.norm(data))
print("Norm:", np.linalg.norm(data))