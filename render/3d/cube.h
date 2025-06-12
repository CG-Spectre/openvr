//
// Created by Aidan Anderson on 4/7/25.
//

#ifndef CUBE_H
#define CUBE_H
#include "Face3d.h"
#include "Vertex3d.h"
#include "../renderableRT.h"
#include "../../Pose3d.h"


class camera;

class cube : public renderableRT {
    public:
    void render(SDL_Renderer *renderer) override;
    SerializedObject getSerializedFaces(float tx, float ty, float tz) override;

    Face3d f1;
    Face3d f2;
    Face3d f3;
    Face3d f4;
    Face3d f5;
    Face3d f6;
    Vertex3d v1;
    Vertex3d v2;
    Vertex3d v3;
    Vertex3d v4;
    Vertex3d v5;
    Vertex3d v6;
    Vertex3d v7;
    Vertex3d v8;
    bool renderRay(SDL_Renderer *renderer, camera* camera, Vector3d point, Vector3d direction);
    cube(Pose3d pos, float edgeLength);
    std::vector<Face3d*> faces;
    private:
    float maxFloat;
    float minFloat;
    Vertex3d firstVertex;
    Pose3d pos;

};



#endif //CUBE_H
