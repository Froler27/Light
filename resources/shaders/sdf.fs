#version 330 core

out vec4 fragColor;
 
void main()
{
    vec2 p = gl_FragCoord.xy + 0.5;
    
    vec4 col = vec4(1., 0., 0., 1.);
    fragColor = col;
}
