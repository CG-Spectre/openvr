//
// Created by aidan on 4/22/2025.
//

#ifndef SERIALIZEDOBJECT_H
#define SERIALIZEDOBJECT_H
#include <vector>

#include "Vector3d.h"


class SerializedObject {
    public:
      std::vector<float> serialized;
      std::vector<int> indices;
        std::vector<int> textures;
      Vector3d minPos, maxPos;
};



#endif //SERIALIZEDOBJECT_H
