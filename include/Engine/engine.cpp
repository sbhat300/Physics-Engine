#include "engine.h"
#include <Engine/setup.h>
#include <camera/camera2D.h>
#include <Physics/collisionSolver.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <Sound/audioPlayer.h>
#include <Engine/inputHandler.h>

float setup::fixedDeltaTime = 1 / 60.0f;
float setup::linearDamping = 0.999f;
float setup::angularDamping = 0.997f;
int setup::maxLayers = 10;
bool setup::windows = false;
std::atomic<bool> setup::shouldWindowClose(false);

const char* DataLoader::name = "D:\\PhysicsEngine\\src\\collisionsObjectData.txt";

std::unordered_map<unsigned int, entity*>* gui::entityList = nullptr;
int gui::currentID = -1;
bool gui::editMode = false;
bool gui::saveAll = false;
bool gui::paused = false;
int gui::maxEntityCount = 0;
float gui::fps = 0;
spatialHashGrid* gui::spatialHash = nullptr; 

std::string fileLoader::rootPath = ROOT_DIR;
std::function<void()> gui::debugStep = nullptr;

float engine::windowHeight = 0;
float engine::windowWidth = 0;
camera2D engine::camera = camera2D();
float engine::deltaTime = 0;
float engine::lastFrame = 0;
float engine::accumulator = 0;
unsigned int engine::counter = 0;
std::unordered_map<unsigned int, entity*> engine::entities = std::unordered_map<unsigned int, entity*>();
sharedData engine::shared = sharedData();
collisionSolver engine::solver = collisionSolver(nullptr);
spatialHashGrid engine::grid = spatialHashGrid();
std::thread engine::audioThread;
float engine::fpsTimer = 0;
GLFWwindow* engine::window = nullptr;
unsigned int engine::matrixUBO = 0;
Shader engine::rayShader;
std::string engine::rootPath = "";
std::stack<entity*> engine::deleteQueue;
glm::vec3 engine::clearColor(0, 0, 0);

void engine::setupWindow(float height, float width, float maxLayers)
{
    windowHeight = height;
    windowWidth = width;
    setup::maxLayers = maxLayers;
}

void engine::initializeSpatialHashGrid(float width, float height, glm::vec2 numCells, glm::vec2 start)
{
    grid = spatialHashGrid(width, height, numCells, start);
    grid.setColor(glm::vec3(1, 0.5f, 1));
    grid.setLayer(0);
    gui::spatialHash = &grid;
    rayShader = Shader("rayVShader", "gravityFShader");
    configureShader(rayShader);
    grid.debugShaderProgram = rayShader.ID;
}

int engine::initialize(std::string rootDir)
{
    rootPath = rootDir;
    fileLoader::rootPath = rootPath;
    camera = camera2D(glm::vec3(0, 0, setup::maxLayers));
    deltaTime = 0.0f;
    lastFrame = 0.0f;
    accumulator = 0;
    counter = 0;
    solver = collisionSolver(&counter);
    gui::debugStep = debugRender;
    gui::entityList = &entities;

    camera.camPos = glm::vec3(-windowWidth / 2, -windowHeight / 2, 10);
    camera.speed = 200.0f;

    #ifdef _WIN32 
        setup::windows = true;
    #endif

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    #endif
    window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "EPIC OPENGL PROJECT", NULL, NULL);
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

    glEnable(GL_PROGRAM_POINT_SIZE);  
    glEnable(GL_DEPTH_TEST);  
    shared.initVAOs();
    gui::init(window);

    glGenBuffers(1, &matrixUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matrixUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, matrixUBO, 0, 2 * sizeof(glm::mat4));
    glm::mat4 projection = glm::ortho(0.0f, windowWidth, 0.0f, windowHeight, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    camera.camPos = glm::vec3(-windowWidth / 2, -windowHeight / 2, 10);
    camera.speed = 200.0f;

    audioThread = std::thread(audioPlayer::start);
    return 0;
}

