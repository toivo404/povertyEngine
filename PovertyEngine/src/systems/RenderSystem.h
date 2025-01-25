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


class RenderSystem
{
public:
    static int Render(secs::World& world);
    static void RegisterComponents(secs::World* world);
};

