#ifndef ENGINE_H
#define ENGINE_H

#include <Engine/setup.h>
#include <atomic>
#include <camera/camera2D.h>
#include <FileLoader/objDataLoader.h>
#include <Physics/collisionSolver.h>
#include <Engine/sharedData.h>
#include <ImguiImplementation/imguiInitialize.h>
#include <FileLoader/fileLoader.h>
#include <glm/glm.hpp>
#include <Shader/shader.h>
#include <thread>
#include "config.h"


namespace engine
{
    extern float windowHeight, windowWidth;
    extern camera2D camera;
    extern float deltaTime, lastFrame;
    extern float accumulator;
    extern unsigned int counter;
    extern std::unordered_map<unsigned int, entity*> entities;
    extern sharedData shared;
    extern collisionSolver solver;
    extern spatialHashGrid grid;
    const unsigned char KEY_OFFSET = 65;
    extern float fpsTimer;
    extern std::thread audioThread;
    extern GLFWwindow* window;
    extern unsigned int matrixUBO;
    extern Shader rayShader;

    void setupWindow(float height=600, float width=1200, float maxLayers=10);
    void initializeSpatialHashGrid(float width, float height, glm::vec2 numCells, glm::vec2 start);
    int initialize();
    void initializeSolver(float bias, float slop, float smallestImpulse, float restitutionSlop);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void timestep();
    void physics();
    void render(float alpha);
    void debugRender();
    void shutdown();
    void bufferMatrices(int ubo);
    void configureShader(Shader& shader);
    void run();
}

#endif