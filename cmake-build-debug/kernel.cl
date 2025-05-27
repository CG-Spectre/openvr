#define PI 3.1415926f

__kernel void precomputeRayTrig(const int width,
                         const int height, const float yaw, const float pitch,
                         __global float* output)
{

    int x = get_global_id(0);
    int y = get_global_id(1);
    float aspect = width/height;
    float f = 1/tan(((PI/180)*90)/2);
    float ndc_x = (x + 0.5)/width*2 - 1;
    float ndc_y = 1 - (y + 0.5)/height*2;
    float px = ndc_x * aspect;
    float py = ndc_y;
    float pz = -3;
    float cosP = cos((PI/180)*pitch);
    float sinP = sin((PI/180)*pitch);
    float cosY = cos((PI/180)*yaw);
    float sinY = sin((PI/180)*yaw);
    float3 dir = normalize((float3)(px, py, pz));
    dir = (float3)(
                dir.x,
                dir.y,
                dir.z
        );
    dir = (float3)(
            dir.x,
            dir.y * cosP - dir.z * sinP,
            dir.y * sinP + dir.z * cosP
    );
   dir = (float3)(
            dir.x * cosY + dir.z * sinY,
            dir.y,
            -dir.x * sinY + dir.z * cosY
    );
    /*float yawOffset = ((x-width/2.0)/(width/2.0))*15.0;
    float pitchOffset = ((y-height/2.0)/(height/2.0))*20.0;
    float correctedPitch = pitch + pitchOffset;
    float correctedYaw = yaw + yawOffset;
    //Vector3d rayDirection = Vector3d(cos(correctedPitch * (PI/180.0))*sin(correctedYaw * (PI/180.0)), -sin(correctedPitch * (PI/180.0)), cos(correctedPitch * (PI/180.0))*cos(correctedYaw * (PI/180.0)));
    if (x < width && y < height) {
        float sinYaw = sin(correctedYaw*(PI/180.0));
        float sinPitch = sin(correctedPitch*(PI/180.0));
        float cosYaw = cos(correctedYaw*(PI/180.0));
        float cosPitch = cos(correctedPitch*(PI/180.0));
        output[(y*width + x)*3] = cosPitch*sinYaw;
        output[(y*width + x)*3 + 1] = -sinPitch;
        output[(y*width + x)*3 + 2] = cosPitch*cosYaw;
    }*/
    output[(y*width + x)*3] = dir.x;
    output[(y*width + x)*3 + 1] = dir.y;
    output[(y*width + x)*3 + 2] = dir.z;
}
static inline float3 rotateAroundIntersection(float3 p, float3 pos, float3 intersection, float3 axis, float angle){
    float3 v = p + pos - intersection;
    float3 term1 = v * cos(angle);
    float3 term2 = cross(axis, v) * sin(angle);
    float3 term3 = axis * dot(axis, v) * (1 - cos(angle));
    return term1 + term2 + term3 + intersection;
}
__kernel void mapTexture(
    const float yaw,
    const float pitch,
    const float roll,
    const int width,
    const int height,
    __global float* data,
    __global uchar4* image,
    const float focalLength
){
    int vx = get_global_id(0);
    int vy = get_global_id(1);
    //image[vy*width + vx] = (uchar4)(0, 0, 0, 255);
    bool render = data[4*(vy*width + vx) + 3];
    if(render){
        float3 point = (float3)(data[4*(vy*width + vx) + 0], data[4*(vy*width + vx) + 1], data[4*(vy*width + vx) + 2]);
        float x = point.x;
        float y = point.y;
        float z = point.z;
        float tmpx = x;
        x = x*cos(yaw*(PI/180)) - z*sin(yaw*(PI/180));
        z = z * cos(yaw*(PI/180)) + tmpx * sin(yaw*(PI/180));
        float tmpy = y;
        y = y * cos(pitch*(PI/180)) - z * sin(pitch*(PI/180));
        z = z * cos(pitch*(PI/180)) + tmpy * sin(pitch*(PI/180));
        tmpx = x;
        x = x * cos(roll*(PI/180)) - y * sin(roll*(PI/180));
        y = y * cos(roll*(PI/180)) + tmpx * sin(roll*(PI/180));
        if (z <= 0) {
            z = 0.00001;
        }
        float x2d = x * focalLength / z;
        float y2d = y * focalLength / z;
        //out[((int))*width + ()] = 0xFFFFFFFF;
        //write_imageui(out, (int2)((int)((((float)width)/2)+x2d), ((((float)height)/2)+y2d)), (uint4)(255, 255, 255, 255));
        int px = (int)(width / 2 + x2d);
        int py = (int)(height / 2 + y2d);
        if (px >= 0 && px < width && py >= 0 && py < height) {
            //write_imageui(out, (int2)(px, py), (uint4)(255, 255, 255, 255));
            //write_imageui(out, (int2)(px, py), (uint4)(0, 255, 255, 255));
            //printf("Writing white pixel at (%d, %d)\n", px, py);
            image[py*width + px] = (uchar4)(255, 255, 255, 255);
        }
        //image[vy*width + vx] = (uchar4)(0, 0, 255, 255);

    }
}
__kernel void fillTexture(const int width, const int height, __global uchar4* image) {
     int x = get_global_id(0);
     int y = get_global_id(1);
     if((x % 2 == 0) != (y % 2 == 0)){
            image[y*width + x] = (uchar4)(255, 255, 255, 255);
      }else{
             image[y*width + x] = (uchar4)(0, 0, 255, 255);
      }

 }
