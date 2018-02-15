// "The Drive Home" by Martijn Steinrucken aka BigWings - 2017
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// I was looking for something 3d, that can be made just with a point-line distance function.
// Then I saw the cover graphic of the song I'm using here on soundcloud, which is a bokeh traffic
// shot which is a perfect for for what I was looking for.
//
// It took me a while to get to a satisfying rain effect. Most other people use a render buffer for
// this so that is how I started. In the end though, I got a better effect without. Uncomment the
// DROP_DEBUG define to get a better idea of what is going on.
//
// If you are watching this on a weaker device, you can uncomment the HIGH_QUALITY define
//
// Music:
// Mr. Bill - Cheyah (Zefora's digital rain remix) 
// https://soundcloud.com/zefora/cheyah
//
// Video can be found here:
// https://www.youtube.com/watch?v=WrxZ4AZPdOQ
//
// Making of tutorial:
// https://www.youtube.com/watch?v=eKtsY7hYTPg
//
// downloaded from https://www.shadertoy.com/view/MdfBRX
//

#define S(x, y, z)       smoothstep(x, y, z)
#define B(a, b, edge, t) S(a - edge, a + edge, t) * S(b + edge, b - edge, t)
#define sat(x)           clamp(x, 0.0, 1.0)

#define streetLightCol vec3(1.0, 0.7, 0.3)
#define headLightCol   vec3(0.8, 0.8, 1.0)
#define tailLightCol   vec3(1.0, 0.1, 0.1)

#define HIGH_QUALITY
#define CAM_SHAKE 1.0
#define LANE_BIAS 0.5
#define RAIN
//#define DROP_DEBUG

precision highp float;
varying float iTime;
varying vec2  iResolution;
varying vec2  iMouse;

vec3 ro, rd;

float N(float t)
{
    return fract(sin(t * 10234.324) * 123423.23512);
}

vec3 N31(float p)
{
    //  3 out, 1 in... DAVE HOSKINS
   vec3 p3  = fract(vec3(p) * vec3(0.1031, 0.11369, 0.13787));
   p3      += dot(p3, p3.yzx + 19.19);
   return fract(vec3((p3.x + p3.y) * p3.z, (p3.x+p3.z) * p3.y, (p3.y+p3.z) * p3.x));
}

