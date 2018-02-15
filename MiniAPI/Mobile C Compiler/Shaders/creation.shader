// creation by Silexars
// http://www.pouet.net/prod.php?which=57245
// downloaded from https://www.shadertoy.com/view/XsXXDn

precision highp float;
varying float iTime;
varying vec2  iResolution;

void main()
{
    vec3  c;
    float l, z = iTime;

    for (int i = 0; i < 3; ++i)
    {
        vec2 uv, p  = gl_FragCoord.xy / iResolution.xy;
        uv          = p;
        p          -= 0.5;
        p.x        *= iResolution.x / iResolution.y;
        z          += 0.07;
        l           = length(p);
        uv         += p / l * (sin(z) + 1.0) * abs(sin(l * 9.0 - z * 2.0));
        c[i]        = 0.01 / length(abs(mod(uv, 1.0) - 0.5));
    }

    gl_FragColor = vec4(c / l, iTime);
}
