#version 330 core

out vec4 outColor;

// void main()
// {
//     gl_FragCoord;
//     gl_FragColor = vec4(0., 0.8, 1., 1.0);
// }

uniform vec2 iResolution;
uniform float iTime;

float opSmoothUnion( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); }
    
    
void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 p = gl_FragCoord.xy;
    vec2 uv = gl_FragCoord.xy/iResolution.xy;
    uv -= vec2(0.5);
    uv.x *= iResolution.x / iResolution.y;

    float d = length(uv) + sin(iTime * 2.) * 0.01 - 0.2;
    for (int i = 0;i < 6;i++) {
        float fi = float(i);
        float ltime = iTime * 0.5 + fi * 1.7341;
        float timePart = fract(ltime);
        float timeInt = round(ltime);
        vec2 uv2  = uv + vec2(cos(timeInt), sin(timeInt)) * (0.4 - timePart * 0.4);
        d = opSmoothUnion(length(uv2) + 0.05, d, 0.03);
    }
    
    d = 1.-smoothstep(0.1,0.108,d);

    outColor = vec4(d,d,d,1.0);
}