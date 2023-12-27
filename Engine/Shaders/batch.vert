#version 450 core
layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec2 vecUV;
layout (location = 2) in vec4 vecMod;
layout (location = 3) in float vecTexIndex;
uniform mat4 view;
uniform mat4 projection;
out vec2 uv;
out vec4 mod;
out float texIndex;
void main() {
    gl_Position = projection*view*vec4(vecPos, 1.0);
    uv=vecUV;
    mod=vecMod;
    texIndex=vecTexIndex;
}