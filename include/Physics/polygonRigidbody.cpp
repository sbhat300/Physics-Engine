#include "Physics/polygonRigidbody.h"
#include <entity.h>
#include <setup.h>

polygonRigidbody::polygonRigidbody(){}
polygonRigidbody::polygonRigidbody(float m, float moi, float r, entity* b)
{
    mass = m;
    momentOfInertia = moi;
    restitution = r;
    invMass = mass == 0 ? 0 : 1 / mass;
    invMomentOfInertia = momentOfInertia == 0 ? 0 : 1 / momentOfInertia;
    force = glm::vec2(0, 0);
    impulse = glm::vec2(0, 0);
    velocity = glm::vec2(0, 0);
    base = b;
    angularVelocity = 0;
    torque = 0;
    angularImpulse = 0;
}
void polygonRigidbody::update()
{
    velocity += force * invMass * setup::fixedDeltaTime + impulse * invMass;
    angularVelocity += -torque * invMomentOfInertia * setup::fixedDeltaTime + -angularImpulse * invMomentOfInertia;
    glm::vec2 newPos = (*base).position + velocity * setup::fixedDeltaTime;
    float newRot = (*base).rotation + angularVelocity * setup::fixedDeltaTime;
    (*base).setPosition(newPos.x, newPos.y);
    (*base).setRotation(newRot);
    force = glm::vec2(0.0f, 0.0f);
    impulse = glm::vec2(0.0f, 0.0f);
    torque = 0;
    angularImpulse = 0;
}
void polygonRigidbody::setRectangleMomentOfInertia()
{
    if(mass == 0) return;
    glm::vec2 scale = (*base).scale * (*base).polygonColliderInstance.scaleOffset;
    momentOfInertia = mass * (scale.x * scale.x + scale.y + scale.y) / 12.0f;
    invMomentOfInertia = 1 / momentOfInertia;
}
void polygonRigidbody::addForce(float x, float y)
{
    force += glm::vec2(x, y);
}
void polygonRigidbody::addImpulse(float x, float y)
{
    impulse += glm::vec2(x, y);
}
void polygonRigidbody::addTorque(float forceX, float forceY, float xPos, float yPos)
{
    glm::vec2 dist = glm::vec2(xPos , yPos) - (*base).polygonColliderInstance.centroid;
    glm::vec2 torqueForce = glm::vec2(forceX, forceY);
    torque += dist.x * torqueForce.y - dist.y * torqueForce.x;
}
void polygonRigidbody::addAngularImpulse(float impulseX, float impulseY, float xPos, float yPos)
{
    glm::vec2 dist = glm::vec2(xPos , yPos) - (*base).polygonColliderInstance.centroid;
    glm::vec2 impulse = glm::vec2(impulseX, impulseY);
    angularImpulse += dist.x * impulse.y - dist.y * impulse.x;
}
void polygonRigidbody::addForceAtPoint(float forceX, float forceY, float xPos, float yPos)
{
    addTorque(forceX, forceY, xPos, yPos);
    addForce(forceX, forceY);
}
void polygonRigidbody::addImpulseAtPoint(float impulseX, float impulseY, float xPos, float yPos)
{
    addAngularImpulse(impulseX, impulseY, xPos, yPos);
    addImpulse(impulseX, impulseY);
}