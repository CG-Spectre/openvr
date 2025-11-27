#pragma once

#include "renderer.h"

class RayTracingRenderer : public Renderer {
public:
    void render(Scene& scene, camera& cam, Vector3d translationalVelocity,
                Vector3d rotationalVelocity) override;
};
