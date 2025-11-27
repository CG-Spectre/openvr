#include "window.h"

#include <GL/glew.h>
#include <iostream>

#include "render/SDLConfig.h"
#include "render/GPU/gpu.h"

WindowContext::WindowContext() = default;

WindowContext::~WindowContext() {
    if (context) {
        SDL_GL_DestroyContext(context);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

bool WindowContext::initialize(const char* title, int defaultWidth, int defaultHeight) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(title, defaultWidth, defaultHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetSwapInterval(0);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        return false;
    }

    syncWindowSize();
    SDLConfig::WINDOW_WIDTH = &width;
    SDLConfig::WINDOW_HEIGHT = &height;
    gpu::setGLContext(&context);
    gpu::setWindow(window);

    return true;
}

void WindowContext::syncWindowSize() {
    if (window) {
        SDL_GetWindowSize(window, &width, &height);
    }
}
