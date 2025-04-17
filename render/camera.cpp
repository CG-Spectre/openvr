//
// Created by Aidan Anderson on 4/7/25.
//

#include "camera.h"

#include <cmath>
#include <iostream>
#include <thread>
#include <CL/cl.hpp>

#include "renderableRT.h"
#include "SDLConfig.h"
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
    int threadAmt = 1;//std::thread::hardware_concurrency()/2;
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
    /*for (int i = 0; i < 10; ++i) {
        std::cout << output[i] << " ";
    }
    std::cout << std::endl;*/
    int pixelsPerThread = width/threadAmt;
    renderStack stack = this->stack;
    for (int i = 0; i < threadAmt; i++) {
        threads.push_back(std::thread([i, width, height, &stack, renderer, rayPoint, this, yaw, pitch, pixelsPerThread, trigData]() {
            for (int w = 0; w < pixelsPerThread * (1 + i); w++) {
                for (int h = 0; h < height; h++) {
                    renderNode *currentNode = stack.getFirst();
                    while (currentNode != nullptr) {
                        //currentNode->getInfo()->render(renderer);
                        /*float yawOffset = (((float)(w-width/2.0))/((float)(width/2.0))) * 45.0;
                        float pitchOffset = (((float)(h-height/2.0))/((float)(height/2.0))) * 45.0;
                        float correctedPitch = pitch + pitchOffset;
                        float correctedYaw = yaw + yawOffset;*/
                        //Vector3d rayDirection = Vector3d(cos(correctedPitch * (M_PI/180.0))*sin(correctedYaw * (M_PI/180.0)), -sin(correctedPitch * (M_PI/180.0)), cos(correctedPitch * (M_PI/180.0))*cos(correctedYaw * (M_PI/180.0)));
                        //std::cout << rayDirection.x << ":" << trigData[3*(h*width + w)] << std::endl;
                        Vector3d rayDirection = Vector3d(trigData[3*(h*width + w)], trigData[3*(h*width + w) + 1], trigData[3*(h*width + w) + 2]);
                        dynamic_cast<renderableRT *>(currentNode->getInfo())->renderRay(renderer, this, rayPoint, rayDirection);
                        currentNode = currentNode->getNext();
                    }

                }
            }
        }));
    }
    for (int i = 0; i < threads.size(); i++) {
        threads.at(i).join();
    }

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
    std::cout << "done" << std::endl;

}


