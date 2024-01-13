#version 450 core
layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec2 vecUV;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 uvShift;
uniform vec2 uvScale;
uniform float uvRot;
out vec2 uv;
vec2 getUv() {
    return uvShift+vec2(
        (cos(uvRot) * (vecUV.x-0.5f)+sin(uvRot)*(vecUV.y-0.5f)+0.5f)*uvScale.x,
        (cos(uvRot) * (vecUV.y-0.5f)-sin(uvRot)*(vecUV.x-0.5f)+0.5f)*uvScale.y
    );
}
void main() {
    gl_Position = projection*view*model*vec4(vecPos, 1.0);
    uv = getUv();
}