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
    mat4 light_proj_view_matrix;
} steady_block;

layout(std140, binding = 1, row_major) uniform TransientBlock
{
    Material material;
} transient_block;

layout(location = 1) uniform sampler2D texture_diffuse;
layout(location = 2) uniform sampler2D directional_light_shadow;

vec2 ndcxy_to_uv(highp vec2 ndcxy) { return ndcxy * vec2(0.5, 0.5) + vec2(0.5, 0.5); }

vec2 uv_to_ndcxy(highp vec2 uv) { return uv * vec2(2.0, 2.0) + vec2(-1.0, -1.0); }

bool no_directional_light_shadow() {
    vec4 position_clip = steady_block.light_proj_view_matrix * vec4(in_world_position, 1.0);
    vec3 position_ndc  = position_clip.xyz / position_clip.w;
    vec2 uv = ndcxy_to_uv(position_ndc.xy);
    float closest_depth = texture(directional_light_shadow, uv).r  + 0.00075;
    float current_depth = (position_ndc.z+1.)/2.;
    //closest_depth = 0.05;
    return closest_depth > current_depth;
}

void main()
{
    vec3 N = normalize(in_normal);

    vec3 L;
    vec3 E = vec3(0.);
    if (steady_block.light.pos_or_dir.w == 0.) { // 平行光
        L = -normalize(steady_block.light.pos_or_dir.xyz);
        //E = max(0, dot(N, L))*steady_block.light.color.rgb;   // irradiance
        float NoL = dot(N, L);
        if (NoL > 0. && no_directional_light_shadow()) 
        {
            E = NoL*steady_block.light.color.rgb;               // irradiance
        }
        
    }else{  // 点光源
        float r = length(steady_block.light.pos_or_dir.xyz - in_world_position);
        L = (steady_block.light.pos_or_dir.xyz - in_world_position)/r;
        E = max(0, dot(N, L)) / (r*r) * steady_block.light.color.rgb;   // irradiance
    }

    vec3 R = texture(texture_diffuse, in_texcoord).xyz;
    vec3 diffuse = R/PI;    // 兰伯特反射的BRDF

    vec3 V = normalize(steady_block.camera_position - in_world_position);
    vec3 H = normalize(L+V);
    vec3 specular = transient_block.material.ks_p.xyz * pow(max(0, dot(N, H)), transient_block.material.ks_p.w);// 高光反射的BRDF
    vec3 brdf = diffuse + specular;

    vec3 Lr = brdf * E;     // radiance = brdf * irradiance

    vec3 Ia = vec3(0.1);  // 环境光强度
    vec3 ka = R;            // 环境光反射系数
    vec3 La = ka * Ia;      // 环境光radiance

    out_scene_color = vec4(Lr + La, 1.0);
}