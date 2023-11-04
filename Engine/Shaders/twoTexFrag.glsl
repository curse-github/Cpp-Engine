#version 330 core
#define MPI 3.1415926535897932384626433832795
out vec4 FragColor;

in vec2 uv;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixVal;

void main() {
    vec4 vertcolor = mix(texture(texture1,uv), texture(texture2,uv), mixVal);
    if (vertcolor.a<0.01) discard;
    FragColor = vertcolor;
} 