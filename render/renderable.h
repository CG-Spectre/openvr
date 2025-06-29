//
// Created by Aidan Anderson on 4/4/25.
//

#ifndef RENDERABLE_H
#define RENDERABLE_H
#include <SDL3/SDL_render.h>

#include "Vector3d.h"


class renderable {
public:
    virtual ~renderable() = default;

    virtual void render(SDL_Renderer *renderer) = 0;

    virtual void render(SDL_Renderer * renderer, Vector3d vector3d, Vector3d rotational_velocity) = 0;
};



#endif //RENDERABLE_H
