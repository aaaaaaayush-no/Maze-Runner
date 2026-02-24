#version 330 core

in vec3 vColor;
in vec2 vScreenPos;
out vec4 outColor;

uniform vec2 mapCenter;   // center of minimap in NDC
uniform float mapRadius;  // radius in NDC (X-axis scale)
uniform float mapAspect;  // screenWidth / screenHeight for correct circle
uniform bool circleClip;  // enable circular clipping

void main()
{
    if (circleClip) {
        vec2 diff = vScreenPos - mapCenter;
        diff.y /= mapAspect; // correct for aspect ratio to make a circle
        float dist = length(diff);
        if (dist > mapRadius)
            discard;
        // Soft border edge
        float borderWidth = mapRadius * 0.03;
        float borderInner = mapRadius - borderWidth;
        if (dist > borderInner) {
            // Blend to border color (white-ish)
            float t = (dist - borderInner) / borderWidth;
            vec3 borderColor = vec3(0.6, 0.65, 0.7);
            outColor = vec4(mix(vColor, borderColor, t * 0.8), 1.0);
            return;
        }
    }
    outColor = vec4(vColor, 1.0);
}
