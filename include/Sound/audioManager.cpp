#include "Sound/audioManager.h"

std::unordered_map<const char*, wavFile> audioManager::clips = std::unordered_map<const char*, wavFile>();

void audioManager::addClip(const char* name, const char* filePath)
{
    clips[name] = wavFile();
    clips[name].read(filePath);
}

void audioManager::deleteClip(const char* name)
{
    clips.erase(name);
}

void audioManager::destroy()
{
    for(std::unordered_map<const char*, wavFile>::iterator it = clips.begin(); it != clips.end(); it++)
        clips.erase(it->first);
}