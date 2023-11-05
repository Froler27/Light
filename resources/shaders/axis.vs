#version 410 core

layout(location = 0) in vec3 in_position; // for some types as dvec3 takes 2 locations
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_world_position; // output in framebuffer 0 for fragment shader
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec2 out_texcoord;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

void main()
{
    highp vec3 model_position;
    highp vec3 model_normal;
    highp vec3 model_tangent;

    model_position = in_position;
    model_normal   = in_normal;
    model_tangent  = in_tangent;

    out_world_position = (model_matrix * vec4(model_position, 1.0)).xyz;
    gl_Position = proj_matrix * view_matrix * vec4(out_world_position, 1.0);

    // TODO: normal matrix
    mat3x3 tangent_matrix = mat3x3(model_matrix[0].xyz, model_matrix[1].xyz, model_matrix[2].xyz);
    out_normal            = normalize(tangent_matrix * model_normal);
    out_tangent           = normalize(tangent_matrix * model_tangent);

    out_texcoord = in_texcoord;
}