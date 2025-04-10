#version 400 core

in vec4 particleColor; // Color received from vertex shader
in float lifeRatio;   // Normalized life ratio (0.0 = dead, 1.0 = full life)

out vec4 FragColor;

void main()
{ 
    // Basic circular particle shape
    vec2 coord = gl_PointCoord - vec2(0.5);
    if(length(coord) > 0.5) {
        discard;
    }

    // Modulate alpha based on life ratio (smoother fade)
    float alpha = particleColor.a * smoothstep(0.0, 0.8, lifeRatio); // Fade out faster near end
    //float alpha = particleColor.a * lifeRatio; // Linear fade

    FragColor = vec4(particleColor.rgb, alpha);
} 