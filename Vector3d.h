//
// Created by Aidan Anderson on 4/7/25.
//

#ifndef VECTOR3D_H
#define VECTOR3D_H



class Vector3d {
    public:
    Vector3d();
    Vector3d(float x, float y, float z);

    Vector3d cross(const Vector3d &other);

    Vector3d normalize();

    float dot(const Vector3d &other);

    Vector3d operator*(float scalar);

    Vector3d operator+(const Vector3d &other);

    Vector3d operator-(const Vector3d &other);

    float x;
    float y;
    float z;
};



#endif //VECTOR3D_H
