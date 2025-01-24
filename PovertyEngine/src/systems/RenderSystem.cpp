#include "RenderSystem.h"
#include "TransformSystem.h" 
#include <unordered_map>
#include "Engine.h"
#include "MaterialCache.h"

void RenderSystem::RegisterComponents(secs::World* world)
{
    secs::ComponentRegistry::registerType<Transform>("Transform");
    secs::ComponentRegistry::registerType<Material>("Material");
    secs::ComponentRegistry::registerType<Shader>("Shader");
    secs::ComponentRegistry::registerType<Light>("Light");
    secs::ComponentRegistry::registerType<Camera>("Camera");
    secs::ComponentRegistry::registerType<Mesh>("Mesh");
}


int RenderSystem::Render(secs::World& world)
{
    int renderedObjects = 0;

    // Use queryChunks to efficiently process entities with the required components
    secs::queryChunks<Transform, Mesh, Material, Shader>(
        world,
        [&](const std::vector<secs::Entity>& ents,
            Transform* transforms,
            Mesh* meshes,
            Material* materials,
            Shader* shaders,
            size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                // Update the transform's model matrix
                transforms[i].UpdateModelMatrix();

                // Use the shader program
                glUseProgram(shaders[i].program);

                // Set global uniforms
                GLint camMatrixLoc = glGetUniformLocation(shaders[i].program, "camMatrix");
                glUniformMatrix4fv(camMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::camMatrix));

                GLint lightColorLoc = glGetUniformLocation(shaders[i].program, "lightColor");
                glUniform3fv(lightColorLoc, 1, glm::value_ptr(Engine::lightColor));

                GLint lightDirLoc = glGetUniformLocation(shaders[i].program, "lightDir");
                glUniform3fv(lightDirLoc, 1, glm::value_ptr(Engine::lightDir));

                GLint viewPosLoc = glGetUniformLocation(shaders[i].program, "viewPos");
                glUniform3fv(viewPosLoc, 1, glm::value_ptr(Engine::camPos));

                // Bind the VAO
                glBindVertexArray(meshes[i].VAO);

                // Set the model matrix for the current entity
                GLint modelLoc = glGetUniformLocation(shaders[i].program, "model");
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transforms[i].model));

                // Draw the mesh
                glDrawElements(GL_TRIANGLES, meshes[i].indexCount, GL_UNSIGNED_INT, nullptr);
                renderedObjects++;

                // Unbind the VAO
                glBindVertexArray(0);
            }
        });

    return renderedObjects;
}



void RenderSystem::CreateSystems( std::vector<secs::System>* systems)
{
    int lightTypeId = secs::ComponentRegistry::registerType<Light>("Light");
    int transformTypeId = secs::ComponentRegistry::registerType<Transform>("Transform");

    const auto lightSystem = secs::System(
        {lightTypeId, transformTypeId},
        [](secs::Entity e, secs::World& w)
        {
            const auto* transform = w.getComponent<Transform>(e);
            if (transform)
            {
                Engine::lightDir = transform->GetDirection();
            }
        });
    
    systems->push_back(lightSystem);
    int camTypeId = secs::ComponentRegistry::registerType<Camera>("Camera");
    
    const auto camSystem = secs::System(
        {camTypeId, transformTypeId},
        [](secs::Entity e, secs::World& w)
        {
            const auto* transform = w.getComponent<Transform>(e);
            Engine::camPos = transform->position;
            Engine::camLook = transform->GetDirection();
        });
    systems->push_back(camSystem);
}



