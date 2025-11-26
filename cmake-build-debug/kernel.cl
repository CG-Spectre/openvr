#define PI 3.1415926f

__kernel void precomputeRayTrig(const int width, const int height,
                                const float yaw, const float pitch,
                                __global float *output) {

  int x = get_global_id(0);
  int y = get_global_id(1);
  float aspect = width / height;
  float f = 1 / tan(((PI / 180) * 90) / 2);
  float ndc_x = (x + 0.5) / width * 2 - 1;
  float ndc_y = 1 - (y + 0.5) / height * 2;
  float px = ndc_x * aspect;
  float py = ndc_y;
  float pz = -3;
  float cosP = cos((PI / 180) * pitch);
  float sinP = sin((PI / 180) * pitch);
  float cosY = cos((PI / 180) * yaw);
  float sinY = sin((PI / 180) * yaw);
  float3 dir = normalize((float3)(px, py, pz));
  dir = (float3)(dir.x, dir.y, dir.z);
  dir =
      (float3)(dir.x, dir.y * cosP - dir.z * sinP, dir.y * sinP + dir.z * cosP);
  dir = (float3)(dir.x * cosY + dir.z * sinY, dir.y,
                 -dir.x * sinY + dir.z * cosY);
  /*float yawOffset = ((x-width/2.0)/(width/2.0))*15.0;
  float pitchOffset = ((y-height/2.0)/(height/2.0))*20.0;
  float correctedPitch = pitch + pitchOffset;
  float correctedYaw = yaw + yawOffset;
  //Vector3d rayDirection = Vector3d(cos(correctedPitch *
  (PI/180.0))*sin(correctedYaw * (PI/180.0)), -sin(correctedPitch * (PI/180.0)),
  cos(correctedPitch * (PI/180.0))*cos(correctedYaw * (PI/180.0))); if (x <
  width && y < height) { float sinYaw = sin(correctedYaw*(PI/180.0)); float
  sinPitch = sin(correctedPitch*(PI/180.0)); float cosYaw =
  cos(correctedYaw*(PI/180.0)); float cosPitch = cos(correctedPitch*(PI/180.0));
      output[(y*width + x)*3] = cosPitch*sinYaw;
      output[(y*width + x)*3 + 1] = -sinPitch;
      output[(y*width + x)*3 + 2] = cosPitch*cosYaw;
  }*/
  output[(y * width + x) * 3] = dir.x;
  output[(y * width + x) * 3 + 1] = dir.y;
  output[(y * width + x) * 3 + 2] = dir.z;
}
static inline float3 rotateAroundIntersection(float3 p, float3 pos,
                                              float3 intersection, float3 axis,
                                              float angle) {
  float3 v = p + pos - intersection;
  float3 term1 = v * cos(angle);
  float3 term2 = cross(axis, v) * sin(angle);
  float3 term3 = axis * dot(axis, v) * (1 - cos(angle));
  return term1 + term2 + term3 + intersection;
}
__kernel void mapTexture(const float yaw, const float pitch, const float roll,
                         const int width, const int height,
                         __global float *data, __global uchar4 *image,
                         const float focalLength) {
  int vx = get_global_id(0);
  int vy = get_global_id(1);
  // image[vy*width + vx] = (uchar4)(0, 0, 0, 255);
  bool render = data[4 * (vy * width + vx) + 3];
  if (render) {
    float3 point = (float3)(data[4 * (vy * width + vx) + 0],
                            data[4 * (vy * width + vx) + 1],
                            data[4 * (vy * width + vx) + 2]);
    float x = point.x;
    float y = point.y;
    float z = point.z;
    float tmpx = x;
    x = x * cos(yaw * (PI / 180)) - z * sin(yaw * (PI / 180));
    z = z * cos(yaw * (PI / 180)) + tmpx * sin(yaw * (PI / 180));
    float tmpy = y;
    y = y * cos(pitch * (PI / 180)) - z * sin(pitch * (PI / 180));
    z = z * cos(pitch * (PI / 180)) + tmpy * sin(pitch * (PI / 180));
    tmpx = x;
    x = x * cos(roll * (PI / 180)) - y * sin(roll * (PI / 180));
    y = y * cos(roll * (PI / 180)) + tmpx * sin(roll * (PI / 180));
    if (z <= 0) {
      z = 0.00001;
    }
    float x2d = x * focalLength / z;
    float y2d = y * focalLength / z;
    // out[((int))*width + ()] = 0xFFFFFFFF;
    // write_imageui(out, (int2)((int)((((float)width)/2)+x2d),
    // ((((float)height)/2)+y2d)), (uint4)(255, 255, 255, 255));
    int px = (int)(width / 2 + x2d);
    int py = (int)(height / 2 + y2d);
    if (px >= 0 && px < width && py >= 0 && py < height) {
      // write_imageui(out, (int2)(px, py), (uint4)(255, 255, 255, 255));
      // write_imageui(out, (int2)(px, py), (uint4)(0, 255, 255, 255));
      // printf("Writing white pixel at (%d, %d)\n", px, py);
      image[py * width + px] = (uchar4)(255, 255, 255, 255);
    }
    // image[vy*width + vx] = (uchar4)(0, 0, 255, 255);
  }
}
__kernel void fillTexture(const int width, const int height,
                          __global uchar4 *image) {
  int x = get_global_id(0);
  int y = get_global_id(1);
  if ((x % 2 == 0) != (y % 2 == 0)) {
    image[y * width + x] = (uchar4)(255, 255, 255, 255);
  } else {
    image[y * width + x] = (uchar4)(0, 0, 255, 255);
  }
}
__kernel void clearScreen(const int width, const int height,
                          __global uchar4 *image, const int time) {

  int x = get_global_id(0);
  int y = get_global_id(1);
  // if(x==0&&y==0){
  //   printf("hi");
  //}
  image[y * width + x] = (uchar4)(0, 0, 0, 255);
}

bool intersect_triangle(float3 orig, float3 dir, float3 v0, float3 v1,
                        float3 v2, float *t_out, float3 *intersection_out) {
  const float EPSILON = 1e-6f;
  float3 edge1 = v1 - v0;
  float3 edge2 = v2 - v0;
  float3 h = cross(dir, edge2);
  float a = dot(edge1, h);
  if (fabs(a) < EPSILON)
    return false; // Ray is parallel

  float f = 1.0 / a;
  float3 s = orig - v0;
  float u = f * dot(s, h);
  if (u < 0.0 || u > 1.0)
    return false;

  float3 q = cross(s, edge1);
  float v = f * dot(dir, q);
  if (v < 0.0 || u + v > 1.0)
    return false;

  float t = f * dot(edge2, q);
  if (t > EPSILON) {
    *t_out = t;
    *intersection_out = orig + dir * t;
    return true;
  }
  return false;
}

inline float random(int seed) {
  // Simple LCG parameters
  uint a = 166452500;
  uint c = 1013904223;
  int gid = 0; // get_global_id(0) + get_global_id(1);
  // Use seed + gid to get different sequences for each work item
  uint state = seed + gid;

  // Generate a random number
  state = a * state + c;

  // Normalize to range [0, 1)
  return (float)(state & 0x00FFFFFF) / (float)0x01000000;
}

inline float3 randomFromHemisphere(float3 R, float shininess, int seed) {
  float u1 = random(seed);
  float u2 = random(seed * 10);
  float cosTheta = pow(u1, 1.0f / (shininess + 1.0f));
  float sinTheta = sqrt(1 - cosTheta * cosTheta);
  float phi = 2.0 * PI * u2;
  float xl = sinTheta * cos(phi);
  float yl = sinTheta * sin(phi);
  float zl = cosTheta;
  float3 sampleDirLocal = (float3)(xl, yl, zl);
  float3 T;
  if (fabs(R.x) > 0.5f)
    T = normalize(cross((float3)(0.0f, 1.0f, 0.0f), R));
  else
    T = normalize(cross((float3)(1.0f, 0.0f, 0.0f), R));
  float3 B = cross(R, T);
  float3 sampleDirWorld = normalize(
      T * sampleDirLocal.x + B * sampleDirLocal.y + R * sampleDirLocal.z);
  return sampleDirWorld;
}

inline float3 staticFromHemisphere(float3 R, float xl, float yl, float zl) {
  float3 sampleDirLocal = (float3)(xl, yl, zl);
  float3 T;
  if (fabs(R.x) > 0.5f)
    T = normalize(cross((float3)(0.0f, 1.0f, 0.0f), R));
  else
    T = normalize(cross((float3)(1.0f, 0.0f, 0.0f), R));
  float3 B = cross(R, T);
  float3 sampleDirWorld = normalize(
      T * sampleDirLocal.x + B * sampleDirLocal.y + R * sampleDirLocal.z);
  return sampleDirWorld;
}

inline float3 reflect(float3 I, float3 N) { return I - 2.0f * dot(I, N) * N; }

