//
// Created by aidan on 6/11/2025.
//

#ifndef LIGHT_H
#define LIGHT_H
#include <vector>

#include "Vector3d.h"


class light {
public:
    light(Vector3d pos, Vector3d color, float power);
    Vector3d position;
    Vector3d color;
    float power;
    std::vector<float> serialize();
};



#endif //LIGHT_H
