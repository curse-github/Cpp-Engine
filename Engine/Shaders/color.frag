#version 330 core
out vec4 outColor;
in vec2 uv;
uniform vec4 color;
void main() {
    outColor = color;
}