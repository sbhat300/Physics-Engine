#ifndef TEXTURE_H
#define TEXTURE_H

#include <FileLoader/tgaLoader.h>

class texture 
{
    public:
        unsigned int textureID;
        ~texture();
        texture();
        texture(texture&& other) noexcept;
        texture(const texture&) = delete;
        texture& operator=(const texture&) = delete;
        texture& operator=(texture&& other) noexcept;
        void loadTexture(const char* path);
        void loadTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
        void deleteTexture();

    private:
        tgaLoader imgLoader;
};

#endif