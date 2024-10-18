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
    collisionManifolds.clear();
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
    collisions.push_back(newCollision);
}
void collisionSolver::setupManifolds()
{
    for(collisionInfo collision : collisions)
    {
        collisionManifold manifold;
        manifold.firstID = collision.firstID;
        manifold.secondID = collision.secondID;
        manifold.numContacts = collision.numContacts;
        manifold.penetrationDepth = collision.penetrationDepth;
        manifold.collisionNormal = collision.collisionNormal;
        manifold.cp[0] = collision.cp1;
        manifold.cp[1] = collision.cp2;
        manifold.tot[0] = 0;
        manifold.tot[1] = 0;
        manifold.aDist[0] = collision.cp1 - (*entities)[collision.firstID]->polygonColliderInstance.centroid;
        manifold.bDist[0] = collision.cp1 - (*entities)[collision.secondID]->polygonColliderInstance.centroid;
        if(collision.numContacts == 2)
        {
            manifold.aDist[1] = collision.cp2 - (*entities)[collision.firstID]->polygonColliderInstance.centroid;
            manifold.bDist[1] = collision.cp2 - (*entities)[collision.secondID]->polygonColliderInstance.centroid;
        }
        manifold.j_wa[0] = -mathFuncs::cross(manifold.aDist[0], collision.collisionNormal);
        manifold.j_wb[0] = mathFuncs::cross(manifold.bDist[0], collision.collisionNormal);
        if(collision.numContacts == 2)
        {
            manifold.j_wa[1] = -mathFuncs::cross(manifold.aDist[1], collision.collisionNormal);
            manifold.j_wb[1] = mathFuncs::cross(manifold.bDist[1], collision.collisionNormal);
        }
        manifold.effectiveMass[0] = 1.0f / ((*entities)[collision.firstID]->polygonRigidbodyInstance.invMass + 
                        (*entities)[collision.secondID]->polygonRigidbodyInstance.invMass + 
                        manifold.j_wa[0] * manifold.j_wa[0] * (*entities)[collision.firstID]->polygonRigidbodyInstance.invMomentOfInertia +
                        manifold.j_wb[0] * manifold.j_wb[0] * (*entities)[collision.secondID]->polygonRigidbodyInstance.invMomentOfInertia);
        if(collision.numContacts == 2)
        {
            manifold.effectiveMass[1] = 1.0f / ((*entities)[collision.firstID]->polygonRigidbodyInstance.invMass + 
                    (*entities)[collision.secondID]->polygonRigidbodyInstance.invMass + 
                    manifold.j_wa[1] * manifold.j_wa[1] * (*entities)[collision.firstID]->polygonRigidbodyInstance.invMomentOfInertia +
                    manifold.j_wb[1] * manifold.j_wb[1] * (*entities)[collision.secondID]->polygonRigidbodyInstance.invMomentOfInertia);
        }
        collisionManifolds.push_back(manifold);
    }
}
void collisionSolver::resolveCollisions()
{
    float largestImpulse = 0;
    setupManifolds();
    do
    {
        largestImpulse = 0;
        for(collisionManifold collision : collisionManifolds)
        {

            for(int i = 0; i < collision.numContacts; i++)
            {
                glm::vec2 j_va = -collision.collisionNormal;
                float j_wa = collision.j_wa[i];
                glm::vec2 j_vb = collision.collisionNormal;
                float j_wb = collision.j_wb[i];
                        
                float jv = glm::dot(j_va, (*entities)[collision.firstID]->polygonRigidbodyInstance.velocity) + 
                        j_wa * (*entities)[collision.firstID]->polygonRigidbodyInstance.angularVelocity + 
                        glm::dot(j_vb, (*entities)[collision.secondID]->polygonRigidbodyInstance.velocity) +
                        j_wb * (*entities)[collision.secondID]->polygonRigidbodyInstance.angularVelocity;

                float restitution = (*(*entities)[collision.firstID]).polygonRigidbodyInstance.restitution * 
                                    (*(*entities)[collision.secondID]).polygonRigidbodyInstance.restitution;
                glm::vec2 relativeVel = -(*entities)[collision.firstID]->polygonRigidbodyInstance.velocity +
                                    -mathFuncs::cross((*entities)[collision.firstID]->polygonRigidbodyInstance.angularVelocity, collision.aDist[i]) + 
                                    (*entities)[collision.secondID]->polygonRigidbodyInstance.velocity +
                                    mathFuncs::cross((*entities)[collision.secondID]->polygonRigidbodyInstance.angularVelocity, collision.aDist[i]);
                float velAlongNormal = glm::dot(relativeVel, collision.collisionNormal);

                float b = -(bias / setup::fixedDeltaTime) * std::max(collision.penetrationDepth - slop, 0.0f) + restitution * velAlongNormal;

                float lambda = collision.effectiveMass[i] * (-(jv + b));

                float oldTot;

                oldTot = collision.tot[i];
                collision.tot[i] = std::max(0.0f, collision.tot[i] + lambda);
                lambda = collision.tot[i] - oldTot;

                largestImpulse = std::max(lambda, largestImpulse);

                (*entities)[collision.firstID]->polygonRigidbodyInstance.velocity += j_va * lambda * 
                                                                (*entities)[collision.firstID]->polygonRigidbodyInstance.invMass;
                (*entities)[collision.firstID]->polygonRigidbodyInstance.angularVelocity += j_wa * lambda *
                                                                (*entities)[collision.firstID]->polygonRigidbodyInstance.invMomentOfInertia;
                (*entities)[collision.secondID]->polygonRigidbodyInstance.velocity += j_vb * lambda * 
                                                                (*entities)[collision.secondID]->polygonRigidbodyInstance.invMass;
                (*entities)[collision.secondID]->polygonRigidbodyInstance.angularVelocity += j_wb * lambda *
                                                                (*entities)[collision.secondID]->polygonRigidbodyInstance.invMomentOfInertia;

            }
            for(collisionManifold collision : collisionManifolds)
            {
                for(int i = 0; i < collision.numContacts; i++)
                {
                    glm::vec2 first = collision.cp[i] - (*entities)[collision.firstID]->polygonColliderInstance.centroid;
                    glm::vec2 second = collision.cp[i] - (*entities)[collision.secondID]->polygonColliderInstance.centroid;
                    glm::vec2 relativeVel = -(*entities)[collision.firstID]->polygonRigidbodyInstance.velocity +
                                        -mathFuncs::cross((*entities)[collision.firstID]->polygonRigidbodyInstance.angularVelocity, first) + 
                                        (*entities)[collision.secondID]->polygonRigidbodyInstance.velocity +
                                        mathFuncs::cross((*entities)[collision.secondID]->polygonRigidbodyInstance.angularVelocity, second);
                    if(relativeVel.x * relativeVel.x + relativeVel.y * relativeVel.y <= minRelVel2) continue;
                    glm::vec2 tangent = -relativeVel - (glm::dot(relativeVel, collision.collisionNormal) * collision.collisionNormal);
                    tangent = glm::normalize(tangent);
                    
                    if(collision.firstID == 1) std::cout << tangent.x << " " << tangent.y << std::endl;
                }
            }

        }
    } while(std::abs(largestImpulse) > std::abs(smallestImpulse));
}