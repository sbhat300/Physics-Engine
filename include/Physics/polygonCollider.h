// #ifndef POLYGONCOLLIDER_H
// #define POLYGONCOLLIDER_H
// class entity;
// #include <entity.h>
// #include <Objects/point.h>
// #include <iostream>
// #include <map>
// #include <functional>
// #include <vector>
// #include <algorithm>  

// class polygonCollider
// {
//     public:
//         int id;
//         bool collide;
//         std::map<int, entity*>* entities;
//         std::function<void(int, int, glm::vec2, float, int, glm::vec2, glm::vec2)> collisionCallback;
//         int debugShaderProgram;
//         std::vector<glm::vec2> points;
//         std::vector<float> vertices;
//         int numVertices;
//         float rotationOffset;
//         glm::vec2 scaleOffset, positionOffset;
//         glm::vec2* basePosition;
//         glm::vec2* baseScale;
//         float* baseRotation;
//         bool initialized;

//         polygonCollider(glm::vec2 p = glm::vec2(0, 0), glm::vec2 s = glm::vec2(1, 1), float r = 0, 
//                             std::map<int, entity*>* cs = 0)
//         {
//             initialized = false;
//             entities = cs;
//             collide = false;
//             positionOffset = p;
//             scaleOffset = s;
//             rotationOffset = r;
//             debugPoint.setColor(glm::vec3(1.0f, 1.0f, 0.0f));
//             updatePoints();
//         }
//         void updateCollider()
//         {
//             if(collide) checkCollisions();
//         }
//         void setCollisionCallback(std::function<void(int, int, glm::vec2, float, int, glm::vec2, glm::vec2)> cb)
//         {
//             collisionCallback = cb;
//             collide = true;
//         }
//         bool pointInPolygon(glm::vec2 point)
//         {
//             float neg = 0, pos = 0;
//             for(int i = 0; i < numVertices; i++)
//             {
//                 int vNext = i + 1;
//                 if(vNext == numVertices) vNext = 0;
//                 glm::vec2 perpVec = glm::normalize(points[vNext] - points[i]);
//                 perpVec = glm::vec2(-perpVec.y, perpVec.x);
//                 float side = glm::dot(perpVec, point - glm::vec2(points[i]));
//                 if(side == 0) continue;
//                 if(side < 0) neg++;
//                 else pos++;
//                 if(neg > 0 && pos > 0) return false;
//             }
//             return true;
//         }
//         void initPolygon(int vertexCount, float* p)
//         {
//             numVertices = vertexCount;
//             vertices.reserve(vertexCount * 3);
//             points.reserve(vertexCount);
//             int extraOffset = 0;
//             for(int i = 0; i < vertexCount * 2; i += 2)
//             {
//                 vertices[i + extraOffset] = *(p + i);
//                 vertices[i + 1 + extraOffset] = *(p + i + 1);
//                 vertices[i + 2 + extraOffset] = 1;
//                 extraOffset++;
//             }
//             normalizePoints();
//             updatePoints();
//             initialized = true;
//         }
//         void initRectangle()
//         {
//             float p[] = {
//                 1 / 2.0f,  1 / 2.0f,  // top right
//                 -1 / 2.0f,  1 / 2.0f,   // top left
//                 -1 / 2.0f, -1 / 2.0f,  // bottom left 
//                 1 / 2.0f, -1 / 2.0f  // bottom right
//             };  
//             vertices.reserve(12);
//             points.reserve(4);
//             numVertices = 4;
//             int extraOffset = 0;
//             for(int i = 0; i < numVertices * 2; i += 2)
//             {
//                 vertices[i + extraOffset] = *(p + i);
//                 vertices[i + 1 + extraOffset] = *(p + i + 1);
//                 vertices[i + 2 + extraOffset] = 1;
//                 extraOffset++;
//             }
//             normalizePoints();
//             updatePoints();
//             initialized = true;
//         }
//         void updatePoints()
//         {
//             for(int i = 0; i < 12; i += 3)
//             {
//                 glm::vec2 point = glm::vec2(vertices[i], vertices[i + 1]);
//                 points[i / 3] = point * *baseScale * scaleOffset;
//                 float temp[2] = {points[i / 3].x, points[i / 3].y};
//                 points[i / 3].x = temp[0] * cos(glm::radians(-(*baseRotation + rotationOffset))) - temp[1] * sin(glm::radians(-(*baseRotation + rotationOffset)));
//                 points[i / 3].y = temp[0] * sin(glm::radians(-(*baseRotation + rotationOffset))) + temp[1] * cos(glm::radians(-(*baseRotation + rotationOffset)));
//                 points[i / 3].x += (positionOffset + *basePosition).x;
//                 points[i / 3].y += (positionOffset + *basePosition).y;
//             }
//         }
//         void normalizePoints()
//         {
//             glm::vec2 centroid(0, 0);
//             for(int i = 0; i < numVertices * 3; i += 3)
//             {
//                 centroid.x += vertices[i];
//                 centroid.y += vertices[i + 1];
//             }
//             centroid.x /= numVertices;
//             centroid.y /= numVertices;
//             for(int i = 0; i < numVertices * 3; i += 3)
//             {
//                 vertices[i] -= centroid.x;
//                 vertices[i + 1] -= centroid.y;
//             }
//             float area = 0;
//             for(int i = 0; i < numVertices * 3; i += 3)
//             {
//                 int nextX = i + 3;
//                 int nextY = i + 4;
//                 if(i + 3 >= numVertices * 3)
//                 { 
//                     nextX = 0;
//                     nextY = 1;
//                 }
//                 area += vertices[i] * vertices[nextY] - vertices[nextX] * vertices[i + 1];
//             }
//             area /= 2;
//             float scale = 1 / std::sqrt(area);
//             for(int i = 0; i < numVertices * 3; i += 3)
//             {
//                 vertices[i] *= scale;;
//                 vertices[i + 1] *= scale;
//             }
//         }
//     private:
//         point debugPoint = point(0, 0, 3);
//         struct edge {
//             glm::vec2 v1;
//             glm::vec2 v2;
//             glm::vec2 vMax;
//         };
//         struct clippedPoints{
//             glm::vec2 points[2];
//             int numPoints;
//         };
//         void checkCollisions()
//         {
//             if(!collisionCallback) return;
//             for(auto i = (*entities).begin(); i != (*entities).end(); i++)
//             {
//                 if(!(*(*i).second).contain[1]) continue;
//                 if(i->second->id == id) continue;
//                 float minOverlap = FLT_MAX;
//                 glm::vec2 smallestAxis;
//                 polygonCollider test = (*(*i).second).polygonColliderInstance;
//                 std::vector<glm::vec2> axes(numVertices + test.numVertices);
//                 for(int i = 0; i < numVertices; i++)
//                 {
//                     glm::vec2 v1(points[i].x, points[i].y);
//                     int next = i + 1;
//                     if(next >= numVertices) next = 0;
//                     glm::vec2 v2(points[next].x, points[next].y);
//                     glm::vec2 normal = v1 - v2;
//                     normal = glm::normalize(glm::vec2(-normal.y, normal.x));
//                     axes[i] = normal;
//                 }
//                 for(int i = 0; i < test.numVertices; i++)
//                 {
//                     glm::vec2 v1(test.points[i].x, test.points[i].y);
//                     int next = i + 1;
//                     if(next >= test.numVertices) next = 0;
//                     glm::vec2 v2(test.points[next].x, test.points[next].y);
//                     glm::vec2 normal = v1 - v2;
//                     normal = glm::normalize(glm::vec3(-normal.y, normal.x, 0));
//                     axes[numVertices + i] = normal;
//                 }
//                 int notColliding = false;
//                 for(int i = 0; i < axes.size(); i++)
//                 {
//                     project(&axes[i], points, numVertices);
//                     float one[2] = {minMax[0], minMax[1]};
//                     project(&axes[i], test.points, test.numVertices);
//                     float two[2] = {minMax[0], minMax[1]};
//                     float right = two[1] - one[0];
//                     float left = one[1] - two[0];
//                     if(left < 0 || right < 0)
//                     {
//                         notColliding = true;
//                         break;
//                     }
//                     float overlap;
//                     if(right < left) overlap = right;
//                     else overlap = left;
//                     if(overlap < std::abs(minOverlap))
//                     {
//                         minOverlap = overlap;
//                         smallestAxis = axes[i];
//                         if(right <= left) smallestAxis *= -1;
//                     }
//                 }
//                 if(notColliding) continue;
//                 std::cout << "collsion" << std::endl;
//                 edge edge1 = findEdge(points, numVertices, smallestAxis);
//                 edge edge2 = findEdge(test.points, test.numVertices, -smallestAxis);
//                 glm::vec2 referenceVector;
//                 edge reference, incident;
//                 bool flip = false;
//                 if(abs(glm::dot(glm::normalize(edge1.v2 - edge1.v1), smallestAxis)) <= glm::dot(glm::normalize(edge2.v2 - edge2.v1), smallestAxis))
//                 {
//                     reference = edge1;
//                     referenceVector = glm::normalize(edge1.v2 - edge1.v1);
//                     incident = edge2;
//                 }
//                 else
//                 {
//                     reference = edge2;
//                     referenceVector = glm::normalize(edge2.v2 - edge2.v1);
//                     incident = edge1;
//                     flip = true;
//                 }

