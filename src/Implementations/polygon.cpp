#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <math.h>    
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <entity.h>
#include "Objects/polygon.h"

polygon::polygon(){}
polygon::polygon(entity* b, glm::vec2 p, glm::vec2 s, float r, glm::vec3 col, int l)
{
    initialized = false;
    scaleOffset = s;
    rotationOffset = r;
    positionOffset = p;
    startPositionOffset = p;
    startRotationOffset = r;
    startScaleOffset = s;
    startColor = col;
    startLayer = l;
    color = col;
    layer = l;
    polygonVAO = 0;
    polygonVBO = 0;
    base = b;
}
void polygon::initPolygon(int vertexCount, float* p, int indexCount, int* ind)
{
    numVertices = vertexCount;
    vertices.reserve(vertexCount * 2);
    indices.reserve(indexCount);
    memcpy(&vertices[0], p, vertexCount * 2 * sizeof(float));
    memcpy(&indices[0], ind, indexCount * sizeof(float));
    numIndices = indexCount;
    normalizePoints();
    initialized = true;
}
void polygon::initRectangle()
{
    polygonVAO = (*shared).rectVAO;
    numIndices = 6;
    numVertices = 4;
    initialized = true;
}
void polygon::render()
{
    renderPolygon();
}
void polygon::setPositionOffset(float x, float y)
{
    positionOffset = glm::vec3(x, y, 0);
}
void polygon::setScaleOffset(float x, float y)
{
    scaleOffset = glm::vec2(x, y);
}
void polygon::setColor(glm::vec3 col)
{
    color = col;
}
void polygon::setLayer(int l)
{
    layer = l;
    if(layer < 0) layer = 0;
}
void polygon::setRotationOffset(float degrees)
{
    rotationOffset = degrees;
}
void polygon::renderPolygon()
{
    GLint currentShader = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);   
    if(polygonVAO == 0)
    {
        initVAO();
    }
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(*basePosition, layer));
    model = glm::rotate(model, glm::radians(*baseRotation), glm::vec3(0, 0, -1));
    model = glm::translate(model, glm::vec3(positionOffset, 0));
    model = glm::rotate(model, glm::radians(rotationOffset), glm::vec3(0, 0, -1));
    model = glm::scale(model, glm::vec3(*baseScale * scaleOffset, 0));
    int modelLoc = glGetUniformLocation(currentShader, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    int colorLoc = glGetUniformLocation(currentShader, "col");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(polygonVAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);             
}
void polygon::initVAO()
{
    glGenVertexArrays(1, &polygonVAO);
    glGenBuffers(1, &polygonVBO);
    glBindVertexArray(polygonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 2 * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(int), &indices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  
    glBindVertexArray(0); 
}
void polygon::bufferNewData()
{
    glBindVertexArray(polygonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 3 * sizeof(float), &vertices[0]);
    glBindVertexArray(0);
}
void polygon::normalizePoints()
{
    glm::vec2 centroid(0, 0);
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        centroid.x += vertices[i];
        centroid.y += vertices[i + 1];
    }
    centroid.x /= numVertices;
    centroid.y /= numVertices;
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        vertices[i] -= centroid.x;
        vertices[i + 1] -= centroid.y;
    }
    float area = 0;
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        int nextX = i + 2;
        int nextY = i + 3;
        if(nextX >= numVertices * 2)
        { 
            nextX = 0;
            nextY = 1;
        }
        area += vertices[i] * vertices[nextY] - vertices[nextX] * vertices[i + 1];
    }
    area /= 2;
    float scale = 1 / std::sqrt(area);
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        vertices[i] *= scale;;
        vertices[i + 1] *= scale;
    }
}