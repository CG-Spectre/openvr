//
// Created by aidan on 4/16/2025.
//

#ifndef GPU_H
#define GPU_H
#include <CL/cl.hpp>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_video.h>

//tmp
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

class gpu {
public:
    static cl::Context context;
    static cl::Device device;
    static cl::Program program;
    static cl::Kernel precomputeRayTrig;
    static cl::Kernel renderPixel;
    static cl::Kernel mapTexture;
    static cl::Kernel fillTexture;




    static void initialize();
    static SDL_GLContext* getGLContext();

    static void setWindow(SDL_Window *context);

    static SDL_Window *getWindow();
    static GLuint texture;
    static void setGLContext(SDL_GLContext* context);
    static GLuint vertexShader;
    static GLuint fragmentShader;
    static GLuint shaderProgram;
    static GLuint pbo;
    static cl_mem image;
    static GLuint vao;
    static void resize();
private:
    static SDL_GLContext* glContext;
    static SDL_Window* window;
    static const char *vertexShaderSrc;
    static const char *fragmentShaderSrc;


    static bool checkCompile(GLuint shader, const char *type);

    static GLuint compileShader(GLenum type, const char *src);

    static GLuint createProgram();


};



#endif //GPU_H
