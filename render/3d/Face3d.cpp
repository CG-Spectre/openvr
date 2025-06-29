//
// Created by Aidan Anderson on 4/12/25.
//

#include "Face3d.h"

Face3d::Face3d() {

}


std::vector<Vertex3d*>* Face3d::getVertices() {
    return &vertices;
}

void Face3d::addVertex(Vertex3d *v) {
    vertices.push_back(v);
}

void Face3d::setTextureId(int id) {
    this->textureId = id;
}

void Face3d::setTextureRotation(int rot) {
    this->textureRotation = rot;
}

void Face3d::setNormalMap(int id) {
    this->normalMap = id;
}

void Face3d::setSpecularMap(int id) {
    this->specularMap = id;
}

void Face3d::setReflectionMap(int id) {
    this->reflectionMap = id;
}

void Face3d::setDisplacementMap(int id) {
    this->displacementMap = id;
}

