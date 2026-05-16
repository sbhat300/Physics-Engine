#include "Physics/polygonRigidbody.h"
#include <entity.h>
#include <Engine/setup.h>
#include <mathFuncs.h>

polygonRigidbody::polygonRigidbody(){}
polygonRigidbody::polygonRigidbody(float m, float moi, float r, float u, entity* b)
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
    mu = u;
    polygonMomentOfInertia = false;
    circleMomentOfInertia = false;
    startMass = mass;
    startMOI = momentOfInertia;
    startRestitution = restitution;
    startMu = u;
}
void polygonRigidbody::updateVel()
{
    velocity += force * invMass * setup::fixedDeltaTime + impulse * invMass;
    angularVelocity += torque * invMomentOfInertia * setup::fixedDeltaTime + angularImpulse * invMomentOfInertia;
    force = glm::vec2(0.0f, 0.0f);
    impulse = glm::vec2(0.0f, 0.0f);
    torque = 0;
    angularImpulse = 0;
}
void polygonRigidbody::updatePos()
{
    velocity *= setup::linearDamping;
    angularVelocity *= setup::angularDamping;
    glm::vec2 newPos = base->position + velocity * setup::fixedDeltaTime;
    float newRot = base->rotation + angularVelocity * setup::fixedDeltaTime;
    (*base).setPosition(newPos.x, newPos.y);
    (*base).setRotation(newRot);
}
void polygonRigidbody::setPolygonMomentOfInertia()
{
    polygonMomentOfInertia = true;
    if(mass == 0) return;

    std::vector<float>& points = base->collider.vertices;
    int numVertices = base->collider.numVertices;
    
    float sumX = 0.0f;
    float sumY = 0.0f;
    float localArea = 0.0f;

    for(int i = 0; i < numVertices * 2; i += 2)
    {
        int nextX = (i + 2) % (numVertices * 2);
        int nextY = (i + 3) % (numVertices * 2);

        float x0 = points[i];
        float y0 = points[i + 1];
        float x1 = points[nextX];
        float y1 = points[nextY];

        float cross = (x0 * y1) - (x1 * y0);
        localArea += cross;

        sumX += cross * (x0 * x0 + x0 * x1 + x1 * x1);
        sumY += cross * (y0 * y0 + y0 * y1 + y1 * y1);
    }
    localArea /= 2.0f;

    float factor = mass / (12.0f * localArea);
    float ix = sumY * factor;
    float iy = sumX * factor;

    glm::vec2 scale = (*base).scale * (*base).collider.scaleOffset;
    
    momentOfInertia = std::abs(ix * scale.y * scale.y + iy * scale.x * scale.x);
    invMomentOfInertia = momentOfInertia == 0 ? 0 : 1.0f / momentOfInertia;
}
void polygonRigidbody::setCircleMomentOfInertia()
{
    circleMomentOfInertia = true;
    if(mass == 0)
    {
        momentOfInertia = 0;
        invMomentOfInertia = 0;
        return;
    }
    float r = base->collider.radius * base->scale.x * base->collider.scaleOffset.x;
    momentOfInertia = 0.5f * mass * r * r;
    invMomentOfInertia = 1.0f / momentOfInertia;
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
    glm::vec2 dist = glm::vec2(xPos , yPos) - (*base).collider.centroid;
    glm::vec2 torqueForce = glm::vec2(forceX, forceY);
    torque += dist.x * torqueForce.y - dist.y * torqueForce.x;
}
void polygonRigidbody::addTorque(float amt)
{
    torque += amt;
}
void polygonRigidbody::addAngularImpulse(float impulseX, float impulseY, float xPos, float yPos)
{
    glm::vec2 dist = glm::vec2(xPos , yPos) - (*base).collider.centroid;
    glm::vec2 impulse = glm::vec2(impulseX, impulseY);
    angularImpulse += mathFuncs::cross(dist, impulse);
}
void polygonRigidbody::addAngularImpulse(float amt)
{
    angularImpulse += amt;
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
void polygonRigidbody::gravity(float amount)
{
    addForce(0, -amount * mass);
}
void polygonRigidbody::applyImpulses()
{
    velocity += impulse * invMass;
    angularVelocity += angularImpulse * invMomentOfInertia;
    impulse = glm::vec2(0.0f, 0.0f);
    angularImpulse = 0;
}
void polygonRigidbody::setMass(float m)
{
    mass = m < 0 ? 0 : m;
    invMass = mass == 0 ? 0 : 1 / mass;
    if(polygonMomentOfInertia) setPolygonMomentOfInertia();
    else if(circleMomentOfInertia) setCircleMomentOfInertia();
}
void polygonRigidbody::setMomentOfInertia(float i)
{
    momentOfInertia = i < 0 ? 0 : i;
    invMomentOfInertia = momentOfInertia == 0 ? 0 : 1 / momentOfInertia;
}
void polygonRigidbody::setRestitution(float r)
{
    restitution = mathFuncs::clamp(0.0f, 1.0f, r);
}
void polygonRigidbody::setFriction(float u)
{
    mu = std::max(0.0f, u);
}