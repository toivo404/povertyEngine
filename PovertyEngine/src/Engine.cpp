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
#include <cstdlib> // For random color generation

// Components
struct Transform {
    glm::vec3 position = glm::vec3(0.0f);  
    glm::vec3 scale    = glm::vec3(1.0f);     
    glm::vec3 rotation = glm::vec3(0.0f);  
    glm::mat4 model    = glm::mat4(1.0f);     
};

struct Material {
    glm::vec3 objectColor;
    GLuint shaderProgram;
};

struct Mesh {
    GLuint VAO;       
    GLuint VBO, EBO;  
    GLsizei indexCount; 
    bool shared;       
};

// Instead of a camera component, we’ll just manually do the camera math

struct Manipulator {
    float moveSpeed   = 2.0f;  
    float rotateSpeed = 50.0f; 
};

SDL_GLContext Engine::glContext = nullptr;
SDL_Window* Engine::graphicsApplicationWindow = nullptr;
bool Engine::quit = false;
unsigned int Engine::shader = 0;
int windowWidth = 640;
int windowHeight= 480;
char* basePath;
std::unordered_map<std::string, Mesh> meshCache;

// Forward declarations
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

void Engine::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL2 failed to init";
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    basePath = SDL_GetBasePath();
    graphicsApplicationWindow = SDL_CreateWindow("babbys first opengl", 
        0, 0, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
    if (!graphicsApplicationWindow) {
        std::cerr << "SDL_Window creation failed\n";
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

    // IMPORTANT: set the viewport after creating the context
    glViewport(0, 0, windowWidth, windowHeight);

    ShaderLoader::Init(basePath);

    MainLoop();
    CleanUp();
}

void Engine::Input()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT) {
            std::cout << "goodbye" << std::endl;
            quit = true;
        }
        // You can add more input logic here if needed
    }
}

bool IsKeyPressed(SDL_Keycode key) {
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    return state[SDL_GetScancodeFromKey(key)];
}

float GetDeltaTime() {
    static Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f; 
    lastTime = currentTime;
    return deltaTime;
}

void PlaceMonkey(flecs::world& ecs, const Mesh& monkeyMesh, 
                 const glm::vec3& position, const glm::vec3& color,
                 const std::string& name = "Monkey") 
{
    ecs.entity(name.c_str())
        .set<Transform>({
            position,                // Position
            glm::vec3(1.0f),        // Scale
            glm::vec3(0.0f, 45.0f, 0.0f) // Rotation
        })
        .set<Material>({
            color, // Object color
            ShaderLoader::GetShaderProgram("basic") // Shader
        })
        .set<Mesh>(monkeyMesh);
}

void PopulateMonkeys(flecs::world& ecs, const Mesh& monkeyMesh) {
    const float spacing = 2.5f; 
    const int count = 20;      
    const glm::vec3 baseColor(0.6f, 0.2f, 0.8f);

    for (int i = 0; i < count; ++i) {
        float x = (i % 5) * spacing - 5.0f;
        float z = (i / 5) * spacing - 5.0f;

        glm::vec3 color = baseColor + glm::vec3(
            static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
            static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
            static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f
        );

        PlaceMonkey(ecs, monkeyMesh, glm::vec3(x, 0.0f, z), color, "Monkey" + std::to_string(i + 1));
    }
}

