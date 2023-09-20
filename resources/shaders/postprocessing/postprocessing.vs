#version 410 core

const vec4 vertices[3] = vec4[3] (
    vec4(-1., 3., 0, 1.),
    vec4(-1., -1., 0, 1.),
    vec4(3., -1., 0, 1.)
);

const vec2 texcoords[3] = vec2[3] (
    vec2(0., 2.),
    vec2(0., 0.),
    vec2(2., 0.)
);

layout(location = 0) out vec2 out_texcoord;

void main()
{
    out_texcoord = texcoords[gl_VertexID];
    gl_Position = vertices[gl_VertexID];
}