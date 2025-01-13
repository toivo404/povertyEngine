#pragma once

#include <flecs.h>
#include <glad.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>


struct Shader
{
    GLuint program;
};

struct Mesh {
    GLuint VAO;
    GLuint VBO, EBO;
    GLsizei indexCount;
    bool shared;
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
    static void SetupSystems(flecs::world& ecs);
    static void Render(flecs::world& ecs);
};

