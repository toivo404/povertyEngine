#pragma once
#include <unordered_map>
#include <xstring>
#include <unordered_map>
#include "CachedMesh.h"
#include "PEPhysics.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

class MeshCache
{
public:
    static void SetGLVertexAttributes();
    static void Load(const std::string& path);
    static Mesh GetMesh(const std::string& path);
    static AABB GetAABB(const std::string& path);
    static AABB CalculateAABB(const std::vector<float>& vertices);
    static void CleanUp();

private:    
    static std::unordered_map<std::string, CachedMesh> cache;
};
