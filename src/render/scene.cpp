#include "scene.h"

Scene::Scene() = default;

renderableRT* Scene::addObject(std::unique_ptr<renderableRT> object) {
    renderableRT* raw = object.get();
    objects.push_back(std::move(object));
    return raw;
}

light* Scene::addLight(std::unique_ptr<light> sceneLight) {
    light* raw = sceneLight.get();
    lights.push_back(std::move(sceneLight));
    return raw;
}

renderableRT* Scene::addCube(const Pose3d& pose, float edgeLength) {
    return addObject(std::make_unique<cube>(pose, edgeLength));
}

renderableRT* Scene::addPlane(const Pose3d& pose, float scale) {
    return addObject(std::make_unique<plane>(pose, scale));
}

renderableRT* Scene::addModel(const std::string& name, const Pose3d& pose, float scale) {
    return addObject(std::make_unique<model>(name, pose, scale));
}

void Scene::rebuildSimpleBVH() {
    rootBVH.children.clear();
    rootBVH.objects.clear();
    for (size_t i = 0; i < objects.size(); ++i) {
        rootBVH.children.emplace_back();
        rootBVH.children.back().objects.push_back(static_cast<int>(i));
    }
}

std::vector<renderableRT*> Scene::getObjectPointers() const {
    std::vector<renderableRT*> result;
    result.reserve(objects.size());
    for (const auto& obj : objects) {
        result.push_back(obj.get());
    }
    return result;
}

std::vector<light*> Scene::getLightPointers() const {
    std::vector<light*> result;
    result.reserve(lights.size());
    for (const auto& l : lights) {
        result.push_back(l.get());
    }
    return result;
}

BVHNode& Scene::getRootBVH() {
    return rootBVH;
}
