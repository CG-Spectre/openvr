#define PI 3.1415926f

__kernel void precomputeRayTrig(const int width,
                         const int height, const float yaw, const float pitch,
                         __global float* output)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    float yawOffset = ((x-width/2.0)/(width/2.0))*45.0;
    float pitchOffset = ((y-height/2.0)/(height/2.0))*45.0;
    float correctedPitch = pitch + pitchOffset;
    float correctedYaw = yaw + yawOffset;
    //Vector3d rayDirection = Vector3d(cos(correctedPitch * (M_PI/180.0))*sin(correctedYaw * (M_PI/180.0)), -sin(correctedPitch * (M_PI/180.0)), cos(correctedPitch * (M_PI/180.0))*cos(correctedYaw * (M_PI/180.0)));
    if (x < width && y < height) {
        float sinYaw = sin(correctedYaw*(PI/180.0));
        float sinPitch = sin(correctedPitch*(PI/180.0));
        float cosYaw = cos(correctedYaw*(PI/180.0));
        float cosPitch = cos(correctedPitch*(PI/180.0));
        output[(y*width + x)*3] = cosPitch*sinYaw;
        output[(y*width + x)*3 + 1] = -sinPitch;
        output[(y*width + x)*3 + 2] = cosPitch*cosYaw;
    }
}