#version 450 core

in vec2 TexCoord;
in float Alpha;

out vec4 FragColor;

uniform sampler2D particleTexture;
uniform vec4 particleColor;
uniform bool highContrastMode;
uniform vec4 debugColor;
uniform bool debugViewEnabled;

void main() {
    // Sample texture
    vec4 texColor = texture(particleTexture, TexCoord);
    
    // Apply particle color
    vec4 color = texColor * particleColor;
    
    // Apply high contrast mode
    if (highContrastMode) {
        // Convert to grayscale and increase contrast
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        color.rgb = vec3(step(0.5, gray));
    }
    
    // Apply debug color if enabled
    if (debugViewEnabled) {
        color = debugColor;
    }
    
    // Apply alpha
    color.a *= Alpha;
    
    // Output final color
    FragColor = color;
} 