#pragma once
#include <string>
#include <vector>

struct ModelData {
    std::string modelFile;
    std::string textureFile;
};

class ModelFileLoader
{
public:
    static std::string Load(const std::string& filePath, std::vector<ModelData>& models);
};
