#include <SDL.h>
#include <glad.h>
#include "Engine.h"
#include <filesystem>
#include <iostream>
#include "AssimpLoader.h"
#include "ShaderLoader.h"

SDL_GLContext Engine::glContext = nullptr;
SDL_Window* Engine::graphicsApplicationWindow = nullptr;
bool Engine::quit = false;
unsigned int Engine::shader = 0;
unsigned int Engine::VAO = 0;
unsigned int Engine::VBO = 0;
unsigned int Engine::EBO = 0;

std::vector<float> vertices;
std::vector<unsigned int> indices;

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    char* basePath = SDL_GetBasePath();
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
    AssimpLoader loader;

    // Load the model
    if (loader.LoadModel(std::string(basePath) + "shaders\\monkey.obj", vertices, indices)) {
        std::cout << "Model loaded successfully!" << std::endl;
    } else {
        std::cerr << "Failed to load model!" << std::endl;
        exit(1);
    }
    SDL_free(basePath);
    MainLoop();
    CleanUp();
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

    // Create and bind VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Configure vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Texture coords

    glBindVertexArray(0);

    // Main render loop
    while (!quit)
    {
        Input();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(graphicsApplicationWindow);
    }
}

void Engine::CleanUp()
{
    glDeleteProgram(shader);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(graphicsApplicationWindow);
    SDL_Quit();
}
