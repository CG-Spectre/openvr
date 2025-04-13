//
// Created by Aidan Anderson on 4/4/25.
//

#ifndef RENDERNODE_H
#define RENDERNODE_H
#include "renderable.h"


class renderNode {
    public:
    renderNode();
    renderNode(renderable *info);
    renderable* getInfo();
    renderNode* getNext();
    void setInfo(renderable *info);
    void setNext(renderNode *next);
    private:
    renderable *info;
    renderNode *next;
};



#endif //RENDERNODE_H
