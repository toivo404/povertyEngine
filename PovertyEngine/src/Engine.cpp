#include "Engine.h"
#include <SDL.h>
#include <glad.h>
#include <filesystem>
#include <iostream>
#include "AssimpLoader.h"
#include "ShaderLoader.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <Secs.h>
#include <cstdlib>

#include "DebugLineRenderer.h"
#include "EngineInfo.h"
#include "imgui.h"
#include "ImGUIHelper.h"
#include "MeshCache.h"
#include "MaterialCache.h"
#include "systems/RenderSystem.h"

GameClient*                           Engine::client;
char*                                 Engine::baseFilePath;
glm::mat4                             Engine::camMatrix;
glm::vec3                             Engine::camPos  = glm::vec3(0.0f, 5.0f, 10.0f); 
glm::vec3                             Engine::camLook = glm::vec3(0.0f, 0.0f, 0.0f);  
glm::vec3                             Engine::camUp   = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4                             Engine::projectionMatrix;
glm::mat4                             Engine::viewMatrix;
glm::vec3                             Engine::lightColor = glm::vec3(1.0f, 0.8f, 0.6f); 
glm::vec3                             Engine::lightDir = glm::vec3(-0.5f, -1.0f, -0.5f); 
std::vector<secs::System>             Engine::systems;
std::unordered_map<SDL_Keycode, bool> Engine::heldKeys;        
std::unordered_map<SDL_Keycode, bool> Engine::justPressedKeys; 
std::unordered_map<SDL_Keycode, bool> Engine::justReleasedKeys;
std::unordered_map<int, bool>         Engine::heldMouseButtons;
std::unordered_map<int, bool>         Engine::justPressedMouseButtons;
std::unordered_map<int, bool>         Engine::justReleasedMouseButtons;


float Engine::deltaTime;
double Engine::time;


SDL_GLContext Engine::glContext = nullptr;
SDL_Window* Engine::graphicsApplicationWindow = nullptr;
bool Engine::quit = false;
int windowWidth = 640;
int windowHeight= 480;
int mouseX;
int mouseY;

bool deltaTimeCalculated;
ImGUIHelper imguiHelper;
DebugLineRenderer* debugLineRenderer;
GLuint debugLineShader;

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
    auto dr = DebugLineRenderer{};
    debugLineRenderer = &dr;
    debugLineShader = ShaderLoader::GetShaderProgram("debugline");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    imguiHelper.Init(graphicsApplicationWindow, glContext);

    secs::ComponentRegistry::registerType<AABB>("AABB");
    RenderSystem::RegisterComponents(&client->world);
    RenderSystem::CreateSystems( &systems);
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

void Engine::DebugDrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, float lineWidth)
{
    debugLineRenderer->DrawLine(start, end, color, lineWidth);
}

// Mouse input
bool Engine::GetMouseButton(int button) {
    return heldMouseButtons.find(button) != heldMouseButtons.end();
}

bool Engine::GetMouseButtonDown(int button) {
    return justPressedMouseButtons.find(button) != justPressedMouseButtons.end();
}

bool Engine::GetMouseButtonUp(int button) {
    return justReleasedMouseButtons.find(button) != justReleasedMouseButtons.end();
}

std::pair<int, int> Engine::GetMousePosition()
{
    return {mouseX, mouseY};
}

void Engine::MousePositionToRay(glm::vec3& origin, glm::vec3& dir) {

    // Normalize mouse coordinates to range [-1, 1]
    float x = (2.0f * mouseX) / windowWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / windowHeight; // Flip Y for screen space
    float z = 1.0f; // Default depth (far plane)

    // Create a normalized device coordinate (NDC) vector
    glm::vec4 rayNDC = glm::vec4(x, y, z, 1.0f);

    // Transform NDC to clip space (inverse of projection matrix)
    glm::vec4 rayClip = glm::inverse(projectionMatrix) * rayNDC;

    // Transform clip space to view space
    glm::vec4 rayView = glm::vec4(rayClip.x, rayClip.y, -1.0f, 0.0f); // Z = -1 for direction

    // Transform view space to world space
    glm::vec3 rayWorld = glm::vec3(glm::inverse(viewMatrix) * rayView);
    rayWorld = glm::normalize(rayWorld); // Normalize the direction vector

    // Ray origin is the camera position
    origin = glm::vec3(glm::inverse(viewMatrix)[3]);

    // Ray direction is the normalized world-space direction
    dir = rayWorld;
}

