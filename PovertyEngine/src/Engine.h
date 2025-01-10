#pragma once
#include <SDL_video.h>
#include <string>
#include <unordered_map>
#include "Core.h"
struct Mesh;

class PE_API Engine
{
public:
	Engine();
	~Engine();
	static void Init();
	static void Input();
	static void MainLoop();
	static void CleanUp();
private:
	static SDL_GLContext glContext;
	static SDL_Window* graphicsApplicationWindow;
	static bool quit;
	static unsigned int VAO;
	static unsigned int VBO;
	static unsigned int EBO;

};

Engine::Engine()
{
}

Engine::~Engine()
{
}