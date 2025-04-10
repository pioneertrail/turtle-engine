#version 400 core

// Input vertex attributes for each particle
layout (location = 0) in vec3 aPos;      
layout (location = 1) in vec4 aColor;    
layout (location = 2) in float aLifeRatio; // NEW: Normalized life (0-1)

// Uniforms
uniform mat4 view;
uniform mat4 projection;
// Model matrix is likely identity for world-space particles, pass if needed
// uniform mat4 model;

// Outputs to Fragment Shader
out vec4 particleColor;
out float lifeRatio; // Pass life ratio through

void main()
{
    // Calculate final position in clip space using view and projection
    // Assuming aPos is world position
    gl_Position = projection * view * vec4(aPos, 1.0);

    // Set the size of the point sprite (Can make this dynamic later)
    gl_PointSize = 5.0; // Smaller default size

    // Pass color and life ratio to fragment shader
    particleColor = aColor;
    lifeRatio = aLifeRatio;
} 