float N2(vec2 p)
{
    // Dave Hoskins - https://www.shadertoy.com/view/4djSRW
    vec3 p3  = fract(vec3(p.xyx) * vec3(443.897, 441.423, 437.195));
    p3      += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float DistLine(vec3 ro, vec3 rd, vec3 p)
{
    return length(cross(p - ro, rd));
}

vec3 ClosestPoint(vec3 ro, vec3 rd, vec3 p)
{
    // returns the closest point on ray r to point p
    return ro + max(0.0, dot(p - ro, rd)) * rd;
}

float Remap(float a, float b, float c, float d, float t)
{
    return ((t - a) / (b - a)) * (d - c) + c;
}

float BokehMask(vec3 ro, vec3 rd, vec3 p, float size, float blur)
{
    float d = DistLine(ro, rd, p);
    float m = S(size, size * (1.0 - blur), d);

    #ifdef HIGH_QUALITY
        m *= mix(0.7, 1.0, S(0.8 * size, size, d));
    #endif

    return m;
}

float SawTooth(float t)
{
    return cos(t + cos(t)) + sin(2.0 * t) * 0.2 + sin(4.0 * t) * 0.02;
}

float DeltaSawTooth(float t)
{
    return 0.4 * cos(2.0 * t) + 0.08 * cos(4.0 * t) - (1.0 - sin(t)) * sin(t + cos(t));
}

vec2 GetDrops(vec2 uv, float seed, float m)
{
    float t = iTime + m * 30.0;
    vec2  o = vec2(0.0);

    #ifndef DROP_DEBUG
        uv.y += t * 0.05;
    #endif

    uv      *= vec2(10.0, 2.5) * 2.0;
    vec2 id  = floor(uv);
    vec3 n   = N31(id.x + (id.y + seed) * 546.3524);
    vec2 bd  = fract(uv);

    vec2 uv2 = bd;

    bd -= 0.5;

    bd.y *= 4.0;

    bd.x += (n.x - 0.5) * 0.6;

    t           += n.z * 6.28;
    float slide  = SawTooth(t);

    float ts       = 1.5;
    vec2  trailPos = vec2(bd.x * ts, (fract(bd.y * ts * 2.0 - t * 2.0) - 0.5) * 0.5);

    // make drops slide down
    bd.y += slide * 2.0;

    #ifdef HIGH_QUALITY
        float dropShape  = bd.x * bd.x;
        dropShape       *= DeltaSawTooth(t);

        // change shape of drop when it is falling
        bd.y += dropShape;
    #endif

    // distance to main drop
    float d = length(bd);

    // mask out drops that are below the main
    float trailMask = S(-0.2, 0.2, bd.y);

    // fade dropsize
    trailMask *= bd.y;

    // distance to trail drops
    float td = length(trailPos * max(0.5, trailMask));

    float mainDrop  = S(0.2, 0.1, d);
    float dropTrail = S(0.1, 0.02, td);

    dropTrail *= trailMask;

    // mix main drop and drop trail
    o = mix(bd*mainDrop, trailPos, dropTrail);

    #ifdef DROP_DEBUG
        if (uv2.x < 0.02 || uv2.y < 0.01)
            o = vec2(1.0);
    #endif

    return o;
}

void CameraSetup(vec2 uv, vec3 pos, vec3 lookat, float zoom, float m)
{
    ro      = pos;
    vec3  f = normalize(lookat - ro);
    vec3  r = cross(vec3(0.0, 1.0, 0.0), f);
    vec3  u = cross(f, r);
    float t = iTime;

    vec2 offs = vec2(0.0);

    #ifdef RAIN
        vec2 dropUv = uv;

        #ifdef HIGH_QUALITY
            float x = (sin(t * 0.1) * 0.5 + 0.5) * 0.5;
            x       = -x * x;
            float s = sin(x);
            float c = cos(x);

            mat2 rot = mat2(c, -s, s, c);

            #ifndef DROP_DEBUG
                dropUv    =  uv * rot;
                dropUv.x += -sin(t * 0.1) * 0.5;
            #endif
        #endif

        offs = GetDrops(dropUv, 1.0, m);

        #ifndef DROP_DEBUG
            offs += GetDrops(dropUv * 1.4, 10.0, m);

            #ifdef HIGH_QUALITY
                offs += GetDrops(dropUv * 2.4, 25.0, m);
                //offs += GetDrops(dropUv * 3.4, 11.0);
                //offs += GetDrops(dropUv * 3.0, 2.0);
            #endif

            float ripple  = sin(t + uv.y * 3.1415 * 30.0 + uv.x * 124.0) * 0.5 + 0.5;
            ripple       *= 0.005;
            offs         += vec2(ripple * ripple, ripple);
        #endif
    #endif

    vec3 center = ro + f * zoom;
    vec3 i      = center + (uv.x - offs.x) * r + (uv.y - offs.y) * u;

    rd = normalize(i - ro);
}

vec3 HeadLights(float i, float t)
{
    float z = fract(-t * 2.0 + i);
    vec3  p = vec3(-0.3, 0.1, z * 40.0);
    float d = length(p - ro);

    float size  = mix(0.03, 0.05, S(0.02, 0.07, z)) * d;
    float m     = 0.0;
    float blur  = 0.1;
    m          += BokehMask(ro, rd, p - vec3(0.08, 0.0, 0.0), size, blur);
    m          += BokehMask(ro, rd, p + vec3(0.08, 0.0, 0.0), size, blur);

    #ifdef HIGH_QUALITY
        m += BokehMask(ro, rd, p + vec3(0.1, 0.0, 0.0), size, blur);
        m += BokehMask(ro, rd, p - vec3(0.1, 0.0, 0.0), size, blur);
    #endif

    float distFade = max(0.01, pow(1.0 - z, 9.0));

    blur     = 0.8;
    size    *= 2.5;
    float r  = 0.0;
    r       += BokehMask(ro, rd, p + vec3(-0.09, -0.2, 0.0), size, blur);
    r       += BokehMask(ro, rd, p + vec3( 0.09, -0.2, 0.0), size, blur);
    r       *= distFade * distFade;

    return headLightCol * (m + r) * distFade;
}

vec3 TailLights(float i, float t)
{
    t = t * 1.5 + i;

    float id = floor(t) + i;
    vec3  n  = N31(id);

    float laneId = S(LANE_BIAS, LANE_BIAS + 0.01, n.y);

    float ft = fract(t);

    // distance ahead
    float z = 3.0 - ft * 3.0;

    // get out of the way!
    laneId     *= S(0.2, 1.5, z);
    float lane  = mix(0.6, 0.3, laneId);
    vec3  p     = vec3(lane, 0.1, z);
    float d     = length(p - ro);

    float size = 0.05 * d;
    float blur = 0.1;
    float m    = BokehMask(ro, rd, p - vec3(0.08, 0.0, 0.0), size, blur) +
                 BokehMask(ro, rd, p + vec3(0.08, 0.0, 0.0), size, blur);

    #ifdef HIGH_QUALITY
        // start braking at random distance
        float bs    = n.z * 3.0;
        float brake = S(bs, bs+.01, z);

        // n.y = random brake duration
        brake *= S(bs + 0.01, bs, z - 0.5 * n.y);

        m += (BokehMask(ro, rd, p + vec3(0.1, 0.0, 0.0), size, blur) +
              BokehMask(ro, rd, p - vec3(0.1, 0.0, 0.0), size, blur)) * brake;
    #endif

    float refSize  = size * 2.5;
    m             += BokehMask(ro, rd, p + vec3(-0.09, -0.2, 0.0), refSize, 0.8);
    m             += BokehMask(ro, rd, p + vec3( 0.09, -0.2, 0.0), refSize, 0.8);
    vec3  col      = tailLightCol * m * ft;

    float b  = BokehMask(ro, rd, p+vec3(0.12,  0.0, 0.0), size, blur);
    b       += BokehMask(ro, rd, p+vec3(0.12, -0.2, 0.0), refSize, 0.8) * 0.2;

    vec3 blinker  = vec3(1.0, 0.7, 0.2);
    blinker      *= S(1.5, 1.4, z) * S(0.2, 0.3, z);
    blinker      *= sat(sin(t * 200.0) * 100.0);
    blinker      *= laneId;
    col          += blinker * b;

    return col;
}

vec3 StreetLights(float i, float t)
{
    float side      = sign(rd.x);
    float offset    = max(side, 0.0) * (1.0 / 16.0);
    float z         = fract(i - t + offset);
    vec3  p         = vec3(2.0 * side, 2.0, z * 60.0);
    float d         = length(p - ro);
    float blur      = 0.1;
    vec3  rp        = ClosestPoint(ro, rd, p);
    float distFade  = Remap(1.0, 0.7, 0.1, 1.5, 1.0 - pow(1.0 -z, 6.0));
    distFade       *= (1.0 - z);
    float m = BokehMask(ro, rd, p, 0.05 * d, blur) * distFade;

    return m * streetLightCol;
}

vec3 EnvironmentLights(float i, float t)
{
    float n = N(i + floor(t));

    float side      = sign(rd.x);
    float offset    = max(side, 0.0) * (1.0 / 16.0);
    float z         = fract(i - t + offset + fract(n * 234.0));
    float n2        = fract(n * 100.0);
    vec3  p         = vec3((3.0 + n) * side, n2 * n2 * n2 * 1.0, z * 60.0);
    float d         = length(p - ro);
    float blur      = 0.1;
    vec3  rp        = ClosestPoint(ro, rd, p);
    float distFade  = Remap(1.0, 0.7, 0.1, 1.5, 1.0 - pow(1.0 - z, 6.0));
    float m         = BokehMask(ro, rd, p, 0.05 * d, blur);
    m              *= distFade * distFade * 0.5;

    m              *= 1.0 - pow(sin(z * 6.28 * 20.0 * n) * 0.5 + 0.5, 20.0);
    vec3 randomCol  = vec3(fract(n * -34.5), fract(n * 4572.0), fract(n * 1264.0));
    vec3 col        = mix(tailLightCol, streetLightCol, fract(n * -65.42));
    col             = mix(col, randomCol, n);
    return m * col * 0.2;
}

void main(void)
{
    float t   = iTime;
    vec3  col = vec3(0.0);
    vec2  uv  = gl_FragCoord.xy / iResolution.xy; // 0 <> 1

    uv   -= 0.5;
    uv.x *= iResolution.x / iResolution.y;

    vec2 mouse = iMouse.xy / iResolution.xy;

    vec3 pos = vec3(0.3, 0.15, 0.0);

    float bt    = t * 5.0;
    float h1    = N(floor(bt));
    float h2    = N(floor(bt + 1.0));
    float bumps = mix(h1, h2, fract(bt))*.1;
    bumps       = bumps * bumps * bumps * CAM_SHAKE;

    pos.y         += bumps;
    float lookatY  = pos.y + bumps;
    vec3  lookat   = vec3(0.3, lookatY, 1.0);
    vec3  lookat2  = vec3(0.0, lookatY, 0.7);
    lookat         = mix(lookat, lookat2, sin(t * 0.1) * 0.5 + 0.5);

    uv.y += bumps * 4.0;
    CameraSetup(uv, pos, lookat, 2.0, mouse.x);

    t *= 0.03;
    t += mouse.x;

    // fix for GLES devices by MacroMachines
    #ifdef GL_ES
        const float stp = 1.0 / 8.0;
    #else
        float stp = 1.0 / 8.0;
    #endif

    for (float i = 0.0; i < 1.0; i += stp)
       col += StreetLights(i, t);

    for (float i = 0.0; i < 1.0; i += stp)
    {
        float n  = N(i + floor(t));
        col     += HeadLights(i + n * stp * 0.7, t);
    }

    #ifndef GL_ES
        #ifdef HIGH_QUALITY
            stp = 1.0 / 32.0;
        #else
            stp = 1.0 / 16.0;
        #endif
    #endif

    for (float i = 0.0; i < 1.0; i += stp)
       col += EnvironmentLights(i, t);

    col += TailLights(0.0, t);
    col += TailLights(0.5, t);

    col += sat(rd.y) * vec3(0.6, 0.5, 0.9);

    gl_FragColor = vec4(col, 0.0);
}
