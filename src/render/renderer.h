#pragma once

#include "scene.h"
#include "render/camera.h"

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void render(Scene& scene, camera& cam, Vector3d translationalVelocity,
                        Vector3d rotationalVelocity) = 0;
};
