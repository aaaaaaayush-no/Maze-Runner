#version 330 core

in vec3 FragColor;
in vec3 FragNormal;
in vec3 FragPos;
in float Visibility;

out vec4 outColor;

uniform vec3 lightDir;
uniform vec3 fogColor;

void main()
{
    // Simple directional light
    vec3 norm    = normalize(FragNormal);
    vec3 lDir    = normalize(lightDir);
    float diff   = max(dot(norm, lDir), 0.0);
    float ambient = 0.3;
    float light  = ambient + diff * 0.7;

    vec3 color = FragColor * light;

    // Mix with fog
    color = mix(fogColor, color, Visibility);

    outColor = vec4(color, 1.0);
}
