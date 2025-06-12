//
// Created by aidan on 5/30/2025.
//

#ifndef PLANE_H
#define PLANE_H
#include "Face3d.h"
#include "Pose3d.h"
#include "SerializedObject.h"
#include "Vertex3d.h"
#include "render/renderableRT.h"


class plane : public renderableRT {
    public:
    void render(SDL_Renderer *renderer) override;
    SerializedObject getSerializedFaces(float tx, float ty, float tz) override;
    Face3d f1;
    Vertex3d v1;
    Vertex3d v2;
    Vertex3d v3;
    Vertex3d v4;
    plane(Pose3d pos, float edgeLength);
    plane(Pose3d pos, float edgeLength, int texture);
    std::vector<Face3d*> faces;
    private:
    int texture;
    Vertex3d firstVertex;
    Pose3d pos;
    float maxFloat;
    float minFloat;
};



#endif //PLANE_H
