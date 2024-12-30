#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

class AssimpLoader {
public:
    bool LoadModel(const std::string& filePath) {
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
        ProcessNode(scene->mRootNode, scene);
        return true;
    }

private:
    void ProcessNode(aiNode* node, const aiScene* scene) {
        // Process all the node's meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh, scene);
        }

        // Recursively process child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    void ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        // Extract vertex data, normals, textures, etc.
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // Position
            aiVector3D position = mesh->mVertices[i];
            std::cout << "Vertex Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        }

        // Extract indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                std::cout << "Index: " << face.mIndices[j] << std::endl;
            }
        }
    }
};
