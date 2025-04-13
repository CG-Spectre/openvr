//
// Created by Aidan Anderson on 4/4/25.
//

#include "renderNode.h"

renderable *renderNode::getInfo() {
    return this->info;
}

renderNode *renderNode::getNext() {
    return this->next;
}

renderNode::renderNode() {
    this->info = nullptr;
    this->next = nullptr;
}
renderNode::renderNode(renderable *info) {
    this->info = info;
    this->next = nullptr;
}

void renderNode::setInfo(renderable *info) {
    this->info = info;
}

void renderNode::setNext(renderNode *next) {
    this->next = next;
}



