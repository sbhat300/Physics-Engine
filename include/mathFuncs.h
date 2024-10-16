#ifndef MATHFUNCS_H
#define MATHFUNCS_H
#include  <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace mathFuncs
{
    float cross(glm::vec2 first, glm::vec2 second);
    glm::vec2 cross(glm::vec2 first, float second);
    glm::vec2 cross(float first, glm::vec2 second);
};
#endif  