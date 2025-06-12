//
// Created by aidan on 5/30/2025.
//

#include "plane.h"

#include <cmath>
#include <iostream>
#include <ostream>

plane::plane(Pose3d pos, float edgeLength) : plane(pos, edgeLength, 7){
    maxFloat = std::numeric_limits<float>::max();
    minFloat = std::numeric_limits<float>::min();
}

plane::plane(Pose3d pos, float edgeLength, int texture) {
    maxFloat = std::numeric_limits<float>::max();
    minFloat = std::numeric_limits<float>::min();
    float cosY = std::cos(pos.rotation.y * (M_PI / 180));
    float sinY = std::sin(pos.rotation.y * (M_PI / 180));
    float cosX = std::cos(pos.rotation.x * (M_PI / 180));
    float sinX = std::sin(pos.rotation.x * (M_PI / 180));
    float cosZ = std::cos(pos.rotation.z * (M_PI / 180));
    float sinZ = std::sin(pos.rotation.z * (M_PI / 180));
    this->pos = pos;
    float halfedge = edgeLength / 2.0f;
    float x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
    y1 = 0; y2 = 0; y3 = 0; y4 = 0;
    x1 = halfedge; x2 = -halfedge; x3 = halfedge; x4 = -halfedge;
    z1 = halfedge; z2 = halfedge; z3 = -halfedge; z4 = -halfedge;
    float tmpx = x1, tmpy = y1;
    x1 = x1*cosY - z1*sinY;
    z1 = z1*cosY + tmpx*sinY;
    y1 = y1*cosX - z1*sinX;
    z1 = z1*cosX + tmpy*sinX;
    x1 = x1*cosZ - y1*sinZ;
    y1 = y1*cosZ + tmpx*sinZ;

    tmpx = x2, tmpy = y2;
    x2 = x2*cosY - z2*sinY;
    z2 = z2*cosY + tmpx*sinY;
    y2= y2*cosX - z2*sinX;
    z2 = z2*cosX + tmpy*sinX;
    x2 = x2*cosZ - y2*sinZ;
    y2 = y2*cosZ + tmpx*sinZ;

    tmpx = x3, tmpy = y3;
    x3 = x3*cosY - z3*sinY;
    z3 = z3*cosY + tmpx*sinY;
    y3= y3*cosX - z3*sinX;
    z3 = z3*cosX + tmpy*sinX;
    x3 = x3*cosZ - y3*sinZ;
    y3 = y3*cosZ + tmpx*sinZ;

    tmpx = x4, tmpy = y4;
    x4 = x4*cosY - z4*sinY;
    z4 = z4*cosY + tmpx*sinY;
    y4= y4*cosX - z4*sinX;
    z4 = z4*cosX + tmpy*sinX;
    x4 = x4*cosZ - y4*sinZ;
    y4 = y4*cosZ + tmpx*sinZ;

    v1 = Vertex3d(x1, y1, z1);
    v2 = Vertex3d(x2, y2, z2);
    v3 = Vertex3d(x3, y3, z3);
    v4 = Vertex3d(x4, y4, z4);
    // std::cout << x1 + pos.pose.x << ", " << y1 + pos.pose.y << ", " << z1 + pos.pose.z << std::endl;
    //std::cout << x2 + pos.pose.x << ", " << y2 + pos.pose.y << ", " << z2 + pos.pose.z << std::endl;
    //std::cout << x3 + pos.pose.x << ", " << y3 + pos.pose.y << ", " << z3 + pos.pose.z << std::endl;
    //std::cout << x4 + pos.pose.x << ", " << y4 + pos.pose.y << ", " << z4 + pos.pose.z << std::endl;
    f1.addVertex(&v1);
    f1.addVertex(&v2);
    f1.addVertex(&v4);
    f1.addVertex(&v3);

    faces.push_back(&f1);
    this->texture = texture;
    f1.setTextureId(this->texture);
}


void plane::render(SDL_Renderer *renderer) {

}


SerializedObject plane::getSerializedFaces(float tx, float ty, float tz) {
    //std::cout << tx << std::endl;
    tx = 0, ty = 0, tz = 0;
    std::vector<float> facesTrulySerialized;
    std::vector<int> indicesSerialized;
    std::vector<int> texturesSerialized;
    SerializedObject output;
    float minX = maxFloat, minY = maxFloat, minZ = maxFloat;
    float maxX = minFloat, maxY = minFloat, maxZ = minFloat;
    for (int i = 0; i < faces.size(); i++) {
        int faceIndex = i;
        int start = facesTrulySerialized.size();
        indicesSerialized.push_back(start);
        //std::cout << faces[faceIndex]->getVertices()->size() << std::endl;
        texturesSerialized.push_back(faces[faceIndex]->textureId);
        texturesSerialized.push_back(faces[faceIndex]->textureRotation);
        for (int k = 0; k < faces[faceIndex]->getVertices()->size(); k++) {
            Vector3d posNS = *faces[faceIndex]->getVertices()->at(k)->getPose() + this->pos.pose;
            facesTrulySerialized.push_back(posNS.x);
            facesTrulySerialized.push_back(posNS.y);
            facesTrulySerialized.push_back(posNS.z);
            minX = std::min(minX, posNS.x);
            maxX = std::max(maxX, posNS.x);
            minY = std::min(minY, posNS.y);
            maxY = std::max(maxY, posNS.y);
            minZ = std::min(minZ, posNS.z);
            maxZ = std::max(maxZ, posNS.z);
        }
    }
    output.serialized = facesTrulySerialized;
    output.indices = indicesSerialized;
    output.textures = texturesSerialized;
    output.minPos = Vector3d(minX, minY, minZ);
    output.maxPos = Vector3d(maxX, maxY, maxZ);
    //std::cout << output.serialized.size() << std::endl;
    return output;
}