inline float3 rotateEuler(float3 v, float pitch, float yaw, float roll) {
  float cp = cos(pitch);
  float sp = sin(pitch);
  float cy = cos(yaw);
  float sy = sin(yaw);
  float cr = cos(roll);
  float sr = sin(roll);

  // Compute rotation matrix rows (as float3 vectors)
  float3 row0 = (float3)(cy * cr, sr, -sy * cr);
  float3 row1 =
      (float3)(-cy * sr * cp + sy * sp, cr * cp, sr * sy * cp + cy * sp);
  float3 row2 =
      (float3)(cy * sr * sp + sy * cp, -cr * sp, -sr * sy * sp + cy * cp);

  return normalize((float3)(dot(row0, v), dot(row1, v), dot(row2, v)));
}

inline bool intersectAABB(float3 rayOrigin, float3 rayDir, float3 boxMin,
                          float3 boxMax) {
  float3 t1 = (boxMin - rayOrigin) / rayDir;
  float3 t2 = (boxMax - rayOrigin) / rayDir;

  float3 tmin = fmin(t1, t2); // componentwise min
  float3 tmax = fmax(t1, t2); // componentwise max

  float tEntry = fmax(fmax(tmin.x, tmin.y), tmin.z);
  float tExit = fmin(fmin(tmax.x, tmax.y), tmax.z);

  if (tEntry > tExit || tExit < 0.0f)
    return false;

  // if (tNear) *tNear = tEntry;
  // if (tFar)  *tFar  = tExit;
  return true;
}

