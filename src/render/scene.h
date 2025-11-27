#pragma once

#include <memory>
#include <string>
#include <vector>

#include "render/3d/BVHNode.h"
#include "render/3d/cube.h"
#include "render/3d/light.h"
#include "render/3d/model.h"
#include "render/3d/plane.h"

class Scene {
public:
    Scene();

    renderableRT* addObject(std::unique_ptr<renderableRT> object);
    light* addLight(std::unique_ptr<light> sceneLight);

    renderableRT* addCube(const Pose3d& pose, float edgeLength);
    renderableRT* addPlane(const Pose3d& pose, float scale);
    renderableRT* addModel(const std::string& name, const Pose3d& pose, float scale = 1.0f);

    void rebuildSimpleBVH();

    std::vector<renderableRT*> getObjectPointers() const;
    std::vector<light*> getLightPointers() const;

    BVHNode& getRootBVH();

private:
    std::vector<std::unique_ptr<renderableRT>> objects;
    std::vector<std::unique_ptr<light>> lights;
    BVHNode rootBVH;
};
