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

entity::entity(const char* l, glm::vec2 p, glm::vec2 s, float r, std::unordered_map<int, entity*>* e, int* counter, std::fstream* dat)
{
    data = dat;
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
    
    DataLoader::data << id << " p" << position.x << "," << position.y <<
                                " s" << scale.x << "," << scale.y  <<
                                " r" << rotation << std::endl;
    guiSave = false;
}
void entity::addPolygon(glm::vec2 p, glm::vec2 s, float r)
{
    polygonInstance = polygon(this, p, s, r);
    polygonInstance.basePosition = &position;
    polygonInstance.baseScale = &scale;
    polygonInstance.baseRotation = &rotation;
    contain[0] = true;
}
void entity::addPolygonCollider(spatialHashGrid* spg, glm::vec2 p, glm::vec2 s, float r)
{
    polygonColliderInstance = polygonCollider(spg, this, p, s, r);
    polygonColliderInstance.basePosition = &position;
    polygonColliderInstance.baseScale = &scale;
    polygonColliderInstance.baseRotation = &rotation;
    polygonColliderInstance.id = id;
    contain[1] = true;
}
void entity::setPosition(float x, float y)
{
    previousPos = glm::vec2(position.x, position.y);
    position = glm::vec2(x, y);
    if(contain[1]) polygonColliderInstance.updatePoints();
}
void entity::setScale(float x, float y)
{
    scale = glm::vec2(x, y);
    if(contain[1])
    {
        polygonColliderInstance.updatePoints();
        polygonColliderInstance.updateFurthestPoint();
    }
}
void entity::setRotation(float degrees)
{
    rotation = degrees;
    if(contain[1]) polygonColliderInstance.updatePoints();
}