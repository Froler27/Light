#version 330 core

uniform vec2 iResolution;
uniform vec2 iMouse;
uniform float iTime;
uniform int iFrame;
uniform sampler2D iChannel0;
uniform vec2 iChannel0Resolution;

out vec4 fragColor;

float opSmoothUnion( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); }
    
    
void main()
{
    vec2 p = gl_FragCoord.xy + 0.5;
    vec2 uv = gl_FragCoord.xy/iResolution.xy;
    
    vec4 col = texelFetch(iChannel0, ivec2(gl_FragCoord.xy), 0);
    fragColor = col;
    //fragColor = texture(iChannel0, uv);
    //fragColor = vec4(1.,0.,0,1.);
    if (p.x>iChannel0Resolution.x) {
        fragColor = vec4(1.,0.,0,1.);
    }else{
        if (col.a == 0.) {
            fragColor = vec4(0.,0.,1.,1.);
        }
    }
    // }else if (p.x<iResolution.x) {
    //     fragColor = vec4(0.,1.,0,1.);
    // }else{
    //     fragColor = vec4(0.,0.,0,1.);
    // }
}
