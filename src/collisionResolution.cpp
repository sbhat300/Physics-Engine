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
#include <cmath>
#include <Physics/rayData.h>
#include <Physics/spatialHashGrid.h>
#include <ImguiImplementation/imguiInitialize.h>
#include <fstream>
#include <string>
#include <FileLoader/objDataLoader.h>
#include <FileLoader/fileLoader.h>
#include <sharedData.h>
#include <setup.h>
#include <algorithm>

struct collisionInfo
{
    unsigned int firstID, secondID;
    int numContacts;
    float penetrationDepth;
    glm::vec2 collisionNormal;
    glm::vec2 cp1, cp2;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void updateDeltaTime();
void configureShader(Shader& shader);
void setCamSettings();
void collisionCallback(unsigned int first, unsigned int second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void bufferMatrices(int ubo);
void timestep();
void physics();
void registerCollision(unsigned int first, unsigned int second, int numContactPoints, glm::vec2 normal, float penDepth, glm::vec2 point1, glm::vec2 point2);
void render(float alpha);
bool compareCollision(collisionInfo lhs, collisionInfo rhs);
void updateCollisions();


float windowHeight = 600, windowWidth = 1200;
int setup::maxLayers = 10;
camera2D camera(glm::vec3(0, 0, setup::maxLayers));
float deltaTime = 0.0f, lastFrame = 0.0f;
float setup::fixedDeltaTime = 1 / 60.0f;
float accumulator = 0;
unsigned int counter = 0;

int DataLoader::previousDataPos = -1;
const char* DataLoader::name = "D:\\PhysicsEngine\\src\\collisionsObjectData.txt";

std::unordered_map<int, entity*> entities;

sharedData shared;

/*-----ENTITY INITIALIZATION-----*/
entity bottomFloor("small rect", glm::vec2(-79.000000, 126.000000), glm::vec2(50.000000, 50.000000), 0.000000, &entities, &counter, &shared);
entity rect("player", glm::vec2(100.863342, 126.167450), glm::vec2(40.000000, 40.000000), 0, &entities, &counter, &shared);
entity rect2("big rect", glm::vec2(68.000000, -246.000000), glm::vec2(861.000000, 98.000000), 0.000000, &entities, &counter, &shared);
/*-----END-----*/

point rDebugPoint(0, 0, 6);

spatialHashGrid grid(900, 700, glm::vec2(3, 3), glm::vec2(-380, -400));

std::vector<collisionInfo> collisions;

bool keys[26];
const unsigned char KEY_OFFSET = 65;

std::unordered_map<int, entity*>* gui::entityList = &entities;
int gui::currentID = -1;
bool gui::editMode = false;
bool gui::saveAll = false;
int gui::maxEntityCount = counter - 1;
float gui::fps = 0;
spatialHashGrid* gui::spatialHash = &grid; 

std::string fileLoader::rootPath = ROOT_DIR;

glm::vec2 mousePos;

int main() { 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    #endif
    GLFWwindow* window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "EPIC OPENGL PROJECT", NULL, NULL);
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
    shared.initVAOs();

    /*-----POLYGON INITIALIZATION-----*/
	bottomFloor.addPolygon(glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.800000, 0.400000, 0.600000), 1);
	rect.addPolygon(glm::vec2(0, 0), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.500000, 0.500000, 0.700000), 1);
	rect2.addPolygon(glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.200000, 0.400000, 0.300000), 1);
    /*-----END-----*/
    
    /*-----COLLIDER INITIALIZATION-----*/
	bottomFloor.addPolygonCollider(&grid, glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000);
	rect.addPolygonCollider(&grid, glm::vec2(0, 0), glm::vec2(1.000000, 1.000000), 0.000000);
	rect2.addPolygonCollider(&grid, glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000);
    /*-----END-----*/

    /*-----RIGIDBODY INITIALIZATION-----*/
	bottomFloor.addPolygonRigidbody(10.0f, 10.0f, 0.2f);
    rect.addPolygonRigidbody(15.0f, 10.0f, 0.4f);
    rect2.addPolygonRigidbody(0.0f, 0.0f, 0.0f);
    /*-----END-----*/

    rDebugPoint.setColor(glm::vec3(1, 1, 1));
    rDebugPoint.setLayer(setup::maxLayers - 1);

    grid.setColor(glm::vec3(1, 0.5f, 1));
    grid.setLayer(0);

    rect.polygonInstance.initRectangle();
    bottomFloor.polygonInstance.initRectangle();
    rect2.polygonInstance.initRectangle();

    rect2.polygonColliderInstance.initRectangle();
    rect.polygonColliderInstance.initRectangle();
    bottomFloor.polygonColliderInstance.initRectangle();

    rect.polygonColliderInstance.setCollisionCallback(collisionCallback);
    rect2.polygonColliderInstance.setCollisionCallback(collisionCallback);
    bottomFloor.polygonColliderInstance.setCollisionCallback(collisionCallback);

    bottomFloor.polygonRigidbodyInstance.setRectangleMomentOfInertia();
    rect.polygonRigidbodyInstance.setRectangleMomentOfInertia();
    rect2.polygonRigidbodyInstance.setRectangleMomentOfInertia();
    // bottomFloor.polygonColliderInstance.collide = false;

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
    grid.debugShaderProgram = rayShader.ID;
    rect.polygonInstance.shaderProgram = shader.ID;
    rect2.polygonInstance.shaderProgram = shader.ID;
    bottomFloor.polygonInstance.shaderProgram = shader.ID;
    rDebugPoint.debugShaderProgram = pointShader.ID;

    unsigned int matrixUBO;
    glGenBuffers(1, &matrixUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matrixUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, matrixUBO, 0, 2 * sizeof(glm::mat4));
    glm::mat4 projection = glm::ortho(0.0f, windowWidth, 0.0f, windowHeight, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    setCamSettings();

    float fpsTimer = 0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        gui::preLoop();

        updateDeltaTime();
        
        fpsTimer -= deltaTime;
        if(fpsTimer < 0)
        {
            gui::fps = std::round(1 / deltaTime);
            fpsTimer = 1;
        }
        processInput(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bufferMatrices(matrixUBO);

        timestep();

        grid.drawGrid();

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
    windowWidth = (float)width;
    windowHeight = (float)height;
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
        keys[GLFW_KEY_E - KEY_OFFSET] = true;
    else 
        keys[GLFW_KEY_E - KEY_OFFSET] = false;
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
void collisionCallback(unsigned int first, unsigned int second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2)
{
    std::cout << glm::to_string(collisionNormal) << " " << (float)penetrationDepth << std::endl;
    if(contactPoints == 1)
    {
        rDebugPoint.setPosition(cp1.x, cp1.y);
        rDebugPoint.render();
    }
    if(contactPoints == 2)
    {
        rDebugPoint.setPosition(cp1.x, cp1.y);
        rDebugPoint.render();
        rDebugPoint.setPosition(cp2.x, cp2.y);
        rDebugPoint.render();
    }
    registerCollision(first, second, contactPoints, collisionNormal, penetrationDepth, cp1, cp2);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    mousePos = glm::vec2(camera.camPos.x + xpos, camera.camPos.y + (windowHeight - ypos));
    // rect.setPosition(mousePos.x, mousePos.y);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGuiIO& io =  ImGui::GetIO();
    if(!io.WantCaptureMouse && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
    {
        gui::currentID = grid.testPoint(mousePos.x, mousePos.y);
    }
}
void timestep()
{
    accumulator += deltaTime > 0.2f ? 0.2f : deltaTime;
    if(accumulator > 0.2f) accumulator = 0.2f;
    while(accumulator >= setup::fixedDeltaTime)
    {
        physics();
        accumulator -= setup::fixedDeltaTime;
    }
    float alpha = accumulator / setup::fixedDeltaTime;
    render(alpha);
}
void physics()
{
    rect.polygonRigidbodyInstance.gravity(30);
    bottomFloor.polygonRigidbodyInstance.gravity(30);
    updateCollisions();
    for(std::pair<const int, entity*> obj : entities) 
        if((*obj.second).contain[2]) (*obj.second).polygonRigidbodyInstance.update();
}
void render(float alpha)
{
    for(std::pair<const int, entity*> obj : entities)
    {
        if((*obj.second).contain[0]) (*obj.second).polygonInstance.render(alpha);
        if((*obj.second).contain[1])
        {
            if((*obj.second).polygonColliderInstance.shouldRenderBounds) 
                (*obj.second).polygonColliderInstance.renderColliderBounds(); //FOR DEBUG REMOVE
        }
    }
}
void registerCollision(unsigned int first, unsigned int second, int numContactPoints, glm::vec2 normal, float penDepth, glm::vec2 point1, glm::vec2 point2)
{
    collisionInfo newCollision;
    if(first < second)
    {
        newCollision.firstID = first;
        newCollision.secondID = second;
    }
    else
    {
        newCollision.secondID = first;
        newCollision.firstID = second;
    }
    newCollision.numContacts = numContactPoints;
    newCollision.penetrationDepth = penDepth;
    newCollision.cp1 = point1;
    newCollision.cp2 = point2;
    newCollision.collisionNormal = normal;
    collisions.push_back(newCollision);
}
bool compareCollision(collisionInfo lhs, collisionInfo rhs)
{
    if(lhs.firstID < rhs.firstID) return true;
    if(lhs.firstID == rhs.firstID) return lhs.secondID < rhs.secondID;
    return false;
}
void updateCollisions()
{
    collisions.clear();
    for(std::pair<const int, entity*> obj : entities) 
        if((*obj.second).contain[1]) (*obj.second).polygonColliderInstance.updateCollider();
    std::sort(collisions.begin(), collisions.end(), compareCollision);
    std::vector<collisionInfo> uniqueCollisions;
    int i = 0;
    while(i < collisions.size())
    {
        uniqueCollisions.push_back(*(collisions.begin() + i));
        i++;
        while(i < collisions.size())
        {
            auto collision = collisions.begin() + i;
            if((*collision).firstID != uniqueCollisions.back().firstID || (*collision).secondID != uniqueCollisions.back().secondID) break;
            i++;
        }
    }
    collisions = uniqueCollisions;
}