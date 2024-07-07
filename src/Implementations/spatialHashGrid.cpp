#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <limits>

#include "Physics/spatialHashGrid.h"

spatialHashGrid::spatialHashGrid(float w, float h, glm::vec2 nc, glm::vec2 s)
{
    width = w;
    height = h;
    numCells = nc;
    start = s;
    grid.resize(numCells.x);
    for(int i = 0; i < numCells.x; i++)
    {
        grid[i].resize(numCells.y);
    }
    queryID = 1;
    cellWidth = width / numCells.x;
    cellHeight = height / numCells.y;
}
void spatialHashGrid::setLayer(int l)
{
    layer = l;
}
void spatialHashGrid::setColor(glm::vec3 col)
{
    color = col;
}
void spatialHashGrid::drawGrid()
{
    float xOff = width / numCells.x;
    GLint currentShader = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);  
    int colorLoc = glGetUniformLocation(currentShader, "col");
    for(int i = 0; i < numCells.x + 1; i++)
    {
        glm::vec2 firstPoint = glm::vec2(start.x + xOff * i, start.y);
        glm::vec2 secondPoint = glm::vec2(start.x + xOff * i, start.y + height);
    
        float points[] = {firstPoint.x, firstPoint.y, layer, secondPoint.x, secondPoint.y, layer};
        if(gridVAO == 0)
        {
            glGenVertexArrays(1, &gridVAO);
            glGenBuffers(1, &gridVBO);
            glBindVertexArray(gridVAO);
            glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);  
            glBindVertexArray(0); 
        }    
        glUniform3fv(colorLoc, 1, glm::value_ptr(color));
        glBindVertexArray(gridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
        glDrawArrays(GL_LINES, 0, 2);
    }
    float yOff = height / numCells.y;
    for(int i = 0; i < numCells.y + 1; i++)
    {
        glm::vec2 firstPoint = glm::vec2(start.x, start.y + i * yOff);
        glm::vec2 secondPoint = glm::vec2(start.x + width, start.y + i * yOff);
    
        float points[] = {firstPoint.x, firstPoint.y, layer, secondPoint.x, secondPoint.y, layer};
        glUniform3fv(colorLoc, 1, glm::value_ptr(color));
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
        glDrawArrays(GL_LINES, 0, 2);
    }
    glBindVertexArray(0);  
}
void spatialHashGrid::add(polygonCollider* obj)
{
    std::pair<int, int> lower = getCellIndex((*obj).minX, (*obj).minY);
    std::pair<int, int> upper = getCellIndex((*obj).maxX, (*obj).maxY);
    (*obj).minIndices.first = lower.first;
    (*obj).minIndices.second = lower.second;
    (*obj).maxIndices.first = upper.first;
    (*obj).maxIndices.second = upper.second;
    (*obj).shgIndex.resize(upper.first - lower.first + 1);
    for(int i = lower.first; i <= upper.first; i++)
    {
        int xInd = i - lower.first;
        (*obj).shgIndex[xInd].clear();
        for(int j = lower.second; j <= upper.second; j++)
        {
            grid[i][j].push_back(obj);
            (*obj).shgIndex[xInd].push_back(grid[i][j].size() - 1);
        }
    }
} 
std::vector<polygonCollider*> spatialHashGrid::getNearby(polygonCollider* obj)
{
    std::vector<polygonCollider*> out;
    queryID++;
    for(int i = (*obj).minIndices.first; i <= (*obj).maxIndices.first; i++)
    {
        for(int j = (*obj).minIndices.second; j <= (*obj).maxIndices.second; j++)
        {
            for(auto object = grid[i][j].begin(); object != grid[i][j].end(); object++)
            {
                if(queryID != (*(*object)).queryID)
                {
                    out.push_back(*object);
                    (*(*object)).queryID = queryID;
                }
            }
        }
    }
    return out;
}
std::vector<polygonCollider*> spatialHashGrid::getNearby(float x, float y)
{
    std::vector<polygonCollider*> out;
    std::pair<int, int> ind = getCellIndex(x, y);
    for(auto object = grid[ind.first][ind.second].begin(); object != grid[ind.first][ind.second].end(); object++)
        out.push_back(*object);
    return out;
}
void spatialHashGrid::remove(polygonCollider* obj)
{
    for(int i = (*obj).minIndices.first; i <= (*obj).maxIndices.first; i++)
    {
        for(int j = (*obj).minIndices.second; j <= (*obj).maxIndices.second; j++)
        {
            int pos = (*obj).shgIndex[i - (*obj).minIndices.first][j - (*obj).minIndices.second];
            polygonCollider* second = grid[i][j].back();
            grid[i][j][pos] = second;
            (*second).shgIndex[i - (*second).minIndices.first][j - (*second).minIndices.second] = pos;
            grid[i][j].pop_back();
        }
    }
}
std::pair<int, int> spatialHashGrid::getCellIndex(float x, float y)
{
    float xInd = clamp((x - start.x) / width, 0, 1);
    float yInd = clamp((y - start.y) / height, 0, 1);
    std::pair<int, int> out(std::min(numCells.x - 1, floor(xInd * (numCells.x))), std::min(numCells.y - 1, floor(yInd * (numCells.y))));
    return out;
}
std::pair<int, int> spatialHashGrid::getCellIndexNoClamp(float x, float y)
{
    float xInd = (x - start.x) / width;
    float yInd = (y - start.y) / height;
    std::pair<int, int> out(floor(xInd * (numCells.x)), floor(yInd * (numCells.y)));
    return out;
}
float spatialHashGrid::clamp(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}
std::vector<polygonCollider*> spatialHashGrid::getNearbyRay(ray* r)
{
    std::vector<polygonCollider*> output;
    glm::vec2 secondPoint = (*r).origin + (*r).direction * (*r).length;
    std::pair<int, int> lower = getCellIndexNoClamp((*r).origin.x, (*r).origin.y);
    std::pair<int, int> upper = getCellIndexNoClamp(secondPoint.x, secondPoint.y);
    if(lower.first < 0 || lower.second < 0 || lower.first >= numCells.x || lower.second >= numCells.y) return output;
    if(lower.first == upper.first && lower.second == upper.second) return getNearby((*r).origin.x, (*r).origin.y);
    int stepX, stepY;
    float xLength = (*r).direction.x * (*r).length;
    float yLength = (*r).direction.y * (*r).length;
    float tMaxX, tMaxY;
    if((*r).direction.x > 0)
    {
        float top = start.x + (lower.first + 1) * cellWidth - (*r).origin.x;
        if(top == 0 && xLength == 0) tMaxX = std::numeric_limits<float>::infinity();
        else tMaxX = std::abs(top / xLength);
        stepX = 1;
    }
    else
    {
        float top = (*r).origin.x - (start.x + lower.first * cellWidth);
        if(top == 0 && xLength == 0) tMaxX = std::numeric_limits<float>::infinity();
        tMaxX = std::abs(top / xLength);
        stepX = -1;
    }
    if((*r).direction.y > 0) 
    { 
        float top = start.y + (lower.second + 1) * cellHeight - (*r).origin.y;
        if(top == 0 && yLength == 0) tMaxY = std::numeric_limits<float>::infinity();
        else tMaxY = std::abs(top / yLength);
        stepY = 1;
    }
    else
    {
        float top = (*r).origin.y - (start.y + lower.second * cellHeight);
        if(top == 0 && yLength == 0) tMaxY = std::numeric_limits<float>::infinity();
        else tMaxY = std::abs(top / yLength);
        stepY = -1;
    }
    float tDeltaX = std::abs(cellWidth / xLength);
    float tDeltaY = std::abs(cellHeight / yLength);
    queryID++;
    do
    {
        int xInd = clamp(lower.first, 0, numCells.x - 1);
        int yInd = clamp(lower.second, 0, numCells.y - 1);
        if(lower.first != xInd || lower.second != yInd) return output;
        for(auto object = grid[xInd][yInd].begin(); object != grid[xInd][yInd].end(); object++)
        {
            if(queryID != (*(*object)).queryID)
            {
                output.push_back(*object);
                (*(*object)).queryID = queryID;
            }
        }
        if(tMaxX == tMaxY)
        {
            int next = xInd + stepX;
            for(auto object = grid[next][yInd].begin(); object != grid[next][yInd].end(); object++)
            {
                if(queryID != (*(*object)).queryID)
                {
                    output.push_back(*object);
                    (*(*object)).queryID = queryID;
                }
            }
        }
        if(tMaxX < tMaxY)
        {
            tMaxX += tDeltaX;
            lower.first += stepX;
        }
        else
        {
            tMaxY += tDeltaY;
            lower.second += stepY;
        }
    } while (tMaxX <= 1 || tMaxY <= 1);
    std::vector<polygonCollider*> lastCell = getNearby(secondPoint.x, secondPoint.y);
    for(auto object = lastCell.begin(); object != lastCell.end(); object++)
    {
        if(queryID != (*(*object)).queryID)
        {
            output.push_back(*object);
            (*(*object)).queryID = queryID;
        }
    }
    return output;
}
std::pair<bool, rayData> spatialHashGrid::getNearbyRaySingle(ray* r)
{
    glm::vec2 secondPoint = (*r).origin + (*r).direction * (*r).length;
    std::pair<int, int> lower = getCellIndexNoClamp((*r).origin.x, (*r).origin.y);
    std::pair<int, int> upper = getCellIndexNoClamp(secondPoint.x, secondPoint.y);
    std::pair<bool, rayData> test;
    if(lower.first < 0 || lower.second < 0 || lower.first >= numCells.x || lower.second >= numCells.y) 
    {
        test.first = false;
        return test;
    }
    if(lower.first == upper.first && lower.second == upper.second)
    {
        test = (*r).getFirstCollision(&grid[clamp(lower.first, 0, numCells.x - 1)][clamp(lower.second, 0, numCells.y - 1)]);
        if(test.first) return test;
    }
    int stepX, stepY;
    float xLength = (*r).direction.x * (*r).length;
    float yLength = (*r).direction.y * (*r).length;
    float tMaxX, tMaxY;
    if((*r).direction.x > 0)
    {
        float top = start.x + (lower.first + 1) * cellWidth - (*r).origin.x;
        if(top == 0 && xLength == 0) tMaxX = std::numeric_limits<float>::infinity();
        else tMaxX = std::abs(top / xLength);
        stepX = 1;
    }
    else
    {
        float top = (*r).origin.x - (start.x + lower.first * cellWidth);
        if(top == 0 && xLength == 0) tMaxX = std::numeric_limits<float>::infinity();
        tMaxX = std::abs(top / xLength);
        stepX = -1;
    }
    if((*r).direction.y > 0) 
    { 
        float top = start.y + (lower.second + 1) * cellHeight - (*r).origin.y;
        if(top == 0 && yLength == 0) tMaxY = std::numeric_limits<float>::infinity();
        else tMaxY = std::abs(top / yLength);
        stepY = 1;
    }
    else
    {
        float top = (*r).origin.y - (start.y + lower.second * cellHeight);
        if(top == 0 && yLength == 0) tMaxY = std::numeric_limits<float>::infinity();
        else tMaxY = std::abs(top / yLength);
        stepY = -1;
    }
    float tDeltaX = std::abs(cellWidth / xLength);
    float tDeltaY = std::abs(cellHeight / yLength);
    queryID++;
    do
    {
        int xInd = clamp(lower.first, 0, numCells.x - 1);
        int yInd = clamp(lower.second, 0, numCells.y - 1);
        if(lower.first != xInd || lower.second != yInd) return test;
        test = (*r).getFirstCollision(&grid[xInd][yInd]);
        if(test.first) return test;
        if(tMaxX == tMaxY)
        {
            int next = xInd + stepX;
            test = (*r).getFirstCollision(&grid[next][yInd]);
            if(test.first) return test;
        }
        if(tMaxX < tMaxY)
        {
            tMaxX += tDeltaX;
            lower.first += stepX;
        }
        else
        {
            tMaxY += tDeltaY;
            lower.second += stepY;
        }
    } while (tMaxX <= 1 || tMaxY <= 1);
    std::vector<polygonCollider*> lastCell = getNearby(secondPoint.x, secondPoint.y);
    test = (*r).getFirstCollision(&lastCell);
    return test;
}