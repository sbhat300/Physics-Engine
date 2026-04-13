#include <iostream>
#include <Shader/shader.h>
#include <glm/glm.hpp>
#include <entity.h>
#include <unordered_map> 
#include <Engine/sharedData.h>
#include <Engine/engine.h>
#include <Scripting/baseScript.h>
#include <config.h>
#include "playerScripts.h"

int main() { 
    engine::setupWindow();
    engine::initialize(ROOT_DIR);
    engine::initializeSpatialHashGrid(900, 700, glm::vec2(3, 3), glm::vec2(-380, -400));
    engine::initializeSolver(0.01f, 0.1f, 0.1f, 1.0f);

    Shader shader("gravityVShader", "gravityFShader");
    Shader pointShader("pointVShader", "gravityFShader");

    engine::configureShader(shader);
    engine::configureShader(pointShader);
    engine::shared.mainShaderID = shader.ID;
    engine::shared.pointShaderID = pointShader.ID;

    //------ENGINE INITIALIZATION DONE-----

    entity bottomFloor("small rect", glm::vec2(-79.000000, -40.000000), glm::vec2(50.000000, 50.000000), glm::radians(180.000000));
    entity rect("player", glm::vec2(0, 10), glm::vec2(40.000000, 40.000000), glm::radians(20.0f));
    entity rect2("big rect", glm::vec2(68.000000, -246.000000), glm::vec2(861.000000, 98.000000), 0.000000);
    entity controls("controls");

    rect.addScript<playerScripts::rect>();
    bottomFloor.addScript<playerScripts::bottomFloor>();
    rect2.addScript<playerScripts::rect2>();
    controls.addScript<playerScripts::controls>();


    unsigned int inputs[] = {GLFW_KEY_ESCAPE, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_Q, GLFW_KEY_E, GLFW_MOUSE_BUTTON_1};
    inputHandler::trackKeys(&inputs[0], 8);

    engine::run();
	return 0;
}
