#pragma once
#include <string>
#include <unordered_map>
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

class CachedMesh
{

public:
    Mesh mesh;
    std::unordered_map<std::string, Transform> nodes;
    

    Mesh Reuse();
    Transform GetChildTransform(std::string trans);
};
