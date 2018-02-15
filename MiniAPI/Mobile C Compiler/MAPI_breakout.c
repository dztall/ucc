/*****************************************************************************
 * ==> Breakout game --------------------------------------------------------*
 *****************************************************************************
 * Description : A simple breakout game                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// supported platforms check. NOTE iOS only, but may works on other platforms
#if !defined(_OS_IOS_) && !defined(_OS_ANDROID_) && !defined(_OS_WINDOWS_)
    #error "Not supported platform!"
#endif

#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// opengl
#include <gles2.h>
#include <gles2ext.h>

// mini API
#include "MiniAPI/MiniCommon.h"
#include "MiniAPI/MiniGeometry.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniShapes.h"
#include "MiniAPI/MiniCollision.h"
#include "MiniAPI/MiniShader.h"
#include "MiniAPI/MiniRenderer.h"
#include "MiniAPI/MiniPlayer.h"

#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    #include <ccr.h>
#endif

// resources
#define BALL_REBOUND_SOUND_FILE "Resources/ball_rebound.wav"
#define BAR_EXPLODE_SOUND_FILE  "Resources/bar_explode.wav"

//------------------------------------------------------------------------------
#define M_BALL_VELOCITY_X 0.005f
#define M_BALL_VELOCITY_Y 0.005f
//------------------------------------------------------------------------------
typedef struct
{
    float m_Left;
    float m_Right;
    float m_Top;
    float m_Bottom;
    float m_BarY;
    float m_Width;
} MINI_Screen;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Rect    m_Geometry;
    MINI_Vector2 m_L;
    MINI_Vector2 m_R;
    int          m_Exploding;
    MINI_Vector2 m_ExpLOffset;
    MINI_Vector2 m_ExpROffset;
    ALuint       m_BufferID;
    ALuint       m_SoundID;
} MINI_Bar;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Circle  m_Geometry;
    MINI_Vector2 m_Offset;
    MINI_Vector2 m_Inc;
    MINI_Vector2 m_Max;
    ALuint       m_BufferID;
    ALuint       m_SoundID;
} MINI_Ball;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Rect m_Geometry;
    int       m_Visible;
} MINI_Block;
//------------------------------------------------------------------------------
const int g_Level1[15] =
{
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int g_Level2[15] =
{
    1, 0, 0, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int g_Level3[15] =
{
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int g_Level4[15] =
{
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
};
//------------------------------------------------------------------------------
const int g_Level5[15] =
{
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
};
//------------------------------------------------------------------------------
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader        g_Shader;
MINI_Screen        g_Screen;
MINI_Ball          g_Ball;
MINI_Bar           g_Bar;
MINI_Block         g_Blocks[15];
float*             g_pBarVertices       = 0;
int                g_BarVerticesCount   = 0;
float*             g_pBlockVertices     = 0;
int                g_BlockVerticesCount = 0;
float*             g_pBallVertices      = 0;
unsigned           g_BallVerticesCount  = 0;
int                g_BlockColumns       = 5;
int                g_BlockLines         = 3;
int                g_Level              = 0;
static ALCdevice*  g_pOpenALDevice      = 0;
static ALCcontext* g_pOpenALContext     = 0;
MINI_VertexFormat  g_VertexFormat;
GLuint             g_ShaderProgram;
//------------------------------------------------------------------------------
void GetScreen(float* pWidth, float* pHeight, MINI_Screen* pScreen)
{
    float width;
    float height;

    // transform the width and height to keep the correct aspect ratio
    if (width >= height)
    {
        width            = *pWidth  / *pHeight;
        height           = *pHeight / *pHeight;
        pScreen->m_Width = *pHeight;
    }
    else
    {
        width            = *pWidth  / *pWidth;
        height           = *pHeight / *pWidth;
        pScreen->m_Width = *pWidth;
    }

    // calculate the screen bounds (in the OpenGL view)
    pScreen->m_Left   = -(width  * 0.5f);
    pScreen->m_Right  =  (width  * 0.5f);
    pScreen->m_Top    =  (height * 0.5f);
    pScreen->m_Bottom = -(height * 0.5f);
 
    // calculate bar start y position
    pScreen->m_BarY = pScreen->m_Bottom + 0.05f;
}
//------------------------------------------------------------------------------
void ApplyOrtho(MINI_Screen* pScreen, float* pNear, float* pFar)
{
    MINI_Matrix ortho;

    // get orthogonal projection matrix
    miniGetOrtho(&pScreen->m_Left,
                 &pScreen->m_Right,
                 &pScreen->m_Bottom,
                 &pScreen->m_Top,
                 pNear,
                 pFar,
                 &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    int            i;
    int            j;
    int            index;
    float          blockWidth;
    float          blockTop;
    float          surfaceWidth;
    float          surfaceHeight;
    unsigned int   ballSoundFileLen;
    unsigned int   barSoundFileLen;
    unsigned char* pBallSndBuffer;
    unsigned char* pBarSndBuffer;
    float          w = view_w;
    float          h = view_h;

    // renderer buffers should no more be generated since CCR version 1.1
    #if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
        #ifndef _OS_ANDROID_
            // generate and bind in memory frame buffers to render to
            glGenRenderbuffers(1, &g_Renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, g_Renderbuffer);
            glGenFramebuffers(1,&g_Framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_RENDERBUFFER,
                                      g_Renderbuffer);
        #endif
    #endif

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // get vertex and color slots
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "mini_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "mini_vColor");

    // configure screen
    GetScreen(&w, &h, &g_Screen);

    // initialize ball data
    g_Ball.m_Geometry.m_Pos.m_X = 0.0f;
    g_Ball.m_Geometry.m_Pos.m_Y = 0.0f;
    g_Ball.m_Geometry.m_Radius  = 0.015f;
    g_Ball.m_Offset.m_X         = M_BALL_VELOCITY_X;
    g_Ball.m_Offset.m_Y         = M_BALL_VELOCITY_Y;
    g_Ball.m_Inc.m_X            = 0.001;
    g_Ball.m_Inc.m_Y            = 0.0015f;
    g_Ball.m_Max.m_X            = 0.3f;
    g_Ball.m_Max.m_Y            = 0.3f;
    g_Ball.m_SoundID            = M_OPENAL_ERROR_ID;

    // initialize bar data
    g_Bar.m_Geometry.m_Pos.m_X       = 0.0f;
    g_Bar.m_Geometry.m_Pos.m_Y       = g_Screen.m_Bottom + g_Ball.m_Max.m_Y;
    g_Bar.m_Geometry.m_Size.m_Width  = 0.06f;
    g_Bar.m_Geometry.m_Size.m_Height = 0.0125f;
    g_Bar.m_R.m_X                    = 0.0f;
    g_Bar.m_R.m_Y                    = 0.0f;
    g_Bar.m_L.m_X                    = 0.0f;
    g_Bar.m_L.m_Y                    = 0.0f;
    g_Bar.m_ExpROffset.m_X           = 0.01f;
    g_Bar.m_ExpROffset.m_Y           = 0.0125f;
    g_Bar.m_ExpLOffset.m_X           = 0.01f;
    g_Bar.m_ExpLOffset.m_Y           = 0.015f;
    g_Bar.m_SoundID                  = M_OPENAL_ERROR_ID;

    // calculate block width (block width * block count + inter space width * block count - 1)
    blockWidth = (g_BlockColumns * 0.06f) + ((g_BlockColumns - 1) * 0.0075f);
    blockTop   =  g_Screen.m_Top - 0.15f;

    // iterate through block lines
    for (j = 0; j < g_BlockLines; ++j)
    {
        // iterate through block columns
        for (i = 0; i < g_BlockColumns; ++i)
        {
            index = (j * g_BlockColumns) + i;

            g_Blocks[index].m_Geometry.m_Pos.m_X       = -(blockWidth * 0.5f) + 0.03f + (i * 0.065f);
            g_Blocks[index].m_Geometry.m_Pos.m_Y       =   blockTop - (j * 0.045f);
            g_Blocks[index].m_Geometry.m_Size.m_Width  =   0.06;
            g_Blocks[index].m_Geometry.m_Size.m_Height =   0.0125f;
            g_Blocks[index].m_Visible                  =   1;
        }
    }

    // populate vertex format
    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate disk to draw
    miniCreateDisk(0.0f,
                   0.0f,
                   g_Ball.m_Geometry.m_Radius,
                   20,
                   0xFFFF00FF,
                  &g_VertexFormat,
                  &g_pBallVertices,
                  &g_BallVerticesCount);

    surfaceWidth  = 0.06f;
    surfaceHeight = 0.0125f;

    // create a surface for the bar
    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                       0xFF0033FF,
                      &g_VertexFormat,
                      &g_pBarVertices,
                      &g_BarVerticesCount);

    surfaceWidth  = 0.06f;
    surfaceHeight = 0.025f;

    // update some colors
    g_pBarVertices[11] = 0.4f;
    g_pBarVertices[12] = 0.0f;
    g_pBarVertices[18] = 0.2f;
    g_pBarVertices[19] = 0.4f;
    g_pBarVertices[25] = 0.2f;
    g_pBarVertices[26] = 0.4f;

    // create a surface for the blocks
    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                       0x0000FFFF,
                      &g_VertexFormat,
                      &g_pBlockVertices,
                      &g_BlockVerticesCount);

    // update some colors
    g_pBlockVertices[10] = 0.2f;
    g_pBlockVertices[11] = 0.2f;
    g_pBlockVertices[12] = 0.9f;
    g_pBlockVertices[17] = 0.0f;
    g_pBlockVertices[18] = 0.1f;
    g_pBlockVertices[19] = 0.3f;
    g_pBlockVertices[24] = 0.3f;
    g_pBlockVertices[25] = 0.5f;
    g_pBlockVertices[26] = 0.8f;

    miniInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // get the sound files length
    ballSoundFileLen = miniGetFileSize(BALL_REBOUND_SOUND_FILE);
    barSoundFileLen  = miniGetFileSize(BAR_EXPLODE_SOUND_FILE);

    // allocate buffers
    pBallSndBuffer = (unsigned char*)calloc(ballSoundFileLen, sizeof(unsigned char));
    pBarSndBuffer  = (unsigned char*)calloc(barSoundFileLen,  sizeof(unsigned char));

    // load ball sound file
    miniLoadSoundBuffer(BALL_REBOUND_SOUND_FILE,
                        ballSoundFileLen,
                        &pBallSndBuffer);

    // load bar sound file and get length
    miniLoadSoundBuffer(BAR_EXPLODE_SOUND_FILE,
                        barSoundFileLen,
                        &pBarSndBuffer);

    // create ball rebound sound file
    miniCreateSound(g_pOpenALDevice,
                    g_pOpenALContext,
                    pBallSndBuffer,
                    ballSoundFileLen,
                    48000,
                    &g_Ball.m_BufferID,
                    &g_Ball.m_SoundID);

    // create bar explode sound file
    miniCreateSound(g_pOpenALDevice,
                    g_pOpenALContext,
                    pBarSndBuffer,
                    barSoundFileLen,
                    48000,
                    &g_Bar.m_BufferID,
                    &g_Bar.m_SoundID);

    // delete ball sound resource
    if (pBallSndBuffer)
        free(pBallSndBuffer);

    // delete bar sound resource
    if (pBarSndBuffer)
        free(pBarSndBuffer);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete ball vertices
    if (g_pBallVertices)
    {
        free(g_pBallVertices);
        g_pBallVertices = 0;
    }

    // delete bar vertices
    if (g_pBarVertices)
    {
        free(g_pBarVertices);
        g_pBarVertices = 0;
    }

    // delete block vertices
    if (g_pBlockVertices)
    {
        free(g_pBlockVertices);
        g_pBlockVertices = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;

    // stop running ball rebound sound, if needed
    if (miniIsSoundPlaying(g_Ball.m_SoundID))
        miniStopSound(g_Ball.m_SoundID);

    // stop running bar explode sound, if needed
    if (miniIsSoundPlaying(g_Bar.m_SoundID))
        miniStopSound(g_Bar.m_SoundID);

    // release OpenAL interface
    miniReleaseSound(g_Ball.m_BufferID, g_Ball.m_SoundID);
    miniReleaseSound(g_Bar.m_BufferID,  g_Bar.m_SoundID);
    miniReleaseOpenAL(g_pOpenALDevice, g_pOpenALContext);
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    float zNear;
    float zFar;
    float w = view_w;
    float h = view_h;

    // configure screen
    GetScreen(&w, &h, &g_Screen);

    zNear = -1.0f;
    zFar  =  1.0f;

    glViewport(0, 0, view_w, view_h);
    ApplyOrtho(&g_Screen, &zNear, &zFar);

    // is bar out of bounds? (may happen after orientation changed)
    if (g_Bar.m_Geometry.m_Pos.m_X >= g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);
    else
    if (g_Bar.m_Geometry.m_Pos.m_X <= g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    int i;
    int collisionX;
    int collisionY;
    int allBlocksBroken;
    int blockCount;
    int rebuildLevel;
    int doPlaySound;

    // set bar y position (may change if the screen change)
    g_Bar.m_Geometry.m_Pos.m_Y = g_Screen.m_BarY;

    // is bar exploding?
    if (g_Bar.m_Exploding)
    {
        // move bar polygons
        g_Bar.m_R.m_X += g_Bar.m_ExpROffset.m_X * (timeStep_sec * 50.0f);
        g_Bar.m_R.m_Y += g_Bar.m_ExpROffset.m_Y * (timeStep_sec * 50.0f);
        g_Bar.m_L.m_X -= g_Bar.m_ExpLOffset.m_X * (timeStep_sec * 50.0f);
        g_Bar.m_L.m_Y += g_Bar.m_ExpLOffset.m_Y * (timeStep_sec * 50.0f);

        // explosion ends?
        if (g_Bar.m_L.m_Y > g_Screen.m_Top)
        {
            // reset values
            g_Bar.m_R.m_X     = 0.0f;
            g_Bar.m_R.m_Y     = 0.0f;
            g_Bar.m_L.m_X     = 0.0f;
            g_Bar.m_L.m_Y     = 0.0f;
            g_Bar.m_Exploding = 0;

            // reset x offset velocity
            if (g_Ball.m_Offset.m_X < 0.0f)
                g_Ball.m_Offset.m_X = -M_BALL_VELOCITY_X;
            else
                g_Ball.m_Offset.m_X =  M_BALL_VELOCITY_X;

            // reset y offset velocity
            if (g_Ball.m_Offset.m_Y < 0.0f)
                g_Ball.m_Offset.m_Y = -M_BALL_VELOCITY_Y;
            else
                g_Ball.m_Offset.m_Y =  M_BALL_VELOCITY_Y;
        }
    }

    // move ball
    g_Ball.m_Geometry.m_Pos.m_X += g_Ball.m_Offset.m_X * (timeStep_sec * 50.0f);
    g_Ball.m_Geometry.m_Pos.m_Y += g_Ball.m_Offset.m_Y * (timeStep_sec * 50.0f);

    collisionX      = 0;
    collisionY      = 0;
    allBlocksBroken = 1;
    blockCount      = g_BlockColumns * g_BlockLines;

    // iterate through blocks, and check each block to find a collision
    for (i = 0; i < blockCount; ++i)
    {
        // ignore broken blocks
        if (!g_Blocks[i].m_Visible)
            continue;

        // at least 1 block is visible
        allBlocksBroken = 0;

        // is ball in collision with block?
        if (miniCircleRectIntersect(&g_Ball.m_Geometry, &g_Blocks[i].m_Geometry))
        {
            // break the block
            g_Blocks[i].m_Visible = 0;

            // rebound happened on the left or right edge?
            if (g_Ball.m_Geometry.m_Pos.m_Y < g_Blocks[i].m_Geometry.m_Pos.m_Y &&
                g_Ball.m_Geometry.m_Pos.m_Y > g_Blocks[i].m_Geometry.m_Pos.m_Y - g_Blocks[i].m_Geometry.m_Size.m_Height)
                collisionX = 1;
            else
                collisionY = 1;
        }
    }

    rebuildLevel = 0;

    // edge reached?
    if (g_Ball.m_Geometry.m_Pos.m_X >= g_Screen.m_Right)
    {
        g_Ball.m_Geometry.m_Pos.m_X = g_Screen.m_Right;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (g_Ball.m_Geometry.m_Pos.m_X <= g_Screen.m_Left)
    {
        g_Ball.m_Geometry.m_Pos.m_X = g_Screen.m_Left;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }

    // ball was moving down and is colliding with bar?
    if (g_Ball.m_Offset.m_Y < 0.0f && miniCircleRectIntersect(&g_Ball.m_Geometry, &g_Bar.m_Geometry))
    {
        collisionY = 1;

        // can increase x velocity?
        if (g_Ball.m_Offset.m_X < g_Ball.m_Max.m_X)
            // increase x velocity
            if (g_Ball.m_Offset.m_X > 0.0f)
                g_Ball.m_Offset.m_X += g_Ball.m_Inc.m_X;
            else
                g_Ball.m_Offset.m_X -= g_Ball.m_Inc.m_X;

        // can increase y velocity?
        if (g_Ball.m_Offset.m_X < g_Ball.m_Max.m_Y)
            // increase y velocity
            if (g_Ball.m_Offset.m_Y > 0.0f)
                g_Ball.m_Offset.m_Y += g_Ball.m_Inc.m_Y;
            else
                g_Ball.m_Offset.m_Y -= g_Ball.m_Inc.m_Y;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    // edge reached?
    if (g_Ball.m_Geometry.m_Pos.m_Y >= g_Screen.m_Top)
    {
        g_Ball.m_Geometry.m_Pos.m_Y = g_Screen.m_Top;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (g_Ball.m_Geometry.m_Pos.m_Y <= g_Screen.m_Bottom)
    {
        g_Ball.m_Geometry.m_Pos.m_Y = g_Screen.m_Bottom;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;

        // bottom reached? Game over...
        g_Bar.m_Exploding = 1;

        miniPlaySound(g_Bar.m_SoundID);
    }

    // rebuild level, if needed
    if (rebuildLevel)
    {
        // go to next level
        ++g_Level;

        // last level reached?
        if (g_Level >= 5)
            g_Level = 0;

        // iterate through blocks to regenerate
        for (i = 0; i < blockCount; ++i)
            switch (g_Level)
            {
                case 0:
                g_Blocks[i].m_Visible = g_Level1[i];
                break;

                case 1:
                g_Blocks[i].m_Visible = g_Level2[i];
                break;

                case 2:
                g_Blocks[i].m_Visible = g_Level3[i];
                break;

                case 3:
                g_Blocks[i].m_Visible = g_Level4[i];
                break;

                case 4:
                g_Blocks[i].m_Visible = g_Level5[i];
                break;
            }
    }

    doPlaySound = 0;

    // collision on the x axis?
    if (collisionX)
    {
        g_Ball.m_Offset.m_X = -g_Ball.m_Offset.m_X;
        doPlaySound         =  1;
    }

    // collision on the y axis?
    if (collisionY)
    {
        g_Ball.m_Offset.m_Y = -g_Ball.m_Offset.m_Y;

        // is bar already exploding?
        if (!g_Bar.m_Exploding)
            doPlaySound = 1;
    }

    // play ball rebound sound
    if (doPlaySound == 1)
        miniPlaySound(g_Ball.m_SoundID);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;
    int          i;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set rotation axis
    t.m_X = g_Ball.m_Geometry.m_Pos.m_X;
    t.m_Y = g_Ball.m_Geometry.m_Pos.m_Y;
    t.m_Z = 0.0f;

    // calculate model view matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the ball
    miniDrawDisk(g_pBallVertices, g_BallVerticesCount, &g_VertexFormat, &g_Shader);

    // is bar currently exploding?
    if (!g_Bar.m_Exploding)
    {
        // set bar position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y;

        // get bar matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the bar
        miniDrawSurface(g_pBarVertices,
                        g_BarVerticesCount,
                        &g_VertexFormat,
                        &g_Shader);
    }
    else
    {
        // connect vertex buffer slots to shader
        glEnableVertexAttribArray(g_Shader.m_VertexSlot);
        glEnableVertexAttribArray(g_Shader.m_ColorSlot);

        // set bar left vertex position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X + g_Bar.m_L.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y + g_Bar.m_L.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the first polygon composing the broken bar
        miniDrawBuffer(g_pBarVertices,
                       3,
                       E_Triangles,
                       &g_VertexFormat,
                       &g_Shader);

        // set bar right vertex position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X + g_Bar.m_R.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y + g_Bar.m_R.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the second polygon composing the broken bar
        miniDrawBuffer(g_pBarVertices + g_VertexFormat.m_Stride,
                       3,
                       E_Triangles,
                       &g_VertexFormat,
                       &g_Shader);

        // disconnect vertex buffer slots from shader
        glDisableVertexAttribArray(g_Shader.m_VertexSlot);
        glDisableVertexAttribArray(g_Shader.m_ColorSlot);
    }

    // iterate through blocks to draw
    for (i = 0; i < g_BlockColumns * g_BlockLines; ++i)
    {
        // is block visible?
        if (!g_Blocks[i].m_Visible)
            continue;

        // set block position
        t.m_X = g_Blocks[i].m_Geometry.m_Pos.m_X;
        t.m_Y = g_Blocks[i].m_Geometry.m_Pos.m_Y;

        // get block matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect block model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the block
        miniDrawSurface(g_pBlockVertices,
                        g_BlockVerticesCount,
                        &g_VertexFormat,
                        &g_Shader);
    }

    miniEndScene();
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
    // is bar exploding?
    if (g_Bar.m_Exploding)
        return;

    // calculate bar next position
    g_Bar.m_Geometry.m_Pos.m_X += (x - prev_x) / (g_Screen.m_Width);

    // is bar out of bounds?
    if (g_Bar.m_Geometry.m_Pos.m_X >= g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);
    else
    if (g_Bar.m_Geometry.m_Pos.m_X <= g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);
}
//------------------------------------------------------------------------------
#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    int main()
    {
        ccrSet_GLES2_Init_Callback(on_GLES2_Init);
        ccrSet_GLES2_Final_Callback(on_GLES2_Final);
        ccrSet_GLES2_Size_Callback(on_GLES2_Size);
        ccrSet_GLES2_Update_Callback(on_GLES2_Update);
        ccrSet_GLES2_Render_Callback(on_GLES2_Render);
        ccrSet_GLES2_TouchBegin_Callback(on_GLES2_TouchBegin);
        ccrSet_GLES2_TouchMove_Callback(on_GLES2_TouchMove);
        ccrSet_GLES2_TouchEnd_Callback(on_GLES2_TouchEnd);

        ccrBegin_GLES2_Drawing();

        while (ccrGetEvent(false) != CCR_EVENT_QUIT);

        ccrEnd_GLES2_Drawing();

        return 0;
    }
#endif
//------------------------------------------------------------------------------
