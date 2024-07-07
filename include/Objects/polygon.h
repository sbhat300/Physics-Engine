#ifndef POLYGON_H
#define POLYGON_H

class entity;
#include <glm/glm.hpp>
#include <vector>

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
        entity* base;
        polygon();
        polygon(entity* b, glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0);
        void initPolygon(int vertexCount, float* p, int indexCount, int* ind);
        void initRectangle();
        void render();
        void setPositionOffset(float x, float y);
        void setColor(glm::vec3 col);
        void setLayer(int l);
        void setRotationOffset(float degrees);
    private:
        unsigned int polygonVAO, polygonVBO;
        int prevLayer;
        void renderPolygon();
        void initVAO();
        void bufferNewData();
        void normalizePoints();
};
#endif