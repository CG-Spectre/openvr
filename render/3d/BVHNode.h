//
// Created by aidan on 6/9/2025.
//

#ifndef BVHNODE_H
#define BVHNODE_H
#include <vector>
#include <array>

#include "SerializedBVH.h"
#include "Vector3d.h"


class BVHNode {
public:
    BVHNode();
    BVHNode( Vector3d p1, Vector3d p2);
    std::vector<int> objects{};
    Vector3d point1;
    Vector3d point2;
    std::vector<BVHNode> children;
    std::array<Vector3d, 2> getRectBounds(std::vector<Vector3d> mins, std::vector<Vector3d> maxs);

    SerializedBVH serialize();
    struct result{std::vector<std::vector<float>> resultVector; int count;};
    result flatten(int idStart);

    float maxFloat;
    float minFloat;
    std::array<Vector3d, 2> bounds;
};



#endif //BVHNODE_H
