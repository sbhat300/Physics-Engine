#ifndef ENTITY_H
#define ENTITY_H

#include <Objects/polygon.h>
#include <Physics/polygonCollider.h>
#include <Physics/polygonRigidbody.h>
#include <glm/glm.hpp>
#include <map>
#include <fstream>
#include <sharedData.h>

class entity
{
    public:
        //0 = polygon, 1 = polygonCollider, 2 = polygonRigidbody
        bool contain[3];
        polygon polygonInstance;
        polygonCollider polygonColliderInstance;
        polygonRigidbody polygonRigidbodyInstance;
        glm::vec2 position, scale, startingPosition, startingScale;
        glm::vec2 previousPos;
        float rotation, startingRotation;   
        unsigned int id;
        std::unordered_map<int, entity*>* entities;
        const char* label;
        std::fstream* data;
        bool guiSave;
        sharedData* shared;
        entity(const char* l = "default", glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, std::unordered_map<int, entity*>* e = 0, unsigned int* counter = 0, sharedData* sh = 0);
        void addPolygon(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, glm::vec3 col = glm::vec3(1, 1, 1), int layer = 1);
        void addPolygonCollider(spatialHashGrid* spg = 0, glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0);
        void addPolygonRigidbody(float mass, float momentOfInertia, float restitution);
        void addPolygonRigidbody();
        void setPosition(float x, float y);
        void setScale(float x, float y);
        void setRotation(float degrees);
    private:

};

#endif