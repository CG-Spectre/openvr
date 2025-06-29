import json
import torch
from torch.utils.data import Dataset, DataLoader
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import DataLoader

class SampleDirectionNet(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(9, 64)
        self.fc2 = nn.Linear(64, 64)
        self.fc3 = nn.Linear(64, 3)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = self.fc3(x)
        norm = torch.norm(x, dim=1, keepdim=True).clamp(min=1e-6)
        x = x / norm
        return x

class SampleDataset(Dataset):
    def __init__(self, json_path):
        with open(json_path, 'r') as f:
            data = json.load(f)
        required_keys = [
            "albedo", "interX", "interY", "interZ",
            "normalX", "normalY", "normalZ",
            "radiance", "shininess",
            "sampleX", "sampleY", "sampleZ"
        ]
        self.inputs = []
        self.targets = []
        count = 0
        for sample in data:
            if any(sample.get(k) is None for k in required_keys):
                print(f"Skipping sample {count} due to missing or None data.")
                continue
            count += 1
            # Prepare input vector
            inp = [
                sample["albedo"],
                sample["interX"],
                sample["interY"],
                sample["interZ"],
                sample["normalX"],
                sample["normalY"],
                sample["normalZ"],
                sample["radiance"],
                sample["shininess"]
            ]

            # Target output vector (sample direction)
            target = [
                sample["sampleX"],
                sample["sampleY"],
                sample["sampleZ"]
            ]

            # Normalize target vector to unit length
            target = np.array(target)
            norm = np.linalg.norm(target)
            if norm > 0:
                target = target / norm

            self.inputs.append(inp)
            self.targets.append(target.tolist())
        # Convert to tensors
        self.inputs = torch.tensor(self.inputs, dtype=torch.float32)
        self.targets = torch.tensor(self.targets, dtype=torch.float32)

    def __len__(self):
        return len(self.inputs)

    def __getitem__(self, idx):
        return self.inputs[idx], self.targets[idx]


def train(model, dataloader, optimizer, device):
    model.train()
    total_loss = 0.0
    for inputs, targets in dataloader:
        inputs = inputs.to(device)
        targets = targets.to(device)

        optimizer.zero_grad()
        outputs = model(inputs)

        # Use cosine similarity loss (1 - cosine similarity)
        cos_sim = F.cosine_similarity(outputs, targets, dim=1)
        loss = torch.mean(1 - cos_sim)

        loss.backward()
        optimizer.step()

        total_loss += loss.item() * inputs.size(0)

    return total_loss / len(dataloader.dataset)

if __name__ == "__main__":
    # Replace with your JSON file path
    dataset = SampleDataset("../cmake-build-debug/samples.json")
    torch.save({"inputs":dataset.inputs, "targets":dataset.targets}, "dataset.pt")
    # Example dataloader
    dataloader = DataLoader(dataset, batch_size=64, shuffle=True)

    # Iterate one batch example
    for batch_inputs, batch_targets in dataloader:
        print("Inputs shape:", batch_inputs.shape)     # (batch_size, 9)
        print("Targets shape:", batch_targets.shape)   # (batch_size, 3)
        break

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using CUDA: {torch.cuda.is_available()}")

    model = SampleDirectionNet().to(device)
    optimizer = torch.optim.Adam(model.parameters(), lr=1e-3)

    epochs = 50
    for epoch in range(epochs):
        loss = train(model, dataloader, optimizer, device)
        print(f"Epoch {epoch + 1}/{epochs}, Loss: {loss:.6f}")

    # Save model weights
    torch.save(model.state_dict(), "model.pth")
    print("Model saved to sample_direction_model.pth")



