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
    private:
    std::vector<Vertex3d*> vertices;
};



#endif //FACE3D_H
