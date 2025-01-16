#include <Engine/inputHandler.h>
#include <iostream>

std::unordered_map<unsigned int, inputState> inputHandler::buttons = std::unordered_map<unsigned int, inputState>();
std::vector<unsigned int> inputHandler::trackedKeys = std::vector<unsigned int>();
glm::vec2 inputHandler::mousePos = glm::vec2(0, 0);
glm::vec2 inputHandler::worldMousePos = glm::vec2(0, 0);

void inputHandler::trackKeys(unsigned int key)
{
    trackedKeys.push_back(key);
    buttons[key] = inputState();
}
void inputHandler::trackKeys(unsigned int* keys, int count)
{
    for(int i = 0; i < count; i++) 
    {
        trackedKeys.push_back(*(keys + i));
        buttons[*(keys + i)] = inputState();
    }
}

void inputHandler::update(GLFWwindow* window)
{
    for(unsigned int key : trackedKeys)
    {
        if(key <= GLFW_MOUSE_BUTTON_8)
        {
            if(glfwGetMouseButton(window, key))
            {
                if(!buttons[key].down) buttons[key].pressed = true;
                else buttons[key].pressed = false;
                buttons[key].down = true;
                buttons[key].up = false;
                buttons[key].released = false;
            }
            else
            {
                if(!buttons[key].up) buttons[key].released = true;
                else buttons[key].released = false;
                buttons[key].up = true;
                buttons[key].down = false;
                buttons[key].pressed = false;
            }
            continue;
        }
        if(glfwGetKey(window, key) == GLFW_PRESS)
        {
            if(!buttons[key].down) buttons[key].pressed = true;
            else buttons[key].pressed = false;
            buttons[key].down = true;
            buttons[key].up = false;
            buttons[key].released = false;
        }
        else
        {
            if(!buttons[key].up) buttons[key].released = true;
            else buttons[key].released = false;
            buttons[key].up = true;
            buttons[key].down = false;
            buttons[key].pressed = false;
        }
    }
}

void inputHandler::update(GLFWwindow* window, float windowHeight)
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    mousePos = glm::vec2(xPos, windowHeight - yPos);
    update(window);
}

void inputHandler::update(GLFWwindow* window, camera2D* cam, float windowHeight)
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    worldMousePos = glm::vec2(cam->camPos.x + xPos, cam->camPos.y + (windowHeight - yPos));
    mousePos = glm::vec2(xPos, windowHeight - yPos);
    update(window);
}