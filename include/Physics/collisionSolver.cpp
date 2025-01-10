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
void collisionSolver::registerCollision(unsigned int first, unsigned int second, int numContactPoints, glm::vec2 normal, 
                                        float penDepth, glm::vec2 point1, glm::vec2 point2)
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
        manifold.totTangent[0] = 0;
        manifold.totTangent[1] = 0;
        manifold.aDist[0] = collision.cp1 - (*entities)[collision.firstID]->collider.centroid;
        manifold.bDist[0] = collision.cp1 - (*entities)[collision.secondID]->collider.centroid;
        if(collision.numContacts == 2)
        {
            manifold.aDist[1] = collision.cp2 - (*entities)[collision.firstID]->collider.centroid;
            manifold.bDist[1] = collision.cp2 - (*entities)[collision.secondID]->collider.centroid;
        }
        manifold.j_wa[0] = mathFuncs::cross(-manifold.aDist[0], collision.collisionNormal);
        manifold.j_wb[0] = mathFuncs::cross(manifold.bDist[0], collision.collisionNormal);
        if(collision.numContacts == 2)
        {
            manifold.j_wa[1] = mathFuncs::cross(-manifold.aDist[1], collision.collisionNormal);
            manifold.j_wb[1] = mathFuncs::cross(manifold.bDist[1], collision.collisionNormal);
        }
        manifold.effectiveMass[0] = (*entities)[collision.firstID]->rigidbody.invMass + 
                        (*entities)[collision.secondID]->rigidbody.invMass + 
                        manifold.j_wa[0] * manifold.j_wa[0] * (*entities)[collision.firstID]->rigidbody.invMomentOfInertia +
                        manifold.j_wb[0] * manifold.j_wb[0] * (*entities)[collision.secondID]->rigidbody.invMomentOfInertia;
        if(collision.numContacts == 2)
        {
            manifold.effectiveMass[1] = (*entities)[collision.firstID]->rigidbody.invMass + 
                    (*entities)[collision.secondID]->rigidbody.invMass + 
                    manifold.j_wa[1] * manifold.j_wa[1] * (*entities)[collision.firstID]->rigidbody.invMomentOfInertia +
                    manifold.j_wb[1] * manifold.j_wb[1] * (*entities)[collision.secondID]->rigidbody.invMomentOfInertia;
        }
        manifold.relativeVel[0] = -(*entities)[collision.firstID]->rigidbody.velocity +
                                    -mathFuncs::cross((*entities)[collision.firstID]->rigidbody.angularVelocity, manifold.aDist[0]) + 
                                    (*entities)[collision.secondID]->rigidbody.velocity +
                                    mathFuncs::cross((*entities)[collision.secondID]->rigidbody.angularVelocity, manifold.bDist[0]);
        if(collision.numContacts == 2)
        {
            manifold.relativeVel[1] = -(*entities)[collision.firstID]->rigidbody.velocity +
                            -mathFuncs::cross((*entities)[collision.firstID]->rigidbody.angularVelocity, manifold.aDist[1]) + 
                            (*entities)[collision.secondID]->rigidbody.velocity +
                            mathFuncs::cross((*entities)[collision.secondID]->rigidbody.angularVelocity, manifold.bDist[1]);
        }   
        
        collisionManifolds.push_back(manifold);
    }
}
void collisionSolver::warmStart()
{
    if(prevTots.size() == 0 || collisionManifolds.size() == 0) return;
    int warmIndex = 0;
    int manifoldIndex = 0;
    while(warmIndex < prevTots.size() && manifoldIndex < collisionManifolds.size())
    {
        while(manifoldIndex < collisionManifolds.size() && collisionManifolds[manifoldIndex].firstID < prevTots[warmIndex].firstID) 
        {
            manifoldIndex++;
        }
        while(warmIndex < prevTots.size() && collisionManifolds[manifoldIndex].firstID > prevTots[warmIndex].firstID) 
        {
            warmIndex++;
        }
        while(warmIndex < prevTots.size() && manifoldIndex < collisionManifolds.size() && collisionManifolds[manifoldIndex].firstID == prevTots[warmIndex].firstID)
        {
            while(manifoldIndex < collisionManifolds.size() && collisionManifolds[manifoldIndex].secondID < prevTots[warmIndex].secondID) 
            {
                manifoldIndex++;
            }
            while(warmIndex < prevTots.size() && collisionManifolds[manifoldIndex].secondID > prevTots[warmIndex].secondID) 
            {
                warmIndex++;
            }
            if(collisionManifolds[manifoldIndex].secondID == prevTots[warmIndex].secondID)
            {
                for(int i = 0; i < collisionManifolds[manifoldIndex].numContacts; i++)
                {
                    if(prevTots[warmIndex].numContacts != collisionManifolds[manifoldIndex].numContacts) continue;
                    collisionManifolds[manifoldIndex].tot[i] = prevTots[warmIndex].tot[i];
                    (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.velocity += -collisionManifolds[manifoldIndex].collisionNormal * prevTots[warmIndex].tot[i] * 
                                                                    (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.invMass;
                    (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.angularVelocity += collisionManifolds[manifoldIndex].j_wa[i] * prevTots[warmIndex].tot[i] *
                                                                    (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.invMomentOfInertia;
                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.velocity += collisionManifolds[manifoldIndex].collisionNormal * prevTots[warmIndex].tot[i] * 
                                                                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.invMass;
                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.angularVelocity += collisionManifolds[manifoldIndex].j_wb[i] * prevTots[warmIndex].tot[i] *
                                                                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.invMomentOfInertia;
                }
                manifoldIndex++;
                warmIndex++;
            }
        }
    }
}
void collisionSolver::blockSolve(collisionManifold* collision, float* out)
{
    float jv[2];
    glm::vec2 j_va1 = -collision->collisionNormal;
    float j_wa1 = collision->j_wa[0];
    glm::vec2 j_vb1 = collision->collisionNormal;
    float j_wb1 = collision->j_wb[0];
    jv[0] = glm::dot(j_va1, (*entities)[collision->firstID]->rigidbody.velocity) + 
            j_wa1 * (*entities)[collision->firstID]->rigidbody.angularVelocity + 
            glm::dot(j_vb1, (*entities)[collision->secondID]->rigidbody.velocity) +
            j_wb1 * (*entities)[collision->secondID]->rigidbody.angularVelocity;
    glm::vec2 j_va2 = -collision->collisionNormal;
    float j_wa2 = collision->j_wa[1];
    glm::vec2 j_vb2 = collision->collisionNormal;
    float j_wb2 = collision->j_wb[1];
    jv[1] = glm::dot(j_va2, (*entities)[collision->firstID]->rigidbody.velocity) + 
            j_wa2 * (*entities)[collision->firstID]->rigidbody.angularVelocity + 
            glm::dot(j_vb2, (*entities)[collision->secondID]->rigidbody.velocity) +
            j_wb2 * (*entities)[collision->secondID]->rigidbody.angularVelocity;

    float b = -(bias / setup::fixedDeltaTime) * std::max(collision->penetrationDepth - slop, 0.0f);
    jv[0] += b;
    jv[1] += b;

    float mass[2][2];
    mass[0][0] = collision->effectiveMass[0];
    mass[1][1] = collision->effectiveMass[1];
    mass[0][1] = ((*entities)[collision->firstID]->rigidbody.invMass + 
                (*entities)[collision->secondID]->rigidbody.invMass + 
                j_wa2 * j_wa1 * (*entities)[collision->firstID]->rigidbody.invMomentOfInertia +
                j_wa2 * j_wa1 * (*entities)[collision->secondID]->rigidbody.invMomentOfInertia);
    mass[1][0] = mass[0][1];
    float det = 1 / (mass[0][0] * mass[1][1] - mass[0][1] * mass[1][0]);
    float effectiveMass[2][2];
    effectiveMass[0][0] = mass[1][1] * det;
    effectiveMass[1][1] = mass[0][0] * det;
    effectiveMass[0][1] = -mass[0][1] * det;
    effectiveMass[1][0] = -mass[1][0] * det;
    *(out + 0) = effectiveMass[0][0] * -jv[0] + effectiveMass[0][1] * -jv[1];
    *(out + 1) = effectiveMass[1][0] * -jv[0] + effectiveMass[1][1] * -jv[1];
}
float collisionSolver::singularSolve(collisionManifold* collision, int p)
{
    glm::vec2 j_va = -collision->collisionNormal;
    float j_wa = collision->j_wa[p];
    glm::vec2 j_vb = collision->collisionNormal;
    float j_wb = collision->j_wb[p];
            
    float jv = glm::dot(j_va, (*entities)[collision->firstID]->rigidbody.velocity) + 
            j_wa * (*entities)[collision->firstID]->rigidbody.angularVelocity + 
            glm::dot(j_vb, (*entities)[collision->secondID]->rigidbody.velocity) +
            j_wb * (*entities)[collision->secondID]->rigidbody.angularVelocity;

    float b = -(bias / setup::fixedDeltaTime) * std::max(collision->penetrationDepth - slop, 0.0f);
    
    float lambda = (1.0f / collision->effectiveMass[p]) * -(jv + b);
    return lambda;
}
void collisionSolver::resolveCollisions()
{
    float largestImpulse = 0;
    int counter = 0;
    setupManifolds();
    // warmStart();
    do
    {
        counter++;
        largestImpulse = 0;
        for(int pos = 0; pos < collisionManifolds.size(); pos++)
        {
            collisionManifold* collision = &collisionManifolds[pos];
            float lambdas[2] = {collision->tot[0], collision->tot[1]};
            for(int i = 0; i < collision->numContacts; i++)
            {
                glm::vec2 relativeVel = -(*entities)[collision->firstID]->rigidbody.velocity +
                                -mathFuncs::cross((*entities)[collision->firstID]->rigidbody.angularVelocity, collision->aDist[i]) + 
                                (*entities)[collision->secondID]->rigidbody.velocity +
                                mathFuncs::cross((*entities)[collision->secondID]->rigidbody.angularVelocity, collision->bDist[i]);
                glm::vec2 tangent = relativeVel - (glm::dot(relativeVel, collision->collisionNormal) * collision->collisionNormal);
                if(tangent.x * tangent.x + tangent.y * tangent.y == 0) continue;
                tangent = glm::normalize(tangent);
                glm::vec2 j_ta = -tangent;
                glm::vec2 j_tb = tangent;
                float j_wa = mathFuncs::cross(-collision->aDist[i], j_tb);
                float j_wb = mathFuncs::cross(collision->bDist[i], j_tb);                
                float jv = glm::dot(j_ta, (*entities)[collision->firstID]->rigidbody.velocity) + 
                        j_wa * (*entities)[collision->firstID]->rigidbody.angularVelocity + 
                        glm::dot(j_tb, (*entities)[collision->secondID]->rigidbody.velocity) +
                        j_wb * (*entities)[collision->secondID]->rigidbody.angularVelocity;

                float effectiveMass = (*entities)[collision->firstID]->rigidbody.invMass + 
                        (*entities)[collision->secondID]->rigidbody.invMass + 
                        j_wa * j_wa * (*entities)[collision->firstID]->rigidbody.invMomentOfInertia +
                        j_wb * j_wb * (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                float lambda = 1.0f / effectiveMass * -jv;

                float friction = std::max((*entities)[collision->firstID]->rigidbody.mu, (*entities)[collision->secondID]->rigidbody.mu);
                float normalForce = std::abs(lambdas[i]) * friction;
                
                float oldTot = collision->totTangent[i];
                collision->totTangent[i] = mathFuncs::clamp(-normalForce, normalForce, collision->totTangent[i] + lambda);
                lambda = collision->totTangent[i] - oldTot;
                lambda = 0;

                (*entities)[collision->firstID]->rigidbody.velocity += j_ta * lambda * 
                                                            (*entities)[collision->firstID]->rigidbody.invMass;
                (*entities)[collision->firstID]->rigidbody.angularVelocity += j_wa * lambda *
                                                                (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                (*entities)[collision->secondID]->rigidbody.velocity += j_tb * lambda * 
                                                                (*entities)[collision->secondID]->rigidbody.invMass;
                (*entities)[collision->secondID]->rigidbody.angularVelocity += j_wb * lambda *
                                                                (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;

            }
            if(collision->numContacts == 2)
            {
                float out[2];
                blockSolve(collision, out);
                float lambda;
                if(collision->tot[0] + out[0] >= 0 && collision->tot[1] + out[1] >= 0)
                {
                    for(int i = 0; i < 2; i++) collision->tot[i] = collision->tot[i] + out[i];
                    largestImpulse = std::max(std::abs(out[1]), std::max(std::abs(out[0]), largestImpulse));

                    (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * (out[0] + out[1]) * 
                                                                (*entities)[collision->firstID]->rigidbody.invMass;
                    (*entities)[collision->firstID]->rigidbody.angularVelocity += (collision->j_wa[0] * out[0] + collision->j_wa[1] * out[1]) *
                                                                    (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                    (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * (out[0] + out[1]) * 
                                                                    (*entities)[collision->secondID]->rigidbody.invMass;
                    (*entities)[collision->secondID]->rigidbody.angularVelocity += (collision->j_wb[0] * out[0] + collision->j_wb[1] * out[1]) *
                                                                    (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                }
                else
                {
                    lambda = singularSolve(collision, 0);
                    if(collision->tot[0] + lambda >= 0)
                    {
                        collision->tot[0] = collision->tot[0] + lambda;
                        largestImpulse = std::max(std::abs(lambda), largestImpulse);

                        (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * lambda * 
                                                                        (*entities)[collision->firstID]->rigidbody.invMass;
                        (*entities)[collision->firstID]->rigidbody.angularVelocity += collision->j_wa[0] * lambda *
                                                                        (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                        (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * lambda * 
                                                                        (*entities)[collision->secondID]->rigidbody.invMass;
                        (*entities)[collision->secondID]->rigidbody.angularVelocity += collision->j_wb[0] * lambda *
                                                                        (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                    }
                    else
                    {
                        lambda = singularSolve(collision, 1);
                        if(collision->tot[1] + lambda >= 0)
                        {
                            collision->tot[1] = collision->tot[1] + lambda;
                            largestImpulse = std::max(std::abs(lambda), largestImpulse);

                            (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * lambda * 
                                                                            (*entities)[collision->firstID]->rigidbody.invMass;
                            (*entities)[collision->firstID]->rigidbody.angularVelocity += collision->j_wa[1] * lambda *
                                                                            (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                            (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * lambda * 
                                                                            (*entities)[collision->secondID]->rigidbody.invMass;
                            (*entities)[collision->secondID]->rigidbody.angularVelocity += collision->j_wb[1] * lambda *
                                                                            (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                        }
                    }
                }
            }
            else
            {
                for(int i = 0; i < collision->numContacts; i++)
                {
                    float lambda = singularSolve(collision, i);
                    float oldTot = collision->tot[i];
                    collision->tot[i] = std::max(0.0f, collision->tot[i] + lambda);
                    lambda = collision->tot[i] - oldTot;
                    lambdas[0] = collision->tot[i];
                    largestImpulse = std::max(std::abs(lambda), largestImpulse);

                    (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * lambda * 
                                                                    (*entities)[collision->firstID]->rigidbody.invMass;
                    (*entities)[collision->firstID]->rigidbody.angularVelocity += collision->j_wa[i] * lambda *
                                                                    (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                    (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * lambda * 
                                                                    (*entities)[collision->secondID]->rigidbody.invMass;
                    (*entities)[collision->secondID]->rigidbody.angularVelocity += collision->j_wb[i] * lambda *
                                                                    (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                }
            }
        }
    } while(std::abs(largestImpulse) > std::abs(smallestImpulse) && counter < 50);
    
    // prevTots.clear();
    // for(int pos = 0; pos < collisionManifolds.size(); pos++)
    // {
    //     warmStarts prev;
    //     prev.firstID = collisionManifolds[pos].firstID;
    //     prev.secondID = collisionManifolds[pos].secondID;
    //     prev.tot[0] = collisionManifolds[pos].tot[0];
    //     if(collisionManifolds[pos].numContacts == 2) prev.tot[1] = collisionManifolds[pos].tot[1];
    //     else prev.tot[1] = -1;
    //     prev.numContacts = collisionManifolds[pos].numContacts;
    //     prevTots.push_back(prev);
    // }
    
    // for(int pos = 0; pos < collisionManifolds.size(); pos++)
    // {
    //     collisionManifold* collision = &collisionManifolds[pos];
    //     std::cout << collision->tot[0] << " " << collision->tot[1] << std::endl;
    //     for(int i = 0; i < collision->numContacts; i++)
    //     {
    //         float restitution = (*entities)[collision->firstID]->rigidbody.restitution * 
    //                             (*entities)[collision->secondID]->rigidbody.restitution;

    //         float velAlongNormal = glm::dot(collision->relativeVel[i], collision->collisionNormal);
    //         float b = -(bias / setup::fixedDeltaTime) * std::max(collision->penetrationDepth - slop, 0.0f) + restitution * velAlongNormal;
    //         float lambda = 1.0f / collision->effectiveMass[i] * (-b);
    //         // std::cout << b << std::endl;
    //         largestImpulse = std::max(largestImpulse, lambda);
    //         // lambda = 0;

    //         (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * lambda * 
    //                                                         (*entities)[collision->firstID]->rigidbody.invMass;
    //         (*entities)[collision->firstID]->rigidbody.angularVelocity += collision->j_wa[i] * lambda *
    //                                                         (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
    //         (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * lambda * 
    //                                                         (*entities)[collision->secondID]->rigidbody.invMass;
    //         (*entities)[collision->secondID]->rigidbody.angularVelocity += collision->j_wb[i] * lambda *
    //                                                         (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
    //         // collision->relativeVel[i] =  -(*entities)[collision->firstID]->rigidbody.velocity +
    //         //                     -mathFuncs::cross((*entities)[collision->firstID]->rigidbody.angularVelocity, collision->aDist[i]) + 
    //         //                     (*entities)[collision->secondID]->rigidbody.velocity +
    //         //                     mathFuncs::cross((*entities)[collision->secondID]->rigidbody.angularVelocity, collision->bDist[i]);
    //     }
    // }
    std::cout << std::endl;
}