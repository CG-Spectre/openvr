//
// Created by Aidan Anderson on 4/4/25.
//

#include "renderStack.h"

#include <iostream>

renderNode *renderStack::getFirst() {
    return this->first;
}

void renderStack::push(renderNode *node) {
    renderNode *tmpLast = this->last;
    this->last = node;
    if (this->first == nullptr) {
        this->first = node;
        return;
    }
    tmpLast->setNext(node);
}

renderStack::renderStack() {
    this->first = nullptr;
    this->last = nullptr;
}

void renderStack::render(SDL_Renderer *renderer) {
    if (this->first == nullptr) {
        std::cout << "Empty renderstack!" << std::endl;
        return;
    }
    renderNode *currentNode = this->first;
    while (currentNode != nullptr) {
        currentNode->getInfo()->render(renderer);
        currentNode = currentNode->getNext();
    }
}


