#include "sharedData.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

void sharedData::createCirclePoints()
{
    circleVertices[0] = 0.0f;
    circleVertices[1] = 0.0f;
    circleVertices[2] = 0.5f;
    circleVertices[3] = 0.5f;

    for(int i = 1; i < circleNumVertices; i++)
    {
        int vIndex = i * 4;
        float theta = 2 * M_PI * (float)(i - 1) / (circleNumVertices - 1);
        float x = cos(theta);
        float y = sin(theta);
        circleVertices[vIndex] = x;
        circleVertices[vIndex + 1] = y;
        circleVertices[vIndex + 2] = (x + 1) / 2.0f;
        circleVertices[vIndex + 3] = (y + 1) / 2.0f;
    }

    for(int i = 0; i < circleNumVertices - 1; i++)
    {
        int iIndex = i * 3;
        circleIndices[iIndex] = 0;
        circleIndices[iIndex + 1] = i + 1; 
        circleIndices[iIndex + 2] = (i + 1 == circleNumVertices - 1) ? 1 : i + 2;
    }
}

void sharedData::initVAOs()
{
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndices), rectIndices, GL_STATIC_DRAW);
    glBindVertexArray(0); 

    createCirclePoints();
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    unsigned int circleEBO;
    glGenBuffers(1, &circleEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circleIndices), circleIndices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}