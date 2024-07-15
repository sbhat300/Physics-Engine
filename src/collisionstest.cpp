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
#include <imgui/imguiInitialize.h>
#include <fstream>
#include <string>
#include <FileLoader/objDataLoader.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void updateDeltaTime();
void configureShader(Shader& shader);
void setCamSettings();
void collisionCallback(int first, int second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void bufferMatrices(int ubo);

float windowHeight = 600, windowWidth = 1200;
int maxLayers = 10;
camera2D camera(glm::vec3(0, 0, maxLayers));
float deltaTime = 0.0f, lastFrame = 0.0f;
int counter = 0;

std::fstream DataLoader::data = std::fstream("collisionsObjectData.txt", std::ios::out | std::ios::in | std::ios::trunc);
int DataLoader::previousDataPos = -1;
const char* DataLoader::name = "collisionsObjectData.txt";

std::unordered_map<int, entity*> entities;

/*-----ENTITY INITIALIZATION-----*/
entity bottomFloor("small rect", glm::vec2(20.000000, -5.000000), glm::vec2(10.000000, 10.000000), 0.000000, &entities, &counter, &DataLoader::data);
entity rect("player", glm::vec2(-200.000000, -300.000000), glm::vec2(40.000000, 40.000000), 0.000000, &entities, &counter, &DataLoader::data);
entity rect2("big rect", glm::vec2(200.000000, -300.000000), glm::vec2(40.000000, 100.000000), 0.000000, &entities, &counter, &DataLoader::data);
/*-----END-----*/

point rDebugPoint(0, 0, 6);

spatialHashGrid grid(500, 500, glm::vec2(4, 4), glm::vec2(-300, -400));

ray r(glm::vec2(-50, -300), glm::vec2(0.35, 1.4), 50, &grid);

std::unordered_map<int, entity*>* gui::entityList = &entities;

int gui::currentID = -1;
bool gui::editMode = false;
bool gui::saveAll = false;
int gui::maxEntityCount = counter - 1;

glm::vec2 mousePos;

int main() {
    /*-----POLYGON INITIALIZATION-----*/
	bottomFloor.addPolygon(glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.800000, 0.400000, 0.600000), 1);
	rect.addPolygon(glm::vec2(40.000000, 40.000000), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.500000, 0.500000, 0.700000), 1);
	rect2.addPolygon(glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.200000, 0.400000, 0.300000), 1);
    /*-----END-----*/
    
    rect.polygonInstance.initRectangle();
    bottomFloor.polygonInstance.initRectangle();
    rect2.polygonInstance.initRectangle();
    
    /*-----COLLIDER INITIALIZATION-----*/
	bottomFloor.addPolygonCollider(&grid, glm::vec2(12, 52), glm::vec2(12, 93), 91);
	rect.addPolygonCollider(&grid, glm::vec2(23, 99), glm::vec2(93, 11), 64);
	rect2.addPolygonCollider(&grid, glm::vec2(54, 19), glm::vec2(67, 28), 99);
    /*-----END-----*/

    rect2.polygonColliderInstance.initRectangle();
    rect.polygonColliderInstance.initRectangle();
    bottomFloor.polygonColliderInstance.initRectangle();

    // rect.polygonColliderInstance.setPositionOffset(40, 40);
    // rect.polygonInstance.setPositionOffset(40, 40);
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
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
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
    int initWidth, initHeight;
    glfwGetFramebufferSize(window, &initWidth, &initHeight);
    glViewport(0, 0, initWidth, initHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glEnable(GL_PROGRAM_POINT_SIZE);  
    glEnable(GL_DEPTH_TEST);  

    gui::init(window);

    Shader shader("gravityVShader", "gravityFShader");
    Shader pointShader("pointVShader", "gravityFShader");
    Shader rayShader("rayVShader", "gravityFShader");


    configureShader(shader);
    configureShader(pointShader);
    configureShader(rayShader);
    rect.polygonColliderInstance.debugShaderProgram = pointShader.ID;
    rect2.polygonColliderInstance.debugShaderProgram = pointShader.ID;
    bottomFloor.polygonColliderInstance.debugShaderProgram = pointShader.ID;

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
        glfwPollEvents();

        gui::preLoop();

        updateDeltaTime();
        processInput(window);
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
        // grid.drawGrid();
        // rect.polygonColliderInstance.renderColliderBounds();
        // std::pair<bool, rayData> rdata2 = r.getFirstCollision();
        // if(rdata2.first)
        // {
        //     rDebugPoint.setPosition(rdata2.second.collisionPoint.x, rdata2.second.collisionPoint.y);
        //     rDebugPoint.render();
        // }
        pointShader.use();
        std::vector<rayData> rdata = r.getCollisions();
        for(int i = 0; i < rdata.size(); i++) 
        {
            rDebugPoint.setPosition(rdata[i].collisionPoint.x, rdata[i].collisionPoint.y);
            rDebugPoint.render();
        }
        // std::cout << &(rect.polygonColliderInstance) << " " << &(rect2.polygonColliderInstance) << " " << &(bottomFloor.polygonColliderInstance) << std::endl;
        // std::vector<polygonCollider*> b = grid.getNearby(&(rect.polygonColliderInstance));
        // for(auto i = b.begin(); i != b.end(); i++)
        // {
        //     std::cout << (*(*i)).id << std::endl;
        // }
        // std::vector<polygonCollider*> b = grid.getNearby(rect.position.x, rect.position.y);
        // for(auto i = b.begin(); i != b.end(); i++)
        // {
        //     std::cout << (*(*i)).id << std::endl;
        // }
        // std::cout << std::endl;
        // std::pair<int, int> a = grid.getCellIndexNoClamp(rect.position.x, rect.position.y);
        // std::cout << a.first << " " << a.second << std::endl;
        // for(auto i : grid.grid[a.first][a.second])
        // {
        //     std::cout << (*i).id << std::endl;
        // }
        // std::cout << std::endl;
        // std::cout << a.first << " " << a.second << std::endl;
        // rect2.setPosition(rect2.position.x, -300 + 100 * sin(glfwGetTime()));

        gui::postLoop();
        glfwSwapBuffers(window);
    }
    gui::terminate();
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
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        r.length += 100 * deltaTime;  
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
    mousePos = glm::vec2(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos));
    rect.setPosition(mousePos.x, mousePos.y);
    // std::cout << rect.pointInPolygon(glm::vec2(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos))) << std::endl;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGuiIO& io =  ImGui::GetIO();
    if(!io.WantCaptureMouse && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
    {
        gui::currentID = grid.testPoint(mousePos.x, mousePos.y);
    }
}