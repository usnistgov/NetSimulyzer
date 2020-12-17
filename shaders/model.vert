#version 330

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

out vec2 texture_coordinates;
out vec3 normal;
out vec3 fragment_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(in_position, 1.0);
    texture_coordinates = in_texture;

    // Only nessary if we allow non-uniform scaling
    mat3 Nonuniform_scale_model = mat3(transpose(inverse(model)));

    normal = Nonuniform_scale_model * in_normal;
    fragment_position = (model * vec4(in_position, 1.0)).xyz;
}
