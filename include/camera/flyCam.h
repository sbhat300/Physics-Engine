#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <math.h>       

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

//default values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.15f;
const float FOV =  45.0f;

class flyCam
{
    public:
        glm::vec3 camPos;
        glm::vec3 camFront;
        glm::vec3 camUp;
        glm::vec3 camRight;
        glm::vec3 worldUp;
        float pitch;
        float yaw;
        float fov;
        float sens;
        float speed;
        flyCam(glm::vec3 position = glm::vec3(0, 0, 0), glm::vec3 up = glm::vec3(0, 1, 0), float camYaw = YAW, float camPitch = PITCH, float sensitivity = SENSITIVITY, float camSpeed = SPEED, float camFov = FOV)
        {
            camPos = position;
            worldUp = up;
            yaw = camYaw;
            pitch = camPitch;
            sens = sensitivity;
            speed = camSpeed;
            fov = camFov;
        }
        glm::mat4 GetViewMatrix()
        {
            return glm::lookAt(camPos, camPos + camFront, camUp);
        }
        void ProcessMouseScroll(float yoffset)
        {
            fov -= (float)yoffset;
            if (fov < 1.0f)
                fov = 1.0f;
            if (fov > 90.0f)
                fov = 90.0f;
        }
        void ProcessMouseMovement(float xoffset, float yoffset)
        {
            xoffset *= sens;
            yoffset *= sens;

            yaw += xoffset;
            pitch += yoffset;

            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
            yaw = fmod(yaw, (float)360);

            UpdateCameraVectors();
        }
        void ProcessKeyboard(Camera_Movement direction, float deltaTime)
        {
            float velocity = speed * deltaTime;
            if (direction == FORWARD)
                camPos += camFront * velocity;
            if (direction == BACKWARD)
                camPos -= camFront * velocity;
            if (direction == LEFT)
                camPos -= camRight * velocity;
            if (direction == RIGHT)
                camPos += camRight * velocity;
            if (direction == DOWN)
                camPos -= worldUp * velocity;
            if (direction == UP)
                camPos += worldUp * velocity;
        }
    private:
    void UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        camFront = glm::normalize(front);
        camRight = glm::normalize(glm::cross(camFront, worldUp));
        camUp = glm::normalize(glm::cross(camRight, camFront));
    }
};

#endif