#pragma once
#include <string>
#include <unordered_map>

#include "PEPhysics.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

class CachedMesh
{

public:
    Mesh mesh;
    std::unordered_map<std::string, Transform> nodes;
    AABB aabb;

    Mesh Reuse();
    Transform GetChildTransform(std::string trans);
};
