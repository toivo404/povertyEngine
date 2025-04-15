#pragma once
#include <Core.h>
#include <glm.hpp>
#include <memory>
#include <gtc/type_ptr.hpp>

#include "Secs.h"
using ShaderHandle = uint32_t;
using MeshHandle = uint32_t;

struct Shader
{
    ShaderHandle program;
};

struct Mesh {
    uint32_t  VAO;
    uint32_t  VBO, EBO;
    uint32_t  indexCount;
};


class RenderSystem
{
public:
    static int Render(secs::World& world);
    static void RegisterComponents(secs::World* world);
};

