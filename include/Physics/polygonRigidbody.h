#ifndef POLYGONRIGIDBODY_H
#define POLYGONRIGIDBODY_H

class entity;
#include <glm/glm.hpp>

class polygonRigidbody
{
    public:
        glm::vec2 force, velocity, impulse;
        float mass, invMass, momentOfInertia;
        float angularImpulse, angularVelocity, torque;
        float restitution;
        entity* base;
        polygonRigidbody(float m, float moi, float r);
        void addForce(float x, float y);
        void addTorque(float amt);
        void addImpulse(float x, float y);
        void addAngularImpulse(float amt);
    private:
};
#endif