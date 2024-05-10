#ifndef RECTANGLERIGIDBODY_H
#define RECTANGLERIGIDBODY_H
#include <Objects/rectangle.h>
#include <Physics/rectangleCollider.h>
#include <iostream>
#include <map>
#include <functional>

class rectangleRigidbody : public rectangleCollider
{
    public:
        glm::vec3 force, velocity, impulse, acceleration;
        float mass;
        float invMass;
        float staticFriction, dynamicFriction;
        std::map<int, rectangleRigidbody*>* collidables;
        float restitution;
        float* iterations;
        float damping;

        rectangleRigidbody(float h = 1, float w = 1, float r = 0, glm::vec3 s = glm::vec3(1, 1, 1), glm::vec3 p = glm::vec3(0, 0, 0), 
        float* dt = 0, int* counter = 0, std::map<int, rectangleCollider*>* cs = 0, float m = 0, std::map<int, rectangleRigidbody*>* rbs = 0, float res = 0, float muS = 0, float muK = 0, float* iter = 0) 
        : rectangleCollider(h, w, r, s, p, dt, counter, cs)
        {
            mass = m;
            force = glm::vec3(0, 0, 0);
            velocity = glm::vec3(0, 0, 0);
            impulse = glm::vec3(0, 0, 0);
            collidables = rbs;
            (*rbs)[id] = this;
            restitution = res;
            if(mass == 0) invMass = 0;
            else invMass = 1 / mass;
            staticFriction = muS;
            dynamicFriction = muK;
            iterations = iter;
            damping = 1.0f;
        }
        void updateRigidbody()
        {
            float time = *deltaTime / *iterations;
            previousPos = glm::vec3(position.x, position.y, 0);
            acceleration = force * invMass;
            velocity = velocity * damping + acceleration * time + impulse * invMass;
            position += velocity * time;
            force = glm::vec3(0, 0, 0);
            impulse = glm::vec3(0, 0, 0);
            updatePoints();
        }
        void addForce(float x, float y)
        {
            force += glm::vec3(x, y, 0);
        }
        void addImpulse(float x, float y)
        {
            impulse += glm::vec3(x, y, 0);
        }
        void applyImpulses()
        {
            velocity += impulse * invMass;
            impulse = glm::vec3(0, 0, 0);
        }
    private:
};
#endif