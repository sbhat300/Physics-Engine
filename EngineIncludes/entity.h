#ifndef ENTITY_H
#define ENTITY_H

#include <Objects/polygon.h>
#include <Physics/polygonCollider.h>
#include <Physics/polygonRigidbody.h>
#include <glm/glm.hpp>
#include <map>
#include <fstream>
#include <Engine/sharedData.h>
#include <Scripting/baseScript.h>
#include <unordered_set>

const int ATTRS = 4;
class entity
{
    public:
        //0 = polygon, 1 = polygonCollider, 2 = polygonRigidbody 3 = script
        bool contain[ATTRS];
        polygon polygonInstance;
        polygonCollider collider;
        polygonRigidbody rigidbody;
        std::vector<baseScript*> scripts;
        glm::vec2 position, scale, startingPosition, startingScale;
        glm::vec2 previousPos;
        float rotation, startingRotation;   
        unsigned int id;
        unsigned int* count;
        std::unordered_map<unsigned int, entity*>* entities;
        const char* label;
        std::fstream* data;
        bool guiSave;
        sharedData* shared;
        std::unordered_set<std::string> tags;
        entity(const char* l);
        entity(const char* l, glm::vec2 p, glm::vec2 s, float r);
        ~entity();
        void addPolygon(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, glm::vec3 col = glm::vec3(1, 1, 1), int layer = 1);
        void addPolygonCollider(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0);
        void addPolygonRigidbody(float mass, float momentOfInertia, float restitution, float mu);
        template <typename T>
        void addScript();
        void setPosition(float x, float y);
        void setScale(float x, float y);
        void setRotation(float degrees);
        void addTag(std::string tag);
        void removeTag(std::string tag);
        bool hasTag(std::string tag);
    private:

};

#include "entity.inl"
#endif