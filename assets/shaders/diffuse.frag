#version 450 core

out vec4 FragColor;

in vec3 FragPos;   // Fragment position (from vertex shader)
in vec3 Normal;    // Normal vector (from vertex shader)
in vec2 TexCoord;  // Texture coordinates (passed from vertex shader)

uniform vec3 objectColor;   // Base object color
uniform vec3 lightColor;    // Color of the light
uniform vec3 lightDir;      // Direction of the light (normalized)
uniform vec3 viewPos;       // Position of the viewer/camera
uniform sampler2D diffuseTexture; // Diffuse texture

void main() {
    // Normalize the normal vector
    vec3 norm = normalize(Normal);

    // Use the normalized light direction (negative for consistency with OpenGL conventions)
    vec3 lightDirection = normalize(-lightDir);

    // Diffuse shading: max(dot product of normal and light direction, 0.0)
    float diff = max(dot(norm, lightDirection), 0.0);

    // Combine diffuse shading with light color
    vec3 diffuse = diff * lightColor;

    // Ambient shading: small base color to avoid complete darkness
    vec3 ambient = 0.1 * lightColor;

    // Sample the texture
    vec3 textureColor = texture(diffuseTexture, TexCoord).rgb;

    // Combine lighting with texture color
    vec3 result = (ambient + diffuse) * textureColor;

    FragColor = vec4(result, 1.0);
}
