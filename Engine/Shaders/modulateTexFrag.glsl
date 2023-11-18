#version 330 core
in vec2 uv;
out vec4 outColor;
uniform sampler2D _texture;
uniform vec3 color;
void main() {
    vec4 vertcolor = texture(_texture,uv);
    if (vertcolor.a<0.05) discard;
    outColor = vec4(vertcolor.rgb*color,vertcolor.a);
}