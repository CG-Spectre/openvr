import math

import gym
import json
import numpy as np
import pipe_client
from stable_baselines3 import PPO
import torch

class RadianceEnv(gym.Env):
    currentStep = 0
    def __init__(self):
        super().__init__()
        self.observation_space = gym.spaces.Box(low=-np.inf, high=np.inf, shape=(12,), dtype=np.float32)
        self.action_space = gym.spaces.Box(low=-1.0, high=1.0, shape=(3,), dtype=np.float32)
    def reset(self):
        self.state = self.newState()
        self.currentStep += 1
        if(np.isnan(self.state).any()):
            print("Warning: NAN Found, resetting")
            return self.reset()
        return self.state
    def step(self, action):
        if np.isnan(action).any():
            print("Warning: NaN action detected")
            return self.state, 0, True, {}
        norm = np.linalg.norm(action)
        if(norm < 1e-3):
            direction = np.array([0, 0, 1], dtype=np.float32)
        else:
            direction = action/norm

        x,y,z = direction
        if(z < 0):
            return self.state, 0, True, {}
        radiance = pipe_client.getRadiance(x, y, z)
        if np.isnan(radiance) or not np.isfinite(radiance):
            print("Warning: got NaN or inf reward, skipping step")
            reward = 0
        else:
            reward = radiance
        done = True
        return self.state, reward, done, {}
    def newState(self):
        x, z, dPitch, yaw = self.stepPos()
        interX, interY, interZ, normalX, normalY, normalZ, reflectX, reflectY, reflectZ, albedo, shininess = pipe_client.update(x, 0, z+4, yaw, dPitch, 0)
        return np.array([
            albedo,  # albedo
            interX,  # interX
            interY,
            interZ,
            normalX,
            normalY,
            normalZ,
            0.0,  # radiance (ignored)
            shininess,  # shininess
            reflectX,
            reflectY,
            reflectZ
        ], dtype=np.float32)
    def stepPos(self):
        Pitch = math.sin(self.currentStep/10)*10 - 2
        z =4*math.sin(self.currentStep/50)
        x = 4*math.cos(self.currentStep/50)
        Yaw = 270 - (180/math.pi)*(self.currentStep/50) + math.cos(self.currentStep/11)*8
        """ + (180/math.pi)*(self.currentStep/50)"""
        return x, z, Pitch, Yaw


if __name__ == "__main__":
    #inst = RadianceEnv()
    #print(inst.newState())
    #pipe_client.getRadiance(0, 0, 1)
    torch.autograd.set_detect_anomaly(True)
    env = RadianceEnv()
    model = PPO("MlpPolicy", env, verbose=1, clip_range=0.1, learning_rate=1e-4)
    model.learn(total_timesteps=1_000_000)
    model.save("direction_policy")