inline float isShadow(__global float *bvhSerialized, __global int *bvhIndices,
                      const int bvhSize, const int bvhIndicesSize,
                      float3 intersection, float3 lightDir,
                      __global int *indicesSquared,
                      __global int *indicesOfIndices,
                      __global float *allObjectsSerialized,
                      __global int *allIndicesSerialized, int isSize,
                      int aosSize, int aisSize, int i, int k, float3 pos,
                      float3 pos2, float3 light, float3 *outputColor) {
  const int maxSize = 64;
  int stack[maxSize];
  stack[0] = 0;
  int stackSize = 1;
  float shadow = 0;
  int index = 0;
  while (index < stackSize) {
    int indiceIndex = stack[index];
    int b = indiceIndex;
    int end = b + 1 < bvhIndicesSize ? bvhIndices[b + 1] : bvhSize;
    float3 minBound = (float3)(0.0f, 0.0f, 0.0f);
    float3 maxBound = (float3)(0.0f, 0.0f, 0.0f);
    for (int o = bvhIndices[b]; o < bvhIndices[b] + 6; o++) {
      switch (o - bvhIndices[b]) {
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
    if (intersect) {
      for (int o = bvhIndices[b] + 6; o < end; o++) {
        if (bvhSerialized[o] < 0) {
          int i2 = -(int)(bvhSerialized[o]) - 1;
          int startIndice2 = indicesOfIndices[i2];

          int endIndice2 = aisSize;
          int offset2 = indicesSquared[i2];
          int endIndiceSquared2 = aosSize - offset2;
          if (i2 < isSize - 1) {
            endIndiceSquared2 = indicesSquared[i2 + 1] - offset2;
            endIndice2 = indicesOfIndices[i2 + 1];
          }

          for (int k2 = startIndice2 + 0; k2 < endIndice2 - 0; k2++) {
            if (i2 == i && k2 == k) {
              continue;
            }
            int startIndice22 = allIndicesSerialized[k2] + offset2;
            int endIndice22 = endIndiceSquared2 + offset2;
            if (k2 < endIndice2 - 1) {
              endIndice22 = allIndicesSerialized[k2 + 1] + offset2;
            }

            float3 p12 =
                (float3)(allObjectsSerialized[startIndice22 + 0] - pos.x,
                         allObjectsSerialized[startIndice22 + 1] - pos.y,
                         allObjectsSerialized[startIndice22 + 2] - pos.z);
            float3 p22 =
                (float3)(allObjectsSerialized[startIndice22 + 3] - pos.x,
                         allObjectsSerialized[startIndice22 + 4] - pos.y,
                         allObjectsSerialized[startIndice22 + 5] - pos.z);
            float3 p32 =
                (float3)(allObjectsSerialized[startIndice22 + 6] - pos.x,
                         allObjectsSerialized[startIndice22 + 7] - pos.y,
                         allObjectsSerialized[startIndice22 + 8] - pos.z);
            float3 e1 = p22 - p12;
            float3 e2 = p32 - p12;
            float3 n2 = normalize(cross(e1, e2));
            float denom = dot(n2, lightDir);
            if (fabs(denom) > 0.001) {
              float t = dot(n2, p12 - intersection) / denom;
              float dist = distance(light, intersection);
              float3 intersection2 = lightDir * t + intersection;
              float dist2 = distance(intersection, intersection2);
              float dist3 = distance(light, intersection2);
              if (dist2 < dist && dist3 < dist) {

                float3 target = (float3)(0, 1, 0);
                float3 axis = normalize(cross(n2, target));
                if (axis.x == 0 && axis.y == 0 && axis.z == 0) {
                  axis = target;
                }
                double angle = acos(min(max(dot(n2, target), -1.0f), 1.0f));
                bool inside = false;
                float3 fp = rotateAroundIntersection(
                    (float3)(allObjectsSerialized[startIndice22] - pos.x,
                             allObjectsSerialized[startIndice22 + 1] - pos.y,
                             allObjectsSerialized[startIndice22 + 2] - pos.z),
                    pos2, intersection2, axis, angle);
                float3 p = fp;
                float3 np;
                for (int j2 = startIndice22; j2 < endIndice22; j2 += 3) {
                  // if(x == width/2 && y == height/2)
                  // printf("%f, %f, %f | ", p.x, p.y, p.z);
                  if (j2 + 3 < endIndice22) {
                    np = rotateAroundIntersection(
                        (float3)(allObjectsSerialized[j2 + 3] - pos.x,
                                 allObjectsSerialized[j2 + 4] - pos.y,
                                 allObjectsSerialized[j2 + 5] - pos.z),
                        pos2, intersection2, axis, angle);
                  } else {
                    np = fp;
                  }
                  if (fabs((np.x - p.x) * (intersection2.z - p.z) -
                           (np.z - p.z) * (intersection2.x - p.x)) < 1e-6) {
                    inside = true;
                    break;
                  }
                  if (((np.z > intersection2.z) != (p.z > intersection2.z)) &&
                      (intersection2.x <
                       (p.x - np.x) *
                               ((intersection2.z - np.z) / (p.z - np.z)) +
                           np.x)) {
                    inside = !inside;
                  }
                  p = np;
                }
                // if(x == width/2 && y == height/2)
                // printf("%f, %f, %f\n", axis.x, axis.y, axis.z);
                if (inside) {
                  shadow = max(shadow, 1.0f);
                }
              }
            }
          }
        } else {
          if (!(shadow == 1) && stackSize < maxSize) {
            stack[stackSize] = (int)(bvhSerialized[o]);
            stackSize++;
            // shadow = max(shadow, isShadow((int)(bvhSerialized[o]),
            // bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, intersection,
            // lightDir, indicesSquared, indicesOfIndices, allObjectsSerialized,
            // allIndicesSerialized, isSize, aosSize, aisSize, i, k, pos,
            // (float3)(0, 0, 0), light, outputColor));
          }
        }
        if (shadow == 1) {

          break;
        }
      }
    }
    index++;
  }
  return shadow;
}

inline float3 TBN_mult(float3 t, float3 b, float3 n, float3 vec) {
  return vec.x * t + vec.y * b + vec.z * n;
}
inline float4 hexToColor(int hexValue) {
  // printf("%d\n", hexValue);
  float r = (float)((hexValue >> 16) & 0xFF) / 255.0f; // Red channel
  float g = (float)((hexValue >> 8) & 0xFF) / 255.0f;  // Green channel
  float b = (float)(hexValue & 0xFF) / 255.0f;         // Blue channel

  return (float4)(r, g, b, 1);
}
inline void rayToObject(
    __global int *indicesSquared, __global int *indicesOfIndices,
    __global float *allObjectsSerialized, __global int *allIndicesSerialized,
    const int isSize, const int iiSize, const int aosSize, const int aisSize,
    __global float *bvhSerialized, __global int *bvhIndices, const int bvhSize,
    const int bvhIndicesSize, float3 pos, float3 pos2, float3 direction,
    float3 *intersectionRet, float4 *color, float *dist, int x, int y,
    int width, int height, sampler_t sampler, read_only image2d_t textures,
    __global int *heightsSerialized, __global int *widthsSerialized,
    __global int *uvSerialized, __global int *texturesSerialized,
    __global int *textureIndices, int *objindex, int *face, float3 *normal) {
  const int maxSize = 64;
  int stack[maxSize];
  stack[0] = 0;
  int size = 1;
  int index = 0;
  while (index < size) {
    int b = index;
    int end = b + 1 < bvhIndicesSize ? bvhIndices[b + 1] : bvhSize;
    float3 minBound = (float3)(0.0f, 0.0f, 0.0f);
    float3 maxBound = (float3)(0.0f, 0.0f, 0.0f);
    for (int o = bvhIndices[b]; o < bvhIndices[b] + 6; o++) {
      switch (o - bvhIndices[b]) {
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

    if (intersect) {
      for (int o = bvhIndices[b] + 6; o < end; o++) {
        if (bvhSerialized[o] < 0) {
          int i2 = -(int)(bvhSerialized[o]) - 1;
          int startIndice2 = indicesOfIndices[i2];

          int endIndice2 = aisSize;
          int offset2 = indicesSquared[i2];
          int endIndiceSquared2 = aosSize - offset2;
          if (i2 < isSize - 1) {
            endIndiceSquared2 = indicesSquared[i2 + 1] - offset2;
            endIndice2 = indicesOfIndices[i2 + 1];
          }

          for (int k2 = startIndice2 + 0; k2 < endIndice2 - 0; k2++) {
            int startIndice22 = allIndicesSerialized[k2] + offset2;
            int endIndice22 = endIndiceSquared2 + offset2;
            if (k2 < endIndice2 - 1) {
              endIndice22 = allIndicesSerialized[k2 + 1] + offset2;
            }

            float3 p12 =
                (float3)(allObjectsSerialized[startIndice22 + 0] - pos.x,
                         allObjectsSerialized[startIndice22 + 1] - pos.y,
                         allObjectsSerialized[startIndice22 + 2] - pos.z);
            float3 p22 =
                (float3)(allObjectsSerialized[startIndice22 + 3] - pos.x,
                         allObjectsSerialized[startIndice22 + 4] - pos.y,
                         allObjectsSerialized[startIndice22 + 5] - pos.z);
            float3 p32 =
                (float3)(allObjectsSerialized[startIndice22 + 6] - pos.x,
                         allObjectsSerialized[startIndice22 + 7] - pos.y,
                         allObjectsSerialized[startIndice22 + 8] - pos.z);
            float3 e1 = p22 - p12;
            float3 e2 = p32 - p12;
            float3 n2 = normalize(cross(e1, e2));
            float3 n3 = n2;
            float denom = dot(n2, direction);
            if (fabs(denom) < 0.001) {
              continue;
            }
            float t = dot(n2, p12 - pos2) / denom;
            if (t > 0) {
              continue;
            }
            float3 intersection = pos2 + (direction * t);
            float dist2 = /*distance(pos2, intersection)*/ -t;
            if (dist2 < *dist) {

              float3 target = (float3)(0, 1, 0);
              float3 axis = normalize(cross(n2, target));
              if (axis.x == 0 && axis.y == 0 && axis.z == 0) {
                axis = target;
              }
              double angle = acos(min(max(dot(n2, target), -1.0f), 1.0f));
              bool inside = false;
              float3 fp = rotateAroundIntersection(
                  (float3)(allObjectsSerialized[startIndice22] - pos.x,
                           allObjectsSerialized[startIndice22 + 1] - pos.y,
                           allObjectsSerialized[startIndice22 + 2] - pos.z),
                  pos2, intersection, axis, angle);
              float3 sp = rotateAroundIntersection(
                  (float3)(allObjectsSerialized[startIndice22 + 3] - pos.x,
                           allObjectsSerialized[startIndice22 + 4] - pos.y,
                           allObjectsSerialized[startIndice22 + 5] - pos.z),
                  pos2, intersection, axis, angle);
              float3 fpBelow = (float3)(fp.x, fp.y - 1, fp.z);
              float3 n4 = normalize(cross(sp - fp, fpBelow - fp));
              float3 target2 = (float3)(0, 0, -1);
              float3 axis2 = normalize(cross(n4, target2));
              bool rotateTwice = true;
              if (axis2.x == 0 && axis2.y == 0 && axis2.z == 0) {
                // axis2 = target;
                rotateTwice = false;
              }
              double angle2 = acos(min(max(dot(n4, target2), -1.0f), 1.0f));
              if (rotateTwice) {
                fp = rotateAroundIntersection(fp, pos2, intersection, axis2,
                                              angle2);
              }

              float3 p = fp;
              float3 np;
              float maxX = -100000.0;
              float maxY = -100000.0;
              float minX = 100000.0;
              float minY = 100000.0;
              for (int j2 = startIndice22; j2 < endIndice22; j2 += 3) {
                maxX = max(maxX, p.x);
                maxY = max(maxY, p.z);
                minX = min(minX, p.x);
                minY = min(minY, p.z);
                if (j2 + 3 < endIndice22) {
                  np = rotateAroundIntersection(
                      (float3)(allObjectsSerialized[j2 + 3] - pos.x,
                               allObjectsSerialized[j2 + 4] - pos.y,
                               allObjectsSerialized[j2 + 5] - pos.z),
                      pos2, intersection, axis, angle);
                  if (rotateTwice) {
                    np = rotateAroundIntersection(np, pos2, intersection, axis2,
                                                  angle2);
                  }

                } else {
                  np = fp;
                }
                if (fabs((np.x - p.x) * (intersection.z - p.z) -
                         (np.z - p.z) * (intersection.x - p.x)) < 1e-6) {
                  inside = true;
                  break;
                }
                if (((np.z > intersection.z) != (p.z > intersection.z)) &&
                    (intersection.x <
                     (p.x - np.x) * ((intersection.z - np.z) / (p.z - np.z)) +
                         np.x)) {
                  inside = !inside;
                }
                p = np;
              }

              if (inside) {
                //*intersected = true;

                float4 colort;
                int textureId = texturesSerialized[(k2) * 6];
                int normalMapId = texturesSerialized[(k2) * 6 + 2];
                // normalMapId = 6;
                int dispMapId = texturesSerialized[(k2) * 6 + 5];
                int textureRot = texturesSerialized[(k2) * 6 + 1];
                float localX = intersection.x - minX;
                float localY = intersection.z - minY;
                float width = maxX - minX;
                float height = maxY - minY;
                int textureWidth = 0;
                int textureHeight = 0;
                int coordX = 0;
                int coordY = 0;
                if (textureId == -1) {
                  colort = (float4)(1, 0, 1, 1);
                } else if (textureId == -2) {
                  colort = (float4)(0, 0, 0, 0);
                } else if (textureId > 99999) {
                  colort = hexToColor(textureId - 100000);

                } else {
                  coordX =
                      ((float)localX / width) * widthsSerialized[textureId];
                  coordY =
                      ((float)localY / height) * heightsSerialized[textureId];
                  textureWidth = widthsSerialized[textureId];
                  textureHeight = heightsSerialized[textureId];
                  float sinRot = sin((PI / 180) * (float)textureRot);
                  float cosRot = cos((PI / 180) * (float)textureRot);

                  if (sinRot < -0.001 || cosRot < -0.001) {
                    coordX++;
                    coordY++;
                  }

                  int coordXtmp = coordX;
                  int coordYtmp = coordY;
                  coordX =
                      (sinRot * (coordY - (heightsSerialized[textureId] / 2)) +
                       cosRot * (coordX - (widthsSerialized[textureId] / 2)) +
                       widthsSerialized[textureId] / 2);
                  coordY =
                      (sinRot *
                           (coordXtmp - (widthsSerialized[textureId] / 2)) +
                       cosRot * (coordY - (heightsSerialized[textureId] / 2)) +
                       heightsSerialized[textureId] / 2);

                  colort = read_imagef(
                      textures, sampler,
                      (int2)(coordX + uvSerialized[textureId], coordY));
                }
                if (normalMapId != -1) {

                  if (!(widthsSerialized[normalMapId] == textureWidth &&
                        heightsSerialized[normalMapId] == textureHeight)) {
                    coordX =
                        ((float)localX / width) * widthsSerialized[normalMapId];
                    coordY = ((float)localY / height) *
                             heightsSerialized[normalMapId];
                    float sinRot = sin((PI / 180) * (float)textureRot);
                    float cosRot = cos((PI / 180) * (float)textureRot);

                    if (sinRot < -0.001 || cosRot < -0.001) {
                      coordX++;
                      coordY++;
                    }

                    int coordXtmp = coordX;
                    int coordYtmp = coordY;
                    coordX =
                        (sinRot *
                             (coordY - (heightsSerialized[normalMapId] / 2)) +
                         cosRot *
                             (coordX - (widthsSerialized[normalMapId] / 2)) +
                         widthsSerialized[normalMapId] / 2);
                    coordY =
                        (sinRot *
                             (coordXtmp - (widthsSerialized[normalMapId] / 2)) +
                         cosRot *
                             (coordY - (heightsSerialized[normalMapId] / 2)) +
                         heightsSerialized[normalMapId] / 2);
                  }
                  int dX = 1;
                  int dY = 1;
                  float hN =
                      read_imagef(
                          textures, sampler,
                          (int2)(coordX + uvSerialized[normalMapId], coordY))
                          .x;
                  float hL = read_imagef(
                                 textures, sampler,
                                 (int2)(coordX + uvSerialized[normalMapId] - dX,
                                        coordY))
                                 .x;
                  float hR = read_imagef(
                                 textures, sampler,
                                 (int2)(coordX + uvSerialized[normalMapId] + dX,
                                        coordY))
                                 .x;
                  float hU =
                      read_imagef(textures, sampler,
                                  (int2)(coordX + uvSerialized[normalMapId],
                                         coordY - dY))
                          .x;
                  float hD =
                      read_imagef(textures, sampler,
                                  (int2)(coordX + uvSerialized[normalMapId],
                                         coordY + dY))
                          .x;

                  // Gradient in height
                  float dX2 = hR - hL;
                  float dY2 = hU - hD;

                  // Construct perturbed normal in tangent space
                  // float3 normalTangent = normalize((float3)(-dX2,
                  // -dY2, 1.0));
                  float3 T;
                  if (n2.x < 0.99) {
                    T = normalize(cross((float3)(0, 1, 0), n2));
                  } else {
                    T = normalize(cross((float3)(0, 0, 1), n2));
                  }
                  float3 B = normalize(cross(n2, T));
                  // float3 perturbedNormal = normalize(TBN_mult(T, B, n2,
                  // normalTangent));
                  float4 sampledNormal = read_imagef(
                      textures, sampler,
                      (int2)(coordX + uvSerialized[normalMapId], coordY));
                  float3 rgbNormal = (float3)(sampledNormal.x, sampledNormal.y,
                                              sampledNormal.z) *
                                     sampledNormal.w;
                  float3 normalTangent = normalize((rgbNormal * 2.0f) - 1.0f);
                  float nmult = 1.0f; // min(5.0f/t, 1.0f);
                  float3 perturbedNormal = normalize(
                      nmult * normalTangent.x * T +
                      nmult * normalTangent.y * B + normalTangent.z * n2);
                  n2 = perturbedNormal;
                  // float deflator = 300.0f/t;
                  // intersection +=
                  // ((1-normalColor.x)*normalColor.w*n2)/deflator; colort =
                  // normalColor;
                  textureWidth = widthsSerialized[normalMapId];
                  textureHeight = heightsSerialized[normalMapId];
                }
                if (dispMapId != -1) {
                  if (!(widthsSerialized[dispMapId] == textureWidth &&
                        heightsSerialized[dispMapId] == textureHeight)) {
                    coordX =
                        ((float)localX / width) * widthsSerialized[normalMapId];
                    coordY = ((float)localY / height) *
                             heightsSerialized[normalMapId];
                    float sinRot = sin((PI / 180) * (float)textureRot);
                    float cosRot = cos((PI / 180) * (float)textureRot);

                    if (sinRot < -0.001 || cosRot < -0.001) {
                      coordX++;
                      coordY++;
                    }

                    int coordXtmp = coordX;
                    int coordYtmp = coordY;
                    coordX =
                        (sinRot *
                             (coordY - (heightsSerialized[normalMapId] / 2)) +
                         cosRot *
                             (coordX - (widthsSerialized[normalMapId] / 2)) +
                         widthsSerialized[normalMapId] / 2);
                    coordY =
                        (sinRot *
                             (coordXtmp - (widthsSerialized[normalMapId] / 2)) +
                         cosRot *
                             (coordY - (heightsSerialized[normalMapId] / 2)) +
                         heightsSerialized[normalMapId] / 2);
                  }
                  float4 sampled = read_imagef(
                      textures, sampler,
                      (int2)(coordX + uvSerialized[normalMapId], coordY));
                  // float2 sampledLocal = (float2)();
                  float actual = (sampled.x - 0.5) * sampled.w;
                  float dampener = 150.0f / t;
                  intersection += n3 * actual / dampener;
                }
                if (colort.w == 0) {
                  continue;
                }
                *dist = dist2;
                *intersectionRet = intersection;
                *objindex = i2;
                *normal = n2;
                *face = k2;
                *color = colort;
              }
            }
          }
        } else {
          // rayToObject((int)(bvhSerialized[o]), indicesSquared,
          // indicesOfIndices, allObjectsSerialized, allIndicesSerialized,
          // isSize, iiSize, aosSize, aisSize, bvhSerialized, bvhIndices,
          // bvhSize, bvhIndicesSize, pos, pos2, direction, intersectionRet,
          // color, dist, x, y, width, height, sampler, textures,
          // widthsSerialized, heightsSerialized, uvSerialized,
          // texturesSerialized, textureIndices, objindex, face);
          if (size < maxSize) {
            stack[size] = bvhSerialized[o];
            size++;
          }
        }
      }
    }
    index++;
  }
}

float tanh_approx(float x) {
  float e1 = exp(x);
  float e2 = exp(-x);
  return (e1 - e2) / (e1 + e2);
}

inline void predict_direction(float input[12],         // 12
                              __global float *W0,      // 12 x 64
                              __global float *W1,      // 64
                              __global float *W2,      // 64 x 64
                              __global float *b0,      // 64
                              __global float *b1,      // 64 x 3
                              __global float *b2,      // 3
                              float *output, bool log) // 3
{
  float h1[64];
  float h2[64];

  // FC1
  for (int i = 0; i < 64; i++) {
    h1[i] = b0[i];
    for (int j = 0; j < 12; j++) {
      // if(log && i == 1){
      // printf("BEFORE %d | %f\n", j, h1[i]);
      //}
      h1[i] += input[j] * W0[j * 64 + i];
    }
    h1[i] = fmax(h1[i], 0.0f); // ReLU
  }

  // FC2
  for (int i = 0; i < 64; i++) {
    h2[i] = b1[i];
    for (int j = 0; j < 64; j++) {
      h2[i] += h1[j] * W1[j * 64 + i];
    }
    h2[i] = fmax(h2[i], 0.0f); // ReLU
  }

  // Output layer
  for (int i = 0; i < 3; i++) {
    output[i] = b2[i];
    for (int j = 0; j < 64; j++) {
      output[i] += h2[j] * W2[j * 3 + i];
    }
  }

  // tanh squash
  for (int i = 0; i < 3; ++i) {
    output[i] = tanh_approx(output[i]);
  }
}

inline void indirectRay(
    __global int *indicesSquared, __global int *indicesOfIndices,
    __global float *allObjectsSerialized, __global int *allIndicesSerialized,
    const int isSize, const int iiSize, const int aosSize, const int aisSize,
    __global float *bvhSerialized, __global int *bvhIndices, const int bvhSize,
    const int bvhIndicesSize, float3 pos, float3 pos2, float3 direction,
    float3 *intersectionRet, float4 *color, float *dist, int x, int y,
    int width, int height, sampler_t sampler, read_only image2d_t textures,
    __global int *heightsSerialized, __global int *widthsSerialized,
    __global int *uvSerialized, __global int *texturesSerialized,
    __global int *textureIndices, int *objindex, int *face, float3 *normal,
    int f, int lightsSize, __global float *lightsSerialized, float kd,
    float4 ambient, float albedo) {

  // if(x == width/2 && y == height/2)
  // printf("%f, %f, %f | %f, %f, %f\n", direction.x, direction.y, direction.z);
  const int maxSize = 64;
  int stack[maxSize];
  stack[0] = 0;
  int size = 1;
  int index = 0;
  while (index < size) {
    int b = index;
    int end = b + 1 < bvhIndicesSize ? bvhIndices[b + 1] : bvhSize;
    float3 minBound = (float3)(0.0f, 0.0f, 0.0f);
    float3 maxBound = (float3)(0.0f, 0.0f, 0.0f);
    for (int o = bvhIndices[b]; o < bvhIndices[b] + 6; o++) {
      switch (o - bvhIndices[b]) {
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

    if (intersect) {
      for (int o = bvhIndices[b] + 6; o < end; o++) {
        if (bvhSerialized[o] < 0) {
          int i2 = -(int)(bvhSerialized[o]) - 1;
          int startIndice2 = indicesOfIndices[i2];

          int endIndice2 = aisSize;
          int offset2 = indicesSquared[i2];
          int endIndiceSquared2 = aosSize - offset2;
          if (i2 < isSize - 1) {
            endIndiceSquared2 = indicesSquared[i2 + 1] - offset2;
            endIndice2 = indicesOfIndices[i2 + 1];
          }

          for (int k2 = startIndice2 + 0; k2 < endIndice2 - 0; k2++) {
            if (k2 == f)
              continue;
            int startIndice22 = allIndicesSerialized[k2] + offset2;
            int endIndice22 = endIndiceSquared2 + offset2;
            if (k2 < endIndice2 - 1) {
              endIndice22 = allIndicesSerialized[k2 + 1] + offset2;
            }

            float3 p12 =
                (float3)(allObjectsSerialized[startIndice22 + 0] - pos.x,
                         allObjectsSerialized[startIndice22 + 1] - pos.y,
                         allObjectsSerialized[startIndice22 + 2] - pos.z);
            float3 p22 =
                (float3)(allObjectsSerialized[startIndice22 + 3] - pos.x,
                         allObjectsSerialized[startIndice22 + 4] - pos.y,
                         allObjectsSerialized[startIndice22 + 5] - pos.z);
            float3 p32 =
                (float3)(allObjectsSerialized[startIndice22 + 6] - pos.x,
                         allObjectsSerialized[startIndice22 + 7] - pos.y,
                         allObjectsSerialized[startIndice22 + 8] - pos.z);
            float3 e1 = p22 - p12;
            float3 e2 = p32 - p12;
            float3 n2 = normalize(cross(e1, e2));
            float3 n3 = n2;
            float denom = dot(n2, direction);
            if (fabs(denom) < 0.001) {
              continue;
            }
            float t = dot(n2, p12 - pos2) / denom;
            if (t > -0.01) {
              continue;
            }
            float3 intersection = pos2 + (direction * t);
            float dist2 = /*distance(pos2, intersection)*/ -t;
            if (dist2 < *dist) {
              // if(x == width/2 && y == height/2 && *dist < 10000)
              if (x == -1 && y == -1) {
                printf("asd");
              }

              float3 target = (float3)(0, 1, 0);
              float3 axis = normalize(cross(n2, target));
              if (axis.x == 0 && axis.y == 0 && axis.z == 0) {
                axis = target;
              }
              double angle = acos(min(max(dot(n2, target), -1.0f), 1.0f));
              bool inside = false;
              float3 fp = rotateAroundIntersection(
                  (float3)(allObjectsSerialized[startIndice22] - pos.x,
                           allObjectsSerialized[startIndice22 + 1] - pos.y,
                           allObjectsSerialized[startIndice22 + 2] - pos.z),
                  (float3)(0, 0, 0), intersection, axis, angle);
              float3 sp = rotateAroundIntersection(
                  (float3)(allObjectsSerialized[startIndice22 + 3] - pos.x,
                           allObjectsSerialized[startIndice22 + 4] - pos.y,
                           allObjectsSerialized[startIndice22 + 5] - pos.z),
                  (float3)(0, 0, 0), intersection, axis, angle);
              float3 fpBelow = (float3)(fp.x, fp.y - 1, fp.z);
              float3 n4 = normalize(cross(sp - fp, fpBelow - fp));
              float3 target2 = (float3)(0, 0, -1);
              float3 axis2 = normalize(cross(n4, target2));
              bool rotateTwice = true;
              if (axis2.x == 0 && axis2.y == 0 && axis2.z == 0) {
                // axis2 = target;
                rotateTwice = false;
              }
              // rotateTwice = false;
              double angle2 = acos(min(max(dot(n4, target2), -1.0f), 1.0f));
              if (rotateTwice) {
                fp = rotateAroundIntersection(fp, (float3)(0, 0, 0),
                                              intersection, axis2, angle2);
              }
              float3 p = fp;
              float3 np;
              float maxX = -100000.0;
              float maxY = -100000.0;
              float minX = 100000.0;
              float minY = 100000.0;
              for (int j2 = startIndice22; j2 < endIndice22; j2 += 3) {
                maxX = max(maxX, p.x);
                maxY = max(maxY, p.z);
                minX = min(minX, p.x);
                minY = min(minY, p.z);
                if (j2 + 3 < endIndice22) {
                  np = rotateAroundIntersection(
                      (float3)(allObjectsSerialized[j2 + 3] - pos.x,
                               allObjectsSerialized[j2 + 4] - pos.y,
                               allObjectsSerialized[j2 + 5] - pos.z),
                      (float3)(0, 0, 0), intersection, axis, angle);
                  if (rotateTwice) {
                    np = rotateAroundIntersection(np, (float3)(0, 0, 0),
                                                  intersection, axis2, angle2);
                  }

                } else {
                  np = fp;
                }
                if (fabs((np.x - p.x) * (intersection.z - p.z) -
                         (np.z - p.z) * (intersection.x - p.x)) < 1e-6) {
                  inside = true;
                  break;
                }
                if (((np.z > intersection.z) != (p.z > intersection.z)) &&
                    (intersection.x <
                     (p.x - np.x) * ((intersection.z - np.z) / (p.z - np.z)) +
                         np.x)) {
                  inside = !inside;
                }
                p = np;
              }
              // printf("%d\n", inside);
              if (inside) {
                //*intersected = true;

                float4 colort;
                int textureId = texturesSerialized[(k2) * 6];
                int normalMapId = texturesSerialized[(k2) * 6 + 2];
                // normalMapId = 6;
                int dispMapId = texturesSerialized[(k2) * 6 + 5];
                int textureRot = texturesSerialized[(k2) * 6 + 1];
                float localX = intersection.x - minX;
                float localY = intersection.z - minY;
                float width = maxX - minX;
                float height = maxY - minY;
                int textureWidth = 0;
                int textureHeight = 0;
                int coordX = 0;
                int coordY = 0;
                if (textureId == -1) {
                  colort = (float4)(1, 0, 1, 1);
                } else if (textureId == -2) {
                  colort = (float4)(0, 0, 0, 0);
                } else if (textureId > 99999) {
                  colort = hexToColor(textureId - 100000);

                } else {
                  coordX =
                      ((float)localX / width) * widthsSerialized[textureId];
                  coordY =
                      ((float)localY / height) * heightsSerialized[textureId];
                  textureWidth = widthsSerialized[textureId];
                  textureHeight = heightsSerialized[textureId];
                  float sinRot = sin((PI / 180) * (float)textureRot);
                  float cosRot = cos((PI / 180) * (float)textureRot);

                  if (sinRot < -0.001 || cosRot < -0.001) {
                    coordX++;
                    coordY++;
                  }

                  int coordXtmp = coordX;
                  int coordYtmp = coordY;
                  coordX =
                      (sinRot * (coordY - (heightsSerialized[textureId] / 2)) +
                       cosRot * (coordX - (widthsSerialized[textureId] / 2)) +
                       widthsSerialized[textureId] / 2);
                  coordY =
                      (sinRot * (coordXtmp - (widthsSerialized[textureId] / 2)) +
                       cosRot * (coordY - (heightsSerialized[textureId] / 2)) +
                       heightsSerialized[textureId] / 2);

                  colort = read_imagef(
                      textures, sampler,
                      (int2)(coordX + uvSerialized[textureId], coordY));
                }
                if (colort.w == 0) {
                  continue;
                }
                *dist = dist2;
                *intersectionRet = intersection;
                *objindex = i2;
                *normal = n2;
                *face = k2;
                *color = colort;
              }
          }
        } else {
          // rayToObject((int)(bvhSerialized[o]), indicesSquared,
          // indicesOfIndices, allObjectsSerialized, allIndicesSerialized,
          // isSize, iiSize, aosSize, aisSize, bvhSerialized, bvhIndices,
          // bvhSize, bvhIndicesSize, pos, pos2, direction, intersectionRet,
          // color, dist, x, y, width, height, sampler, textures,
          // widthsSerialized, heightsSerialized, uvSerialized,
          // texturesSerialized, textureIndices, objindex, face);
          if (size < maxSize) {
            stack[size] = bvhSerialized[o];
            size++;
          }
        }
      }
    }
    index++;
  }
  float3 totalDiffuse = (float3)(0, 0, 0);
  for (int l = 0; l < lightsSize; l++) {
    float3 light =
        (float3)(lightsSerialized[l * 7], lightsSerialized[l * 7 + 1],
                 lightsSerialized[l * 7 + 2]) -
        pos;
    float3 lightDir = normalize(light + (float3)(0, 0, 0) - *intersectionRet);
    float3 lightColor =
        (float3)(lightsSerialized[l * 7 + 3], lightsSerialized[l * 7 + 4],
                 lightsSerialized[l * 7 + 5]);
    float lightPower = lightsSerialized[l * 7 + 6];
    // float offset = 0.05;

    float dist = distance(light, *intersectionRet);

    kd *= lightPower;
    //
    float diff = fabs(dot(*normal, lightDir));
    // if(x == width/2 && y == height/2)
    // printf("%f\n", diff);
    float3 diffuse = (kd * lightColor * diff) / dist;
    float magnitude = length(diffuse);
    if (magnitude < 0.03) {
      diffuse *= 0;
    }

    float shadow = 0;
    float3 shadowOut;
    shadow += isShadow(
        bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, *intersectionRet,
        lightDir, indicesSquared, indicesOfIndices, allObjectsSerialized,
        allIndicesSerialized, isSize, aosSize, aisSize, *objindex, *face, pos,
        (float3)(0, 0, 0), light, &shadowOut);
    totalDiffuse += diffuse * (1 - shadow);
  }
  float4 lighting =
      (float4)(totalDiffuse.x, totalDiffuse.y, totalDiffuse.z, 1) + ambient;
  (*color).x *= lighting.x;
  (*color).y *= lighting.y;
  (*color).z *= lighting.z;
  //(*color) /= max(*dist, 1.0f);
  *color *= albedo;
  // isShadow(bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, inter,
  // lightDir1, indicesSquared, indicesOfIndices, allObjectsSerialized,
  // allIndicesSerialized, isSize, aosSize, aisSize, index, face, pos, pos2,
  // light1, &shadowOut);
}

}

inline float3
calcIL(__global int *indicesSquared, __global int *indicesOfIndices,
       __global float *allObjectsSerialized, __global int *allIndicesSerialized,
       const int isSize, const int iiSize, const int aosSize, const int aisSize,
       __global float *bvhSerialized, __global int *bvhIndices,
       const int bvhSize, const int bvhIndicesSize, float3 pos, float3 pos2,
       float3 direction, float3 *intersectionRet, float4 *color, float *dist,
       int x, int y, int width, int height, sampler_t sampler,
       read_only image2d_t textures, __global int *heightsSerialized,
       __global int *widthsSerialized, __global int *uvSerialized,
       __global int *texturesSerialized, __global int *textureIndices,
       int *objindex, int *face, float3 *normal, int f, int lightsSize,
       __global float *lightsSerialized, float kd, float4 ambient, float albedo,
       float3 hemisphere, float3 *sampleDirsWorld, int rays, float shininess) {
  float3 totalIndirectColor = (float3)(0, 0, 0);
  for (int r = 0; r < rays; r++) {
    float4 indirectColorR = (float4)(0, 0, 0, 0);
    float indirectDistR = 1000000000;
    indirectRay(indicesSquared, indicesOfIndices, allObjectsSerialized,
                allIndicesSerialized, isSize, iiSize, aosSize, aisSize,
                bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, pos, pos2,
                sampleDirsWorld[r], intersectionRet, &indirectColorR,
                &indirectDistR, x, y, width, height, sampler, textures,
                widthsSerialized, heightsSerialized, uvSerialized,
                texturesSerialized, textureIndices, objindex, face, normal, f,
                lightsSize, lightsSerialized, kd, ambient, albedo);
    if (isnan(indirectColorR.x)) {
      indirectColorR = (float4)(0, 0, 0, 0);
    }
    indirectColorR = indirectColorR * dot(hemisphere, sampleDirsWorld[r]);
    float3 indirectColor3R =
        (float3)(indirectColorR.x, indirectColorR.y, indirectColorR.z);

    float3 indirectDir = sampleDirsWorld[r];
    float3 reflectionDir = reflect(-indirectDir, hemisphere);
    float specAngle = max(dot(reflectionDir, -direction), 0.0f);
    float phongBRDF =
        (shininess + 2.0f) / (2.0f * PI) * pow(specAngle, shininess);
    float3 brdf = indirectColor3R * phongBRDF;
    float falloff =
        1.0f / max((4.0f * PI * indirectDistR * indirectDistR), 1.0f);
    totalIndirectColor += indirectColor3R * falloff * brdf;
  }
  return totalIndirectColor;
}

__kernel void renderPixel(
    const int width, const int height, __global float *trigData,
    __global int *indicesSquared, __global int *indicesOfIndices,
    __global float *allObjectsSerialized, __global int *allIndicesSerialized,
    const int isSize, const int iiSize, const int aosSize, const int aisSize,
    __global float *output, const float posX, const float posY,
    const float posZ, const float yaw, const float pitch, const float roll,
    __global uchar4 *image, const float focalLength, const int time,
    read_only image2d_t textures, sampler_t sampler,
    __global int *texturesSerialized, __global int *uvSerialized,
    __global int *heightsSerialized, __global int *widthsSerialized,
    __global float *bvhSerialized, __global int *bvhIndices, const int bvhSize,
    const int bvhIndicesSize, __global float *lightsSerialized,

    const int lightsSize, __global int *textureIndices,
    __global float *allShadows, float tranVelX, float tranVelY, float tranVelZ,
    float rotVelX, float rotVelY, float rotVelZ, __global float *irResults,
    int useReflectDir, float reflectX, float reflectY, float reflectZ,
    __global float *W0, __global float *W1, __global float *W2,
    __global float *b0, __global float *b1, __global float *b2,
    __global float *irLighting, __global float *raysIn, const int raysInAmt) {
  // asdasd

  int x = get_global_id(0);
  int y = get_global_id(1);
  float3 translationalVelocity = (float3)(tranVelX, tranVelY, tranVelZ);
  float3 rotationalVelocity = (float3)(rotVelX, rotVelY, rotVelZ);
  if ((time == 1) == (x % 2 == 0)) {
    // return;
  }
  float roll2 = -roll * (PI / 180.0f);
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

  if (x == width / 2 && y == height / 2) {
    // printf("%f\n", random());
  }
  int y2 = y;
  int x2 = x;

  // float aspect = width/height;
  float f = 1 / tan(((PI / 180) * 90) / 2);

  float aspect = (float)width / (float)height;
  float ndc_x = ((float)x + 0.5f) / (float)width * 2.0f - 1.0f;
  float ndc_y = 1.0f - ((float)y + 0.5f) / (float)height * 2.0f;

  float px = ndc_x / f * aspect;
  float py = ndc_y / f;
  float pz = -(2020.0 / (float)height);
  float cosP = cos((PI / 180) * -pitch);
  float sinP = sin((PI / 180) * -pitch);
  float cosY = cos((PI / 180) * yaw);
  float sinY = sin((PI / 180) * yaw);
  float cosR = cos(roll2);
  float sinR = sin(roll2);
  float3 dir = normalize((float3)(px, py, pz));
  dir =
      (float3)(dir.x * cosR - dir.y * sinR, dir.x * sinR + dir.y * cosR, dir.z);
  dir =
      (float3)(dir.x, dir.y * cosP - dir.z * sinP, dir.y * sinP + dir.z * cosP);
  dir = (float3)(dir.x * cosY + dir.z * sinY, dir.y,
                 -dir.x * sinY + dir.z * cosY);
  float3 inter;
  float3 direction = normalize(dir);
  float3 pos = (float3)(posX, posY, posZ);
  float3 pos2 = (float3)(0, 0, 0);
  float resp = 0;
  bool insideG = false;
  float4 color;
  bool colorDefined = false;
  float minT = 10000;
  int usedK = -1;
  int faceCount = 0;
  // rayToObject(0);
  float dist = 1000000000;
  int index;
  int face;
  float3 n = (float3)(0, 0, 0);
  inter = (float3)(0, 0, 0);
  bool intersected = false;
  rayToObject(indicesSquared, indicesOfIndices, allObjectsSerialized,
              allIndicesSerialized, isSize, iiSize, aosSize, aisSize,
              bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, pos, pos2,
              direction, &inter, &color, &dist, x, y, width, height, sampler,
              textures, widthsSerialized, heightsSerialized, uvSerialized,
              texturesSerialized, textureIndices, &index, &face, &n);
  bool collision = !(inter.x == 0 && inter.y == 0 && inter.z == 0);
  if (!colorDefined) {

    // color = (float4)(0, 0, 0, 0);
  }
  if (!collision) {
    return;
  }
  float3 indirectNormal = (float3)(0, 0, 0);
  float4 indirectColor = (float4)(0, 0, 0, 0);
  float4 indirectColor1 = (float4)(0, 0, 0, 0);
  float4 indirectColor2 = (float4)(0, 0, 0, 0);
  float3 indirectInter;
  int indirectIndex;
  int indirectFace;
  float indirectDist = 1000000000;
  float ka = 0.2;
  float ks = 1;
  float kd = 1.0f;
  float3 ambient = ka * (float3)(0.79f, 0.89f, 1);
  float3 R = reflect(direction, n);
  float shininess = 0.5f;
  float albedo = 0.3f;
  const int rays = 64;
  float3 sampleDirsWorld[rays];
  float3 totalIndirectColor2 = (float3)(0, 0, 0);
  float3 sampleDirWorld;
  float3 sampleDirWorld2;
  float3 hemisphere = n;
  if (dot(hemisphere, R) < 0) {
    hemisphere = -hemisphere;
  }
  float3 raysInArr[4];
  if (useReflectDir) {
    // sampleDirWorld = staticFromHemisphere(hemisphere, reflectX, reflectY,
    // reflectZ); sampleDirWorld2 = sampleDirWorld;

    float4 indirectColorRIn = (float4)(0, 0, 0, 0);
    float indirectDistRIn = 1000000000;
    for (int r = 0; r < raysInAmt; r++) {
      float3 prevec =
          (float3)(raysIn[r * 3 + 0], raysIn[r * 3 + 1], raysIn[r * 3 + 2]);
      float3 vec;
      if (length(prevec) > 0) {
        vec = normalize(prevec);
      } else {
        vec = (float3)(0, 0, 1);
      }
      raysInArr[r] = staticFromHemisphere(hemisphere, vec.x, vec.y, vec.z);
    }
    totalIndirectColor2 = calcIL(
        indicesSquared, indicesOfIndices, allObjectsSerialized,
        allIndicesSerialized, isSize, iiSize, aosSize, aisSize, bvhSerialized,
        bvhIndices, bvhSize, bvhIndicesSize, pos, inter, direction,
        &indirectInter, &indirectColorRIn, &indirectDistRIn, x, y, width,
        height, sampler, textures, widthsSerialized, heightsSerialized,
        uvSerialized, texturesSerialized, textureIndices, &indirectIndex,
        &indirectFace, &indirectNormal, face, lightsSize, lightsSerialized, kd,
        (float4)(ambient.x, ambient.y, ambient.z, 1), albedo, hemisphere,
        &raysInArr, raysInAmt, shininess);
  }
  for (int r = 0; r < rays; r++) {
    sampleDirsWorld[r] = randomFromHemisphere(R, shininess, time / (r + 1));
  }
  float4 indirectColorR = (float4)(0, 0, 0, 0);
  float indirectDistR = 1000000000;
  float3 totalIndirectColor = calcIL(
      indicesSquared, indicesOfIndices, allObjectsSerialized,
      allIndicesSerialized, isSize, iiSize, aosSize, aisSize, bvhSerialized,
      bvhIndices, bvhSize, bvhIndicesSize, pos, inter, direction,
      &indirectInter, &indirectColorR, &indirectDistR, x, y, width, height,
      sampler, textures, widthsSerialized, heightsSerialized, uvSerialized,
      texturesSerialized, textureIndices, &indirectIndex, &indirectFace,
      &indirectNormal, face, lightsSize, lightsSerialized, kd,
      (float4)(ambient.x, ambient.y, ambient.z, 1), albedo, hemisphere,
      &sampleDirsWorld, rays, shininess);
  if (time > 1) {
    int blurOffsetIR = 2;
    int modifiedBlurOffsetIR = min((((float)blurOffsetIR) / (dist / 10)), 7.0f);
    int directionsIR = 4;
    int validDirectionsIR = 0;
    if (x > modifiedBlurOffsetIR && y > modifiedBlurOffsetIR &&
        y < height - modifiedBlurOffsetIR && x < width - modifiedBlurOffsetIR &&
        modifiedBlurOffsetIR != 0) {
      int tx;
      int ty;
      float3 tmpIR = totalIndirectColor;
      for (int i = 0; i < directionsIR; i++) {
        float3 variableIR;
        switch (i % 4) {
        case 0:
          tx = x + modifiedBlurOffsetIR;
          ty = y;
          variableIR =
              (float3)(irLighting[((int)(width * (ty) + (tx)) * 6 + 0)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 1)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 2)]);
          break;
        case 1:
          tx = x - modifiedBlurOffsetIR;
          ty = y;
          variableIR =
              (float3)(irLighting[((int)(width * (ty) + (tx)) * 6 + 0)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 1)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 2)]);
          break;
        case 2:
          tx = x;
          ty = y + modifiedBlurOffsetIR;
          variableIR =
              (float3)(irLighting[((int)(width * (ty) + (tx)) * 6 + 0)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 1)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 2)]);
          break;
        case 3:
          tx = x;
          ty = y - modifiedBlurOffsetIR;
          variableIR =
              (float3)(irLighting[((int)(width * (ty) + (tx)) * 6 + 0)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 1)],
                       irLighting[((int)(width * (ty) + (tx)) * 6 + 2)]);
          break;
        }
        if (dot(normalize(variableIR), normalize(tmpIR)) > 0.5 &&
            dot(n, (float3)(irLighting[((int)(width * (ty) + (tx)) * 6 + 3)],
                            irLighting[((int)(width * (ty) + (tx)) * 6 + 4)],
                            irLighting[((int)(width * (ty) + (tx)) * 6 + 5)])) >
                0.9) {
          totalIndirectColor += variableIR;
          validDirectionsIR++;
        }
        // if(x == width/2 && y == height/2){
        //  printf("%f\n", irLighting[((int)(width*(ty) + (tx))*6 + 0)]);
        //}
      }

      totalIndirectColor /= (validDirectionsIR + 1);
    }
  }
  /*if(x == width/2 && y == height/2 && false){
      printf("%f, %f, %f, %f, %f, %f\n",
      irLighting[6*(y*width + x) + 0],
      irLighting[6*(y*width + x) + 1],
      irLighting[6*(y*width + x) + 2],
      irLighting[6*(y*width + x) + 3],
      irLighting[6*(y*width + x) + 4],
      irLighting[6*(y*width + x) + 5]
      );
  }*/
  irLighting[6 * (y * width + x) + 0] = totalIndirectColor.x;
  irLighting[6 * (y * width + x) + 1] = totalIndirectColor.y;
  irLighting[6 * (y * width + x) + 2] = totalIndirectColor.z;
  irLighting[6 * (y * width + x) + 3] = n.x;
  irLighting[6 * (y * width + x) + 4] = n.y;
  irLighting[6 * (y * width + x) + 5] = n.z;
  // indirectRay(indicesSquared, indicesOfIndices, allObjectsSerialized,
  // allIndicesSerialized, isSize, iiSize, aosSize, aisSize, bvhSerialized,
  // bvhIndices, bvhSize, bvhIndicesSize, pos, inter, sampleDirWorld,
  // &indirectInter, &indirectColor1, &indirectDist, x, y, width, height,
  // sampler, textures, widthsSerialized, heightsSerialized, uvSerialized,
  // texturesSerialized, textureIndices, &indirectIndex, &indirectFace,
  // &indirectNormal, face, lightsSize, lightsSerialized, kd,
  // (float4)(ambient.x, ambient.y, ambient.z, 1), albedo);
  // indirectRay(indicesSquared, indicesOfIndices, allObjectsSerialized,
  // allIndicesSerialized, isSize, iiSize, aosSize, aisSize, bvhSerialized,
  // bvhIndices, bvhSize, bvhIndicesSize, pos, inter, sampleDirWorld2,
  // &indirectInter, &indirectColor2, &indirectDist, x, y, width, height,
  // sampler, textures, widthsSerialized, heightsSerialized, uvSerialized,
  // texturesSerialized, textureIndices, &indirectIndex, &indirectFace,
  // &indirectNormal, face, lightsSize, lightsSerialized, kd,
  // (float4)(ambient.x, ambient.y, ambient.z, 1), albedo);

  // if(isnan(indirectColor.x)){
  //     indirectColor = (float4)(0, 0, 0, 0);
  // }

  // float3 indirectColor3 = (float3)(indirectColor.x, indirectColor.y,
  // indirectColor.z);

  if (x == width / 2 && y == height / 2) {
    // printf("%f\n", length(totalIndirectColor2));
    // if(length(indirectColor3) > 0.5)
    // printf("%f %f %f | %f %f %f | %f %f %f | %f %f %f | %f\n",
    // sampleDirWorld.x, sampleDirWorld.y, sampleDirWorld.z, hemisphere.x,
    // hemisphere.y, hemisphere.z, inter.x + pos.x, inter.y + pos.y, inter.z +
    // pos.z, n.x, n.y, n.z, length(indirectColor3));
    irResults[0] =
        fabs(length(totalIndirectColor) - length(totalIndirectColor2));
    irResults[1] = inter.x + pos.x;
    irResults[2] = inter.y + pos.y;
    irResults[3] = inter.z + pos.z;
    irResults[4] = randomFromHemisphere(R, shininess, time).x;
    irResults[5] = sampleDirWorld.y;
    irResults[6] = sampleDirWorld.z;
    irResults[7] = 1.0f;
    irResults[8] = shininess;
    irResults[9] = hemisphere.x;
    irResults[10] = hemisphere.y;
    irResults[11] = hemisphere.z;
    irResults[12] = R.x;
    irResults[13] = R.y;
    irResults[14] = R.z;
    irResults[15] =
        isnan(length(indirectColor2)) ? 0.0f : length(indirectColor2);
    irResults[16] = inter.x;
    irResults[17] = inter.y;
    irResults[18] = inter.z;
    irResults[19] = sampleDirWorld2.x;
    irResults[20] = sampleDirWorld2.y;
    irResults[21] = sampleDirWorld2.z;
    irResults[22] = 1.0f;
    irResults[23] = shininess;
    irResults[24] = n.x;
    irResults[25] = n.y;
    irResults[26] = n.z;
    irResults[27] = direction.x;
    irResults[28] = direction.y;
    irResults[29] = direction.z;
  }

  bool overallLP = 0;
  float3 overallDiffuse = (float3)(0, 0, 0);
  for (int l = 0; l < lightsSize; l++) {
    float3 light =
        (float3)(lightsSerialized[l * 7], lightsSerialized[l * 7 + 1],
                 lightsSerialized[l * 7 + 2]) -
        pos;
    float3 lightDir = normalize(light + (float3)(0, 0, 0) - inter);
    float radius = 200;
    float theta = random(time) * PI * 2;
    float u = random(time);
    float r = radius * sqrt(u);
    float2 offsetVector = (float2)(r * cos(theta), r * sin(theta));
    float3 tmp = fabs(n.z) < 0.99 ? (float3)(0, 0, 1) : (float3)(1, 0, 0);
    float3 u2 = normalize(cross(lightDir, tmp));
    float3 v = normalize(cross(lightDir, u2));
    float3 offsetVector3d = offsetVector.x * u2 + offsetVector.y * v;
    float temp = offsetVector3d.y;
    offsetVector3d.y = offsetVector3d.z;
    offsetVector3d.z = temp;
    float3 offsetVector3d2 = -offsetVector3d;
    // if(width/2 == x && height/2 == y){
    // printf("%f, %f, %f\n", theta);
    //}

    float3 light1 = (float3)(lightsSerialized[l * 7] + offsetVector3d.x,
                             lightsSerialized[l * 7 + 1],
                             lightsSerialized[l * 7 + 2] + offsetVector3d.y) -
                    pos;
    float3 light2 = (float3)(lightsSerialized[l * 7] + offsetVector3d2.x,
                             lightsSerialized[l * 7 + 1],
                             lightsSerialized[l * 7 + 2] + offsetVector3d2.y) -
                    pos;
    float3 lightColor =
        (float3)(lightsSerialized[l * 7 + 3], lightsSerialized[l * 7 + 4],
                 lightsSerialized[l * 7 + 5]);
    float lightPower = lightsSerialized[l * 7 + 6];
    // float offset = 0.05;

    float3 lightDir1 = normalize(light1 + (float3)(0, 0, 0) - inter);
    float3 lightDir2 = normalize(light2 + (float3)(0, 0, 0) - inter);
    float dist = distance(light, inter);

    kd *= lightPower;
    float diff = fabs(dot(n, lightDir));
    // diff = 0.5;
    // if(x == width/2 && y == height/2)
    // printf("%f\n", diff);
    float3 diffuse = (kd * lightColor * diff) / dist;
    float magnitude = length(diffuse);
    if (magnitude < 0.03) {
      continue;
    }
    float shadow = 0;
    int count = 0;
    int tmp1 = 0;
    int tmp2 = 0;
    float3 shadowOut;

    shadow +=
        isShadow(bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, inter,
                 lightDir1, indicesSquared, indicesOfIndices,
                 allObjectsSerialized, allIndicesSerialized, isSize, aosSize,
                 aisSize, index, face, pos, pos2, light1, &shadowOut);
    shadow +=
        isShadow(bvhSerialized, bvhIndices, bvhSize, bvhIndicesSize, inter,
                 lightDir2, indicesSquared, indicesOfIndices,
                 allObjectsSerialized, allIndicesSerialized, isSize, aosSize,
                 aisSize, index, face, pos, pos2, light2, &shadowOut);

    shadow /= 2;
    // if(time > 2){
    //     shadow += allShadows[width*y + x];
    //    shadow /= 2;
    //}

    overallLP += (1 - shadow) * lightPower;
    overallDiffuse += (1 - shadow) * diffuse;
  }
  bool firstTime = time == 0;
  float quality = 100.0f;
  bool notMoving =
      length(translationalVelocity) < 0.1 && length(rotationalVelocity) < 5;
  if (!firstTime && notMoving && !useReflectDir) {
    if (!notMoving) {
      quality = 5.0f;
    }
    float3 prevDiffuse = (float3)(allShadows[(width * y + x) * 3 + 0],
                                  allShadows[(width * y + x) * 3 + 1],
                                  allShadows[(width * y + x) * 3 + 2]);
    overallDiffuse += prevDiffuse - (prevDiffuse / quality);
    allShadows[(width * y + x) * 3 + 0] = overallDiffuse.x;
    allShadows[(width * y + x) * 3 + 1] = overallDiffuse.y;
    allShadows[(width * y + x) * 3 + 2] = overallDiffuse.z;
    overallDiffuse /= quality;
  } else {
    allShadows[(width * y + x) * 3 + 0] = overallDiffuse.x * quality;
    allShadows[(width * y + x) * 3 + 1] = overallDiffuse.y * quality;
    allShadows[(width * y + x) * 3 + 2] = overallDiffuse.z * quality;
  }
  int blurOffset = 5;
  int modifiedBlurOffset = min((((float)blurOffset) / (dist / 10)), 7.0f);
  int directions = 4;
  int validDirections = 0;
  if (x == width / 2 && y == height / 2) {
    // printf("%d\n", modifiedBlurOffset);
  }
  if (x > modifiedBlurOffset && y > modifiedBlurOffset &&
      y < height - modifiedBlurOffset && x < width - modifiedBlurOffset &&
      modifiedBlurOffset != 0) {
    int tx;
    int ty;
    float3 tmpDiffuse = overallDiffuse;
    for (int i = 0; i < directions; i++) {
      float3 variableDiffuse;
      switch (i % 4) {
      case 0:
        tx = x + modifiedBlurOffset;
        ty = y;
        variableDiffuse =
            (float3)(allShadows[((int)(width * (ty) + (tx)) * 3 + 0)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 1)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 2)] /
                         quality);
        break;
      case 1:
        tx = x - modifiedBlurOffset;
        ty = y;
        variableDiffuse =
            (float3)(allShadows[((int)(width * (ty) + (tx)) * 3 + 0)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 1)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 2)] /
                         quality);
        break;
      case 2:
        tx = x;
        ty = y + modifiedBlurOffset;
        variableDiffuse =
            (float3)(allShadows[((int)(width * (ty) + (tx)) * 3 + 0)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 1)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 2)] /
                         quality);
        break;
      case 3:
        tx = x;
        ty = y - modifiedBlurOffset;
        variableDiffuse =
            (float3)(allShadows[((int)(width * (ty) + (tx)) * 3 + 0)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 1)] / quality,
                     allShadows[((int)(width * (ty) + (tx)) * 3 + 2)] /
                         quality);
        break;
      }
      // if(y == height/2 && x == width/2){
      //     printf("%f\n", fabs(length(tmpDiffuse) - length(variableDiffuse)));
      // }
      if (fabs(length(tmpDiffuse) - length(variableDiffuse)) < 0.35) {
        overallDiffuse += variableDiffuse;
        validDirections++;
      }
    }
    overallDiffuse /= (validDirections + 1);
  }

  // overallDiffuse *= 0;

  // indirectColor3 = indirectColor3/max(4 * PI *
  // indirectDist*indirectDist, 1.0);

  // if(x == width/2 && y == height/2){
  //  printf("%f\n", length(indirectColor3));

  //}
  float3 lighting = ambient + overallDiffuse;
  if (useReflectDir) {
    totalIndirectColor = totalIndirectColor2;
  }
  color.x *= lighting.x + totalIndirectColor.x;
  color.y *= lighting.y + totalIndirectColor.y;
  color.z *= lighting.z + totalIndirectColor.z;
  output[(get_global_id(1) * width + get_global_id(0)) * 4 + 0] = inter.x;
  output[(get_global_id(1) * width + get_global_id(0)) * 4 + 1] = inter.y;
  output[(get_global_id(1) * width + get_global_id(0)) * 4 + 2] = inter.z;
  output[(get_global_id(1) * width + get_global_id(0)) * 4 + 3] = insideG;
  int vx = x;
  int vy = y;

  if (true) {

    float3 point = inter;
    float x = point.x;
    float y = point.y;
    float z = point.z; //+ random()/100;
    // float distFromLight = sqrt((x - light.x)*(x - light.x) + (y - light.y)*(y
    // - light.y) + (z - light.z)*(z - light.z));
    if (y2 == 327 && x2 == 400) {
      // printf("%d, %d, %f, %f\n",x, point.x, pos.x);
    }
    // float pitch = 0;

    float tmpx = x;
    x = x * cos(yaw * (PI / 180)) - z * sin(yaw * (PI / 180));
    z = z * cos(yaw * (PI / 180)) + tmpx * sin(yaw * (PI / 180));
    float tmpy = y;
    y = y * cos(pitch * (PI / 180)) - z * sin(pitch * (PI / 180));
    z = z * cos(pitch * (PI / 180)) + tmpy * sin(pitch * (PI / 180));
    tmpx = x;
    x = x * cos(roll * (PI / 180)) - y * sin(roll * (PI / 180));
    y = y * cos(roll * (PI / 180)) + tmpx * sin(roll * (PI / 180));
    if (z <= 0) {
      z = 0.00001;
    }
    float x2d = x * focalLength / z;
    float y2d = y * focalLength / z;
    // out[((int))*width + ()] = 0xFFFFFFFF;
    // write_imageui(out, (int2)((int)((((float)width)/2)+x2d),
    // ((((float)height)/2)+y2d)), (uint4)(255, 255, 255, 255));
    int px = (int)(width / 2 + x2d);
    int py = (int)(height / 2 + y2d);

    if (px >= 0 && px < width && py >= 0 && py < height) {

      // write_imageui(out, (int2)(px, py), (uint4)(255, 255, 255, 255));
      // write_imageui(out, (int2)(px, py), (uint4)(0, 255, 255, 255));
      // printf("Writing white pixel at (%d, %d)\n", px, py);
      // float4 color = read_imagef(textures, sampler, (int2)((float)px/width *
      // 255, (float)py/height * 255));
      float coolness = 1.2;
      float contrast = 1.15;
      uchar r =
          (uchar)(clamp(color.x * pow(color.x, contrast - 1), 0.0f, 1.0f) *
                  255.0f / max(1.0f, 1.0f));
      uchar g =
          (uchar)(clamp(color.y * pow(color.y, contrast - 1), 0.0f, 1.0f) *
                  255.0f / max(1.0f, 1.0f));
      uchar b =
          (uchar)(clamp(color.z * pow(color.z, contrast - 1), 0.0f, 1.0f) *
                  255.0f / max(1.0f, 1.0f));
      uchar a = (uchar)(clamp(color.w, 0.0f, 1.0f) * 255.0f / max(1.0f, 1.0f));
      image[py * width + px] = (uchar4)(r, g, b, a);
    }
    // image[vy*width + vx] = (uchar4)(0, 0, 255, 255);
  }
}
