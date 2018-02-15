// M.A.M. Stairs by Leon Denise aka ponk
// another raymarching sketch inspired by Marc-Antoine Mathieu.
// using code from IQ, Mercury, LJ, Duke, Koltes
// made with Atom Editor GLSL viewer (that's why there is 2 space tabulations)
// 2017-11-24
// downloaded from https://www.shadertoy.com/view/MllBR7

precision highp float;
varying float iTime;
varying vec2  iResolution;
varying vec2  iMouse;

#define STEPS 50.0
#define VOLUME 0.01
#define PI 3.14159
#define TAU (2.0 * PI)
#define time iTime
#define repeat(v, c) (mod(v, c) - c / 2.0)
#define sDist(v, r) (length(v)-r)

mat2 rot(float a)
{
    float c = cos(a), s = sin(a);
    return mat2(c, -s, s, c);
}

float rng(vec2 seed)
{
    return fract(sin(dot(seed * 0.1684, vec2(32.649, 321.547))) * 43415.0);
}

float sdBox(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float amod(inout vec2 p, float count)
{
    float an = TAU / count;
    float a  = atan(p.y, p.x) + an / 2.0;
    float c  = floor(a / an);
    c        = mix(c, abs(c), step(count * 0.5, abs(c)));
    a        = mod(a, an) - an / 2.0;
    p.xy     = vec2(cos(a), sin(a)) * length(p);
    return c;
}

float aindex(vec2 p, float count)
{
    float an = TAU / count;
    float a  = atan(p.y, p.x) + an / 2.0;
    float c  = floor(a / an);
    return mix(c, abs(c), step(count * 0.5, abs(c)));
}

float map(vec3);

vec3 getNormal(vec3 p)
{
    vec2 e = vec2(0.001, 0);
    return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy), map(p + e.yyx) - map(p - e.yyx)));
}

float hardShadow(vec3 pos, vec3 light)
{
    vec3  dir  = normalize(light - pos);
    float maxt = length(light - pos);
    float t    = 0.02;

    for (float i = 0.0; i <= 1.0; i += 1.0 / 30.0)
    {
        float dist = map(pos + dir * t);

        if (dist < VOLUME)
            return 0.0;

        t += dist;

        if (t >= maxt)
            break;
    }

    return 1.0;
}

