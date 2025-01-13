#pragma once

#include <glm.hpp>
#include <flecs.h>
#include <gtc/matrix_transform.hpp>

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);  
    glm::vec3 scale = glm::vec3(1.0f);     
    glm::vec3 rotation = glm::vec3(0.0f);  
    glm::mat4 model = glm::mat4(1.0f);     
};

class TransformSystem
{
public:
    static void RegisterSystem(flecs::world& ecs);
};