bool Engine::IsOnScreen(const glm::vec3& position)
{
    glm::vec4 clipSpacePos = projectionMatrix * viewMatrix * glm::vec4(position, 1.0f);

    // Perspective divide to normalize the position
    if (clipSpacePos.w != 0.0f)
    {
        clipSpacePos.x /= clipSpacePos.w;
        clipSpacePos.y /= clipSpacePos.w;
        clipSpacePos.z /= clipSpacePos.w;
    }
    else
    {
        return false; // Avoid division by zero, not on screen
    }

    // Check if the normalized device coordinates are within [-1, 1]
    if (clipSpacePos.x >= -1.0f && clipSpacePos.x <= 1.0f &&
        clipSpacePos.y >= -1.0f && clipSpacePos.y <= 1.0f &&
        clipSpacePos.z >= -1.0f && clipSpacePos.z <= 1.0f)
    {
        return true;
    }

    return false; // Out of bounds
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

AABB Engine::GetAABB(const std::string& string)
{
    return  MeshCache::GetAABB(string);
}

void Engine::AddSystem(secs::System& system)
{
    systems.push_back(system); 
}
bool Engine::GetKey(SDL_Keycode key)
{
    return heldKeys.find(key) != heldKeys.end();
}

bool Engine::GetKeyUp(SDL_Keycode key)
{
    return justReleasedKeys.find(key) != justReleasedKeys.end();
}

const char* currentMessage = nullptr;

void Engine::DisplayMessage(const char* message)
{
    currentMessage = message;
}


void Engine::ProcessEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            std::cout << "goodbye" << std::endl;
            quit = true;
        }

        if (e.type == SDL_KEYDOWN && !e.key.repeat)
        {
            SDL_Keycode key = e.key.keysym.sym;

            if (heldKeys.find(key) == heldKeys.end())
            {
                justPressedKeys[key] = true;
                heldKeys[key] = true;
            }
        }
        else if (e.type == SDL_KEYUP)
        {
            SDL_Keycode key = e.key.keysym.sym;

            // Remove from heldKeys and add to justReleasedKeys
            if (heldKeys.find(key) != heldKeys.end())
            {
                justReleasedKeys[key] = true;
                heldKeys.erase(key);
            }
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            int button = e.button.button; // SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT, etc.
            justPressedMouseButtons[button] = true;
            if (heldMouseButtons.find(button) == heldMouseButtons.end())
            {
                heldMouseButtons[button] = true;
            }
        }
        else if (e.type == SDL_MOUSEBUTTONUP)
        {
            int button = e.button.button;
            justReleasedMouseButtons[button] = true;
            if (heldMouseButtons.find(button) != heldMouseButtons.end())
            {
                heldMouseButtons.erase(button);
            }
        }
        else if (e.type == SDL_MOUSEMOTION)
        {
            mouseX = e.motion.x;
            mouseY = e.motion.y;
        }


    imguiHelper.OnSDLEvent(e);
    }
}

float CalculateFPS(float& deltaTime) {
    static float frameCount = 0;
    static float fps = 0.0f;
    static float timeAccumulated = 0.0f;

    frameCount++;
    timeAccumulated += deltaTime;

    // Update FPS every second
    if (timeAccumulated >= 1.0f) {
        fps = frameCount / timeAccumulated; // Calculate FPS
        frameCount = 0;                     // Reset frame count
        timeAccumulated = 0.0f;             // Reset time accumulator
    }

    return fps;
}

void Engine::MainLoop()
{
    static Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    time += deltaTime;

    ProcessEvents();
    
    glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    imguiHelper.OnFrameStart();
    
    /*
    if (updateCallback) {
        updateCallback();
    }
    */
    client->OnUpdate(deltaTime);

    for (auto system : systems)
        system.execute(client->world);

    viewMatrix = glm::lookAt(camPos, camLook, camUp);

    projectionMatrix = glm::perspective(
        glm::radians(45.0f),
        (float)windowWidth / (float)windowHeight,
        0.1f, 1000.0f
    );

    camMatrix = projectionMatrix * viewMatrix;

 
    if (currentMessage != nullptr)
    {
        ImGui::Begin("hello");
        ImGui::Text( currentMessage );
        ImGui::End();    
    }


    const int renderedCount = RenderSystem::Render(client->world);
    debugLineRenderer->Render(viewMatrix, projectionMatrix, debugLineShader);

    ImGui::Begin("stats");
    ImGui::Text(Combine("deltaTime: ", deltaTime, "\n", "time: ", time, "\n", "fps: ", CalculateFPS(deltaTime), "\n", "renderedObjects: ", renderedCount).c_str());
    ImGui::End();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        std::cerr << "OpenGL error: " << error << std::endl;

    imguiHelper.OnFrameEnd();

    justPressedKeys.clear();
    justReleasedKeys.clear();
    justPressedMouseButtons.clear();
    justReleasedMouseButtons.clear();
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
