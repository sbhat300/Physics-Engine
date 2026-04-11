#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <unordered_map>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <camera/camera2D.h>

struct inputState
{
    bool pressed = false;
    bool down = false;
    bool released = false;
    bool up = true;
};

namespace inputHandler 
{
    extern std::unordered_map <unsigned int, inputState> buttons;
    extern std::vector<unsigned int> trackedKeys;
    extern glm::vec2 mousePos;
    extern glm::vec2 worldMousePos;
    void trackKeys(unsigned int key);
    void trackKeys(unsigned int* keys, int count);
    void update(GLFWwindow* window);
    void update(GLFWwindow* window, float windowHeight);
    void update(GLFWwindow* window, camera2D* cam, float windowHeight);
};

#endif