#version 410 core

#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 in_position; // for some types as dvec3 takes 2 locations
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_world_position; // output in framebuffer 0 for fragment shader
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec2 out_texcoord;

struct Light{
    vec4 color;     // 点光源的辐射强度或平行光的直射辐照度
    vec4 pos_or_dir;// 点光源的位置或平行光的方向
};

struct AmbientLight {
    vec3 color;
    float intensity;
};

layout(std140, binding = 0, row_major) uniform SteadyBlock
{
    Light light;
    AmbientLight ambient_light;
    vec3  camera_position;
    float _padding_camera_position;
    mat4 view_matrix;
    mat4 proj_matrix;
    mat4 proj_view_matrix;
    mat4 light_proj_view_matrix;
} steady_block;

layout(location = 0) uniform mat4 model_matrix;

void main()
{
    vec3 model_position;
    vec3 model_normal;
    vec3 model_tangent;

    model_position = in_position;
    model_normal   = in_normal;
    model_tangent  = in_tangent;

    out_world_position = (model_matrix * vec4(model_position, 1.0)).xyz;
    gl_Position = steady_block.proj_view_matrix * vec4(out_world_position, 1.0);

    mat3x3 tangent_matrix = mat3x3(model_matrix[0].xyz, model_matrix[1].xyz, model_matrix[2].xyz);
    out_normal = normalize(tangent_matrix * model_normal);
    out_tangent = normalize(tangent_matrix * model_tangent);
    out_texcoord = in_texcoord;
}