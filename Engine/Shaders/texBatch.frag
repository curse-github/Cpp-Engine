#version 450 core
in vec2 uv;
in vec4 mod;
in float texIndex;
out vec4 outColor;
uniform sampler2D _textures[32];
void main() {
    if (texIndex>=32||texIndex<0) outColor=mod;
    else {
        vec4 vertColor=texture(_textures[int(texIndex)],uv);
        if (vertColor.a<=0.05f) discard;
        if (mod.r==0.0f&&mod.g==0.0f&&mod.b==0.0f&&mod.a==0.0f) { outColor=vertColor; }
        else { outColor=vertColor*mod; }
    }
}