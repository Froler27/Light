#version 310 es

// read in fragnormal (from vertex shader)
layout(location = 0) in highp vec3 in_world_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec3 in_tangent;
layout(location = 3) in highp vec2 in_texcoord;

layout(location = 0) out highp vec4 out_scene_color;

const highp vec3 LIGHT_POS = vec3(3., 0., 5);
const highp vec3 LIGHT_DIR = -vec3(3., 0., 5);
const highp vec3 LIGHT_COLOR = vec3(1.);

highp vec3 calculateNormal()
{
    highp vec3 tangent_normal = in_normal * 2.0 - 1.0;

    highp vec3 N = normalize(in_normal);
    highp vec3 T = normalize(in_tangent.xyz);
    highp vec3 B = normalize(cross(N, T));

    highp mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangent_normal);
}

void main()
{
    highp vec3 result_color = vec3(1., 1., 1.);

    highp float ambientStrenth = 0.1;
    highp vec3 ambient = ambientStrenth * LIGHT_COLOR;

    highp vec3 norm = normalize(in_normal);
    highp vec3 lightDir = normalize(-LIGHT_DIR);
    highp float diff = max(dot(norm, lightDir), 0.0);
    highp vec3 diffuse = diff * LIGHT_COLOR;

    highp float specularStrength = 0.5;
    highp vec3 viewDir = vec3(0.,-1.,0.);
    highp vec3 reflectDir = reflect(-lightDir, norm);  
    highp float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.);
    highp vec3 specular = specularStrength * spec * LIGHT_COLOR;  
        
    result_color = (ambient + diffuse + specular) * result_color;

    //result_color = vec3(1., 0., 0.);

    
    out_scene_color = vec4(result_color, 1.0);
}