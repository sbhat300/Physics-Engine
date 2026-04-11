#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <FileLoader/wavFile.h>
#include <unordered_map>

namespace audioManager
{
    extern std::unordered_map<const char*, wavFile> clips;
    void addClip(const char* name, const char* filePath);
    void deleteClip(const char* name);
    void destroy();
};

#endif