#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in mat4 aInstanceMatrix;

out vec2 TexCoord;
out float Alpha;

uniform mat4 projection;
uniform mat4 view;
uniform bool highContrastMode;
uniform vec4 debugColor;
uniform bool debugViewEnabled;

void main() {
    TexCoord = aTexCoord;
    
    // Calculate position in view space
    vec4 viewPos = view * aInstanceMatrix * vec4(aPos, 1.0);
    
    // Apply projection
    gl_Position = projection * viewPos;
    
    // Calculate alpha based on distance for high contrast mode
    if (highContrastMode) {
        float dist = length(viewPos.xyz);
        Alpha = 1.0 - smoothstep(5.0, 20.0, dist);
    } else {
        Alpha = 1.0;
    }
    
    // Override alpha in debug mode
    if (debugViewEnabled) {
        Alpha = debugColor.a;
    }
} 