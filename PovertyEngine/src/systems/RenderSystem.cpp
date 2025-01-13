#include "RenderSystem.h"
#include "TransformSystem.h" 
#include <unordered_map>
#include "Engine.h"

struct RenderQueItem
{
    flecs::entity entity;
    GLuint shaderProgram;
};

std::unordered_map<GLuint, std::vector<RenderQueItem>> renderQueue;

void RenderSystem::SetupSystems(flecs::world& ecs) {
    Engine::camUp   = glm::vec3(0.0f, 1.0f, 0.0f);
    
    ecs.system<Light, Transform>()
       .each([](flecs::entity e, const Light& light, const Transform& transform)
       {
           Engine::lightDir = glm::normalize(glm::vec3(
               cos(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x)),
               sin(glm::radians(transform.rotation.x)),
               sin(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x))
           ));
       });

    ecs.system<Camera, Transform>()
       .each([&](flecs::entity e, const Camera& camera, const Transform& transform)
       {
           Engine::camPos = transform.position;

           glm::vec3 forward = glm::normalize(glm::vec3(
               cos(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x)),
               sin(glm::radians(transform.rotation.x)),
               sin(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x))
           ));
           
           Engine::camLook = Engine::camPos + forward;
       });

    ecs.system<Mesh, Transform, Material, Shader>()
       .kind(flecs::PostUpdate)
       .each([](flecs::entity e, const Mesh& mesh, const Transform& transform, Material& material, Shader& shader)
       {
           renderQueue[shader.program].push_back({e, shader.program});
       });
}



void RenderSystem::Render(flecs::world& ecs)
{
    for (const auto& [shaderProgram, items] : renderQueue)
    {
        // Bind the shader program
        glUseProgram(shaderProgram);

        // Set global uniforms for this shader
        GLint camMatrixLoc = glGetUniformLocation(shaderProgram, "camMatrix");
        glUniformMatrix4fv(camMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::camMatrix));

        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(Engine::lightColor));

        GLint lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(Engine::lightDir));

        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(Engine::camPos));

        // Render each item in this shader program group
        for (const auto& item : items) {
            // Retrieve components dynamically to avoid dangling pointers
            const Transform* transform = item.entity.get<Transform>();
            const Material* material = item.entity.get<Material>();
            const Mesh* mesh = item.entity.get<Mesh>();

            // Ensure components are valid
            if (!transform || !material || !mesh) {
                continue;
            }

            // Set per-entity uniforms
            GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform->model));

            GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform3fv(colorLoc, 1, glm::value_ptr(material->objectColor));

          //  // Bind textures
          //  int textureUnit = 0;
          //  for (auto tex : mesh->textures) {
          //      glActiveTexture(GL_TEXTURE0 + textureUnit);
          //      glBindTexture(GL_TEXTURE_2D, tex.id);
          //      textureUnit++;
          //  }

            // Bind VAO and draw
            glBindVertexArray(mesh->VAO);
            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
    renderQueue.clear();
}
