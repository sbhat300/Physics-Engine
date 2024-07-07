#include <entity.h>
#include <Physics/spatialHashGrid.h>
#include <Objects/point.h>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <vector>
#include <algorithm>  
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <set>
#include "Physics/polygonCollider.h"

polygonCollider::polygonCollider(){}
polygonCollider::polygonCollider(spatialHashGrid* spg, entity* b, glm::vec2 p, glm::vec2 s, float r)
{
    initialized = false;
    collide = false;
    positionOffset = p;
    scaleOffset = s;
    rotationOffset = r;
    debugPoint = point(0, 0, 3);
    furthestDistance = -1;
    grid = spg;
    base = b;
    queryID = 0;
}
void polygonCollider::updateCollider()
{
    if(collide) checkCollisions();
}
void polygonCollider::setCollisionCallback(std::function<void(int, int, glm::vec2, float, int, glm::vec2, glm::vec2)> cb)
{
    collisionCallback = cb;
    collide = true;
}
bool polygonCollider::pointInPolygon(glm::vec2 point)
{
    float neg = 0, pos = 0;
    if(glm::length2(centroid - point) > furthestDistance * furthestDistance) return false;
    for(int i = 0; i < numVertices; i++)
    {
        int vNext = i + 1;
        if(vNext == numVertices) vNext = 0;
        glm::vec2 perpVec = glm::normalize(points[vNext] - points[i]);
        perpVec = glm::vec2(-perpVec.y, perpVec.x);
        float side = glm::dot(perpVec, point - glm::vec2(points[i]));
        if(side == 0) continue;
        if(side < 0) neg++;
        else pos++;
        if(neg > 0 && pos > 0) return false;
    }
    return true;
}
void polygonCollider::initPolygon(int vertexCount, float* p)
{
    numVertices = vertexCount;
    vertices.reserve(vertexCount * 3);
    points.reserve(vertexCount);
    int extraOffset = 0;
    for(int i = 0; i < vertexCount * 2; i += 2)
    {
        vertices[i + extraOffset] = *(p + i);
        vertices[i + 1 + extraOffset] = *(p + i + 1);
        vertices[i + 2 + extraOffset] = 1;
        extraOffset++;
    }
    normalizePoints();
    updatePointsNoRemove();
    updateFurthestPoint();
    initialized = true;
}
void polygonCollider::initRectangle()
{
    float p[] = {
        1 / 2.0f,  1 / 2.0f,  // top right
        -1 / 2.0f,  1 / 2.0f,   // top left
        -1 / 2.0f, -1 / 2.0f,  // bottom left 
        1 / 2.0f, -1 / 2.0f  // bottom right
    };  
    vertices.reserve(12);
    points.reserve(4);
    numVertices = 4;
    int extraOffset = 0;
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        vertices[i + extraOffset] = *(p + i);
        vertices[i + 1 + extraOffset] = *(p + i + 1);
        vertices[i + 2 + extraOffset] = 1;
        extraOffset++;
    }
    normalizePoints();
    updatePointsNoRemove();
    updateFurthestPoint();
    initialized = true;
}
void polygonCollider::updatePoints()
{
    calcPoints();
    std::pair<int, int> lower = (*grid).getCellIndex(minX, minY);
    std::pair<int, int> upper = (*grid).getCellIndex(maxX, maxY);
    if( lower.first == minIndices.first &&
        lower.second == minIndices.second &&
        upper.first == maxIndices.first &&
        upper.second == maxIndices.second) return;
    (*grid).remove(this);
    (*grid).add(this);
}
void polygonCollider::updatePointsNoRemove()
{
    calcPoints();
    (*grid).add(this);
}
void polygonCollider::calcPoints()
{
    centroid = glm::vec2(0, 0);
    for(int i = 0; i < numVertices * 3; i += 3)
    {
        glm::vec2 point = glm::vec2(vertices[i], vertices[i + 1]);
        points[i / 3] = point * *baseScale * scaleOffset;
        float temp[2] = {points[i / 3].x, points[i / 3].y};
        points[i / 3].x = temp[0] * cos(glm::radians(-(rotationOffset))) - temp[1] * sin(glm::radians(-(rotationOffset)));
        points[i / 3].y = temp[0] * sin(glm::radians(-(rotationOffset))) + temp[1] * cos(glm::radians(-(rotationOffset)));
        points[i / 3].x += positionOffset.x;
        points[i / 3].y += positionOffset.y;
        temp[0] = points[i / 3].x;
        temp[1] = points[i / 3].y;
        points[i / 3].x = temp[0] * cos(glm::radians(-(*baseRotation))) - temp[1] * sin(glm::radians(-(*baseRotation)));
        points[i / 3].y = temp[0] * sin(glm::radians(-(*baseRotation))) + temp[1] * cos(glm::radians(-(*baseRotation)));
        points[i / 3].x += (*basePosition).x;
        points[i / 3].y += (*basePosition).y;
        centroid.x += points[i / 3].x;
        centroid.y += points[i / 3].y;
    }
    maxX = points[0].x;
    minX = points[0].x;
    maxY = points[0].y;
    minY = points[0].y;
    for(int i = 0; i < numVertices * 3; i += 3)
    {
        minX = std::min(minX, points[i / 3].x);
        maxX = std::max(maxX, points[i / 3].x);
        minY = std::min(minY, points[i / 3].y);
        maxY = std::max(maxY, points[i / 3].y);
    }
    centroid.x /= numVertices;
    centroid.y /= numVertices;
}
void polygonCollider::normalizePoints()
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
        vertices[i] *= scale;
        vertices[i + 1] *= scale;
    }
}
void polygonCollider::checkCollisions()
{
    if(!collisionCallback) return;
    std::vector<polygonCollider*> colliders;
    colliders = (*grid).getNearby(this);
    for(auto i = colliders.begin(); i != colliders.end(); i++)
    {
        if((*(*i)).id == id) continue;
        float minOverlap = FLT_MAX;
        glm::vec2 smallestAxis;
        polygonCollider* test = *i;
        float centerDist = glm::length2(centroid - (*test).centroid);
        float radiusDist = furthestDistance + (*test).furthestDistance;
        if(centerDist > radiusDist * radiusDist) continue;
        std::vector<glm::vec2> axes(numVertices + (*test).numVertices);
        for(int i = 0; i < numVertices; i++)
        {
            glm::vec2 v1(points[i].x, points[i].y);
            int next = i + 1;
            if(next >= numVertices) next = 0;
            glm::vec2 v2(points[next].x, points[next].y);
            glm::vec2 normal = v1 - v2;
            normal = glm::normalize(glm::vec2(-normal.y, normal.x));
            axes[i] = normal;
        }
        for(int i = 0; i < (*test).numVertices; i++)
        {
            glm::vec2 v1((*test).points[i].x, (*test).points[i].y);
            int next = i + 1;
            if(next >= (*test).numVertices) next = 0;
            glm::vec2 v2((*test).points[next].x, (*test).points[next].y);
            glm::vec2 normal = v1 - v2;
            normal = glm::normalize(glm::vec3(-normal.y, normal.x, 0));
            axes[numVertices + i] = normal;
        }
        int notColliding = false;
        for(int i = 0; i < axes.size(); i++)
        {
            project(&axes[i], points, numVertices);
            float one[2] = {minMax[0], minMax[1]};
            project(&axes[i], (*test).points, (*test).numVertices);
            float two[2] = {minMax[0], minMax[1]};
            float right = two[1] - one[0];
            float left = one[1] - two[0];
            if(left < 0 || right < 0)
            {
                notColliding = true;
                break;
            }
            float overlap;
            if(right < left) overlap = right;
            else overlap = left;
            if(overlap < std::abs(minOverlap))
            {
                minOverlap = overlap;
                smallestAxis = axes[i];
                if(right <= left) smallestAxis *= -1;
            }
        }
        if(notColliding) continue;

        edge edge1 = findEdge(points, numVertices, smallestAxis);
        edge edge2 = findEdge((*test).points, (*test).numVertices, -smallestAxis);
        glm::vec2 referenceVector;
        edge reference, incident;
        if(std::abs(glm::dot(glm::normalize(edge1.v2 - edge1.v1), smallestAxis)) <= std::abs(glm::dot(glm::normalize(edge2.v2 - edge2.v1), smallestAxis)))
        {
            reference = edge1;
            referenceVector = glm::normalize(edge1.v2 - edge1.v1);
            incident = edge2;
        }
        else
        {
            reference = edge2;
            referenceVector = glm::normalize(edge2.v2 - edge2.v1);
            incident = edge1;
        }
        float offset = glm::dot(referenceVector, reference.v1);
        clippedPoints clipped = clipPoints(&incident.v1, &incident.v2, &referenceVector, &offset);
        if(clipped.numPoints < 2) 
        {
            std::cout << "1 clipping failed" << std::endl;
            return;
        }
        offset = -glm::dot(referenceVector, reference.v2);
        glm::vec2 negReferenceVec = -referenceVector;
        clipped = clipPoints(&clipped.points[0], &clipped.points[1], &negReferenceVec, &offset);
        if(clipped.numPoints < 2) 
        {
            std::cout << "2 clipping failed" << std::endl;
            return;
        }
        glm::vec2 perpVec = glm::vec2(-referenceVector.y, referenceVector.x);
        float max = glm::dot(perpVec, reference.vMax);
        if(glm::dot(perpVec, clipped.points[0]) - max < 0)
        {
            clipped.numPoints--;
            clipped.points[0] = clipped.points[1];
        }
        if(glm::dot(perpVec, clipped.points[1]) - max < 0)
        {
            clipped.numPoints--;
        }
        //DEBUG
        for(int i = 0; i < clipped.numPoints; i++)
        {
            glUseProgram(debugShaderProgram);
            debugPoint.setColor(glm::vec3(1.0f, 1.0f, 0.0f));
            debugPoint.setLayer(3);
            debugPoint.setPosition(clipped.points[i].x, clipped.points[i].y);
            debugPoint.render();
        }
        collisionCallback(id, (*test).id, smallestAxis, minOverlap, clipped.numPoints, clipped.points[0], clipped.points[1]);
    }
}
void polygonCollider::project(glm::vec2* axis, std::vector<glm::vec2> &vertices, int numVertices)
{
    minMax[0] = glm::dot(glm::vec3(*axis, 0), glm::vec3(vertices[0], 0));
    minMax[1] = minMax[0];
    for(int i = 1; i < numVertices; i++)
    {
        float projected = glm::dot(glm::vec3(*axis, 0), glm::vec3(vertices[i], 0));
        if(projected < minMax[0]) minMax[0] = projected;
        if(projected > minMax[1]) minMax[1] = projected;
    }
}
polygonCollider::edge polygonCollider::findEdge(std::vector<glm::vec2> &points, int numVertices, glm::vec2 collisionNormal)
{
    float max = glm::dot(glm::vec3(collisionNormal, 0), glm::vec3(points[0], 0));
    int index = 0;
    for(int i = 1; i < numVertices; i++)
    {
        float proj = glm::dot(glm::vec3(collisionNormal, 0), glm::vec3(points[i], 0));
        if(proj > max)
        {
            index = i;
            max = proj;
        }
    }

    glm::vec3 v1 = glm::vec3(points[index], 0);
    glm::vec3 vNext;
    glm::vec3 vPrev;
    if(index + 1 == numVertices) vNext = glm::vec3(points[0], 0);
    else vNext = glm::vec3(points[index + 1], 0);
    if(index - 1 == -1) vPrev = glm::vec3(points[numVertices - 1], 0); 
    else vPrev = glm::vec3(points[index - 1], 0);
    glm::vec3 vNextLine = glm::normalize(v1 - vNext);
    glm::vec3 vPrevLine = glm::normalize(v1 - vPrev);
    edge output;
    if(glm::dot(vPrevLine, glm::vec3(collisionNormal, 0)) <= glm::dot(vNextLine, glm::vec3(collisionNormal, 0)))
    {
        output.v1 = vPrev;
        output.v2 = v1;
    }
    else 
    {
        output.v1 = v1;
        output.v2 = vNext;
    }
    output.vMax = v1;
    
    return output;
}
polygonCollider::clippedPoints polygonCollider::clipPoints(glm::vec2* point1, glm::vec2* point2, glm::vec2* referenceEdge, float* offset)
{
    clippedPoints output;
    output.numPoints = 0;
    float firstOffset = glm::dot(*point1, *referenceEdge) - *offset;
    float secondOffset = glm::dot(*point2, *referenceEdge) - *offset;
    if(firstOffset >= 0)
    {
        output.points[output.numPoints] = *point1;
        output.numPoints++;
    }
    if(secondOffset >= 0)
    {
        output.points[output.numPoints] = *point2;
        output.numPoints++;
    }
    if(secondOffset * firstOffset < 0)
    {
        glm::vec2 clipEdge = *point2 - *point1;
        float scale = firstOffset / (firstOffset - secondOffset);
        clipEdge *= scale;
        clipEdge += *point1;
        output.points[output.numPoints] = clipEdge;
        output.numPoints++;
    }
    return output;
}
void polygonCollider::setPositionOffset(float x, float y)
{
    positionOffset = glm::vec2(x, y);
    updatePoints();
}
void polygonCollider::setScaleOffset(float x, float y)
{
    scaleOffset = glm::vec2(x, y);
    updatePoints();
    updateFurthestPoint();
}
void polygonCollider::updateFurthestPoint()
{
    furthestDistance = -1;
    for(int i = 0; i < numVertices; i++)
    {
        glm::vec2 diffVec = centroid - points[i];
        float possible = glm::length2(diffVec);
        if(furthestDistance == -1 || possible > furthestDistance)
            furthestDistance = possible;
    }
    furthestDistance = sqrt(furthestDistance);
}
void polygonCollider::setRotationOffset(float r)
{
    rotationOffset = r;
    updatePoints();
}
void polygonCollider::renderColliderBounds()
{
    debugPoint.setColor(glm::vec3(0.0f, 1.0f, 1.0f));
    for(int i = 0; i < numVertices; i++)
    {
        glUseProgram(debugShaderProgram);
        debugPoint.setPosition(points[i].x, points[i].y);
        debugPoint.setLayer(2);
        debugPoint.size = 6;
        debugPoint.render();
    }
}