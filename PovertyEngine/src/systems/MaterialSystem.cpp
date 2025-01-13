#include "MaterialSystem.h"
#include <iostream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
std::unordered_map<std::string, int> cache;
std::vector<Material> materials;
int materialIds;
// Helper function to load a single texture
static GLuint LoadTextureFromFile(const std::string& texturePath)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
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
    return materials[id - 1];
}

Material MaterialSystem::LoadMaterial(const std::string& materialPath)
{
    >"tähän pitää lisää varmaan"
    if (cache.find(materialPath) != cache.end())
    {
        return GetMaterialByID(cache[materialPath]);
    }
    materialIds++;
    Material material;
    // Load diffuse texture
    std::string diffusePath = materialPath + "mainTex.png";
    material.diffuseTextureID = LoadTextureFromFile(diffusePath);
    material.materialId = materialIds;
    if (material.diffuseTextureID == 0)
    {
        std::cerr << "Diffuse texture failed to load for material: " << materialPath << std::endl;
        return material;
    }

    cache[materialPath] = materialIds;
    materials.push_back(material); 
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
        glDeleteTextures(1, &material.diffuseTextureID);
    }
    materials.clear();
    cache.clear(); 
}
