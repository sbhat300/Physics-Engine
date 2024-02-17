#ifndef RECTANGLECOLLIDER_H
#define RECTANGLECOLLIDER_H
#include <Objects/rectangle.h>
#include <iostream>
#include <map>
#include <functional>
#include <vector>
#include <algorithm>  

class rectangleCollider : public rectangle
{
    public:
        int id;
        float* deltaTime;
        bool collide;
        std::map<int, rectangleCollider*>* collidables;
        glm::vec2 collider;
        std::function<void(int, int, glm::vec3, float)> collisionCallback;
        bool collisionCheck;

        rectangleCollider(float h = 1, float w = 1, float r = 0, glm::vec3 s = glm::vec3(1, 1, 1), glm::vec3 p = glm::vec3(0, 0, 0), float* dt = 0, int* counter = 0, 
                            std::map<int, rectangleCollider*>* cs = 0) : rectangle(h, w, r, s, p)
        {
            deltaTime = dt;
            collidables = cs;
            collider.x = w * s.x;
            collider.y = h * s.y;
            id = *counter;
            *counter += 1;
            (*cs)[id] = this;
            collide = true;
            collisionCheck = false;

        }
        void updateCollider()
        {
            if(collide) checkCollisions();
        }
        void configureCollider(float w, float h)
        {
            collider.x = w;
            collider.y = h;
        }
        void setCollisionCallback(std::function<void(int, int, glm::vec3, float)> cb)
        {
            collisionCallback = cb;
            collisionCheck = true;
        }
    private:
        void checkCollisions()
        {
            if(!collisionCallback) return;
            for(auto i = (*collidables).begin(); i != (*collidables).end(); i++)
            {
                if(i->second->id == id) continue;
                float minOverlap = FLT_MAX;
                glm::vec3 smallestAxis;
                rectangleCollider test = *(i->second);
                std::vector<glm::vec3> axes(vertices + test.vertices);
                for(int i = 0; i < vertices; i++)
                {
                    glm::vec3 v1(points[i].x, points[i].y, points[i].z);
                    int next = i + 1;
                    if(next >= vertices) next = 0;
                    glm::vec3 v2(points[next].x, points[next].y, points[next].z);
                    glm::vec3 normal = v1 - v2;
                    normal = glm::normalize(glm::vec3(-normal.y, normal.x, 0));
                    axes[i] = normal;
                }
                for(int i = 0; i < test.vertices; i++)
                {
                    glm::vec3 v1(test.points[i].x, test.points[i].y, test.points[i].z);
                    int next = i + 1;
                    if(next >= test.vertices) next = 0;
                    glm::vec3 v2(test.points[next].x, test.points[next].y, test.points[next].z);
                    glm::vec3 normal = v1 - v2;
                    normal = glm::normalize(glm::vec3(-normal.y, normal.x, 0));
                    axes[vertices + i] = normal;
                }
                int notColliding = false;
                for(int i = 0; i < axes.size(); i++)
                {
                    project(&axes[i], points, vertices);
                    float one[2] = {minMax[0], minMax[1]};
                    project(&axes[i], test.points, test.vertices);
                    float two[2] = {minMax[0], minMax[1]};
                    float right = two[1] - one[0];
                    float left = one[1] - two[0];
                    if(left < 0 || right < 0)
                    {
                        notColliding = true;
                        break;
                    }
                    float overlap;
                    if(right < left) overlap = right;
                    else overlap = left;
                    if(overlap < std::abs(minOverlap))
                    {
                        minOverlap = overlap;
                        smallestAxis = axes[i];
                        if(right >= left) smallestAxis *= -1;
                    }
                }
                if(notColliding) continue;
                int dot = std::abs(glm::dot(smallestAxis, glm::vec3(0, 1, 0)));
                collisionCallback(id, test.id, smallestAxis, minOverlap);
            }
        }
        float minMax[2];
        void project(glm::vec3* axis, glm::vec3* vertices, int numVertices)
        {
            minMax[0] = glm::dot(*axis, *(vertices + 0));
            minMax[1] = minMax[0];
            for(int i = 1; i < numVertices; i++)
            {
                float projected = glm::dot(*axis, *(vertices + i));
                if(projected < minMax[0]) minMax[0] = projected;
                if(projected > minMax[1]) minMax[1] = projected;
            }
        }
};
#endif