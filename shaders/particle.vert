#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor; // Pass color from VBO

uniform mat4 model; // Keep model uniform for potential future use (though likely identity for world-space particles)
uniform mat4 view;
uniform mat4 projection;

out vec4 particleColor; // Pass color to fragment shader

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    particleColor = aColor;
    gl_PointSize = 5.0; // Set a default size for now
} 