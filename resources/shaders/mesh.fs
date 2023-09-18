#version 330 core

// read in fragnormal (from vertex shader)
layout(location = 0) in highp vec3 in_world_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec3 in_tangent;
layout(location = 3) in highp vec2 in_texcoord;

layout(location = 0) out highp vec4 out_scene_color;

highp vec3 calculateNormal()
{
    highp vec3 tangent_normal = texture(normal_texture_sampler, in_texcoord).xyz * 2.0 - 1.0;

    highp vec3 N = normalize(in_normal);
    highp vec3 T = normalize(in_tangent.xyz);
    highp vec3 B = normalize(cross(N, T));

    highp mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangent_normal);
}

void main()
{
    highp vec3 result_color;
    
    out_scene_color = vec4(result_color, 1.0);
}