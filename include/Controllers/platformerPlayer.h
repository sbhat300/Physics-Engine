#ifndef PLATFORMERPLAYER_H
#define PLATFORMERPLAYER_H
#include <Objects/rectangle.h>
#include <Physics/rectangleCollider.h>
#include <Physics/rectangleRigidbody.h>
#include <iostream>
#include <map>
#include <functional>

class platformerPlayer : public rectangleRigidbody
{
    public:
        float speed;
        float jumpForce;
        bool grounded;

        platformerPlayer(float h = 1, float w = 1, float r = 0, glm::vec3 s = glm::vec3(1, 1, 1), glm::vec3 p = glm::vec3(0, 0, 0), 
        float* dt = 0, int* counter = 0, std::map<int, rectangleCollider*>* cs = 0, float m = 0, std::map<int, rectangleRigidbody*>* rbs = 0, float res = 0, float sp = 0, float jf = 0) 
        : rectangleRigidbody(h, w, r, s, p, dt, counter, cs, m, rbs, res)
        {
            speed = sp;
            jumpForce = jf;
            grounded = false;
        }
    private:
};
#endif