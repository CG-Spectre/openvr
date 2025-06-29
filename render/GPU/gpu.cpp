//
// Created by aidan on 4/16/2025.
//
#include <GL/glew.h>
#include "gpu.h"
#include <fstream>
#include <iostream>
#include <SDL3/SDL_events.h>

#include "render/SDLConfig.h"
#include "render/3d/TextureManager.h"


cl::Device gpu::device;
cl::Context gpu::context;
cl::Program gpu::program;
cl::Kernel gpu::precomputeRayTrig;
cl::Kernel gpu::renderPixel;
cl::Kernel gpu::mapTexture;
cl::Kernel gpu::fillTexture;
cl::Kernel gpu::clearScreen;
SDL_GLContext* gpu::glContext;
SDL_Window* gpu::window;
GLuint gpu::vertexShader;
GLuint gpu::fragmentShader;
GLuint gpu::shaderProgram;
GLuint gpu::pbo;
cl_mem gpu::image;
GLuint gpu::texture;
GLuint gpu::vao;
cl_mem gpu::textures;
cl::CommandQueue gpu::queue;
cl_sampler gpu::sampler;
TextureManager gpu::textureManager;
cl::Buffer gpu::widthsSerialized;
cl::Buffer gpu::heightsSerialized;
cl::Buffer gpu::uvSerialized;

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
void gpu::initialize(TextureManager texManager) {
    gpu::textureManager = texManager;
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
    cl::Program program;
    int mode = 1;
    if (mode == 0) {
        std::ifstream binFile("openvrkernel.bin", std::ifstream::binary | std::ios::ate);
        if (!binFile.is_open()) {
            std::cerr << "Failed to open kernel binary file\n";
            return;
        }
        std::streamsize size = binFile.tellg();
        binFile.seekg(0, std::ios::beg);
        std::vector<unsigned char> binary(size);
        if (!binFile.read(reinterpret_cast<char*>(binary.data()), size)) {
            std::cerr << "Failed to read kernel binary file\n";
            return;
        }
        cl::Program::Binaries binaries = {binary};
        std::vector<cl::Device> singleDevice = {device};
        program = cl::Program(context, singleDevice, binaries);

    }else if (mode == 1) {
        program = cl::Program(context, sources);
    }

    if (program.build(devices) != CL_SUCCESS) {
        std::cerr << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
        return;
    }

    std::vector<size_t> binarySizes = program.getInfo<CL_PROGRAM_BINARY_SIZES>();
    std::vector<std::vector<unsigned char>> binaryData(binarySizes.size());

    for (size_t i = 0; i < binarySizes.size(); ++i) {
        binaryData[i].resize(binarySizes[i]);
    }

    std::vector<unsigned char*> binaryPointers(binarySizes.size());
    for (size_t i = 0; i < binarySizes.size(); ++i) {
        binaryPointers[i] = binaryData[i].data();
    }

    cl_program rawProgram = program();

    cl_int err2 = clGetProgramInfo(
        rawProgram,
        CL_PROGRAM_BINARIES,
        binaryPointers.size() * sizeof(unsigned char*),
        binaryPointers.data(),
        nullptr
    );

    if (err2 != CL_SUCCESS) {
        std::cerr << "clGetProgramInfo failed: " << err2 << "\n";
        return;
    }

    std::ofstream binFile("openvrkernel.bin", std::ios::binary);
    binFile.write(reinterpret_cast<char*>(binaryData[0].data()), binarySizes[0]);
    binFile.close();

    std::cout << "Compiled kernel binary." << std::endl;

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
    gpu::clearScreen = cl::Kernel(program, "clearScreen");
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

    cl_image_format imageFormat;
    imageFormat.image_channel_data_type = CL_UNORM_INT8;
    imageFormat.image_channel_order = CL_RGBA;
    cl_image_desc desc;
    memset(&desc, 0, sizeof(desc));
    desc.image_type = CL_MEM_OBJECT_IMAGE2D;
    desc.image_width = texManager.width;
    desc.image_height = texManager.height;
    cl_int err;
    cl_mem imageTex = clCreateImage(context.get(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, & imageFormat, & desc, texManager.image, &err);
    if (err != CL_SUCCESS) {
        std::cout << "Error loading image textures onto GPU." << std::endl;
        return;
    }
    gpu::textures = imageTex;
    std::cout << "Successfully loaded textures!" << std::endl;

    gpu::queue = cl::CommandQueue(gpu::context, gpu::device);
    gpu::sampler = clCreateSampler(context.get(), CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, &err);
    if (err != CL_SUCCESS) {
        std::cout << "Error creating image sampler." << std::endl;
        return;
    }
    uvSerialized = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, textureManager.serialized.uvSerialized.size() * sizeof(int), textureManager.serialized.uvSerialized.data());
    widthsSerialized = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, textureManager.serialized.widthsSerialized.size() * sizeof(int), textureManager.serialized.widthsSerialized.data());
    heightsSerialized = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, textureManager.serialized.heightsSerialized.size() * sizeof(int), textureManager.serialized.heightsSerialized.data());
    std::cout << "Successfully initialized GPU kernel!" << std::endl;
    size_t logsize;
    clGetProgramBuildInfo(program.get(), device.get(), CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
    char *log = (char *)malloc(logsize);
    clGetProgramBuildInfo(program.get(), device.get(), CL_PROGRAM_BUILD_LOG, logsize, log, NULL);

    printf("Build log:\n%s\n", log);
    free(log);

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
