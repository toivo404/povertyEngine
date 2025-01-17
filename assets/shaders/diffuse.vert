#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;    // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTex;    // Texture coordinates

// Outputs for the Fragment Shader
out vec3 FragPos;   // Fragment position in world space
out vec3 Normal;    // Normal vector in world space
out vec2 TexCoord;  // Texture coordinates

// Uniforms
uniform mat4 camMatrix; // Combined view-projection matrix
uniform mat4 model;     // Model matrix

void main()
{
    // Transform the vertex position into world space
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transform the normal vector into world space
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Pass the texture coordinates directly to the fragment shader
    TexCoord = aTex;

    // Transform the vertex position into clip space
    gl_Position = camMatrix * vec4(FragPos, 1.0);
}
