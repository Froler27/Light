#version 330 core

uniform sampler2D screenTexture;

layout(location = 0) in vec2 in_texcoord;

out vec4 fragColor;

void main()
{
    fragColor = texture(screenTexture, in_texcoord);
}
