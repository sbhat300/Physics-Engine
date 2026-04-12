#include "tgaLoader.h"
#include <iostream>
#include <bitset>
#include <cstring>
#include <glad/glad.h>

bool tgaLoader::load(const char* path, bool srgb, bool flip)
{
    FILE* tgaFile;
    tgaFile = fopen(path, "rb");
    if(tgaFile == NULL)
    {
        std::cout << "TGA FILE NOT OPENED" << std::endl;
        return false;
    }
    unsigned char header[12];
    if(fread(header, 1, 12, tgaFile) == 0)
    {
        std::cout << "ERROR READING TGA FILE" << std::endl;
        std::cout << header[2] << std::endl;
        return false;
    }
    if(!loadHeader(path, tgaFile, srgb)) return false;
    if(memcmp(header, uncompressedHeader, sizeof(uncompressedHeader)) == 0)
    {
        return loadUncompressed(path, tgaFile, flip);
    }
    else if (memcmp(header, compressedHeader, sizeof(compressedHeader)) == 0)
    {
        std::cout << "WARNING COMPRESSED NOT IMPLEMENTED" << std::endl;
        return loadCompressed(path, tgaFile, flip);
    }
    else
    {
        std::cout << "TGA FORAMT NOT SUPPORTED" << std::endl;
        return false;
    }
    
    fclose(tgaFile);
    return true;
}

bool tgaLoader::loadUncompressed(const char* path, FILE* tgaFile, bool flip)
{
    data = (unsigned char*)malloc(imageSize);
    if(flip)
        for(int i = 0; i < height; i++)
            fread(data + (height - (i + 1)) * width * bytesPerPixel, 1, width * bytesPerPixel, tgaFile);
    else fread(data, 1, imageSize, tgaFile);
    for(int i = 0; i < imageSize; i += bytesPerPixel)
    {
        char temp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = temp;
    }
    return true;
}

bool tgaLoader::loadCompressed(const char* path, FILE* tgaFile, bool flip)
{
    return false;
}
bool tgaLoader::loadHeader(const char* path, FILE* tgaFile, bool srgb)
{
    unsigned char headerFinal[6];
    if(fread(headerFinal, 1, 6, tgaFile) == 0)
    {
        std::cout << "FINAL TGA HEADER NOT READ" << std::endl;
        return false;
    }
    width = headerFinal[1] * 256 + headerFinal[0];
    height = headerFinal[3] * 256 + headerFinal[2];
    bpp = headerFinal[4];
    if(width <= 0 || height <= 0 || (bpp != 32 && bpp != 24))
    {
        std::cout << "TGA CONSTRUCTS NOT VALID" << std::endl;
        return false;
    }
    if(!srgb)
    {
        if(bpp == 24) 
        {
            type = GL_RGB;
            internalType = GL_RGB;
        }
        else 
        {
            type = GL_RGBA;
            internalType = GL_RGBA;
        }
    }
    else
    {
        if(bpp == 24) 
        {
            type = GL_RGB;
            internalType = GL_SRGB;
        }
        else 
        {
            type = GL_RGBA;
            internalType = GL_SRGB_ALPHA;
        }
    }
    bytesPerPixel = bpp / 8;
    imageSize = bytesPerPixel * width * height;
    return true;
}
void tgaLoader::freeData()
{
    free(data);
}