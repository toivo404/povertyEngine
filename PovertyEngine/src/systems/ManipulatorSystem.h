#pragma once
#include "flecs.h"

struct Manipulator {
    float moveSpeed   = 2.0f;  
    float rotateSpeed = 50.0f;
};

class ManipulatorSystem
{
public:
    static void RegisterSystem(flecs::world &ecs);
};
