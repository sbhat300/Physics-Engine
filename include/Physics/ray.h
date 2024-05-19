#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>
#include <map>
#include <Physics/rayData.h>
#include <vector>
#include <Objects/point.h>

class ray
{
    public:
        glm::vec2 origin;
        glm::vec2 direction;
        glm::vec3 color;
        float length;
        float layer;
        std::map<int, entity*>* entities;

        ray(glm::vec2 o = glm::vec2(0, 0), glm::vec2 d = glm::vec2(0, 1), float l = 0, std::map<int, entity*>* e = 0);
        void render();
        void setColor(glm::vec3 col);
        void setLayer(int l);
        void bufferNewData();
        float getLineIntersection(glm::vec2* point1, glm::vec2* point2);
        std::vector<rayData> getCollisions();
        std::pair<bool, rayData> getFirstCollision();
    private:
        unsigned int rayVAO, rayVBO;
        void drawRay();
        float crossVectors(glm::vec2* point1, glm::vec2* point2);
};

#endif