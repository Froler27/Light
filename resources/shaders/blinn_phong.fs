#version 410 core

#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

#define PI 3.1416

// read in fragnormal (from vertex shader)
layout(location = 0) in highp vec3 in_world_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec3 in_tangent;
layout(location = 3) in highp vec2 in_texcoord;

layout(location = 0) out highp vec4 out_scene_color;

struct Light{
    vec4 color;     // 点光源的辐射强度或平行光的直射辐照度
    vec4 pos_or_dir;// 点光源的位置或平行光的方向
};

struct AmbientLight {
    vec3 color;
    float intensity;
};

struct Material{
    vec4 color; // 表面反射光照的比例
    vec4 ks_p;  // 前3个分量表示高光反射系数，第4个分量表示高光强度系数
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
} steady_block;

layout(std140, binding = 1, row_major) uniform TransientBlock
{
    Material material;
} transient_block;

layout(location = 1) uniform sampler2D texture_diffuse;

const Material material_c = Material(vec4(1.f, 0.5f, 0.31f, 1.f), vec4(vec3(1.), 32));

void main()
{
    vec3 n = normalize(in_normal);

    vec3 l, E;
    if (steady_block.light.pos_or_dir.w == 0.) { // 平行光
        l = -normalize(steady_block.light.pos_or_dir.xyz);
        E = max(0, dot(n, l))*steady_block.light.color.rgb;  // irradiance
    }else{  // 点光源
        float r = length(steady_block.light.pos_or_dir.xyz - in_world_position);
        l = (steady_block.light.pos_or_dir.xyz - in_world_position)/r;
        E = max(0, dot(n, l)) / (r*r) * steady_block.light.color.rgb;// irradiance
    }

    vec3 diffuse = texture(texture_diffuse, in_texcoord).xyz/PI;
    //vec3 diffuse = material.color.xyz/PI;// 兰伯特反射的BRDF

    vec3 v = normalize(steady_block.camera_position - in_world_position);
    vec3 h = normalize(l+v);
    vec3 specular = transient_block.material.ks_p.xyz * pow(max(0, dot(n, h)), transient_block.material.ks_p.w);// 高光反射的BRDF
    vec3 brdf = diffuse + specular;

    vec3 L = brdf * E; // radiance = brdf * irradiance

    out_scene_color = vec4(L, 1.0);
}