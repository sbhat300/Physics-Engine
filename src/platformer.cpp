#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <Shader/shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <camera/camera2D.h>
#include <Physics/rectangleRigidbody.h>
#include <Physics/rectangleCollider.h>
#include <Controllers/platformerPlayer.h>
#include <Objects/rectangle.h>
#include <list>
#include <iterator>
#include <map>
#include <vector>

struct collisionInfo {
    int id;
    glm::vec3 collisionNormal;
    float penetrationDepth;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void processWireframeChange(GLFWwindow* window);
void updateDeltaTime();
void configureShader(Shader& shader);
void setCamSettings();
void playerCollision(int first, int second, glm::vec3 collisionNormal, float penetrationDepth);
void physics(GLFWwindow* window);
void resolveCollisions();
bool sortCollisions(std::pair<collisionInfo, collisionInfo> lhs, std::pair<collisionInfo, collisionInfo> rhs);
void registerCollision(int first, int second, glm::vec3 collisionNormal, float penetrationDepth);
void render();

float windowHeight = 600, windowWidth = 800;
camera2D camera(glm::vec3(0, 0, 1));
float deltaTime = 0.0f, lastFrame = 0.0f;
int counter = 0;

std::map<int, rectangleCollider*> rectangleColliders;
std::map<int, rectangleRigidbody*> rectangleRbs;
std::vector<std::pair<collisionInfo, collisionInfo>> collisions;

rectangleRigidbody rect(1, 1, 0, glm::vec3(40, 40, 40), glm::vec3(0, 200, 0), &deltaTime, &counter, &rectangleColliders, 10, &rectangleRbs, 0.5f);
rectangleRigidbody rect2(1, 1, 0, glm::vec3(40, 40, 40), glm::vec3(200, 200, 0), &deltaTime, &counter, &rectangleColliders, 10, &rectangleRbs, 0.0f);
platformerPlayer player(1, 1, 0, glm::vec3(40, 40, 40), glm::vec3(-200, 200, 0), &deltaTime, &counter, &rectangleColliders, 10, &rectangleRbs, 1.0f, 6.0f, 7);
rectangleRigidbody bottomFloor(1, 1, 0, glm::vec3(1000, 60, 60), glm::vec3(0, -300, 0), &deltaTime, &counter, &rectangleColliders, 0, &rectangleRbs, 0.0f);

int main() {
    rect.setCollisionCallback(registerCollision);
    rect2.setCollisionCallback(registerCollision);
    player.setCollisionCallback(playerCollision);
    rect.setColor(glm::vec3(0.5f, 0.5f, 0.5f));
    player.setColor(glm::vec3(0.8f, 0.8f, 0.8f));
    rect2.setColor(glm::vec3(1, 1, 1));
    bottomFloor.setColor(glm::vec3(0.2f, 0.2f, 0.2f));
    collisions.reserve(counter + 1);

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
    

    Shader shader("D:/Physics-Engine/shaders/gravityVShader.glsl", "D:/Physics-Engine/shaders/gravityFShader.glsl");
    setCamSettings();
    
    while (!glfwWindowShouldClose(window))
    {
        updateDeltaTime();
        processInput(window);
        processWireframeChange(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        physics(window);
        render();
        configureShader(shader);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
	return 0;
}
void physics(GLFWwindow* window)
{
    player.grounded = false;

    //Add forces
    rect.addForce(0, -9.8f);
    player.addForce(0, -9.8f);
    rect2.addForce(0, -9.8f);

    //Apply forces and render
    rect.updateRigidbody();
    rect2.updateRigidbody();
    player.updateRigidbody();
    bottomFloor.updateRigidbody();

    //Colliders
    rect.updateCollider();
    rect2.updateCollider();
    player.updateCollider();
    bottomFloor.updateCollider();
    resolveCollisions();

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        player.addForce(player.speed, 0);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        player.addForce(-player.speed, 0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && player.grounded)
        player.addImpulse(0, player.jumpForce);
}

void render()
{
    rect.render();
    rect2.render();
    player.render();
    bottomFloor.render();
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
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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
void playerCollision(int first, int second, glm::vec3 collisionNormal, float penetrationDepth)
{
    glm::vec3 worldUp(0, 1, 0);
    float diff = glm::dot(collisionNormal, worldUp);
    if((std::abs(diff) > 0.707106781f) && (std::abs(diff) <= 1)) player.grounded = true;
    registerCollision(first, second, collisionNormal, penetrationDepth);
}
void registerCollision(int first, int second, glm::vec3 collisionNormal, float penetrationDepth)
{
    collisionInfo one;
    one.id = first;
    one.collisionNormal = -collisionNormal;
    one.penetrationDepth = penetrationDepth;
    collisionInfo two;
    two.id = second;
    two.collisionNormal = collisionNormal;
    two.penetrationDepth = penetrationDepth;
    if(first < second) collisions.push_back(std::pair<collisionInfo, collisionInfo>(one, two));
    else collisions.push_back(std::pair<collisionInfo, collisionInfo>(two, one));
}
void resolveCollisions()
{

    std::vector<std::pair<collisionInfo, collisionInfo>> uniqueCollisions;
    sort(collisions.begin(), collisions.end(), sortCollisions);
    
    int i = 0;
    while(i < collisions.size())
    {
        std::vector<std::pair<collisionInfo, collisionInfo>>::iterator test = collisions.begin() + i;
        uniqueCollisions.push_back(*test);
        i++;
        while(i < collisions.size())
        {
            std::vector<std::pair<collisionInfo, collisionInfo>>::iterator potential = collisions.begin() + i;
            if((*test).first.id != (*potential).first.id || (*test).second.id != (*potential).second.id) break;
            i++;
        }
    }
    collisions.clear();

    for(auto i : uniqueCollisions)
    {
        rectangleRigidbody* f = rectangleRbs[i.first.id];
        rectangleRigidbody* s = rectangleRbs[i.second.id];
        glm::vec3 relativeVelocity = (*s).velocity - (*f).velocity;
        float velAlongNormal = glm::dot(relativeVelocity, i.first.collisionNormal);
        if(velAlongNormal > 0) continue;
        float e = std::min((*f).restitution, (*s).restitution);
        float j = -(1 + e) * velAlongNormal;
        j /= (*f).invMass + (*s).invMass;
        glm::vec3 impulse = j * i.first.collisionNormal;
        (*f).addImpulse(-impulse.x, -impulse.y);
        (*s).addImpulse(impulse.x, impulse.y);

        const float percent = 0.2; // usually 20% to 80% 
        const float slop = 0.01; // usually 0.01 to 0.1 
        glm::vec3 correction = std::max(i.first.penetrationDepth - slop, 0.0f) / ((*f).invMass + (*s).invMass) * percent * i.first.collisionNormal;
        (*f).position -= (*f).invMass * correction;
        (*s).position += (*s).invMass * correction;
    }
}
bool sortCollisions(std::pair<collisionInfo, collisionInfo> lhs, std::pair<collisionInfo, collisionInfo> rhs)
{
    if(lhs.first.id < rhs.first.id) return true;
    if(lhs.first.id == rhs.first.id) return lhs.second.id < rhs.second.id;
    return false;
}