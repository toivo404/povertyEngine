#pragma once
#include <SDL_video.h>
#include <vector>
#include <flecs.h>
#include <fwd.hpp>
#include <vec3.hpp>
#include "Core.h"
#include "ImGUIHelper.h"
struct Mesh;

class PE_API Engine
{
public:
	static void Init();
	static void ProcessEvents();
	static void SetupSystems(Mesh& monkeyMesh, std::vector<flecs::entity>& monkeys);
	static void MainLoop();
	static void CleanUp();
	static bool	IsKeyPressed(SDL_Keycode key);
	static float deltaTime;
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
	static unsigned int VAO;
	static unsigned int VBO;
	static unsigned int EBO;

};
