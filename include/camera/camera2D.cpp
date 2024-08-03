#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera/camera2D.h"

camera2D::camera2D(glm::vec3 position, glm::vec3 up, float camSpeed)
{
    camPos = position;
    worldUp = up;
    speed = camSpeed;
    camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camRight = glm::normalize(glm::cross(camFront, worldUp));
    camUp = worldUp;
}
glm::mat4 camera2D::GetViewMatrix()
{
    return glm::lookAt(camPos, camPos + camFront, camUp);
}
void camera2D::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = speed * deltaTime;
    if (direction == LEFT)
        camPos -= camRight * velocity;
    if (direction == RIGHT)
        camPos += camRight * velocity;
    if (direction == DOWN)
        camPos -= worldUp * velocity;
    if (direction == UP)
        camPos += worldUp * velocity;
}

// class camera2D
// {
//     public:
//         glm::vec3 camPos;
//         glm::vec3 camFront;
//         glm::vec3 camUp;
//         glm::vec3 camRight;
//         glm::vec3 worldUp;
//         float speed;
//         camera2D(glm::vec3 position = glm::vec3(0, 0, 1), glm::vec3 up = glm::vec3(0, 1, 0), float camSpeed = SPEED)
//         {
//             camPos = position;
//             worldUp = up;
//             speed = camSpeed;
//             camFront = glm::vec3(0.0f, 0.0f, -1.0f);
//             camRight = glm::normalize(glm::cross(camFront, worldUp));
//             camUp = worldUp;
//         }
//         glm::mat4 GetViewMatrix()
//         {
//             return glm::lookAt(camPos, camPos + camFront, camUp);
//         }
//         void ProcessKeyboard(Camera_Movement direction, float deltaTime)
//         {
//             float velocity = speed * deltaTime;
//             if (direction == LEFT)
//                 camPos -= camRight * velocity;
//             if (direction == RIGHT)
//                 camPos += camRight * velocity;
//             if (direction == DOWN)
//                 camPos -= worldUp * velocity;
//             if (direction == UP)
//                 camPos += worldUp * velocity;
//         }
// };