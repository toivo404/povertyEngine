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

                // 2. Get the uniform location for your sampler2D
                GLuint texLoc = glGetUniformLocation(shaders[i].program, "diffuseTexture");

                // 3. Activate texture unit 0, then bind your actual texture ID to that unit
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, materials[i].diffuseTextureID);
                // 4. Tell the sampler in the shader that "diffuseTexture" corresponds to texture unit 0
                glUniform1i(texLoc, 0);

                
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




