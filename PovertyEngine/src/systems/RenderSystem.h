#pragma once

#include <flecs.h>
#include <glad.h>
#include <glm.hpp>
#include <string>
#include <vector>
#include <gtc/type_ptr.hpp>

struct Material {
    glm::vec3 objectColor;
    GLuint shaderProgram;
};

struct Texture {
    GLuint id;
    std::string type; // e.g., "diffuse", "specular"
    std::string path;
};

struct Mesh {
    GLuint VAO;
    GLuint VBO, EBO;
    GLsizei indexCount;
    bool shared;
    std::vector<Texture> textures; // Add this for textures
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

