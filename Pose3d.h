//
// Created by Aidan Anderson on 4/7/25.
//

#ifndef POSE3D_H
#define POSE3D_H
#include "Vector3d.h"


class Pose3d {
public:
    Pose3d(Vector3d translation, Vector3d rotation);

    Pose3d(float x, float y, float z, float pitch, float yaw, float roll);

    Pose3d();
    Vector3d rotation;
    Vector3d pose;
    void setTranslation(Vector3d translation);

    void setRotation(Vector3d translation);
};



#endif //POSE3D_H
