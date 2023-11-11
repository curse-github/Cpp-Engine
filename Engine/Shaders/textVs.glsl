#version 330 core
layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec2 vecUV;
uniform mat4 view;
uniform mat4 projection;
out vec2 uv;
void main() {
    gl_Position = projection*view*vec4(vecPos, 1.0);
    uv = vecUV;
}