#version 330 core
#define MPI 3.1415926535897932384626433832795
out vec4 FragColor;

in vec2 uv;

uniform sampler2D _texture;

void main() {
    vec4 vertcolor = texture(_texture,uv);
    if (vertcolor.a<0.01) discard;
    FragColor = vertcolor;
} 