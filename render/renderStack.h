//
// Created by Aidan Anderson on 4/4/25.
//

#ifndef RENDERSTACK_H
#define RENDERSTACK_H
#include "renderNode.h"


class renderStack : public renderable{
public:
    renderStack();
    renderNode *getFirst();
    void push(renderNode *node);
    void render(SDL_Renderer *renderer) override;
private:
    renderNode *first;
    renderNode *last;
};



#endif //RENDERSTACK_H
