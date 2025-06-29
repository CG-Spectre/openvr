# pipe_client.py
import time
import win32pipe, win32file, pywintypes
import json

pipe_name = r'\\.\pipe\MyPipe'

print("Connecting to C++ server...")
while True:
    try:
        handle = win32file.CreateFile(
            pipe_name,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0,
            None,
            win32file.OPEN_EXISTING,
            0,
            None
        )
        break
    except pywintypes.error as e:
        if e.args[0] != 2:  # ERROR_FILE_NOT_FOUND
            raise
        time.sleep(1)

print("Connected to C++ server.")
#win32file.WriteFile(handle, b"Hello from Python!")
win32file.FlushFileBuffers(handle)
# Optional: Read response from C++
#_, data = win32file.ReadFile(handle, 64)
#print("Received from C++:", data.decode())
posX = 0
posY = 0
posZ = 0
yaw = 0
pitch = 0
roll = 0

def getRadiance(actionX, actionY, actionZ):
    win32file.WriteFile(handle, f"1 {actionX} {actionY} {actionZ}".encode("utf-8"))
    win32file.FlushFileBuffers(handle)
    _, data = win32file.ReadFile(handle, 512)
    res = json.loads(data.decode())
    return res["radiance"]

def update(dPosX, dPosY, dPosZ, dYaw, dPitch, dRoll):
    global posX, posY, posZ, yaw, pitch, roll
    posX += dPosX
    posY += dPosY
    posZ += dPosZ
    yaw += dYaw
    pitch += dPitch
    roll += dRoll
    win32file.WriteFile(handle, f"0 {dPosX} {dPosY} {dPosZ} {dYaw} {dPitch} {dRoll}".encode("utf-8"))
    win32file.FlushFileBuffers(handle)
    _, data = win32file.ReadFile(handle, 512)
    res = json.loads(data.decode())
    return res["interX"], res["interY"], res["interZ"], res["normalX"], res["normalY"], res["normalZ"], res["reflectX"], res["reflectY"], res["reflectZ"], res["albedo"], res["shininess"]

#while(True):
