#pragma once
#include <glad.h>
#include <unordered_map>
#include <xstring>

class ShaderLoader
{
public:
    static GLuint CompileShaderProgram(const std::string& vertPath, const std::string& fragPath, char* basePath);
    static void Init(char* basePath);
    static GLuint GetShaderProgram(const std::string& name);
    static void CleanUp();
private:    
    static std::unordered_map<std::string, GLuint> shaders;
};
