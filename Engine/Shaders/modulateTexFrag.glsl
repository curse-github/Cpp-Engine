#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D _texture;
uniform vec3 color;
void main() {
    vec4 vertcolor = texture(_texture,uv);
    if (vertcolor.a<=0.01) discard;
    FragColor = vec4(vertcolor.rgb*color,vertcolor.a);
}