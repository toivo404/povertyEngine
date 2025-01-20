#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp> // For glm::translate, glm::rotate, glm::scale
#include <gtc/quaternion.hpp>

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);  
    glm::vec3 scale = glm::vec3(1.0f);     
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::mat4 model = glm::mat4(1.0f);

    void UpdateModelMatrix()
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
    }

    void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        glm::vec3 forward = glm::normalize(target - position);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        glm::vec3 correctedUp = glm::cross(forward, right);

        // Create rotation matrix from these vectors
        glm::mat4 rotationMatrix(1.0f);
        rotationMatrix[0] = glm::vec4(right, 0.0f);
        rotationMatrix[1] = glm::vec4(correctedUp, 0.0f);
        rotationMatrix[2] = glm::vec4(-forward, 0.0f);

        // Decompose rotation matrix into Euler angles
        rotation = glm::degrees(glm::eulerAngles(glm::quat_cast(rotationMatrix)));
    }
};
