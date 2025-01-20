#include <glm.hpp>
#include <vector>
#include <glad.h>

class DebugLineRenderer {
public:
    DebugLineRenderer() {
        // Initialize buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &CBO);

        glBindVertexArray(VAO);

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * maxLines, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        // Color buffer
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * maxLines, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    ~DebugLineRenderer() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &CBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, float lineWidth = 1.0f) {
        // Store line vertices
        lines.push_back(start);
        lines.push_back(end);

        // Store line colors
        colors.push_back(color);
        colors.push_back(color);

        // Store line width
        lineWidths.push_back(lineWidth);
    }

    void Render(const glm::mat4& view, const glm::mat4& projection, GLuint shaderProgram) {
        if (lines.empty()) return;

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(glm::vec3), lines.data());

        // Upload color data
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(glm::vec3), colors.data());

        // Use shader program
        glUseProgram(shaderProgram);
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(VAO);

        // Draw lines
        for (size_t i = 0; i < lines.size() / 2; ++i) {
            glLineWidth(lineWidths[i]);
            glDrawArrays(GL_LINES, i * 2, 2);
        }

        glBindVertexArray(0);

        // Clear data after rendering
        lines.clear();
        colors.clear();
        lineWidths.clear();
    }

private:
    GLuint VAO, VBO, CBO;
    std::vector<glm::vec3> lines;
    std::vector<glm::vec3> colors;
    std::vector<float> lineWidths;

    static constexpr size_t maxLines = 1000; // Maximum number of lines
};
