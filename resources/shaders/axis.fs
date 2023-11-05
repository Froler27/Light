#version 410 core

// read in fragnormal (from vertex shader)
layout(location = 0) in highp vec3 in_world_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec3 in_tangent;
layout(location = 3) in highp vec2 in_texcoord;

layout(location = 0) out highp vec4 out_scene_color;

uniform mat4 proj_view_matrix;

const highp vec3 LIGHT_POS = vec3(3., 0., 5);
const highp vec3 LIGHT_DIR = -vec3(3., 0., 5);
const highp vec3 LIGHT_COLOR = vec3(1.);

void main()
{
    highp vec3 result_color = vec3(1., 1., 1.);

    if (in_texcoord.x < 0.33) {
        result_color = vec3(1., 0., 0.);
    }else if (in_texcoord.x < 0.66) {
        result_color = vec3(0., 1., 0.);
    }else{
        result_color = vec3(0., 0., 1.);
    }
    out_scene_color = vec4(result_color, 1.0);
}