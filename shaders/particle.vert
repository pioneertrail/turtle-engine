#version 400 core

// Input vertex attributes for each particle
layout (location = 0) in vec3 aPos;      // Particle position (updated each frame)
layout (location = 1) in vec4 aColor;    // Particle color (could include alpha for fade)
layout (location = 2) in float aSize;     // Particle size

// Uniforms
uniform mat4 viewProjection; // Combined View * Projection matrix

// Outputs to Fragment Shader
out vec4 particleColor;

void main()
{
    // Calculate final position in clip space
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);

    // Set the size of the point sprite
    gl_PointSize = 10.0;

    // Pass color to fragment shader
    particleColor = aColor;
} 