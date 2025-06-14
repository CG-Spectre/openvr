//
// Created by aidan on 6/12/2025.
//

#include "model.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <mfobjects.h>
#include <pstl/glue_memory_defs.h>

model::model(std::string name, Pose3d pos) : pos(pos) {
    maxFloat = std::numeric_limits<float>::max();
    minFloat = std::numeric_limits<float>::min();
    std::ifstream file("models/" + name + ".vrobj");
    std::string line;
    bool faceMode = false;
    while (std::getline(file, line)) {
        if(line == "") {
            faceMode = true;
            continue;
        }
        if (faceMode) {
            Face3d face;
            while (line.size() > 0) {
                std::string str = line.substr(0, line.find(" "));
                if (str.starts_with("t")) {
                    line.erase(0, line.find(" ") + 1);
                    int id = std::stoi(str.substr(1, str.size() - 1));
                    face.setTextureId(id);
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
        float posY= std::stof(line.substr(0, line.find(" ")));
        line.erase(0, line.find(" ") + 1);
        float posZ = std::stof(line.substr(0, line.find(" ")));
        vertices.push_back(Vertex3d(posX, posY, posZ));
    }
    file.close();
}

SerializedObject model::getSerializedFaces(float tx, float ty, float tz) {
    tx = 0, ty = 0, tz = 0;
    std::vector<float> facesTrulySerialized;
    std::vector<int> indicesSerialized;
    std::vector<int> texturesSerialized;
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
        for (int k = 0; k < faces[faceIndex].getVertices()->size(); k++) {
            Vector3d posNS = *faces[faceIndex].getVertices()->at(k)->getPose() + this->pos.pose;
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

