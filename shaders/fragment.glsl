#version 330 core

in vec3 FragColor;
in vec3 FragNormal;
in vec3 FragPos;
in vec2 TexCoord;
in float Visibility;

out vec4 outColor;

uniform vec3 lightDir;
uniform vec3 fogColor;
uniform sampler2D wallTexture;
uniform bool useTexture;

// Sun/sky lighting
uniform vec3 sunColor;
uniform float ambientLevel;

// Torch lighting
uniform bool torchEnabled;
uniform vec3 torchPos;
uniform vec3 torchColor;
uniform float torchRadius;

// Edge outline effect
uniform bool enableEdgeOutline;

void main()
{
    vec3 norm    = normalize(FragNormal);
    vec3 lDir    = normalize(lightDir);

    // Sun/moon directional lighting
    float diff   = max(dot(norm, lDir), 0.0);
    float light  = ambientLevel + diff * (1.0 - ambientLevel) * 0.65;

    // Tint with sun color
    vec3 tint = mix(vec3(0.7, 0.75, 0.9), sunColor, 0.6);

    vec3 baseColor = FragColor;
    if (useTexture) {
        baseColor = texture(wallTexture, TexCoord).rgb * FragColor;
    }

    vec3 color = baseColor * light * tint;

    // Torch point light contribution
    if (torchEnabled) {
        vec3 toTorch = torchPos - FragPos;
        float dist = length(toTorch);
        if (dist < torchRadius) {
            vec3 torchDir = normalize(toTorch);
            float torchDiff = max(dot(norm, torchDir), 0.0);
            // Radial falloff (quadratic attenuation)
            const float LINEAR_ATTEN = 0.15;
            const float QUADRATIC_ATTEN = 0.08;
            float attenuation = 1.0 / (1.0 + LINEAR_ATTEN * dist + QUADRATIC_ATTEN * dist * dist);
            float falloff = clamp(1.0 - dist / torchRadius, 0.0, 1.0);
            falloff = falloff * falloff; // smooth falloff
            vec3 torchContrib = torchColor * torchDiff * attenuation * falloff * 1.5;
            color += baseColor * torchContrib;
        }
    }

    // Edge outline effect for blocky aesthetic
    if (enableEdgeOutline) {
        // Detect edges using texture coordinate proximity to 0/1 boundaries
        const float EDGE_WIDTH = 0.02;
        float edgeX = min(TexCoord.x, 1.0 - TexCoord.x);
        float edgeY = min(TexCoord.y, 1.0 - TexCoord.y);
        float edgeFactor = smoothstep(0.0, EDGE_WIDTH, min(edgeX, edgeY));
        color *= mix(0.6, 1.0, edgeFactor);
    }

    // Mix with fog
    color = mix(fogColor, color, Visibility);

    outColor = vec4(color, 1.0);
}
