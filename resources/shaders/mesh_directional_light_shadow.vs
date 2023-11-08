#version 410 core

#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 in_position; // for some types as dvec3 takes 2 locations

layout(std140, binding = 0, row_major) uniform SteadyBlock
{
    mat4 proj_view_matrix;
} steady_block;

layout(location = 0) uniform mat4 model_matrix;

void main()
{
    vec3 position_world_space = (model_matrix * vec4(in_position, 1.0)).xyz;
    gl_Position = steady_block.proj_view_matrix * vec4(position_world_space, 1.0);
}