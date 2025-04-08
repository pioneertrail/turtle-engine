#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform vec4 color;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if outside shadow map
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // PCF (Percentage Closer Filtering)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - 0.005 > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Ambient
    vec3 ambient = 0.3 * color.rgb;
    
    // Diffuse
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0)); // Directional light
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color.rgb;
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.3 * spec * color.rgb;
    
    // Shadow
    float shadow = ShadowCalculation(FragPosLightSpace);
    
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    
    FragColor = vec4(result, color.a);
} 