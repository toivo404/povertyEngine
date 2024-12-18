#include "ShaderLoader.h"
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <glad.h>
#include <iostream>
#include <unordered_map>

std::unordered_map<std::string, GLuint> ShaderLoader::shaders;

void ShaderLoader::Init() {
    std::cout<<"ShaderLoader::Init" << std::endl;
    shaders["basic"] = CompileShaderProgram("shaders/simple.vert", "shaders/simple.frag");
}

std::string LoadShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint ShaderLoader::CompileShaderProgram(const std::string& vertPath, const std::string& fragPath) {
    std::string vertCode = LoadShaderSource(vertPath);
    std::string fragCode = LoadShaderSource(fragPath);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertSource = vertCode.c_str();
    glShaderSource(vertexShader, 1, &vertSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation error:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragSource = fragCode.c_str();
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation error:\n" << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        throw std::runtime_error("Shader program linking failed:\n" + std::string(infoLog));
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}



GLuint ShaderLoader::GetShaderProgram(const std::string& name) {
    return shaders[name];
}