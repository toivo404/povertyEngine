#include <SDL.h>
#include <glad.h>
#include "Engine.h"
#include <common.hpp>
#include <filesystem>
#include <iostream>
#include "AssimpLoader.h"
#include "ShaderLoader.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <Secs.h>
#include <cstdlib>
#include "EngineInfo.h"
#include "imgui.h"
#include "ImGUIHelper.h"
#include "MeshCache.h"
#include "MaterialCache.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

GameClient* Engine::client;
char*   Engine::baseFilePath;
glm::mat4 Engine::camMatrix;
glm::vec3 Engine::camPos  = glm::vec3(0.0f, 5.0f, 10.0f); 
glm::vec3 Engine::camLook = glm::vec3(0.0f, 0.0f, 0.0f);  
glm::vec3 Engine::camUp   = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Engine::lightColor = glm::vec3(1.0f, 0.8f, 0.6f); 
glm::vec3 Engine::lightDir = glm::vec3(-0.5f, -1.0f, -0.5f); 
std::vector<secs::System> Engine::systems;
float Engine::deltaTime;

SDL_GLContext Engine::glContext = nullptr;
SDL_Window* Engine::graphicsApplicationWindow = nullptr;
bool Engine::quit = false;
int windowWidth = 640;
int windowHeight= 480;

bool deltaTimeCalculated;
ImGUIHelper imguiHelper;

void Engine::Init(GameClient* gameClientImplementation)
{
    client = gameClientImplementation;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL2 failed to init";
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    baseFilePath = SDL_GetBasePath();
    graphicsApplicationWindow = SDL_CreateWindow("babbys first opengl",0, 0, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
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
    
    glViewport(0, 0, windowWidth, windowHeight);

    EngineInfo::LogInfo();
    ShaderLoader::Init(baseFilePath);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    imguiHelper.Init(graphicsApplicationWindow, glContext);

    RenderSystem::RegisterComponents(&client->world, &client->componentRegistry);
    TransformSystem::RegisterComponents(&client->world, &client->componentRegistry);
    if(client->componentRegistry.getidToNameSize() == 0 || client->componentRegistry.getTypeToIdSize() == 0)
    {
        std::cerr << "Components not registering" << std::endl;
        quit = true;
    }
    if (client->componentRegistry.getTypeToIdSize() != client->componentRegistry.getidToNameSize())
    {
        std::cerr << "Something bad" << std::endl;
        quit = true;
    }
    
    RenderSystem::CreateSystems(&client->componentRegistry, &systems);
    systems.push_back(TransformSystem::CreateSystem(&client->componentRegistry));
    client->OnInit();
    std::cout << "Systems:" << systems.size() << std::endl;
    while (!quit)
    {
        MainLoop();
    }
    CleanUp();
}

bool Engine::IsKeyPressed(SDL_Keycode key)
{
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    return state[SDL_GetScancodeFromKey(key)];
}

GLuint Engine::GetShader(const std::string& str)
{
    return ShaderLoader::GetShaderProgram(str);
}

Material Engine::GetMaterial(const std::string& str)
{
    return MaterialSystem::LoadMaterial(str, baseFilePath);
}

Mesh Engine::GetMesh(const std::string& string)
{
    return  MeshCache::GetMesh(string);
}


void Engine::ProcessEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT) {
            std::cout << "goodbye" << std::endl;
            quit = true;
        }
        imguiHelper.OnSDLEvent(e);
    }
}

void Engine::MainLoop()
{
    static Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    ProcessEvents();

    /*
    if (updateCallback) {
        updateCallback();
    }
    */
    client->OnUpdate(deltaTime);

    client->world.progress(&systems);

    glm::mat4 view = glm::lookAt(camPos, camLook, camUp);

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)windowWidth / (float)windowHeight,
        0.1f, 100.0f
    );

    camMatrix = projection * view;

    float time = SDL_GetTicks() / 1000.0f;
    float color = (sin(time) * 0.5f) + 0.5f;
    glClearColor(color, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    imguiHelper.OnFrameStart();

    ImGui::Begin("Hello, World!"); 
    ImGui::Text("This is a Hello World window in ImGui!"); 
    ImGui::End(); 


    RenderSystem::Render();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        std::cerr << "OpenGL error: " << error << std::endl;

    imguiHelper.OnFrameEnd();
    
    SDL_GL_SwapWindow(graphicsApplicationWindow);
}

void Engine::CleanUp()
{
    client->OnShutdown();
    SDL_free(baseFilePath);
    ShaderLoader::CleanUp();

    MeshCache::CleanUp();
    imguiHelper.CleanUp();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(graphicsApplicationWindow);
    SDL_Quit();
}
