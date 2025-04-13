//
// Created by Aidan Anderson on 4/4/25.
//

#ifndef RENDERDUMMY_H
#define RENDERDUMMY_H
#include "renderable.h"
#include "string"

class renderDummy : public renderable{
    public:
    renderDummy(std::string message);
    void render(SDL_Renderer *renderer) override;
    private:
    std::string message;
};



#endif //RENDERDUMMY_H
