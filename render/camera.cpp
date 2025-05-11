//
// Created by Aidan Anderson on 4/7/25.
//
#include <GL/glew.h>
#include "camera.h"

#include <cmath>
#include <iostream>
#include <thread>
#include <CL/cl.hpp>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_opengl.h>

#include "renderableRT.h"
#include "SDLConfig.h"
#include "3d/renderUtil.h"
#include "3d/Vertex3d.h"
#include "GPU/gpu.h"
#include <glm.hpp>

camera::camera() {
    this->stack = renderStack();
    this->prevWidth = *SDLConfig::WINDOW_WIDTH;
    this->prevHeight = *SDLConfig::WINDOW_HEIGHT;
}

camera::camera(Pose3d pos) {
    this->pos = pos;
    this->prevWidth = *SDLConfig::WINDOW_WIDTH;
    this->prevHeight = *SDLConfig::WINDOW_HEIGHT;
}

void camera::addObject(renderableRT *object) {
    this->stack.push(new renderNode(object));
}

Pose3d* camera::getPos() {
    return &this->pos;
}

void camera::render(SDL_Renderer *renderer) {
    Vector3d rayPoint = this->pos.pose;
    float yaw = this->pos.rotation.y;
    float pitch = this->pos.rotation.x;
    float roll = this->pos.rotation.z;

    int width = *SDLConfig::WINDOW_WIDTH;
    int height = *SDLConfig::WINDOW_HEIGHT;
    if (width != this->prevWidth || height != this->prevHeight) {
        this->prevWidth = *SDLConfig::WINDOW_WIDTH;
        this->prevHeight = *SDLConfig::WINDOW_HEIGHT;
        std::cout << "resize" << std::endl;
        gpu::resize();
    }

    cl::Buffer outputBuffer(gpu::context, CL_MEM_WRITE_ONLY, width * height*3 * sizeof(float));
    cl::CommandQueue queue(gpu::context, gpu::device);
    /*gpu::precomputeRayTrig.setArg(0, width);
    gpu::precomputeRayTrig.setArg(1, height);
    gpu::precomputeRayTrig.setArg(2, yaw);
    gpu::precomputeRayTrig.setArg(3, pitch);
    gpu::precomputeRayTrig.setArg(4, outputBuffer);
    queue.enqueueNDRangeKernel(gpu::precomputeRayTrig, cl::NullRange, cl::NDRange(width, height));*/
    //serialize and format all objects in the scene
    renderStack stack2 = this->stack;
    std::vector<int> indicesSquared;
    std::vector<int> indicesOfIndices;
    std::vector<float> allObjectsSerialized;
    std::vector<int> allIndicesSerialized;
    renderNode *currentNode2 = stack2.getFirst();
    while (currentNode2 != nullptr) {
        SerializedObject objs = dynamic_cast<renderableRT*>(currentNode2->getInfo())->getSerializedFaces();
        indicesSquared.push_back(allObjectsSerialized.size());
        indicesOfIndices.push_back(allIndicesSerialized.size());
        allObjectsSerialized.reserve(allObjectsSerialized.size() + objs.serialized.size());
        allObjectsSerialized.insert(allObjectsSerialized.end(), objs.serialized.begin(), objs.serialized.end());
        allIndicesSerialized.reserve(allIndicesSerialized.size() + objs.indices.size());
        allIndicesSerialized.insert(allIndicesSerialized.end(), objs.indices.begin(), objs.indices.end());
        currentNode2 = currentNode2->getNext();
    }
    /*for (int i =0; i < allObjectsSerialized.size(); i++) {
        std::cout << allObjectsSerialized[i] << ", ";
    }
    std::cout << std::endl;*/
    //end of serialization
    cl::Buffer outputBuffer2 = cl::Buffer(gpu::context, CL_MEM_WRITE_ONLY, width * height *4* sizeof(float));
    gpu::renderPixel.setArg(0, width);
    gpu::renderPixel.setArg(1, height);
    cl::Buffer indicesSquaredBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, indicesSquared.size() * sizeof(int), indicesSquared.data());
    cl::Buffer indicesOfIndicesBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, indicesOfIndices.size() * sizeof(int), indicesOfIndices.data());
    cl::Buffer allObjectsSerializedBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, allObjectsSerialized.size() * sizeof(int), allObjectsSerialized.data());
    cl::Buffer allIndicesSerializedBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, allIndicesSerialized.size() * sizeof(int), allIndicesSerialized.data());
    gpu::renderPixel.setArg(2, outputBuffer);
    gpu::renderPixel.setArg(3, indicesSquaredBuffer);
    gpu::renderPixel.setArg(4, indicesOfIndicesBuffer);
    gpu::renderPixel.setArg(5, allObjectsSerializedBuffer);
    gpu::renderPixel.setArg(6, allIndicesSerializedBuffer);
    gpu::renderPixel.setArg(7, static_cast<int>(indicesSquared.size()));
    gpu::renderPixel.setArg(8, static_cast<int>(indicesOfIndices.size()));
    gpu::renderPixel.setArg(9, static_cast<int>(allObjectsSerialized.size()));
    gpu::renderPixel.setArg(10, static_cast<int>(allIndicesSerialized.size()));
    gpu::renderPixel.setArg(11, outputBuffer2);
    gpu::renderPixel.setArg(12, rayPoint.x);
    gpu::renderPixel.setArg(13, rayPoint.y);
    gpu::renderPixel.setArg(14, rayPoint.z);
    gpu::renderPixel.setArg(15, yaw);
    gpu::renderPixel.setArg(16, pitch);
    gpu::renderPixel.setArg(17, roll);
    clSetKernelArg(gpu::renderPixel.get(), 18, sizeof(cl_mem), &gpu::image);
    gpu::renderPixel.setArg(19, SDLConfig::FOCAL_LENGTH);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gpu::pbo);
    void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    memset(ptr, 0, width*height*4);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    clEnqueueAcquireGLObjects(queue.get(), 1, &gpu::image, 0, nullptr, nullptr);
    queue.enqueueNDRangeKernel(gpu::renderPixel, cl::NullRange, cl::NDRange(width, height));
    clEnqueueReleaseGLObjects(queue.get(), 1, &gpu::image, 0, nullptr, nullptr);

    glBindTexture(GL_TEXTURE_2D, gpu::texture);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gpu::pbo);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(gpu::shaderProgram);
    glBindVertexArray(gpu::vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gpu::texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    SDL_GL_SwapWindow(gpu::getWindow());

    /*// Setup VAO, VBO, EBO (only once!)
    cl::Buffer imgBuffer(gpu::context, CL_MEM_WRITE_ONLY, width * height * sizeof(cl_uchar4));
    //std::cout << err << std::endl;
    gpu::mapTexture.setArg(0, yaw);
    gpu::mapTexture.setArg(1, pitch);
    gpu::mapTexture.setArg(2, roll);
    gpu::mapTexture.setArg(3, width);
    gpu::mapTexture.setArg(4, height);
    gpu::mapTexture.setArg(5, outputBuffer2);
    gpu::mapTexture.setArg(6, imgBuffer);
    gpu::mapTexture.setArg(7, SDLConfig::FOCAL_LENGTH);

    gpu::fillTexture.setArg(0, width);
    gpu::fillTexture.setArg(1, height);
    gpu::fillTexture.setArg(2, imgBuffer);

    queue.enqueueNDRangeKernel(gpu::fillTexture, cl::NullRange, cl::NDRange(width, height));
    queue.finish();
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    GLuint pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * sizeof(cl_uchar4), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    struct Vertex { glm::vec2 pos, uv; };
    std::vector<Vertex> quad = {
        {{-1,-1}, {0,0}}, {{+1,-1}, {1,0}}, {{-1,+1}, {0,1}},
        {{-1,+1}, {0,1}}, {{+1,-1}, {1,0}}, {{+1,+1}, {1,1}}
    };
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, quad.size()*sizeof(Vertex), quad.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);        // position
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(2*sizeof(float))); // uv
    glBindVertexArray(0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    void* pboPtr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    */


    /*std::vector<float> data(width*height*4, 0);
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, width*height*4*sizeof(int), data.data());*/
    /*GLuint tex2;
    glGenTextures(1, &tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    cl_int error;

    cl_mem clImage = clCreateFromGLTexture(gpu::context.get(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, tex2, &error);

    gpu::mapTexture.setArg(0, yaw);
    gpu::mapTexture.setArg(1, pitch);
    gpu::mapTexture.setArg(2, roll);
    gpu::mapTexture.setArg(3, width);
    gpu::mapTexture.setArg(4, height);
    gpu::mapTexture.setArg(5, outputBuffer2);
    gpu::mapTexture.setArg(6, sizeof(cl_mem), &clImage);
    gpu::mapTexture.setArg(7, SDLConfig::FOCAL_LENGTH);
    //gpu::fillTexture.setArg(0, image);
    clEnqueueAcquireGLObjects(queue.get(), 1, &clImage, 0, nullptr, nullptr);
    queue.enqueueNDRangeKernel(gpu::mapTexture, cl::NullRange, cl::NDRange(width, height));
    clEnqueueReleaseGLObjects(queue.get(), 1, &clImage, 0, nullptr, nullptr);
    clFinish(queue.get());
    //std::vector<unsigned char> testPixels(width * height * 4, 255); // solid white
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, testPixels.data());
    /*void* pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    int pitch2;
    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_LockTexture(tex, NULL, &pixels, &pitch2);
    //queue.enqueueReadBuffer(out, CL_TRUE, 0, height*pitch2, pixels);
    SDL_UnlockTexture(tex);
    int threadAmt = 24;//std::thread::hardware_concurrency()/2;
    int pixelsPerThread = width/threadAmt;#1#
    glViewport(0, 0, width, height);
    /*float vertices[] = {
        // x, y, u, v
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // UV attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create white 1x1 texture
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };#1#
    /*glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint program = glCreateProgram();
    glAttachShader(program, gpu::vertexShader);
    glAttachShader(program, gpu::fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glUseProgram(0);
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);#1#
    GLuint VAO, VBO, EBO;

    // Generate the VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create VBO and EBO
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Example vertex data: x, y, u, v
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    // Bind and upload data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set attribute pointers: position (0), UV (1)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint program = glCreateProgram();
    glAttachShader(program, gpu::vertexShader);
    glAttachShader(program, gpu::fragmentShader);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glBindVertexArray(VAO);
    SDL_GL_SwapWindow(gpu::getWindow());
    std::cout << "done" << std::endl;
    //SDL_UpdateTexture(tex, NULL, pixels, width * sizeof(uint32_t));
    //SDL_RenderTexture(renderer, tex, NULL, NULL);*/

}


