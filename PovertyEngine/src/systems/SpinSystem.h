#pragma once
#include <flecs.h>

struct Spin {
    float rotateSpeed = 50.0f; // Rotation speed in degrees per second
};

class SpinSystem
{
public:
    static void RegisterSystem(flecs::world& ecs);
};
