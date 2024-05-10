#ifndef RECTANGLE_H
#define RECTANGLE_H

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

class rectangle
{
    public:
        float height, width;
        glm::vec3 scale, position, previousPos;
        float rotation;
        glm::vec3 points[4];
        float rect_vertices[12];
        int vertices;
        glm::vec3 color;
        float layer;
        rectangle(float h = 1, float w = 1, float r = 0, glm::vec2 s = glm::vec2(1, 1), glm::vec2 p = glm::vec2(0, 0))
        {
            height = h;
            width = w;
            scale = glm::vec3(s, 1);
            rotation = r;
            position = glm::vec3(p.x, p.y, 0);
            color = glm::vec3(1,1,1);
            layer = 1;
            //COUNTER CLOCKWISE CONVENTION
            float tempVertices[] = {
                width / 2,  height / 2, 1.0f,  // top right
                -width / 2,  height / 2, 1.0f,   // top left
                -width / 2, -height / 2, 1.0f,  // bottom left 
                width / 2, -height / 2, 1.0f  // bottom right
            };
            vertices = (sizeof(rect_vertices) / sizeof(float)) / 3;
            std::copy(std::begin(tempVertices), std::end(tempVertices), std::begin(rect_vertices));
            glm::vec2 centroid(0, 0);
            for(int i = 0; i < vertices * 3; i += 3)
            {
                centroid.x += rect_vertices[i];
                centroid.y += rect_vertices[i + 1];
            }
            centroid.x /= vertices;
            centroid.y /= vertices;
            for(int i = 0; i < vertices * 3; i += 3)
            {
                rect_vertices[i] -= centroid.x;
                rect_vertices[i + 1] -= centroid.y;
            }
            float area = 0;
            for(int i = 0; i < vertices * 3; i += 3)
            {
                int nextX = i + 3;
                int nextY = i + 4;
                if(i + 3 >= vertices * 3)
                { 
                    nextX = 0;
                    nextY = 1;
                }
                area += rect_vertices[i] * rect_vertices[nextY] - rect_vertices[nextX] * rect_vertices[i + 1];
            }
            area /= 2;
            float scale = 1 / std::sqrt(area);
            for(int i = 0; i < vertices * 3; i += 3)
            {
                rect_vertices[i] *= scale;;
                rect_vertices[i + 1] *= scale;
            }
            updatePoints();
        }
        void render()
        {
            renderPolygon();
        }
        void render(float x, float y)
        {
            renderPolygon(x, y);
        }
        void setPosition(float x, float y)
        {
            previousPos = glm::vec3(position.x, position.y, 0);
            position = glm::vec3(x, y, 0);
            updatePoints();
        }
        void setColor(glm::vec3 col)
        {
            color = col;
        }
        void setLayer(int l)
        {
            layer = l;
            if(layer < 0) layer = 0;
            for(int i = 0; i < vertices; i++)
            {
                rect_vertices[2 + 3 * i] = layer;
            }
            std::cout << layer << std::endl;
        }
        void updatePoints()
        {
            for(int i = 0; i < 12; i += 3)
            {
                glm::vec3 point = glm::vec3(rect_vertices[i], rect_vertices[i + 1], 0);;
                points[i / 3] = point * scale;
                float temp[2] = {points[i / 3].x, points[i / 3].y};
                points[i / 3].x = temp[0] * cos(glm::radians(-rotation)) - temp[1] * sin(glm::radians(-rotation));
                points[i / 3].y = temp[0] * sin(glm::radians(-rotation)) + temp[1] * cos(glm::radians(-rotation));
                points[i / 3].x += position.x;
                points[i / 3].y += position.y;
            }
        }
        void setRotation(float degrees)
        {
            rotation = degrees;
            updatePoints();
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
            model = glm::translate(model, position);
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 0, -1));
            model = glm::scale(model, scale);
            int modelLoc = glGetUniformLocation(currentShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            int colorLoc = glGetUniformLocation(currentShader, "col");
            glUniform3fv(colorLoc, 1, glm::value_ptr(color));
            glBindVertexArray(polygonVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);             
        }
        void renderPolygon(float x, float y)
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
            glm::vec3 pos(x, y, 0);
            model = glm::translate(model, pos);
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 0, -1));
            model = glm::scale(model, scale);
            int modelLoc = glGetUniformLocation(currentShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            int colorLoc = glGetUniformLocation(currentShader, "col");
            glUniform3fv(colorLoc, 1, glm::value_ptr(color));
            glBindVertexArray(polygonVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);             
        }
        void initVAO()
        {
            unsigned int indices[] = {  // note that we start from 0!
                0, 1, 3,   // first triangle
                1, 2, 3    // second triangle
            };  
            glGenVertexArrays(1, &polygonVAO);
            glGenBuffers(1, &polygonVBO);
            glBindVertexArray(polygonVAO);
            glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_STATIC_DRAW);
            unsigned int EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);  
            glBindVertexArray(0); 
        }
        void bufferNewData()
        {
            glBindVertexArray(polygonVAO);
            glBindBuffer(GL_ARRAY_BUFFER, polygonVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rect_vertices), rect_vertices);
            glBindVertexArray(0);
        }
};
#endif