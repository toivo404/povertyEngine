#include "RenderSystem.h"
#include "TransformSystem.h" 
#include <unordered_map>
#include "Engine.h"
#include "MaterialCache.h"

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
    secs::Entity e;
    glm::mat4 model = glm::mat4(1.0f);
    int meshIndexCount;
};

std::unordered_map<RenderBatchKey, std::vector<RenderQueItem>> renderQueue;


void RenderSystem::RegisterComponents(secs::World* world, secs::ComponentRegistry* compReg)
{
    compReg->registerType<Material>("Material");
    compReg->registerType<Shader>("Shader");
    compReg->registerType<Light>("Light");
    compReg->registerType<Camera>("Camera");
    compReg->registerType<Mesh>("Mesh");

}

void RenderSystem::Render()
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



void RenderSystem::CreateSystems(secs::ComponentRegistry* compReg, std::vector<secs::System>* systems)
{
    int meshTypeId = compReg->getID<Mesh>();
    int transformTypeId = compReg->getID<Transform>();
    int materialTypeId = compReg->getID<Material>();
    int shaderTypeId = compReg->getID<Shader>();
    int lightTypeId = compReg->getID<Light>();
    int cameraTypeId = compReg->getID<Camera>();

    auto renderSystem = secs::System(
        {meshTypeId, transformTypeId, materialTypeId, shaderTypeId},
        [meshTypeId, transformTypeId, materialTypeId, shaderTypeId](secs::Entity e, secs::World& w)
        {
            const auto& mesh = w.getComponent<Mesh>(e, meshTypeId);
            const auto& transform = w.getComponent<Transform>(e, transformTypeId);
            const auto& material = w.getComponent<Material>(e, materialTypeId);
            const auto& shader = w.getComponent<Shader>(e, shaderTypeId);

            RenderBatchKey key;
            key.shaderProgram = shader.program;
            key.VAO = mesh.VAO;
            key.materialId = material.materialId;

            renderQueue[key].push_back(RenderQueItem{e, transform.model, mesh.indexCount});
        });
    systems->push_back(renderSystem);

    auto lightSystem = secs::System(
        {lightTypeId, transformTypeId},
        [lightTypeId, transformTypeId](secs::Entity e, secs::World& w)
        {
            const auto& light = w.getComponent<Light>(e, lightTypeId);
            const auto& transform = w.getComponent<Transform>(e, transformTypeId);

            Engine::lightDir = glm::normalize(glm::vec3(
                cos(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x)),
                sin(glm::radians(transform.rotation.x)),
                sin(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x))
            ));
        });
    systems->push_back(lightSystem);


    auto camSystem =
        secs::System(
            {cameraTypeId, transformTypeId},
            [cameraTypeId, transformTypeId](secs::Entity e, secs::World& w)
            {
                const auto& transform = w.getComponent<Transform>(e, transformTypeId);
                Engine::camPos = transform.position;

                glm::vec3 forward = glm::normalize(glm::vec3(
                    cos(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x)),
                    sin(glm::radians(transform.rotation.x)),
                    sin(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x))
                ));

                Engine::camLook = Engine::camPos + forward;
            });
    systems->push_back(camSystem);
}



