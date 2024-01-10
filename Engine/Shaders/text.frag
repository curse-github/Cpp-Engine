#version 450 core
in vec2 uv;
out vec4 outColor;
uniform sampler2D text;
uniform vec4 textColor;
void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, uv).r);
    if (sampled.a<=0.05) { discard;return; }
    outColor = textColor * sampled;
}