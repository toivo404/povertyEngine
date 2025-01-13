#include "MeshCache.h"
#include <string>
#include "AssimpLoader.h"
#include "systems/RenderSystem.h"

std::unordered_map<std::string, Mesh> MeshCache::cache;

Mesh MeshCache::GetMesh(const std::string& path)
{
    auto it = cache.find(path);
    if (it != cache.end())
    {
        Mesh cachedMesh = it->second;

        GLuint newVAO;
        glGenVertexArrays(1, &newVAO);
        glBindVertexArray(newVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cachedMesh.VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cachedMesh.EBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);

        return {newVAO, cachedMesh.VBO, cachedMesh.EBO, cachedMesh.indexCount, true};
    }

    AssimpLoader loader;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    if (!loader.LoadModel(path, vertices, indices))
    {
        std::cerr << "Failed to load model from: " << path << std::endl;
        exit(1);
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    Mesh mesh = {VAO, VBO, EBO, static_cast<GLsizei>(indices.size()), false};
    cache[path] = mesh;

    return mesh;
}

void MeshCache::CleanUp()
{
    for (auto& [path, mesh] : cache)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        if (!mesh.shared)
        {
            glDeleteBuffers(1, &mesh.VBO);
            glDeleteBuffers(1, &mesh.EBO);
        }
    }
    cache.clear();
}
