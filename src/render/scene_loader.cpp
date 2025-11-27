#include "scene_loader.h"

SceneLoader::SceneLoader(TextureManager& textureManager) : textureManager(textureManager) {}

void SceneLoader::loadDefaultTextures() {
    textureManager.addTexture("dirt.png");
    textureManager.addTexture("grass_block_side.png");
    textureManager.addTexture("turbo.png");
    textureManager.addTexture("Avatar_Aang.png");
    textureManager.addTexture("fire_0.png");
    textureManager.addTexture("bricks.jpg");
    textureManager.addTexture("bricks_normal.png");
    textureManager.addTexture("tiles.png");
    textureManager.addTexture("grass_top.png");
    textureManager.addTexture("gbn.png");
    textureManager.addTexture("bricks_displacement.jpg");
    textureManager.initialize();
    gpu::initialize(textureManager);
}

Scene SceneLoader::loadDefaultScene() {
    Scene scene;

    scene.addPlane(Pose3d(Vector3d(0, -0.5f, 0), Vector3d(0, 0, 0)), 100.0f);
    scene.addCube(Pose3d(Vector3d(0, 0, 4), Vector3d(0, 180, 0)), 1.0f);
    scene.addModel("test", Pose3d(Vector3d(0, 0, -3), Vector3d(0, 0, 0)), 1.0f);

    scene.addLight(std::make_unique<light>(Vector3d(20000, 50000, -30000), Vector3d(1, 1, 1), 70000));

    scene.rebuildSimpleBVH();

    return scene;
}
