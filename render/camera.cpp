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

void camera::addLight(light *object) {
    this->lights.push_back(object);
}

Pose3d* camera::getPos() {
    return &this->pos;
}
bool firstTime = true;
cl::Buffer outputBuffer;
cl::Buffer indicesSquaredBuffer;
cl::Buffer indicesOfIndicesBuffer;
cl::Buffer allObjectsSerializedBuffer;
cl::Buffer allIndicesSerializedBuffer;
cl::Buffer texturesSerializedBuffer;
cl::Buffer bvhSerializedBuffer;
cl::Buffer bvhIndicesBuffer;
cl::Buffer lightsSerializedBuffer;
cl::Buffer textureIndicesBuffer;
void camera::render(SDL_Renderer *renderer) {

    int width = *SDLConfig::WINDOW_WIDTH;
    int height = *SDLConfig::WINDOW_HEIGHT;

    time = !time;
    Vector3d rayPoint = this->pos.pose;
    float yaw = this->pos.rotation.y;
    float pitch = this->pos.rotation.x;
    float roll = this->pos.rotation.z;


    if (width != this->prevWidth || height != this->prevHeight) {
        this->prevWidth = *SDLConfig::WINDOW_WIDTH;
        this->prevHeight = *SDLConfig::WINDOW_HEIGHT;
        std::cout << "resize" << std::endl;
        gpu::resize();
    }

    //cl::CommandQueue queue(gpu::context, gpu::device);
    //serialize and format all objects in the scene
    renderStack stack2 = this->stack;
    std::vector<int> indicesSquared;
    std::vector<int> indicesOfIndices;
    std::vector<float> allObjectsSerialized;
    std::vector<int> allIndicesSerialized;
    std::vector<int> texturesSerialized;
    std::vector<int> textureIndices;
    std::vector<float> lightsSerialized;
    for (int i = 0; i < this->lights.size(); i++) {
        std::vector<float> serialized = this->lights[i]->serialize();
        lightsSerialized.reserve(lightsSerialized.size() + serialized.size());
        lightsSerialized.insert(lightsSerialized.end(), serialized.begin(), serialized.end());
    }
    renderNode *currentNode2 = stack2.getFirst();
    std::vector<Vector3d> minPos;
    std::vector<Vector3d> maxPos;
    while (currentNode2 != nullptr) {
        SerializedObject objs = dynamic_cast<renderableRT*>(currentNode2->getInfo())->getSerializedFaces(-this->pos.pose.x, -this->pos.pose.y, -this->pos.pose.z);
        minPos.push_back(objs.minPos);
        maxPos.push_back(objs.maxPos);
        indicesSquared.push_back(allObjectsSerialized.size());
        indicesOfIndices.push_back(allIndicesSerialized.size());
        textureIndices.push_back(texturesSerialized.size());
        allObjectsSerialized.reserve(allObjectsSerialized.size() + objs.serialized.size());
        allObjectsSerialized.insert(allObjectsSerialized.end(), objs.serialized.begin(), objs.serialized.end());
        allIndicesSerialized.reserve(allIndicesSerialized.size() + objs.indices.size());
        allIndicesSerialized.insert(allIndicesSerialized.end(), objs.indices.begin(), objs.indices.end());
        //std::cout << objs.serialized[0] << std::endl;
        texturesSerialized.reserve(texturesSerialized.size() + objs.textures.size());
        texturesSerialized.insert(texturesSerialized.end(), objs.textures.begin(), objs.textures.end());
        //std::cout << allIndicesSerialized.size() << std::endl;
        currentNode2 = currentNode2->getNext();
    }

    //std::cout << rootBVH.getRectBounds(minPos, maxPos)[0].x << ", " << rootBVH.getRectBounds(minPos, maxPos)[0].y << ", " << rootBVH.getRectBounds(minPos, maxPos)[0].z << std::endl;
    //std::cout << rootBVH.getRectBounds(minPos, maxPos)[1].x << ", " << rootBVH.getRectBounds(minPos, maxPos)[1].y << ", " << rootBVH.getRectBounds(minPos, maxPos)[1].z << std::endl;
    rootBVH.getRectBounds(minPos, maxPos);
    SerializedBVH rootSer = rootBVH.serialize();


    //end of serialization
    cl::Buffer outputBuffer2 = cl::Buffer(gpu::context, CL_MEM_WRITE_ONLY, width * height *4* sizeof(float));
    gpu::renderPixel.setArg(0, width);
    gpu::renderPixel.setArg(1, height);
   // if (firstTime) {
        //firstTime = false;
        //std::cout << allObjectsSerialized[0] << std::endl;
    outputBuffer = cl::Buffer(gpu::context, CL_MEM_WRITE_ONLY, width * height*3 * sizeof(float));
    indicesSquaredBuffer= cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, indicesSquared.size() * sizeof(int), indicesSquared.data());
    indicesOfIndicesBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, indicesOfIndices.size() * sizeof(int), indicesOfIndices.data());
    allObjectsSerializedBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, allObjectsSerialized.size() * sizeof(float), allObjectsSerialized.data());
    allIndicesSerializedBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, allIndicesSerialized.size() * sizeof(int), allIndicesSerialized.data());
    texturesSerializedBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, texturesSerialized.size() * sizeof(int), texturesSerialized.data());
    bvhSerializedBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, rootSer.data.size() * sizeof(float), rootSer.data.data());
    bvhIndicesBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, rootSer.indices.size() * sizeof(float), rootSer.indices.data());
    lightsSerializedBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, lightsSerialized.size() * sizeof(float), lightsSerialized.data());
    textureIndicesBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, textureIndices.size() * sizeof(int), textureIndices.data());
    //}

    gpu::renderPixel.setArg(2, outputBuffer);
    gpu::renderPixel.setArg(3, indicesSquaredBuffer);
    gpu::renderPixel.setArg(4, indicesOfIndicesBuffer);
    gpu::renderPixel.setArg(5, allObjectsSerializedBuffer);
    gpu::renderPixel.setArg(6, allIndicesSerializedBuffer);
    gpu::renderPixel.setArg(13, rayPoint.y);
    gpu::renderPixel.setArg(14, rayPoint.z);
    gpu::renderPixel.setArg(7, static_cast<int>(indicesSquared.size()));
    gpu::renderPixel.setArg(8, static_cast<int>(indicesOfIndices.size()));
    gpu::renderPixel.setArg(9, static_cast<int>(allObjectsSerialized.size()));
    gpu::renderPixel.setArg(10, static_cast<int>(allIndicesSerialized.size()));
    gpu::renderPixel.setArg(11, outputBuffer2);
    gpu::renderPixel.setArg(12, rayPoint.x);
    gpu::renderPixel.setArg(15, yaw);
    gpu::renderPixel.setArg(16, pitch);
    gpu::renderPixel.setArg(17, roll);
    clSetKernelArg(gpu::renderPixel.get(), 18, sizeof(cl_mem), &gpu::image);
    gpu::renderPixel.setArg(19, SDLConfig::FOCAL_LENGTH);
    gpu::renderPixel.setArg(20, static_cast<int>(time));
    clSetKernelArg(gpu::renderPixel.get(), 21, sizeof(cl_mem), &gpu::textures);
    clSetKernelArg(gpu::renderPixel.get(), 22, sizeof(cl_sampler), &gpu::sampler);
    gpu::renderPixel.setArg(23, texturesSerializedBuffer);
    gpu::renderPixel.setArg(24, gpu::uvSerialized);
    gpu::renderPixel.setArg(25, gpu::heightsSerialized);
    gpu::renderPixel.setArg(26, gpu::widthsSerialized);
    gpu::renderPixel.setArg(27, bvhSerializedBuffer);
    gpu::renderPixel.setArg(28, bvhIndicesBuffer);
    gpu::renderPixel.setArg(29, static_cast<int>(rootSer.data.size()));
    gpu::renderPixel.setArg(30, static_cast<int>(rootSer.indices.size()));
    gpu::renderPixel.setArg(31, lightsSerializedBuffer);
    gpu::renderPixel.setArg(32, static_cast<int>(lightsSerialized.size()));
    gpu::renderPixel.setArg(33, textureIndicesBuffer);
    //gpu::renderPixel.setArg(24, static_cast<int>(text.size()));

    gpu::clearScreen.setArg(0, width);
    gpu::clearScreen.setArg(1, height);
    gpu::clearScreen.setArg(3, static_cast<int>(time));
    clSetKernelArg(gpu::clearScreen.get(), 2, sizeof(cl_mem), &gpu::image);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gpu::pbo);
    /*void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    memset(ptr, 0, width*height*4);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);*/
    clEnqueueAcquireGLObjects(gpu::queue.get(), 1, &gpu::image, 0, nullptr, nullptr);

    gpu::queue.enqueueNDRangeKernel(gpu::clearScreen, cl::NullRange, cl::NDRange(width, height));
    //gpu::queue.finish();
    //std::cout << "start" << std::endl;
    gpu::queue.enqueueNDRangeKernel(gpu::renderPixel, cl::NullRange, cl::NDRange(width, height));
    //gpu::queue.finish();
   // std::cout << "end" << std::endl;
    clEnqueueReleaseGLObjects(gpu::queue.get(), 1, &gpu::image, 0, nullptr, nullptr);

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
    //std::cout << "end2" << std::endl;
}


