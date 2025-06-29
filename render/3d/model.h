//
// Created by aidan on 6/12/2025.
//

#ifndef MODEL_H
#define MODEL_H
#include <string>
#include <vector>

#include "Face3d.h"
#include "Pose3d.h"
#include "Vertex3d.h"
#include "render/renderableRT.h"


class model : public renderableRT {
public:
    explicit model(std::string name, Pose3d pos);
    explicit model(std::string name, Pose3d pos, float scale);
    void render(SDL_Renderer *renderer) override;
    SerializedObject getSerializedFaces(float tx, float ty, float tz) override;
    Pose3d pos;
    void render(SDL_Renderer * renderer, Vector3d vector3d, Vector3d rotational_velocity) override;
private:
    std::vector<Vertex3d> vertices;
    std::vector<Face3d> faces;
    float maxFloat, minFloat;
    float scale;
};



#endif //MODEL_H
