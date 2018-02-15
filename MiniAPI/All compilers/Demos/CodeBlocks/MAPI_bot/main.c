/*****************************************************************************
 * ==> Bot demo -------------------------------------------------------------*
 *****************************************************************************
 * Description : An intelligent bot demo                                     *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <math.h>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniModels.h"
#include "MiniLevel.h"
#include "MiniShader.h"
#include "MiniRenderer.h"
#include "MiniPlayer.h"
#include "MiniAI.h"

// windows
#include <Windowsx.h>

// resources
#include "resources.rh"

// NOTE the mdl model was extracted from the Quake game package
#define MDL_FILE               "..\\..\\..\\Resources\\wizard.mdl"
#define SOIL_TEXTURE_FILE      "..\\..\\..\\Resources\\Soil_25_256x256.bmp"
#define WALL_TEXTURE_FILE      "..\\..\\..\\Resources\\Wall_Tilleable_64_256x256.bmp"
#define CEIL_TEXTURE_FILE      "..\\..\\..\\Resources\\Soil_2_25_256x256.bmp"
#define BULLET_TEXTURE_FILE    "..\\..\\..\\Resources\\bullet.bmp"
#define PLAYER_STEP_SOUND_FILE "..\\..\\..\\Resources\\human_walk_step.wav"
#define PLAYER_FIRE_SOUND_FILE "..\\..\\..\\Resources\\fire_and_reload.wav"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
typedef struct
{
    MINI_Sphere m_Shape;
    float       m_Angle;
    int         m_Fired;
} MINI_Bullet;
//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} MINI_MDLAnimation;
//------------------------------------------------------------------------------
const char* g_pLevelMap =
    "***   ***"
    "*********"
    "*********"
    "*** * ***"
    "*** * ***"
    "***   ***"
    "*********"
    "*********"
    "***   ***";
//------------------------------------------------------------------------------
const int g_BotPath[] =
{
    0, 0,  0,  0,  0,  0,  0,  0, 0,
    0, 13, 12, 11, 10, 9,  8,  7, 0,
    0, 14, 0,  0,  0,  0,  0,  6, 0,
    0, 15, 0,  0,  0,  0,  0,  5, 0,
    0, 16, 0,  0,  0,  0,  0,  4, 0,
    0, 17, 0,  0,  0,  0,  0,  3, 0,
    0, 18, 0,  0,  0,  0,  0,  2, 0,
    0, 19, 20, 21, 22, 23, 24, 1, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0
};
//------------------------------------------------------------------------------
const char* g_BotVS =
    "precision mediump float;"
    "attribute vec4  mini_vPosition;"
    "attribute vec4  mini_vColor;"
    "attribute vec2  mini_vTexCoord;"
    "uniform   mat4  mini_uProjection;"
    "uniform   mat4  mini_uView;"
    "uniform   mat4  mini_uModelview;"
    "uniform   float mini_uAlpha;"
    "uniform   float mini_uFadeFactor;"
    "uniform   int   mini_uRedFilter;"
    "varying   vec4  mini_fColor;"
    "varying   vec2  mini_fTexCoord;"
    "varying   float mini_fAlpha;"
    "varying   float mini_fRedFilter;"
    "varying   float mini_fFadeFactor;"
    "void main(void)"
    "{"
    "    mini_fColor      = mini_vColor;"
    "    mini_fTexCoord   = mini_vTexCoord;"
    "    mini_fAlpha      = mini_uAlpha;"
    "    mini_fFadeFactor = mini_uFadeFactor;"
    "    gl_Position      = mini_uProjection * mini_uView * mini_uModelview * mini_vPosition;"
    ""
    "    if (mini_uRedFilter == 1)"
    "        mini_fRedFilter = 1.0;"
    "    else"
    "        mini_fRedFilter = 0.0;"
    "}";
//------------------------------------------------------------------------------
const char* g_BotFS =
    "precision mediump float;"
    "uniform sampler2D  mini_sColorMap;"
    "varying lowp vec4  mini_fColor;"
    "varying      vec2  mini_fTexCoord;"
    "varying      float mini_fAlpha;"
    "varying      float mini_fRedFilter;"
    "varying      float mini_fFadeFactor;"
    "void main(void)"
    "{"
    "    vec4 color = mini_fColor * texture2D(mini_sColorMap, mini_fTexCoord);"
    ""
    "    if (mini_fRedFilter > 0.5)"
    "        gl_FragColor = vec4(color.x, 0.0, 0.0, mini_fAlpha);"
    "    else"
    "        gl_FragColor = vec4(color.x * mini_fFadeFactor, color.y * mini_fFadeFactor, color.z * mini_fFadeFactor, mini_fAlpha);"
    "}";
//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
MINI_LevelItem*    g_pLevel             = 0;
MINI_MDLModel*     g_pModel             = 0;
MINI_Index*        g_pBulletIndexes     = 0;
GLuint             g_ShaderProgram      = 0;
float*             g_pSurfaceVB         = 0;
float*             g_pBulletVertices    = 0;
const float        g_LevelItemWidth     = 5.0f;
const float        g_LevelItemHeight    = 5.0f;
const float        g_LevelItemLength    = 5.0f;
const float        g_BulletVelocity     = 25.0f;
const float        g_BotFadeVelocity    = 0.5f;
const float        g_ControlRadius      = 40.0f;
float              g_PosVelocity        = 0.0f;
float              g_DirVelocity        = 0.0f;
float              g_BotVelocity        = 10.0f;
float              g_Angle              = 0.0f;
float              g_BotAngle           = 0.0f;
float              g_Time               = 0.0f;
float              g_StepTime           = 0.0f;
float              g_Interval           = 0.0f;
float              g_StepInterval       = 350.0f;
float              g_BotAlphaLevel      = 1.0f;
float              g_BulletAlphaLevel   = 0.8f;
float              g_FadeLevel          = 1.0f;
float              g_FadeLevelVelocity  = 0.5f;
float              g_HitTime            = 0.0f;
const unsigned int g_FPS                = 15;
const unsigned int g_MapWidth           = 9;
const unsigned int g_MapHeight          = 9;
unsigned int       g_MapLength          = 0;
unsigned int       g_SurfaceVertexCount = 0;
unsigned int       g_SceneInitialized   = 0;
unsigned int       g_MeshIndex          = 0;
unsigned int       g_CurMeshIndex       = 0;
unsigned int       g_AnimLooped         = 0;
unsigned int       g_BotPathPos         = 1;
unsigned int       g_BotHurt            = 0;
unsigned int       g_BotDied            = 0;
unsigned int       g_BotBulletStopped   = 0;
unsigned int       g_PlayerWasFound     = 0;
unsigned int       g_PlayerHit          = 0;
unsigned int       g_PlayerDied         = 0;
unsigned int       g_PlayerEnergy       = 3; // the number of impacts the player can support before die
unsigned int       g_BotEnergy          = 3; // the number of impacts the bot can support before die
unsigned int       g_BulletVertexCount;
unsigned int       g_BulletIndexCount;
MINI_Sphere        g_Player;
MINI_Sphere        g_Bot;
MINI_Bullet        g_PlayerBullet;
MINI_Bullet        g_BotBullet;
MINI_Vector2       g_TouchOrigin;
MINI_Vector2       g_TouchPosition;
GLuint             g_SoilTextureIndex   = GL_INVALID_VALUE;
GLuint             g_WallTextureIndex   = GL_INVALID_VALUE;
GLuint             g_CeilTextureIndex   = GL_INVALID_VALUE;
GLuint             g_ModelTextureIndex  = GL_INVALID_VALUE;
GLuint             g_BulletTextureIndex = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot     = 0;
GLuint             g_AlphaSlot          = 0;
GLuint             g_FadeFactorSlot     = 0;
GLuint             g_RedFilterSlot      = 0;
GLuint             g_ViewUniform        = 0;
GLuint             g_ModelviewUniform   = 0;
ALCdevice*         g_pOpenALDevice      = 0;
ALCcontext*        g_pOpenALContext     = 0;
ALuint             g_PlayerStepBufferID = 0;
ALuint             g_PlayerStepSoundID  = 0;
ALuint             g_PlayerFireBufferID = 0;
ALuint             g_PlayerFireSoundID  = 0;
MINI_VertexFormat  g_VertexFormat;
MINI_VertexFormat  g_ModelFormat;
MINI_VertexFormat  g_BulletVertexFormat;
MINI_MDLAnimation  g_Animation[5]; // 0 = hover, 1 = fly, 2 = attack, 3 = pain, 4 = death
MINI_BotItem       g_Items[1];
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_LEFT:  g_DirVelocity = -5.0f; break;
                case VK_RIGHT: g_DirVelocity =  5.0f; break;
                case VK_UP:    g_PosVelocity = -30.0f; break;
                case VK_DOWN:  g_PosVelocity =  30.0f; break;

                case VK_SPACE:
                {
                    g_PlayerBullet.m_Shape.m_Pos.m_X = g_Player.m_Pos.m_X;
                    g_PlayerBullet.m_Shape.m_Pos.m_Z = g_Player.m_Pos.m_Z;
                    g_PlayerBullet.m_Angle           = g_Angle;
                    miniStopSound(g_PlayerFireSoundID);
                    miniPlaySound(g_PlayerFireSoundID);
                    g_PlayerBullet.m_Fired = 1;
                    break;
                }
            }

            break;
        }

        case WM_KEYUP:
        {
            switch (wParam)
            {
                case VK_LEFT:
                case VK_RIGHT: g_DirVelocity = 0.0f; break;

                case VK_UP:
                case VK_DOWN:  g_PosVelocity = 0.0f; break;
            }

            break;
        }

        case WM_SIZE:
        {
            if (!g_SceneInitialized)
                break;

            const int width  = ((int)(short)LOWORD(lParam));
            const int height = ((int)(short)HIWORD(lParam));

            CreateViewport(width, height);
            break;
        }

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}
//------------------------------------------------------------------------------
void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    // get the device context (DC)
    *hDC = GetDC(hwnd);

    // set the pixel format for the DC
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    // create and enable the render context (RC)
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}
//------------------------------------------------------------------------------
void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
//------------------------------------------------------------------------------
int PathPosToLevelIndex(unsigned pathPos, const int* pPath, unsigned pathLength)
{
    unsigned i;

    // search the level item the path position represents
    for (i = 0; i < pathLength; ++i)
        if (pPath[i] == pathPos)
            return i;

    return -1;
}
//------------------------------------------------------------------------------
int BulletHitBody(MINI_Bullet* pBullet, MINI_Sphere* pBody)
{
    // no bullet?
    if (!pBullet)
        return 0;

    // no body?
    if (!pBody)
        return 0;

    // was bullet fired?
    if (!pBullet->m_Fired)
        return 0;

    // was body hit by bullet? NOTE just do a simple approximation for the hit detection. It's
    // enough for this demo, but in a real game a more complex polygon collision detection
    // should be done. See miniPopulateTree() and miniResolveTree() for further information
    if (!miniSpheresIntersect(pBody, &pBullet->m_Shape))
        return 0;

    // stop the bullet
    pBullet->m_Fired = 0;

    return 1;
}
//------------------------------------------------------------------------------
int BulletHitPlayer()
{
    // has bullet hit the player?
    if (!BulletHitBody(&g_BotBullet, &g_Player))
        return 0;

    // player loose a point of energy
    if (g_PlayerEnergy)
        --g_PlayerEnergy;

    return 1;
}
//------------------------------------------------------------------------------
int BulletHitBot()
{
    // has bullet hit the bot?
    if (!BulletHitBody(&g_PlayerBullet, &g_Bot))
        return 0;

    // bot loose a point of energy
    if (g_BotEnergy)
        --g_BotEnergy;

    // notify that bot was hurt
    g_BotHurt = 1;

    return 1;
}
//------------------------------------------------------------------------------
void BotRunAnimation(int animIndex, float elapsedTime)
{
    int frameCount = 0;
    int deltaRange = (int)(g_Animation[animIndex].m_Range[1] -
                           g_Animation[animIndex].m_Range[0]);

    // calculate next time
    g_Time += (elapsedTime * 1000.0f);

    // count frames
    while (g_Time > g_Interval)
    {
        g_Time -= g_Interval;
        ++frameCount;
    }

    // calculate next mesh index to show. Index should always be between animation range
    g_CurMeshIndex = ((g_CurMeshIndex + frameCount) % deltaRange);
    g_MeshIndex    = (int)g_Animation[animIndex].m_Range[0] + g_CurMeshIndex;
}
//------------------------------------------------------------------------------
int BotRunAnimationOnce(int animIndex, float elapsedTime)
{
    unsigned int frameCount = 0;

    // calculate next time
    g_Time += (elapsedTime * 1000.0f);

    // count frames
    while (g_Time > g_Interval)
    {
        g_Time -= g_Interval;
        ++frameCount;
    }

    // calculate next mesh index to show
    g_MeshIndex += frameCount;

    // was animation end reached?
    if (g_MeshIndex >= (int)g_Animation[animIndex].m_Range[1])
    {
        g_MeshIndex = (int)g_Animation[animIndex].m_Range[1];
        return 1;
    }

    return 0;
}
//------------------------------------------------------------------------------
int BotSpottedPlayer()
{
    MINI_Vector2 start;
    MINI_Vector2 end;
    MINI_Vector3 dir;
    MINI_Vector3 nDir;
    MINI_Plane   botFieldOfView;
    float        distToFOV;

    // player is no longer visible if he died
    if (g_PlayerDied)
        return 0;

    dir.m_Y = 0.0f;

    // calculate the direction for the bot left field of view plane
    dir.m_X = -cosf(((M_PI * 2.0f) - g_BotAngle) - 0.35f);
    dir.m_Z = -sinf(((M_PI * 2.0f) - g_BotAngle) - 0.35f);

    // normalize it
    miniNormalize(&dir, &nDir);

    // calculate the bot left field of view plane
    miniPlaneFromPointNormal(&g_Bot.m_Pos, &nDir, &botFieldOfView);

    // calculate if the player is inside the bot left field of view
    miniDistanceToPlane(&g_Player.m_Pos, &botFieldOfView, &distToFOV);

    if (distToFOV >= 0.0f)
        return 0;

    // calculate the direction for the bot field of view right plane
    dir.m_X = -cosf(((M_PI * 2.0f) - g_BotAngle) + 0.35f);
    dir.m_Z = -sinf(((M_PI * 2.0f) - g_BotAngle) + 0.35f);

    // normalize it
    miniNormalize(&dir, &nDir);

    // calculate the bot right field of view plane
    miniPlaneFromPointNormal(&g_Bot.m_Pos, &nDir, &botFieldOfView);

    // calculate if the player is inside the bot right field of view
    miniDistanceToPlane(&g_Player.m_Pos, &botFieldOfView, &distToFOV);

    if (distToFOV >= 0.0f)
        return 0;

    start.m_X = g_Player.m_Pos.m_X;
    start.m_Y = g_Player.m_Pos.m_Z;

    end.m_X = g_Bot.m_Pos.m_X;
    end.m_Y = g_Bot.m_Pos.m_Z;

    // check if a wall is hiding the player
    if (!miniBodyIntersectWall(&start,
                               &end,
                                g_pLevel,
                                g_LevelItemWidth,
                                g_LevelItemHeight,
                                g_MapLength))
    {
        // unfortunately not :-) the player was shown by the bot
        g_PlayerWasFound = 1;
        return 1;
    }

    return 0;
}
//------------------------------------------------------------------------------
float BotLookAtPlayerAngle()
{
    MINI_Vector3 dir;
    MINI_Vector3 normal;
    float        angle;
    float        botAngle;

    // calculate the normal from bot position and pointing to player
    miniSub(&g_Bot.m_Pos, &g_Player.m_Pos, &dir);
    miniNormalize(&dir, &normal);

    // calculate the angle to apply to bot to look at the player
    angle = normal.m_Z / normal.m_X;

    if (normal.m_X <= 0.0f)
        botAngle = (M_PI * 2.0f) - atan(angle);
    else
        botAngle = M_PI - atan(angle);

    // and validate it
    if (botAngle > M_PI * 2.0f)
        botAngle -= M_PI * 2.0f;
    else
    if (botAngle < 0.0f)
        botAngle += M_PI * 2.0f;

    return botAngle;
}
//------------------------------------------------------------------------------
int OnBotWatching(float elapsedTime, MINI_BotItem* pItem)
{
    // run and loop the bot animation
    BotRunAnimation(0, elapsedTime);

    // do fade in the bot?
    if (g_BotAlphaLevel != 1.0f)
    {
        // increase the bot alpha level
        g_BotAlphaLevel += (g_BotFadeVelocity * elapsedTime);

        // maximal alpha level reached?
        if (g_BotAlphaLevel > 1.0f)
            g_BotAlphaLevel = 1.0f;

        return 0;
    }

    // bot will execute the next task if:
    // 1. he is hit by a bullet
    // 2. he shows the player
    // 3. he randomly decides to search the player
    return (BulletHitBot() || BotSpottedPlayer() || (rand() % 200 == 15));
}
//------------------------------------------------------------------------------
int OnBotSearching(float elapsedTime, MINI_BotItem* pItem)
{
    int   startLevelItemIndex;
    int   endLevelItemIndex;
    int   doGetNextItem;
    float startX;
    float startY;
    float endX;
    float endY;
    float lengthX;
    float lengthY;

    // run and loop the bot animation
    BotRunAnimation(1, elapsedTime);

    // is the search starting?
    if (g_BotPathPos == 1)
    {
        // set the bot to the first position and select the next level item to reach
        g_Bot.m_Pos.m_X = g_pLevel[70].m_X + (g_MapWidth  * 0.5f);
        g_Bot.m_Pos.m_Z = g_pLevel[70].m_Y + (g_MapHeight * 0.5f);
        ++g_BotPathPos;
    }

    // get the level item on which the bot is located, and the next item to reach. NOTE a simple pre-calculated
    // table is used here. This is enough for this demo but in a real game the search should use a path finding
    // algorithm to be really efficient
    startLevelItemIndex = PathPosToLevelIndex(g_BotPathPos,     g_BotPath, g_MapLength);
    endLevelItemIndex   = PathPosToLevelIndex(g_BotPathPos + 1, g_BotPath, g_MapLength);

    // is the last search pos reached?
    if (endLevelItemIndex == -1)
        endLevelItemIndex = PathPosToLevelIndex(1, g_BotPath, g_MapLength);

    // is the search end reached?
    if (startLevelItemIndex == -1)
    {
        // set the bot to the last position, and reset the search task
        g_Bot.m_Pos.m_X = g_pLevel[70].m_X + (g_MapWidth  * 0.5f);
        g_Bot.m_Pos.m_Z = g_pLevel[70].m_Y + (g_MapHeight * 0.5f);
        g_BotAngle      = M_PI * 0.5f;
        g_BotPathPos    = 1;

        // notify that the task is completed
        return 1;
    }

    // get the real start and end position, and calculate the length
    startX  = g_pLevel[startLevelItemIndex].m_X + (g_MapWidth  * 0.5f);
    startY  = g_pLevel[startLevelItemIndex].m_Y + (g_MapHeight * 0.5f);
    endX    = g_pLevel[endLevelItemIndex].m_X   + (g_MapWidth  * 0.5f);
    endY    = g_pLevel[endLevelItemIndex].m_Y   + (g_MapHeight * 0.5f);
    lengthX = startX - endX;
    lengthY = endY   - startY;

    // calculate the both direction
    if (!lengthY)
    {
        if (lengthX < 0)
            g_BotAngle = 0.0f;
        else
            g_BotAngle = M_PI;
    }
    else
    if (!lengthX)
    {
        if (lengthY < 0)
            g_BotAngle = (M_PI * 0.5f);
        else
            g_BotAngle = -(M_PI * 0.5f);
    }
    else
        g_BotAngle = atanf(lengthX / lengthY) + (M_PI * 0.5f);

    // and validate it
    if (g_BotAngle > M_PI * 2.0f)
        g_BotAngle -= M_PI * 2.0f;
    else
    if (g_BotAngle < 0.0f)
        g_BotAngle += M_PI * 2.0f;

    // calculate the bot next position
    g_Bot.m_Pos.m_X += g_BotVelocity * cosf(g_BotAngle) * elapsedTime;
    g_Bot.m_Pos.m_Z -= g_BotVelocity * sinf(g_BotAngle) * elapsedTime;

    doGetNextItem = 0;

    // do get the next item (from the x axis)
    if (endX > startX && g_Bot.m_Pos.m_X >= endX)
        doGetNextItem = 1;
    else
    if (endX < startX && g_Bot.m_Pos.m_X <= endX)
        doGetNextItem = 1;

    // do get the next item (from the z axis)
    if (endY > startY && g_Bot.m_Pos.m_Z >= endY)
        doGetNextItem = 1;
    else
    if (endY < startY && g_Bot.m_Pos.m_Z <= endY)
        doGetNextItem = 1;

    // get the next item, if required
    if (doGetNextItem)
        ++g_BotPathPos;

    // bot will execute the next task if:
    // 1. he is hit by a bullet
    // 2. he shows the player
    return (BulletHitBot() || BotSpottedPlayer());
}
//------------------------------------------------------------------------------
int OnBotAttacking(float elapsedTime, MINI_BotItem* pItem)
{
    // a player also fired a bullet, and this bullet hit the bot?
    if (BulletHitBot())
        return 1;

    // run the bot animation
    BotRunAnimationOnce(2, elapsedTime);

    // make the bot looking the player
    g_BotAngle = BotLookAtPlayerAngle();

    // bullet hit something?
    if (g_BotBulletStopped)
    {
        g_BotBulletStopped = 0;

        // is the player still visible?
        if (!BotSpottedPlayer())
            return 1;

        // reload the animation to allow to fire a new bullet
        g_MeshIndex = (int)g_Animation[2].m_Range[0];
    }
    else
    // did the bot fired?
    if (!g_BotBullet.m_Fired)
    {
        g_BotBulletStopped = 0;

        // fire a new bullet
        g_BotBullet.m_Shape.m_Pos.m_X =  g_Bot.m_Pos.m_X;
        g_BotBullet.m_Shape.m_Pos.m_Z =  g_Bot.m_Pos.m_Z;
        g_BotBullet.m_Angle           = (M_PI * 2.0f) - (g_BotAngle - (M_PI * 0.5f));
        g_BotBullet.m_Fired           =  1;

        miniPlaySound(g_PlayerFireSoundID);
    }

    // the task ends when the animation was executed completely
    if (g_AnimLooped)
        // and only if user is no longer visible
        return (!BotSpottedPlayer());

    return 0;
}
//------------------------------------------------------------------------------
int OnBotBeingHurt(float elapsedTime, MINI_BotItem* pItem)
{
    // the task ends when the animation was executed completely
    return BotRunAnimationOnce(3, elapsedTime);
}
//------------------------------------------------------------------------------
int OnBotDying(float elapsedTime, MINI_BotItem* pItem)
{
    // bot animation was executed?
    if (g_BotDied)
    {
        // decrease the bot alpha level
        g_BotAlphaLevel -= (g_BotFadeVelocity * elapsedTime);

        // minimal alpha level reached?
        if (g_BotAlphaLevel > 0.0f)
            return 0;

        // yes, it's time to restart
        g_BotAlphaLevel = 0.0f;
        return 1;
    }

    // run dying animation once
    if (BotRunAnimationOnce(4, elapsedTime))
        g_BotDied = 1;

    return 0;
}
//------------------------------------------------------------------------------
void OnNewTask(float elapsedTime, MINI_BotItem* pItem)
{
    // bot died?
    if (g_BotDied)
    {
        g_BotDied = 0;

        // reset all bot data and restart to wait
        g_Bot.m_Pos.m_X   =  g_pLevel[70].m_X + (g_MapWidth * 0.5f);
        g_Bot.m_Pos.m_Y   = -1.0f;
        g_Bot.m_Pos.m_Z   =  g_pLevel[70].m_Y + (g_MapHeight * 0.5f);
        g_Bot.m_Radius    =  2.0f;
        g_BotAngle        =  M_PI * 0.5f;
        g_BotEnergy       =  3;
        g_BotPathPos      =  1;
        g_Items[0].m_Task =  E_BT_Watching;
        g_MeshIndex       = (int)g_Animation[0].m_Range[0];

        return;
    }

    // bot spotted the player?
    if (g_PlayerWasFound)
    {
        g_PlayerWasFound = 0;

        // attack it
        pItem->m_Task = E_BT_Attacking;

        // set the animation index to start pos, to avoid jumps in animation
        g_MeshIndex = (int)g_Animation[2].m_Range[0];

        return;
    }

    // bot was hurt?
    if (g_BotHurt)
    {
        g_BotHurt = 0;

        // bot still have energy?
        if (g_BotEnergy)
        {
            // set the animation index to start pos, to avoid jumps in animation
            g_MeshIndex = (int)g_Animation[3].m_Range[0];

            pItem->m_Task = E_BT_Being_Hurt;
            return;
        }

        // set the animation index to start pos, to avoid jumps in animation
        g_MeshIndex = (int)g_Animation[4].m_Range[0];

        pItem->m_Task = E_BT_Dying;
        return;
    }

    // bot was searching?
    if (pItem->m_Task == E_BT_Searching)
    {
        // return to watch
        pItem->m_Task = E_BT_Watching;
        g_MeshIndex   = (int)g_Animation[0].m_Range[0];
        return;
    }

    // begin the search
    pItem->m_Task = E_BT_Searching;
    g_MeshIndex   = (int)g_Animation[1].m_Range[0];
}
//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.001f;
    const float zFar   = 1000.0f;
    const float fov    = 45.0f;
    const float aspect = (GLfloat)w/(GLfloat)h;

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    float          bulletRadius;
    unsigned int   playerStepSoundFileLen;
    unsigned int   playerFireSoundFileLen;
    unsigned char* pPlayerStepSndBuffer;
    unsigned char* pPlayerFireSndBuffer;
    MINI_Texture   texture;

    g_MapLength = g_MapWidth * g_MapHeight;

    bulletRadius = 0.1f;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(g_BotVS, g_BotFS);
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram,  "mini_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram,  "mini_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram,  "mini_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram,  "mini_sColorMap");
    g_AlphaSlot             = glGetUniformLocation(g_ShaderProgram, "mini_uAlpha");
    g_FadeFactorSlot        = glGetUniformLocation(g_ShaderProgram, "mini_uFadeFactor");
    g_RedFilterSlot         = glGetUniformLocation(g_ShaderProgram, "mini_uRedFilter");

    // create the viewport
    CreateViewport(w, h);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // generate the level
    miniGenerateLevel(g_pLevelMap,
                      g_MapWidth,
                      g_MapHeight,
                      &g_LevelItemWidth,
                      &g_LevelItemHeight,
                      &g_pLevel);

    g_BulletVertexFormat.m_UseNormals  = 0;
    g_BulletVertexFormat.m_UseTextures = 1;
    g_BulletVertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&bulletRadius,
                      5,
                      5,
                      0xFFFFFFFF,
                     &g_BulletVertexFormat,
                     &g_pBulletVertices,
                     &g_BulletVertexCount,
                     &g_pBulletIndexes,
                     &g_BulletIndexCount);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // calculate the stride
    miniCalculateStride(&g_VertexFormat);

    // generate surface
    miniCreateSurface(&g_LevelItemWidth,
                      &g_LevelItemHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);

    // load textures
    g_SoilTextureIndex   = miniLoadTexture(SOIL_TEXTURE_FILE);
    g_WallTextureIndex   = miniLoadTexture(WALL_TEXTURE_FILE);
    g_CeilTextureIndex   = miniLoadTexture(CEIL_TEXTURE_FILE);
    g_BulletTextureIndex = miniLoadTexture(BULLET_TEXTURE_FILE);

    g_ModelFormat.m_UseNormals  = 0;
    g_ModelFormat.m_UseTextures = 1;
    g_ModelFormat.m_UseColors   = 1;

    // load MDL file and create mesh to draw
    miniLoadMDLModel((const unsigned char*)MDL_FILE,
                     &g_ModelFormat,
                     0xFFFFFFFF,
                     &g_pModel,
                     &texture);

    // create new OpenGL texture
    glGenTextures(1, &g_ModelTextureIndex);
    glBindTexture(GL_TEXTURE_2D, g_ModelTextureIndex);

    // set texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set texture wrapping mode
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // generate texture from bitmap data
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 texture.m_Width,
                 texture.m_Height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 texture.m_pPixels);

    // delete buffers
    free(texture.m_pPixels);

    miniInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // get the sound files length
    playerStepSoundFileLen = miniGetFileSize(PLAYER_STEP_SOUND_FILE);
    playerFireSoundFileLen = miniGetFileSize(PLAYER_FIRE_SOUND_FILE);

    // allocate buffers
    pPlayerStepSndBuffer = (unsigned char*)calloc(playerStepSoundFileLen, sizeof(unsigned char));
    pPlayerFireSndBuffer = (unsigned char*)calloc(playerFireSoundFileLen, sizeof(unsigned char));

    // load step sound file
    miniLoadSoundBuffer(PLAYER_STEP_SOUND_FILE,
                        playerStepSoundFileLen,
                        &pPlayerStepSndBuffer);

    // load fire sound file and get length
    miniLoadSoundBuffer(PLAYER_FIRE_SOUND_FILE,
                        playerFireSoundFileLen,
                        &pPlayerFireSndBuffer);

    // create step sound file
    miniCreateSound(g_pOpenALDevice,
                    g_pOpenALContext,
                    pPlayerStepSndBuffer,
                    playerStepSoundFileLen,
                    48000,
                    &g_PlayerStepBufferID,
                    &g_PlayerStepSoundID);

    // create fire sound file
    miniCreateSound(g_pOpenALDevice,
                    g_pOpenALContext,
                    pPlayerFireSndBuffer,
                    playerFireSoundFileLen,
                    48000,
                    &g_PlayerFireBufferID,
                    &g_PlayerFireSoundID);

    // delete step sound resource
    if (pPlayerStepSndBuffer)
        free(pPlayerStepSndBuffer);

    // delete fire sound resource
    if (pPlayerFireSndBuffer)
        free(pPlayerFireSndBuffer);

    // initialize the player
    g_Player.m_Pos.m_X = 0.0f;
    g_Player.m_Pos.m_Y = 0.0f;
    g_Player.m_Pos.m_Z = 0.0f;
    g_Player.m_Radius  = 2.0f;

    g_Angle = 0.0f;

    // initialize the bot
    g_Bot.m_Pos.m_X =  g_pLevel[70].m_X + (g_MapWidth * 0.5f);
    g_Bot.m_Pos.m_Y = -1.0f;
    g_Bot.m_Pos.m_Z =  g_pLevel[70].m_Y + (g_MapHeight * 0.5f);
    g_Bot.m_Radius  =  2.0f;

    g_BotAngle = M_PI * 0.5f;

    // initialize the bot task
    g_Items[0].m_Task = E_BT_Watching;

    // initialize the player bullet
    g_PlayerBullet.m_Shape.m_Pos.m_X = 0.0f;
    g_PlayerBullet.m_Shape.m_Pos.m_Z = 0.0f;
    g_PlayerBullet.m_Shape.m_Radius  = bulletRadius;
    g_PlayerBullet.m_Angle           = 0.0f;
    g_PlayerBullet.m_Fired           = 0;

    // initialize the touch
    g_TouchOrigin.m_X   = 0.0f;
    g_TouchOrigin.m_Y   = 0.0f;
    g_TouchPosition.m_X = 0.0f;
    g_TouchPosition.m_Y = 0.0f;

    // create MD2 animation list
    g_Animation[0].m_Range[0] = 0;  g_Animation[0].m_Range[1] = 14; // wizard hovers
    g_Animation[1].m_Range[0] = 15; g_Animation[1].m_Range[1] = 28; // wizard flies
    g_Animation[2].m_Range[0] = 29; g_Animation[2].m_Range[1] = 42; // wizard attacks
    g_Animation[3].m_Range[0] = 43; g_Animation[3].m_Range[1] = 47; // wizard feels pain
    g_Animation[4].m_Range[0] = 48; g_Animation[4].m_Range[1] = 53; // wizard dies

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    miniReleaseMDLModel(g_pModel);
    g_pModel = 0;

    if (g_ModelTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_ModelTextureIndex);

    g_ModelTextureIndex = GL_INVALID_VALUE;

    // delete bullet vertex buffer
    if (g_pBulletVertices)
    {
        free(g_pBulletVertices);
        g_pBulletVertices = 0;
    }

    // delete bullet index buffer
    if (g_pBulletIndexes)
    {
        free(g_pBulletIndexes);
        g_pBulletIndexes = 0;
    }

    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    // delete the level
    if (g_pLevel)
    {
        free(g_pLevel);
        g_pLevel = 0;
    }

    // delete textures
    if (g_SoilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_SoilTextureIndex);

    g_SoilTextureIndex = GL_INVALID_VALUE;

    if (g_WallTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_WallTextureIndex);

    g_WallTextureIndex = GL_INVALID_VALUE;

    if (g_CeilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_CeilTextureIndex);

    g_CeilTextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;

    // stop running step sound, if needed
    if (miniIsSoundPlaying(g_PlayerStepSoundID))
        miniStopSound(g_PlayerStepSoundID);

    // stop running fire sound, if needed
    if (miniIsSoundPlaying(g_PlayerFireSoundID))
        miniStopSound(g_PlayerFireSoundID);

    // release OpenAL interface
    miniReleaseSound(g_PlayerStepBufferID, g_PlayerStepSoundID);
    miniReleaseSound(g_PlayerFireBufferID, g_PlayerFireSoundID);
    miniReleaseOpenAL(g_pOpenALDevice, g_pOpenALContext);
}
//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime)
{
    float        angle;
    MINI_Vector3 newPos;
    MINI_Vector2 bulletStart;
    MINI_Vector2 bulletEnd;

    // execute the bot tasks
    miniExecuteTasks(elapsedTime,
                     g_Items,
                     1,
                     OnBotWatching,
                     OnBotSearching,
                     OnBotAttacking,
                     OnBotBeingHurt,
                     OnBotDying,
                     OnNewTask);

    // player died?
    if (g_PlayerDied)
    {
        // fade to black
        g_FadeLevel -= (g_FadeLevelVelocity * elapsedTime);

        // reset the player hit value
        g_PlayerHit = 0;

        // minimal fade value was reached?
        if (g_FadeLevel <= 0.0f)
        {
            g_FadeLevel = 0.0f;

            // resurrect the player
            g_Player.m_Pos.m_X = 0.0f;
            g_Player.m_Pos.m_Y = 0.0f;
            g_Player.m_Pos.m_Z = 0.0f;
            g_Angle            = 0.0f;

            // and restore his status and energy
            g_PlayerEnergy = 3;
            g_PlayerDied   = 0;
        }

        return;
    }
    else
    if (g_FadeLevel < 1.0f)
    {
        // fade from black
        g_FadeLevel += (g_FadeLevelVelocity * elapsedTime);

        // maximal fade value was reached?
        if (g_FadeLevel >= 1.0f)
            g_FadeLevel = 1.0f;
    }

    // bot bullet hit the player?
    if (BulletHitPlayer())
    {
        g_PlayerHit = 1;
        g_HitTime   = 0.0f;

        // remove one energy, notify that player died if no energy remains
        if (g_PlayerEnergy)
            --g_PlayerEnergy;
        else
            g_PlayerDied = 1;
    }
    else
    if (g_HitTime < 0.1f)
        g_HitTime += elapsedTime;
    else
        g_PlayerHit = 0;

    // did the player fired?
    if (g_PlayerBullet.m_Fired)
    {
        bulletStart.m_X = g_PlayerBullet.m_Shape.m_Pos.m_X;
        bulletStart.m_Y = g_PlayerBullet.m_Shape.m_Pos.m_Z;

        // calculate the bullet next position
        g_PlayerBullet.m_Shape.m_Pos.m_X -= g_BulletVelocity * cosf(g_PlayerBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;
        g_PlayerBullet.m_Shape.m_Pos.m_Z -= g_BulletVelocity * sinf(g_PlayerBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;

        bulletEnd.m_X = g_PlayerBullet.m_Shape.m_Pos.m_X;
        bulletEnd.m_Y = g_PlayerBullet.m_Shape.m_Pos.m_Z;

        // check if the bullet intersected a wall
        if (miniBodyIntersectWall(&bulletStart,
                                  &bulletEnd,
                                   g_pLevel,
                                   g_LevelItemWidth,
                                   g_LevelItemHeight,
                                   g_MapLength))
            g_PlayerBullet.m_Fired = 0;
    }

    // did the bot fired?
    if (g_BotBullet.m_Fired)
    {
        // calculate the next bullet position
        bulletStart.m_X = g_BotBullet.m_Shape.m_Pos.m_X;
        bulletStart.m_Y = g_BotBullet.m_Shape.m_Pos.m_Z;

        g_BotBullet.m_Shape.m_Pos.m_X -= g_BulletVelocity * cosf(g_BotBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;
        g_BotBullet.m_Shape.m_Pos.m_Z -= g_BulletVelocity * sinf(g_BotBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;

        bulletEnd.m_X = g_BotBullet.m_Shape.m_Pos.m_X;
        bulletEnd.m_Y = g_BotBullet.m_Shape.m_Pos.m_Z;

        // check if the bullet intersected a wall
        if (miniBodyIntersectWall(&bulletStart,
                                  &bulletEnd,
                                   g_pLevel,
                                   g_LevelItemWidth,
                                   g_LevelItemHeight,
                                   g_MapLength))
        {
            // yes, stop the bullet
            g_BotBullet.m_Fired = 0;

            // and notify that bullet was stopped
            g_BotBulletStopped = 1;
        }
    }

    // no time elapsed?
    if (!elapsedTime)
        return;

    // is player rotating?
    if (g_DirVelocity)
    {
        // calculate the player direction
        g_Angle += g_DirVelocity * elapsedTime;

        // validate and apply it
        if (g_Angle > M_PI * 2.0f)
            g_Angle -= M_PI * 2.0f;
        else
        if (g_Angle < 0.0f)
            g_Angle += M_PI * 2.0f;
    }

    // is player moving?
    if (g_PosVelocity)
    {
        newPos = g_Player.m_Pos;

        // calculate the next player position
        newPos.m_X += g_PosVelocity * cosf(g_Angle + (M_PI * 0.5f)) * elapsedTime;
        newPos.m_Z += g_PosVelocity * sinf(g_Angle + (M_PI * 0.5f)) * elapsedTime;

        // validate and apply it
        miniValidateNextPos(g_pLevel, g_LevelItemWidth, g_LevelItemHeight, g_MapLength, &g_Player, &newPos);
        g_Player.m_Pos = newPos;

        // calculate next time where the step sound should be played
        g_StepTime += (elapsedTime * 1000.0f);

        // do play the sound?
        if (g_StepTime > g_StepInterval)
        {
            miniStopSound(g_PlayerStepSoundID);
            miniPlaySound(g_PlayerStepSoundID);
            g_StepTime = 0.0f;
        }
    }

    if (miniSpheresIntersect(&g_Player, &g_Bot))
    {
        // todo -cFeature -oJean: see what to do with that
    }
}
//------------------------------------------------------------------------------
void DrawScene()
{
    MINI_Vector3       t;
    MINI_Vector3       axis;
    MINI_Vector3       factor;
    MINI_Matrix        translateMatrix;
    MINI_Matrix        rotateMatrixX;
    MINI_Matrix        rotateMatrixY;
    MINI_Matrix        scaleMatrix;
    MINI_Matrix        combinedMatrixLevel1;
    MINI_Matrix        combinedMatrixLevel2;
    MINI_Matrix        modelViewMatrix;
    MINI_LevelDrawInfo drawInfo;
    float              angle;
    GLint              modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    glDisable(GL_BLEND);

    // apply the fade level (change while player is dying)
    glUniform1f(g_FadeFactorSlot, g_FadeLevel);

    // apply the red filter (change the scene to red while player is hit)
    glUniform1i(g_RedFilterSlot, g_PlayerHit);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    glFrontFace(GL_CCW);

    // configure the draw info
    drawInfo.m_pSurfaceVB         =  g_pSurfaceVB;
    drawInfo.m_SurfaceVertexCount =  g_SurfaceVertexCount;
    drawInfo.m_pVertexFormat      = &g_VertexFormat;
    drawInfo.m_pShader            = &g_Shader;
    drawInfo.m_ShaderProgram      =  g_ShaderProgram;
    drawInfo.m_SoilTextureIndex   =  g_SoilTextureIndex;
    drawInfo.m_WallTextureIndex   =  g_WallTextureIndex;
    drawInfo.m_CeilTextureIndex   =  g_CeilTextureIndex;
    drawInfo.m_MapMode            =  0;

    // draw the level
    miniDrawLevel(g_pLevel,
                  g_LevelItemWidth,
                  g_LevelItemHeight,
                  g_LevelItemLength,
                  g_MapLength,
                 &g_Player,
                  g_Angle,
                 &drawInfo);

    glFrontFace(GL_CW);

    // set translation
    t.m_X = g_Bot.m_Pos.m_X - g_Player.m_Pos.m_X;
    t.m_Y = g_Bot.m_Pos.m_Y;
    t.m_Z = g_Bot.m_Pos.m_Z - g_Player.m_Pos.m_Z;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.5f;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    miniGetRotateMatrix(&g_BotAngle, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 0.05f;
    factor.m_Y = 0.05f;
    factor.m_Z = 0.05f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniMatrixMultiply(&scaleMatrix,          &rotateMatrixX,   &combinedMatrixLevel1);
    miniMatrixMultiply(&combinedMatrixLevel1, &rotateMatrixY,   &combinedMatrixLevel2);
    miniMatrixMultiply(&combinedMatrixLevel2, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // bind the model texture
    glBindTexture(GL_TEXTURE_2D, g_ModelTextureIndex);

    // set alpha transparency value to draw the bot
    glUniform1f(g_AlphaSlot, g_BotAlphaLevel);

    // configure OpenGL to draw transparency (NOTE all opaque objects should be drawn before,
    // because depth test should be disabled to allow alpha blending to work correctly)
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw the bot model
    miniDrawMDL(g_pModel, &g_Shader, g_MeshIndex);

    // did the player fired?
    if (g_PlayerBullet.m_Fired)
    {
        // set alpha transparency value to draw the bullet
        glUniform1f(g_AlphaSlot, g_BulletAlphaLevel);

        glFrontFace(GL_CCW);

        // set translation
        t.m_X = g_PlayerBullet.m_Shape.m_Pos.m_X - g_Player.m_Pos.m_X;
        t.m_Y = 0.0f;
        t.m_Z = g_PlayerBullet.m_Shape.m_Pos.m_Z - g_Player.m_Pos.m_Z;

        // get the bullet model view matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // bind the model texture
        glBindTexture(GL_TEXTURE_2D, g_BulletTextureIndex);

        // draw the bullet
        miniDrawSphere(g_pBulletVertices,
                       g_BulletVertexCount,
                       g_pBulletIndexes,
                       g_BulletIndexCount,
                      &g_VertexFormat,
                      &g_Shader);
    }

    // did the bot fired?
    if (g_BotBullet.m_Fired)
    {
        // set alpha transparency value to draw the bullet
        glUniform1f(g_AlphaSlot, g_BulletAlphaLevel);

        glFrontFace(GL_CCW);

        // set translation
        t.m_X = g_BotBullet.m_Shape.m_Pos.m_X - g_Player.m_Pos.m_X;
        t.m_Y = 0.0f;
        t.m_Z = g_BotBullet.m_Shape.m_Pos.m_Z - g_Player.m_Pos.m_Z;

        // get the bullet model view matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // bind the model texture
        glBindTexture(GL_TEXTURE_2D, g_BulletTextureIndex);

        // draw the bullet
        miniDrawSphere(g_pBulletVertices,
                       g_BulletVertexCount,
                       g_pBulletIndexes,
                       g_BulletIndexCount,
                      &g_VertexFormat,
                      &g_Shader);
    }

    miniEndScene();
}
//------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    // try to load application icon from resources
    HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL),
                                   MAKEINTRESOURCE(IDI_MAIN_ICON),
                                   IMAGE_ICON,
                                   16,
                                   16,
                                   0);

    WNDCLASSEX wcex;
    HWND       hWnd;
    HDC        hDC;
    HGLRC      hRC;
    MSG        msg;
    BOOL       bQuit = FALSE;

    // register the window class
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_OWNDC;
    wcex.lpfnWndProc   = WindowProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm       = hIcon;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = "MAPI_underground";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_underground",
                          "MiniAPI Underground Demo",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          400,
                          400,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    // enable OpenGL for the window
    EnableOpenGL(hWnd, &hDC, &hRC);

    // stop GLEW crashing on OSX :-/
    glewExperimental = GL_TRUE;

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        // shutdown OpenGL
        DisableOpenGL(hWnd, hDC, hRC);

        // destroy the window explicitly
        DestroyWindow(hWnd);

        return 0;
    }

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    // initialize the scene
    InitScene(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    // initialize the timer
    unsigned __int64 previousTime = GetTickCount();

    // application main loop
    while (!bQuit)
    {
        // check for messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // handle or dispatch messages
            if (msg.message == WM_QUIT)
                bQuit = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            // calculate time interval
            const unsigned __int64 now          =  GetTickCount();
            const double           elapsedTime  = (now - previousTime) / 1000.0;
                                   previousTime =  now;

            UpdateScene(elapsedTime);
            DrawScene();

            SwapBuffers(hDC);

            Sleep (1);
        }
    }

    // delete the scene
    DeleteScene();

    // shutdown OpenGL
    DisableOpenGL(hWnd, hDC, hRC);

    // destroy the window explicitly
    DestroyWindow(hWnd);

    return msg.wParam;
}
//------------------------------------------------------------------------------
