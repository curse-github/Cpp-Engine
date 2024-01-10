#version 450 core
in vec2 uv;
out vec4 outColor;
uniform vec4 color;
void main() {
    if ((pow(uv.x-0.5f,2)+pow(uv.y-0.5f,2))>0.25f) { discard;return; }
    else outColor = color;
}