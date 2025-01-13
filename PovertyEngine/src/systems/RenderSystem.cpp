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


uint64_t group_by_shader(flecs::world_t* world, flecs::table_t* table, flecs::id_t id, void* ctx)
{
    // The context provides the ID of the Shader component
    flecs::entity_t shader_id = reinterpret_cast<flecs::entity_t>(ctx);

    // Use Flecs's API to check if the table has the Shader component
    const Shader* shader = static_cast<const Shader*>(table->column(shader_id));
    if (shader)
    {
        return shader->program; // Return the shader program ID as the group ID
    }

    return 0; // Default group if Shader is not present
}


void RenderSystem::Render(flecs::world& ecs)
{
    
    
     auto query = ecs.query_builder<Mesh, Transform, Shader, Material>()
        .group_by<Shader>(
            // group_by callback
            [&](flecs::world_t* world, flecs::table_t *table, flecs::id_t id, void* ctx) -> uint64_t 
            {
                // Wrap the raw C pointers in C++ Flecs types for convenience
                flecs::world w(world);
                flecs::table* t(table);

                // We want the column that has the Shader component:
                if ( t->has<Shader>()) {
                    // Return the GLuint program as the group_id
                    return static_cast<uint64_t>(shader->program);
                }
                // If there's no Shader, default to 0
                return 0ULL;
            }
        )
        .build();

    // Now iterate each group (i.e., each unique Shader::program)
    query.each([&](flecs::id_t group_id, flecs::iter& it) {
        // group_id corresponds to Shader::program
        GLuint shaderProgram = static_cast<GLuint>(group_id);
        glUseProgram(shaderProgram);

        // Set uniforms common to this shader:
        GLint camMatrixLoc = glGetUniformLocation(shaderProgram, "camMatrix");
        glUniformMatrix4fv(camMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::camMatrix));

        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(Engine::lightColor));

        GLint lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(Engine::lightDir));

        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(Engine::camPos));

        // Iterate all entities in this group
        for (auto i : it) {
            // Retrieve each component for this entity
            const Mesh& mesh       = it.get<Mesh>(i);
            const Transform& trans = it.get<Transform>(i);
            const Material& mat    = it.get<Material>(i);

            // Per-entity uniforms
            GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(trans.model));

            GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform3fv(colorLoc, 1, glm::value_ptr(mat.objectColor));

            // Bind each texture and set uniforms for them
            int textureUnit = 0;
            for (auto &texInfo : mesh.textures) {
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, texInfo.id);

                if (texInfo.type == "diffuse") {
                    glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), textureUnit);
                } else if (texInfo.type == "specular") {
                    glUniform1i(glGetUniformLocation(shaderProgram, "specularTexture"), textureUnit);
                }
                textureUnit++;
            }

            // Bind and draw
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    });
}
