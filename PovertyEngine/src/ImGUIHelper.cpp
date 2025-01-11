#include "ImGUIHelper.h"

#include <iostream>
#include <SDL_video.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"


bool ImGUIHelper::debugUIIsOn;
bool debugUIIsOnInternal;

void ImGUIHelper::Init(SDL_Window* window, SDL_GLContext glContext)
{
    debugUIIsOn = true;
    debugUIIsOnInternal = debugUIIsOn;
    
    // Initialize Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Initialize Dear ImGui SDL2 and OpenGL ES 2.0 backends
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    //ImGui_ImplOpenGL3_Init(GL_ES_VERSION); // Pass OpenGL ES version
    ImGui_ImplOpenGL3_Init();
    std::cout << ImGui::ErrorLog << std::endl;
}

void ImGUIHelper::CleanUp()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ImGUIHelper::OnSDLEvent(SDL_Event e)
{
    if(e.type == SDL_KEYDOWN) {
        SDL_Keycode key = e.key.keysym.sym;
        if(key == SDLK_F12)ImGUIHelper::debugUIIsOn = !ImGUIHelper::debugUIIsOn;        
    }    

    
    if(!debugUIIsOnInternal)return;
    
    ImGui_ImplSDL2_ProcessEvent(&e);
}

void ImGUIHelper::OnFrameStart()
{
    if(!debugUIIsOnInternal)return;
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGUIHelper::OnFrameEnd()
{
    if(debugUIIsOnInternal) {
        // Render Dear ImGui content
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());        
    }
    debugUIIsOnInternal = debugUIIsOn;
}
