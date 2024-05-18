#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

class ray
{
    public:
        glm::vec2 origin;
        glm::vec2 direction;
        glm::vec3 color;
        float length;
        float layer;

        ray(glm::vec2 o = glm::vec2(0, 0), glm::vec2 d = glm::vec2(0, 1), float l = 0);
        void render();
        void setColor(glm::vec3 col);
        void setLayer(int l);
        void bufferNewData();
    private:
        unsigned int rayVAO, rayVBO;
        void drawRay();
};

#endif