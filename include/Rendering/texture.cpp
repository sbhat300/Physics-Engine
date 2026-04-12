#include "texture.h"
#include <glad/glad.h>

texture::texture() 
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char temp[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp);
    glGenerateMipmap(GL_TEXTURE_2D);
}

texture::texture(texture&& other) noexcept 
{
    textureID = other.textureID; 
    other.textureID = 0;
}

texture::~texture()
{
    glDeleteTextures(1, &textureID);
}

void texture::loadTexture(const char* path)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    imgLoader.load(path, false); 
    glTexImage2D(GL_TEXTURE_2D, 0, imgLoader.internalType, imgLoader.width, imgLoader.height, 0, imgLoader.type, GL_UNSIGNED_BYTE, imgLoader.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    imgLoader.freeData();
}

void texture::loadTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    unsigned char temp[4] = {r, g, b, a};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp);
    glGenerateMipmap(GL_TEXTURE_2D);
}

texture& texture::operator=(texture&& other) noexcept 
{
    if (this != &other) 
    {
        glDeleteTextures(1, &textureID); 
        textureID = other.textureID;     
        other.textureID = 0;             
    }
    return *this;
}