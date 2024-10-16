#include "mathFuncs.h"

float mathFuncs::cross(glm::vec2 first, glm::vec2 second)
{
    return first.x * second.y - first.y * second.x;
}
glm::vec2 mathFuncs::cross(glm::vec2 first, float second)
{
    return glm::vec2(first.y * second, -first.x * second);
}
glm::vec2 mathFuncs::cross(float first, glm::vec2 second)
{
    return glm::vec2(-first * second.y, first * second.x);
}