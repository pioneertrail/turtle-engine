#version 330 core

// Input vertex attributes for each particle
layout (location = 0) in vec3 aPos;      
layout (location = 1) in vec4 aColor;    
layout (location = 2) in float aLifeRatio; // NEW: Normalized life (0-1)

// Uniforms
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Time uniform for pulsing effect
// Model matrix is likely identity for world-space particles, pass if needed
// uniform mat4 model;

// Outputs to Fragment Shader
out vec4 particleColor;
out float lifeRatio; // Pass life ratio through
out float pulseFactor; // Pass pulse factor to fragment shader

void main()
{
    // Calculate world position (simple pass-through for now, could add offset later)
    vec3 worldPos = aPos;
    
    // Calculate final screen position
    gl_Position = projection * view * vec4(worldPos, 1.0);

    // Enable setting particle size in vertex shader if needed
    // gl_PointSize = particleSize; 
    gl_PointSize = 10.0; // Example fixed size

    // Pass attributes to fragment shader
    particleColor = aColor;
    lifeRatio = aLifeRatio;
    
    // Calculate pulsing factor based on time and position
    pulseFactor = sin(time * 3.0 + aPos.x + aPos.y); 
} 