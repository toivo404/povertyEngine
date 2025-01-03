#include <SDL.h>
#include <glad.h>
#include "Engine.h"
#include <filesystem>
#include <iostream>
#include "AssimpLoader.h"
#include "ShaderLoader.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <flecs.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Components
struct Transform {
    glm::vec3 position = glm::vec3(0.0f);  // Entity position
    glm::vec3 scale = glm::vec3(1.0f);     // Entity scale
    glm::vec3 rotation = glm::vec3(0.0f);  // Rotation angles (in degrees) around x, y, z axes
    glm::mat4 model = glm::mat4(1.0f);     // Computed model matrix
};

struct Material {
    glm::vec3 objectColor;
    GLuint shaderProgram;
};

struct Mesh {
    GLuint VAO;         // Unique to each entity
    GLuint VBO, EBO;    // Shared across entities for the same mesh
    GLsizei indexCount; // Number of indices
    bool shared;        // Whether VBO and EBO are shared
};
struct Camera {
    glm::mat4 projection;  // Projection matrix
    float fov = 45.0f;     // Field of view (for perspective cameras)
    float near = 0.1f;     // Near clipping plane
    float far = 100.0f;    // Far clipping plane
    bool active = true;    // Whether this camera is active
};
SDL_GLContext Engine::glContext = nullptr;
SDL_Window* Engine::graphicsApplicationWindow = nullptr;
bool Engine::quit = false;
unsigned int Engine::shader = 0;
unsigned int Engine::VAO = 0;
unsigned int Engine::VBO = 0;
unsigned int Engine::EBO = 0;
char* basePath;

void Engine::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL2 failed to init";
        exit(1);
    }
    SDL_version compiled, linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    std::cout << "Compiled against SDL version: "
              << (int)compiled.major << "." << (int)compiled.minor << "." << (int)compiled.patch << std::endl;
    std::cout << "Linked against SDL version: "
              << (int)linked.major << "." << (int)linked.minor << "." << (int)linked.patch << std::endl;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    std::cout << "hello world";

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    basePath = SDL_GetBasePath();
    graphicsApplicationWindow = SDL_CreateWindow("babbys first opengl", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
    if (graphicsApplicationWindow == nullptr)
    {
        std::cout << "SDL_Window creation failed";
        exit(1);
    }
    glContext = SDL_GL_CreateContext(graphicsApplicationWindow);
    if (!glContext) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        exit(1);
    }
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    ShaderLoader::Init(basePath);

 //   // Load the model
 //   if (loader.LoadModel(std::string(basePath) + "shaders\\monkey.obj")) {
 //       std::cout << "Model loaded successfully!" << std::endl;
 //   } else {
 //       std::cerr << "Failed to load model!" << std::endl;
 //       exit(1);
 //   }
    MainLoop();
    CleanUp();
}
std::unordered_map<std::string, Mesh> meshCache;

Mesh GetMesh(const std::string& path) {
    // Check if the mesh is already in the cache
    auto it = meshCache.find(path);
    if (it != meshCache.end()) {
        // Create a new VAO while reusing the cached VBO and EBO
        Mesh cachedMesh = it->second;

        GLuint newVAO;
        glGenVertexArrays(1, &newVAO);
        glBindVertexArray(newVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cachedMesh.VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cachedMesh.EBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);

        return { newVAO, cachedMesh.VBO, cachedMesh.EBO, cachedMesh.indexCount, true };
    }

    // Load the model using a loader
    AssimpLoader loader;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    if (!loader.LoadModel(path, vertices, indices)) {
        std::cerr << "Failed to load model from: " << path << std::endl;
        exit(1);
    }

    // Create buffers
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    // Cache the shared buffers (VBO and EBO) and return the unique VAO
    Mesh mesh = { VAO, VBO, EBO, static_cast<GLsizei>(indices.size()), false };
    meshCache[path] = mesh;

    return mesh;
}

void Engine::Input()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            std::cout << "goodbye" << std::endl;
            quit = true;
        }
    }
}