__kernel void clearScreen(
     const int width,
     const int height,
     __global uchar4* image
){
    int x = get_global_id(0);
    int y = get_global_id(1);
    image[y*width + x] = (uchar4)(0, 0, 0, 255);
}

bool intersect_triangle(float3 orig, float3 dir, float3 v0, float3 v1, float3 v2, float* t_out, float3* intersection_out) {
    const float EPSILON = 1e-6f;
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;
    float3 h = cross(dir, edge2);
    float a = dot(edge1, h);
    if (fabs(a) < EPSILON) return false; // Ray is parallel

    float f = 1.0 / a;
    float3 s = orig - v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;

    float3 q = cross(s, edge1);
    float v = f * dot(dir, q);
    if (v < 0.0 || u + v > 1.0) return false;

    float t = f * dot(edge2, q);
    if (t > EPSILON) {
        *t_out = t;
        *intersection_out = orig + dir * t;
        return true;
    }
    return false;
}

inline float random() {
    int gid = get_global_id(0) * get_global_id(1);
    int seed = 10120;
    // Simple LCG parameters
    uint a = 166452500;
    uint c = 1013904223;

    // Use seed + gid to get different sequences for each work item
    uint state = seed + gid;

    // Generate a random number
    state = a * state + c;

    // Normalize to range [0, 1)
    return (float)(state & 0x00FFFFFF) / (float)0x01000000;
}

inline float3 rotateEuler(float3 v, float pitch, float yaw, float roll) {
    float cp = cos(pitch);
    float sp = sin(pitch);
    float cy = cos(yaw);
    float sy = sin(yaw);
    float cr = cos(roll);
    float sr = sin(roll);

    // Compute rotation matrix rows (as float3 vectors)
    float3 row0 = (float3)(
        cy * cr,
        sr,
        -sy * cr
    );
    float3 row1 = (float3)(
        -cy * sr * cp + sy * sp,
        cr * cp,
        sr * sy * cp + cy * sp
    );
    float3 row2 = (float3)(
        cy * sr * sp + sy * cp,
        -cr * sp,
        -sr * sy * sp + cy * cp
    );

    return normalize((float3)(
        dot(row0, v),
        dot(row1, v),
        dot(row2, v)
    ));
}

