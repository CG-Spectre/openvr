//
// Created by Aidan Anderson on 4/9/25.
//

#ifndef VERTEXSTACK_H
#define VERTEXSTACK_H
#include <vector>

#include "../../Vector3d.h"


class Vertex3d {
    public:
    Vertex3d(Vector3d pose);
    Vertex3d(float x, float y, float z);
    Vertex3d();

    Vector3d *getPose();
    void setPose(Vector3d pose);
    std::vector<Vertex3d*> *getConnections();
    void addConnection(Vertex3d *connection);
    private:
    std::vector<Vertex3d*> connections;
    Vector3d pos;
};



#endif //VERTEXSTACK_H
