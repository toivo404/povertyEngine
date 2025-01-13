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

class MaterialSystem {
public:
    static Material GetMaterialByID(int id);
    static Material LoadMaterial(const std::string& materialPath);
    static void BindTexture(const Material& material);
    static  void CleanUp();
};

