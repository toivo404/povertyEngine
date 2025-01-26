#include "parseLevelFile.h"

#include <fstream>
#include <iostream>

// Function to convert Blender axes to OpenGL axes
glm::vec3 convertBlenderToOpenGLPosition(const glm::vec3& blenderPosition) {
    return glm::vec3(blenderPosition.x, blenderPosition.z, -blenderPosition.y);
}

glm::quat convertBlenderToOpenGLRotation(const glm::quat& blenderRotation) {
    // Convert Blender quaternion to OpenGL by swapping axes
    // Blender's Y-axis becomes OpenGL's negative Z-axis
    glm::mat4 rotationMatrix = glm::toMat4(blenderRotation);

    glm::mat4 conversionMatrix = glm::mat4(1.0f);
    conversionMatrix[1][1] = 0.0f;  // Blender Y (OpenGL Z)
    conversionMatrix[1][2] = 1.0f;  // Blender Z -> OpenGL Y
    conversionMatrix[2][1] = -1.0f; // Blender Y -> OpenGL -Z
    conversionMatrix[2][2] = 0.0f;

    rotationMatrix = conversionMatrix * rotationMatrix;
    return glm::quat_cast(rotationMatrix);
}
// Function to parse the file and return a list of pairs of name and Transform
std::vector<std::pair<std::string, Transform>> parseLevelFile(const std::string& filePath) {
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

            // Create Transform object
            Transform transform;
            transform.position = convertBlenderToOpenGLPosition(glm::vec3(posX, posY, posZ));
            transform.scale = glm::vec3(scaleX, scaleY, scaleZ);

            // Convert Euler angles (rotX, rotY, rotZ) to a quaternion
            glm::quat blenderRotation = glm::quat(glm::vec3(rotX, rotY, rotZ));
            transform.not_rotation = convertBlenderToOpenGLRotation(blenderRotation);
            transform.RotateAroundAxis(glm::vec3(1,0,0),-180);
            // Add to the result list
            result.emplace_back(name, transform);
        }
    }

    file.close();
    return result;
}
