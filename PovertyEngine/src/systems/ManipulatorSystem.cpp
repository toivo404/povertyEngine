#include "ManipulatorSystem.h"
#include "Engine.h"
#include "TransformSystem.h"

void ManipulatorSystem::RegisterSystem(flecs::world &ecs)
{
    ecs.system<Manipulator, Transform>()
       .each([](flecs::entity e, Manipulator& manipulator, Transform& transform) {
           // Basic WASD
           if (Engine::IsKeyPressed(SDLK_w)) { transform.position.z -= manipulator.moveSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_s)) { transform.position.z += manipulator.moveSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_a)) { transform.position.x -= manipulator.moveSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_d)) { transform.position.x += manipulator.moveSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_z)) { transform.position.y -= manipulator.moveSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_x)) { transform.position.y += manipulator.moveSpeed * Engine::deltaTime; }

           // Rotation with arrow keys
           if (Engine::IsKeyPressed(SDLK_UP))    { transform.rotation.x -= manipulator.rotateSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_DOWN))  { transform.rotation.x += manipulator.rotateSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_LEFT))  { transform.rotation.y -= manipulator.rotateSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_RIGHT)) { transform.rotation.y += manipulator.rotateSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_q))     { transform.rotation.z -= manipulator.rotateSpeed * Engine::deltaTime; }
           if (Engine::IsKeyPressed(SDLK_e))     { transform.rotation.z += manipulator.rotateSpeed * Engine::deltaTime; }
       });
    
}

