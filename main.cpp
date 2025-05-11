#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include "GL/glew.h"
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow("OpenVR", 640, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    //SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0);
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        return 1;
    }else {
        std::cout << "GLEW initialization successful" << std::endl;
    }
    if (!context) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        exit(1);
    }
    /*if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }*/
    SDL_GetWindowSize(window, &width, &height);
    SDLConfig::WINDOW_WIDTH = &width;
    SDLConfig::WINDOW_HEIGHT = &height;
    gpu::initialize();
    gpu::setGLContext(&context);
    gpu::setWindow(window);



    bool running = true;
    SDL_Event event;
    renderStack stack;

    camera cam(Pose3d(0, 0, 0, 0, 0, 0));
    cam.addObject(new cube(Pose3d(Vector3d(0, 0, 8), Vector3d(0, 0, 0)), 2));
    stack.push(new renderNode(&cam));
    auto lastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    int frames = 0;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        SDL_GetWindowSize(window, &width, &height);
        SDL_Renderer* renderer = nullptr;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        stack.render(renderer);

        //SDL_RenderPresent(renderer);
        cam.getPos()->setRotation(Vector3d(0, 0, cam.getPos()->rotation.z + 1));
        // Game loop code goes here
        auto current = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        frames++;
        std::cout << ((float)frames)/((float)(current - lastUpdate)/100) << std::endl;
        if (frames > 200) {
            frames = 0;
            lastUpdate = current;
        }
        //lastUpdate = current;
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
/*#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GL/glext.h>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define WIDTH 512
#define HEIGHT 512

const char* vertexShaderSrc = R"(
#version 330 core
out vec2 TexCoord;
const vec2 verts[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);
const vec2 uvs[4] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);
void main() {
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
    TexCoord = uvs[gl_VertexID];
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTex;
void main() {
    FragColor = texture(screenTex, TexCoord);
}
)";

const char* clKernelSrc = R"(
__kernel void fillBlue(__global uchar4* image) {
    int gid = get_global_id(0);
    if(gid % 3 == 0){
        image[gid] = (uchar4)(255, 255, 255, 255);
        return;
    }
    image[gid] = (uchar4)(0, 0, 255, 255);
}
)";

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return shader;
}

GLuint createProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow("OpenCL + OpenGL", WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    glewInit();

    // Texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // PBO
    GLuint pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, WIDTH * HEIGHT * 4, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // OpenCL setup
    cl_platform_id platform;
    cl_device_id device;
    cl_context clCtx;
    cl_command_queue queue;

    clGetPlatformIDs(1, &platform, nullptr);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    cl_context_properties props[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0
    };
    clCtx = clCreateContext(props, 1, &device, nullptr, nullptr, nullptr);
    queue = clCreateCommandQueueWithProperties(clCtx, device, nullptr, nullptr);

    cl_mem clImage = clCreateFromGLBuffer(clCtx, CL_MEM_WRITE_ONLY, pbo, nullptr);

    cl_program program = clCreateProgramWithSource(clCtx, 1, &clKernelSrc, nullptr, nullptr);
    clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
    cl_kernel kernel = clCreateKernel(program, "fillBlue", nullptr);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &clImage);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint shaderProg = createProgram();
    glUseProgram(shaderProg);
    glUniform1i(glGetUniformLocation(shaderProg, "screenTex"), 0);

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        // CL: run kernel
        glFinish();
        clEnqueueAcquireGLObjects(queue, 1, &clImage, 0, nullptr, nullptr);
        size_t globalSize = WIDTH * HEIGHT;
        clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
        clEnqueueReleaseGLObjects(queue, 1, &clImage, 0, nullptr, nullptr);
        clFinish(queue);

        // Update texture from PBO
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProg);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    clReleaseMemObject(clImage);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(clCtx);
    glDeleteBuffers(1, &pbo);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shaderProg);
    glDeleteVertexArrays(1, &vao);

    SDL_GL_DestroyContext(glCtx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}*/


