#include "MeshCache.h"

#include <iostream>
#include <optional>
#include <string>
#include "AssimpLoader.h"
#include "PEPhysics.h"
#include "systems/RenderSystem.h"

std::unordered_map<std::string, CachedMesh> MeshCache::cache;


// NOTE: Remember if you need to add or remove vertex attributes you need to adjust the stride in CalculateAABB accordingly
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



void MeshCache::Load(const std::string& path)
{
    AssimpLoader loader;
    std::vector<float> vertices;
    CachedMesh cachedMesh = {};
    
    if (!loader.LoadModel(path, vertices, cachedMesh.indices, cachedMesh.nodes)) {
        std::cerr << "Failed to load model from: " << path << std::endl;
        exit(1);
    }

    MakeCalculations(vertices, cachedMesh);
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cachedMesh.indices.size() * sizeof(unsigned int), cachedMesh.indices.data(), GL_STATIC_DRAW);

    SetGLVertexAttributes();

    cachedMesh.mesh = Mesh{VAO, VBO, EBO, static_cast<GLsizei>(cachedMesh.indices.size())};
    cache[path] = cachedMesh;
}

CachedMesh* MeshCache::GetMesh(const std::string& path)
{
    auto it = cache.find(path);
    if (it == cache.end())
    {
        Load(path);
        return &cache[path];
    }
    return &it->second;
}

AABB MeshCache::GetAABB(const std::string& path)
{
    auto it = cache.find(path);
    if (it == cache.end())
    {
        Load(path);
        return cache[path].aabb;
    }
    return it->second.aabb;
}

void MeshCache::MakeCalculations(const std::vector<float>& vertices, CachedMesh& cachedMesh) {
    if (vertices.empty()) {
        throw std::invalid_argument("Vertex data is empty!");
    }

    // Initialize min/max with extreme values


    // Extract position data from interleaved vertex data
    size_t stride = 8; // 8 floats per vertex
    cachedMesh.aabb.min = glm::vec3(vertices[0],vertices[1],vertices[2]);
    cachedMesh.aabb.max = glm::vec3(vertices[0],vertices[1],vertices[2]);
    
    for (size_t i = stride; i < vertices.size(); i += stride) {
        glm::vec3 position(vertices[i], vertices[i + 1], vertices[i + 2]);
        cachedMesh.vertices.push_back(position);
        cachedMesh.aabb.min = position;
        // Update AABB bounds
        cachedMesh.aabb.Encapsulate(position);
    }

    
    
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
