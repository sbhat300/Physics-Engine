#ifndef TGALOADER_H
#define TGALOADER_H

#include <stdio.h>

class tgaLoader
{
    public:
        int width, height;
        unsigned char* data;
        int bpp;
        int type; //0 = rgb, 1 = rgba
        int internalType;
        int imageSize;
        int bytesPerPixel;
        bool load(const char* path, bool srgb = false, bool flip = true);
        void freeData();
    private:
        // Uncompressed TGA Header
        const char uncompressedHeader[12] = {0,0, 2,0,0,0,0,0,0,0,0,0};
        // Compressed TGA Header
        const char compressedHeader[12] = {0,0,10,0,0,0,0,0,0,0,0,0};
        bool loadUncompressed(const char* path, FILE* tgaFile, bool flip = true);
        bool loadCompressed(const char* path, FILE* tgaFile, bool flip = true);
        bool loadHeader(const char* path, FILE* tgaFile, bool srgb);

};

#endif