#version 400 core
in vec4 particleColor; // Receive color from vertex shader
out vec4 FragColor;

void main()
{
    // Optional: Could add fading based on particle life if that data is passed
    // For now, just use the color passed from the vertex stage
    FragColor = particleColor; 
    // Simple circle rendering (optional, can just use square points)
    // vec2 coord = gl_PointCoord - vec2(0.5);
    // if(length(coord) > 0.5) {
    //     discard;
    // }
} 