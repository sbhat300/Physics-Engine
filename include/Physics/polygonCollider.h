#ifndef POLYGONCOLLIDER_H
#define POLYGONCOLLIDER_H
class entity;
#include <Objects/point.h>
#include <iostream>
#include <map>
#include <functional>
#include <vector>
#include <algorithm>  

class polygonCollider
{
    public:
        struct edge {
            glm::vec2 v1;
            glm::vec2 v2;
            glm::vec2 vMax;
        };
        struct clippedPoints{
            glm::vec2 points[2];
            int numPoints;
        };

        int id;
        bool collide;
        std::map<int, entity*>* entities;
        std::function<void(int, int, glm::vec2, float, int, glm::vec2, glm::vec2)> collisionCallback;
        int debugShaderProgram;
        std::vector<glm::vec2> points;
        std::vector<float> vertices;
        int numVertices;
        float rotationOffset;
        glm::vec2 scaleOffset, positionOffset;
        glm::vec2* basePosition;
        glm::vec2* baseScale;
        float* baseRotation;
        bool initialized;

        polygonCollider(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, 
                            std::map<int, entity*>* cs = 0);
        void updateCollider();
        void setCollisionCallback(std::function<void(int, int, glm::vec2, float, int, glm::vec2, glm::vec2)> cb);
        bool pointInPolygon(glm::vec2 point);
        void initPolygon(int vertexCount, float* p);
        void initRectangle();
        void updatePoints();
        void normalizePoints();
        void setPositionOffset(float x, float y);
        void setScaleOffset(float x, float y);
        void setRotationOffset(float r);
    private:
        point debugPoint;
        float minMax[2];
        
        void checkCollisions();
        void project(glm::vec2* axis, std::vector<glm::vec2> &vertices, int numVertices);
        edge findEdge(std::vector<glm::vec2> &points, int numVertices, glm::vec2 collisionNormal);
        clippedPoints clipPoints(glm::vec2* point1, glm::vec2* point2, glm::vec2* referenceEdge, float* offset);

};
#endif