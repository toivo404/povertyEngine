#include "TransformSystem.h"
#include "Secs.h"


void TransformSystem::RegisterComponents(secs::World* world, secs::ComponentRegistry* compReg)
{
    compReg->registerType<Transform>("Transform");
}

secs::System TransformSystem::CreateSystem(secs::ComponentRegistry* compReg)
{
    int transTypeId = compReg->getID<Transform>();
    secs::System transformSystem ({ transTypeId}, [transTypeId](secs::Entity e, secs::World w)
    {
        auto& transform = w.getComponent<Transform>(e, transTypeId);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, transform.position);
        model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, transform.scale);
        transform.model = model;
    });
    return transformSystem;
}
