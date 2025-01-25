#pragma once
#include <glad.h>
#include <string>
#include <core.h>
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

