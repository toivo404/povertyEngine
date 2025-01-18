#pragma once
#include "flecs.h"
#include "core.h"

struct PE_API Manipulator {
    float moveSpeed   = 2.0f;  
    float rotateSpeed = 50.0f;
};

class ManipulatorSystem
{
public:
    static void RegisterSystem(flecs::world &ecs);
};
