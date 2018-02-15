// Wolfenstein. Created by Reinder Nijhoff 2013
// @reindernijhoff
//
// https://www.shadertoy.com/view/4sfGWX
//

#define NUM_MATERIALS 3
#define NUM_OBJECTS 1
#define SECONDS_IN_ROOM 3.0
#define ROOM_SIZE 10.0
#define MAXSTEPS 17
#define MATERIAL_DOOR 200
#define MATERIAL_DOORWAY 201

#define COL(r, g, b) vec3(r / 255.0, g / 255.0, b / 255.0)

#define time (iTime + 40.0)

precision highp float;
varying float iTime;
varying vec2  iResolution;
varying vec2  iMouse;

vec3 rdcenter;

//----------------------------------------------------------------------
// Math functions

float hash(const float n)
{
    return fract(sin(n * 14.1234512) * 51231.545341231);
}

float hash(const vec2 x)
{
    float n = dot(x, vec2(14.1432, 1131.15532));
    return fract(sin(n) * 51231.545341231);
}

float crossp(const vec2 a, const vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

vec3 rotate(vec3 r, float v)
{
    return vec3(r.x * cos(v) + r.z * sin(v), r.y, r.z * cos(v) - r.x * sin(v));
}

bool intersectSegment(const vec3 ro, const vec3 rd, const vec2 a, const vec2 b, out float dist, out float u)
{
    vec2  p       = ro.xz; vec2 r = rd.xz;
    vec2  q       = a-p;   vec2 s = b-a;
    float rCrossS = crossp(r, s);

    if (rCrossS == 0.0)
        return false;
    else
    {
        dist = crossp(q, s) / rCrossS;
        u    = crossp(q, r) / rCrossS;

        return (0.0 <= dist && 0.0 <= u && u <= 1.0);
    }
}

//----------------------------------------------------------------------
// Material helper functions

float onCircle(const vec2 c, const vec2 centre, const float radius)
{
    return clamp(4.0 * (radius - distance(c, centre)), 0.0, 1.0);
}

float onCircleLine(const vec2 c, const vec2 centre, const float radius)
{
    return clamp(1.0 - 1.5 * abs(radius - distance(c, centre)), 0.0, 1.0);
}

float onLine(const float c, const float b)
{
    return clamp(1.0 - abs(b-c), 0.0, 1.0);
}

float onBand(const float c, const float mi, const float ma)
{
    return clamp((ma - c + 1.0), 0.0, 1.0) * clamp((c - mi + 1.0), 0.0, 1.0);
}

float onLineSegmentX(const vec2 c, const float b, const float mi, const float ma)
{
    return onLine(c.x, b) * onBand(c.y, mi, ma);
}

float onLineSegmentY(const vec2 c, const float b, const float mi, const float ma)
{
    return onLine(c.y, b) * onBand(c.x, mi, ma);
}

float onRect(const vec2 c, const vec2 lt, const vec2 rb)
{
    return onBand(c.x, lt.x, rb.x) * onBand(c.y, lt.y, rb.y);
}

vec3 addBevel(const vec2 c, const vec2 lt, const vec2 rb, const float size, const float strength, const float lil, const float lit, const vec3 col)
{
    float xl = clamp((c.x - lt.x) / size, 0.0, 1.0);
    float xr = clamp((rb.x - c.x) / size, 0.0, 1.0);
    float yt = clamp((c.y - lt.y) / size, 0.0, 1.0);
    float yb = clamp((rb.y - c.y) / size, 0.0, 1.0);

    return mix(col, col * clamp(1.0 + strength * (lil * (xl - xr) + lit * (yb - yt)), 0.0, 2.0), onRect(c, lt, rb));
}

vec3 addKnob(const vec2 c, const vec2 centre, const float radius, const float strength, const vec3 col)
{
    vec2 lv = normalize(centre - c);
    return mix(col, col * (1.0 + strength * dot(lv, vec2(-0.7071, 0.7071))), onCircle(c, centre, radius));
}

float stepeq(float a, float b)
{
    return step(a, b) * step(b, a);
}

//----------------------------------------------------------------------
// Generate materials!

void getMaterialColor(const int material, in vec2 uv, const float decorationHash, out vec3 col)
{
    vec3 fgcol;

    uv       = floor(mod(uv + 64.0, vec2(64.0)));
    vec2 uvs = uv / 64.0;

    // basecolor
    vec3  basecol = vec3(mix(55.0 / 255.0, 84.0 / 255.0, uvs.y));
    float br      = hash(uv);
    col           = basecol;

    // grey bricks
    if (material == 0 || material == 1)
    {
        vec2  buv = vec2(mod(uv.x + 1.0 + (floor((uv.y + 1.0) / 16.0) * 16.0), 32.0), mod(uv.y + 1.0, 16.0));
        float bbr = mix(190.0 / 255.0, 91.0 / 255.0, (buv.y) / 14.0) + 0.05 * br;

        if (buv.x < 2.0 || buv.y < 2.0)
            bbr = 72.0 / 255.0;

        col = vec3(bbr * 0.95);
        col = addBevel(buv, vec2(1.0, 1.0), vec2(31.5, 15.0), 2.0, 0.35, 1.0, 1.0, col);

        // blue wall
        if (material == 1)
        {
            col *= 1.3 * COL(11.0, 50.0, 209.0);
            col  = mix(col, COL(2.0, 15.0, 86.0), onBand(uv.y, 14.0, 49.0));
            col  = mix(col, COL(9.0, 44.0, 185.0) * (0.9 + 0.1 * br), onBand(uv.y, 16.0, 47.0));
            col  = mix(col, COL(3.0, 25.0, 122.0), onBand(uv.y, 21.0, 42.0));
            col  = addBevel(uv, vec2(-1.0, 16.0), vec2(65.0, 21.0), 1.0, 0.35, 1.0, 1.0, col);
            col  = addBevel(uv, vec2(-1.0, 43.0), vec2(65.0, 48.0), 1.0, 0.35, 1.0, 1.0, col);

            col = mix(col, COL(2.0, 11.0, 74.0), onRect(uv, vec2(22.0, 22.0), vec2(42.0, 42.0)));
            col = mix(col, COL(9.0, 44.0, 185.0) * (0.95 + 0.1 * br), onRect(uv, vec2(22.0, 23.0), vec2(42.0, 40.0)));
            col = addBevel(uv, vec2(22.0, 23.0), vec2(42.0, 40.0), 1.0, 0.2, -1.0, 1.0, col);
            col = mix(col, mix(COL(2.0, 11.0, 74.0), COL(3.0, 25.0, 122.0), (uv.x - 26.0) / 3.0), onRect(uv, vec2(26.0, 23.0), vec2(29.0, 29.0)));
            col = mix(col, mix(COL(2.0, 11.0, 74.0), COL(3.0, 25.0, 122.0), (uv.y - 34.0) / 2.0), onRect(uv, vec2(22.0, 34.0), vec2(29.0, 36.0)));
            col = mix(col, mix(COL(2.0, 11.0, 74.0), COL(3.0, 25.0, 122.0), (uv.y - 27.0) / 2.0), onRect(uv, vec2(35.0, 27.0), vec2(42.0, 29.0)));
            col = mix(col, mix(COL(2.0, 11.0, 74.0), COL(3.0, 25.0, 122.0), (uv.y - 34.0) / 8.0), onRect(uv, vec2(35.0, 34.0), vec2(38.0, 42.0)));
        }
    }
    // wooden wall
    else
    if (material == 2)
    {
        float mx = mod(uv.x, 64.0 / 5.0);
        float h1 = hash(floor(uv.x / (64.0 / 5.0)));
        float h2 = hash(1.0 + 1431.16 * floor(uv.x / (64.0 / 5.0)));
        col      = mix(COL(115.0, 75.0, 43.0), COL(71.0, 56.0, 26.0), smoothstep(0.2, 1.0, (0.7 + h2) * abs(mod(h2 - uv.y * (0.05 + 0.1 * h2) + (1.0 + h1 + h2) * sin(mx * (0.1 + 0.2 * h2)), 2.0) - 1.0)));

        col = mix(col, mix(COL(40.0, 31.0, 13.0), COL(142.0, 91.0, 56.0), (uv.x)        / 2.0), step(uv.x, 2.0));
        col = mix(col, mix(COL(40.0, 31.0, 13.0), COL(142.0, 91.0, 56.0), (uv.x - 10.0) / 2.0), step(10.0, uv.x) * step(uv.x, 12.0));
        col = mix(col, mix(COL(40.0, 31.0, 13.0), COL(142.0, 91.0, 56.0), (uv.x - 26.0) / 2.0), step(26.0, uv.x) * step(uv.x, 28.0));
        col = mix(col, mix(COL(40.0, 31.0, 13.0), COL(142.0, 91.0, 56.0), (uv.x - 40.0) / 2.0), step(40.0, uv.x) * step(uv.x, 42.0));
        col = mix(col, mix(COL(40.0, 31.0, 13.0), COL(142.0, 91.0, 56.0), (uv.x - 54.0) / 2.0), step(54.0, uv.x) * step(uv.x, 56.0));

        col = mix(col, mix(COL(83.0, 60.0, 31.0), COL(142.0, 91.0, 56.0), (uv.x - 8.0)),  step( 8.0, uv.x) * step(uv.x, 9.0));
        col = mix(col, mix(COL(83.0, 60.0, 31.0), COL(142.0, 91.0, 56.0), (uv.x - 24.0)), step(24.0, uv.x) * step(uv.x, 25.0));
        col = mix(col, mix(COL(83.0, 60.0, 31.0), COL(142.0, 91.0, 56.0), (uv.x - 38.0)), step(38.0, uv.x) * step(uv.x, 39.0));
        col = mix(col, mix(COL(83.0, 60.0, 31.0), COL(142.0, 91.0, 56.0), (uv.x - 52.0)), step(52.0, uv.x) * step(uv.x, 53.0));
        col = mix(col, mix(COL(83.0, 60.0, 31.0), COL(142.0, 91.0, 56.0), (uv.x - 62.0)), step(62.0, uv.x));

        col  = mix(col, mix(COL(40.0, 31.0, 13.0), COL(142.0, 91.0, 56.0), (uv.y) / 2.0), step(uv.y, 2.0));
        col *= 1.0 - 0.3 * stepeq(uv.y, 3.0);
    }
    // door
    else
    if (material == MATERIAL_DOOR)
    {
        fgcol = COL(44.0, 176.0, 175.0) * (0.95 + 0.15 * sin(-0.25 + 4.0 * ((-0.9 - uvs.y) / (1.3 - 0.8 * uvs.x))));
        fgcol = addBevel(uv, vec2(-1.0, 1.0), vec2(62.0, 66.0), 2.0,  0.4, -1.0, -1.0, fgcol);
        fgcol = addBevel(uv, vec2( 6.0, 6.0), vec2(57.0, 57.0), 2.25, 0.5, -1.0, -1.0, fgcol);
        fgcol = mix(addKnob(mod(uv, vec2(8.0)), vec2(3.5), 1.65, 0.5, fgcol), fgcol, onRect(uv, vec2(6.0, 6.0), vec2(57.0, 57.0)));

        //knob
        fgcol *= 1.0 - 0.2 * onRect(uv, vec2(13.5, 28.5), vec2(22.5, 44.5));
        fgcol  = mix(fgcol, mix(COL(44.0, 44.0, 44.0), COL(152.0, 152.0, 152.0), ((uv.x + (43.0 - uv.y) - 15.0) / 25.0)), onRect(uv, vec2(15.0, 27.0), vec2(24.0, 43.0)));
        fgcol  = addBevel(uv, vec2(15.0, 27.0), vec2(24.0, 43.0), 1.0, 0.45, 1.0, 1.0, fgcol);
        fgcol  = mix(fgcol, addKnob(mod(uv, vec2(6.0)), vec2(4.25, 5.5), 1.15, 0.75, fgcol), onRect(uv, vec2(15.0, 27.0), vec2(24.0, 43.0))) ;

        fgcol *= 1.0 - 0.5 * onRect(uv, vec2(16.5, 33.5), vec2(20.5, 38.5));
        fgcol  = mix(fgcol, mix(COL(88.0, 84.0, 11.0), COL(251.0, 242.0, 53.0), ((uv.x + (37.0 - uv.y) - 18.0) / 7.0)), onRect(uv, vec2(18.0, 33.0), vec2(21.0, 37.0)));
        fgcol  = mix(fgcol, COL(0.0, 0.0, 0.0), onRect(uv, vec2(19.0, 34.0), vec2(20.0, 35.7)));

        fgcol *= 1.0 - 0.2 * onRect(uv, vec2(6.5, 29.5), vec2(10.5, 41.5));
        fgcol  = mix(fgcol, mix(COL(88.0, 84.0, 11.0), COL(251.0, 242.0, 53.0), ((uv.x + (40.0 - uv.y) - 9.0) / 13.0)), onRect(uv, vec2(9.0, 29.0), vec2(11.0, 40.0)));
        fgcol  = addBevel(uv, vec2(9.0, 29.0), vec2(11.0, 40.0), 0.75, 0.5, 1.0, 1.0, fgcol);

        col = mix(basecol, fgcol, onRect(uv, vec2(1.0, 1.0), vec2(62.0, 62.0)));
    }
    // doorway
    else
    if (material == MATERIAL_DOORWAY)
    {
        fgcol     = COL(44.0, 176.0, 175.0) * (0.95 + 0.15 * sin(-0.25 + 4.0 * ((-0.9 - uvs.y) / (1.3 - 0.8 * uvs.x))));
        vec2 uvhx = vec2(32.0 - abs(uv.x - 32.0), uv.y);
        fgcol     = addBevel(uvhx, vec2(-1.0, 1.0), vec2(28.0, 66.0), 2.0,  0.4, -1.0, -1.0, fgcol);
        fgcol     = addBevel(uvhx, vec2( 6.0, 6.0), vec2(23.0, 57.0), 2.25, 0.5, -1.0, -1.0, fgcol);
        fgcol     = mix(addKnob(vec2(mod(uvhx.x, 22.0), mod(uvhx.y, 28.0)), vec2(3.5), 1.65, 0.5, fgcol), fgcol, onRect(uvhx, vec2(6.0, 6.0), vec2(24.0, 57.0)));
        fgcol     = mix(fgcol, vec3(0.0), onRect(uv, vec2(29.0, 1.0), vec2(35.0, 63.0)));
        col       = mix(basecol, fgcol, onRect(uv, vec2(1.0, 1.0), vec2(62.0, 62.0)));
    }

    // prison door
    if (decorationHash > 0.93 && material < (NUM_MATERIALS + 1))
    {
        vec4 prisoncoords = vec4(12.0, 14.0, 52.0, 62.0);

        // shadow
        col *= 1.0 - 0.5 * onRect(uv, vec2(11.0, 13.0), vec2(53.0, 63.0));

        // hinge
        col = mix(col, COL(72.0,  72.0,  72.0),  stepeq(uv.x, 53.0) * step(mod(uv.y + 2.0, 25.0), 5.0) * step(13.0, uv.y));
        col = mix(col, COL(100.0, 100.0, 100.0), stepeq(uv.x, 53.0) * step(mod(uv.y + 1.0, 25.0), 3.0) * step(13.0, uv.y));

        vec3 pcol  = vec3(0.0) + COL(100.0, 100.0, 100.0) * step(mod(uv.x - 4.0, 7.0), 0.0);
        pcol      += COL(55.0, 55.0, 55.0) * step(mod(uv.x - 5.0, 7.0), 0.0);
        pcol       = addBevel(uv, vec2(0.0, 17.0), vec2(63.0, 70.0), 3.0, 0.8, 0.0, -1.0, pcol);
        pcol       = addBevel(uv, vec2(0.0, 45.0), vec2(22.0, 70.0), 3.0, 0.8, 0.0, -1.0, pcol);

        fgcol = COL(72.0, 72.0, 72.0);
        fgcol = addBevel(uv, prisoncoords.xy, prisoncoords.zw + vec2(1.0, 1.0), 1.0, 0.5, -1.0, 1.0, fgcol);
        fgcol = addBevel(uv, prisoncoords.xy + vec2(3.0, 3.0), prisoncoords.zw - vec2(2.0, 1.0), 1.0, 0.5, 1.0, -1.0, fgcol);
        fgcol = mix(fgcol, pcol, onRect(uv, prisoncoords.xy + vec2(3.0, 3.0), prisoncoords.zw - vec2(3.0, 2.0)));
        fgcol = mix(fgcol, COL(72.0, 72.0, 72.0), onRect(uv, vec2(15.0, 32.5), vec2(21.0, 44.0)));

        fgcol = mix(fgcol, mix(COL(0.0, 0.0, 0.0), COL(43.0, 43.0, 43.0), (uv.y - 37.0)), stepeq(uv.x, 15.0) * step(37.0, uv.y) * step(uv.y, 38.0));
        fgcol = mix(fgcol, mix(COL(0.0, 0.0, 0.0), COL(43.0, 43.0, 43.0), (uv.y - 37.0) / 3.0), stepeq(uv.x, 17.) * step(37.0, uv.y) * step(uv.y, 40.0));
        fgcol = mix(fgcol, COL(43.0, 43.0, 43.0),  stepeq(uv.x, 18.0) * step(37.0, uv.y) * step(uv.y, 41.0));
        fgcol = mix(fgcol, mix(COL(0.0, 0.0, 0.0), COL(100.0, 100.0, 100.0), (uv.y - 37.0) / 3.0), stepeq(uv.x, 18.0) * step(36.0, uv.y) * step(uv.y, 40.0));
        fgcol = mix(fgcol, COL(43.0, 43.0, 43.0),  stepeq(uv.x, 19.0) * step(37.0, uv.y) * step(uv.y, 40.0));

        fgcol = mix(fgcol, mix(COL(84.0, 84.0, 84.0), COL(108.0, 108.0, 108.0), (uv.x - 15.0) / 2.0), stepeq(uv.y, 32.0) * step(15.0, uv.x) * step(uv.x, 17.0));
        fgcol = mix(fgcol, COL(81.0, 81.0, 81.0), stepeq(uv.y, 32.0) * step(20.0, uv.x) * step(uv.x, 21.0));

        col = mix(col, fgcol, onRect(uv, prisoncoords.xy, prisoncoords.zw));
    }
    // flag
    else
    if (decorationHash > 0.63 && material < (NUM_MATERIALS + 1))
    {
        vec2 uvc = uv-vec2(32.0, 30.0);

        // shadow
        vec4 shadowcoords  = vec4(14.0, 7.0, 54.0, max(56.0 + sin(uv.x * 0.32 - 1.0), 56.0));
        col               *= 1.0 - 0.3 * onRect(uv, vec2(6.0, 6.0), vec2(61.0, 7.0));
        col               *= 1.0 - 0.3 * clamp(0.25 * (56.0 - uv.x), 0.0, 1.0) * onRect(uv, shadowcoords.xy, shadowcoords.zw);

        // rod
        col = mix(col, COL(250.0, 167.0, 98.0),  onLineSegmentX(vec2(abs(uv.x - 32.0), uv.y), 26.0, 4.0, 6.5));
        col = mix(col, COL(251.0, 242.0, 53.0),  onLineSegmentY(uv, 5.0, 4.0, 60.0));
        col = mix(col, COL(155.0, 76.0,  17.0),  onLineSegmentY(uv, 6.0, 4.0, 60.0));
        col = mix(col, COL(202.0, 96.0,  25.0),  onLineSegmentY(vec2(abs(uv.x - 32.0), uv.y), 6.0,  26.0, 28.0));
        col = mix(col, COL(251.0, 242.0, 53.0),  onLineSegmentX(vec2(abs(uv.x - 32.0), uv.y), 25.0, 3.0,  7.0));
        col = mix(col, COL(252.0, 252.0, 217.0), onLineSegmentX(vec2(abs(uv.x - 32.0), uv.y), 25.0, 4.3,  5.5));
        col = mix(col, COL(252.0, 252.0, 217.0), onLineSegmentX(vec2(abs(uv.x - 32.0), uv.y), 26.0, 5.3,  5.5));
        col = mix(col, COL(0.0,   0.0,   0.0),   onLineSegmentY(vec2(abs(uv.x - 32.0), uv.y), 6.0,  18.3, 19.5));

        // flag
        vec4 flagcoords = vec4(13.0, min(9.5 - pow(5.5 * (uvs.x - 0.5), 2.0), 9.0), 51.0, max(55.0 + sin(uv.x * 0.4 + 2.7), 55.0));

        fgcol = COL(249.0, 41.0, 27.0);

        fgcol = mix(fgcol, COL(255.0, 255.0, 255.0), onBand(min(abs(uvc.x), abs(uvc.y)), 2.0, 4.0));
        fgcol = mix(fgcol, COL(72.0, 72.0, 72.0), onLine(min(abs(uvc.x), abs(uvc.y)), 3.0));

        fgcol = mix(fgcol, COL(255.0, 255.0, 255.0), onCircle(uv, vec2(32.0, 30.0), 12.5));
        fgcol = mix(fgcol, COL(0.0, 0.0, 0.0), onCircleLine(uv, vec2(32.0, 30.0), 11.0));
        fgcol = mix(fgcol, COL(0.0, 0.0, 0.0), onCircleLine(uv, vec2(32.0, 30.0), 9.0));

        vec2 uvr = vec2((uvc.x - uvc.y) * 0.7071, (uvc.y + uvc.x) * 0.7071) * sign(uvc.x + 0.5);
        fgcol    = mix(fgcol, COL(72.0, 72.0, 72.0), onRect(uvr, vec2(-1.0, -1.0), vec2(1.0, 4.0)));
        fgcol    = mix(fgcol, COL(72.0, 72.0, 72.0), onRect(uvr, vec2(-4.2,  4.2), vec2(1.0, 6.15)));
        fgcol    = mix(fgcol, COL(72.0, 72.0, 72.0), onRect(uvr, vec2(-1.0, -1.0), vec2(4.0, 1.0)));
        fgcol    = mix(fgcol, COL(72.0, 72.0, 72.0), onRect(uvr, vec2( 4.2, -1.0), vec2(6.15, 4.2)));

        fgcol *= (0.8 + 0.2 * sin(uv.x * 0.4 + 2.7));
        fgcol *= (0.8 + 0.2 * clamp(0.5 * (uv.y - 7.0), 0.0, 1.0));

        // mix flag on background
        col = mix(col, fgcol, onRect(uv, flagcoords.xy, flagcoords.zw));
    }

    // fake 8-bit color palette and dithering
    col = floor((col + 0.5 * mod(uv.x + uv.y, 2.0) / 32.0) * 32.0) / 32.0;
}

bool getObjectColor(const int object, in vec2 uv, inout vec3 icol)
{
    uv        = floor(mod(uv, vec2(64.0)));
    vec2  uvs = uv / 64.0;
    vec3  col = vec3(20.0 / 255.0);
    float d;

    // only a lamp for now

    // lamp top
    d   = distance(uv * vec2(1.0, 2.0), vec2(28.1, 5.8) * vec2(1.0, 2.0));
    col = mix(col, mix(COL(41.0, 250.0, 46.0), COL(13.0, 99.0, 12.0), clamp(d / 8.0 - 0.2, 0.0, 1.0)),
              onCircle(uv, vec2(31.0, 13.6), 11.7) * step(uv.y, 6.0));
    col = mix(col, COL(9.0, 75.0, 6.0), onCircleLine(uv, vec2(31.0, 14.0), 11.6) *
              step(length(uv - vec2(31.0, 13.6)), 11.7) * step(uv.y, 6.0));
    col = mix(col, COL(100.0, 100.0, 100.0), onLine(abs(uv.x - 31.0), 1.0) * step(uv.y, 1.0));
    col = mix(col, COL(140.0, 140.0, 140.0), onLine(abs(uv.x - 31.0), 0.25) * step(uv.y, 1.0) * step(1.0, uv.y));

    // lamp bottom
    d   = distance(uv * vec2(1.0, 2.0), vec2(30.5, 6.5) * vec2(1.0, 2.0));
    col = mix(col, mix(COL(41.0, 250.0, 46.0), COL(13.0, 99.0, 12.0), clamp(abs(uv.x - 31.0) / 4.0 - 1.25, 0.0, 1.0)), step(abs(uv.x - 31.0), 9.0) * stepeq(uv.y, 7.0));
    col = mix(col, mix(COL(41.0, 250.0, 46.0), COL(16.0, 123.0, 17.0), clamp(abs(uv.x - 31.0) / 4.0 - 1.25, 0.0, 1.0)), step(abs(uv.x - 31.0), 9.0) * stepeq(uv.y, 8.0));
    col = mix(col, mix(COL(133.0, 250.0, 130.0), COL(22.0, 150.0, 23.0), clamp(abs(uv.x - 31.0) / 4.0 - 0.75, 0.0, 1.0)), step(abs(uv.x - 31.0), 7.0) * stepeq(uv.y, 9.0));

    col = mix(col, mix(COL(255.0, 251.0, 187.0), col, clamp(d / 4.5 - 0.6, 0.0, 1.0)),
              onCircle(uv, vec2(31.0, 1.0), 10.2) * step(uv.y, 8.0) * step(7.0, uv.y));
    col = mix(col, mix(COL(255.0, 255.0, 255.0), col, clamp(d / 4.0 - 0.7, 0.0, 1.0)),
              onCircle(uv, vec2(31.0, 1.0), 7.2) * step(uv.y, 8.0) * step(7.0, uv.y));

    // floor
    d   = distance(vec2(mod(uv.x, 32.0), uv.y) * vec2(1.5, 30.0 / 3.0), vec2(16.0, 61.5) * vec2(1.5, 30.0 / 3.0));
    col = mix(col, mix(COL(168.0, 168.0, 168.0), COL(124.0, 124.0, 124.0), clamp(d / 15.0 - 0.5, 0.0, 1.0)), step(d, 24.5));
    col = mix(col, mix(COL(124.0, 124.0, 124.0), COL(140.0, 140.0, 140.0), clamp((uv.y - 59.0) / 1.0, 0.0, 1.0)), step(59.0, uv.y) * step(uv.x, 57.0) * step(7.0, uv.x));
    col = mix(col, mix(COL(168.0, 168.0, 168.0), COL(124.0, 124.0, 124.0), clamp(abs(32.0 - uv.x) / 10.0 - 2.0, 0.0, 1.0)), step(uv.y, 62.0) * step(62.0, uv.y)*step(uv.x, 61.0) * step(3.0, uv.x));
    col = mix(col, mix(COL(152.0, 152.0, 152.0), COL(124.0, 124.0, 124.0), clamp(abs(32.0 - uv.x) / 10.0 - 2.25, 0.0, 1.0)), step(uv.y, 61.0) * step(61.0, uv.y)*step(uv.x, 59.0) * step(5.0, uv.x));

    col = floor((col) * 32.0) / 32.0;

    if (any(notEqual(col, vec3(floor((20.0 / 255.0) * 32.0) / 32.0))))
    {
        icol = col;
        return true;
    }

    return false;
}

//----------------------------------------------------------------------
// Proocedural MAP functions

bool isWall(const vec2 vos)
{
    return vos.y < 0.4 * ROOM_SIZE || vos.y > 2.75 * ROOM_SIZE || any(equal(mod(vos, vec2(ROOM_SIZE)), vec2(0.0, 0.0)));
}

bool isDoor(const vec2 vos)
{
    return isWall(vos) && ((hash(vos) > 0.75 &&  any(equal(mod(vos, vec2(ROOM_SIZE * 0.5)), vec2(2.0))))
            || any(equal(mod(vos, vec2(ROOM_SIZE)), vec2(ROOM_SIZE * 0.5))));
}

bool isObject(const vec2 vos)
{
    return hash(vos * 10.0) > 0.95;
}

bool map(const vec2 vos)
{
    return isObject(vos) || isWall(vos);
}

//----------------------------------------------------------------------
// Render MAP functions

bool intersectSprite(const vec3 ro, const vec3 rd, const vec3 vos, const vec3 nor, out vec2 uv)
{
    float dist, u;
    vec2 a = vos.xz + nor.zx * vec2(-0.5, 0.5) + vec2(0.5, 0.5);
    vec2 b = vos.xz - nor.zx * vec2(-0.5, 0.5) + vec2(0.5, 0.5);

    if (intersectSegment(ro, rd, a, b, dist, u))
    {
        uv.x = u; uv.y = 1.0 - (ro + dist * rd).y;

        if (sign(nor.x) < 0.0)
            uv.x = 1.0 - uv.x;

        return uv.y > 0.0 && uv.y < 1.0;
    }

    return false;
}

int getMaterialId( const vec2 vos )
{
    return int(mod(521.21 * hash(floor((vos - vec2(0.5)) / ROOM_SIZE)), float(NUM_MATERIALS)));
}

bool getColorForPosition(const vec3 ro, const vec3 rd, const vec3 vos, const vec3 pos, const vec3 nor, inout vec3 col)
{
    vec2 uv;

    if (isWall(vos.xz))
    {
        if (isDoor(vos.xz))
        {
            if (intersectSprite(ro, rd, vos + nor * 0.03, nor, uv))
            {
                // open the door
                uv.x -= clamp(2.0 - 0.75 * distance(ro.xz, vos.xz + vec2(0.5)), 0.0, 1.0);

                if (uv.x > 0.0)
                {
                    getMaterialColor(MATERIAL_DOOR, uv * 64.0, 0.0, col);
                    return true;
                }
            }

            return false;
        }

        // a wall is hit
        if (pos.y <= 1.0 && pos.y >= 0.0)
        {
            vec2  mpos = vec2(dot(vec3(-nor.z, 0.0, nor.x), pos), -pos.y);
            float sha  = 0.6 + 0.4 * abs(nor.z);
            getMaterialColor(isDoor(vos.xz + nor.xz) ? MATERIAL_DOORWAY : getMaterialId(vos.xz), mpos * 64.0, hash(vos.xz), col);
            col *= sha;
            return true;
        }

        return true;
    }

    if (isObject(vos.xz) && !isWall(vos.xz + vec2(1.0 ,0.0)) && !isWall(vos.xz + vec2(-1.0 ,0.0))
            && !isWall(vos.xz + vec2(0.0, -1.0)) && !isWall(vos.xz + vec2(0.0, 1.0))
            && intersectSprite(ro, rd, vos, rdcenter, uv))
        return getObjectColor(0, uv * 64.0, col);

    return false;
}

bool castRay(const vec3 ro, const vec3 rd, inout vec3 col)
{
    vec3 pos = floor(ro);
    vec3 ri  = 1.0 / rd;
    vec3 rs  = sign(rd);
    vec3 dis = (pos - ro + 0.5 + rs * 0.5) * ri;

    float res = 0.0;
    vec3  mm  = vec3(0.0);
    bool  hit = false;

    for (int i = 0; i < MAXSTEPS; ++i)
    {
        if (hit)
            continue;

        mm   = step(dis.xyz, dis.zyx);
        dis += mm * rs * ri;
        pos += mm * rs;

        if (map(pos.xz))
        {
            vec3  mini = (pos - ro + 0.5 - 0.5 * vec3(rs)) * ri;
            float t    = max (mini.x, mini.z);
            hit        = getColorForPosition(ro, rd, pos, ro + rd * t, -mm * sign(rd), col);
        }
    }

    return hit;
}

//----------------------------------------------------------------------
// Some really ugly code

#define CCOS(a) cos(clamp(a, 0.0, 1.0) * 1.57079632679)
#define CSIN(a) sin(clamp(a, 0.0, 1.0) * 1.57079632679)

vec3 path(const float t)
{
    float tmod   = mod(t / SECONDS_IN_ROOM, 8.0);
    float tfloor = floor(tmod);

    vec3 pos = vec3(4.0 * ROOM_SIZE * floor(t / (SECONDS_IN_ROOM * 8.0)) + 0.5, 0.5, 0.5 * ROOM_SIZE + 0.5);

    return pos + ROOM_SIZE *
            vec3(clamp(tmod, 0.0, 1.0) + clamp(tmod - 4.0, 0.0, 1.0) + 0.5 * (2.0 + CSIN(tmod - 1.0) - CCOS(tmod - 3.0) + CSIN(tmod - 5.0) - CCOS(tmod - 7.0)), 0.0,
                 clamp(tmod - 2.0, 0.0, 1.0) - clamp(tmod - 6.0, 0.0, 1.0) + 0.5 * (-CCOS(tmod - 1.0) + CSIN(tmod - 3.0) + CCOS(tmod - 5.0) - CSIN(tmod - 7.0)));
}

//----------------------------------------------------------------------
// Main

void main(void)
{
    vec2 q  = gl_FragCoord.xy / iResolution.xy;
    vec2 p  = -1.0 + 2.0 * q;
    p.x    *= iResolution.x/ iResolution.y;

    vec3 ro = path(time);
    vec3 ta = path(time + 0.1);

    rdcenter = rotate(normalize(ta - ro), 0.3 * cos(time * 0.75));
    vec3 uu  = normalize(cross(vec3(0.0, 1.0, 0.0), rdcenter));
    vec3 vv  = normalize(cross(rdcenter, uu));
    vec3 rd  = normalize(p.x * uu + p.y * vv + 2.5 * rdcenter);

    vec3 col = rd.y > 0.0 ? vec3(56.0 / 255.0) : vec3(112.0 / 255.0);
    castRay(ro, rd, col);

    gl_FragColor = vec4(col, 1.0);
}
