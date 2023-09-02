#version 330 core

const vec4 vertices[3] = vec4[3] (
    vec4(-1., 3., 0, 1.),
    vec4(-1., -1., 0, 1.),
    vec4(3., -1., 0, 1.)
);

void main()
{
    gl_Position = vertices[gl_VertexID];
}