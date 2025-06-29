import torch
from torch.utils.data import DataLoader
import torch.nn.functional as F

# Import your dataset and model definitions
from train import SampleDataset  # Replace with your actual file
from train import SampleDirectionNet  # Replace with your actual file

def run_model_test():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # Load the dataset
    dataset = SampleDataset("../cmake-build-debug/samples.json")
    dataloader = DataLoader(dataset, batch_size=1, shuffle=False)

    # Load the model architecture and weights
    model = SampleDirectionNet().to(device)
    model.load_state_dict(torch.load("model.pth", map_location=device))
    model.eval()

    # Run inference on the dataset
    with torch.no_grad():
        for i, (inputs, targets) in enumerate(dataloader):
            inputs = inputs.to(device)
            targets = targets.to(device)

            predicted = model(inputs)

            # Print results
            print(f"Sample {i}:")
            print(f"  Input: {inputs.cpu().numpy().flatten()}")
            print(f"  True Direction: {targets.cpu().numpy().flatten()}")
            print(f"  Predicted Direction: {predicted.cpu().numpy().flatten()}")

            # Optional: compute angular error
            cosine_sim = F.cosine_similarity(predicted, targets, dim=1)
            angle_deg = torch.acos(cosine_sim.clamp(-1, 1)) * 180 / torch.pi
            print(f"  Angle Error: {angle_deg.item():.2f}°\n")

            if i >= 4:  # limit to first 5 predictions
                break

def run_model():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # Define your custom input as a list of 9 floats
    custom_input = [
        1.0,  # albedo
        0.0, 1.0, 2.0,  # intersection X, Y, Z
        0.0, 0.0, 1.0,  # normal X, Y, Z
        100,  # radiance
        1.0  # shininess
    ]

    # Convert to tensor and reshape to batch format (1, 9)
    input_tensor = torch.tensor([custom_input], dtype=torch.float32).to(device)

    # Load the trained model
    model = SampleDirectionNet().to(device)
    model.load_state_dict(torch.load("model.pth", map_location=device))
    model.eval()

    # Run the model
    with torch.no_grad():
        output = model(input_tensor)
        direction = output[0].cpu().numpy()

    print("Predicted sample direction:")
    print(f"  X: {direction[0]:.4f}")
    print(f"  Y: {direction[1]:.4f}")
    print(f"  Z: {direction[2]:.4f}")

if __name__ == "__main__":
    run_model()