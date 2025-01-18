#pragma once
#include <flecs.h>
#include <core.h>
struct PE_API Spin {
    float rotateSpeed = 50.0f; // Rotation speed in degrees per second
};

class SpinSystem
{
public:
    static void RegisterSystem(flecs::world& ecs);
};
