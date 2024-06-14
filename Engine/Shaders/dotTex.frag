#version 450 core
in vec2 uv;
out vec4 outColor;
uniform sampler2D _texture;
uniform vec4 modulate=vec4(1.0,1.0,1.0,1.0);
void main() {
    if ((pow(uv.x-0.5f,2)+pow(uv.y-0.5f,2))>0.25f) { discard;return; }
    vec4 vertcolor = texture(_texture,vec2(uv.x,-uv.y));
    if (vertcolor.a<0.05) { discard;return; }
    if (modulate.r==0.0f&&modulate.g==0.0f&&modulate.b==0.0f&&modulate.a==0.0f) outColor = vertcolor;
    else outColor = vertcolor*modulate;
}