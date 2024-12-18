#include <SDL.h>
#include <glad.h>
#include "Engine.h"
#include <filesystem>
#include <iostream>
#include "ShaderLoader.h"

SDL_GLContext Engine::glContext = nullptr;
SDL_Window* Engine::graphicsApplicationWindow = nullptr;
bool Engine::quit = false;
unsigned int Engine::shader = 0;
unsigned int Engine::VAO = 0;
unsigned int Engine::VBO = 0;

void Engine::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL2 failed to init";
		exit(1);
	}
	ShaderLoader::Init();
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,5);

	// disable old smelly open gl stuff
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
	// turn on double buffer for smoother transtions - what ever that means
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	// for epic depth checking precision
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);

	
	 graphicsApplicationWindow = SDL_CreateWindow("babbys first opengl", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
	if(graphicsApplicationWindow == nullptr)
	{
		std::cout << "SDL_Window creation failed";
		exit(1);
	}
	glContext = SDL_GL_CreateContext(graphicsApplicationWindow);
	
	MainLoop();
	
	CleanUp();
	
}

void Engine::Input()
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0)
	{
		if(e.type == SDL_QUIT)
		{
			std::cout << "goodbye" << std::endl;
			quit = true;
		}
		
	}
	
}
void Engine::MainLoop()
{
	shader = ShaderLoader::GetShaderProgram("basic");

	float vertices[] = {
		0.0f,  0.5f, 0.0f,
	   -0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f
	};

	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	
	// Bind and configure VAO and VBO
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	while (!quit)
	{
		Input();
		
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// Update the screen (double buffering)
		SDL_GL_SwapWindow(graphicsApplicationWindow);
	}
}

void Engine::CleanUp() {
	glDeleteProgram(shader);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(graphicsApplicationWindow);
	SDL_Quit();
}