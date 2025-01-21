#pragma once
#include <Core.h>
#include <glad.h>
#include <glm.hpp>
#include <memory>
#include <gtc/type_ptr.hpp>

#include "Secs.h"


struct Shader
{
    GLuint program;
};

struct Mesh {
    GLuint VAO;
    GLuint VBO, EBO;
    GLsizei indexCount;
};

struct Camera
{
    float fov;
};

struct Light {
    glm::vec3 color = glm::vec3(1.0f, 0.8f, 0.6f); 
};


class RenderSystem
{
public:
    static int Render(secs::World& world, secs::ComponentRegistry& compReg);
    static void RegisterComponents(secs::World* world, secs::ComponentRegistry* compReg);
    static void CreateSystems(secs::ComponentRegistry* compReg, std::vector<secs::System>* systems);
};

