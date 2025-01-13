#include "SpinSystem.h"
#include "TransformSystem.h" 
#include <glm.hpp>
#include "Engine.h"


void SpinSystem::RegisterSystem(flecs::world& ecs) {
    ecs.system<Spin, Transform>()
       .each([](flecs::entity e, Spin& s, Transform& t)
       {
           t.rotation.y += s.rotateSpeed * Engine::deltaTime;
           t.position.y = t.position.y + Engine::deltaTime;
           t.rotation.y = glm::mod(t.rotation.y, 360.0f);
       });
}
