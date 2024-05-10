#ifndef RECTANGLECOLLIDER_H
#define RECTANGLECOLLIDER_H
#include <Objects/rectangle.h>
#include <Objects/point.h>
#include <iostream>
#include <map>
#include <functional>
#include <vector>
#include <algorithm>  

class rectangleCollider : public rectangle
{
    public:
        int id;
        float* deltaTime;
        bool collide;
        std::map<int, rectangleCollider*>* collidables;
        std::function<void(int, int, glm::vec2, float, int, glm::vec2, glm::vec2)> collisionCallback;
        int debugShaderProgram;

        rectangleCollider(float h = 1, float w = 1, float r = 0, glm::vec2 s = glm::vec2(1, 1), glm::vec3 p = glm::vec3(0, 0, 0), float* dt = 0, int* counter = 0, 
                            std::map<int, rectangleCollider*>* cs = 0) : rectangle(h, w, r, s, p)
        {
            deltaTime = dt;
            collidables = cs;
            id = *counter;
            *counter += 1;
            (*cs)[id] = this;
            collide = false;
            debugPoint.setColor(glm::vec3(1.0f, 1.0f, 0.0f));
            updatePoints();
        }
        void updateCollider()
        {
            if(collide) checkCollisions();
        }
        void setCollisionCallback(std::function<void(int, int, glm::vec2, float, int, glm::vec2, glm::vec2)> cb)
        {
            collisionCallback = cb;
            collide = true;
        }
        bool pointInPolygon(glm::vec2 point)
        {
            float neg = 0, pos = 0;
            for(int i = 0; i < vertices; i++)
            {
                int vNext = i + 1;
                if(vNext == vertices) vNext = 0;
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
    private:
        point debugPoint = point(0, 0, 3);
        struct edge {
            glm::vec2 v1;
            glm::vec2 v2;
            glm::vec2 vMax;
        };
        struct clippedPoints{
            glm::vec2 points[2];
            int numPoints;
        };
        void checkCollisions()
        {
            if(!collisionCallback) return;
            for(auto i = (*collidables).begin(); i != (*collidables).end(); i++)
            {
                if(i->second->id == id) continue;
                float minOverlap = FLT_MAX;
                glm::vec2 smallestAxis;
                rectangleCollider test = *(i->second);
                std::vector<glm::vec2> axes(vertices + test.vertices);
                for(int i = 0; i < vertices; i++)
                {
                    glm::vec2 v1(points[i].x, points[i].y);
                    int next = i + 1;
                    if(next >= vertices) next = 0;
                    glm::vec2 v2(points[next].x, points[next].y);
                    glm::vec2 normal = v1 - v2;
                    normal = glm::normalize(glm::vec2(-normal.y, normal.x));
                    axes[i] = normal;
                }
                for(int i = 0; i < test.vertices; i++)
                {
                    glm::vec2 v1(test.points[i].x, test.points[i].y);
                    int next = i + 1;
                    if(next >= test.vertices) next = 0;
                    glm::vec2 v2(test.points[next].x, test.points[next].y);
                    glm::vec2 normal = v1 - v2;
                    normal = glm::normalize(glm::vec3(-normal.y, normal.x, 0));
                    axes[vertices + i] = normal;
                }
                int notColliding = false;
                for(int i = 0; i < axes.size(); i++)
                {
                    project(&axes[i], points, vertices);
                    float one[2] = {minMax[0], minMax[1]};
                    project(&axes[i], test.points, test.vertices);
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
                edge edge1 = findEdge(points, vertices, smallestAxis);
                edge edge2 = findEdge(test.points, test.vertices, -smallestAxis);
                glm::vec2 referenceVector;
                edge reference, incident;
                bool flip = false;
                if(abs(glm::dot(glm::normalize(edge1.v2 - edge1.v1), smallestAxis)) <= glm::dot(glm::normalize(edge2.v2 - edge2.v1), smallestAxis))
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
                    flip = true;
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
                    debugPoint.layer = 2;
                    debugPoint.setPosition(clipped.points[i].x, clipped.points[i].y);
                    debugPoint.render();
                }
                collisionCallback(id, test.id, smallestAxis, minOverlap, clipped.numPoints, clipped.points[0], clipped.points[1]);
            }
        }
        float minMax[2];
        void project(glm::vec2* axis, glm::vec3* vertices, int numVertices)
        {
            minMax[0] = glm::dot(glm::vec3(*axis, 0), *(vertices + 0));
            minMax[1] = minMax[0];
            for(int i = 1; i < numVertices; i++)
            {
                float projected = glm::dot(glm::vec3(*axis, 0), *(vertices + i));
                if(projected < minMax[0]) minMax[0] = projected;
                if(projected > minMax[1]) minMax[1] = projected;
            }
        }
        edge findEdge(glm::vec3* points, int numVertices, glm::vec2 collisionNormal)
        {
            float max = glm::dot(glm::vec3(collisionNormal, 0), *(points + 0));
            int index = 0;
            for(int i = 1; i < numVertices; i++)
            {
                float proj = glm::dot(glm::vec3(collisionNormal, 0), *(points + i));
                if(proj > max)
                {
                    index = i;
                    max = proj;
                }
            }

            glm::vec3 v1 = *(points + index);
            glm::vec3 vNext;
            glm::vec3 vPrev;
            if(index + 1 == numVertices) vNext = *(points + 0);
            else vNext = *(points + index + 1);
            if(index - 1 == -1) vPrev = *(points + numVertices - 1);
            else vPrev = *(points + index - 1);
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

            //DEBUG
            // glUseProgram(debugShaderProgram);
            // debugPoint.layer = 1;
            // debugPoint.setPosition(output.v1.x, output.v1.y);
            // debugPoint.render();
            // debugPoint.setPosition(output.v2.x, output.v2.y);
            // debugPoint.render();
            
            return output;
        }
        clippedPoints clipPoints(glm::vec2* point1, glm::vec2* point2, glm::vec2* referenceEdge, float* offset)
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

};
#endif