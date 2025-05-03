//
// Created by Aidan Anderson on 4/7/25.
//

#include "camera.h"

#include <cmath>
#include <iostream>
#include <thread>
#include <CL/cl.hpp>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>

#include "renderableRT.h"
#include "SDLConfig.h"
#include "3d/renderUtil.h"
#include "3d/Vertex3d.h"
#include "GPU/gpu.h"

camera::camera() {
    this->stack = renderStack();
}

camera::camera(Pose3d pos) {
    this->pos = pos;
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
    /*Vector3d rayDirection = Vector3d(cos(pitch * (M_PI/180.0))*sin(yaw * (M_PI/180.0)), -sin(pitch * (M_PI/180.0)), cos(pitch * (M_PI/180.0))*cos(yaw * (M_PI/180.0)));
    pitch += 5;
    Vector3d rayDirection1 = Vector3d(cos(pitch * (M_PI/180.0))*sin(yaw * (M_PI/180.0)), -sin(pitch * (M_PI/180.0)), cos(pitch * (M_PI/180.0))*cos(yaw * (M_PI/180.0)));
    pitch = 0;
    yaw += 5;
    Vector3d rayDirection2 = Vector3d(cos(pitch * (M_PI/180.0))*sin(yaw * (M_PI/180.0)), -sin(pitch * (M_PI/180.0)), cos(pitch * (M_PI/180.0))*cos(yaw * (M_PI/180.0)));*/

    /*std::thread t([]() {
        std::cout << "Hello" << std::endl;
    });
    std::thread t2([]() {
        std::cout << "Bye" << std::endl;
    });
    t.join();
    t2.join();*/

    std::vector<std::thread> threads;

    int width = *SDLConfig::WINDOW_WIDTH;
    int height = *SDLConfig::WINDOW_HEIGHT;
    std::vector<float> trigData(width*height*3, 0);
    cl::Buffer outputBuffer(gpu::context, CL_MEM_WRITE_ONLY, width * height*3 * sizeof(float));
    cl::CommandQueue queue(gpu::context, gpu::device);
    gpu::precomputeRayTrig.setArg(0, width);
    gpu::precomputeRayTrig.setArg(1, height);
    gpu::precomputeRayTrig.setArg(2, yaw);
    gpu::precomputeRayTrig.setArg(3, pitch);
    gpu::precomputeRayTrig.setArg(4, outputBuffer);
    queue.enqueueNDRangeKernel(gpu::precomputeRayTrig, cl::NullRange, cl::NDRange(width, height));
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, width * height * 3 * sizeof(float), trigData.data());
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
    outputBuffer = cl::Buffer(gpu::context, CL_MEM_WRITE_ONLY, width * height *4* sizeof(float));
    gpu::renderPixel.setArg(0, width);
    gpu::renderPixel.setArg(1, height);
    cl::Buffer trigBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, width * height *3 * sizeof(float), trigData.data());
    cl::Buffer indicesSquaredBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, indicesSquared.size() * sizeof(int), indicesSquared.data());
    cl::Buffer indicesOfIndicesBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, indicesOfIndices.size() * sizeof(int), indicesOfIndices.data());
    cl::Buffer allObjectsSerializedBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, allObjectsSerialized.size() * sizeof(int), allObjectsSerialized.data());
    cl::Buffer allIndicesSerializedBuffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, allIndicesSerialized.size() * sizeof(int), allIndicesSerialized.data());
    gpu::renderPixel.setArg(2, trigBuffer);
    gpu::renderPixel.setArg(3, indicesSquaredBuffer);
    gpu::renderPixel.setArg(4, indicesOfIndicesBuffer);
    gpu::renderPixel.setArg(5, allObjectsSerializedBuffer);
    gpu::renderPixel.setArg(6, allIndicesSerializedBuffer);
    gpu::renderPixel.setArg(7, static_cast<int>(indicesSquared.size()));
    gpu::renderPixel.setArg(8, static_cast<int>(indicesOfIndices.size()));
    gpu::renderPixel.setArg(9, static_cast<int>(allObjectsSerialized.size()));
    gpu::renderPixel.setArg(10, static_cast<int>(allIndicesSerialized.size()));
    gpu::renderPixel.setArg(11, outputBuffer);
    gpu::renderPixel.setArg(12, rayPoint.x);
    gpu::renderPixel.setArg(13, rayPoint.y);
    gpu::renderPixel.setArg(14, rayPoint.z);


    queue.enqueueNDRangeKernel(gpu::renderPixel, cl::NullRange, cl::NDRange(width, height));
    /*std::vector<float> data(width*height*4, 0);
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, width*height*4*sizeof(int), data.data());*/

    cl::Buffer out = cl::Buffer(gpu::context, CL_MEM_WRITE_ONLY, width * height * sizeof(uint32_t));
    gpu::mapTexture.setArg(0, yaw);
    gpu::mapTexture.setArg(1, pitch);
    gpu::mapTexture.setArg(2, roll);
    gpu::mapTexture.setArg(3, width);
    gpu::mapTexture.setArg(4, height);
    gpu::mapTexture.setArg(5, outputBuffer);
    gpu::mapTexture.setArg(6, out);
    gpu::mapTexture.setArg(7, SDLConfig::FOCAL_LENGTH);

    queue.enqueueNDRangeKernel(gpu::mapTexture, cl::NullRange, cl::NDRange(width, height));
    uint32_t* pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    queue.enqueueReadBuffer(out, CL_TRUE, 0, width*height*sizeof(uint32_t), pixels);
    std::cout << pixels[0] << std::endl;
    int threadAmt = 24;//std::thread::hardware_concurrency()/2;
    int pixelsPerThread = width/threadAmt;
    renderStack stack = this->stack;
    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
