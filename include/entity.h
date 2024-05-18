#ifndef ENTITY_H
#define ENTITY_H

#include <Objects/polygon.h>
// #include <Physics/polygonCollider.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <map>

class entity
{
    public:
        //0 = polygon, 1 = polygonCollider
        bool contain[2];
        polygon polygonInstance;
        // polygonCollider polygonColliderInstance;
        glm::vec2 position, scale;
        glm::vec2 previousPos;
        float rotation;   
        int id;
        std::map<int, entity*>* entities;

        entity(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, std::map<int, entity*>* e = 0, int* counter = 0)
        {
            id = *counter;
            position = p;
            scale = s;
            rotation = r;
            entities = e;
            (*e)[id] = this;
            *counter += 1;
        }
        void addPolygon(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0)
        {
            polygonInstance = polygon(p, s, r);
            polygonInstance.basePosition = &position;
            polygonInstance.baseScale = &scale;
            polygonInstance.baseRotation = &rotation;
            contain[0] = true;
        }
        void addPolygonCollider(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0)
        {
            // polygonColliderInstance = polygonCollider(p, s, r, entities);
            // polygonColliderInstance.basePosition = &position;
            // polygonColliderInstance.baseScale = &scale;
            // polygonColliderInstance.baseRotation = &rotation;
            // polygonColliderInstance.id = id;
            // contain[1] = true;
        }
        void setPosition(float x, float y)
        {
            previousPos = glm::vec2(position.x, position.y);
            position = glm::vec2(x, y);
            // if(contain[1]) polygonColliderInstance.updatePoints();
        }
        void setScale(float x, float y)
        {
            scale = glm::vec2(x, y);
            // if(contain[1]) polygonColliderInstance.updatePoints();
        }
        void setRotation(float degrees)
        {
            rotation = degrees;
            // if(contain[1]) polygonColliderInstance.updatePoints();
        }
    private:

};

#endif