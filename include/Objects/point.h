#ifndef POINT_H
#define POINT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class point
{
    public:
        float x, y;
        float size;
        glm::vec3 color;
        glm::vec3 orig;
        float layer;

        point(float xPos = 0, float yPos = 0, float s = 1);
        void render();
        void setPosition(float xPos, float yPos);
        void setColor(glm::vec3 col);
        void setLayer(int l);

    private:
        glm::vec3 position;
        unsigned int pointVAO, pointVBO;
        
        void renderPoint();
};
#endif