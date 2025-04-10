#version 400 core

// Inputs from Vertex Shader
in float Velocity; // Normalized velocity (0.0-1.0)

// Output color
out vec4 FragColor;

// Define base and hot colors for the effect
const vec3 BASE_COLOR = vec3(1.0, 0.4, 0.0); // Red-orange
const vec3 HOT_COLOR = vec3(1.0, 0.8, 0.2); // Brighter yellow-orange for high velocity

void main()
{
    // Sharpen the intensity response to velocity. 
    // Using pow makes slower parts dimmer and faster parts reach max intensity quicker.
    // Clamp ensures it stays within [0.5, 1.0] range.
    float intensity = clamp(0.5 + pow(Velocity, 2.0) * 0.5, 0.5, 1.0); 

    // Interpolate color based on velocity.
    // Use smoothstep for a nicer transition between base and hot colors as velocity increases.
    // Thresholds (e.g., 0.6 to 0.9) control when the color shift occurs.
    float hotMix = smoothstep(0.6, 0.9, Velocity);
    vec3 dynamicColor = mix(BASE_COLOR, HOT_COLOR, hotMix);

    // Calculate the final color with intensity applied
    vec3 finalColor = dynamicColor * intensity;

    // Output the final color (still fully opaque)
    FragColor = vec4(finalColor, 1.0); 
} 