/*
const char* kernelSrc = R"(
__kernel void fillWhite(__global uchar4* image) {
    int gid = get_global_id(0);
    //image[gid] = (uchar4)(0, 0, 255, 255);
}
)";

void checkCLError(cl_int err, const char* msg) {
    if (err != CL_SUCCESS) {
        std::cerr << msg << " (Error code: " << err << ")\n";
        exit(1);
    }
}

int main() {
    constexpr int width = 800, height = 600;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Allocate texture storage but don't upload any data yet (nullptr)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set texture filtering (important!)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("CL-GL Interop", width, height, SDL_WINDOW_OPENGL);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glewInit();

    // Create GL Pixel Buffer Object
    GLuint pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * 4, nullptr, GL_DYNAMIC_DRAW);
    glFinish();
    // Get CL platform/device/context with GL interop
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_int err;

    clGetPlatformIDs(1, &platform, nullptr);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    auto glCtx = wglGetCurrentContext();
    auto hdc   = wglGetCurrentDC();
    if (!glCtx || !hdc) {
        std::cerr << "❌ OpenGL context or HDC not available (interop won't work)\n";
    } else {
        std::cout << "✅ OpenGL context and HDC are valid.\n";
    }
    // Log OpenGL version
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

    // Check GLEW init
    if (glewInit() != GLEW_OK) {
        std::cerr << "❌ GLEW initialization failed.\n";
    } else {
        std::cout << "✅ GLEW initialized.\n";
    }
    GLuint debugPBO = 0;
    glGenBuffers(1, &debugPBO);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, debugPBO);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 128 * 128 * 4, nullptr, GL_DYNAMIC_DRAW);
    GLint pboSize = 0;
    glGetBufferParameteriv(GL_PIXEL_UNPACK_BUFFER, GL_BUFFER_SIZE, &pboSize);
    if (pboSize <= 0) {
        std::cerr << "❌ PBO was not allocated properly.\n";
    } else {
        std::cout << "✅ PBO allocated (" << pboSize << " bytes).\n";
    }
    cl_context_properties props[] = {
        CL_GL_CONTEXT_KHR,   (cl_context_properties)glCtx,
        CL_WGL_HDC_KHR,      (cl_context_properties)hdc,
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0
    };
    context = clCreateContext(props, 1, &device, nullptr, nullptr, &err);
    checkCLError(err, "Creating CL context");

    queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
    checkCLError(err, "Creating CL queue");
    char extensions[2048];
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(extensions), extensions, nullptr);
    std::cout << "CL Device Extensions: " << extensions << "\n";
    // Create shared OpenCL buffer from GL PBO
    cl_mem clPBO = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, pbo, &err);
    checkCLError(err, "Creating CL buffer from GL PBO");

    // Compile OpenCL program
    const char* sources[] = { kernelSrc };
    cl_program program = clCreateProgramWithSource(context, 1, sources, nullptr, &err);
    checkCLError(err, "Creating program");

    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        char log[2048];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, nullptr);
        std::cerr << "Build log:\n" << log << "\n";
        exit(1);
    }

    cl_kernel kernel = clCreateKernel(program, "fillWhite", &err);
    checkCLError(err, "Creating kernel");

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &clPBO);

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }
        */

        /*
        // CL: acquire GL buffer
        glFinish(); // make sure GL is done with it
        clEnqueueAcquireGLObjects(queue, 1, &clPBO, 0, nullptr, nullptr);

        // Launch kernel
        size_t globalSize = width * height;
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &clPBO);
        //clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);

        // CL: release GL buffer
        clEnqueueReleaseGLObjects(queue, 1, &clPBO, 0, nullptr, nullptr);
        clFinish(queue);

        // 2. Bind the PBO and update the texture from it
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);*/

        // 3. Render fullscreen quad
        /*glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D); // Only needed if using legacy fixed-function
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (ptr) {
            memset(ptr, 255, width * height * 4); // Set all pixels to black
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        if (ptr) {
            unsigned char* pixels = (unsigned char*)ptr;
            for (int i = 0; i < width * height; ++i) {
                pixels[i * 4 + 0] = 255; // R
                pixels[i * 4 + 1] = 0;   // G
                pixels[i * 4 + 2] = 0;   // B
                pixels[i * 4 + 3] = 255; // A
            }
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glDisable(GL_BLEND);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1, -1);
        glTexCoord2f(1, 0); glVertex2f(1, -1);
        glTexCoord2f(1, 1); glVertex2f(1, 1);
        glTexCoord2f(0, 1); glVertex2f(-1, 1);
        glEnd();
        const GLubyte* version = glGetString(GL_VERSION);
        std::cout << "OpenGL Version: " << version << std::endl;
        SDL_GL_SwapWindow(window);
        /*std::cout << "here" << std::endl;
        // Render PBO to screen
        glClear(GL_COLOR_BUFFER_BIT);
        glRasterPos2i(-1, -1); // bottom-left corner
        glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        SDL_GL_SwapWindow(window);#1#
    }

    // Cleanup
    clReleaseMemObject(clPBO);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    glDeleteBuffers(1, &pbo);
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();



    return 0;
}*/
