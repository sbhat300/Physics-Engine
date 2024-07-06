#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>    
#include <unordered_map>
#include "Physics/ray.h"
#include <Physics/rayData.h>
#include <vector>
#include <Objects/point.h>
#include <Physics/spatialHashGrid.h>

ray::ray(glm::vec2 o, glm::vec2 d, float l, spatialHashGrid* g)
{
    origin = o;
    direction = glm::normalize(d);
    length = l;
    color = glm::vec3(1, 1, 1);
    layer = 1;
    rayVAO = 0;
    rayVBO = 0;
    grid = g;
}
void ray::render()
{
    drawRay();
}
void ray::setColor(glm::vec3 col)
{
    color = col;
}
void ray::setLayer(int l)
{
    layer = l;
}
void ray::bufferNewData()
{
    glm::vec2 firstPoint = origin;
    glm::vec2 secondPoint = origin + direction * length;
    float points[] = {firstPoint.x, firstPoint.y, layer, secondPoint.x, secondPoint.y, layer};
    glBindVertexArray(rayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(float), points);
    glBindVertexArray(0);
}
void ray::drawRay()
{
    glm::vec2 firstPoint = origin;
    glm::vec2 secondPoint;
    if(length == 0) secondPoint = origin + direction * 5.0f;
    else secondPoint = origin + direction * length;
    float points[] = {firstPoint.x, firstPoint.y, layer, secondPoint.x, secondPoint.y, layer};
    GLint currentShader = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);   
    if(rayVAO == 0)
    {
        glGenVertexArrays(1, &rayVAO);
        glGenBuffers(1, &rayVBO);
        glBindVertexArray(rayVAO);
        glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);  
        glBindVertexArray(0); 
    }
    int colorLoc = glGetUniformLocation(currentShader, "col");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(rayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);  
}
float ray::getLineIntersection(glm::vec2* point1, glm::vec2* point2)
{
    glm::vec2 originDiff = *point1 - origin;
    glm::vec2 lineSlope = *point2 - *point1;
    float slopeCross = crossVectors(&direction, &lineSlope);
    float rayTime = crossVectors(&originDiff, &lineSlope) / slopeCross;
    float top = crossVectors(&originDiff, &direction);
    float lineTime = top / slopeCross;
    if(slopeCross == 0) return -1;
    if(length == 0 && rayTime > 0 && lineTime >= 0 && lineTime <= 1) return rayTime;
    else if(rayTime > 0 && lineTime >= 0 && lineTime <= 1 && rayTime <= length) return rayTime;
    return -1;
}
float ray::crossVectors(glm::vec2* point1, glm::vec2* point2)
{
    return (*point1).x * (*point2).y - (*point1).y * (*point2).x;
}
std::vector<rayData> ray::getCollisions()
{
    std::vector<polygonCollider*> possibleCollisions = (*grid).getNearbyRay(this);
    std::vector<rayData> output;
    for(auto i = possibleCollisions.begin(); i != possibleCollisions.end(); i++)
    {
        polygonCollider* current = *i;
        float t = FLT_MAX;
        bool collide = false;
        glm::vec2 point = origin;
        glm::vec2 normal = direction;
        for(int j = 0; j < (*current).numVertices; j++)
        {
            int jNext = j + 1;
            if(jNext == (*current).numVertices) jNext = 0;
            float sideT = getLineIntersection(&(*current).points[j], &(*current).points[jNext]);
            if(sideT != -1)
            {
                collide = true;
                if(sideT < t)
                {
                    t = sideT;
                    point = origin + direction * t;
                    glm::vec2 edge = (*current).points[jNext] - (*current).points[j];
                    normal = glm::normalize(glm::vec2(edge.y, -edge.x));
                }
            }
        }
        if(collide)
        {
            rayData out;
            out.collisionNormal = normal;
            out.collisionPoint = point;
            out.id = (*current).id;
            output.push_back(out);
        }
    }
    return output;
}
std::pair<bool, rayData> ray::getFirstCollision()
{
    return (*grid).getNearbyRaySingle(this);
}
std::pair<bool, rayData> ray::getFirstCollision(std::vector<polygonCollider*>* tests)
{
    float t = FLT_MAX;
    glm::vec2 point = origin;
    glm::vec2 normal = direction;
    bool collide = false;
    int id = -1;
    for(auto i = (*tests).begin(); i != (*tests).end(); i++)
    {
        polygonCollider* current = *i;
        for(int j = 0; j < (*current).numVertices; j++)
        {
            int jNext = j + 1;
            if(jNext == (*current).numVertices) jNext = 0;
            float sideT = getLineIntersection(&(*current).points[j], &(*current).points[jNext]);
            if(sideT != -1)
            {
                collide = true;
                if(sideT < t)
                {
                    t = sideT;
                    point = origin + direction * t;
                    glm::vec2 edge = (*current).points[jNext] - (*current).points[j];
                    normal = glm::normalize(glm::vec2(edge.y, -edge.x));
                    id = (*current).id;
                }
            }
        }
    }
    std::pair<bool, rayData> outPair;
    rayData out;
    outPair.first = false;
    if(collide)
    {
        out.collisionNormal = normal;
        out.collisionPoint = point;
        out.id = id;
        outPair.first = true;
        outPair.second = out;
    }
    return outPair;
}