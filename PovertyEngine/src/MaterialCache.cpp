#include "MaterialCache.h"
#include <iostream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <fstream>
#include <stb_image.h>
#include <MaterialCache.h>
#include "json.hpp"
int MaterialSystem::materialIds; 
std::unordered_map<std::string, MaterialFile> cache;
std::vector<MaterialFile> materials;

// Helper function to load a single texture
static GLuint LoadTextureFromFile(const std::string& texturePath)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << texturePath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, (channels == 4 ? GL_RGBA : GL_RGB), width, height, 0,
                 (channels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

Material MaterialSystem::GetMaterialByID(int id)
{
    if (id <= 0)
    {
        std::cerr << "Bad material ID " << id << std::endl;
        return {};
    }
    auto found = materials[id - 1];
    return found.loadedMaterial;
}

MaterialFile LoadMaterialFile(const std::string filePath)
{
    // Open the file
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    try {
        // Parse the JSON file
        nlohmann::json jsonData;
        file >> jsonData;

        // Check and retrieve the "texture" field
        if (jsonData.contains("texture") && jsonData["texture"].is_string()) {
            std::string texture = jsonData["texture"];
            return MaterialFile{texture};
        } else {
            std::cerr << "The 'texture' field is missing or not a string!" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << filePath << std::endl;
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return MaterialFile{};

}

Material MaterialSystem::LoadMaterial(const std::string& materialPath, const char* basePath)
{
    if (cache.find(materialPath) != cache.end())
    {
        return cache[materialPath].loadedMaterial;
    }
    MaterialSystem::materialIds++;
    // Load diffuse texture
    Material material; 
    auto matFile = LoadMaterialFile( std::string(basePath) + materialPath);
    material.diffuseTextureID = LoadTextureFromFile(std::string(basePath) + matFile.texturePath);
    material.materialId = materialIds;
    if (material.diffuseTextureID == 0)
    {
        std::cerr << "Diffuse texture failed to load for material: " << materialPath << std::endl;
        return material;
    }
    matFile.loadedMaterial = material;
    cache[materialPath] = matFile;
    materials.push_back(matFile); 
    return material;
}

 

void MaterialSystem::BindTexture(const Material& material)
{
    // Bind diffuse texture
    if (material.diffuseTextureID != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.diffuseTextureID);
    }
}

void MaterialSystem::CleanUp()
{
    for (auto material : materials)
    {
        glDeleteTextures(1, &material.loadedMaterial.diffuseTextureID);
    }
    materials.clear();
    cache.clear(); 
}
