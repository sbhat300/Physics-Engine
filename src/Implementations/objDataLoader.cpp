#include "FileLoader/objDataLoader.h"
#include <entity.h>
#include <limits>

void DataLoader::reset()
{
    data.clear();
    data.seekg(0);
    data.seekp(0);
}
void DataLoader::jumpToLine(int line)
{
    reset();
    if(line == 1) previousDataPos = 0;
    for(int i = 0; i < line - 1; i++)
    {
        data.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(i == line - 2) previousDataPos = data.tellg();
    }
}
void DataLoader::setFile(std::string inp)
{
    data.close();
    data.open(name, std::ios::out | std::ios::in | std::ios::trunc);
    data << inp;
    data.close();
}