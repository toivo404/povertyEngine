#version 450 core

layout (location = 0) in vec3 aPos;      // Vertex position
layout (location = 1) in vec3 aNormal;   // Vertex normal
layout (location = 2) in vec2 aTexCoord; // Texture coordinates

uniform mat4 transform; // Transformation matrix

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    gl_Position = transform * vec4(aPos, 1.0); // Apply transformation
    FragPos = aPos;                            // Pass position to fragment shader
    Normal = aNormal;                          // Pass normal to fragment shader
    TexCoord = aTexCoord;                      // Pass texture coordinates
}
