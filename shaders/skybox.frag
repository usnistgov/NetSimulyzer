#version 330

in vec3 textureCoordinates;

out vec4 final_color;

uniform samplerCube skybox;

void main() {
    final_color = texture(skybox, textureCoordinates);
}
