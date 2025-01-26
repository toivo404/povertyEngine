#include "parseLevelFile.h"

#include <fstream>
#include <iostream>

std::vector<std::pair<std::string, Transform>> parseLevelFile(const std::string& filePath)
{
    std::vector<std::pair<std::string, Transform>> result;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << "\n";
        return result;
    }

    std::string line;
    std::regex regexPattern(R"((\S+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+))");

    while (std::getline(file, line)) {
        std::smatch matches;
        if (std::regex_match(line, matches, regexPattern)) {
            // Extract name and transform components
            std::string name = matches[1].str();
            float posX = std::stof(matches[2].str());
            float posY = std::stof(matches[3].str());
            float posZ = std::stof(matches[4].str());
            float rotX = std::stof(matches[5].str());
            float rotY = std::stof(matches[6].str());
            float rotZ = std::stof(matches[7].str());
            float scaleX = std::stof(matches[8].str());
            float scaleY = std::stof(matches[9].str());
            float scaleZ = std::stof(matches[10].str());

            // Convert Euler angles (rotX, rotY, rotZ) to a quaternion
            glm::quat rotation = glm::quat(glm::vec3(rotX, rotY, rotZ));

            // Create Transform object
            Transform transform;
            transform.position = glm::vec3(posX, posY, posZ);
            transform.scale = glm::vec3(scaleX, scaleY, scaleZ);
            transform.not_rotation = rotation;

            // Add to the result list
            result.emplace_back(name, transform);
        }
    }

    file.close();
    return result;
}
