#ifndef RAYDATA_H
#define RAYDATA_H

#include <glm/glm.hpp>
struct rayData
{
    int id;
    glm::vec2 collisionPoint;
    glm::vec2 collisionNormal;
};

#endif