void engine::run()
{
    for(std::pair<const int, entity*> obj : entities)
    {
        if(obj.second->enabled && obj.second->contain[3]) 
        {
            for(baseScript* script : obj.second->scripts) 
            {
                script->preUpdate();
            }
        }
    }
    while(!setup::shouldWindowClose.load())
    {
        glfwPollEvents();

        gui::preLoop();

        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        fpsTimer -= deltaTime;
        if(fpsTimer < 0)
        {
            gui::fps = std::round(1 / deltaTime);
            fpsTimer = 1;
        }
        inputHandler::update(window, &camera, windowHeight);
        //TODO: OPTIMIZE EVERYTHING HERE AND NOT LOOP THROUGH EVERY ENTITY EVERY TIME
        if(!gui::paused)
        {
            for(std::pair<const int, entity*> obj : entities)
            {
                if(obj.second->enabled && obj.second->contain[3]) 
                {
                    for(baseScript* script : obj.second->scripts) 
                    {
                        if(script->active) script->update();
                    }
                }
            }
        }
        bufferMatrices(matrixUBO);

        timestep();
        if(grid.wantsDraw) grid.drawGrid();

        gui::postLoop();
        glfwSwapBuffers(window);
        setup::shouldWindowClose.store(glfwWindowShouldClose(window), std::memory_order::memory_order_relaxed);

        while(!deleteQueue.empty())
        {
            delete deleteQueue.top();
            deleteQueue.pop();
        }
    }
    setup::shouldWindowClose.store(true);
    audioThread.join();
    gui::terminate();
    glfwTerminate();
}

void engine::initializeSolver(float bias, float slop, float smallestImpulse, float restitutionSlop)
{
    solver.entities = &entities;
    solver.bias = bias;
    solver.slop = slop;
    solver.smallestImpulse = smallestImpulse;
    solver.restitutionSlop = restitutionSlop;
}

void engine::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    windowWidth = (float)width;
    windowHeight = (float)height;
}

void engine::timestep()
{
    if(!gui::paused)
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
    else
    {
        render(1);
    }
}

void engine::physics()
{
    solver.reset();
    for(std::pair<const int, entity*> obj : entities) 
    {
        if(obj.second->enabled)
        {
            if(obj.second->contain[0]) 
                obj.second->polygonInstance.updatePreviousState();
            if((*obj.second).contain[1]) (*obj.second).collider.updateCollider();
            if(obj.second->contain[3]) 
            {
                for(baseScript* script : obj.second->scripts) 
                {
                    if(script->active) script->fixedUpdate();
                }
            }
        }
    }
    for(std::pair<unsigned int, entity*> obj : entities) 
        if(obj.second->enabled && (*obj.second).contain[2]) (*obj.second).rigidbody.updateVel();
    //solve collisions
    solver.updateCollisions();
    solver.resolveCollisions();
    for(std::pair<unsigned int, entity*> obj : entities) 
        if(obj.second->enabled && (*obj.second).contain[2]) (*obj.second).rigidbody.updatePos();
}

void engine::render(float alpha)
{
    for(std::pair<const int, entity*> obj : entities)
    {
        if(obj.second->enabled)
        {
            if((*obj.second).contain[0]) 
            {
                if(obj.second->contain[2]) (*obj.second).polygonInstance.render(alpha);
                else (*obj.second).polygonInstance.render(1);
            }
            if((*obj.second).contain[1])
            {
                if((*obj.second).collider.shouldRenderBounds) 
                    (*obj.second).collider.renderColliderBounds(); 
            }
        }
    }
}

void engine::debugRender()
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

void engine::shutdown()
{
    glfwSetWindowShouldClose(window, true);
}

void engine::bufferMatrices(int ubo)
{
    glm::mat4 view = camera.GetViewMatrix();
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void engine::configureShader(Shader& shader)
{
    shader.use();
    unsigned int uniformBlock = glGetUniformBlockIndex(shader.ID, "Matrices");
    glUniformBlockBinding(shader.ID, uniformBlock, 0);
}

void engine::deleteEntity(entity* e)
{
    deleteQueue.push(e);
}

void engine::setBackgroundColor(float x, float y, float z)
{
    clearColor = glm::vec3(x, y, z);
}