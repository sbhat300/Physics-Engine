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
#include <Scripting/baseScript.h>
#include <Engine/engine.h>

entity::entity(const char* l)
{
    label = l;
    id = engine::counter;
    position = glm::vec2(0, 0);
    startingPosition = position;
    scale = glm::vec2(0, 0);
    startingScale = glm::vec2(0, 0);
    rotation = 0;
    startingRotation = 0;
    entities = &engine::entities;
    engine::entities[id] = this;
    engine::counter += 1;
    count = &engine::counter;
    shared = &engine::shared;
    guiSave = false;
    for(int i = 0; i < ATTRS; i++) contain[i] = 0;
}
entity::entity(const char* l, glm::vec2 p, glm::vec2 s, float r)
{
    label = l;
    id = engine::counter;
    position = p;
    startingPosition = p;
    scale = s;
    startingScale = s;
    rotation = r;
    startingRotation = r;
    entities = &engine::entities;
    engine::entities[id] = this;
    engine::counter += 1;
    count = &engine::counter;
    shared = &engine::shared;
    guiSave = false;
    for(int i = 0; i < ATTRS; i++) contain[i] = 0;
}
entity::~entity()
{
    for(baseScript* s : scripts) delete s;
    scripts.clear();
    engine::entities.erase(id);
    if(contain[0]) polygonInstance.polygonTexture.deleteTexture(); //TODO: remove this when polygon stores a texture pointer
    if(contain[1]) collider.grid->remove(&collider);
}
void entity::unregister()
{
    for(baseScript* s : scripts) delete s;
    scripts.clear();
    engine::entities.erase(id);
    if(contain[0]) polygonInstance.polygonTexture.deleteTexture(); //TODO: remove this when polygon stores a texture pointer
    if(contain[1]) collider.grid->remove(&collider);
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
void entity::addPolygonCollider(glm::vec2 p, glm::vec2 s, float r)
{
    collider = polygonCollider(&engine::grid, p, s, r, this);
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
    if(contain[2] && rigidbody.rectangleMomentOfInertia)
    {
        rigidbody.setRectangleMomentOfInertia();
    }
}
void entity::setRotation(float radians)
{
    rotation = radians;
    if(contain[1]) collider.updatePoints();
}

void entity::addTag(std::string tag)
{
    tags.insert(tag);
}

void entity::removeTag(std::string tag)
{
    tags.erase(tag);
}

bool entity::hasTag(std::string tag)
{
    return tags.find(tag) != tags.end();
}