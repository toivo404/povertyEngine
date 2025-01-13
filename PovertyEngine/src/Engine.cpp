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
#include <flecs.h>
#include <cstdlib>
#include "EngineInfo.h"
#include "imgui.h"
#include "ImGUIHelper.h"
#include "MeshCache.h"
#include "systems/ManipulatorSystem.h"
#include "systems/RenderSystem.h"
#include "systems/SpinSystem.h"
#include "systems/TransformSystem.h"

glm::mat4 Engine::camMatrix;
glm::vec3 Engine::camPos  = glm::vec3(0.0f, 5.0f, 10.0f); 
glm::vec3 Engine::camLook = glm::vec3(0.0f, 0.0f, 0.0f);  
glm::vec3 Engine::camUp   = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Engine::lightColor = glm::vec3(1.0f, 0.8f, 0.6f); 
glm::vec3 Engine::lightDir = glm::vec3(-0.5f, -1.0f, -0.5f); 
float Engine::deltaTime;

SDL_GLContext Engine::glContext = nullptr;
SDL_Window* Engine::graphicsApplicationWindow = nullptr;
bool Engine::quit = false;
int windowWidth = 640;
int windowHeight= 480;
char* basePath;

flecs::world ecs;
Mesh monkeyMesh;
std::vector<flecs::entity> monkeys;
bool deltaTimeCalculated;
ImGUIHelper imguiHelper;



void Engine::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL2 failed to init";
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
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
    
    glViewport(0, 0, windowWidth, windowHeight);

    EngineInfo::LogInfo();
    ShaderLoader::Init(basePath);
    
    monkeyMesh = MeshCache::GetMesh("shaders/monkey.obj");
    ManipulatorSystem::RegisterSystem(ecs);
    TransformSystem::RegisterSystem(ecs);
    SpinSystem::RegisterSystem(ecs);
 
    RenderSystem::SetupSystems(ecs);   
    glm::vec3 origCamTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 origDirection = glm::normalize(origCamTarget - camPos);

    float pitch = glm::degrees(asin(origDirection.y)); 
    float yaw = glm::degrees(atan2(origDirection.z, origDirection.x)); 

    ecs.entity("Main Camera")
       .set<Transform>({
           glm::vec3(0.0f, 5.0f, 10.0f), 
           glm::vec3(1.0f),
           glm::vec3(pitch, yaw, 0.0f) 
       })
       .set<Camera>({42})
       .set<Manipulator>({2, 50});
    ecs.entity("Main Light")
       .set<Transform>({
           glm::vec3(0.0f, 0.0f, 0.0f), 
           glm::vec3(0,0,0 ), 
           glm::vec3(15,0, 0.0f) 
       })
      
       .set<Light>({glm::vec3(1.0f, 1.0f, 1.0f)})
       .set<Spin>({50});
   
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    flecs::entity prevEntity;
    
    imguiHelper.Init(graphicsApplicationWindow, glContext);
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


::flecs::entity PlaceMonkey(flecs::world& ecs, const Mesh& monkeyMesh, const glm::vec3& position, const glm::vec3& color, const std::string& name = "Monkey")
{
    return ecs.entity(name.c_str())
              .set<Transform>({
                  position,
                  glm::vec3(1.0f),
                  glm::vec3(0.0f, 45.0f, 0.0f)
              })
              .set<Material>({
                  color,
                  ShaderLoader::GetShaderProgram("basic")
              })
              .set<Mesh>(monkeyMesh)
              .set<Spin>({50.0f});
}

glm::vec3 GetRandomColor()
{
    const glm::vec3 baseColor(0.6f, 0.2f, 0.8f);
    return  baseColor + glm::vec3(
         static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
         static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
         static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f
     );
}

void PopulateMonkeys(flecs::world& ecs, const Mesh& monkeyMesh) {
    const float spacing = 2.5f; 
    const int count = 20;      
    for (int i = 0; i < count; ++i) {
        float x = (i % 5) * spacing - 5.0f;
        float z = (i / 5) * spacing - 5.0f;

     

        PlaceMonkey(ecs, monkeyMesh, glm::vec3(x, 0.0f, z), GetRandomColor(), "Monkey" + std::to_string(i + 1));
    }
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
void InputMonkeys()
{
    if (Engine::IsKeyPressed(SDLK_PERIOD))
    {
        auto newMonkey = PlaceMonkey(ecs, monkeyMesh, glm::vec3(monkeys.size(), 0, 0), GetRandomColor(),
                                     "monkey" + std::to_string(monkeys.size()));
        monkeys.push_back(newMonkey);
        std::cout << "Added a monkey! Total monkeys: " << monkeys.size() << std::endl;
    }
    if (Engine::IsKeyPressed(SDLK_COMMA))
    {
        if (!monkeys.empty())
        {
            auto lastMonkey = monkeys.back();
            lastMonkey.destruct();
            monkeys.pop_back(); 
            std::cout << "Removed a monkey! Total monkeys: " << monkeys.size() << std::endl;
        }
        else
        {
            std::cout << "No monkeys left to remove!" << std::endl;
        }
    }
}


void Engine::MainLoop()
{
    static Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    ProcessEvents();
    InputMonkeys();
    
    ecs.progress();

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


    RenderSystem::Render(ecs);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        std::cerr << "OpenGL error: " << error << std::endl;

    imguiHelper.OnFrameEnd();
    
    SDL_GL_SwapWindow(graphicsApplicationWindow);
}

void Engine::CleanUp()
{
    SDL_free(basePath);
    ShaderLoader::CleanUp();

    MeshCache::CleanUp();
    imguiHelper.CleanUp();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(graphicsApplicationWindow);
    SDL_Quit();
}
