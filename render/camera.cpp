//
// Created by Aidan Anderson on 4/7/25.
//
#include <GL/glew.h>
#include "camera.h"

#include <cmath>
#include <fstream>
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
#include <sys/stat.h>

#include "json.hpp"
#include "pipe_server.h"
#include "GPU/policy_weights.h"
using json = nlohmann::json;

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
cl::Buffer shadows;
cl::Buffer irlighting;
cl::Buffer renderILResultBuffer;
cl::Buffer W0_buffer;
cl::Buffer W1_buffer;
cl::Buffer W2_buffer;
cl::Buffer b0_buffer;
cl::Buffer b1_buffer;
cl::Buffer b2_buffer;
cl::Buffer raysInBuffer;


void camera::init() {
    int width = *SDLConfig::WINDOW_WIDTH;
    int height = *SDLConfig::WINDOW_HEIGHT;
    shadows = cl::Buffer(gpu::context, CL_MEM_READ_WRITE, width * height * sizeof(float) * 3);
    irlighting = cl::Buffer(gpu::context, CL_MEM_READ_WRITE, width * height * sizeof(float) * 6);
    gpu::renderPixel.setArg(34, shadows);
    gpu::renderPixel.setArg(52, irlighting);
    W0_buffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(W0), (void*)W0);
    W1_buffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(W1), (void*)W1);
    W2_buffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(W2), (void*)W2);
    b0_buffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(b0), (void*)b0);
    b1_buffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(b1), (void*)b1);
    b2_buffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(b2), (void*)b2);
}

void camera::setReflectDir(Vector3d *direction, Vector3d *direction1, Vector3d *direction2, Vector3d *direction3) {
    this->reflectDir = direction;
    this->reflectDir1 = direction1;
    this->reflectDir2 = direction2;
    this->reflectDir3 = direction2;
    this->useReflectDir = false;
}

