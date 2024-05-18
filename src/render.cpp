#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <Shader/shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <camera/camera2D.h>
#include <Objects/polygon.h>
#include <entity.h>
#include <Physics/ray.h>
#include <Objects/point.h>
#include <list>
#include <map>
#include <Physics/ray.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void processWireframeChange(GLFWwindow* window);
void updateDeltaTime();
void configureShader(Shader& shader);
void setCamSettings();
void collisionCallback(int first, int second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void bufferMatrices(int ubo);

float windowHeight = 600, windowWidth = 800;
int maxLayers = 10;
camera2D camera(glm::vec3(0, 0, maxLayers));
float deltaTime = 0.0f, lastFrame = 0.0f;
int counter = 0;

std::map<int, entity*> entities;

entity rect(glm::vec2(0, 0), glm::vec2(1, 1), 0, &entities, &counter);
point p(0, 0, 1);
ray r(glm::vec2(0, 10), glm::vec2(1, 0), 5);

int main() {
    rect.addPolygon(glm::vec2(0, 0), glm::vec2(1, 1), 0);
    rect.polygonInstance.initRectangle();
    p.setColor(glm::vec3(1, 1, 1));
    p.setLayer(3);
    // rect.polygonInstance.initPolygon(4, tempVertices, 6, indices);
    rect.polygonInstance.setLayer(1);
    rect.polygonInstance.setColor(glm::vec3(0.5f, 0.5f, 0.7f));
    rect.setScale(40, 40);

    r.layer = 1;

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

    glEnable(GL_PROGRAM_POINT_SIZE);  
    glEnable(GL_DEPTH_TEST);  

    

    Shader shader("D:/Physics-Engine/shaders/gravityVShader.glsl", "D:/Physics-Engine/shaders/gravityFShader.glsl");
    Shader pointShader("D:/Physics-Engine/shaders/pointVShader.glsl", "D:/Physics-Engine/shaders/gravityFShader.glsl");
    Shader rayShader("D:/Physics-Engine/shaders/rayVShader.glsl", "D:/Physics-Engine/shaders/gravityFShader.glsl");
    configureShader(shader);
    configureShader(pointShader);
    configureShader(rayShader);

    unsigned int matrixUBO;
    glGenBuffers(1, &matrixUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matrixUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, matrixUBO, 0, 2 * sizeof(glm::mat4));
    glm::mat4 projection = glm::ortho(0.0f, windowWidth, 0.0f, windowHeight, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    setCamSettings();

    while (!glfwWindowShouldClose(window))
    {
        updateDeltaTime();
        processInput(window);
        processWireframeChange(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bufferMatrices(matrixUBO);
        shader.use();
        rect.polygonInstance.render();
        pointShader.use();
        p.render();
        rayShader.use();
        r.length += deltaTime * 10;
        r.bufferNewData();
        r.render();
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
        rect.setRotation(rect.rotation + 50 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        rect.setRotation(rect.rotation - 50 * deltaTime);   
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
void bufferMatrices(int ubo)
{
    glm::mat4 view = camera.GetViewMatrix();
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void configureShader(Shader& shader)
{
    shader.use();
    unsigned int uniformBlock = glGetUniformBlockIndex(shader.ID, "Matrices");
    glUniformBlockBinding(shader.ID, uniformBlock, 0);
}
void setCamSettings()
{
    camera.camPos = glm::vec3(-windowWidth / 2, -windowHeight / 2, 10);
    camera.speed = 200.0f;
}
void collisionCallback(int first, int second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2)
{
    std::cout << glm::to_string(collisionNormal) << " " << contactPoints << std::endl;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    rect.setPosition(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos));
    // std::cout << rect.pointInPolygon(glm::vec2(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos))) << std::endl;
}