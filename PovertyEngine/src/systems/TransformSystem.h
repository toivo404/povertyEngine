#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp> // For glm::translate, glm::rotate, glm::scale
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::quat not_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity rotation
    glm::mat4 model = glm::mat4(1.0f);

    void UpdateModelMatrix()
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        // Convert the quaternion to a 4x4 rotation matrix
        model *= glm::toMat4(not_rotation);

        // Apply scaling
        model = glm::scale(model, scale);
    }

    void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        glm::vec3 forward = glm::normalize(target - position);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        glm::vec3 correctedUp = glm::cross(forward, right);

        // Create a rotation matrix from the basis vectors
        glm::mat3 rotationMatrix;
        rotationMatrix[0] = right;
        rotationMatrix[1] = correctedUp;
        rotationMatrix[2] = -forward;

        // Convert the rotation matrix to a quaternion
        not_rotation = glm::quat_cast(rotationMatrix);
    }
    
    
    // Helper to rotate around an axis
    void RotateAroundAxis(const glm::vec3& axis, float angle) {
        glm::quat deltaRotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
        not_rotation = deltaRotation * not_rotation; // Apply rotation (order matters)
    }

    glm::vec3 Apply(const glm::vec3& vec)
    {
        return model * glm::vec4(vec, 1.0f);
    }

    // Computes a direction vector based on the rotation
    glm::vec3 GetDirection() const {
        // Forward direction in local space (Z-axis in OpenGL)
        return glm::normalize(glm::rotate(not_rotation, glm::vec3(0.0f, 0.0f, -1.0f)));
    }
};
