#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <vector>

class AssimpLoader {
public:
    // Updated function signature to populate vertices and indices
    bool LoadModel(const std::string& filePath, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
        Assimp::Importer importer;

        // Load the model file
        const aiScene* scene = importer.ReadFile(filePath,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        // Process the model
        ProcessNode(scene->mRootNode, scene, vertices, indices);
        return true;
    }

private:
    void ProcessNode(aiNode* node, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
        // Process all the node's meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh, scene, vertices, indices);
        }

        // Recursively process child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene, vertices, indices);
        }
    }

    void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
        unsigned int vertexStartIndex = vertices.size() / 8; // Each vertex has 8 floats: 3 for position, 3 for normal, 2 for texture coords

        // Extract vertex data (position, normal, texture coordinates)
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D position = mesh->mVertices[i];
            aiVector3D normal = mesh->mNormals[i];
            aiVector3D texCoords = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);

            // Push position
            vertices.push_back(position.x);
            vertices.push_back(position.y);
            vertices.push_back(position.z);

            // Push normal
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Push texture coordinates
            vertices.push_back(texCoords.x);
            vertices.push_back(texCoords.y);
        }

        // Extract indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(vertexStartIndex + face.mIndices[j]);
            }
        }
    }
};
