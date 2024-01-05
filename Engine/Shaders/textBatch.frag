#version 450 core
in vec2 uv;
in vec4 mod;
in float texIndex;
out vec4 outColor;
uniform sampler2D _textures[32];
void main() {
    vec4 vertColor=vec4(1.0f,1.0f,1.0f,texture(_textures[int(texIndex)],uv).r);
    if (vertColor.a<=0.05f) { discard;return; }
    outColor=mod;
}