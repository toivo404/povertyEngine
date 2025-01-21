#include "RenderSystem.h"
#include "TransformSystem.h" 
#include <unordered_map>
#include "Engine.h"
#include "MaterialCache.h"

void RenderSystem::RegisterComponents(secs::World* world, secs::ComponentRegistry* compReg)
{
    compReg->registerType<Transform>("Transform");
    compReg->registerType<Material>("Material");
    compReg->registerType<Shader>("Shader");
    compReg->registerType<Light>("Light");
    compReg->registerType<Camera>("Camera");
    compReg->registerType<Mesh>("Mesh");
}


int RenderSystem::Render(secs::World& world, secs::ComponentRegistry& compReg)
{
    int renderedObjects = 0;
    // Get component type IDs
     // Get component type IDs
    int meshTypeId = compReg.getID<Mesh>();
    int transformTypeId = compReg.getID<Transform>();
    int materialTypeId = compReg.getID<Material>();
    int shaderTypeId = compReg.getID<Shader>();

    // Iterate over all entities
    for (secs::Entity e : world.getAllEntities())
    {
        // Check if the entity has the required components
        if (world.hasComponent(e, meshTypeId) &&
            world.hasComponent(e, transformTypeId) &&
            world.hasComponent(e, materialTypeId) &&
            world.hasComponent(e, shaderTypeId))
        {
            // Get the components
            auto& transform = world.getComponent<Transform>(e, transformTypeId);
            const auto& mesh = world.getComponent<Mesh>(e, meshTypeId);
            const auto& material = world.getComponent<Material>(e, materialTypeId);
            const auto& shader = world.getComponent<Shader>(e, shaderTypeId);
            
            transform.UpdateModelMatrix();

            // Use the shader program
            glUseProgram(shader.program);

            // Set global uniforms
            GLint camMatrixLoc = glGetUniformLocation(shader.program, "camMatrix");
            glUniformMatrix4fv(camMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::camMatrix));

            GLint lightColorLoc = glGetUniformLocation(shader.program, "lightColor");
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(Engine::lightColor));

            GLint lightDirLoc = glGetUniformLocation(shader.program, "lightDir");
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(Engine::lightDir));

            GLint viewPosLoc = glGetUniformLocation(shader.program, "viewPos");
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(Engine::camPos));

            // Bind the VAO
            glBindVertexArray(mesh.VAO);

            // Set the model matrix for the current entity
            GLint modelLoc = glGetUniformLocation(shader.program, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform.model));

            // Draw the mesh
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
            renderedObjects++;
            // Unbind the VAO
            glBindVertexArray(0);
        }
    }
    return renderedObjects;
}



void RenderSystem::CreateSystems(secs::ComponentRegistry* compReg, std::vector<secs::System>* systems)
{
    int transformTypeId = compReg->getID<Transform>();
    int lightTypeId = compReg->getID<Light>();
    int cameraTypeId = compReg->getID<Camera>();

    auto lightSystem = secs::System(
        {lightTypeId, transformTypeId},
        [lightTypeId, transformTypeId](secs::Entity e, secs::World& w)
        {
            const auto& light = w.getComponent<Light>(e, lightTypeId);
            const auto& transform = w.getComponent<Transform>(e, transformTypeId);
            Engine::lightDir = transform.GetDirection();
        });
    systems->push_back(lightSystem);


    auto camSystem =
        secs::System(
            {cameraTypeId, transformTypeId},
            [cameraTypeId, transformTypeId](secs::Entity e, secs::World& w)
            {
       //         const auto& transform = w.getComponent<Transform>(e, transformTypeId);
       //         Engine::camPos = transform.position;
       //         Engine::camLook = Engine::camPos + transform.GetDirection();
            });
    systems->push_back(camSystem);
}



