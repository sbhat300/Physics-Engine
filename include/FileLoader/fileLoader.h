#ifndef FILELOADER_H
#define FILELOADER_H
#include <filesystem>
#include <string>

class fileLoader
{
    public:
        std::string rootPath;
        fileLoader();
        std::string loadShader(const char* name, bool windows);
    private:
};
#endif