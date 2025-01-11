#version 450 core

out vec4 FragColor;

in vec3 FragPos;   // Fragment position (from vertex shader)
in vec3 Normal;    // Normal vector (from vertex shader)
in vec2 TexCoord;  // Texture coordinates (not used yet but available)

uniform vec3 objectColor;   // Base object color
uniform vec3 lightColor;    // Color of the light
uniform vec3 lightDir;      // Direction of the light (normalized)
uniform vec3 viewPos;       // Position of the viewer/camera

void main() {
    // Normalize the normal vector
    vec3 norm = normalize(Normal);

    // Use the normalized light direction (negative for consistency with OpenGL conventions)
    vec3 lightDirection = normalize(-lightDir);

    // Diffuse shading: max(dot product of normal and light direction, 0.0)
    float diff = max(dot(norm, lightDirection), 0.0);

    // Combine diffuse shading with light color and object color
    vec3 diffuse = diff * lightColor;

    // Ambient shading: small base color to avoid complete darkness
    vec3 ambient = 0.1 * lightColor;

    // Final color: ambient + diffuse
    vec3 result = (ambient + diffuse) * objectColor;

    FragColor = vec4(result, 1.0);
}