//                 float offset = glm::dot(referenceVector, reference.v1);
//                 clippedPoints clipped = clipPoints(&incident.v1, &incident.v2, &referenceVector, &offset);
//                 if(clipped.numPoints < 2) 
//                 {
//                     std::cout << "1 clipping failed" << std::endl;
//                     return;
//                 }
//                 offset = -glm::dot(referenceVector, reference.v2);
//                 glm::vec2 negReferenceVec = -referenceVector;
//                 clipped = clipPoints(&clipped.points[0], &clipped.points[1], &negReferenceVec, &offset);
//                 if(clipped.numPoints < 2) 
//                 {
//                     std::cout << "2 clipping failed" << std::endl;
//                     return;
//                 }
//                 glm::vec2 perpVec = glm::vec2(-referenceVector.y, referenceVector.x);
//                 float max = glm::dot(perpVec, reference.vMax);
//                 if(glm::dot(perpVec, clipped.points[0]) - max < 0)
//                 {
//                     clipped.numPoints--;
//                     clipped.points[0] = clipped.points[1];
//                 }
//                 if(glm::dot(perpVec, clipped.points[1]) - max < 0)
//                 {
//                     clipped.numPoints--;
//                 }
//                 //DEBUG
//                 for(int i = 0; i < clipped.numPoints; i++)
//                 {
//                     glUseProgram(debugShaderProgram);
//                     debugPoint.layer = 2;
//                     debugPoint.setPosition(clipped.points[i].x, clipped.points[i].y);
//                     debugPoint.render();
//                 }
//                 collisionCallback(id, test.id, smallestAxis, minOverlap, clipped.numPoints, clipped.points[0], clipped.points[1]);
//             }
//         }
//         float minMax[2];
//         void project(glm::vec2* axis, std::vector<glm::vec2> &vertices, int numVertices)
//         {
//             minMax[0] = glm::dot(glm::vec3(*axis, 0), glm::vec3(vertices[0], 0));
//             minMax[1] = minMax[0];
//             for(int i = 1; i < numVertices; i++)
//             {
//                 float projected = glm::dot(glm::vec3(*axis, 0), glm::vec3(vertices[i], 0));
//                 if(projected < minMax[0]) minMax[0] = projected;
//                 if(projected > minMax[1]) minMax[1] = projected;
//             }
//         }
//         edge findEdge(std::vector<glm::vec2> &points, int numVertices, glm::vec2 collisionNormal)
//         {
//             float max = glm::dot(glm::vec3(collisionNormal, 0), *(points + 0));
//             int index = 0;
//             for(int i = 1; i < numVertices; i++)
//             {
//                 float proj = glm::dot(glm::vec3(collisionNormal, 0), *(points + i));
//                 if(proj > max)
//                 {
//                     index = i;
//                     max = proj;
//                 }
//             }

