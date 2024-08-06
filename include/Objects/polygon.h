#ifndef POLYGON_H
#define POLYGON_H

class entity;
#include <glm/glm.hpp>
#include <vector>
#include <sharedData.h>

class polygon
{
    public:
        glm::vec2 scaleOffset, startScaleOffset, positionOffset, startPositionOffset;
        float rotationOffset, startRotationOffset;
        glm::vec2* basePosition;
        glm::vec2 prevPos, prevBasePos;
        glm::vec2 prevScale, prevBaseScale;
        float prevRot, prevBaseRot;
        glm::vec2* baseScale;
        float* baseRotation;
        int shaderProgram;
        bool initialized;
        std::vector<float> vertices;
        std::vector<int> indices;
        int numVertices;
        int numIndices;
        glm::vec3 color, startColor;
        int layer, startLayer;
        sharedData* shared;
        entity* base;
        polygon();
        polygon(entity* b, glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, glm::vec3 col = glm::vec3(1, 1, 1), int l = 1);
        void initPolygon(int vertexCount, float* p, int indexCount, int* ind);
        void initRectangle();
        void render(float alpha);
        void setPositionOffset(float x, float y);
        void setColor(glm::vec3 col);
        void setLayer(int l);
        void setRotationOffset(float degrees);
        void setScaleOffset(float x, float y);
    private:
        unsigned int polygonVAO, polygonVBO;
        void renderPolygon(float alpha);
        void initVAO();
        void bufferNewData();
        void normalizePoints();
        glm::vec2 interpolate(glm::vec2 first, glm::vec2 second, float alpha);
        float interpolate(float first, float second, float alpha);
        void initPrevious();
};
#endif