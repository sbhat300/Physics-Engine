#include <Objects/polygon.h>
#include <Physics/polygonCollider.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <unordered_map>
#include <Physics/spatialHashGrid.h>
#include <limits>
#include "entity.h"
#include <string>
#include <FileLoader/objDataLoader.h>

entity::entity(const char* l, glm::vec2 p, glm::vec2 s, float r, std::unordered_map<unsigned int, entity*>* e, unsigned int* counter, sharedData* sh)
{
    label = l;
    id = *counter;
    position = p;
    startingPosition = p;
    scale = s;
    startingScale = s;
    rotation = r;
    startingRotation = r;
    entities = e;
    (*e)[id] = this;
    *counter += 1;
    count = counter;
    shared = sh;
    guiSave = false;
}
void entity::addPolygon(glm::vec2 p, glm::vec2 s, float r, glm::vec3 col, int layer)
{
    polygonInstance = polygon(this, p, s, r, col, layer);
    polygonInstance.basePosition = &position;
    polygonInstance.baseScale = &scale;
    polygonInstance.baseRotation = &rotation;
    polygonInstance.shared = shared;
    contain[0] = true;
}
void entity::addPolygonCollider(spatialHashGrid* spg, glm::vec2 p, glm::vec2 s, float r)
{
    collider = polygonCollider(spg, p, s, r, this);
    collider.basePosition = &position;
    collider.baseScale = &scale;
    collider.baseRotation = &rotation;
    collider.id = id;
    contain[1] = true;
}
void entity::addPolygonRigidbody(float mass, float momentOfInertia, float restitution, float mu)
{
    rigidbody = polygonRigidbody(mass, momentOfInertia, restitution, mu, this);
    contain[2] = true;
}
void entity::setPosition(float x, float y)
{
    previousPos = glm::vec2(position.x, position.y);
    position = glm::vec2(x, y);
    if(contain[1]) collider.updatePoints();
}
void entity::setScale(float x, float y)
{
    scale = glm::vec2(x, y);
    if(contain[1])
    {
        collider.updatePoints();
        collider.updateFurthestPoint();
    }
}
void entity::setRotation(float degrees)
{
    rotation = degrees;
    if(contain[1]) collider.updatePoints();
}