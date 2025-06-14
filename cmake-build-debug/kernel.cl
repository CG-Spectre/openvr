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
     __global uchar4* image,
     const int time
){


    int x = get_global_id(0);
    int y = get_global_id(1);
    //if(x==0&&y==0){
      //  printf("hi");
    //}
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

inline float3 reflect(float3 I, float3 N){
    return I - 2.0f * dot(I, N) * N;
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

inline bool intersectAABB(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax){
    float3 t1 = (boxMin - rayOrigin) / rayDir;
    float3 t2 = (boxMax - rayOrigin) / rayDir;

    float3 tmin = fmin(t1, t2); // componentwise min
    float3 tmax = fmax(t1, t2); // componentwise max

    float tEntry = fmax(fmax(tmin.x, tmin.y), tmin.z);
    float tExit  = fmin(fmin(tmax.x, tmax.y), tmax.z);

    if (tEntry > tExit || tExit < 0.0f)
        return false;

    //if (tNear) *tNear = tEntry;
    //if (tFar)  *tFar  = tExit;
    return true;
}

inline float isShadow(__global float* bvhSerialized,
    __global int* bvhIndices,
    const int bvhSize,
    const int bvhIndicesSize,
    float3 intersection,
    float3 lightDir,
    __global int* indicesSquared,
    __global int* indicesOfIndices,
    __global float* allObjectsSerialized,
    __global int* allIndicesSerialized,
    int isSize,
    int aosSize,
    int aisSize,
    int i,
    int k,
    float3 pos,
    float3 pos2,
    float3 light,
    float3* outputColor
){
    const int maxSize = 64;
    int stack[maxSize];
    stack[0] = 0;
    int stackSize = 1;
    float shadow = 0;
    int index = 0;
    while(index < stackSize){
        int indiceIndex = stack[index];
        int b = indiceIndex;
        int end = b + 1 < bvhIndicesSize ? bvhIndices[b + 1] : bvhSize;
        float3 minBound = (float3)(0.0f, 0.0f, 0.0f);
        float3 maxBound = (float3)(0.0f, 0.0f, 0.0f);
        for (int o = bvhIndices[b]; o < bvhIndices[b] + 6; o++) {
            switch(o - bvhIndices[b]){
                case 0:
                    minBound.x = bvhSerialized[o] - pos.x;
                    break;
                case 1:
                    minBound.y = bvhSerialized[o] - pos.y;
                    break;
                case 2:
                    minBound.z = bvhSerialized[o] - pos.z;
                    break;
                case 3:
                    maxBound.x = bvhSerialized[o] - pos.x;
                    break;
                case 4:
                    maxBound.y = bvhSerialized[o] - pos.y;
                    break;
                case 5:
                    maxBound.z = bvhSerialized[o] - pos.z;
                    break;
            }
        }
        bool intersect = intersectAABB(intersection, lightDir, minBound, maxBound);
        if(intersect){
            for (int o = bvhIndices[b] + 6; o < end; o++) {
                if(bvhSerialized[o] < 0){
                    int i2 = -(int)(bvhSerialized[o]) - 1;
                    int startIndice2 = indicesOfIndices[i2];

                    int endIndice2 = aisSize;
                    int offset2 = indicesSquared[i2];
                    int endIndiceSquared2 = aosSize - offset2;
                    if(i2 < isSize - 1){
                        endIndiceSquared2 = indicesSquared[i2 + 1] - offset2;
                        endIndice2 = indicesOfIndices[i2 + 1];
                    }

                    for(int k2 = startIndice2+0; k2 < endIndice2-0; k2++){
                        if(i2 == i && k2 == k){
                            continue;
                        }
                        int startIndice22 = allIndicesSerialized[k2] + offset2;
                        int endIndice22 = endIndiceSquared2 + offset2;
                        if(k2 < endIndice2 - 1){
                            endIndice22 = allIndicesSerialized[k2+1] + offset2;
                        }

                        float3 p12 = (float3)(
                            allObjectsSerialized[startIndice22 + 0] - pos.x,
                            allObjectsSerialized[startIndice22 + 1] - pos.y,
                            allObjectsSerialized[startIndice22 + 2] - pos.z
                        );
                        float3 p22 = (float3)(
                            allObjectsSerialized[startIndice22 + 3] - pos.x,
                            allObjectsSerialized[startIndice22 + 4] - pos.y,
                            allObjectsSerialized[startIndice22 + 5] - pos.z
                        );
                        float3 p32 = (float3)(
                            allObjectsSerialized[startIndice22 + 6] - pos.x,
                            allObjectsSerialized[startIndice22 + 7] - pos.y,
                            allObjectsSerialized[startIndice22 + 8] - pos.z
                        );
                        float3 e1 = p22 - p12;
                        float3 e2 = p32 - p12;
                        float3 n2  =normalize(cross(e1, e2));
                        float denom = dot(n2, lightDir);
                        if(fabs(denom) > 0.001){
                            float t = dot(n2, p12 - intersection)/denom;
                            float dist = distance(light, intersection);
                            float3 intersection2 = lightDir*t + intersection;
                            float dist2 = distance(light, intersection2);
                            if(dist2 < dist){

                                float3 target = (float3)(0,1,0);
                                float3 axis = normalize(cross(n2, target));
                                if(axis.x == 0 && axis.y == 0 && axis.z == 0){
                                    axis = target;
                                }
                                double angle = acos(min(max(dot(n2, target), -1.0f), 1.0f));
                                bool inside = false;
                                float3 fp = rotateAroundIntersection((float3)(
                                    allObjectsSerialized[startIndice22] - pos.x,
                                    allObjectsSerialized[startIndice22 + 1] - pos.y,
                                    allObjectsSerialized[startIndice22 + 2] - pos.z
                                ), pos2, intersection2, axis, angle);
                                float3 p = fp;
                                float3 np;
                                for(int j2 = startIndice22; j2 < endIndice22; j2 += 3){
                                    //if(x == width/2 && y == height/2)
                                        //printf("%f, %f, %f | ", p.x, p.y, p.z);
                                    if(j2 + 3 < endIndice22){
                                        np = rotateAroundIntersection((float3)(
                                             allObjectsSerialized[j2 + 3] - pos.x,
                                             allObjectsSerialized[j2 + 4] - pos.y,
                                             allObjectsSerialized[j2 + 5] - pos.z
                                         ), pos2, intersection2, axis, angle);
                                    }else{
                                        np = fp;
                                    }
                                    if (fabs((np.x - p.x)*(intersection2.z - p.z) - (np.z - p.z)*(intersection2.x - p.x)) < 1e-6) {
                                        inside = true;
                                        break;
                                    }
                                    if (((np.z > intersection2.z) != (p.z > intersection2.z)) && (intersection2.x < (p.x - np.x) * ((intersection2.z - np.z) / (p.z - np.z)) + np.x)) {
                                        inside = !inside;
                                    }
                                    p = np;
                                }
                                //if(x == width/2 && y == height/2)
                                    //printf("%f, %f, %f\n", axis.x, axis.y, axis.z);
                                if(inside){
                                    shadow = max(shadow, 1.0f);
                                }

                            }
                        }
                    }
                }else{
                    if(!(shadow == 1) && stackSize < maxSize){
                        stack[stackSize] = (int)(bvhSerialized[o]);
                        stackSize++;
                        //shadow = max(shadow, isShadow((int)(bvhSerialized[o]), bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, intersection, lightDir, indicesSquared, indicesOfIndices, allObjectsSerialized, allIndicesSerialized, isSize, aosSize, aisSize, i, k, pos, pos2, light, outputColor));
                    }
                }
                if(shadow == 1){

                    break;
                }
            }
        }
        index++;
    }
    return shadow;
}

inline void rayToObject(
    __global int* indicesSquared,
    __global int* indicesOfIndices,
    __global float* allObjectsSerialized,
    __global int* allIndicesSerialized,
    const int isSize,
    const int iiSize,
    const int aosSize,
    const int aisSize,
    __global float* bvhSerialized,
    __global int* bvhIndices,
    const int bvhSize,
    const int bvhIndicesSize,
    float3 pos,
    float3 pos2,
    float3 direction,
    float3* intersectionRet,
    float4* color,
    float* dist,
    int x,
    int y,
    int width,
    int height,
    sampler_t sampler,
    read_only image2d_t textures,
    __global int* heightsSerialized,
    __global int* widthsSerialized,
    __global int* uvSerialized,
    __global int* texturesSerialized,
    __global int* textureIndices,
    int* objindex,
    int* face,
    float3* normal
){
    const int maxSize = 64;
    int stack[maxSize];
    stack[0] = 0;
    int size = 1;
    int index = 0;
    while(index < size){
        int b = index;
        int end = b + 1 < bvhIndicesSize ? bvhIndices[b + 1] : bvhSize;
        float3 minBound = (float3)(0.0f, 0.0f, 0.0f);
        float3 maxBound = (float3)(0.0f, 0.0f, 0.0f);
        for (int o = bvhIndices[b]; o < bvhIndices[b] + 6; o++) {
            switch(o - bvhIndices[b]){
                case 0:
                    minBound.x = bvhSerialized[o] - pos.x;
                    break;
                case 1:
                    minBound.y = bvhSerialized[o] - pos.y;
                    break;
                case 2:
                    minBound.z = bvhSerialized[o] - pos.z;
                    break;
                case 3:
                    maxBound.x = bvhSerialized[o] - pos.x;
                    break;
                case 4:
                    maxBound.y = bvhSerialized[o] - pos.y;
                    break;
                case 5:
                    maxBound.z = bvhSerialized[o] - pos.z;
                    break;
            }
        }
        bool intersect = intersectAABB(pos2, -direction, minBound, maxBound);


        if(intersect){
            for (int o = bvhIndices[b] + 6; o < end; o++) {
                if(bvhSerialized[o] < 0){
                    int i2 = -(int)(bvhSerialized[o]) - 1;
                    int startIndice2 = indicesOfIndices[i2];

                    int endIndice2 = aisSize;
                    int offset2 = indicesSquared[i2];
                    int endIndiceSquared2 = aosSize - offset2;
                    if(i2 < isSize - 1){
                        endIndiceSquared2 = indicesSquared[i2 + 1] - offset2;
                        endIndice2 = indicesOfIndices[i2 + 1];
                    }

                    for(int k2 = startIndice2+0; k2 < endIndice2-0; k2++){
                        int startIndice22 = allIndicesSerialized[k2] + offset2;
                        int endIndice22 = endIndiceSquared2 + offset2;
                        if(k2 < endIndice2 - 1){
                            endIndice22 = allIndicesSerialized[k2+1] + offset2;
                        }

                        float3 p12 = (float3)(
                            allObjectsSerialized[startIndice22 + 0] - pos.x,
                            allObjectsSerialized[startIndice22 + 1] - pos.y,
                            allObjectsSerialized[startIndice22 + 2] - pos.z
                        );
                        float3 p22 = (float3)(
                            allObjectsSerialized[startIndice22 + 3] - pos.x,
                            allObjectsSerialized[startIndice22 + 4] - pos.y,
                            allObjectsSerialized[startIndice22 + 5] - pos.z
                        );
                        float3 p32 = (float3)(
                            allObjectsSerialized[startIndice22 + 6] - pos.x,
                            allObjectsSerialized[startIndice22 + 7] - pos.y,
                            allObjectsSerialized[startIndice22 + 8] - pos.z
                        );
                        float3 e1 = p22 - p12;
                        float3 e2 = p32 - p12;
                        float3 n2  =normalize(cross(e1, e2));
                        float denom = dot(n2, direction);
                        if(fabs(denom) < 0.001){
                            continue;
                        }
                        float t = dot(n2, p12 - pos2)/denom;
                        if(t > 0){
                            continue;
                        }
                        float3 intersection = pos2+(direction*t);
                        float dist2 = /*distance(pos2, intersection)*/-t;
                        if(dist2 < *dist){

                            float3 target = (float3)(0,1,0);
                            float3 axis = normalize(cross(n2, target));
                            if(axis.x == 0 && axis.y == 0 && axis.z == 0){
                                axis = target;
                            }
                            double angle = acos(min(max(dot(n2, target), -1.0f), 1.0f));
                            bool inside = false;
                            float3 fp = rotateAroundIntersection((float3)(
                                allObjectsSerialized[startIndice22] - pos.x,
                                allObjectsSerialized[startIndice22 + 1] - pos.y,
                                allObjectsSerialized[startIndice22 + 2] - pos.z
                            ), pos2, intersection, axis, angle);
                            float3 p = fp;
                            float3 np;
                            float maxX = -100000.0;
                            float maxY = -100000.0;
                            float minX = 100000.0;
                            float minY = 100000.0;
                            for(int j2 = startIndice22; j2 < endIndice22; j2 += 3){
                                maxX = max(maxX, p.x);
                                maxY = max(maxY, p.z);
                                minX = min(minX, p.x);
                                minY = min(minY, p.z);
                                if(j2 + 3 < endIndice22){
                                    np = rotateAroundIntersection((float3)(
                                         allObjectsSerialized[j2 + 3] - pos.x,
                                         allObjectsSerialized[j2 + 4] - pos.y,
                                         allObjectsSerialized[j2 + 5] - pos.z
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
                                p = np;
                            }

                            if(inside){
                                //*intersected = true;
                                *dist = dist2;
                                *intersectionRet = intersection;
                                *objindex = i2;
                                *normal = n2;
                                *face = k2;
                                float4 colort;
                                int faceCount = 1;
                                int textureId = texturesSerialized[textureIndices[i2] + (k2 - startIndice2)];
                                int textureRot = texturesSerialized[faceCount*2 + 1];
                                float localX = intersection.x - minX;
                                float localY = intersection.z - minY;
                                float width = maxX - minX;
                                float height = maxY - minY;
                                if(textureId == -1){
                                    colort = (float4)(1, 0, 1, 1);
                                }else if(textureId == -2){
                                    colort = (float4)(0, 0, 0, 0);
                                }else{
                                    int coordX = ((float)localX/width)*widthsSerialized[textureId];
                                    int coordY = ((float)localY/height)*heightsSerialized[textureId];
                                    float sinRot = sin((PI/180)*(float)textureRot);
                                    float cosRot = cos((PI/180)*(float)textureRot);

                                    int coordXtmp = coordX;
                                    int coordYtmp = coordY;
                                    coordX = (sinRot * (coordY - (heightsSerialized[textureId]/2)) + cosRot * (coordX - (widthsSerialized[textureId]/2)) + widthsSerialized[textureId]/2);
                                    coordY = (sinRot * (coordXtmp - (widthsSerialized[textureId]/2)) + cosRot * (coordY - (heightsSerialized[textureId]/2)) + heightsSerialized[textureId]/2);
                                    if(sinRot < -0.001 || cosRot < -0.001){
                                        //coordX--;
                                        coordY--;
                                    }
                                    colort = read_imagef(textures, sampler, (int2)(coordX + uvSerialized[textureId] - 1, coordY));

                                }
                                *color = colort;
                            }
                        }

                    }
                }else{
                    //rayToObject((int)(bvhSerialized[o]), indicesSquared, indicesOfIndices, allObjectsSerialized, allIndicesSerialized, isSize, iiSize, aosSize, aisSize, bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, pos, pos2, direction, intersectionRet, color, dist, x, y, width, height, sampler, textures, widthsSerialized, heightsSerialized, uvSerialized, texturesSerialized, textureIndices, objindex, face);
                    if(size < maxSize){
                        stack[size] = bvhSerialized[o];
                        size++;
                    }
                }
            }
        }
        index++;
    }
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
    __global int* widthsSerialized,
    __global float* bvhSerialized,
    __global int* bvhIndices,
    const int bvhSize,
    const int bvhIndicesSize,
    __global float* lightsSerialized,

    const int lightsSize,
    __global int* textureIndices
){
    int x = get_global_id(0);
    int y = get_global_id(1);
    if((time ==1) == (x%2==0)){
        //return;
    }
    float roll2 = -roll * (PI/180.0f);
    /*float3 lights[2];
    lights[0] = (float3)(0, 3, 6);
    lights[1] = (float3)(0, 3, 0);
    float3 lightColors[2];
    int lightsSize = 2;
    lightColors[0] = (float3)(1, 1, 1);
    lightColors[1] = (float3)(1, 1, 1);
    float lightPowers[2];
    lightPowers[0] = 4;
    lightPowers[1] = 4;*/

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

    //float aspect = width/height;
    float f = 1/tan(((PI/180)*90)/2);
    //float ndc_x = (x + 0.5)/width*2 - 1;
    //float ndc_y = 1 - (y + 0.5)/height*2;
    /*float aspect = (float)width / (float)height;

    float ndc_x = ((float)x + 0.5f) / (float)width * 2.0f - 1.0f;
    float ndc_y = 1.0f - ((float)y + 0.5f) / (float)height * 2.0f;

    float px = ndc_x * aspect / f;
    float py = ndc_y / f;
    float pz = -3;*/
    float aspect = (float)width / (float)height;
    float ndc_x = ((float)x + 0.5f) / (float)width * 2.0f - 1.0f;
    float ndc_y = 1.0f - ((float)y + 0.5f) / (float)height * 2.0f;

    float px = ndc_x / f * aspect;
    float py = ndc_y / f;
    float pz = -(2020.0/(float)height);
    float cosP = cos((PI/180)*-pitch);
    float sinP = sin((PI/180)*-pitch);
    float cosY = cos((PI/180)*yaw);
    float sinY = sin((PI/180)*yaw);
    float cosR = cos(roll2);
    float sinR = sin(roll2);
    float3 dir = normalize((float3)(px, py, pz));
    dir = (float3)(
        dir.x * cosR - dir.y * sinR,
        dir.x * sinR + dir.y * cosR,
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
    //rayToObject(0);
    if(false){
    for(int i = 0; i < isSize; i++)
    {
        int startIndice = indicesOfIndices[i];

        int endIndice = aisSize;
        int offset = indicesSquared[i];
        int endIndiceSquared = aosSize - offset;
        if(i < isSize - 1){
            endIndiceSquared = indicesSquared[i + 1] - offset;
            endIndice = indicesOfIndices[i + 1];
        }

        for(int k = startIndice + 0; k < endIndice; k++){
            //each face
            //printf("%d\n", k);
            int textureId = texturesSerialized[faceCount*2];
            int textureRot = texturesSerialized[faceCount*2 + 1];


            if(x == width/2 && y == height/2){
                //printf("%f, %f, %f, %f\n", cosRot);
            }
            faceCount++;
            if(x == 0&& y==0){
                //printf("Texture: %d\n", textureRot);
            }

            int startIndice2 = allIndicesSerialized[k] + offset;
            int endIndice2 = endIndiceSquared + offset;
            if(k < endIndice - 1){
                endIndice2 = allIndicesSerialized[k+1] + offset;
            }
            if(k != 6 && x == 0 && y == 0){
                //printf("%d, %d, %d\n", k, endIndice2 - startIndice2, endIndice2);
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
                bool overallLP = 0;
                float3 overallDiffuse = (float3)(0, 0, 0);
                for(int l = 0; l < lightsSize; l++){
                    float3 light = (float3)(
                        lightsSerialized[l*7],
                        lightsSerialized[l*7 + 1],
                        lightsSerialized[l*7 + 2]
                    ) - pos;
                    float3 lightColor = (float3)(
                        lightsSerialized[l*7 + 3],
                        lightsSerialized[l*7 + 4],
                        lightsSerialized[l*7 + 5]
                    );
                    float lightPower = lightsSerialized[l*7 + 6];
                    float3 lightDir = normalize(light - intersection);
                    float dist = distance(light, intersection);
                    float kd = 1;
                    kd *= lightPower;
                    float diff = fabs(dot(n, lightDir));
                    float3 diffuse = (kd * lightColor * diff)/dist;
                    float magnitude = length(diffuse);
                    if(magnitude < 0.03){
                        continue;
                    }
                    bool shadow = false;
                    int count = 0;
                    int tmp1 = 0;
                    int tmp2 = 0;
                    float3 shadowOut;
                    shadow = isShadow(bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, intersection, lightDir, indicesSquared, indicesOfIndices, allObjectsSerialized, allIndicesSerialized, isSize, aosSize, aisSize, i, k, pos, pos2, light, &shadowOut);
                    if(shadow){
                        continue;
                    }
                    overallLP += lightPower;
                    overallDiffuse += diffuse;
                }

                float ka = 0.2;
                float ks = 1;

                float lightPower = 4;
                ks *= overallLP;
                float3 ambient = ka * (float3)(0.79f, 0.89f, 1);


                float3 reflectDir = reflect(-direction, n);
                //float spec = pow(max(dot(direction, reflectDir), 0.0f), 5.0f)/(dist*dist);
                //float3 specular = ks*lightColor*spec;
                float3 specular = (float3)(0, 0, 0);
                float localX = intersection.x - minX;
                float localY = intersection.z - minY;
                float width = maxX - minX;
                float height = maxY - minY;

                float4 colort;
                if(textureId == -1){
                    colort = (float4)(1, 0, 1, 1);
                }else if(textureId == -2){
                    colort = (float4)(0, 0, 0, 0);
                }else{
                    int coordX = ((float)localX/width)*widthsSerialized[textureId];
                    int coordY = ((float)localY/height)*heightsSerialized[textureId];
                    float sinRot = sin((PI/180)*(float)textureRot);
                    float cosRot = cos((PI/180)*(float)textureRot);

                    int coordXtmp = coordX;
                    int coordYtmp = coordY;
                    coordX = (sinRot * (coordY - (heightsSerialized[textureId]/2)) + cosRot * (coordX - (widthsSerialized[textureId]/2)) + widthsSerialized[textureId]/2);
                    coordY = (sinRot * (coordXtmp - (widthsSerialized[textureId]/2)) + cosRot * (coordY - (heightsSerialized[textureId]/2)) + heightsSerialized[textureId]/2);
                    if(sinRot < -0.001 || cosRot < -0.001){
                        //coordX--;
                        coordY--;
                    }
                    colort = read_imagef(textures, sampler, (int2)(coordX + uvSerialized[textureId] - 1, coordY));

                }
                float3 lighting = ambient + overallDiffuse + specular;
                colort.x *= (lighting.x);
                colort.y *= (lighting.y);
                colort.z *= (lighting.z);
                //intersection += ((float)((1-colort.x) - 0.5))/(150/t) * n;

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
    }
    float dist = 1000000000;
    int index;
    int face;
    float3 n = (float3)(0, 0, 0);
    inter = (float3)(0, 0, 0);
    bool intersected = false;
    rayToObject(indicesSquared, indicesOfIndices, allObjectsSerialized, allIndicesSerialized, isSize, iiSize, aosSize, aisSize, bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, pos, pos2, direction, &inter, &color, &dist, x, y, width, height, sampler, textures, widthsSerialized, heightsSerialized, uvSerialized, texturesSerialized, textureIndices, &index, &face, &n);
    bool collision = !(inter.x == 0 && inter.y == 0 && inter.z == 0);
    if(!colorDefined){


        //color = (float4)(0, 0, 0, 0);
    }
    if(!collision){
        return;
    }
    float ka = 0.2;
    float ks = 1;
    float kd = 1;
    bool overallLP = 0;
    float3 overallDiffuse = (float3)(0, 0, 0);
    for(int l = 0; l < lightsSize; l++){
        float3 light = (float3)(
            lightsSerialized[l*7],
            lightsSerialized[l*7 + 1],
            lightsSerialized[l*7 + 2]
        ) - pos;
        float3 lightColor = (float3)(
            lightsSerialized[l*7 + 3],
            lightsSerialized[l*7 + 4],
            lightsSerialized[l*7 + 5]
        );
        float lightPower = lightsSerialized[l*7 + 6];
        float3 lightDir = normalize(light - inter);
        float dist = distance(light, inter);

        kd *= lightPower;
        float diff = fabs(dot(n, lightDir));
        float3 diffuse = (kd * lightColor * diff)/dist;
        float magnitude = length(diffuse);
        if(magnitude < 0.03){
            continue;
        }
        float shadow = 0;
        int count = 0;
        int tmp1 = 0;
        int tmp2 = 0;
        float3 shadowOut;
        shadow = isShadow(bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, inter, lightDir, indicesSquared, indicesOfIndices, allObjectsSerialized, allIndicesSerialized, isSize, aosSize, aisSize, index, face, pos, pos2, light, &shadowOut);

        overallLP += (1-shadow)*lightPower;
        overallDiffuse += (1-shadow)*diffuse;
    }

    float3 ambient = ka * (float3)(0.79f, 0.89f, 1);
    float3 lighting = ambient + overallDiffuse;
    color.x *= lighting.x;
    color.y *= lighting.y;
    color.z *= lighting.z;
    output[(get_global_id(1) * width + get_global_id(0))*4 + 0] = inter.x;
    output[(get_global_id(1) * width + get_global_id(0))*4 + 1] = inter.y;
    output[(get_global_id(1) * width + get_global_id(0))*4 + 2] = inter.z;
    output[(get_global_id(1) * width + get_global_id(0))*4 + 3] = insideG;
    int vx = x;
    int vy = y;


    if(true){

        float3 point = inter;
        float x = point.x;
        float y = point.y;
        float z = point.z; //+ random()/100;
        //float distFromLight = sqrt((x - light.x)*(x - light.x) + (y - light.y)*(y - light.y) + (z - light.z)*(z - light.z));
        if(y2 == 327 && x2 == 400){
           // printf("%d, %d, %f, %f\n",x, point.x, pos.x);
        }
        //float pitch = 0;

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
            float coolness = 1.2;
            float contrast = 1.15;
            uchar r = (uchar)(clamp(color.x * pow(color.x, contrast - 1), 0.0f, 1.0f) * 255.0f/max(1.0f, 1.0f));
            uchar g = (uchar)(clamp(color.y * pow(color.y, contrast - 1), 0.0f, 1.0f) * 255.0f/max(1.0f, 1.0f));
            uchar b = (uchar)(clamp(color.z * pow(color.z, contrast - 1), 0.0f, 1.0f) * 255.0f/max(1.0f, 1.0f));
            uchar a = (uchar)(clamp(color.w, 0.0f, 1.0f) * 255.0f/max(1.0f, 1.0f));
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

