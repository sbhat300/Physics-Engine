#ifndef POLYGONRIGIDBODY_H
#define POLYGONRIGIDBODY_H

class entity;
#include <glm/glm.hpp>

class polygonRigidbody
{
    public:
        glm::vec2 force, velocity, impulse;
        float mass, invMass, momentOfInertia, invMomentOfInertia;
        float angularImpulse, angularVelocity, torque;
        float restitution;
        entity* base;
        polygonRigidbody();
        polygonRigidbody(float m, float moi, float r, entity* b);
        void setRectangleMomentOfInertia();
        void addForce(float x, float y);
        void addTorque(float forceX, float forceY, float xPos, float yPos);
        void addTorque(float amt);
        void addForceAtPoint(float forceX, float forceY, float xPos, float yPos);
        void addImpulse(float x, float y);
        void addAngularImpulse(float impulseX, float impulseY, float xPos, float yPos);
        void addAngularImpulse(float amt);
        void addImpulseAtPoint(float impulseX, float impulseY, float xPos, float yPos);
        void gravity(float amount);
        void update();
    private:
};
#endif