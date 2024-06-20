#ifndef SPATIALHASHGRID_H
#define SPATIALHASHGRID_H

#include <unordered_map>
#include <Physics/polygonCollider.h>
#include <vector>
#include <set>

class spatialHashGrid
{
    public:
        std::vector<std::vector<std::vector<polygonCollider*>>> grid;
        float width, height;
        glm::vec2 start;
        glm::vec2 numCells;
        float layer;
        glm::vec3 color;
        spatialHashGrid(float w = 0, float h = 0, glm::vec2 nc = glm::vec2(0, 0), glm::vec2 s = glm::vec2(0, 0));
        void add(polygonCollider* obj);
        void remove(polygonCollider* obj);
        std::vector<polygonCollider*> getNearby(polygonCollider* obj);
        void drawGrid();
        void setLayer(int l);
        void setColor(glm::vec3 col);
        float clamp(float n, float lower, float upper);
        std::pair<int, int> getCellIndex(float x, float y);

    private:
        unsigned int gridVAO, gridVBO;
        unsigned int queryID;
};

#endif