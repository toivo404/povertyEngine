#include "ModelFileLoader.h"

#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

// ModelData structure

// Function to parse the JSON file
std::string ModelFileLoader::Load(const std::string& filePath, std::vector<ModelData>& models)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << filePath << std::endl;
        return "Error: Failed to open file";
    }

    try {
        // Parse JSON file
        json jsonData;
        file >> jsonData;

        // Check for the "models" field
        if (!jsonData.contains("models") || !jsonData["models"].is_array()) {
            std::cerr << "Error: 'models' field is missing or not an array" << std::endl;
            return "Error: 'models' field is invalid";
        }

        // Parse each model in the array
        for (const auto& modelJson : jsonData["models"]) {
            if (!modelJson.is_object()) {
                std::cerr << "Error: Model entry is not an object" << std::endl;
                continue;
            }

            ModelData model;
            if (modelJson.contains("modelFile") && modelJson["modelFile"].is_string()) {
                model.modelFile = modelJson["modelFile"];
            } else {
                std::cerr << "Error: 'modelFile' field is missing or not a string in one of the models" << std::endl;
                continue;
            }

            if (modelJson.contains("textureFile") && modelJson["textureFile"].is_string()) {
                model.textureFile = modelJson["textureFile"];
            } else {
                std::cerr << "Error: 'textureFile' field is missing or not a string in one of the models" << std::endl;
                continue;
            }

            if (modelJson.contains("name") && modelJson["name"].is_string()) {
                model.name = modelJson["name"];
            }

            models.push_back(model);
        }

        if (models.empty()) {
            return "Error: No valid models found in the JSON file";
        }

        return "Success";
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return std::string("Error: ") + e.what();
    }
}