void Engine::MainLoop()
{
    shader = ShaderLoader::GetShaderProgram("basic");

    flecs::world ecs;

    // Create an entity for your renderable object
//    auto entity = ecs.entity("RenderableObject")
//                     .set<Transform>({
//                         glm::vec3(0, 0.0f, 0), // Position
//                         glm::vec3(1.0f, 1.0f, 1.0f), // Scale
//                         glm::vec3(0.0f, 45.0f, 0.0f) // Rotation (Y-axis rotation)
//                     }) // Default transform
//                     .set<Material>({glm::vec3(0.6f, 0.2f, 0.8f), ShaderLoader::GetShaderProgram("basic")})
//                     .set<Mesh>(GetMesh("shaders/monkey.obj"));
    // Camera 1
    auto camera1 = ecs.entity("MainCamera")
        .set<Transform>({
            glm::vec3(0.0f, 0.0f, 5.0f), // Position
            glm::vec3(1.0f),             // Scale
            glm::vec3(0.0f)              // Rotation
        })
        .set<Camera>({
            glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f), // Projection
            45.0f, 0.1f, 100.0f, true
        });
    auto entity2 = ecs.entity("RenderableObject2")
                  .set<Transform>({
                      glm::vec3(1, 0.0f, 0), // Position
                      glm::vec3(1.0f, 1.0f, 1.0f), // Scale
                      glm::vec3(0.0f, 45.0f, 0.0f) // Rotation (Y-axis rotation)
                  }) // Default transform
                  .set<Material>({glm::vec3(0.6f, 0.2f, 0.8f), ShaderLoader::GetShaderProgram("basic")})
                  .set<Mesh>(GetMesh("shaders/monkey.obj"));

  // auto object2 = ecs.entity("Object2")
  //     .set<Transform>({
  //         glm::vec3(2.0f, 0.0f, -3.0f), // Position
  //         glm::vec3(1.0f, 1.0f, 1.0f), // Scale
  //         glm::vec3(0.0f, 45.0f, 0.0f) // Rotation (Y-axis rotation)
  //     })
  //     .set<Material>({ glm::vec3(0.2f, 0.6f, 0.2f), ShaderLoader::GetShaderProgram("basic") })
  //     .set<Mesh>(GetMesh("shaders/monkey.obj"));
    ecs.system<Camera, Transform>()
      .each([&](flecs::entity camEntity, const Camera& cam, const Transform& camTransform) {
          if (!cam.active) return; // Skip inactive cameras

          // Compute the view matrix from the camera's Transform
          glm::mat4 view = glm::lookAt(
              camTransform.position,                         // Camera position
              camTransform.position + glm::vec3(0.0f, 0.0f, -1.0f), // Forward direction
              glm::vec3(0.0f, 1.0f, 0.0f)                    // Up vector
          );

          // Render all entities with Mesh, Transform, and Material for this camera
          ecs.system<Mesh, Transform, Material>()
              .each([&](flecs::entity objEntity, const Mesh& mesh, const Transform& objTransform, const Material& material) {
                  // Use the shader program from the Material
                  glUseProgram(material.shaderProgram);

                  // Set the camera's view and projection matrices in the shader
                  GLint viewLoc = glGetUniformLocation(material.shaderProgram, "view");
                  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

                  GLint projLoc = glGetUniformLocation(material.shaderProgram, "projection");
                  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam.projection));

                  // Set the object's model matrix
                  GLint modelLoc = glGetUniformLocation(material.shaderProgram, "model");
                  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objTransform.model));

                  // Draw the mesh
                  glBindVertexArray(mesh.VAO);
                  glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
                  glBindVertexArray(0);
              });
      });

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Main render loop
    while (!quit)
    {
        Input();

        // Clear screen with dynamic color
        float time = SDL_GetTicks() / 1000.0f;
        float color = (sin(time) * 0.5f) + 0.5f;
        glClearColor(color, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Progress the ECS world
        ecs.progress();
        // Draw model
      

        // Swap buffers
        SDL_GL_SwapWindow(graphicsApplicationWindow);
    }
}

void Engine::CleanUp()
{
    SDL_free(basePath);
    glDeleteProgram(shader);
    for (auto& [path, mesh] : meshCache) {
        glDeleteVertexArrays(1, &mesh.VAO);
        if (!mesh.shared) {
            glDeleteBuffers(1, &mesh.VBO);
            glDeleteBuffers(1, &mesh.EBO);
        }
    }
    meshCache.clear();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(graphicsApplicationWindow);
    SDL_Quit();
}
