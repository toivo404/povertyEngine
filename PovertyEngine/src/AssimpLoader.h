#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "systems/TransformSystem.h" // Include your Transform definition

class AssimpLoader {
public:
    bool LoadModel(const std::string& filePath, 
                   std::vector<float>& vertices, 
                   std::vector<unsigned int>& indices, 
                   std::unordered_map<std::string, Transform>& nodeTransforms);
    
private:
    void ProcessNode(aiNode* node, 
                     const aiScene* scene, 
                     std::vector<float>& vertices, 
                     std::vector<unsigned int>& indices, 
                     std::unordered_map<std::string, Transform>& nodeTransforms);
    
    void ProcessMesh(aiMesh* mesh, 
                     const aiScene* scene, 
                     std::vector<float>& vertices, 
                     std::vector<unsigned int>& indices);

    Transform ExtractTransform(const aiMatrix4x4& transform);
};
