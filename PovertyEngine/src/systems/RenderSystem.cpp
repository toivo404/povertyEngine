#include "RenderSystem.h"
#include "TransformSystem.h" 
#include <iostream>
#include "Engine.h"


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

}

void RenderSystem::Render(flecs::world& ecs)
{
    ecs.query<Mesh, Transform, Material>()
    .each([](flecs::entity e, const Mesh& mesh, const Transform& transform, const Material& material)
    {
        glUseProgram(material.shaderProgram);

        GLint camMatrixLoc = glGetUniformLocation(material.shaderProgram, "camMatrix");
        glUniformMatrix4fv(camMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::camMatrix));

        GLint modelLoc = glGetUniformLocation(material.shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform.model));

        GLint lightColorLoc = glGetUniformLocation(material.shaderProgram, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(Engine::lightColor));

        GLint lightDirLoc = glGetUniformLocation(material.shaderProgram, "lightDir");
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(Engine::lightDir));

        GLint viewPosLoc = glGetUniformLocation(material.shaderProgram, "viewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(Engine::camPos));

        GLint colorLoc = glGetUniformLocation(material.shaderProgram, "objectColor");
        glUniform3fv(colorLoc, 1, glm::value_ptr(material.objectColor));

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        std::cout << "Rendered entity: " << e.name() << std::endl;
    });
}
