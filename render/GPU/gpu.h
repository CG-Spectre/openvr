//
// Created by aidan on 4/16/2025.
//

#ifndef GPU_H
#define GPU_H
#include <CL/cl.hpp>


class gpu {
public:
    static cl::Context context;
    static cl::Device device;
    static cl::Program program;
    static cl::Kernel precomputeRayTrig;
    static cl::Kernel renderPixel;
    static cl::Kernel mapTexture;
    static void initialize();
};



#endif //GPU_H
