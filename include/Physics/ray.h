#ifndef RAY_H
#define RAY_H

class spatialHashGrid;
#include <glm/glm.hpp>
#include <unordered_map>
#include <Physics/rayData.h>
#include <vector>
#include <Objects/point.h>
#include <Physics/polygonCollider.h>

class ray
{
    public:
        glm::vec2 origin;
        glm::vec2 direction;
        glm::vec3 color;
        float length;
        float layer;
        spatialHashGrid* grid;

        ray(glm::vec2 o = glm::vec2(0, 0), glm::vec2 d = glm::vec2(0, 1), float l = 0, spatialHashGrid* g = 0);
        void render();
        void setColor(glm::vec3 col);
        void setLayer(int l);
        void bufferNewData();
        float getLineIntersection(glm::vec2* point1, glm::vec2* point2);
        std::vector<rayData> getCollisions();
        std::pair<bool, rayData> getFirstCollision();
        std::pair<bool, rayData> getFirstCollision(std::vector<polygonCollider*>* tests);
    private:
        unsigned int rayVAO, rayVBO;
        void drawRay();
        float crossVectors(glm::vec2* point1, glm::vec2* point2);
};

#endif