//
// Created by aidan on 4/16/2025.
//

#include "gpu.h"
#include <fstream>
#include <iostream>
cl::Device gpu::device;
cl::Context gpu::context;
cl::Program gpu::program;
cl::Kernel gpu::precomputeRayTrig;
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
    cl::Platform platform = platforms[0];
    std::cout << "OpenCL platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0
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
    gpu::program = program;
    std::cout << "Successfully initialized GPU kernel!" << std::endl;

    gpu::precomputeRayTrig = cl::Kernel(program, "precomputeRayTrig");

}
