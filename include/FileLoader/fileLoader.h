#ifndef FILELOADER_H
#define FILELOADER_H
#include <string>
#include "config.h"

namespace fileLoader
{
    extern std::string rootPath;
    std::string loadShader(const char* name, bool windows);
    std::string loadData(bool windows);
};
#endif