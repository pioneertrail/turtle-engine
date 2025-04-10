#version 400 core

// Input vertex attributes
// Assuming trajectory points are fed as position
layout (location = 0) in vec3 aPos; 
// Assuming normalized velocity (0.0-1.0) is fed as a float attribute
layout (location = 1) in float aVelocity; 

// Uniforms
// Model-View-Projection matrix (needs to be set from C++ code)
uniform mat4 MVP; 

// Outputs to Fragment Shader
out float Velocity; // Pass normalized velocity through

void main()
{
    // Calculate the final clip space position
    gl_Position = MVP * vec4(aPos, 1.0);

    // Pass the velocity to the fragment shader
    Velocity = aVelocity; 
} 