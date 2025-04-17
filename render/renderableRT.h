//
// Created by Aidan Anderson on 4/12/25.
//

#ifndef RENDERABLERT_H
#define RENDERABLERT_H
#include "renderable.h"
#include "../Vector3d.h"


class camera;

class renderableRT : public renderable {
public:
    virtual ~renderableRT() = default;
    virtual bool renderRay(SDL_Renderer *renderer, camera* camera, Vector3d point, Vector3d direction) = 0;
};



#endif //RENDERABLERT_H
