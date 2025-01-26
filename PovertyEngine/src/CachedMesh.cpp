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
