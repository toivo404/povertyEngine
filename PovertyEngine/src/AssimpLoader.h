#pragma once
#include <iostream>
#include <ostream>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>      // For Assimp::Importer
#include <assimp/postprocess.h>     // For post-processing flags (e.g., aiProcess_Triangulate)
#include "systems/RenderSystem.h"

class AssimpLoader {
public:
    bool LoadModel(const std::string& filePath, 
               std::vector<float>& vertices, 
               std::vector<unsigned int>& indices, 
               std::vector<Texture>& textures) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(filePath,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        // Process the root node recursively
        ProcessNode(scene->mRootNode, scene, vertices, indices, textures);

        return true;
    }


private:
    void ProcessNode(aiNode* node, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh, scene, vertices, indices, textures);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene, vertices, indices, textures);
        }
    }

    void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures) {
        unsigned int vertexStartIndex = vertices.size() / 8;

        // Extract vertex data
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

        // Extract indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(vertexStartIndex + face.mIndices[j]);
            }
        }

        // Load textures from materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        LoadMaterialTextures(scene, material, aiTextureType_DIFFUSE, "diffuse", textures);
        LoadMaterialTextures(scene, material, aiTextureType_SPECULAR, "specular", textures);
    }


    void LoadMaterialTextures(const aiScene* scene, aiMaterial* mat, aiTextureType type, const std::string& typeName, std::vector<Texture>& textures) {
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString path;
            aiTextureMapping mapping;
            unsigned int uvIndex;
            ai_real blendFactor;
            aiTextureOp textureOp;
            aiTextureMapMode mapMode;
            // Get the texture data
            if (mat->GetTexture(type, i, &path, &mapping, &uvIndex, &blendFactor, &textureOp, &mapMode) == AI_SUCCESS) {
                const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());
                if (embeddedTexture) {
                    // Handle the embedded texture
                    GLuint textureID;
                    glGenTextures(1, &textureID);

                    glBindTexture(GL_TEXTURE_2D, textureID);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, embeddedTexture->mWidth, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, embeddedTexture->pcData);

                    glGenerateMipmap(GL_TEXTURE_2D);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    textures.push_back({textureID, typeName, path.C_Str()});
                } else {
                    // Handle external textures if necessary
                    std::cerr << "External texture path: " << path.C_Str() << std::endl;
                }
            }

        }
    }



};
