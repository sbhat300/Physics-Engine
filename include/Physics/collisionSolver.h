#ifndef COLLISIONSOLVER_H
#define COLLISIONSOLVER_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entity.h>

struct collisionInfo
{
    unsigned int firstID, secondID;
    int numContacts;
    float penetrationDepth;
    glm::vec2 collisionNormal;
    glm::vec2 cp1, cp2;
    float tot1, tot2;
};

struct totalLambda
{
    std::pair<float, float> total = std::make_pair(-1.0f, -1.0f);
    unsigned int mark;
};

class collisionSolver
{
    public:
        std::pair<float, float> temp;

        unsigned int* counter;
        std::vector <collisionInfo> collisions;
        std::unordered_map<unsigned int, entity*>* entities;
        float bias;
        float slop;
        float smallestImpulse;
        collisionSolver();
        collisionSolver(unsigned int* c);
        void reset();
        void updateCollisions();
        void registerCollision(unsigned int first, unsigned int second, int numContactPoints, glm::vec2 normal, float penDepth, glm::vec2 point1, glm::vec2 point2);
        void resolveCollisions();
    private:
        static bool compareCollision(collisionInfo lhs, collisionInfo rhs);
};

#endif