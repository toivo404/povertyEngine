#include "MeshCache.h"

#include <iostream>
#include <string>
#include "AssimpLoader.h"
#include "systems/RenderSystem.h"

std::unordered_map<std::string, CachedMesh> MeshCache::cache;

void MeshCache::SetGLVertexAttributes()
{
    // Enable and define position attribute (index 0, 3 floats, starts at 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // Enable and define normal attribute (index 1, 3 floats, starts after position)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Enable and define texture coordinate attribute (index 2, 2 floats, starts after normals)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // Unbind VAO
    glBindVertexArray(0);
}

Mesh MeshCache::GetMesh(const std::string& path)
{
    auto it = cache.find(path);
    if (it == cache.end())
    {
        AssimpLoader loader;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        CachedMesh cachedMesh = {};

        if (!loader.LoadModel(path, vertices, indices, cachedMesh.nodes)) {
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

        SetGLVertexAttributes();

        cachedMesh.mesh = Mesh{VAO, VBO, EBO, static_cast<GLsizei>(indices.size())};
        cache[path] = cachedMesh;
        return cachedMesh.mesh;
    }
    return it->second.mesh;
}


void MeshCache::CleanUp()
{
    for (auto& [path, cachedMesh] : cache)
    {
        glDeleteVertexArrays(1, &cachedMesh.mesh.VAO);
        glDeleteBuffers(1, &cachedMesh.mesh.VBO);
        glDeleteBuffers(1, &cachedMesh.mesh.EBO);
    }
    cache.clear();
}
