#version 400 core

// Inputs from Vertex Shader
in float Velocity; // Normalized velocity (0.0-1.0)

// Output color
out vec4 FragColor;

void main()
{
    // Define the base color (red-orange)
    vec3 baseColor = vec3(1.0, 0.4, 0.0); 

    // Basic intensity modulation based on velocity 
    // Faster movement -> brighter color (simple example)
    float intensity = 0.5 + (Velocity * 0.5); // Map velocity [0,1] to intensity [0.5, 1.0]

    // Calculate the final color
    vec3 finalColor = baseColor * intensity;

    // Set the fragment color (fully opaque for now)
    FragColor = vec4(finalColor, 1.0); 
} 