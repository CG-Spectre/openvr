//
// Created by Aidan Anderson on 4/7/25.
//

#include "camera.h"

#include <iostream>

#include "renderableRT.h"
#include "3d/Vertex3d.h"

camera::camera() {
    this->stack = renderStack();
}

camera::camera(Pose3d pos) {
    this->pos = pos;
}

void camera::addObject(renderableRT *object) {
    this->stack.push(new renderNode(object));
}

void camera::render(SDL_Renderer *renderer) {
    Vector3d rayPoint = this->pos.pose;
    Vector3d rayDirection = Vector3d(0.12, -0.12, 1).normalize();

    renderNode *currentNode = stack.getFirst();
    while (currentNode != nullptr) {
        currentNode->getInfo()->render(renderer);
        std::cout << dynamic_cast<renderableRT *>(currentNode->getInfo())->renderRay(renderer, rayPoint, rayDirection) << std::endl;
        currentNode = currentNode->getNext();
    }
}

