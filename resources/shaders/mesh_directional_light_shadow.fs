#version 410 core

layout(location = 0) out highp float out_depth;

void main()
{
    out_depth = gl_FragCoord.z;
}
