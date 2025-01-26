#pragma once
#include <string>
#include <unordered_map>

#include "PEPhysics.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

class PE_API CachedMesh
{

public:
    Mesh mesh;
    std::unordered_map<std::string, Transform> nodes;
    AABB aabb;
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    Mesh Reuse();
    Transform GetChildTransform(std::string trans);


    void CopyVerticesTo(glm::vec3* buffer, size_t bufferSize) const;
};
