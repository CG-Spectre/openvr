//
// Created by aidan on 6/21/2025.
//

#include "pipe_server.h"
#include <locale>
#include <codecvt>
// pipe_server.cpp
#include <windows.h>
#include <iostream>
#include <json.hpp>
#include <thread>

using json = nlohmann::json;

camera::indirectLightingResult pipe_server::data;
int pipe_server::awaitingData;

void pipe_server::setData(camera::indirectLightingResult datad) {
    data = datad;
    if (awaitingData == 1) {
        awaitingData++;
    }else if (awaitingData == 2) {
        awaitingData = 0;
    }


}

void pipe_server::start(Pose3d* pos, Vector3d* refDir) {
    const char* pipeName = R"(\\.\pipe\MyPipe)";

    HANDLE hPipe = CreateNamedPipeA(
        pipeName,                // Pipe name
        PIPE_ACCESS_DUPLEX,     // Read/Write access
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // Byte stream pipe
        1,                      // Max instances
        1024,                   // Output buffer size
        1024,                   // Input buffer size
        0,                      // Default timeout
        NULL                    // Default security attributes
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create pipe.\n";
        return;
    }

    std::cout << "Waiting for Python client...\n";
    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (connected) {
        std::cout << "Client connected.\n";
    }
    char buffer[512];
    DWORD bytesRead, bytesWritten;
    while (true) {
        // Read from client
        BOOL success = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!success || bytesRead == 0) {
            std::cerr << "ReadFile failed or client disconnected. Error: " << GetLastError() << std::endl;
            BOOL connected = FALSE;
            while (!connected) {
                connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
                if (connected) {
                    std::cout << "Client connected.\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

        }
        buffer[bytesRead] = '\0';
        std::string str(buffer);
        //std::cout << "Received: " << bytesRead << std::endl;
        int mode = std::stoi(str.substr(0, str.find(" ")));
        str.erase(0, str.find(" ") + 1);
        if (mode == 0) {
            float x = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            float y = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            float z = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            float yaw = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            float pitch = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            float roll = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);

            pos->pose.x = x;
            pos->pose.y = y;
            pos->pose.z = z;
            pos->rotation.y = yaw;
            pos->rotation.x = pitch;
            pos->rotation.z = roll;
            //std::cout << pos->pose.x << " " << pos->pose.y << " " << pos->pose.z << std::endl;
            awaitingData = 1;
            while (awaitingData) {}
            //std::cout << intersection.x << " " << intersection.y << " " << intersection.z << std::endl;
            std::string resp = json{
                {"interX", data.interX},
                {"interY", data.interY},
                {"interZ", data.interZ},
                {"normalX", data.normalX},
                {"normalY", data.normalY},
                {"normalZ", data.normalZ},
                {"reflectX", data.reflectX},
                {"reflectY", data.reflectY},
                {"reflectZ", data.reflectZ},
                {"albedo", data.albedo},
                {"shininess", data.shininess},
                {"radiance", data.radiance}
            }.dump();
            WriteFile(hPipe, resp.c_str(), resp.size(), &bytesWritten, NULL);
        }else if (mode == 1) {

            float x = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            float y = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            float z = std::stof(str.substr(0, str.find(" ")));
            str.erase(0, str.find(" ") + 1);
            refDir->x = x;
            refDir->y = y;
            refDir->z = z;
            awaitingData = 1;
            while (awaitingData) {
                //std::cout << awaitingData << std::endl;
            }
            std::string resp = json{
                    {"radiance", data.radiance}
            }.dump();
            WriteFile(hPipe, resp.c_str(), resp.size(), &bytesWritten, NULL);
        }
    }

    std::cout << "Pipe stopped." << std::endl;
    CloseHandle(hPipe);
}

std::wstring pipe_server::utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}