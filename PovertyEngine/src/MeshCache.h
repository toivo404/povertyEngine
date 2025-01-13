#pragma once
#include <unordered_map>
#include <xstring>
#include <unordered_map>
#include "systems/RenderSystem.h"

class MeshCache
{
public:
    static Mesh GetMesh(const std::string& path);
    static void CleanUp();

private:    
    static std::unordered_map<std::string, Mesh> cache;
};
