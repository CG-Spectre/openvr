//
// Created by aidan on 6/11/2025.
//

#include "light.h"

light::light(Vector3d pos, Vector3d color, float power) {
    this->power = power;
    this->position = pos;
    this->color = color;
}

std::vector<float> light::serialize() {
    std::vector<float> data;
    data.push_back(position.x);
    data.push_back(position.y);
    data.push_back(position.z);
    data.push_back(color.x);
    data.push_back(color.y);
    data.push_back(color.z);
    data.push_back(power);
    return data;
}