__kernel void renderPixel(
    const int width,
    const int height,
    __global float* trigData,
    __global int* indicesSquared,
    __global int* indicesOfIndices,
    __global float* allObjectsSerialized,
    __global int* allIndicesSerialized,
    const int isSize,
    const int iiSize,
    const int aosSize,
    const int aisSize,
    __global float* output,
    const float posX,
    const float posY,
    const float posZ,
    const float yaw,
    const float pitch,
    const float roll,
    __global uchar4* image,
    const float focalLength,
    const int time,
    read_only image2d_t textures,
    sampler_t sampler,
    __global int* texturesSerialized,
    __global int* uvSerialized,
    __global int* heightsSerialized,
    __global int* widthsSerialized
){
    int x = get_global_id(0);
    int y = get_global_id(1);
    if(x == width/2 && y == height/2){
        //printf("%f\n", random());
    }
    int y2 = y;
    int x2 = x;
   /* if(x == 0 && y == 0){
        float4 color = read_imagef(textures, sampler, (int2)(254, 254));
        uchar r = (uchar)(clamp(color.x, 0.0f, 1.0f) * 255.0f);
        uchar g = (uchar)(clamp(color.y, 0.0f, 1.0f) * 255.0f);
        uchar b = (uchar)(clamp(color.z, 0.0f, 1.0f) * 255.0f);
        uchar a = (uchar)(clamp(color.w, 0.0f, 1.0f) * 255.0f);
        printf("%d %d %d %d\n", r, g, b, a);
    }*/
    if((time ==1) == (x%2==0)){
        //return;
    }
    float aspect = width/height;
    float f = 1/tan(((PI/180)*90)/2);
    float ndc_x = (x + 0.5)/width*2 - 1;
    float ndc_y = 1 - (y + 0.5)/height*2;
    float px = ndc_x * aspect;
    float py = ndc_y;
    float pz = -3;
    float cosP = cos((PI/180)*pitch);
    float sinP = sin((PI/180)*pitch);
    float cosY = cos((PI/180)*yaw);
    float sinY = sin((PI/180)*yaw);
    float3 dir = normalize((float3)(px, py, pz));
    dir = (float3)(
                dir.x,
                dir.y,
                dir.z
        );
    dir = (float3)(
            dir.x,
            dir.y * cosP - dir.z * sinP,
            dir.y * sinP + dir.z * cosP
    );
   dir = (float3)(
            dir.x * cosY + dir.z * sinY,
            dir.y,
            -dir.x * sinY + dir.z * cosY
    );
    float3 inter;
    float3 direction = normalize(dir);
    float3 pos = (float3)(
        posX,
        posY,
        posZ
    );
    float3 pos2 = (float3)(0,0,0);
    //if(x==0&&y==0){
       // printf("%f\n", pos.x);
    //}
    float resp = 0;
    bool insideG = false;
    float4 color;
    bool colorDefined = false;
    float minT = 10000;
    int usedK = -1;
    int faceCount = 0;
    for(int i = 0; i < isSize; i++)
    {
        int startIndice = indicesOfIndices[i];
        int endIndice = aisSize;
        if(i < isSize - 1){
            endIndice = indicesOfIndices[i + 1];
        }
        for(int k = startIndice + 0; k < /*endIndice*/startIndice+6; k++){
            //each face

            int textureId = texturesSerialized[faceCount];
            faceCount++;
            if(x == 0&& y==0){
                printf("Texture: %d\n", textureId);
            }

            int startIndice2 = allIndicesSerialized[k];
            int endIndice2 = aosSize;
            if(k < endIndice - 1){
                endIndice2 = allIndicesSerialized[k+1];
            }
            float3 p1 = (float3)(
                allObjectsSerialized[startIndice2 + 0] - pos.x,
                allObjectsSerialized[startIndice2 + 1] - pos.y,
                allObjectsSerialized[startIndice2 + 2] - pos.z
            );
            float3 p2 = (float3)(
                allObjectsSerialized[startIndice2 + 3] - pos.x,
                allObjectsSerialized[startIndice2 + 4] - pos.y,
                allObjectsSerialized[startIndice2 + 5] - pos.z
            );
            float3 p3 = (float3)(
                allObjectsSerialized[startIndice2 + 6] - pos.x,
                allObjectsSerialized[startIndice2 + 7] - pos.y,
                allObjectsSerialized[startIndice2 + 8] - pos.z
            );
            if(x == 0 && y == 0){
                //printf("%d\n", p2.x);
            }
            float3 v1 = p2 - p1;
            float3 v2 = p3 - p1;
            float3 n = normalize(cross(v1, v2));
            float denom = dot(n, direction);
            if(fabs(denom) < 0.001){
                continue;
            }
            float t = dot(n, p1-pos2)/denom;
            //if(t < 0){

            float3 intersection = pos2 + (direction*t);
            //float t;
            //float3 intersection;
            //intersect_triangle(pos, direction, p1, p2, p3, &t, &intersection);

            if(-t > minT || -t < 0){
                continue;
            }
            //if(x == 0 && y == height/2 - 40 && k == 2){
              //      printf("%f,%f, %d\n",-t, minT, k);
            //}
            float3 target = (float3)(0,1,0);
            float3 axis = normalize(cross(n, target));
            if(axis.x == 0 && axis.y == 0 && axis.z == 0){
                axis = target;
            }
            double angle = acos(min(max(dot(n, target), -1.0f), 1.0f));
            bool inside = false;
            int c = 0;
            float3 fp = rotateAroundIntersection((float3)(
                allObjectsSerialized[startIndice2] - pos.x,
                allObjectsSerialized[startIndice2 + 1] - pos.y,
                allObjectsSerialized[startIndice2 + 2] - pos.z
            ), pos2, intersection, axis, angle);
            float3 p = fp;
            float3 np;
            float maxX = -100000.0;
            float maxY = -100000.0;
            float minX = 100000.0;
            float minY = 100000.0;
            //if(x == 0 && y == 0){
           // printf("rendering");
            for(int j = startIndice2; j < endIndice2; j += 3){

                maxX = max(maxX, p.x);
                maxY = max(maxY, p.z);
                minX = min(minX, p.x);
                minY = min(minY, p.z);

                //maxX = p.x;
                //maxY = p.y;
                //minX = p.x;
                //minY = p.y;

                if(j + 3 < endIndice2){
                    np = rotateAroundIntersection((float3)(
                         allObjectsSerialized[j + 3] - pos.x,
                         allObjectsSerialized[j + 4] - pos.y,
                         allObjectsSerialized[j + 5] - pos.z
                     ), pos2, intersection, axis, angle);
                }else{
                    np = fp;
                }

                if (fabs((np.x - p.x)*(intersection.z - p.z) - (np.z - p.z)*(intersection.x - p.x)) < 1e-6) {
                    inside = true;
                    break;
                }
                if (((np.z > intersection.z) != (p.z > intersection.z)) && (intersection.x < (p.x - np.x) * ((intersection.z - np.z) / (p.z - np.z)) + np.x)) {
                    inside = !inside;
                }
                if(j == startIndice2 + 6){
                    resp = p.z;
                }

                p = np;
            }

            if(inside){

                // && fabs(t) > fabs(maxT)
                //maxT = t;
                float localX = intersection.x - minX;
                float localY = intersection.z - minY;
                float width = maxX - minX;
                float height = maxY - minY;
                //printf("%f, %f, %f, %f, %f, %f\n", width, height, localX, localY, 0, 0);
                if(x == width/2 && y == height/2){
                    //printf("%f, %f, %f, %f\n", localX, localY, width, height);
                }
                float4 colort;
                if(textureId == -1){
                    colort = (float4)(1, 0, 1, 1);
                }else if(textureId == -2){
                    colort = (float4)(0, 0, 0, 0);
                }else{
                    colort = read_imagef(textures, sampler, (int2)(((float)localX/width)*widthsSerialized[textureId] + uvSerialized[textureId], ((float)localY/height)*heightsSerialized[textureId]));
                }
                intersection += ((float)((1-colort.x) - 0.5))/(200/t) * n;

                if(colort.w == 0.00){
                    //color = (float4)(0, 0, 0, 0);
                }else{
                    color = colort;
                    colorDefined = true;
                    usedK = k;
                    minT = -t;
                }
                inter = (float3)(intersection.x, intersection.y, intersection.z);
            }else{
                //inter = (float3)(0, 0, 0);
            }
            insideG = insideG ? insideG : inside;
            //}
        }
    }
    if(!colorDefined){
        color = (float4)(0, 0, 0, 0);
    }
    output[(get_global_id(1) * width + get_global_id(0))*4 + 0] = inter.x;
    output[(get_global_id(1) * width + get_global_id(0))*4 + 1] = inter.y;
    output[(get_global_id(1) * width + get_global_id(0))*4 + 2] = inter.z;
    output[(get_global_id(1) * width + get_global_id(0))*4 + 3] = insideG;
    int vx = x;
    int vy = y;


    if(insideG){
        float3 point = inter;
        float x = point.x;
        float y = point.y;
        float z = point.z; //+ random()/100;
        if(y2 == 327 && x2 == 400){
           // printf("%d, %d, %f, %f\n",x, point.x, pos.x);
        }


        float tmpx = x;
        x = x*cos(yaw*(PI/180)) - z*sin(yaw*(PI/180));
        z = z * cos(yaw*(PI/180)) + tmpx * sin(yaw*(PI/180));
        float tmpy = y;
        y = y * cos(pitch*(PI/180)) - z * sin(pitch*(PI/180));
        z = z * cos(pitch*(PI/180)) + tmpy * sin(pitch*(PI/180));
        tmpx = x;
        x = x * cos(roll*(PI/180)) - y * sin(roll*(PI/180));
        y = y * cos(roll*(PI/180)) + tmpx * sin(roll*(PI/180));
        if (z <= 0) {
            z = 0.00001;
        }
        float x2d = x * focalLength / z;
        float y2d = y * focalLength / z;
        //out[((int))*width + ()] = 0xFFFFFFFF;
        //write_imageui(out, (int2)((int)((((float)width)/2)+x2d), ((((float)height)/2)+y2d)), (uint4)(255, 255, 255, 255));
        int px = (int)(width / 2 + x2d);
        int py = (int)(height / 2 + y2d);
        if (px >= 0 && px < width && py >= 0 && py < height) {
            //write_imageui(out, (int2)(px, py), (uint4)(255, 255, 255, 255));
            //write_imageui(out, (int2)(px, py), (uint4)(0, 255, 255, 255));
            //printf("Writing white pixel at (%d, %d)\n", px, py);
            //float4 color = read_imagef(textures, sampler, (int2)((float)px/width * 255, (float)py/height * 255));
            uchar r = (uchar)(clamp(color.x, 0.0f, 1.0f) * 255.0f);
            uchar g = (uchar)(clamp(color.y, 0.0f, 1.0f) * 255.0f);
            uchar b = (uchar)(clamp(color.z, 0.0f, 1.0f) * 255.0f);
            uchar a = (uchar)(clamp(color.w, 0.0f, 1.0f) * 255.0f);
            image[py*width + px] = (uchar4)(r, g, b, a);
        }
        //image[vy*width + vx] = (uchar4)(0, 0, 255, 255);

    }else{
        //image[vy*width + vx] = (uchar4)(0, 0, 0, 255);
    }
    //if(abs(x - (width/2-50)) < 3 && abs(y - (height/2)) < 3){
    //    image[y*width + x] = (uchar4)(255, 255, 255, 255);
    //}
}

