#ifndef CAMERA2D_H
#define CAMERA2D_H

#include <glm/glm.hpp>

enum Camera_Movement {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class camera2D
{
    public:
        glm::vec3 camPos;
        glm::vec3 camFront;
        glm::vec3 camUp;
        glm::vec3 camRight;
        glm::vec3 worldUp;
        float speed;
        camera2D(glm::vec3 position = glm::vec3(0, 0, 1), glm::vec3 up = glm::vec3(0, 1, 0), float camSpeed = 5);
        glm::mat4 GetViewMatrix();
        void ProcessKeyboard(Camera_Movement direction, float deltaTime);
};

#endif