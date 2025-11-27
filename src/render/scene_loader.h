#pragma once

#include "render/3d/TextureManager.h"
#include "render/GPU/gpu.h"
#include "scene.h"

class SceneLoader {
public:
    explicit SceneLoader(TextureManager& textureManager);

    void loadDefaultTextures();
    Scene loadDefaultScene();

private:
    TextureManager& textureManager;
};