void camera::render(SDL_Renderer * renderer, Vector3d translationalVelocity, Vector3d rotational_velocity) {
    int width = *SDLConfig::WINDOW_WIDTH;
    int height = *SDLConfig::WINDOW_HEIGHT;

    std::vector<indirectLightingResult> indirectLightingResults;

    Vector3d rayPoint = this->pos.pose;
    float yaw = this->pos.rotation.y;
    float pitch = this->pos.rotation.x;
    float roll = this->pos.rotation.z;


    if (width != this->prevWidth || height != this->prevHeight) {
        this->prevWidth = *SDLConfig::WINDOW_WIDTH;
        this->prevHeight = *SDLConfig::WINDOW_HEIGHT;
        std::cout << "resize" << std::endl;
        gpu::resize();
        init();
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
    //std::cout << "here" << std::endl;
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
    std::vector<float> zero(1, 0.0f);
    renderILResultBuffer = cl::Buffer(gpu::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 30 * sizeof(float), zero.data());
    //std::cout << "here" << std::endl;
    if (useReflectDir) {
        //assert(reflectDir->x && reflectDir->y && reflectDir->z && reflectDir1->x&& reflectDir1->y&& reflectDir1->z&&reflectDir2->x&& reflectDir2->y&& reflectDir2->z&&reflectDir3->x&& reflectDir3->y&& reflectDir3->z);
        //std::vector<float> rays = {reflectDir->x, reflectDir->y, reflectDir->z, reflectDir1->x, reflectDir1->y, reflectDir1->z,reflectDir2->x, reflectDir2->y, reflectDir2->z,reflectDir3->x, reflectDir3->y, reflectDir3->z};
        raysInBuffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 12*sizeof(float), zero.data());
    }else {
        raysInBuffer = cl::Buffer(gpu::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 12 * sizeof(float), zero.data());
    }
    //std::cout << "here1" << std::endl;
    //std::cout << rays[0] << std::endl;

    //}
    //std::vector<float> zeros(width * height, 0.0f);
    //gpu::queue.enqueueWriteBuffer(shadows, CL_TRUE, 0, zeros.size() * sizeof(float), zeros.data());

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
    gpu::renderPixel.setArg(32, static_cast<int>(lightsSerialized.size()/7));
    gpu::renderPixel.setArg(33, textureIndicesBuffer);
    gpu::renderPixel.setArg(35, translationalVelocity.x);
    gpu::renderPixel.setArg(36, translationalVelocity.y);
    gpu::renderPixel.setArg(37, translationalVelocity.z);
    gpu::renderPixel.setArg(38, rotational_velocity.x);
    gpu::renderPixel.setArg(39, rotational_velocity.y);
    gpu::renderPixel.setArg(40, rotational_velocity.z);
    gpu::renderPixel.setArg(41, renderILResultBuffer);
    gpu::renderPixel.setArg(42, static_cast<int>(useReflectDir));
    float refX = useReflectDir ? this->reflectDir->x : 0.0f;
    float refY = useReflectDir ? this->reflectDir->y : 0.0f;
    float refZ = useReflectDir ? this->reflectDir->z : 1.0f;
    gpu::renderPixel.setArg(43, refX);
    gpu::renderPixel.setArg(44, refY);
    gpu::renderPixel.setArg(45, refZ);
    gpu::renderPixel.setArg(46, W0_buffer);
    gpu::renderPixel.setArg(47, W1_buffer);
    gpu::renderPixel.setArg(48, W2_buffer);
    gpu::renderPixel.setArg(49, b0_buffer);
    gpu::renderPixel.setArg(50, b1_buffer);
    gpu::renderPixel.setArg(51, b2_buffer);
    gpu::renderPixel.setArg(53, raysInBuffer);
    gpu::renderPixel.setArg(54, 4);


    //gpu::renderPixel.setArg(24, static_cast<int>(text.size()));

    gpu::clearScreen.setArg(0, width);
    gpu::clearScreen.setArg(1, height);
    gpu::clearScreen.setArg(3, static_cast<int>(time));

    //std::cout << "here2" << std::endl;
    gpu::queue.enqueueWriteBuffer(shadows, CL_TRUE, 0, 0, zero.data());
    gpu::queue.enqueueWriteBuffer(irlighting, CL_TRUE, 0, 0, zero.data());

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
    //gpu::queue.finish();d
   // std::cout << "end" << std::endl;
    clEnqueueReleaseGLObjects(gpu::queue.get(), 1, &gpu::image, 0, nullptr, nullptr);
    gpu::queue.finish();

    std::vector<float> results(30);
    std::vector<indirectLightingResult> ilresults;
    gpu::queue.enqueueReadBuffer(renderILResultBuffer, CL_TRUE, 0, 30 * sizeof(float), results.data());
    for (int i = 0; i < 2; i++) {
        ilresults.push_back({
            results[i*15 + 0],
            results[i*15 + 1],
            results[i*15 + 2],
            results[i*15 + 3],
            results[i*15 + 4],
            results[i*15 + 5],
            results[i*15 + 6],
            results[i*15 + 7],
            results[i*15 + 8],
            results[i*15 + 9],
            results[i*15 + 10],
            results[i*15 + 11],
            results[i*15 + 12],
            results[i*15 + 13],
            results[i*15 + 14]
        });
    }
    pipe_server::setData(ilresults[0]);
    //allILResults.insert(allILResults.end(), ilresults.begin(), ilresults.end());
    //indirectLightingResults.push_back({});
    //indirectLightingResults.push_back({});

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
    time++;
    //std::cout << "here3" << std::endl;
}

void camera::stop() {
    return;
    std::cout << "Saving " << allILResults.size() << " results." << std::endl;
    std::ofstream outFile("samples.json");
    json output;
    if (!outFile.is_open()) {
        std::cerr << "Could not open file for writing\n";
        return;
    }
    for (int i = 0; i < allILResults.size(); i++) {
        indirectLightingResult res = allILResults[i];
        json out = json{
                {"radiance", res.radiance},
                {"interX", res.interX},
                {"interY", res.interY},
                {"interZ", res.interZ},
                {"sampleX", res.sampleX},
                {"sampleY", res.sampleY},
                {"sampleZ", res.sampleZ},
                {"albedo", res.albedo},
                {"shininess", res.shininess},
                {"normalX", res.normalX},
                {"normalY", res.normalY},
                {"normalZ", res.normalZ},
                {"normalZ", res.reflectX},
                {"normalZ", res.reflectY},
                {"normalZ", res.reflectZ},
        };
        output.push_back(out);
    }
    outFile << output.dump(4);
    outFile.close();
}


void camera::render(SDL_Renderer *renderer) {
    render(renderer, Vector3d(0, 0, 0), Vector3d(0, 0, 0));
}


