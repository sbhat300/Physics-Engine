#include "FileLoader/fileLoader.h"
#include <iostream>

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