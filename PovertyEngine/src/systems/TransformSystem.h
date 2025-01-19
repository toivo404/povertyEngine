#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp> // For glm::translate, glm::rotate, glm::scale

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
};
