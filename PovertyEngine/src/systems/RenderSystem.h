#pragma once
#include <Core.h>
#include <flecs.h>
#include <glad.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>


struct PE_API Shader
{
    GLuint program;
};

struct PE_API Mesh {
    GLuint VAO;
    GLuint VBO, EBO;
    GLsizei indexCount;
};

struct PE_API Camera
{
    float fov;
};

struct PE_API Light {
    glm::vec3 color = glm::vec3(1.0f, 0.8f, 0.6f); 
};


class RenderSystem
{
public:
    static void SetupSystems(flecs::world& ecs);
    static void Render(flecs::world& ecs);
};

