#include <Scripting/baseScript.h>
#include <ImguiImplementation/imguiInitialize.h>
#include <Engine/engine.h>
#include <Engine/inputHandler.h>
#include <entity.h>

namespace textures
{
    extern texture* defaultTexture;
    extern texture* cat;
    void loadTextures() 
    {
        cat = new texture();
        cat->loadTexture(fileLoader::loadTGA("cat", setup::windows).c_str());

        defaultTexture = new texture();
    }

    void deleteTextures()
    {
        delete cat;
        delete defaultTexture;
    }
};

namespace playerScripts {
    class rect : public baseScript 
    {
        public:
            void collisionCallback(entity* first, entity* second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2)
            {
                engine::solver.registerCollision(first, second, contactPoints, collisionNormal, penetrationDepth, cp1, cp2);
            }

            void start() override 
            {
                parent->addPolygon(glm::vec2(0, 0), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(1.0, 1.0, 1.0), 1);
                parent->addPolygonCollider(glm::vec2(0, 0), glm::vec2(1.000000, 1.000000), 0.000000);
                parent->addPolygonRigidbody(15.0f, 0.0f, 0.0f, 0.4f);
                //parent->polygonInstance.initRectangle();
                parent->polygonInstance.initCircle(32);
                parent->collider.initCircle(1);
                parent->collider.setCollisionCallback([this](entity* first, entity* second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2) {
                    collisionCallback(first, second, collisionNormal, penetrationDepth, contactPoints, cp1, cp2);
                });
                parent->rigidbody.setCircleMomentOfInertia();
                parent->collider.debugShaderProgram = engine::shared.pointShaderID;
                parent->polygonInstance.shaderProgram = engine::shared.mainShaderID;

                parent->polygonInstance.polygonTexture = textures::cat;
            }

            void update() override 
            {
                if(!gui::paused)
                {
                    if (inputHandler::buttons[GLFW_KEY_Q].down)
                        parent->rigidbody.addForce(-200000* engine::deltaTime, 0);
                    if (inputHandler::buttons[GLFW_KEY_E].down)
                        parent->rigidbody.addForce(200000 * engine::deltaTime, 0);
                }
            }

            void fixedUpdate() override 
            {
                parent->rigidbody.gravity(50);
            }
    };

    class bottomFloor: public baseScript 
    {
        public:
            void collisionCallback(entity* first, entity* second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2)
            {
                engine::solver.registerCollision(first, second, contactPoints, collisionNormal, penetrationDepth, cp1, cp2);
            }
            float rectVertices[16] = {
                1 / 2.0f,  1 / 2.0f, 1.0f, 1.0f,  // top right
                -1 / 2.0f,  2.0f, 0.0f, 1.0f,   // top left
                -1 / 2.0f, -1 / 2.0f, 0.0f, 0.0f, // bottom left 
                1 / 2.0f, -1 / 2.0f, 1.0f, 0.0f// bottom right
            };
            int rectIndices[6] = { 
                                    0, 1, 3,  
                                    1, 2, 3    
                                }; 
            float colliderVertices[8] = {
                1 / 2.0f,  1 / 2.0f,
                -1 / 2.0f,  2.0f,
                -1 / 2.0f, -1 / 2.0f,
                1 / 2.0f, -1 / 2.0f 
            };
            void start() override
            {
                parent->addPolygon(glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.800000, 0.400000, 0.600000), 1);
                parent->addPolygonCollider(glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000);
                parent->addPolygonRigidbody(10.0f, 0.0f, 0.0f, 0.4f);
                parent->polygonInstance.initPolygon(4, rectVertices, 6, rectIndices);
                parent->collider.initPolygon(4, colliderVertices);
                parent->collider.setCollisionCallback([this](entity* first, entity* second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2) {
                    collisionCallback(first, second, collisionNormal, penetrationDepth, contactPoints, cp1, cp2);
                });
                parent->rigidbody.setRectangleMomentOfInertia();
                parent->collider.debugShaderProgram = engine::shared.pointShaderID;
                parent->polygonInstance.shaderProgram = engine::shared.mainShaderID;
                parent->polygonInstance.polygonTexture = textures::defaultTexture;
            }

            void fixedUpdate() override
            {
                parent->rigidbody.gravity(50);
            }
    };

    class rect2 : public baseScript 
    {
        public:
            void collisionCallback(entity* first, entity* second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2)
            {
                engine::solver.registerCollision(first, second, contactPoints, collisionNormal, penetrationDepth, cp1, cp2);
            }
            void start() override
            {
                parent->addPolygon(glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000, glm::vec3(0.200000, 0.400000, 0.300000), 1);
                parent->addPolygonCollider( glm::vec2(0.000000, 0.000000), glm::vec2(1.000000, 1.000000), 0.000000);
                parent->addPolygonRigidbody(0.0f, 0.0f, 0.0f, 0.4f);
                parent->polygonInstance.initRectangle();
                parent->collider.initRectangle();
                parent->collider.setCollisionCallback([this](entity* first, entity* second, glm::vec2 collisionNormal, float penetrationDepth, int contactPoints, glm::vec2 cp1, glm::vec2 cp2) {
                    collisionCallback(first, second, collisionNormal, penetrationDepth, contactPoints, cp1, cp2);
                });
                parent->rigidbody.setRectangleMomentOfInertia();
                parent->collider.debugShaderProgram = engine::shared.pointShaderID;
                parent->polygonInstance.shaderProgram = engine::shared.mainShaderID;
                parent->polygonInstance.polygonTexture = textures::defaultTexture;
            }
    };

    class controls : public baseScript 
    {
        public:
            void update() override 
            {
                ImGuiIO& io = ImGui::GetIO();
                if (inputHandler::buttons[GLFW_KEY_ESCAPE].down)
                    engine::shutdown();
                if (inputHandler::buttons[GLFW_KEY_W].down)
                    engine::camera.ProcessKeyboard(UP, engine::deltaTime);
                if (inputHandler::buttons[GLFW_KEY_S].down)
                    engine::camera.ProcessKeyboard(DOWN, engine::deltaTime);
                if (inputHandler::buttons[GLFW_KEY_A].down)
                    engine::camera.ProcessKeyboard(LEFT, engine::deltaTime);
                if (inputHandler::buttons[GLFW_KEY_D].down)
                    engine::camera.ProcessKeyboard(RIGHT, engine::deltaTime);
                if(!io.WantCaptureMouse && inputHandler::buttons[GLFW_MOUSE_BUTTON_1].pressed)
                    gui::currentID = engine::grid.testPoint(inputHandler::worldMousePos.x, inputHandler::worldMousePos.y);
            }
    };
}