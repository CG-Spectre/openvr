//
// Created by aidan on 4/13/2025.
//

#include "renderUtil.h"

#include <cmath>
#include <iostream>

#include "../camera.h"
#include "../SDLConfig.h"

class camera;

void renderUtil::render2dPoint(SDL_Renderer *renderer, float x, float y) {
    SDL_RenderPoint(renderer,(((float)*SDLConfig::WINDOW_WIDTH)/2)+x, (((float)*SDLConfig::WINDOW_HEIGHT)/2)-y);
}

void renderUtil::render3dPoint(SDL_Renderer *renderer, camera *camera, Vector3d point) {
    //float yaw = std::atan2(x, z) * (180.0 / M_PI);
    //float pitch = std::asin(-y) * 180.0 / M_PI;
    //float roll = 0;
    float x = point.x;
    float y = point.y;
    float z = point.z;
    x -= camera->getPos()->pose.x;
    y -= camera->getPos()->pose.y;
    z -= camera->getPos()->pose.z;
    //std::cout << "x: " << camera->getPos()->pose.x << std::endl;
    //std::cout << x << " " << y << " " << z << std::endl;
    float yaw = camera->getPos()->rotation.y;
    float pitch = camera->getPos()->rotation.x;
    float roll = camera->getPos()->rotation.z;
    //std::cout << yaw << ", " << pitch << ", " << roll << std::endl;
    float tmpx = x;
    x = x*cos(yaw*(M_PI/180)) - z*sin(yaw*(M_PI/180));
    z = z * cos(yaw*(M_PI/180)) + tmpx * sin(yaw*(M_PI/180));
    float tmpy = y;
    y = y * cos(pitch*(M_PI/180)) - z * sin(pitch*(M_PI/180));
    z = z * cos(pitch*(M_PI/180)) + tmpy * sin(pitch*(M_PI/180));
    tmpx = x;
    x = x * cos(roll*(M_PI/180)) - y * sin(roll*(M_PI/180));
    y = y * cos(roll*(M_PI/180)) + tmpx * sin(roll*(M_PI/180));

    if (z <= 0) {
        z = 0.00001;
    }
    float x2d = x * SDLConfig::FOCAL_LENGTH / z;
    float y2d = y * SDLConfig::FOCAL_LENGTH / z;
    render2dPoint(renderer, x2d, y2d);

}
Vector2d renderUtil::get2dPoint(SDL_Renderer *renderer, camera *camera, Vector3d point) {
    //float yaw = std::atan2(x, z) * (180.0 / M_PI);
    //float pitch = std::asin(-y) * 180.0 / M_PI;
    //float roll = 0;
    //std::cout << camera->getPos()->pose.x << std::endl;
    float x = point.x;
    float y = point.y;
    float z = point.z;
    //x -= camera->getPos()->pose.x;
    //std::cout << camera->getPos()->pose.x << std::endl;
   // y -= camera->getPos()->pose.y;
    //z -= camera->getPos()->pose.z;
    //std::cout << "x: " << camera->getPos()->pose.x << std::endl;
    //std::cout << x << " " << y << " " << z << std::endl;
    float yaw = camera->getPos()->rotation.y;
    float pitch = camera->getPos()->rotation.x;
    float roll = camera->getPos()->rotation.z;
    //std::cout << yaw << ", " << pitch << ", " << roll << std::endl;
    float tmpx = x;
    x = x*cos(yaw*(M_PI/180)) - z*sin(yaw*(M_PI/180));
    z = z * cos(yaw*(M_PI/180)) + tmpx * sin(yaw*(M_PI/180));
    float tmpy = y;
    y = y * cos(pitch*(M_PI/180)) - z * sin(pitch*(M_PI/180));
    z = z * cos(pitch*(M_PI/180)) + tmpy * sin(pitch*(M_PI/180));
    tmpx = x;
    x = x * cos(roll*(M_PI/180)) - y * sin(roll*(M_PI/180));
    y = y * cos(roll*(M_PI/180)) + tmpx * sin(roll*(M_PI/180));

    if (z <= 0) {
        z = 0.00001;
    }
    float x2d = x * SDLConfig::FOCAL_LENGTH / z;
    float y2d = y * SDLConfig::FOCAL_LENGTH / z;
    return Vector2d((((float)*SDLConfig::WINDOW_WIDTH)/2)+x2d, (((float)*SDLConfig::WINDOW_HEIGHT)/2)-y2d);
}

bool renderUtil::isInside(Vector3d point, std::vector<Vector3d> points) {
    bool inside = false;
    for (int i = 0; i < points.size(); i++) {
        Vector3d p = points[i];
        Vector3d np;
        if (i < points.size() - 1) {
            np = points[i+1];
        }else {
            np = points[0];
        }

        if (std::abs((np.x - p.x)*(point.z - p.z) - (np.z - p.z)*(point.x - p.x)) < 1e-6) {
            return true;
        }
        if (((np.z > point.z) != (p.z > point.z)) && (point.x < (p.x - np.x) * ((point.z - np.z) / (p.z - np.z)) + np.x)) {
            inside = !inside;
        }
    }
    return inside;
}
