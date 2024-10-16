#ifndef IMGUIINITIALIZE_H
#define IMGUIINITIALIZE_H
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <entity.h>
#include <unordered_map>
#include <string>
#include <Physics/spatialHashGrid.h>


namespace gui
{
    extern int currentID;
    extern bool editMode;
    extern bool saveAll;
    extern int maxEntityCount;
    extern float fps;
    extern std::unordered_map<unsigned int, entity*>* entityList;
    extern ImGuiIO& io;
    extern spatialHashGrid* spatialHash;
    void init(GLFWwindow *window);
    void preLoop();
    void entityOptions();
    void polygonOptions();
    void polygonColliderOptions();
    void shg();
    std::string getEntityData(int i, bool saved);
    void postLoop();
    void terminate();
};

#endif