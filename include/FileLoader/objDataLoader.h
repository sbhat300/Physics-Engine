#ifndef OBJDATALOADER_H
#define OBJDATALOADER_H

class entity;
#include <fstream>
#include <string>

namespace DataLoader
{
    extern int previousDataPos;
    extern const char* name;
    // void reset();
    // void jumpToLine(int line);
    void setFileData(std::string inp);
};
#endif