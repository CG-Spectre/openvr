//
// Created by Aidan Anderson on 4/7/25.
//

#include "Vector3d.h"
#include "cmath"

Vector3d::Vector3d() {
    x = 0;
    y = 0;
    z = 0;
}

Vector3d::Vector3d(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Vector3d Vector3d::cross(const Vector3d& other) {
    return {
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    };
}

Vector3d Vector3d::normalize()
{
    float magnitude = std::sqrt(x * x + y * y + z * z);
    return Vector3d(
        x/magnitude,
        y/magnitude,
        z/magnitude
    );
}

float Vector3d::dot(const Vector3d& other) {
    return x * other.x + y * other.y + z * other.z;
}

Vector3d Vector3d::operator*(float scalar) {
    return { x * scalar, y * scalar, z * scalar };
}

Vector3d Vector3d::operator+(const Vector3d& other) {
    return { x + other.x, y + other.y, z + other.z };
}
Vector3d Vector3d::operator-(const Vector3d& other)
{
    return { x - other.x, y - other.y, z - other.z };
}



