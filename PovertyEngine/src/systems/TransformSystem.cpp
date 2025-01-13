#include "TransformSystem.h"

void TransformSystem::RegisterSystem(flecs::world& ecs)
{
    ecs.system<Transform>()
       .kind(flecs::PreUpdate) // Run this system before the main update loop
       .each([](flecs::entity e, Transform& transform)
       {
           // Update the model matrix
           glm::mat4 model = glm::mat4(1.0f);
           model = glm::translate(model, transform.position);
           model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
           model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
           model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
           model = glm::scale(model, transform.scale);
           transform.model = model;
       });
}
