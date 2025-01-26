#include "CachedMesh.h"

Mesh CachedMesh::Reuse()
{
    /* Not necessary it seems
    GLuint newVAO;
    glGenVertexArrays(1, &newVAO);
    glBindVertexArray(newVAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    */
    return mesh;
}

Transform CachedMesh::GetChildTransform(std::string name)
{
    return nodes[name];
}

void CachedMesh::CopyVerticesTo(glm::vec3* buffer, size_t bufferSize) const
{
    if (vertices.size() > bufferSize)
    {
        throw std::runtime_error("Provided buffer is too small to hold all vertices.");
    }

    std::memcpy(buffer, vertices.data(), vertices.size() * sizeof(glm::vec3));
}