//             glm::vec3 v1 = *(points + index);
//             glm::vec3 vNext;
//             glm::vec3 vPrev;
//             if(index + 1 == numVertices) vNext = *(points + 0);
//             else vNext = *(points + index + 1);
//             if(index - 1 == -1) vPrev = *(points + numVertices - 1);
//             else vPrev = *(points + index - 1);
//             glm::vec3 vNextLine = glm::normalize(v1 - vNext);
//             glm::vec3 vPrevLine = glm::normalize(v1 - vPrev);
//             edge output;
//             if(glm::dot(vPrevLine, glm::vec3(collisionNormal, 0)) <= glm::dot(vNextLine, glm::vec3(collisionNormal, 0)))
//             {
//                 output.v1 = vPrev;
//                 output.v2 = v1;
//             }
//             else 
//             {
//                 output.v1 = v1;
//                 output.v2 = vNext;
//             }
//             output.vMax = v1;

//             //DEBUG
//             // glUseProgram(debugShaderProgram);
//             // debugPoint.layer = 1;
//             // debugPoint.setPosition(output.v1.x, output.v1.y);
//             // debugPoint.render();
//             // debugPoint.setPosition(output.v2.x, output.v2.y);
//             // debugPoint.render();
            
//             return output;
//         }
//         clippedPoints clipPoints(glm::vec2* point1, glm::vec2* point2, glm::vec2* referenceEdge, float* offset)
//         {
//             clippedPoints output;
//             output.numPoints = 0;
//             float firstOffset = glm::dot(*point1, *referenceEdge) - *offset;
//             float secondOffset = glm::dot(*point2, *referenceEdge) - *offset;
//             if(firstOffset >= 0)
//             {
//                 output.points[output.numPoints] = *point1;
//                 output.numPoints++;
//             }
//             if(secondOffset >= 0)
//             {
//                 output.points[output.numPoints] = *point2;
//                 output.numPoints++;
//             }
//             if(secondOffset * firstOffset < 0)
//             {
//                 glm::vec2 clipEdge = *point2 - *point1;
//                 float scale = firstOffset / (firstOffset - secondOffset);
//                 clipEdge *= scale;
//                 clipEdge += *point1;
//                 output.points[output.numPoints] = clipEdge;
//                 output.numPoints++;
//             }
//             return output;
//         }

// };
// #endif