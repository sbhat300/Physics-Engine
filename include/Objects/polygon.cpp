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
#include <mathFuncs.h>

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
    shaderProgram = -1;
}
void polygon::initPrevious()
{
    prevBasePos = *basePosition;
    prevBaseRot = *baseRotation;
    prevBaseScale = *baseScale;
    prevScale = scaleOffset;
    prevRot = rotationOffset;
    prevPos = positionOffset;
}
void polygon::initPolygon(int vertexCount, float* p, int indexCount, int* ind)
{
    numVertices = vertexCount;
    vertices.resize(vertexCount * 4);
    indices.resize(indexCount);
    memcpy(&vertices[0], p, vertexCount * 4 * sizeof(float));
    memcpy(&indices[0], ind, indexCount * sizeof(float));
    numIndices = indexCount;
    normalizePoints();
    initPrevious();
    initialized = true;
}
void polygon::initRectangle()
{
    polygonVAO = (*shared).rectVAO;
    numIndices = 6;
    numVertices = 4;
    initPrevious();
    initialized = true;
}
void polygon::render(float alpha)
{
    renderPolygon(alpha);
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
void polygon::renderPolygon(float alpha)
{
    GLint currentShader = 0;
    if(shaderProgram != -1) glUseProgram(shaderProgram);
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);   
    if(polygonVAO == 0)
    {
        initVAO();
    }
    glm::mat4 model = glm::mat4(1.0f);    
    model = glm::translate(model, glm::vec3(interpolate(prevBasePos, *basePosition, alpha), layer));
    model = glm::rotate(model, mathFuncs::interpolate(prevBaseRot, *baseRotation, alpha), glm::vec3(0, 0, 1));
    model = glm::translate(model, glm::vec3(interpolate(prevPos, positionOffset, alpha), 0));
    model = glm::rotate(model, mathFuncs::interpolate(prevRot, rotationOffset, alpha), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(interpolate(prevBaseScale * prevScale, *baseScale * scaleOffset, alpha), 0));
    int modelLoc = glGetUniformLocation(currentShader, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    int colorLoc = glGetUniformLocation(currentShader, "col");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(polygonVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, polygonTexture.textureID);
    glUniform1i(glGetUniformLocation(currentShader, "tex"), 0); 

    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);             
}
void polygon::initVAO()
{
    glGenVertexArrays(1, &polygonVAO);
    glGenBuffers(1, &polygonVBO);
    glBindVertexArray(polygonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 4 * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(int), &indices[0], GL_STATIC_DRAW);
    glBindVertexArray(0); 
}
void polygon::bufferNewData()
{
    glBindVertexArray(polygonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 4 * sizeof(float), &vertices[0]);
    glBindVertexArray(0);
}
void polygon::normalizePoints()
{
    glm::vec2 centroid(0, 0);
    for(int i = 0; i < numVertices * 4; i += 4)
    {
        centroid.x += vertices[i];
        centroid.y += vertices[i + 1];
    }
    centroid.x /= numVertices;
    centroid.y /= numVertices;
    for(int i = 0; i < numVertices * 4; i += 4)
    {
        vertices[i] -= centroid.x;
        vertices[i + 1] -= centroid.y;
    }
    float area = 0;
    for(int i = 0; i < numVertices * 4; i += 4)
    {
        int nextX = i + 4;
        int nextY = i + 5;
        if(nextX >= numVertices * 4)
        { 
            nextX = 0;
            nextY = 1;
        }
        area += vertices[i] * vertices[nextY] - vertices[nextX] * vertices[i + 1];
    }
    area /= 2;
    float scale = 1 / std::sqrt(std::abs(area));
    for(int i = 0; i < numVertices * 4; i += 4)
    {
        vertices[i] *= scale;
        vertices[i + 1] *= scale;
    }
}

glm::vec2 polygon::interpolate(glm::vec2 first, glm::vec2 second, float alpha)
{
    return glm::vec2(mathFuncs::interpolate(first.x, second.x, alpha), mathFuncs::interpolate(first.y, second.y, alpha));
}

void polygon::updatePreviousState()
{
    prevBasePos = *basePosition;
    prevBaseRot = *baseRotation;
    prevBaseScale = *baseScale;
    prevScale = scaleOffset;
    prevRot = rotationOffset;
    prevPos = positionOffset;
}