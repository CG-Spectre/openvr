//
// Created by Aidan Anderson on 4/7/25.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "renderable.h"
#include "renderableRT.h"
#include "renderStack.h"
#include "../Pose3d.h"
#include "3d/BVHNode.h"
#include "3d/light.h"
#include "scene.h"


class camera : public renderable {
public:
    camera();
    camera(Pose3d pos);
    void render(SDL_Renderer *renderer) override;
    void addObject(renderableRT *object);
    Pose3d* getPos();
    BVHNode rootBVH;
    void addLight(light *light);
    void stop();
    void init();
    void render(SDL_Renderer * renderer, Vector3d vector3d, Vector3d rotational_velocity) override;
    void setReflectDir(Vector3d* direction, Vector3d* direction1, Vector3d* direction2, Vector3d* direction3);
    void setScene(Scene* sceneRef);
    struct indirectLightingResult{
        float radiance;
        float interX;
        float interY;
        float interZ;
        float sampleX;
        float sampleY;
        float sampleZ;
        float albedo;
        float shininess;
        float normalX;
        float normalY;
        float normalZ;
        float reflectX;
        float reflectY;
        float reflectZ;
    };
private:
    int time = 0;
    int prevWidth;
    int prevHeight;
    Pose3d pos;
    renderStack stack;
    renderStack lightStack;
    std::vector<light *> lights;
    Scene* scene = nullptr;
    BVHNode* sceneBVH = nullptr;
    bool useReflectDir = false;
    Vector3d *reflectDir;
    Vector3d *reflectDir1;
    Vector3d *reflectDir2;
    Vector3d *reflectDir3;
    std::vector<indirectLightingResult> allILResults;
};



#endif //CAMERA_H
