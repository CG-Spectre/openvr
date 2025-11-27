#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_opengl.h>
#include <memory>

class WindowContext {
public:
    WindowContext();
    ~WindowContext();

    bool initialize(const char* title, int defaultWidth, int defaultHeight);

    SDL_Window* getWindow() const { return window; }
    SDL_GLContext getContext() const { return context; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    void syncWindowSize();

private:
    SDL_Window* window{nullptr};
    SDL_GLContext context{nullptr};
    int width{0};
    int height{0};
};
