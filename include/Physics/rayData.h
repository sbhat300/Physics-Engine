#ifndef RAYDATA_H
#define RAYDATA_H

#include <glm/glm.hpp>
struct rayData
{
    unsigned int id;
    glm::vec2 collisionPoint;
    glm::vec2 collisionNormal;
};

#endif