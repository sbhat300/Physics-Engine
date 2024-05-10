#ifndef RAY_H
#define RAY_H

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

class ray
{
    public:
        glm::vec2 origin;
        glm::vec2 direction;
        glm::vec3 color;
        float length;
        float layer;
        ray(glm::vec2 o = glm::vec2(0, 0), glm::vec2 d = glm::vec2(0, 1), float l = 0)
        {
            origin = o;
            direction = glm::normalize(d);
            length = l;
            color = glm::vec3(1, 1, 1);
            layer = 1;
        }
        void render()
        {
            drawRay();
        }
        void setColor(glm::vec3 col)
        {
            color = col;
        }
        void setLayer(int l)
        {
            layer = l;
        }
    private:
        unsigned int rayVAO = 0, rayVBO = 0;
        void drawRay()
        {
            glm::vec2 firstPoint = origin;
            glm::vec2 secondPoint = origin + direction * length;
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
};

#endif