/*
    for (int i = 0; i < threadAmt; i++) {
        threads.push_back(std::thread([i, width, height, renderer, this, data, pixelsPerThread, pixels, pixel_mutex]() {
            for (int w = pixelsPerThread * i; w < pixelsPerThread * (1 + i); w++) {
                for (int h = 0; h < height; h++) {

                    if (data[4*(h*width + w) + 3] != 0) {
                        Vector3d pointD(data[4*(h*width + w) + 0], data[4*(h*width + w) + 1], data[4*(h*width + w) + 2]);
                        Vector2d point = renderUtil::get2dPoint(renderer, this, pointD);
                        if (point.x > -1 && point.y > -1 && point.x < width && point.y < height) {
                            pixels[point.y * width + point.x] = 0xFFFFFFFF;
                        }

                    }

                    //SDL_RenderPoint(renderer, pointD.x,  pointD.y);
                }
            }
        }));
    }
    for (int i = 0; i < threads.size(); i++) {
        threads.at(i).join();
    }*/
    SDL_UpdateTexture(tex, NULL, pixels, width * sizeof(uint32_t));
    SDL_RenderTexture(renderer, tex, NULL, NULL);
    /*std::cout << std::thread::hardware_concurrency() << std::endl;
    int width = *SDLConfig::WINDOW_WIDTH;
    int height = *SDLConfig::WINDOW_HEIGHT;
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            renderNode *currentNode = stack.getFirst();
            while (currentNode != nullptr) {
                //currentNode->getInfo()->render(renderer);
                float yawOffset = (((float)(w-width/2))/((float)(width/2))) * 90;
                float pitchOffset = (((float)(h-height/2))/((float)(width/2))) * 90;
                float correctedPitch = pitch + pitchOffset;
                float correctedYaw = yaw + yawOffset;
                Vector3d rayDirection = Vector3d(cos(correctedPitch * (M_PI/180.0))*sin(correctedYaw * (M_PI/180.0)), -sin(correctedPitch * (M_PI/180.0)), cos(correctedPitch * (M_PI/180.0))*cos(correctedYaw * (M_PI/180.0)));
                dynamic_cast<renderableRT *>(currentNode->getInfo())->renderRay(renderer, this, rayPoint, rayDirection);
                currentNode = currentNode->getNext();
            }

        }
    }*/

}


