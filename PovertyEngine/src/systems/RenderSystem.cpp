#include "RenderSystem.h"
#include "TransformSystem.h" 
#include <unordered_map>
#include "Engine.h"
#include "MaterialSystem.h"

struct RenderBatchKey {
    GLuint shaderProgram;     // Shader program ID
    GLuint VAO;               // Vertex Array Object
    int materialId;               // Vertex Array Object

    bool operator==(const RenderBatchKey& other) const {
        return shaderProgram == other.shaderProgram &&
               VAO == other.VAO &&
               materialId == other.materialId;
    }
};

// Custom hash function for RenderBatchKey
namespace std {
    template <>
    struct hash<RenderBatchKey> {
        std::size_t operator()(const RenderBatchKey& key) const {
            std::size_t hash = std::hash<GLuint>()(key.shaderProgram);
            hash ^= std::hash<GLuint>()(key.VAO) << 1;
            hash ^= std::hash<GLuint>()(key.materialId) << 1;
            return hash;
        }
    };
}

struct RenderQueItem
{
    flecs::entity e;
    glm::mat4 model = glm::mat4(1.0f);
    int meshIndexCount;
};

std::unordered_map<RenderBatchKey, std::vector<RenderQueItem>> renderQueue;

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
             // Create a render batch key
           RenderBatchKey key;
           key.shaderProgram = shader.program;
           key.VAO = mesh.VAO;
           key.materialId = material.materialId;
           
           // Insert the entity into the render queue
           renderQueue[key].push_back({e, transform.model, mesh.indexCount});
     });
}


void RenderSystem::Render(flecs::world& ecs)
{
    for (const auto& [key, items] : renderQueue)
    {
        // Bind the shader program
        glUseProgram(key.shaderProgram);

        // Set global uniforms for this shader
        GLint camMatrixLoc = glGetUniformLocation(key.shaderProgram, "camMatrix");
        glUniformMatrix4fv(camMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::camMatrix));

        GLint lightColorLoc = glGetUniformLocation(key.shaderProgram, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(Engine::lightColor));

        GLint lightDirLoc = glGetUniformLocation(key.shaderProgram, "lightDir");
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(Engine::lightDir));

        GLint viewPosLoc = glGetUniformLocation(key.shaderProgram, "viewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(Engine::camPos));

        // Bind the VAO
        glBindVertexArray(key.VAO);
        
        // Render each entity in this batch
        for (const auto& item : items) {

            // Set the model matrix for the current entity
            GLint modelLoc = glGetUniformLocation(key.shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(item.model));

            // Draw the mesh
            glDrawElements(GL_TRIANGLES, item.meshIndexCount, GL_UNSIGNED_INT, nullptr);
        }

        // Unbind the VAO
        glBindVertexArray(0);
    }

    // Clear the render queue after rendering
    renderQueue.clear();
}