#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main() {
    mat4 mvp = projection * view * model;
    float d = sqrt(mvp[0][0] * mvp[0][0] + mvp[1][0] * mvp[1][0] + mvp[2][0]* mvp[2][0]);


    mvp[0] = vec4(1, 1, 1, mvp[0][3]);
    mvp[1] = vec4(1, 1, 1, mvp[1][3]);
    mvp[2] = vec4(1, 1, 1, mvp[2][3]);


    gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
