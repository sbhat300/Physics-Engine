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
    float effectiveMass1, effectiveMass2;
};

struct collisionManifold
{
    unsigned int firstID, secondID;
    int numContacts;
    float penetrationDepth;
    glm::vec2 collisionNormal;
    glm::vec2 cp[2];
    float tot[2];
    float totTangent[2];
    float effectiveMass[2];
    float j_wa[2], j_wb[2];
    glm::vec2 aDist[2], bDist[2];
    float restitutions[2];
};

struct warmStarts
{
    unsigned int firstID, secondID;
    int numContacts;
    float tot[2];
    float totTangent[2];
};

class collisionSolver
{
    public:
        std::pair<float, float> temp;

        unsigned int* counter;
        std::vector<collisionInfo> collisions;
        std::vector<collisionManifold> collisionManifolds;
        std::vector<warmStarts> prevTots;
        std::unordered_map<unsigned int, entity*>* entities;
        float bias;
        float slop;
        float restitutionSlop;
        float smallestImpulse;
        collisionSolver();
        collisionSolver(unsigned int* c);
        void reset();
        void updateCollisions();
        void registerCollision(unsigned int first, unsigned int second, int numContactPoints, glm::vec2 normal, float penDepth, glm::vec2 point1, glm::vec2 point2);
        void resolveCollisions();
        void setupManifolds();
        void warmStart();
        void blockSolve(collisionManifold* collision, float* out, float (&effectiveMass)[2][2], float* combinedMass);
        float singularSolve(collisionManifold* collision, int p);
    private:
        static bool compareCollision(collisionInfo lhs, collisionInfo rhs);
};

#endif