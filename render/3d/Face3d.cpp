//
// Created by Aidan Anderson on 4/12/25.
//

#include "Face3d.h"

Face3d::Face3d() {

}


std::vector<Vertex3d*>* Face3d::getVertices() {
    return &vertices;
}

void Face3d::addVertex(Vertex3d *v) {
    vertices.push_back(v);
}
