#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <math.h>    
#include <glm/gtx/string_cast.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>

class polygon
{
    public:
        glm::vec2 scaleOffset, positionOffset;
        float rotationOffset;
        glm::vec2* basePosition;
        glm::vec2* baseScale;
        float* baseRotation;
        bool initialized;
        std::vector<float> vertices;
        std::vector<int> indices;
        int numVertices;
        int numIndices;
        glm::vec3 color;
        float layer;
        polygon(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0)
        {
            initialized = false;
            scaleOffset = s;
            rotationOffset = r;
            positionOffset = p;
            color = glm::vec3(1,1,1);
            layer = 1;
            
        }
        void initPolygon(int vertexCount, float* p, int indexCount, int* ind)
        {
            numVertices = vertexCount;
            vertices.reserve(vertexCount * 3);
            indices.reserve(indexCount);
            int extraOffset = 0;
            for(int i = 0; i < vertexCount * 2; i += 2)
            {
                vertices[i + extraOffset] = *(p + i);
                vertices[i + 1 + extraOffset] = *(p + i + 1);
                vertices[i + 2 + extraOffset] = 1;
                extraOffset++;
            }
            for(int i = 0; i < indexCount; i++)
            {
                indices[i] = *(ind + i);
            }
            numIndices = indexCount;
            normalizePoints();
            initialized = true;
        }
        void initRectangle()
        {
            float p[] = {
                1 / 2.0f,  1 / 2.0f,  // top right
                -1 / 2.0f,  1 / 2.0f,   // top left
                -1 / 2.0f, -1 / 2.0f,  // bottom left 
                1 / 2.0f, -1 / 2.0f  // bottom right
            };
            int ind[] = { 
                0, 1, 3,  
                1, 2, 3    
            };  
            vertices.reserve(12);
            indices.reserve(6);
            numVertices = 4;
            numIndices = 6;
            int extraOffset = 0;
            for(int i = 0; i < numVertices * 2; i += 2)
            {
                vertices[i + extraOffset] = *(p + i);
                vertices[i + 1 + extraOffset] = *(p + i + 1);
                vertices[i + 2 + extraOffset] = 1;
                extraOffset++;
            }
            for(int i = 0; i < numIndices; i++)
            {
                indices[i] = *(ind + i);
            }
            normalizePoints();
            initialized = true;
        }
        void render()
        {
            renderPolygon();
        }
        void setPositionOffset(float x, float y)
        {
            positionOffset = glm::vec3(x, y, 0);
        }
        void setColor(glm::vec3 col)
        {
            color = col;
        }
        void setLayer(int l)
        {
            layer = l;
            if(layer < 0) layer = 0;
            for(int i = 0; i < numVertices; i++)
            {
                vertices[2 + 3 * i] = layer;
            }
        }
        void setRotationOffset(float degrees)
        {
            rotationOffset = degrees;
        }
    private:
        unsigned int polygonVAO = 0, polygonVBO = 0;
        int prevLayer = -1;
        void renderPolygon()
        {
            GLint currentShader = 0;
            glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);   
            if(polygonVAO == 0)
            {
                initVAO();
            }
            if(prevLayer != layer)
            {
                prevLayer = layer;
                bufferNewData();
            }
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(*basePosition + positionOffset, layer));
            model = glm::rotate(model, glm::radians(*baseRotation + rotationOffset), glm::vec3(0, 0, -1));
            model = glm::scale(model, glm::vec3(*baseScale * scaleOffset, 0));
            int modelLoc = glGetUniformLocation(currentShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            int colorLoc = glGetUniformLocation(currentShader, "col");
            glUniform3fv(colorLoc, 1, glm::value_ptr(color));
            glBindVertexArray(polygonVAO);
            glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);             
        }
        void initVAO()
        {
            glGenVertexArrays(1, &polygonVAO);
            glGenBuffers(1, &polygonVBO);
            glBindVertexArray(polygonVAO);
            glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
            glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(float), &vertices[0], GL_STATIC_DRAW);
            unsigned int EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(int), &indices[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);  
            glBindVertexArray(0); 
        }
        void bufferNewData()
        {
            glBindVertexArray(polygonVAO);
            glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 3 * sizeof(float), &vertices[0]);
            glBindVertexArray(0);
        }
        void normalizePoints()
        {
            glm::vec2 centroid(0, 0);
            for(int i = 0; i < numVertices * 3; i += 3)
            {
                centroid.x += vertices[i];
                centroid.y += vertices[i + 1];
            }
            centroid.x /= numVertices;
            centroid.y /= numVertices;
            for(int i = 0; i < numVertices * 3; i += 3)
            {
                vertices[i] -= centroid.x;
                vertices[i + 1] -= centroid.y;
            }
            float area = 0;
            for(int i = 0; i < numVertices * 3; i += 3)
            {
                int nextX = i + 3;
                int nextY = i + 4;
                if(i + 3 >= numVertices * 3)
                { 
                    nextX = 0;
                    nextY = 1;
                }
                area += vertices[i] * vertices[nextY] - vertices[nextX] * vertices[i + 1];
            }
            area /= 2;
            float scale = 1 / std::sqrt(area);
            for(int i = 0; i < numVertices * 3; i += 3)
            {
                vertices[i] *= scale;;
                vertices[i + 1] *= scale;
            }
        }
};
