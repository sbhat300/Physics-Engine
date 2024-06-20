#ifndef ENTITY_H
#define ENTITY_H

#include <Objects/polygon.h>
#include <Physics/polygonCollider.h>
#include <glm/glm.hpp>
#include <map>

class entity
{
    public:
        //0 = polygon, 1 = polygonCollider
        bool contain[2];
        polygon polygonInstance;
        polygonCollider polygonColliderInstance;
        glm::vec2 position, scale;
        glm::vec2 previousPos;
        float rotation;   
        int id;
        std::unordered_map<int, entity*>* entities;

        entity(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, std::unordered_map<int, entity*>* e = 0, int* counter = 0);
        void addPolygon(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0);
        void addPolygonCollider(spatialHashGrid* spg = 0, glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0);
        void setPosition(float x, float y);
        void setScale(float x, float y);
        void setRotation(float degrees);
    private:

};

#endif