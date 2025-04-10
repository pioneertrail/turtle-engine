#version 330 core

in vec4 particleColor; // Color passed from vertex shader
in float lifeRatio;   // Normalized lifetime (1.0 at spawn, 0.0 at death)
in float pulseFactor; // Time-based pulsing factor from vertex shader

out vec4 FragColor;

void main()
{
    vec3 color = particleColor.rgb;
    
    // Dynamic Glow: Scale intensity based on lifeRatio
    // Brighter when new (lifeRatio near 1.0), fades as it approaches 0.0
    float intensity = smoothstep(0.0, 1.0, lifeRatio); // Smooth fade
    color *= (0.5 + intensity * 0.5); // Scale from 0.5 to 1.0 intensity

    // Apply Pulsing Effect
    color *= (0.8 + 0.2 * pulseFactor); // Modulate between 0.8 and 1.0 intensity

    // Fade out alpha towards the end of life
    float alpha = particleColor.a * smoothstep(0.0, 0.8, lifeRatio); // Start fading alpha early

    FragColor = vec4(color, alpha);
} 