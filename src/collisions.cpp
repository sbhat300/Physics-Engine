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
#include <unordered_map>
#include <math.h>
#include <Physics/rayData.h>
#include <Physics/spatialHashGrid.h>

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

std::unordered_map<int, entity*> entities;

entity bottomFloor(glm::vec2(20, -5), glm::vec2(10, 10), 0, &entities, &counter);
entity rect(glm::vec2(-200, -300), glm::vec2(40, 40), 0, &entities, &counter);
entity rect2(glm::vec2(0, -300), glm::vec2(40, 100), 0, &entities, &counter);


ray r(glm::vec2(0, 0), glm::vec2(1, 0), 60, &entities);
point rDebugPoint(0, 0, 6);

spatialHashGrid grid(500, 500, glm::vec2(4, 4), glm::vec2(-300, -400));

int main() {
    rect.addPolygon();
    rect.polygonInstance.initRectangle();
    rect.polygonInstance.setColor(glm::vec3(0.5f, 0.5f, 0.7f));
    rect.polygonInstance.setLayer(1);
    bottomFloor.addPolygon();
    bottomFloor.polygonInstance.initRectangle();
    bottomFloor.polygonInstance.setColor(glm::vec3(0.8f, 0.4f, 0.6f));
    rect2.addPolygon();
    rect2.polygonInstance.initRectangle();
    rect2.polygonInstance.setColor(glm::vec3(0.2f, 0.4f, 0.3f));

    rect.addPolygonCollider(&grid);
    rect.polygonColliderInstance.initRectangle();
    bottomFloor.addPolygonCollider(&grid);
    bottomFloor.polygonColliderInstance.initRectangle();
    rect2.addPolygonCollider(&grid);
    rect2.polygonColliderInstance.initRectangle();

    rect.polygonColliderInstance.setPositionOffset(40, 40);
    rect.polygonInstance.setPositionOffset(40, 40);
    rect.polygonColliderInstance.setCollisionCallback(collisionCallback);
    bottomFloor.polygonColliderInstance.collide = false;

    r.layer = 1;

    rDebugPoint.setColor(glm::vec3(1, 1, 1));
    rDebugPoint.setLayer(2);

    grid.setColor(glm::vec3(1, 0.5f, 1));
    grid.setLayer(0);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

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

    Shader shader("/Users/shreyas/Documents/GitHub/Physics-Engine/shaders/gravityVShader.glsl", "/Users/shreyas/Documents/GitHub/Physics-Engine/shaders/gravityFShader.glsl");
    Shader pointShader("/Users/shreyas/Documents/GitHub/Physics-Engine/shaders/pointVShader.glsl", "/Users/shreyas/Documents/GitHub/Physics-Engine/shaders/gravityFShader.glsl");
    Shader rayShader("/Users/shreyas/Documents/GitHub/Physics-Engine/shaders/rayVShader.glsl", "/Users/shreyas/Documents/GitHub/Physics-Engine/shaders/gravityFShader.glsl");


    configureShader(shader);
    configureShader(pointShader);
    configureShader(rayShader);
    rect.polygonColliderInstance.debugShaderProgram = pointShader.ID;

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
        bottomFloor.polygonInstance.render();
        rect2.polygonInstance.render();
        rect.polygonColliderInstance.updateCollider();
        bottomFloor.polygonColliderInstance.updateCollider();
        rect2.polygonColliderInstance.updateCollider();
        rayShader.use();
        r.render();
        grid.drawGrid();
        std::vector<rayData> rdata = r.getCollisions();
        // std::pair<bool, rayData> rdata2 = r.getFirstCollision();
        rect.polygonColliderInstance.renderColliderBounds();
        // if(rdata2.first)
        // {
        //     rDebugPoint.setPosition(rdata2.second.collisionPoint.x, rdata2.second.collisionPoint.y);
        //     rDebugPoint.render();
        // }
        for(int i = 0; i < rdata.size(); i++) 
        {
            rDebugPoint.setPosition(rdata[i].collisionPoint.x, rdata[i].collisionPoint.y);
            rDebugPoint.render();
        }
        // std::cout << &(rect.polygonColliderInstance) << " " << &(rect2.polygonColliderInstance) << " " << &(bottomFloor.polygonColliderInstance) << std::endl;
        std::vector<polygonCollider*> b = grid.getNearby(&(rect.polygonColliderInstance));
        for(auto i = b.begin(); i != b.end(); i++)
        {
            std::cout << (*(*i)).id << std::endl;
        }
        // std::cout << std::endl;
        // std::pair<int, int> a = grid.getCellIndex(rect.position.x, rect.position.y);
        // for(auto i : grid.grid[a.first][a.second])
        // {
        //     std::cout << (*i).id << std::endl;
        // }
        // std::cout << std::endl;
        // std::cout << a.first << " " << a.second << std::endl;
        rect2.setPosition(rect2.position.x, -300 + 100 * sin(glfwGetTime()));
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
    // std::cout << glm::to_string(collisionNormal) << " " << contactPoints << std::endl;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    rect.setPosition(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos));
    // std::cout << rect.pointInPolygon(glm::vec2(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos))) << std::endl;
}