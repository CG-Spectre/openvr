//
// Created by Aidan Anderson on 4/9/25.
//

#include "Vertex3d.h"

#include "../../Pose3d.h"

Vertex3d::Vertex3d(Vector3d pose) {
    this->pos = pose;
}
Vertex3d::Vertex3d(float x, float y, float z) {
    this->pos = Vector3d(x, y, z);
}
Vertex3d::Vertex3d() {
    this->pos = Vector3d(0, 0, 0);
}
Vector3d *Vertex3d::getPose() {
    return &this->pos;
}
void Vertex3d::setPose(Vector3d pose) {
    this->pos = pose;
}
void Vertex3d::addConnection(Vertex3d *connection) {
    this->connections.push_back(connection);
}
std::vector<Vertex3d*> *Vertex3d::getConnections() {
    return &this->connections;
}
