#pragma once
#include <SDL_events.h>
#include <SDL_video.h>

class ImGUIHelper
{
public:
    static bool debugUIIsOn;
    
    void Init(SDL_Window* window, SDL_GLContext glContext);
    void CleanUp();
    void OnSDLEvent(SDL_Event e);
    void OnFrameStart();
    void OnFrameEnd();
};
