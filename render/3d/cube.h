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
    SerializedObject getSerializedFaces() override;


    bool renderRay(SDL_Renderer *renderer, camera* camera, Vector3d point, Vector3d direction) override;
    cube(Pose3d pos, float edgeLength);
    std::vector<Face3d*> faces;
    private:
    Vertex3d firstVertex;
    Pose3d pos;

};



#endif //CUBE_H
