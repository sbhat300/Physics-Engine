#include "FileLoader/fileLoader.h"

fileLoader::fileLoader()
{
    rootPath = std::filesystem::current_path().parent_path().c_str();    
}
std::string fileLoader::loadShader(const char* name, bool windows)
{
    if(windows) return rootPath + "\\shaders\\" + name + ".glsl";
    else return rootPath + "/shaders/" + name + ".glsl"; 
}