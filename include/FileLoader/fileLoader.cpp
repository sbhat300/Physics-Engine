#include <iostream>
#include "fileLoader.h"

std::string fileLoader::loadShader(const char* name, bool windows)
{
    if(windows) return rootPath + "\\shaders\\" + name + ".glsl";
    else return rootPath + "/shaders/" + name + ".glsl"; 
}
std::string fileLoader::loadData(bool windows)
{
    if(windows) return rootPath + "\\src\\collisionsObjectData.txt";
    else return rootPath + "/src/collisionsObjectData.txt";
}
std::string fileLoader::loadWAV(const char* name, bool windows)
{
    return rootPath + "/Audio/" + name + ".wav";
}
std::string fileLoader::loadTGA(const char* name, bool windows)
{
    return rootPath + "/Textures/" + name + ".tga";
}