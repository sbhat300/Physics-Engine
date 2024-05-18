#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
  

class Shader
{
public:
    unsigned int ID;
  
    Shader(const char* vertexPath, const char* fragmentPath);
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, float value1, float value2, float value3) const;
    void setVec2(const std::string &name, float value1, float value2) const;
    void setMat4(const std::string &name, glm::mat4 val) const;
};
  
#endif