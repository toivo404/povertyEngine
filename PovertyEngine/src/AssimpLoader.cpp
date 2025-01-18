#include "AssimpLoader.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "systems/TransformSystem.h" // Include your Transform definition
#include "AssimpLoader.h"
#include <gtc/quaternion.hpp>

bool AssimpLoader::LoadModel(const std::string& filePath, 
                             std::vector<float>& vertices, 
                             std::vector<unsigned int>& indices, 
                             std::unordered_map<std::string, Transform>& nodeTransforms) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    ProcessNode(scene->mRootNode, scene, vertices, indices, nodeTransforms);
    return true;
}

void AssimpLoader::ProcessNode(aiNode* node, 
                               const aiScene* scene, 
                               std::vector<float>& vertices, 
                               std::vector<unsigned int>& indices, 
                               std::unordered_map<std::string, Transform>& nodeTransforms) {
    // Extract and store the node's local transform
    Transform localTransform = ExtractTransform(node->mTransformation);
    nodeTransforms[node->mName.C_Str()] = localTransform;

    // Process all meshes attached to this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene, vertices, indices);
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, vertices, indices, nodeTransforms);
    }
}

void AssimpLoader::ProcessMesh(aiMesh* mesh, 
                               const aiScene* scene, 
                               std::vector<float>& vertices, 
                               std::vector<unsigned int>& indices) {
    unsigned int vertexStartIndex = vertices.size() / 8;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        aiVector3D position = mesh->mVertices[i];
        aiVector3D normal = mesh->mNormals[i];
        aiVector3D texCoords = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);

        vertices.push_back(position.x);
        vertices.push_back(position.y);
        vertices.push_back(position.z);

        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);

        vertices.push_back(texCoords.x);
        vertices.push_back(texCoords.y);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(vertexStartIndex + face.mIndices[j]);
        }
    }
}

Transform AssimpLoader::ExtractTransform(const aiMatrix4x4& transform) {
    aiVector3D position, scale;
    aiQuaternion rotation;
    transform.Decompose(scale, rotation, position);

    Transform t;
    t.position = glm::vec3(position.x, position.y, position.z);
    t.scale = glm::vec3(scale.x, scale.y, scale.z);

    // Convert Assimp quaternion to Euler angles (pitch, yaw, roll)
    glm::quat glmRotation(rotation.w, rotation.x, rotation.y, rotation.z);
    t.rotation = glm::eulerAngles(glmRotation);

    // Create the model matrix
    t.model = glm::translate(glm::mat4(1.0f), t.position) *
              glm::mat4_cast(glmRotation) *
              glm::scale(glm::mat4(1.0f), t.scale);

    return t;
}