float map(vec3 pos)
{
    float scene       = 1000.0;
    float wallThin    = 0.2;
    float wallRadius  = 8.0;
    float wallOffset  = 0.2;
    float wallCount   = 10.0;
    float floorThin   = 0.1;
    float stairRadius = 5.0;
    float stairHeight = 0.4;
    float stairCount  = 40.0;
    float stairDepth  = 0.31;
    float bookCount   = 100.0;
    float bookRadius  = 9.5;
    float bookSpace   = 1.75;
    vec3 bookSize     = vec3(1.0, 0.2, 0.2);
    vec3 panelSize    = vec3(0.03, 0.2, 0.7);
    vec2 cell         = vec2(1.4, 3.0);
    float paperRadius = 4.0;
    vec3 paperSize    = vec3(0.3, 0.01, 0.4);
    vec3 p;

    // move it
    pos.y += time;

    // twist it
    //pos.xz *= rot(pos.y * 0.05 + time * 0.1);
    //pos.xz += normalize(pos.xz) * sin(pos.y * 0.5 + time);

    // holes
    float holeWall  = sDist(pos.xz, wallRadius);
    float holeStair = sDist(pos.xz, stairRadius);

    // walls
    p      = pos;
    amod(p.xz, wallCount);
    p.x   -= wallRadius;
    scene  = min(scene, max(-p.x, abs(p.z) - wallThin));
    scene  = max(scene, -sDist(pos.xz, wallRadius - wallOffset));

    // floors
    p          = pos;
    p.y        = repeat(p.y, cell.y);
    float disk = max(sDist(p.xz, 1000.0), abs(p.y) - floorThin);
    disk       = max(disk, -sDist(pos.xz, wallRadius));
    scene      = min(scene, disk);

    // stairs
    p                 = pos;
    float stairIndex  = amod(p.xz, stairCount);
    p.y              -= stairIndex * stairHeight;
    p.y               = repeat(p.y, stairCount * stairHeight);
    float stair       = sdBox(p, vec3(100, stairHeight, stairDepth));
    scene             = min(scene, max(stair, max(holeWall, -holeStair)));
    p                 = pos;
    p.xz             *= rot(PI / stairCount);
    stairIndex        = amod(p.xz, stairCount);
    p.y              -= stairIndex * stairHeight;
    p.y               = repeat(p.y, stairCount*stairHeight);
    stair             = sdBox(p, vec3(100, stairHeight, stairDepth));
    scene             = min(scene, max(stair, max(holeWall, -holeStair)));
    p                 = pos;
    p.y              += stairHeight * 0.5;
    p.y              -= stairHeight * stairCount * atan(p.z, p.x) / TAU;
    p.y               = repeat(p.y, stairCount*stairHeight);
    scene             = min(scene, max(max(sDist(p.xz, wallRadius), abs(p.y) - stairHeight), -holeStair));

    // books
    p           = pos;
    p.y        -= cell.y * 0.5;
    vec2 seed   = vec2(floor(p.y / cell.y), 0);
    p.y         = repeat(p.y, cell.y);
    p.xz       *= rot(PI / wallCount);
    seed.y     += amod(p.xz, wallCount) / 10.0;
    seed.y     += floor(p.z / (bookSize.z * bookSpace));
    p.z         = repeat(p.z, bookSize.z * bookSpace);
    float salt  = rng(seed);
    bookSize.x *= 0.5 + 0.5 * salt;
    bookSize.y += salt;
    bookSize.z *= 0.5 + 0.5 * salt;
    p.x        -= bookRadius + wallOffset;
    p.x        += cos(p.z * 2.0) - bookSize.x - salt * 0.25;
    p.x        += 0.01 * smoothstep(0.99, 1.0, sin(p.y * (1.0 + 10.0 * salt)));
    scene       = min(scene, max(sdBox(p, vec3(bookSize.x, 100.0, bookSize.z)), p.y - bookSize.y));

    // panel
    p            = pos;
    p.y          = repeat(p.y, cell.y);
    p.xz        *= rot(PI / wallCount);
    amod(p.xz, wallCount);
    p.x         -= wallRadius;
    float panel  = sdBox(p, panelSize);
    float pz     = p.z;
    p.z = repeat(p.z, 0.2 + 0.3 * salt);
    panel        = min(panel, max(sdBox(p, vec3(0.1, 0.1, 0.04)), abs(pz) - panelSize.z * 0.8));
    scene        = min(scene, panel);

    // papers
    p         = pos;
    p.y      -= stairHeight;
    p.y      += time * 2.0;
    p.xz     *= rot(PI / stairCount);
    float ry  = 8.0;
    float iy  = floor(p.y / ry);
    salt      = rng(vec2(iy));
    float a   = iy;
    p.xz     -= vec2(cos(a),sin(a)) * paperRadius;
    p.y       = repeat(p.y, ry);
    p.xy     *= rot(p.z);
    p.xz     *= rot(PI / 4.0 + salt + time);
    scene     = min(scene, sdBox(p, paperSize));

    return scene;
}

vec3 getCamera(vec3 eye, vec2 uv)
{
    vec3  lookAt   = vec3(0.0);
    float click    = clamp(0.0/*iMouse.w*/, 0.0, 1.0);
    lookAt.x      += mix(0.0,((iMouse.x / iResolution.x) * 2.0 - 1.0) * 10.0, click);
    lookAt.y      += mix(0.0, iMouse.y / iResolution.y * 10.0, click);
    float fov      = 0.65;
    vec3  forward  = normalize(lookAt - eye);
    vec3  right    = normalize(cross(vec3(0, 1, 0), forward));
    vec3  up       = normalize(cross(forward, right));
    return normalize(fov * forward + uv.x * right + uv.y * up);
}

float getLight(vec3 pos, vec3 eye)
{
    vec3  light   = vec3(-0.5, 7.0, 1.0);
    vec3  normal  = getNormal(pos);
    vec3  view    = normalize(eye - pos);
    float shade   = dot(normal, view);
    shade        *= hardShadow(pos, light);
    return shade;
}

vec4 raymarch ()
{
    vec2  uv     = (gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;
    float dither = rng(uv + fract(time));
    vec3  eye    = vec3(0, 5, -4.5);
    vec3  ray    = getCamera(eye, uv);
    vec3  pos    = eye;
    float shade  = 0.0;

    for (float i = 0.0; i <= 1.0; i += 1.0 / STEPS)
    {
        float dist = map(pos);

        if (dist < VOLUME)
        {
            shade = 1.0 - i;
            break;
        }

        dist *= 0.5 + 0.1 * dither;
        pos  += ray * dist;
    }

    vec4 color  = vec4(shade);
    color      *= getLight(pos, eye);
    color       = smoothstep(0.0, 0.5, color);
    color       = sqrt(color);
    return color;
}

void main(void)
{
    gl_FragColor = raymarch();
}