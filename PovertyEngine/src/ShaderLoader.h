﻿#pragma once
#include <glad.h>
#include <unordered_map>
#include <xstring>

class ShaderLoader
{
public:
    static std::unordered_map<std::string, GLuint> shaders;
    static GLuint CompileShaderProgram(const std::string& vertPath, const std::string& fragPath);
    static void Init();
    static GLuint GetShaderProgram(const std::string& name);
};