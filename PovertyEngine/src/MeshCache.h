#pragma once
#include <unordered_map>
#include <xstring>
#include <unordered_map>
#include "CachedMesh.h"
#include "systems/RenderSystem.h"
#include "systems/TransformSystem.h"

class MeshCache
{
public:
    static void SetGLVertexAttributes();
    static Mesh GetMesh(const std::string& path);
    static void CleanUp();

private:    
    static std::unordered_map<std::string, CachedMesh> cache;
};
