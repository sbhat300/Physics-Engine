#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <Shader/shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <camera/camera2D.h>
#include <Physics/rectangleCollider.h>
#include <Objects/rectangle.h>
#include <list>
#include <map>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void processWireframeChange(GLFWwindow* window);
void updateDeltaTime();
void configureShader(Shader& shader);
void setCamSettings();
void collisionCallback(int first, int second, glm::vec3 collisionNormal, float penetrationDepth);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

float windowHeight = 600, windowWidth = 800;
camera2D camera(glm::vec3(0, 0, 1));
float deltaTime = 0.0f, lastFrame = 0.0f;
int counter = 0;

std::map<int, rectangleCollider*> rectangles;

rectangleCollider bottomFloor(1, 1, 0, glm::vec3(40, 40, 40), glm::vec3(0, -300, 0), &deltaTime, &counter, &rectangles);
rectangleCollider rect(1, 1, 0, glm::vec3(40, 40, 40), glm::vec3(-200, -300, 0), &deltaTime, &counter, &rectangles);

int main() {
    rect.setCollisionCallback(collisionCallback);
    bottomFloor.collide = false;
    rect.setColor(glm::vec3(0.3f, 0.5f, 0.7f));
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "EPIC OPENGL PROJECT", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    Shader shader("D:/Physics-Engine/shaders/gravityVShader.glsl", "D:/Physics-Engine/shaders/gravityFShader.glsl");
    setCamSettings();

    while (!glfwWindowShouldClose(window))
    {
        updateDeltaTime();
        processInput(window);
        processWireframeChange(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        configureShader(shader);
        rect.updateCollider();
        bottomFloor.updateCollider();
        rect.render();
        bottomFloor.render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        rect.rotation += 50 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        rect.rotation -= 50 * deltaTime;
    
}
void processWireframeChange(GLFWwindow* window) 
{
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void updateDeltaTime()
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}
void configureShader(Shader& shader)
{
    shader.use();
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("view", view);
    glm::mat4 projection = glm::ortho(0.0f, windowWidth, 0.0f, windowHeight, 0.1f, 100.0f);
    shader.setMat4("projection", projection);
}
void setCamSettings()
{
    camera.camPos = glm::vec3(-windowWidth / 2, -windowHeight / 2, 1);
    camera.speed = 200.0f;
}
void collisionCallback(int first, int second, glm::vec3 collisionNormal, float penetrationDepth)
{
    std::cout << glm::to_string(collisionNormal * penetrationDepth) << std::endl;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    rect.setPosition(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos));
}