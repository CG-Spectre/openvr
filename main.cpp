#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

#include "render/camera.h"
#include "render/renderDummy.h"
#include "render/renderStack.h"
#include "render/SDLConfig.h"
#include "render/3d/cube.h"
#include "render/GPU/gpu.h"
int width, height;
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("OpenVR", 640, 480, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    gpu::initialize();
    SDL_GetWindowSize(window, &width, &height);
    SDLConfig::WINDOW_WIDTH = &width;
    SDLConfig::WINDOW_HEIGHT = &height;

    bool running = true;
    SDL_Event event;
    renderStack stack;

    camera cam(Pose3d(0, 0, 0, 0, 0, 0));
    cam.addObject(new cube(Pose3d(Vector3d(0, 0, 8), Vector3d(0, 0, 0)), 2));
    stack.push(new renderNode(&cam));

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        SDL_GetWindowSize(window, &width, &height);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        stack.render(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(0);
        cam.getPos()->setRotation(Vector3d(0, 0, cam.getPos()->rotation.z + 1));
        // Game loop code goes here
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
