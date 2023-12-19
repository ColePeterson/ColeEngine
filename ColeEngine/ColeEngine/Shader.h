#pragma once



#include"glew.h"

#include <string>

class ShaderProgram
{
public:
    int programId;

    ShaderProgram(std::string _name);
    void AddShader(std::string fileName, const GLenum type);
    void LinkProgram();
    void UseShader();
    void UnuseShader();

    std::string getName() { return name; }

    std::string shaderSrc;

private:
    std::string name;
    
};

