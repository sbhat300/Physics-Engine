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
    newCollision.cp1 = point1;
    newCollision.cp2 = point2;
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
        if(numContactPoints == 2)
        {
            newCollision.cp1 = point2;
            newCollision.cp2 = point1;
        }
    }
    newCollision.numContacts = numContactPoints;
    newCollision.penetrationDepth = penDepth;
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
        float r = (*entities)[collision.firstID]->rigidbody.restitution * (*entities)[collision.secondID]->rigidbody.restitution;
        float rv = glm::dot((-(*entities)[collision.firstID]->rigidbody.velocity +
                                    -mathFuncs::cross((*entities)[collision.firstID]->rigidbody.angularVelocity, manifold.aDist[0]) + 
                                    (*entities)[collision.secondID]->rigidbody.velocity +
                                    mathFuncs::cross((*entities)[collision.secondID]->rigidbody.angularVelocity, manifold.bDist[0])), 
                                    manifold.collisionNormal);
        manifold.restitutions[0] = r * (std::abs(rv) > restitutionSlop ? rv : 0);
        if(collision.numContacts == 2)
        {
            rv = glm::dot((-(*entities)[collision.firstID]->rigidbody.velocity +
                            -mathFuncs::cross((*entities)[collision.firstID]->rigidbody.angularVelocity, manifold.aDist[1]) + 
                            (*entities)[collision.secondID]->rigidbody.velocity +
                            mathFuncs::cross((*entities)[collision.secondID]->rigidbody.angularVelocity, manifold.bDist[1])),
                            manifold.collisionNormal);

            manifold.restitutions[1] = r * (std::abs(rv) > restitutionSlop ? rv : 0);
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
                    glm::vec2 tangent = mathFuncs::cross(collisionManifolds[manifoldIndex].collisionNormal, 1.0f);
                    collisionManifolds[manifoldIndex].totTangent[i] = prevTots[warmIndex].totTangent[i];
                    float j_wa = mathFuncs::cross(-collisionManifolds[manifoldIndex].aDist[i], tangent);
                    float j_wb = mathFuncs::cross(collisionManifolds[manifoldIndex].bDist[i], tangent);   
                    (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.velocity += -tangent * prevTots[warmIndex].totTangent[i] * 
                                                        (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.invMass;
                    (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.angularVelocity += j_wa * prevTots[warmIndex].totTangent[i] *
                                                                    (*entities)[collisionManifolds[manifoldIndex].firstID]->rigidbody.invMomentOfInertia;
                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.velocity += tangent * prevTots[warmIndex].totTangent[i] * 
                                                                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.invMass;
                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.angularVelocity += j_wb * prevTots[warmIndex].totTangent[i] *
                                                                    (*entities)[collisionManifolds[manifoldIndex].secondID]->rigidbody.invMomentOfInertia;  
                    
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
void collisionSolver::blockSolve(collisionManifold* collision, float* out, float (&effectiveMass)[2][2], float* combinedMass)
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
    jv[0] += b + collision->restitutions[0];
    jv[1] += b + collision->restitutions[1];

    float mass[2][2];
    mass[0][0] = collision->effectiveMass[0];
    mass[1][1] = collision->effectiveMass[1];
    mass[0][1] = ((*entities)[collision->firstID]->rigidbody.invMass + 
                (*entities)[collision->secondID]->rigidbody.invMass + 
                j_wa2 * j_wa1 * (*entities)[collision->firstID]->rigidbody.invMomentOfInertia +
                j_wa2 * j_wa1 * (*entities)[collision->secondID]->rigidbody.invMomentOfInertia);
    mass[1][0] = mass[0][1];
    *combinedMass = mass[0][1];
    float det = 1 / (mass[0][0] * mass[1][1] - mass[0][1] * mass[1][0]);
    effectiveMass[0][0] = mass[1][1] * det;
    effectiveMass[1][1] = mass[0][0] * det;
    effectiveMass[0][1] = -mass[0][1] * det;
    effectiveMass[1][0] = -mass[1][0] * det;
    *(out + 0) = jv[0] - (mass[0][0] * collision->tot[0] + mass[0][1] * collision->tot[1]);
    *(out + 1) = jv[1] - (mass[1][0] * collision->tot[0] + mass[1][1] * collision->tot[1]);
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

    float b = -(bias / setup::fixedDeltaTime) * std::max(collision->penetrationDepth - slop, 0.0f) + collision->restitutions[p];
    
    float lambda = (1.0f / collision->effectiveMass[p]) * -(jv + b);
    return lambda;
}
void collisionSolver::resolveCollisions()
{
    float largestImpulse = 0;
    int counter = 0;
    setupManifolds();
    warmStart();
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
                glm::vec2 tangent = mathFuncs::cross(collision->collisionNormal, 1.0f);
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
                float effectiveMass[2][2];
                float combinedMass;
                blockSolve(collision, out, effectiveMass, &combinedMass);
                float impulses[2] = {effectiveMass[0][0] * -out[0] + effectiveMass[0][1] * -out[1], effectiveMass[1][0] * -out[0] + effectiveMass[1][1] * -out[1]};
                float lambdas[2];
                if(impulses[0] >= 0 && impulses[1] >= 0)
                {
                    for(int i = 0; i < 2; i++) 
                    {
                        lambdas[i] = impulses[i] - collision->tot[i];
                        collision->tot[i] = impulses[i];
                    }
                    largestImpulse = std::max(std::abs(lambdas[1]), std::max(std::abs(lambdas[0]), largestImpulse));

                    (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                (*entities)[collision->firstID]->rigidbody.invMass;
                    (*entities)[collision->firstID]->rigidbody.angularVelocity += (collision->j_wa[0] * lambdas[0] + collision->j_wa[1] * lambdas[1]) *
                                                                    (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                    (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                    (*entities)[collision->secondID]->rigidbody.invMass;
                    (*entities)[collision->secondID]->rigidbody.angularVelocity += (collision->j_wb[0] * lambdas[0] + collision->j_wb[1] * lambdas[1]) *
                                                                    (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                }
                else
                {
                    impulses[0] = 1.0f / collision->effectiveMass[0] * -out[0];
                    impulses[1] = 0;
                    float vn = combinedMass * impulses[0] + out[1];
                    if(impulses[0] >= 0 && vn >= 0)
                    {
                        for(int i = 0; i < 2; i++) 
                        {
                            lambdas[i] = impulses[i] - collision->tot[i];
                            collision->tot[i] = impulses[i];
                        }
                        largestImpulse = std::max(std::abs(lambdas[1]), std::max(std::abs(lambdas[0]), largestImpulse));

                        (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                    (*entities)[collision->firstID]->rigidbody.invMass;
                        (*entities)[collision->firstID]->rigidbody.angularVelocity += (collision->j_wa[0] * lambdas[0] + collision->j_wa[1] * lambdas[1]) *
                                                                        (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                        (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                        (*entities)[collision->secondID]->rigidbody.invMass;
                        (*entities)[collision->secondID]->rigidbody.angularVelocity += (collision->j_wb[0] * lambdas[0] + collision->j_wb[1] * lambdas[1]) *
                                                                        (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                    }
                    else
                    {
                        impulses[0] = 0;
                        impulses[1] = 1.0f / collision->effectiveMass[1] * -out[1];
                        vn = combinedMass * impulses[1] + out[0];
                        if(impulses[1] >= 0 && vn >= 0)
                        {
                            for(int i = 0; i < 2; i++) 
                            {
                                lambdas[i] = impulses[i] - collision->tot[i];
                                collision->tot[i] = impulses[i];
                            }
                            largestImpulse = std::max(std::abs(lambdas[1]), std::max(std::abs(lambdas[0]), largestImpulse));

                            (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                        (*entities)[collision->firstID]->rigidbody.invMass;
                            (*entities)[collision->firstID]->rigidbody.angularVelocity += (collision->j_wa[0] * lambdas[0] + collision->j_wa[1] * lambdas[1]) *
                                                                            (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                            (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                            (*entities)[collision->secondID]->rigidbody.invMass;
                            (*entities)[collision->secondID]->rigidbody.angularVelocity += (collision->j_wb[0] * lambdas[0] + collision->j_wb[1] * lambdas[1]) *
                                                                            (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                        }
                        else
                        {
                            impulses[0] = 0;
                            impulses[1] = 0;
                            if(out[0] >= 0 && out[1] >= 0)
                            {
                                for(int i = 0; i < 2; i++) 
                                {
                                    lambdas[i] = impulses[i] - collision->tot[i];
                                    collision->tot[i] = impulses[i];
                                }
                                largestImpulse = std::max(std::abs(lambdas[1]), std::max(std::abs(lambdas[0]), largestImpulse));

                                (*entities)[collision->firstID]->rigidbody.velocity += -collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                            (*entities)[collision->firstID]->rigidbody.invMass;
                                (*entities)[collision->firstID]->rigidbody.angularVelocity += (collision->j_wa[0] * lambdas[0] + collision->j_wa[1] * lambdas[1]) *
                                                                                (*entities)[collision->firstID]->rigidbody.invMomentOfInertia;
                                (*entities)[collision->secondID]->rigidbody.velocity += collision->collisionNormal * (lambdas[0] + lambdas[1]) * 
                                                                                (*entities)[collision->secondID]->rigidbody.invMass;
                                (*entities)[collision->secondID]->rigidbody.angularVelocity += (collision->j_wb[0] * lambdas[0] + collision->j_wb[1] * lambdas[1]) *
                                                                                (*entities)[collision->secondID]->rigidbody.invMomentOfInertia;
                            }
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
    
    prevTots.clear();
    for(int pos = 0; pos < collisionManifolds.size(); pos++)
    {
        warmStarts prev;
        prev.firstID = collisionManifolds[pos].firstID;
        prev.secondID = collisionManifolds[pos].secondID;
        prev.tot[1] = -1;
        prev.totTangent[1] = -1;
        for(int i = 0; i < collisionManifolds[pos].numContacts; i++)
        {
            prev.tot[i] = collisionManifolds[pos].tot[i];
            prev.totTangent[i] = collisionManifolds[pos].totTangent[i];
        }
        prev.numContacts = collisionManifolds[pos].numContacts;
        prevTots.push_back(prev);
    }
    std::cout << std::endl;
}