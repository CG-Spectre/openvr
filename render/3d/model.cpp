//
// Created by aidan on 6/12/2025.
//

#include "model.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <mfobjects.h>
#include <pstl/glue_memory_defs.h>

model::model(std::string name, Pose3d pos, float scale) : pos(pos), scale(scale) {
    maxFloat = std::numeric_limits<float>::max();
    minFloat = std::numeric_limits<float>::min();
    std::ifstream file("models/" + name + ".vrobj");
    std::string line;
    bool faceMode = false;
    while (std::getline(file, line)) {
        if (line == "") {
            faceMode = true;
            continue;
        }
        if (faceMode) {
            Face3d face;
            while (line.size() > 0) {
                std::string str = line.substr(0, line.find(" "));
                if (str.starts_with("t")) {
                    line.erase(0, line.find(" ") + 1);
                    std::string idstr = str.substr(1, str.size() - 1);
                    int id;
                    if (idstr.starts_with("0x")) {
                        //std::cout << std::stoi("0x525dff") << std::endl;
                        id = std::stoi(idstr, nullptr, 16) + 100000;
                    }else {
                        id = std::stoi(idstr);
                    }
                    face.setTextureId(id);
                    //std::cout << id << std::endl;
                    continue;
                }
                int id = std::stoi(str);
                line.erase(0, line.find(" ") + 1);
                face.addVertex(&vertices[id]);
            }
            faces.push_back(face);
            continue;
        }
        float posX = std::stof(line.substr(0, line.find(" ")));
        line.erase(0, line.find(" ") + 1);
        float posY = std::stof(line.substr(0, line.find(" ")));
        line.erase(0, line.find(" ") + 1);
        float posZ = std::stof(line.substr(0, line.find(" ")));
        vertices.push_back(Vertex3d(posX, posY, posZ));
    }
    file.close();
}

model::model(std::string name, Pose3d pos) : model(name, pos, 1){}

SerializedObject model::getSerializedFaces(float tx, float ty, float tz) {
    tx = 0, ty = 0, tz = 0;
    std::vector<float> facesTrulySerialized;
    std::vector<int> indicesSerialized;
    std::vector<int> texturesSerialized;
    float cosP = std::cos(this->pos.rotation.x * (M_PI / 180));
    float sinP = std::sin(this->pos.rotation.x * (M_PI / 180));
    float cosY = std::cos(this->pos.rotation.y * (M_PI / 180));
    float sinY = std::sin(this->pos.rotation.y * (M_PI / 180));
    float cosR = std::cos(this->pos.rotation.z * (M_PI / 180));
    float sinR = std::sin(this->pos.rotation.z * (M_PI / 180));
    float minX = maxFloat, minY = maxFloat, minZ = maxFloat;
    float maxX = minFloat, maxY = minFloat, maxZ = minFloat;
    SerializedObject output;
    for (int i = 0; i < faces.size(); i++) {
        int faceIndex = i;
        int start = facesTrulySerialized.size();
        indicesSerialized.push_back(start);
        //std::cout << faces[faceIndex]->getVertices()->size() << std::endl;
        texturesSerialized.push_back(faces[faceIndex].textureId);
        texturesSerialized.push_back(faces[faceIndex].textureRotation);
        texturesSerialized.push_back(faces[faceIndex].normalMap);
        texturesSerialized.push_back(faces[faceIndex].specularMap);
        texturesSerialized.push_back(faces[faceIndex].reflectionMap);
        texturesSerialized.push_back(faces[faceIndex].displacementMap);
        for (int k = 0; k < faces[faceIndex].getVertices()->size(); k++) {
            Vector3d posRaw = (*faces[faceIndex].getVertices()->at(k)->getPose());
            posRaw = Vector3d(
                posRaw.x,
                posRaw.y * cosP - posRaw.z * sinP,
                posRaw.z * cosP + posRaw.y * sinP
            );
            posRaw = Vector3d(
                posRaw.x * cosY + posRaw.z * sinY,
                posRaw.y,
                -posRaw.x*sinY + posRaw.z * cosY
            );
            posRaw = Vector3d(
                posRaw.x * cosR - posRaw.y * sinR,
                posRaw.x * sinR + posRaw.y*cosR,
                posRaw.z
            );
            Vector3d posNS = posRaw*scale + this->pos.pose;
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
    return output;
}

void model::render(SDL_Renderer *renderer) {

}

void model::render(SDL_Renderer * renderer, Vector3d vector3d, Vector3d rotational_velocity) {

}