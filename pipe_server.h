//
// Created by aidan on 6/21/2025.
//

#ifndef PIPE_SERVER_H
#define PIPE_SERVER_H
#include <string>

#include "Pose3d.h"
#include "render/camera.h"


class pipe_server {
public:
    static void start(Pose3d* pos, Vector3d* refDir);
    static void setData(camera::indirectLightingResult data);
private:
    static std::wstring utf8_to_wstring(const std::string& str);
    static int awaitingData;
    static camera::indirectLightingResult data;
};



#endif //PIPE_SERVER_H
