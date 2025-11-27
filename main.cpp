#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>

#include <SDL3/SDL.h>

#include "pipe_server.h"
#include "GL/glew.h"
#include "render/camera.h"
#include "render/renderDummy.h"
#include "render/renderStack.h"
#include "render/SDLConfig.h"
#include "render/3d/cube.h"
#include "render/3d/model.h"
#include "render/3d/plane.h"
#include "render/3d/TextureManager.h"
#include "render/GPU/gpu.h"
#include "src/app/input_controller.h"
#include "src/app/window.h"
#include "src/render/raytracer.h"
#include "src/render/scene_loader.h"

int main(int argc, char* argv[]) {
    bool controlserver = false;
    bool useReflectDir = false;
    Vector3d reflectDir = Vector3d(0, 0, 0);
    Vector3d reflectDir1 = Vector3d(0, 0, 0);
    Vector3d reflectDir2 = Vector3d(0, 0, 0);
    Vector3d reflectDir3 = Vector3d(0, 0, 0);
    Pose3d pos(0, 0, 0, 0, 0.1, 0);
    if (controlserver) {
        std::thread pipeserver([&pos, &reflectDir, &reflectDir1, &reflectDir2, &reflectDir3]() {
            pipe_server::start(&pos, &reflectDir, &reflectDir1, &reflectDir2, &reflectDir3);
        });
        pipeserver.detach();
    }

    WindowContext windowContext;
    if (!windowContext.initialize("OpenVR", 640, 480)) {
        return 1;
    }

    TextureManager texManager;
    SceneLoader loader(texManager);
    loader.loadDefaultTextures();

    camera cam(Pose3d(0, 0, 0, 0, 0.1, 0));
    if (useReflectDir) {
        cam.setReflectDir(&reflectDir, &reflectDir1, &reflectDir2, &reflectDir3);
    }
    cam.init();

    Scene scene = loader.loadDefaultScene();
    cam.setScene(&scene);

    RayTracingRenderer renderer;
    InputController inputController;

    bool running = true;
    SDL_Event event;
    auto lastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    int frames = 0;
    const bool* kb = SDL_GetKeyboardState(nullptr);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        windowContext.syncWindowSize();

        auto current = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        frames++;
        float dt = static_cast<float>(current - lastUpdate);
        float fps = (frames > 0) ? ((float)frames) / (dt / 100) : 1.0f;
        if (frames > 200) {
            frames = 0;
            lastUpdate = current;
        }

        Vector3d translationalVelocity;
        Vector3d rotationalVelocity;
        inputController.updateCameraFromInput(kb, fps, cam, translationalVelocity, rotationalVelocity);

        if (controlserver) {
            cam.getPos()->pose = pos.pose;
            cam.getPos()->rotation = pos.rotation;
        }

        renderer.render(scene, cam, translationalVelocity, rotationalVelocity);
    }

    cam.stop();
    texManager.freeTextures();
    return 0;
}
