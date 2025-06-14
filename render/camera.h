//
// Created by Aidan Anderson on 4/7/25.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "renderable.h"
#include "renderableRT.h"
#include "renderStack.h"
#include "../Pose3d.h"
#include "3d/BVHNode.h"
#include "3d/light.h"


class camera : public renderable {
public:
    camera();
    camera(Pose3d pos);
    void render(SDL_Renderer *renderer) override;
    void addObject(renderableRT *object);
    Pose3d* getPos();
    BVHNode rootBVH;
    void addLight(light *light);
private:
    bool time = false;
    int prevWidth;
    int prevHeight;
    Pose3d pos;
    renderStack stack;
    renderStack lightStack;
    std::vector<light *> lights;
};



#endif //CAMERA_H
