#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
// uniform vec3 textColor;

void main() {
    float a = texture(text, TexCoords).a;
    color = vec4(mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), a).rgb, 1.0);
    // color = vec4(texture(text, TexCoords).rgb, 1.0);
    // color= vec4(1.0, 0.0, 0.0, 1.0);
}
