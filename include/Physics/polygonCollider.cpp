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
#include <Engine/setup.h>
#include <cmath>
#include <Engine/engine.h>

polygonCollider::polygonCollider(){}
polygonCollider::polygonCollider(spatialHashGrid* spg, glm::vec2 p, glm::vec2 s, float r, entity* b)
{
    initialized = false;
    collide = true;
    positionOffset = p;
    scaleOffset = s;
    rotationOffset = r;
    startPositionOffset= p;
    startScaleOffset = s;
    startRotationOffset = r;
    debugPoint = point(0, 0, 6);
    furthestDistance = -1;
    grid = spg;
    base = b;
    queryID = 0;
    shouldRenderBounds = false;
    aabb = false;
    shape = shapeType::POLYGON;
}
void polygonCollider::updateCollider()
{
    if(collide) checkCollisions();
}
void polygonCollider::setCollisionCallback(std::function<void(entity*, entity*, glm::vec2, float, int, glm::vec2, glm::vec2)> cb)
{
    collisionCallback = cb;
    collide = true;
}
bool polygonCollider::pointInPolygon(glm::vec2 point)
{
    if(shape == shapeType::CIRCLE)
    {
        return glm::length2(centroid - point) <= furthestDistance * furthestDistance;
    }
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
void polygonCollider::initPolygon(int vertexCount, float* p, bool normalize)
{
    numVertices = vertexCount;
    vertices.resize(vertexCount * 2);
    points.resize(vertexCount);
    for(int i = 0; i < vertexCount * 2; i += 2)
    {
        vertices[i] = *(p + i);
        vertices[i + 1] = *(p + i + 1);
    }
    if(normalize) normalizePoints();
    updatePointsNoRemove();
    updateFurthestPoint();
    initialized = true;
}
void polygonCollider::initRectangle(bool axisAligned, bool normalize)
{
    float p[] = {
        1 / 2.0f,  1 / 2.0f,  // top right
        -1 / 2.0f,  1 / 2.0f,   // top left
        -1 / 2.0f, -1 / 2.0f,  // bottom left 
        1 / 2.0f, -1 / 2.0f  // bottom right
    };  
    vertices.resize(8);
    points.resize(4);
    numVertices = 4;
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        vertices[i] = *(p + i);
        vertices[i + 1] = *(p + i + 1);
    }
    if(normalize) normalizePoints();
    updatePointsNoRemove();
    updateFurthestPoint();
    aabb = axisAligned;
    initialized = true;
}
void polygonCollider::initCircle(float r)
{
    shape = shapeType::CIRCLE;
    aabb = false;
    furthestDistance = r * baseScale->x * scaleOffset.x;
    radius = r;
    numVertices = 0;
    initialized = true;
    updatePointsNoRemove();
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

    if(shape == shapeType::CIRCLE)
    {
        centroid = *basePosition + positionOffset;
        minX = centroid.x - furthestDistance;
        maxX = centroid.x + furthestDistance;
        minY = centroid.y - furthestDistance;
        maxY = centroid.y + furthestDistance;
        return;
    }

    centroid = glm::vec2(0, 0);
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        glm::vec2 point = glm::vec2(vertices[i], vertices[i + 1]);
        points[i / 2] = point * *baseScale * scaleOffset;
        float temp[2] = {points[i / 2].x, points[i / 2].y};
        if(!aabb)
        {
            points[i / 2].x = temp[0] * cos(rotationOffset) - temp[1] * sin(rotationOffset);
            points[i / 2].y = temp[0] * sin(rotationOffset) + temp[1] * cos(rotationOffset);
        }
        points[i / 2].x += positionOffset.x;
        points[i / 2].y += positionOffset.y;
        temp[0] = points[i / 2].x;
        temp[1] = points[i / 2].y;
        if(!aabb)
        {
            points[i / 2].x = temp[0] * cos(*baseRotation) - temp[1] * sin(*baseRotation);
            points[i / 2].y = temp[0] * sin(*baseRotation) + temp[1] * cos(*baseRotation);
        }
        points[i / 2].x += (*basePosition).x;
        points[i / 2].y += (*basePosition).y;
        centroid.x += points[i / 2].x;
        centroid.y += points[i / 2].y;
    }
    maxX = points[0].x;
    minX = points[0].x;
    maxY = points[0].y;
    minY = points[0].y;
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        minX = std::min(minX, points[i / 2].x);
        maxX = std::max(maxX, points[i / 2].x);
        minY = std::min(minY, points[i / 2].y);
        maxY = std::max(maxY, points[i / 2].y);
    }
    centroid.x /= numVertices;
    centroid.y /= numVertices;
}
void polygonCollider::normalizePoints()
{
    glm::vec2 centroid(0, 0);
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        centroid.x += vertices[i];
        centroid.y += vertices[i + 1];
    }
    centroid.x /= numVertices;
    centroid.y /= numVertices;
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        vertices[i] -= centroid.x;
        vertices[i + 1] -= centroid.y;
    }
    float area = 0;
    for(int i = 0; i < numVertices * 2; i += 2)
    {
        int nextX = i + 2;
        int nextY = i + 3;
        if(i + 2 >= numVertices * 2)
        { 
            nextX = 0;
            nextY = 1;
        }
        area += vertices[i] * vertices[nextY] - vertices[nextX] * vertices[i + 1];
    }
    area /= 2;
    float scale = 1 / std::sqrt(area);
    for(int i = 0; i < numVertices * 2; i += 2)
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
        if((*(*i)).id < id && (*i)->collisionCallback) continue;
        float minOverlap = FLT_MAX;
        glm::vec2 smallestAxis;
        polygonCollider* test = *i;
        if(!test->collide) continue;
        float centerDist = glm::length2(centroid - (*test).centroid);
        float radiusDist = furthestDistance + (*test).furthestDistance;
        if(centerDist > radiusDist * radiusDist) 
        {
            continue;
        }
        if(aabb && (*test).aabb)
        {
            checkAABBCollisions(test);
            continue;
        }

        if(shape == shapeType::CIRCLE && test->shape == shapeType::CIRCLE)
        {
            glm::vec2 rDiff = test->centroid - centroid;
            float dist = glm::length(rDiff);
            glm::vec2 normal;
            if(dist == 0.0f) normal = glm::vec2(0, 1);
            else normal = rDiff / dist;

            float penetration = radiusDist - dist;
            glm::vec2 cp1 = centroid + normal * furthestDistance;
            glm::vec2 cp2 = test->centroid - normal * test->furthestDistance; 
            collisionCallback(base, test->base, normal, penetration, 1, cp1, cp2);
            if(test->collisionCallback) test->collisionCallback(test->base, base, -normal, penetration, 1, cp1, cp2);
            continue;
        }

        if(shape == shapeType::CIRCLE || test->shape == shapeType::CIRCLE)
        {
            polygonCollider* circle = (shape == shapeType::CIRCLE) ? this : test;
            polygonCollider* polygon = (shape == shapeType::CIRCLE) ? test : this;

            float minDist = FLT_MAX;
            glm::vec2 closestVertex;
            for(int j = 0; j < polygon->numVertices; j++)
            {
                float d = glm::length2(circle->centroid - polygon->points[j]);
                if(d < minDist) 
                { 
                    minDist = d; 
                    closestVertex = polygon->points[j]; 
                }
            }

            std::vector<glm::vec2> axes;
            for(int j = 0; j < polygon->numVertices; j++)
            {
                glm::vec2 v1(polygon->points[j].x, polygon->points[j].y);
                int next = j + 1;
                if(next >= polygon->numVertices) next = 0;
                glm::vec2 v2(polygon->points[next].x, polygon->points[next].y);
                glm::vec2 normal = v1 - v2;
                normal = glm::normalize(glm::vec2(-normal.y, normal.x));
                normal = glm::vec2(std::round(normal.x * 10000.0f) / 10000.0f, std::round(normal.y * 10000.0f) / 10000.0f);
                axes.push_back(normal);
            }
            glm::vec2 circleAxis = circle->centroid - closestVertex;
            if(glm::length2(circleAxis) > 0.0001f) axes.push_back(glm::normalize(circleAxis));

            bool notColliding = false;
            for(glm::vec2& axis : axes)
            {
                project(&axis, polygon->points, polygon->numVertices);
                float pMin = minMax[0];
                float pMax = minMax[1];
                float cProj = glm::dot(axis, circle->centroid);
                float cMin = cProj - circle->furthestDistance;
                float cMax = cProj + circle->furthestDistance;
                float right = cMax - pMin;
                float left = pMax - cMin;
                if(left < 0 || right < 0)
                {
                    notColliding = true;
                    break;
                }

                float overlap = std::min(left, right);
                if(overlap < minOverlap)
                {
                    minOverlap = overlap;
                    smallestAxis = (right < left) ? -axis : axis; 
                }
            }

            if(!notColliding)
            {
                if(glm::dot(smallestAxis, test->centroid - centroid) < 0) smallestAxis *= -1;
                glm::vec2 cp = (shape == shapeType::CIRCLE) ? centroid + smallestAxis * furthestDistance : test->centroid - smallestAxis * test->furthestDistance;
                collisionCallback(engine::entities[id], engine::entities[test->id], smallestAxis, minOverlap, 1, cp, cp);
                if(test->collisionCallback) test->collisionCallback(engine::entities[test->id], engine::entities[id], -smallestAxis, minOverlap, 1, cp, cp);
            }

            continue;
        }

        std::vector<glm::vec2> axes;
        std::set<std::pair<float, float>> currentAxes;
        for(int i = 0; i < numVertices; i++)
        {
            glm::vec2 v1(points[i].x, points[i].y);
            int next = i + 1;
            if(next >= numVertices) next = 0;
            glm::vec2 v2(points[next].x, points[next].y);
            glm::vec2 normal = v1 - v2;
            normal = glm::normalize(glm::vec2(-normal.y, normal.x));
            normal = glm::vec2(std::round(normal.x * 10000.0f) / 10000.0f, std::round(normal.y * 10000.0f) / 10000.0f);
            if(currentAxes.find(std::pair<float, float>(normal.x, normal.y)) == currentAxes.end())
            {
                axes.push_back(normal);
                currentAxes.insert(std::pair<float, float>(-normal.x, -normal.y));
                currentAxes.insert(std::pair<float, float>(normal.x, normal.y));
            }
        }
        for(int i = 0; i < (*test).numVertices; i++)
        {
            glm::vec2 v1((*test).points[i].x, (*test).points[i].y);
            int next = i + 1;
            if(next >= (*test).numVertices) next = 0;
            glm::vec2 v2((*test).points[next].x, (*test).points[next].y);
            glm::vec2 normal = v1 - v2;
            normal = glm::normalize(glm::vec2(-normal.y, normal.x));
            normal = glm::vec2(std::round(normal.x * 10000.0f) / 10000.0f, std::round(normal.y * 10000.0f) / 10000.0f);
            if(currentAxes.find(std::pair<float, float>(normal.x, normal.y)) == currentAxes.end())
            {
                axes.push_back(normal);
                currentAxes.insert(std::pair<float, float>(-normal.x, -normal.y));
            }
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
        if(notColliding) 
        {
            continue;
        }

        edge edge1 = findEdge(points, numVertices, smallestAxis);
        edge edge2 = findEdge((*test).points, (*test).numVertices, -smallestAxis);
        glm::vec2 referenceVector;
        edge reference, incident;
        float firstAlignment = std::abs(glm::dot(glm::normalize(edge1.v2 - edge1.v1), smallestAxis));
        float secondAlignment = std::abs(glm::dot(glm::normalize(edge2.v2 - edge2.v1), smallestAxis));
        if(firstAlignment == secondAlignment)
        {
            if(id > (*test).id)
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
        }
        else if(firstAlignment < secondAlignment)
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
            continue;
        }
        offset = -glm::dot(referenceVector, reference.v2);
        glm::vec2 negReferenceVec = -referenceVector;
        clipped = clipPoints(&clipped.points[0], &clipped.points[1], &negReferenceVec, &offset);
        if(clipped.numPoints < 2) 
        {
            std::cout << "2 clipping failed" << std::endl;
            continue;
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
        collisionCallback(engine::entities[id], engine::entities[(*test).id], smallestAxis, minOverlap, clipped.numPoints, clipped.points[0], clipped.points[1]);
        if(test->collisionCallback) test->collisionCallback(engine::entities[test->id], engine::entities[id], -smallestAxis, minOverlap, clipped.numPoints, clipped.points[0], clipped.points[1]);
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
void polygonCollider::checkAABBCollisions(polygonCollider* second)
{
    glm::vec2 halfA = (*baseScale * scaleOffset) / 2.0f;
    glm::vec2 halfB = (*(*second).baseScale * (*second).scaleOffset) / 2.0f;
    glm::vec2 delta = (*second).centroid - centroid;
    if(halfA.x + halfB.x <= std::abs(delta.x) || halfA.y + halfB.y <= std::abs(delta.y))
    {
        return;
    }
    glm::vec2 maxA = centroid + halfA;
    glm::vec2 minA = centroid - halfA;
    glm::vec2 maxB = (*second).centroid + halfB;
    glm::vec2 minB = (*second).centroid - halfB;
    glm::vec2 normals[4] = {glm::vec2(-1, 0), glm::vec2(1, 0), glm::vec2(0, -1), glm::vec2(0, 1)};
    glm::vec2 normal;
    float distances[4] = {maxB.x - minA.x, maxA.x - minB.x, maxB.y - minA.y, maxA.y - minB.y};
    float penetration = distances[0];
    normal = normals[0];
    for(int i = 1; i < 4; i++)
    {
        if(distances[i] < penetration)
        {
            penetration = distances[i];
            normal = normals[i];
        }
    }
    collisionCallback(engine::entities[id], engine::entities[(*second).id], normal, penetration, 0, centroid, (*second).centroid);
    if(second->collisionCallback) second->collisionCallback(engine::entities[second->id], engine::entities[id], -normal, penetration, 0, second->centroid, centroid);
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
void polygonCollider::setRadius(float r)
{
    radius = r;
    updatePoints();
    updateFurthestPoint();
}
void polygonCollider::updateFurthestPoint()
{
    if(shape == shapeType::CIRCLE)
    {
        furthestDistance = radius * baseScale->x * scaleOffset.x;
        return;
    }
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
    glUseProgram(debugShaderProgram);
    if(shape == shapeType::CIRCLE)
    {
        debugPoint.setPosition(centroid.x, centroid.y);
        debugPoint.setLayer(setup::maxLayers - 1);
        debugPoint.size = 6;
        debugPoint.render();

        float currentRadius = radius * (*baseScale).x * scaleOffset.x;
        float currentRot = *baseRotation + rotationOffset;
        int debugSegments = 16;
        
        for(int i = 0; i < debugSegments; i++)
        {
            float theta = 2.0f * M_PI * ((float)i / debugSegments);
            float angle = currentRot + theta;
            
            debugPoint.setPosition(centroid.x + std::cos(angle) * currentRadius, 
                                   centroid.y + std::sin(angle) * currentRadius);
            
            if (i == 0) 
            {
                debugPoint.setColor(glm::vec3(1.0f, 0.0f, 0.0f)); 
                debugPoint.size = 8;
            } 
            else 
            {
                debugPoint.setColor(glm::vec3(0.0f, 1.0f, 1.0f)); 
                debugPoint.size = 6;
            }
            
            debugPoint.render();
        }
        return;
    }
    for(int i = 0; i < numVertices; i++)
    {
        debugPoint.setPosition(points[i].x, points[i].y);
        debugPoint.setLayer(setup::maxLayers - 1);
        debugPoint.size = 6;
        debugPoint.render();
    }
}