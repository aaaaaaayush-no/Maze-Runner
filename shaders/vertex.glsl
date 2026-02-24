#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 FragColor;
out vec3 FragNormal;
out vec3 FragPos;
out vec2 TexCoord;
out float Visibility;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float fogDensity;
uniform float fogGradient;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 viewPos  = view * worldPos;
    gl_Position   = projection * viewPos;

    FragPos    = worldPos.xyz;
    FragColor  = aColor;
    FragNormal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord   = aTexCoord;

    // Fog calculation
    float dist = length(viewPos.xyz);
    Visibility = clamp(exp(-pow(dist * fogDensity, fogGradient)), 0.0, 1.0);
}
