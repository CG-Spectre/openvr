//
// Created by Aidan Anderson on 4/7/25.
//

#include "cube.h"

#include <iostream>
#include "cmath"
#include "renderUtil.h"
#include "Vertex3d.h"
#include "../camera.h"
#include "../SDLConfig.h"
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
cube::cube(Pose3d pos, float edgeLength) {
    this->pos = pos;
    float halfedge = edgeLength / 2.0f;
    v1 = Vertex3d(halfedge, halfedge, halfedge);
    v2 = Vertex3d(-halfedge, halfedge, halfedge);
    v3 = Vertex3d(halfedge, halfedge, -halfedge);
    v4 = Vertex3d(-halfedge, halfedge, -halfedge);
    v5 = Vertex3d(halfedge, -halfedge, halfedge);
    v6 = Vertex3d(-halfedge, -halfedge, halfedge);
    v7 = Vertex3d(halfedge, -halfedge, -halfedge);
    v8 = Vertex3d(-halfedge, -halfedge, -halfedge);
    v1.addConnection(&v2);
    v1.addConnection(&v3);
    v2.addConnection(&v4);
    v2.addConnection(&v1);
    v3.addConnection(&v4);
    v3.addConnection(&v1);
    v4.addConnection(&v2);
    v4.addConnection(&v3);

    v5.addConnection(&v6);
    v5.addConnection(&v7);
    v6.addConnection(&v8);
    v6.addConnection(&v5);
    v7.addConnection(&v8);
    v7.addConnection(&v5);
    v8.addConnection(&v6);
    v8.addConnection(&v7);

    v1.addConnection(&v5);
    v2.addConnection(&v6);
    v3.addConnection(&v7);
    v4.addConnection(&v8);
    v5.addConnection(&v1);
    v6.addConnection(&v2);
    v7.addConnection(&v3);
    v8.addConnection(&v4);

    f1.addVertex(&v1);
    f1.addVertex(&v2);
    f1.addVertex(&v4);
    f1.addVertex(&v3);

    f2.addVertex(&v5);
    f2.addVertex(&v6);
    f2.addVertex(&v7);
    f2.addVertex(&v8);

    f3.addVertex(&v1);
    f3.addVertex(&v2);
    f3.addVertex(&v6);
    f3.addVertex(&v5);

    f4.addVertex(&v1);
    f4.addVertex(&v3);
    f4.addVertex(&v5);
    f4.addVertex(&v7);

    f5.addVertex(&v3);
    f5.addVertex(&v4);
    f5.addVertex(&v8);
    f5.addVertex(&v7);

    f6.addVertex(&v4);
    f6.addVertex(&v2);
    f6.addVertex(&v8);
    f6.addVertex(&v6);

    faces.push_back(&f1);
    faces.push_back(&f2);
    faces.push_back(&f3);
    faces.push_back(&f4);
    faces.push_back(&f5);
    faces.push_back(&f6);
    firstVertex = v1;
    //firstVertex.getConnections(). [0].addConnection(new Vector3d(-halfedge, halfedge, -halfedge));
}

void cube::render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Black
    //std::cout << faces[0]->getVertices()->size() << std::endl;
    /*for (int i = 0; i < faces[2]->getVertices()->size(); i++) {

        float yaw = 0;
        float pitch = 0;
        float roll = 0;
        Vertex3d* vertex = faces[2]->getVertices()->at(i);
        float x = vertex->getPose()->x + this->pos.pose.x;
        float y = vertex->getPose()->y + this->pos.pose.y;
        float z = vertex->getPose()->z + this->pos.pose.z;
        //std::cout << x << ", " << y << ", " << z << std::endl;
        float tmpx = x;
        x = x*cos(yaw*(M_PI/180)) - z*sin(yaw*(M_PI/180));
        z = z * cos(yaw*(M_PI/180)) + tmpx * sin(yaw*(M_PI/180));
        float tmpy = y;
        y = y * cos(pitch*(M_PI/180)) - z * sin(pitch*(M_PI/180));
        z = z * cos(pitch*(M_PI/180)) + tmpy * sin(pitch*(M_PI/180));
        tmpx = x;
        x = x * cos(roll*(M_PI/180)) - y * sin(roll*(M_PI/180));
        y = y * cos(roll*(M_PI/180)) + tmpx * sin(roll*(M_PI/180));

        if (z <= 0) {
            z = 0.00001;
        }
        float x2d = x * 1000.0 / z;
        float y2d = y * 1000.0 / z;
        //std::cout << "x2d: " << x2d << std::endl;
        //std::cout << "y2d: " << y2d << std::endl;
        SDL_RenderPoint(renderer,(((float)*SDLConfig::WINDOW_WIDTH)/2)+x2d, (((float)*SDLConfig::WINDOW_HEIGHT)/2)-y2d);
    }

    //SDL_RenderLine(renderer, 0, 0, 255, 255);*/

}
float clip(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
}
bool cube::renderRay(SDL_Renderer *renderer, camera* camera, Vector3d point, Vector3d direction) {
    bool rendered = false;
    //direction = direction.normalize();
    //std::cout << direction.x << " " << direction.y << " " << direction.z << std::endl;
    for (int i = 0; i < 1; i++) {

        int faceIndex = 2;
        Face3d face = *faces[faceIndex];
        Vector3d p1 = *face.getVertices()->at(0)->getPose() + this->pos.pose;
        Vector3d p2 = *face.getVertices()->at(1)->getPose() + this->pos.pose;
        Vector3d p3 = *face.getVertices()->at(2)->getPose() + this->pos.pose;
        Vector3d v1 = p2 - p1;
        Vector3d v2 = p3 - p1;
        Vector3d normal = v1.cross(v2).normalize();

        float denom = normal.dot(direction);
        if (std::abs(denom) < 1e-6) {
            continue;
        }
        float t = normal.dot(p1 - point) / denom;
        Vector3d intersection = point + (direction*t);
        Vector3d target(0, 1, 0);
        Vector3d axis = normal.cross(target).normalize();
        if (std::isnan(axis.x) || std::isnan(axis.y) || std::isnan(axis.z)) {
            axis = target;
        }
        //axis = Vector3d(0, 1, 0);
        double angle = acos(  clip(normal.dot(target), -1, 1));
        std::vector<Vector3d> rotatedVertices;

        for (int i = 0; i < face.getVertices()->size(); i++) {
            Vertex3d* vertex = faces[faceIndex]->getVertices()->at(i);
            Vector3d v = *vertex->getPose() + this->pos.pose - intersection;
            Vector3d term1 = v * cos(angle);
            Vector3d term2 = axis.cross(v) * sin(angle);
            Vector3d term3 = axis * (axis.dot(v)) * (1 - cos(angle));
            Vector3d rotated = term1 + term2 + term3 + intersection;
            //std::cout << (rotated).x << " " << (rotated).y << " " << (rotated).z << std::endl;
            rotatedVertices.push_back(rotated);
        }

        //std::cout << (intersection).x << " " << (intersection).y << " " << (intersection).z << std::endl;
        if (!renderUtil::isInside(intersection, rotatedVertices)) {
            continue;
        }
        //render
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        renderUtil::render3dPoint(renderer, camera, intersection);
        rendered = true;
    }
    return rendered;
}
