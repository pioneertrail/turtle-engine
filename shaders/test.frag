#version 330 core
out vec4 FragColor;

uniform vec4 color;

void main() {
    // Output color directly in linear space
    // The framebuffer will handle sRGB conversion
    FragColor = color;
} 