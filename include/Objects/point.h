#ifndef POINT_H
#define POINT_H

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

class point
{
    public:
        float x, y;
        float size;
        glm::vec3 color;
        glm::vec3 orig;
        float layer;
        point(float xPos = 0, float yPos = 0, float s = 1)
        {
            x = xPos;
            y = yPos;
            size = s;
            layer = 1;
            position = glm::vec3(x, y, layer);
            orig = glm::vec3(0, 0, 0);
        }
        void render()
        {
            renderPoint();
        }
        void setPosition(float xPos, float yPos)
        {
            x = xPos;
            y = yPos;
            position = glm::vec3(x, y, layer);
        }
        void setColor(glm::vec3 col)
        {
            color = col;
        }
        void setLayer(int l)
        {
            layer = l;
            position.z = layer;
        }
    private:
        glm::vec3 position;
        unsigned int pointVAO = 0, pointVBO = 0;
        void renderPoint()
        {
            GLint currentShader = 0;
            glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);   
            if(pointVAO == 0)
            {
                glGenVertexArrays(1, &pointVAO);
                glGenBuffers(1, &pointVBO);
                glBindVertexArray(pointVAO);
                glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(orig), glm::value_ptr(orig), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);  
                glBindVertexArray(0); 
            }
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            int modelLoc = glGetUniformLocation(currentShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            int colorLoc = glGetUniformLocation(currentShader, "col");
            glUniform3fv(colorLoc, 1, glm::value_ptr(color));
            int sizeLoc = glGetUniformLocation(currentShader, "size");
            glUniform1f(sizeLoc, size);
            glBindVertexArray(pointVAO);
            glDrawArrays(GL_POINTS, 0, 1);
            glBindVertexArray(0);             
        }
};
#endif