#include "FileLoader/objDataLoader.h"
#include <entity.h>
#include <limits>
#include <FileLoader/fileLoader.h>

void DataLoader::setFileData(std::string inp)
{
    std::fstream data;
    bool windows = false;
    #ifdef _WIN32 
        windows = true;
    #endif
    data.open(fileLoader::loadData(windows), std::ios::out | std::ios::in | std::ios::trunc);
    data << inp;
    data.close();
}