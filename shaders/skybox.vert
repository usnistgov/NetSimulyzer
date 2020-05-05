#version 330
layout (location = 0) in vec3 in_position;

out vec3 textureCoordinates;

uniform mat4 view;
uniform mat4 projection;

void main() {
    textureCoordinates = in_position;
    gl_Position = projection * view * vec4(in_position, 1.0f);
}
