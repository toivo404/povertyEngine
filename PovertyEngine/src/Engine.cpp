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
    std::cout << "hello world";

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

    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    // Main render loop
    while (!quit)
    {
        Input();

        // Light properties
        glm::vec3 lightColor(1.0f, 0.8f, 0.6f);  // Slightly warm light
        glm::vec3 lightPos(5.0f, 5.0f, 5.0f);    // Position above and to the side
        glm::vec3 viewPos(0.0f, 0.0f, 3.0f);     // Camera position

        // Pass light properties to shader
        glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(viewPos));

        // Object color
        glm::vec3 objectColor(0.6f, 0.2f, 0.8f); // Purple object
        glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, glm::value_ptr(objectColor));

        // Clear screen
        float time = SDL_GetTicks() / 1000.0f;
        Uint32 ticks = SDL_GetTicks();
        float color = (sin(time) * 0.5f) + 0.5f; // Oscillates between 0.0 and 1.0
        glClearColor(color, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear with new color
        
        std::cout << "Raw SDL_GetTicks(): " << ticks << " ms" << std::endl;
        std::cout << "Elapsed Time: " << time << " seconds" << std::endl;

        // Create rotation matrix
       glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(50.0f) * time, glm::vec3(0.0f, 1.0f, 0.0f));

      // Pass rotation matrix to shader
       GLint transformLoc = glGetUniformLocation(shader, "transform");
       glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(rotation));

        // Draw model
       glUseProgram(shader);
       glBindVertexArray(VAO);
       glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
       glBindVertexArray(0);

        // Swap buffers
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
