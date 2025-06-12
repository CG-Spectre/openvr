//
// Created by aidan on 6/9/2025.
//

#include "BVHNode.h"

#include <iostream>
#include <limits>

#include "SerializedBVH.h"


BVHNode::BVHNode() {
    maxFloat = std::numeric_limits<float>::max();
    minFloat = std::numeric_limits<float>::min();
}

BVHNode::BVHNode(Vector3d p1, Vector3d p2) {
    this->point1 = p1;
    this->point2 = p2;
    maxFloat = std::numeric_limits<float>::max();
    minFloat = std::numeric_limits<float>::min();
}

std::array<Vector3d, 2> BVHNode::getRectBounds(std::vector<Vector3d> mins, std::vector<Vector3d> maxs) {
    float minX = maxFloat, minY = maxFloat, minZ = maxFloat;
    float maxX = minFloat, maxY = minFloat, maxZ = minFloat;
    for (int i = 0; i < objects.size(); i++) {
        minX = std::min(minX, mins[objects[i]].x);
        maxX = std::max(maxX, maxs[objects[i]].x);
        minY = std::min(minY, mins[objects[i]].y);
        maxY = std::max(maxY, maxs[objects[i]].y);
        minZ = std::min(minZ, mins[objects[i]].z);
        maxZ = std::max(maxZ, maxs[objects[i]].z);
    }
    for (int i = 0; i < children.size(); i++) {
        std::array<Vector3d, 2> res = children[i].getRectBounds(mins, maxs);
        minX = std::min(minX, res[0].x);
        maxX = std::max(maxX, res[1].x);
        minY = std::min(minY, res[0].y);
        maxY = std::max(maxY, res[1].y);
        minZ = std::min(minZ, res[0].z);
        maxZ = std::max(maxZ, res[1].z);
    }

    this->bounds = {Vector3d(minX, minY, minZ), Vector3d(maxX, maxY, maxZ)};
    return this->bounds;
}

SerializedBVH BVHNode::serialize() {
    std::vector<std::vector<float>> flattened = this->flatten(0).resultVector;
    SerializedBVH serialized;
    int counter = 0;
    for (int i = 0; i < flattened.size(); i++) {
        serialized.indices.push_back(counter);
        for (int k = 0; k < flattened[i].size(); k++) {
            counter++;
            serialized.data.push_back(flattened[i][k]);
        }
    }
    return serialized;
}

BVHNode::result BVHNode::flatten(int idStart) {
    std::vector<float> flattened;

    // Step 1: Add bounding box (6 floats)
    flattened.push_back(this->bounds[0].x);
    flattened.push_back(this->bounds[0].y);
    flattened.push_back(this->bounds[0].z);
    flattened.push_back(this->bounds[1].x);
    flattened.push_back(this->bounds[1].y);
    flattened.push_back(this->bounds[1].z);

    int childStartIndex = idStart + 1; // First child will start after this node
    int currentIndex = childStartIndex;

    // Step 2: Add children indices
    for (int i = 0; i < children.size(); i++) {
        flattened.push_back(currentIndex);
        // We'll add the actual child data below and compute how many nodes it added
        result childResult = children[i].flatten(currentIndex);
        currentIndex += childResult.count;
    }

    // Step 3: Add object IDs as negative numbers
    for (int i = 0; i < objects.size(); i++) {
        flattened.push_back(-(objects[i]+1));
    }

    // Step 4: Start building the result
    std::vector<std::vector<float>> resultVector;
    resultVector.push_back(flattened);

    // Step 5: Add all child flattened data
    currentIndex = childStartIndex;
    for (int i = 0; i < children.size(); i++) {
        result childResult = children[i].flatten(currentIndex);
        resultVector.insert(resultVector.end(),
                            childResult.resultVector.begin(),
                            childResult.resultVector.end());
        currentIndex += childResult.count;
    }

    // Step 6: Return the final result
    return result{resultVector, currentIndex - idStart};
}

//std::vector<float>


