#pragma once
#include <string>
#include <vector>

#include "json.hpp"

struct ModelData {
    std::string modelFile;
    std::string textureFile;
    std::string name;
};

class ModelFileLoader
{
public:
    static std::string Load(const std::string& filePath, std::vector<ModelData>& models);
};
