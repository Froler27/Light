#version 330 core

uniform sampler2D image;
uniform bool horizontal;

const int Radius = 5;
const float weight[Radius] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
//const float k = 
//const float weight[Radius] = float[] (0.2, 0.1, 0.1, 0.1, 0.1);

out vec4 fragColor;

void main()
{
    ivec2 uv = ivec2(gl_FragCoord.xy);        
    vec3 result = texelFetch(image, ivec2(gl_FragCoord.xy), 0).rgb * weight[0];
    if(horizontal)
    {
        for(int i = 1; i < Radius; ++i)
        {
            result += texelFetch(image, uv + ivec2(1 * i, 0), 0).rgb * weight[i];
            result += texelFetch(image, uv - ivec2(1 * i, 0), 0).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < Radius; ++i)
        {
            result += texelFetch(image, uv + ivec2(0, 1 * i), 0).rgb * weight[i];
            result += texelFetch(image, uv - ivec2(0, 1 * i), 0).rgb * weight[i];
        }
    }
    fragColor = vec4(result, 1.0);
}
