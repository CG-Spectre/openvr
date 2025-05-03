//
// Created by Aidan Anderson on 4/7/25.
//

#include "Pose3d.h"

Pose3d::Pose3d() {
    this->pose = Vector3d(0, 0, 0);
    this->rotation = Vector3d(0, 0, 0);
}

Pose3d::Pose3d(Vector3d translation, Vector3d rotation) {
    this->pose = translation;
    this->rotation = rotation;
}
Pose3d::Pose3d(float x, float y, float z, float pitch, float yaw, float roll) {
    this->pose = Vector3d(x, y, z);
    this->rotation = Vector3d(pitch, yaw, roll);
}
void Pose3d::setTranslation(Vector3d translation) {
    this->pose = translation;
}
void Pose3d::setRotation(Vector3d rotation) {
    this->rotation = rotation;
}

