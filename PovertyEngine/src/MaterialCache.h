#pragma once
#include <glad.h>
#include <string>
struct Material {
    int materialId;
    GLuint diffuseTextureID;
    /*
    GLuint specularTextureID;
    GLuint normalTextureID;
    */
    
};

struct MaterialFile
{
    std::string texturePath;
};

class MaterialSystem {
public:
    static Material GetMaterialByID(int id);
    static Material LoadMaterial(const std::string& materialPath, const char* basePath);
    static void BindTexture(const Material& material);
    static  void CleanUp();
};

