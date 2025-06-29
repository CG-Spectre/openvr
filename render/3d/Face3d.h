//
// Created by Aidan Anderson on 4/12/25.
//

#ifndef FACE3D_H
#define FACE3D_H
#include <vector>

#include "Vertex3d.h"


class Face3d {
    public:
    Face3d();
    std::vector<Vertex3d*>* getVertices();
    void addVertex(Vertex3d *vertex);
    int textureId = -1;
    void setTextureId(int id);
    int textureRotation = 0;
    void setTextureRotation(int rot);
    int specularMap = -1;
    int reflectionMap = -1;
    int normalMap = -1;
    int displacementMap = -1;
    void setSpecularMap(int id);
    void setReflectionMap(int id);
    void setNormalMap(int id);
    void setDisplacementMap(int id);

private:
    std::vector<Vertex3d*> vertices;
};



#endif //FACE3D_H
