#include "Physics/collisionSolver.h"
#include <iostream>
#include <setup.h>
#include <unordered_map>
#include <mathFuncs.h>
#include <algorithm>

collisionSolver::collisionSolver()
{
    temp.first = 0;
    temp.second = 0;
}
collisionSolver::collisionSolver(unsigned int* c)
{
    counter = c;
    temp.first = 0;
    temp.second = 0;
}
bool collisionSolver::compareCollision(collisionInfo lhs, collisionInfo rhs)
{
    if(lhs.firstID < rhs.firstID) return true;
    if(lhs.firstID == rhs.firstID) return lhs.secondID < rhs.secondID;
    return false;
}
void collisionSolver::updateCollisions()
{
    std::sort(collisions.begin(), collisions.end(), compareCollision);
    std::vector<collisionInfo> uniqueCollisions;
    int i = 0;
    while(i < collisions.size())
    {
        uniqueCollisions.push_back(*(collisions.begin() + i));
        i++;
        while(i < collisions.size())
        {
            auto collision = collisions.begin() + i;
            if((*collision).firstID != uniqueCollisions.back().firstID || (*collision).secondID != uniqueCollisions.back().secondID) break;
            i++;
        }
    }
    collisions = uniqueCollisions;
}
void collisionSolver::reset()
{
    collisions.clear();
}
void collisionSolver::registerCollision(unsigned int first, unsigned int second, int numContactPoints, glm::vec2 normal, float penDepth, glm::vec2 point1, glm::vec2 point2)
{
    collisionInfo newCollision;
    if(first < second)
    {
        newCollision.firstID = first;
        newCollision.secondID = second;
        newCollision.collisionNormal = normal;
    }
    else
    {
        newCollision.secondID = first;
        newCollision.firstID = second;
        newCollision.collisionNormal = -normal;
    }
    newCollision.numContacts = numContactPoints;
    newCollision.penetrationDepth = penDepth;
    newCollision.cp1 = point1;
    newCollision.cp2 = point2;
    newCollision.tot1 = 0;
    newCollision.tot2 = 0;
    collisions.push_back(newCollision);
}
void collisionSolver::resolveCollisions()
{
    float largestImpulse = 0;
    do
    {
        largestImpulse = 0;
        for(collisionInfo collision : collisions)
        {
            for(int i = 0; i < collision.numContacts; i++)
            {
                glm::vec2 contact;
                if(i == 0) contact = collision.cp1;
                else contact = collision.cp2;
                glm::vec2 aDist = contact - (*entities)[collision.firstID]->polygonColliderInstance.centroid;
                glm::vec2 bDist = contact - (*entities)[collision.secondID]->polygonColliderInstance.centroid;

                glm::vec2 j_va = -collision.collisionNormal;
                float j_wa = -mathFuncs::cross(aDist, collision.collisionNormal);
                glm::vec2 j_vb = collision.collisionNormal;
                float j_wb = mathFuncs::cross(bDist, collision.collisionNormal);
                
                float effectiveMass = 1.0f / ((*entities)[collision.firstID]->polygonRigidbodyInstance.invMass + 
                        (*entities)[collision.secondID]->polygonRigidbodyInstance.invMass + 
                        j_wa * j_wa * (*entities)[collision.firstID]->polygonRigidbodyInstance.invMomentOfInertia +
                        j_wb * j_wb * (*entities)[collision.secondID]->polygonRigidbodyInstance.invMomentOfInertia);
                        
                float jv = glm::dot(j_va, (*entities)[collision.firstID]->polygonRigidbodyInstance.velocity) + 
                        j_wa * (*entities)[collision.firstID]->polygonRigidbodyInstance.angularVelocity + 
                        glm::dot(j_vb, (*entities)[collision.secondID]->polygonRigidbodyInstance.velocity) +
                        j_wb * (*entities)[collision.secondID]->polygonRigidbodyInstance.angularVelocity;

                float restitution = (*(*entities)[collision.firstID]).polygonRigidbodyInstance.restitution * 
                                    (*(*entities)[collision.secondID]).polygonRigidbodyInstance.restitution;
                glm::vec2 relativeVel = -(*entities)[collision.firstID]->polygonRigidbodyInstance.velocity +
                                    -mathFuncs::cross((*entities)[collision.firstID]->polygonRigidbodyInstance.angularVelocity, aDist) + 
                                    (*entities)[collision.secondID]->polygonRigidbodyInstance.velocity +
                                    mathFuncs::cross((*entities)[collision.secondID]->polygonRigidbodyInstance.angularVelocity, bDist);
                float velAlongNormal = glm::dot(relativeVel, collision.collisionNormal);

                float b = -(bias / setup::fixedDeltaTime) * std::max(collision.penetrationDepth - slop, 0.0f) + restitution * velAlongNormal;

                float lambda = effectiveMass * (-(jv + b));

                float oldTot;
                if(i == 0)
                {
                    oldTot = collision.tot1;
                    collision.tot1 = std::max(0.0f, collision.tot1 + lambda);
                    lambda = collision.tot1 - oldTot;
                }
                else
                {
                    oldTot = collision.tot2;
                    collision.tot2 = std::max(0.0f, collision.tot2 + lambda);
                    lambda = collision.tot2 - oldTot;
                }
                largestImpulse = std::max(lambda, largestImpulse);

                (*entities)[collision.firstID]->polygonRigidbodyInstance.addImpulse(j_va.x * lambda, j_va.y * lambda);
                (*entities)[collision.secondID]->polygonRigidbodyInstance.addImpulse(j_vb.x * lambda, j_vb.y * lambda);   
                (*entities)[collision.firstID]->polygonRigidbodyInstance.addAngularImpulse(j_wa * lambda);
                (*entities)[collision.secondID]->polygonRigidbodyInstance.addAngularImpulse(j_wb * lambda); 

            }

            (*entities)[collision.firstID]->polygonRigidbodyInstance.applyImpulses();
            (*entities)[collision.secondID]->polygonRigidbodyInstance.applyImpulses();
        }
    } while(std::abs(largestImpulse) > std::abs(smallestImpulse));
}