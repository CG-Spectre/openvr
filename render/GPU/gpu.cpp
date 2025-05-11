//
// Created by aidan on 4/16/2025.
//
#include <GL/glew.h>
#include "gpu.h"
#include <fstream>
#include <iostream>
#include <SDL3/SDL_events.h>

#include "render/SDLConfig.h"


cl::Device gpu::device;
cl::Context gpu::context;
cl::Program gpu::program;
cl::Kernel gpu::precomputeRayTrig;
cl::Kernel gpu::renderPixel;
cl::Kernel gpu::mapTexture;
cl::Kernel gpu::fillTexture;
SDL_GLContext* gpu::glContext;
SDL_Window* gpu::window;
GLuint gpu::vertexShader;
GLuint gpu::fragmentShader;
GLuint gpu::shaderProgram;
GLuint gpu::pbo;
cl_mem gpu::image;
GLuint gpu::texture;
GLuint gpu::vao;


const char* gpu::vertexShaderSrc = R"(
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

const char* gpu::fragmentShaderSrc = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTex;
void main() {
    FragColor = texture(screenTex, TexCoord);
}
)";
bool gpu::checkCompile(GLuint shader, const char* type) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << type << " SHADER COMPILATION FAILED:\n" << log << "\n";
        return false;
    }
    return true;
}
GLuint gpu::compileShader(GLenum type, const char* src) {
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
GLuint gpu::createProgram() {
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
void gpu::resize() {
    if (texture) glDeleteTextures(1, &texture);
    if (pbo) glDeleteBuffers(1, &pbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (image) clReleaseMemObject(image);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *SDLConfig::WINDOW_WIDTH, *SDLConfig::WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    //std::cout << *SDLConfig::WINDOW_WIDTH << ", " << *SDLConfig::WINDOW_HEIGHT << std::endl;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, *SDLConfig::WINDOW_WIDTH * *SDLConfig::WINDOW_HEIGHT*4, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    gpu::image = clCreateFromGLBuffer(context.get(), CL_MEM_WRITE_ONLY, gpu::pbo, nullptr);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    if (!shaderProgram) {
        shaderProgram = createProgram();
        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "screenTex"), 0);
    }
    glViewport(0, 0, *SDLConfig::WINDOW_WIDTH, *SDLConfig::WINDOW_HEIGHT);
}
void gpu::initialize() {

    //std::vector<std::string> kernels = {"precomputeRayTrig"};
    std::ifstream kernelFile("kernel.cl");
    std::string src(std::istreambuf_iterator<char>(kernelFile), {});
    kernelFile.close();
    if (src.empty()) {
        std::cerr << "Kernel file not found or empty!" << std::endl;
        return;
    }
    cl::Program::Sources sources;
    sources.push_back({src.c_str(), src.size()});
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.empty()) {
        std::cerr << "No OpenCL platforms found!" << std::endl;
        return;
    }
    cl::Platform platform = cl::Platform::getDefault();
    std::cout << "OpenCL platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    HGLRC currentContext = wglGetCurrentContext();
    HDC currentDC = wglGetCurrentDC();
    if (currentContext == nullptr || currentDC == nullptr) {
        std::cerr << "Error: Invalid OpenGL context or device context." << std::endl;
    }else {
        std::cout << "Initialized OpenGL context and device context." << std::endl;
    }
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
        CL_GL_CONTEXT_KHR, (cl_context_properties)(currentContext),
        CL_WGL_HDC_KHR, (cl_context_properties)(currentDC),
        0
    };

    cl::Context context(CL_DEVICE_TYPE_GPU, properties);
    gpu::context = context;
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    gpu::device = devices[0];
    std::cout << "Using " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    cl::Program program(context, sources);
    if (program.build(devices) != CL_SUCCESS) {
        std::cerr << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
        return;
    }
    std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
    if (extensions.find("cl_khr_gl_sharing") == std::string::npos) {
        std::cerr << "Interop not supported on this device!" << std::endl;
        exit(1);
    }
    gpu::program = program;

    gpu::precomputeRayTrig = cl::Kernel(program, "precomputeRayTrig");
    gpu::renderPixel = cl::Kernel(program, "renderPixel");
    gpu::mapTexture = cl::Kernel(program, "mapTexture");
    gpu::fillTexture = cl::Kernel(program, "fillTexture");
    resize();



    /*GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fs);
    checkCompile(fs, "FRAGMENT");
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vs);
    checkCompile(vs, "VERTEX");
    gpu::vertexShader = vs;
    gpu::fragmentShader = fs;*/
    std::cout << "Successfully compiled shaders!" << std::endl;
    std::cout << "Successfully initialized GPU kernel!" << std::endl;

}
void gpu::setGLContext(SDL_GLContext* context) {
    glContext = context;
}
SDL_GLContext* gpu::getGLContext() {
    return glContext;
}

void gpu::setWindow(SDL_Window* context) {
    window = context;
}
SDL_Window* gpu::getWindow() {
    return window;
}
