#version 330 core
#define MPI 3.1415926535897932384626433832795
out vec4 FragColor;

in vec2 uv;

uniform vec4 color;

void main() {
    FragColor = color;
} 