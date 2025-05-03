//
// Created by aidan on 4/13/2025.
//

#ifndef RENDERUTIL_H
#define RENDERUTIL_H
#include <vector>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

#include "../../Vector3d.h"
#include "../../Vector2d.h"


class camera;

class renderUtil {
public:
    static void render3dPoint(SDL_Renderer* renderer, camera* cam, Vector3d point);

    static Vector2d get2dPoint(SDL_Renderer *renderer, camera *camera, Vector3d point);

    static void render2dPoint(SDL_Renderer* renderer, float x, float y);
    static bool isInside(Vector3d point, std::vector<Vector3d> points);
};



#endif //RENDERUTIL_H
