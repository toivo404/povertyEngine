#pragma once
#include <SDL_video.h>
#include <vector>
#include <fwd.hpp>
#include <memory>
#include <vec3.hpp>
#include "Core.h"
#include "GameClient.h"
#include "ImGUIHelper.h"
#include "MaterialCache.h"
#include "systems/RenderSystem.h"

class PE_API Engine
{
public:
	static std::vector<secs::System> systems;

	static void Init(GameClient* gameClientImplementation);
	static void ProcessEvents();
	static void MainLoop();
	static void CleanUp();
	static bool	IsKeyPressed(SDL_Keycode key);
	static bool GetMouseButton(int button);
	static bool GetMouseButtonDown(int button);
	static bool GetMouseButtonUp(int button);
	static std::pair<int, int> GetMousePosition();
	static GLuint GetShader(const std::string& str);
	static Material GetMaterial(const std::string& str);
	static Mesh GetMesh(const std::string& string);
	static void AddSystem(secs::System& createSystem);
	static bool GetKeyUp(SDL_Keycode key);

	static GameClient* client;
	static char* baseFilePath;
	static float deltaTime;
	static double time;
	static glm::vec3 camPos;
	static glm::vec3 camLook;
	static glm::vec3 camUp;
	static glm::vec3 lightDir;
	static glm::vec3 lightColor;
	static glm::mat4 camMatrix;
	
private:
	static SDL_GLContext glContext;
	static SDL_Window* graphicsApplicationWindow;
	static bool quit;

	static std::unordered_map<SDL_Keycode, bool> heldKeys;
	static std::unordered_map<SDL_Keycode, bool> justPressedKeys;
	static std::unordered_map<SDL_Keycode, bool> justReleasedKeys;
};