//----- MAIN LOOP -----
void Engine::MainLoop()
{
    shader = ShaderLoader::GetShaderProgram("basic");

    // Create ECS world
    flecs::world ecs;

    // Movement system for entities that have a Manipulator + Transform
    ecs.system<Manipulator, Transform>()
       .each([](flecs::entity e, Manipulator& manipulator, Transform& transform) {
           float deltaTime = GetDeltaTime();
           // Basic WASD
           if (IsKeyPressed(SDLK_w)) { transform.position.z -= manipulator.moveSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_s)) { transform.position.z += manipulator.moveSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_a)) { transform.position.x -= manipulator.moveSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_d)) { transform.position.x += manipulator.moveSpeed * deltaTime; }

           // Rotation with arrow keys
           if (IsKeyPressed(SDLK_UP))    { transform.rotation.x -= manipulator.rotateSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_DOWN))  { transform.rotation.x += manipulator.rotateSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_LEFT))  { transform.rotation.y -= manipulator.rotateSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_RIGHT)) { transform.rotation.y += manipulator.rotateSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_q))     { transform.rotation.z -= manipulator.rotateSpeed * deltaTime; }
           if (IsKeyPressed(SDLK_e))     { transform.rotation.z += manipulator.rotateSpeed * deltaTime; }
       });

    auto monkeyMesh = GetMesh("shaders/monkey.obj");
    // Place a single monkey at origin
    PlaceMonkey(ecs, monkeyMesh, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.5f, 0.2f));
    // Also place a bunch more
    PopulateMonkeys(ecs, monkeyMesh);

    // Maybe make a single manipulator entity if you want it to move
    ecs.entity("DummyController")
       .set<Manipulator>({2.0f, 50.0f});

    // System to update transforms
    ecs.system<Transform>()
       .each([](flecs::entity e, Transform& t)
       {
           glm::mat4 m(1.0f);
           m = glm::translate(m, t.position);
           m = glm::rotate(m, glm::radians(t.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
           m = glm::rotate(m, glm::radians(t.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
           m = glm::rotate(m, glm::radians(t.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
           m = glm::scale(m, t.scale);
           t.model = m;
       });

    // Enable depth testing (for 3D)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // MAIN RENDER LOOP
    while (!quit)
    {
        // --- Input & ECS Update ---
        Input();
        ecs.progress();

        // --- Build camera matrix manually (NO camera component) ---
        glm::vec3 camPos  = glm::vec3(0.0f, 5.0f, 10.0f); // Eye position
        glm::vec3 camLook = glm::vec3(0.0f, 0.0f, 0.0f);  // Look at origin
        glm::vec3 camUp   = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(camPos, camLook, camUp);

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 
            (float)windowWidth / (float)windowHeight,
            0.1f, 100.0f
        );

        // Combined
        glm::mat4 camMatrix = projection * view;

        // --- Clear screen ---
        float time = SDL_GetTicks() / 1000.0f;
        float color = (sin(time) * 0.5f) + 0.5f;
        glClearColor(color, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Draw all your entities ---
        //   Instead of letting a "camera system" store matrices, we 
        //   just pass camMatrix directly
        {
            auto drawQuery = ecs.query<Mesh, Transform, Material>();
            drawQuery.each([&](flecs::entity e, const Mesh& mesh, const Transform& transform, const Material& material)
            {
                glUseProgram(material.shaderProgram);

                // Pass camMatrix
                GLint camMatrixLoc = glGetUniformLocation(material.shaderProgram, "camMatrix");
                glUniformMatrix4fv(camMatrixLoc, 1, GL_FALSE, glm::value_ptr(camMatrix));

                // Pass model matrix
                GLint modelLoc = glGetUniformLocation(material.shaderProgram, "model");
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform.model));

                // Pass color
                GLint colorLoc = glGetUniformLocation(material.shaderProgram, "objectColor");
                glUniform3fv(colorLoc, 1, glm::value_ptr(material.objectColor));

                // Bind & draw
                glBindVertexArray(mesh.VAO);
                glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            });
        }

        // Check for errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
            std::cerr << "OpenGL error: " << error << std::endl;

        // Swap buffers
        SDL_GL_SwapWindow(graphicsApplicationWindow);
    }
}

void Engine::CleanUp()
{
    SDL_free(basePath);
    glDeleteProgram(shader);

    for (auto& [path, mesh] : meshCache)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        if (!mesh.shared)
        {
            glDeleteBuffers(1, &mesh.VBO);
            glDeleteBuffers(1, &mesh.EBO);
        }
    }
    meshCache.clear();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(graphicsApplicationWindow);
    SDL_Quit();
}
