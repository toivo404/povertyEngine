#include "EngineInfo.h"
#include <glad.h>
#include <iostream>
#include <SDL_version.h>

void EngineInfo::LogInfo()
{
    // Log OpenGL version
    const GLubyte* renderer = glGetString(GL_RENDERER); // Renderer name
    const GLubyte* version = glGetString(GL_VERSION);   // OpenGL version string
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); // GLSL version

    std::cout << "OpenGL Renderer: " << renderer << std::endl;
    std::cout << "OpenGL Version: " << version << std::endl;
    std::cout << "GLSL Version: " << glslVersion << std::endl;

    // Log SDL version
    SDL_version compiled;
    SDL_version linked;
    SDL_VERSION(&compiled);               // Get the compiled version of SDL
    SDL_GetVersion(&linked);              // Get the version of the dynamically linked SDL library

    std::cout << "SDL Version (compiled): " << static_cast<int>(compiled.major) << "."
              << static_cast<int>(compiled.minor) << "."
              << static_cast<int>(compiled.patch) << std::endl;

    std::cout << "SDL Version (linked): " << static_cast<int>(linked.major) << "."
              << static_cast<int>(linked.minor) << "."
              << static_cast<int>(linked.patch) << std::endl;
}
