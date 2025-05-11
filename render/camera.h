//
// Created by Aidan Anderson on 4/7/25.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "renderable.h"
#include "renderableRT.h"
#include "renderStack.h"
#include "../Pose3d.h"


class camera : public renderable {
public:
    camera();
    camera(Pose3d pos);
    void render(SDL_Renderer *renderer) override;
    void addObject(renderableRT *object);
    Pose3d* getPos();
private:
    int prevWidth;
    int prevHeight;
    Pose3d pos;
    renderStack stack;
};



#endif //CAMERA_H
