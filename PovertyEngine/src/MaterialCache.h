#pragma once
#include <string>
#include <core.h>

using TextureHandle = uint32_t;

struct Material {
    int materialId;
    TextureHandle diffuseTextureID;
    /*
    TextureHandle specularTextureID;
    TextureHandle normalTextureID;
    */
    
};

struct MaterialFile
{
    std::string texturePath;
    Material loadedMaterial;
};

class PE_API MaterialSystem {
public:
    static Material GetMaterialByID(int id);
    static Material LoadMaterial(const std::string& materialPath, const char* basePath);
    static void BindTexture(const Material& material);
    static  void CleanUp();

private:
    static int materialIds; 
};

