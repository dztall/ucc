// Star Nest by Pablo Román Andrioli
// This content is under the MIT License.
// downloaded from https://www.shadertoy.com/view/XlfGRj

precision highp float;
varying float iTime;
varying vec2  iResolution;
varying vec2  iMouse;

#define iterations 17
#define formuparam 0.53

#define volsteps 20
#define stepsize 0.1

#define zoom   0.800
#define tile   0.850
#define speed  0.010

#define brightness 0.0015
#define darkmatter 0.300
#define distfading 0.730
#define saturation 0.850

void main(void)
{
    // get coords and direction
    vec2  uv    = gl_FragCoord.xy / iResolution.xy - 0.5;
    uv.y       *= iResolution.y   / iResolution.x;
    vec3  dir   = vec3(uv * zoom, 1.0);
    float time  = iTime * speed + 0.25;

    // mouse rotation
    float a1    = 0.5 + iMouse.x / iResolution.x * 2.0;
    float a2    = 0.8 + iMouse.y / iResolution.y * 2.0;
    mat2  rot1  = mat2(cos(a1), sin(a1), -sin(a1), cos(a1));
    mat2  rot2  = mat2(cos(a2), sin(a2), -sin(a2), cos(a2));
    dir.xz     *= rot1;
    dir.xy     *= rot2;
    vec3  from  = vec3(1.0, 0.5, 0.5);
    from       += vec3((iTime * 2.0 / 20.0), (iTime / 20.0), -(2.0 / 20.0));
    from.xz    *= rot1;
    from.xy    *= rot2;

    //volumetric rendering
    float s    = 0.1;
    float fade = 1.0;
    vec3  v    = vec3(0.0);

    for (int r = 0; r < volsteps; ++r)
    {
        vec3 p = from + s * dir * 0.5;

        // tiling fold
        p = abs(vec3(tile) - mod(p, vec3(tile * 2.0)));

        float pa, a = pa = 0.0;

        for (int i = 0; i < iterations; ++i)
        {
            // the magic formula
            p = abs(p) / dot(p,p) - formuparam;

            // absolute sum of average change
            a  += abs(length(p) - pa);
            pa  = length(p);
        }

        // dark matter
        float dm  = max(0.0, darkmatter - a * a * 0.001);

        // add contrast
        a *= a * a;

        // dark matter, don't render near
        if (r > 6)
            fade *= 1.0 - dm;

        //v += vec3(dm, dm * 0.5, 0.0);
        v += fade;

        // coloring based on distance
        v += vec3(s, s * s, s * s * s * s) * a * brightness * fade;

        // distance fading
        fade *= distfading;
        s    += stepsize;
    }

    // color adjust
    v            = mix(vec3(length(v)), v, saturation);
    gl_FragColor = vec4(v * 0.01, 1.0);
}
