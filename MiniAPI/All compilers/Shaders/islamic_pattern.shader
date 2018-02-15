// Author: Rigel rui@gil.com
// licence: https://creativecommons.org/licenses/by/4.0/
// link: https://www.shadertoy.com/view/MlsfRX

/*
    A study on symmetry. Took this pattern as a challenge and then tried to colorize it.
    https://patterninislamicart.com/drawings-diagrams-analyses/5/geometric-patterns-borders/gpb033

    The plane has four kind of symmetric operations
    - translations
    - rotations
    - reflections
    - glide reflections

    This pattern only have the first three.
    Everything is made of two simple rectangles, with translations, rotations, and
    reflections.
*/

precision mediump float;
varying float iTime;
varying vec2  iResolution;

#define TAU 6.2831

// cartesian to polar coordinates
vec2 toPolar(vec2 uv)
{
    return vec2(length(uv), atan(uv.y, uv.x));
}

// polar to cartesian coordinates
vec2 toCarte(vec2 z)
{
    return z.x * vec2(cos(z.y), sin(z.y));
}

// 2d rotation matrix
mat2 uvRotate(float a)
{
    return mat2(cos(a), sin(a), -sin(a), cos(a));
}

// a signed distance function for a rectangle
float sdfRect(vec2 uv, vec2 s)
{
    vec2 auv = abs(uv);
    return max(auv.x - s.x, auv.y - s.y);
}

// to fill an sdf with 0's or 1's
float fill(float d, float i)
{
    return abs(smoothstep(0.0, 0.02, d) - i);
}

// palette from iq -> https://www.shadertoy.com/view/ll2GD3
vec3 pal(float d)
{
    return 0.5 + 0.5 * cos(TAU * (d + vec3(0.0, 0.10, 0.20)));
}

// this makes a symmetric rotation around the origin.
// n is the number of slices, and everything is remmapped to the first one.
vec2 symrot(vec2 uv, float n)
{
    vec2 z = toPolar(uv);
    return toCarte(vec2(z.x, mod(z.y, TAU / n) - TAU/(n * 2.0)));
}

// this is the fundamental pattern where everything in the plane is remapped
// with symmetric operations
vec3 pattern(vec2 uv)
{
    // the coordinates for the two rectangles
    vec2 uv1 = uv * uvRotate(radians( 30.0));
    vec2 uv2 = uv * uvRotate(radians(-30.0));

    // the signed distance functions
    float sdfr1 = sdfRect(uv1, vec2(0.1, 0.7));
    float sdfr2 = sdfRect(uv2, vec2(0.1, 0.7));

    // a fill to keep track of their areas and masks
    float r1 = fill(sdfr1, 1.0);
    float r2 = fill(sdfr2, 1.0);

    float r1mask = 1.0 - r1;
    float r2mask = 1.0 - r2;

    // two waves, they are nothing more than the difference between two sine waves
    float wave1 = r1 * max(fill(0.05 * sin((uv1.y + 0.5) * TAU + 1.57) - uv1.x, 0.0),
                           fill(uv1.x - 0.05 * sin((uv1.y + 0.5) * TAU), 0.0));
    float wave2 = r1mask * r2 * max(fill(0.05 * sin((uv2.y + 0.5) * TAU + 1.57) - uv2.x, 0.0),
                                    fill(uv2.x - 0.05 * sin((uv2.y + 0.5) * TAU), 0.0));

    // the background
    vec3 bg = pal(0.5 - uv.y * 0.1);

    // three circles to make the center flower
    float circle = length(uv - vec2(0.0, 0.4));
    bg           =  mix(bg, pal(0.0), smoothstep(0.4,  0.0,circle));
    bg           =  mix(bg, pal(0.5), smoothstep(0.11, 0.0,circle));
    bg           =  mix(bg, pal(0.9), smoothstep(0.02, 0.0,circle));

    // composing the rectangles and the waves to set up the foreground
    float d = max(min(max(r1mask * r2, r1), wave1), wave2);

    // colorizing the foreground
    vec3 fg = mix(pal(0.9 - uv.y * 2.0), pal(0.15 + uv.y * 0.1), d);

    // adding a black contour to the rectangles
    fg = mix(fg, vec3(0.0), max(r1mask * fill(abs(sdfr2), 1.0), fill(abs(sdfr1), 1.0)));

    // adding a faux 3d to the interlace of the rectangles
    fg = mix(fg, fg * 0.4, r2 * smoothstep(0.0, 0.01, sdfr1) - smoothstep(0.0, 0.1, sdfr1));

    // return foreground and background
    return mix(fg, bg, min(r1mask, r2mask));
}

// from Shane -> https://www.shadertoy.com/view/llSyDh
// I've removed the comments. Go to Shane shader to see them.
// This provides the translation symmetry, remaps everything in the plane to
// an hexagon centered at [0,0]
vec2 lattice6(vec2 uv)
{
    const vec2 s = vec2(1, 1.7320508);

    vec4 hC = floor(vec4(uv, uv - vec2(0.5, 1)) / s.xyxy) + 0.5;
    vec4 h  = vec4(uv - hC.xy * s, uv - (hC.zw + 0.5) * s);

    return dot(h.xy, h.xy) < dot(h.zw, h.zw) ? vec2(h.xy) : vec2(h.zw);
}

// The scene is just symmetry operations
vec3 scene(vec2 uv)
{
    // translation symmetry
    uv = lattice6(uv) * 6.0;

    // a small alignement because the lattice is pointy hexagon side up
    // and my pattern is flat topped.
    uv *= uvRotate(radians(30.0));

    // 6 fold rotations
    uv = symrot(uv, 6.0) - vec2(2.0, 0.0); // 4

    // 3 fold rotation
    uv = symrot(uv, 3.0) - vec2(1.0, 0.0); // 3

    // 3 fold rotation
    uv = symrot(uv, 3.0) - vec2(0.5, 0.0); // 2

    // reflection on the y axis with a flip on the x to do an interlace
    uv = vec2(sign(uv.y) * uv.x, abs(uv.y)) - vec2(0.0, 0.4 + 0.05 * cos(iTime + uv.x * 6.28)); // 1

    // if you want to see how the pattern is constructed
    // comment all the lines 1 to 4, and then uncomment one by one 1, 2, 3, 4

    // draw the pattern
    return pattern(uv);
}

void main(void)
{
    vec2 uv      = (gl_FragCoord.xy - iResolution.xy * 0.5)/ iResolution.y;
    gl_FragColor = vec4(scene(uv), 1.0);

    // uncomment to see the original pattern
    //fragColor = vec4(pattern(uv * 2.0), 1.0);
}
