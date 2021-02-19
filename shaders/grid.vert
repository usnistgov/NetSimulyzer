#version 330

layout (location = 0) in vec2 in_position;

out vec3 fragment_position;

uniform mat4 view;
uniform mat4 projection;
uniform float height;

void main() {
    fragment_position = vec3(in_position[0], height, in_position[1]);
    gl_Position = projection * view * vec4(fragment_position, 1.0);
}
