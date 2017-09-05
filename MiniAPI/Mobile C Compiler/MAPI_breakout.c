/*****************************************************************************
 * ==> Breakout game --------------------------------------------------------*
 *****************************************************************************
 * Description : Simple breakout game.                                       *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

// supported platforms check (for now, only supports iOS and Android devices.
// NOTE Android support is theorical, never tested on a such device)
#if !defined(IOS) && !defined(ANDROID)
    #error "Not supported platform!"
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// OpenGL
#ifdef ANDROID
    #include <gles2/gl2.h>
    #include <gles2/gl2ext.h>
#endif
#ifdef IOS
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#endif

// mini API
#include "MiniAPI/MiniGeometry.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniShapes.h"
#include "MiniAPI/MiniCollision.h"
#include "MiniAPI/MiniShader.h"

// define ENABLE_SOUND to use first sound API, ENABLE_SOUND_OPENAL to use OpenAL,
// or comment line below disable sound
#define ENABLE_SOUND_OPENAL

#ifdef ENABLE_SOUND_OPENAL
    #include "MiniAPI/MiniPlayer.h"
#endif

#if defined(ENABLE_SOUND) || defined(ENABLE_SOUND_OPENAL)
    #ifdef ANDROID
        #define BALL_REBOUND_SOUND_FILE "/sdcard/C++ Compiler/samples/ball_rebound.wav"
        #define BAR_EXPLODE_SOUND_FILE  "/sdcard/C++ Compiler/samples/bar_explode.wav"
    #else
        #define BALL_REBOUND_SOUND_FILE "Resources/ball_rebound.wav"
        #define BAR_EXPLODE_SOUND_FILE  "Resources/bar_explode.wav"
    #endif
#endif

//------------------------------------------------------------------------------
typedef struct
{
    float   m_Left;
    float   m_Right;
    float   m_Top;
    float   m_Bottom;
    MG_Size m_Size;
    int     m_OriginalWidth;
    int     m_OriginalHeight;
    int     m_IsVertical;
} QR_Screen;
//------------------------------------------------------------------------------
typedef struct
{
    MG_Rect            m_Geometry;
    MG_Vector2         m_L;
    MG_Vector2         m_R;
    int                m_Exploding;
    MG_Vector2         m_ExpLOffset;
    MG_Vector2         m_ExpROffset;
    #ifdef ENABLE_SOUND
        int            m_SoundID;
    #elif defined(ENABLE_SOUND_OPENAL)
        ALuint         m_bufferID;
        ALuint         m_SoundID;
    #endif
} QR_Bar;
//------------------------------------------------------------------------------
typedef struct
{
    MG_Circle          m_Geometry;
    MG_Vector2         m_Offset;
    MG_Vector2         m_Inc;
    MG_Vector2         m_Max;
    #ifdef ENABLE_SOUND
        int            m_SoundID;
    #elif defined(ENABLE_SOUND_OPENAL)
        ALuint         m_bufferID;
        ALuint         m_SoundID;
    #endif
} QR_Ball;
//------------------------------------------------------------------------------
typedef struct
{
    MG_Rect m_Geometry;
    int     m_Visible;
} QR_Block;
//------------------------------------------------------------------------------
const int level1[15] =
{
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int level2[15] =
{
    1, 0, 0, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int level3[15] =
{
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int level4[15] =
{
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
};
//------------------------------------------------------------------------------
const int level5[15] =
{
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
};
//------------------------------------------------------------------------------
#ifndef ANDROID
    GLuint g_Renderbuffer, g_Framebuffer;
#endif
QR_Screen              g_Screen;
QR_Ball                g_Ball;
QR_Bar                 g_Bar;
QR_Block               g_Blocks[15];
float*                 g_pBarVertices       = 0;
int                    g_BarVerticesCount   = 0;
float*                 g_pBlockVertices     = 0;
int                    g_BlockVerticesCount = 0;
float*                 g_pBallVertices      = 0;
MV_Index*              g_pBallIndexes       = 0;
unsigned               g_BallVerticesCount  = 0;
unsigned               g_BallIndexCount     = 0;
int                    g_BlockColumns       = 5;
int                    g_BlockLines         = 3;
int                    g_Level              = 0;
float                  g_ScreenWidth        = 24.0f;
float                  g_ScreenHeight       = 36.0f;
MV_VertexFormat        g_VertexFormat;
GLuint                 g_ShaderProgram;
GLuint                 g_PositionSlot;
GLuint                 g_ColorSlot;
#ifdef ENABLE_SOUND_OPENAL
    static ALCdevice*  g_pOpenALDevice  = 0;
    static ALCcontext* g_pOpenALContext = 0;
#endif
//------------------------------------------------------------------------------
void GetScreen(float*     pWidth,
               float*     pHeight,
               int        originalWidth,
               int        originalHeight,
               QR_Screen* pScreen)
{
    // populate screen object
    pScreen->m_OriginalWidth  = originalWidth;
    pScreen->m_OriginalHeight = originalHeight;

    // is screen oriented horizontally or vertically?
    if (originalWidth > originalHeight)
    {
        // oriented horizontally
        pScreen->m_Left       = -(*pHeight / 2.0f);
        pScreen->m_Right      =  (*pHeight / 2.0f);
        pScreen->m_Top        =  (*pWidth  / 2.0f);
        pScreen->m_Bottom     = -(*pWidth  / 2.0f);
        pScreen->m_IsVertical = 0;
    }
    else
    {
        // oriented vertically
        pScreen->m_Left       = -(*pWidth  / 2.0f);
        pScreen->m_Right      =  (*pWidth  / 2.0f);
        pScreen->m_Top        =  (*pHeight / 2.0f);
        pScreen->m_Bottom     = -(*pHeight / 2.0f);
        pScreen->m_IsVertical = 1;
    }
}
//------------------------------------------------------------------------------
void ApplyOrtho(QR_Screen* pScreen, float* pNear, float* pFar)
{
    MG_Matrix ortho;

    // get orthogonal projection matrix
    GetOrtho(&pScreen->m_Left,
             &pScreen->m_Right,
             &pScreen->m_Bottom,
             &pScreen->m_Top,
             pNear,
             pFar,
             &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
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

    #ifndef ANDROID
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

    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(g_pVSColored, g_pFSColored);
    glUseProgram(g_ShaderProgram);

    // configure screen
    GetScreen(&g_ScreenWidth, &g_ScreenHeight, view_w, view_h, &g_Screen);

    // initialize ball data
    g_Ball.m_Geometry.m_Pos.m_X = 0.0f;
    g_Ball.m_Geometry.m_Pos.m_Y = 0.0f;
    g_Ball.m_Geometry.m_Radius  = 0.5f;
    g_Ball.m_Offset.m_X         = 0.2f;
    g_Ball.m_Offset.m_Y         = 0.2f;
    g_Ball.m_Inc.m_X            = 0.02;
    g_Ball.m_Inc.m_Y            = 0.03f;
    g_Ball.m_Max.m_X            = 1.0f;
    g_Ball.m_Max.m_Y            = 1.0f;
    #ifdef ENABLE_SOUND
        g_Ball.m_SoundID        = -1;
    #elif defined(ENABLE_SOUND_OPENAL)
        g_Ball.m_SoundID        = g_OpenALErrorID;
    #endif

    // initialize bar data
    g_Bar.m_Geometry.m_Pos.m_X       = 0.0f;
    g_Bar.m_Geometry.m_Pos.m_Y       = g_Screen.m_Bottom + g_Ball.m_Max.m_Y;
    g_Bar.m_Geometry.m_Size.m_Width  = 2.0;
    g_Bar.m_Geometry.m_Size.m_Height = 0.4f;
    g_Bar.m_R.m_X                    = 0.0f;
    g_Bar.m_R.m_Y                    = 0.0f;
    g_Bar.m_L.m_X                    = 0.0f;
    g_Bar.m_L.m_Y                    = 0.0f;
    g_Bar.m_ExpROffset.m_X           = 0.3f;
    g_Bar.m_ExpROffset.m_Y           = 0.4f;
    g_Bar.m_ExpLOffset.m_X           = 0.3f;
    g_Bar.m_ExpLOffset.m_Y           = 0.5f;
    #ifdef ENABLE_SOUND
        g_Bar.m_SoundID              = -1;
    #elif defined(ENABLE_SOUND_OPENAL)
        g_Bar.m_SoundID              = g_OpenALErrorID;
    #endif

    // calculate block width (block width * block count + inter space width * block count - 1)
    blockWidth = (g_BlockColumns * 2.0f) + ((g_BlockColumns - 1) * 0.25f);
    blockTop   =  g_Screen.m_Top - 5.0f;

    // iterate through block lines
    for (j = 0; j < g_BlockLines; ++j)
    {
        // iterate through block columns
        for (i = 0; i < g_BlockColumns; ++i)
        {
            index = (j * g_BlockColumns) + i;

            g_Blocks[index].m_Geometry.m_Pos.m_X       = -(blockWidth / 2.0f) + 1.0f + (i * 2.25f);
            g_Blocks[index].m_Geometry.m_Pos.m_Y       = blockTop - (j * 1.5f);
            g_Blocks[index].m_Geometry.m_Size.m_Width  = 2.0;
            g_Blocks[index].m_Geometry.m_Size.m_Height = 0.8f;
            g_Blocks[index].m_Visible                  = 1;
        }
    }

    // populate vertex format
    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere to draw
    CreateSphere(&g_Ball.m_Geometry.m_Radius,
                 20,
                 50,
                 0xFFFF00FF,
                 &g_VertexFormat,
                 &g_pBallVertices,
                 &g_BallVerticesCount,
                 &g_pBallIndexes,
                 &g_BallIndexCount);

    surfaceWidth  = 2.0f;
    surfaceHeight = 0.4f;

    CreateSurface(&surfaceWidth,
                  &surfaceHeight,
                  0xFF0033FF,
                  &g_VertexFormat,
                  &g_pBarVertices,
                  &g_BarVerticesCount);

    surfaceWidth  = 2.0f;
    surfaceHeight = 0.8f;

    // update some colors
    g_pBarVertices[11] = 0.4f;
    g_pBarVertices[12] = 0.0f;
    g_pBarVertices[18] = 0.2f;
    g_pBarVertices[19] = 0.4f;
    g_pBarVertices[25] = 0.2f;
    g_pBarVertices[26] = 0.4f;

    CreateSurface(&surfaceWidth,
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

    #ifdef ENABLE_SOUND
        // load sound files
        LoadSound(BALL_REBOUND_SOUND_FILE);
        LoadSound(BAR_EXPLODE_SOUND_FILE);
    #elif defined(ENABLE_SOUND_OPENAL)
        InitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

        // hard code file length values, not a good way but for now...
        ballSoundFileLen = 57416;
        barSoundFileLen  = 820644;

        // allocate buffers
        pBallSndBuffer = (unsigned char*)calloc(ballSoundFileLen, sizeof(unsigned char));
        pBarSndBuffer  = (unsigned char*)calloc(barSoundFileLen,  sizeof(unsigned char));

        // load ball sound file
        LoadSoundBuffer(BALL_REBOUND_SOUND_FILE,
                        ballSoundFileLen,
                        &pBallSndBuffer);

        // load bar sound file and get length
        LoadSoundBuffer(BAR_EXPLODE_SOUND_FILE,
                        barSoundFileLen,
                        &pBarSndBuffer);

        // create ball rebound sound file
        CreateSound(g_pOpenALDevice,
                    g_pOpenALContext,
                    pBallSndBuffer,
                    ballSoundFileLen,
                    48000,
                    &g_Ball.m_bufferID,
                    &g_Ball.m_SoundID);

        // create bar explode sound file
        CreateSound(g_pOpenALDevice,
                    g_pOpenALContext,
                    pBarSndBuffer,
                    barSoundFileLen,
                    48000,
                    &g_Bar.m_bufferID,
                    &g_Bar.m_SoundID);

        // delete ball sound resource
        if (pBallSndBuffer)
            free(pBallSndBuffer);

        // delete bar sound resource
        if (pBarSndBuffer)
            free(pBarSndBuffer);
    #endif
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete ball index table
    if (g_pBallIndexes)
    {
        free(g_pBallIndexes);
        g_pBallIndexes = 0;
    }

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

    #ifdef ENABLE_SOUND
        // stop running ball rebound sound, if needed
        if (g_Ball.m_SoundID != -1)
            StopSound(g_Ball.m_SoundID);

        // stop running bar explode sound, if needed
        if (g_Bar.m_SoundID != -1)
            StopSound(g_Bar.m_SoundID);

        // release sound interface
        UnloadSound(BALL_REBOUND_SOUND_FILE);
        UnloadSound(BAR_EXPLODE_SOUND_FILE);
        UnloadAllSoundAndBGM();
    #elif defined(ENABLE_SOUND_OPENAL)
        // stop running ball rebound sound, if needed
        if (IsSoundPlaying(g_Ball.m_SoundID))
            StopSound(g_Ball.m_SoundID);

        // stop running bar explode sound, if needed
        if (IsSoundPlaying(g_Bar.m_SoundID))
            StopSound(g_Bar.m_SoundID);

        // release OpenAL interface
        ReleaseSound(g_Ball.m_bufferID, g_Ball.m_SoundID);
        ReleaseSound(g_Bar.m_bufferID,  g_Bar.m_SoundID);
        ReleaseOpenAL(g_pOpenALDevice, g_pOpenALContext);
    #endif
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    float screenNear;
    float screenFar;

    // configure screen
    GetScreen(&g_ScreenWidth, &g_ScreenHeight, view_w, view_h, &g_Screen);

    // calculate bar start y position
    g_Bar.m_Geometry.m_Pos.m_Y = g_Screen.m_Bottom + 1.0f;

    screenNear = 1.0f;
    screenFar  = 20.0f;

    glViewport(0, 0, view_w, view_h);
    ApplyOrtho(&g_Screen, &screenNear, &screenFar);
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

    // is bar exploding?
    if (g_Bar.m_Exploding)
    {
        // move bar polygons
        g_Bar.m_R.m_X += g_Bar.m_ExpROffset.m_X;
        g_Bar.m_R.m_Y += g_Bar.m_ExpROffset.m_Y;
        g_Bar.m_L.m_X -= g_Bar.m_ExpLOffset.m_X;
        g_Bar.m_L.m_Y += g_Bar.m_ExpLOffset.m_Y;

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
                g_Ball.m_Offset.m_X = -0.2f;
            else
                g_Ball.m_Offset.m_X = 0.2f;

            // reset y offset velocity
            if (g_Ball.m_Offset.m_Y < 0.0f)
                g_Ball.m_Offset.m_Y = -0.2f;
            else
                g_Ball.m_Offset.m_Y = 0.2f;
        }
    }

    // move ball
    g_Ball.m_Geometry.m_Pos.m_X += g_Ball.m_Offset.m_X;
    g_Ball.m_Geometry.m_Pos.m_Y += g_Ball.m_Offset.m_Y;

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
        if (CircleRectIntersects(&g_Ball.m_Geometry, &g_Blocks[i].m_Geometry))
        {
            // break the block
            g_Blocks[i].m_Visible = 0;

            // do rebound happen on the y axis?
            if (g_Ball.m_Geometry.m_Pos.m_Y < g_Blocks[i].m_Geometry.m_Pos.m_Y ||
                g_Ball.m_Geometry.m_Pos.m_Y > g_Blocks[i].m_Geometry.m_Pos.m_Y
                        + g_Blocks[i].m_Geometry.m_Size.m_Height)
                collisionY = 1;
            else
            // do rebound happen on the x axis?
            if (g_Ball.m_Geometry.m_Pos.m_X < g_Blocks[i].m_Geometry.m_Pos.m_X ||
                g_Ball.m_Geometry.m_Pos.m_X > g_Blocks[i].m_Geometry.m_Pos.m_X
                        + g_Blocks[i].m_Geometry.m_Size.m_Width)
                collisionX = 1;
        }
    }

    rebuildLevel = 0;

    // edge reached?
    if (g_Ball.m_Geometry.m_Pos.m_X >= g_Screen.m_Right)
    {
        g_Ball.m_Geometry.m_Pos.m_X = g_Screen.m_Right;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (this ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (g_Ball.m_Geometry.m_Pos.m_X <= g_Screen.m_Left)
    {
        g_Ball.m_Geometry.m_Pos.m_X = g_Screen.m_Left;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (this ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }

    // ball was moving down and is colliding with bar?
    if (g_Ball.m_Offset.m_Y < 0.0f && CircleRectIntersects(&g_Ball.m_Geometry, &g_Bar.m_Geometry))
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

        // to avoid interference, rebuild level only if one edge is reached (this ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    // edge reached?
    if (g_Ball.m_Geometry.m_Pos.m_Y >= g_Screen.m_Top)
    {
        g_Ball.m_Geometry.m_Pos.m_Y = g_Screen.m_Top;
        collisionY            = 1;

        // to avoid interference, rebuild level only if one edge is reached (this ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (g_Ball.m_Geometry.m_Pos.m_Y <= g_Screen.m_Bottom)
    {
        g_Ball.m_Geometry.m_Pos.m_Y = g_Screen.m_Bottom;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (this ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;

        // bottom reached? Game over...
        g_Bar.m_Exploding = 1;

        #ifdef ENABLE_SOUND
            // play ball rebound sound
            g_Bar.m_SoundID = PlaySound(BAR_EXPLODE_SOUND_FILE, false);
        #elif defined(ENABLE_SOUND_OPENAL)
            PlaySound(g_Bar.m_SoundID);
        #endif
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
                g_Blocks[i].m_Visible = level1[i];
                break;

                case 1:
                g_Blocks[i].m_Visible = level2[i];
                break;

                case 2:
                g_Blocks[i].m_Visible = level3[i];
                break;

                case 3:
                g_Blocks[i].m_Visible = level4[i];
                break;

                case 4:
                g_Blocks[i].m_Visible = level5[i];
                break;
            }
    }

    doPlaySound = 0;

    // collision on the x axis?
    if (collisionX)
    {
        g_Ball.m_Offset.m_X = -g_Ball.m_Offset.m_X;
        doPlaySound         = 1;
    }

    // collision on the y axis?
    if (collisionY)
    {
        g_Ball.m_Offset.m_Y = -g_Ball.m_Offset.m_Y;

        // is bar already exploding?
        if (!g_Bar.m_Exploding)
            doPlaySound = 1;
    }

    #ifdef ENABLE_SOUND
        // play ball rebound sound
        if (doPlaySound == 1)
            g_Ball.m_SoundID = PlaySound(BALL_REBOUND_SOUND_FILE, false);
    #elif defined(ENABLE_SOUND_OPENAL)
        // play ball rebound sound
        if (doPlaySound == 1)
            PlaySound(g_Ball.m_SoundID);
    #endif
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    MG_Vector3 t;
    MG_Matrix  modelViewMatrix;
    int        stride;
    int        i;
    GLvoid*    pCoords;
    GLvoid*    pColors;

    // clear scene background and depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // set rotation axis
    t.m_X = g_Ball.m_Geometry.m_Pos.m_X;
    t.m_Y = g_Ball.m_Geometry.m_Pos.m_Y;
    t.m_Z = -5.0f;

    // calculate model view matrix (it's a rotation on the y axis)
    GetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // get vertex and color slots
    g_PositionSlot = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_vColor");

    // calculate vertex stride
    stride = g_VertexFormat.m_Stride;

    // enable position and color slots
    glEnableVertexAttribArray(g_PositionSlot);
    glEnableVertexAttribArray(g_ColorSlot);

    // iterate through vertex fan buffers to draw
    for (i = 0; (unsigned)i < g_BallIndexCount; ++i)
    {
        // get next vertices fan buffer
        pCoords = &g_pBallVertices[g_pBallIndexes[i].m_Start];
        pColors = &g_pBallVertices[g_pBallIndexes[i].m_Start + 3];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

        // draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, g_pBallIndexes[i].m_Length / stride);
    }

    // set bar z position
    t.m_Z = -5.0f;

    // is bar currently exploding?
    if (!g_Bar.m_Exploding)
    {
        // set bar position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y;

        // get bar matrix
        GetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // get bar polygon buffer
        pCoords = &g_pBarVertices[0];
        pColors = &g_pBarVertices[3];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

        // draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else
    {
        // set bar left vertex position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X + g_Bar.m_L.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y + g_Bar.m_L.m_Y;

        // get bar vertex matrix
        GetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // get bar left polygon buffer
        pCoords = &g_pBarVertices[0];
        pColors = &g_pBarVertices[3];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

        // draw it
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // set bar right vertex position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X + g_Bar.m_R.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y + g_Bar.m_R.m_Y;

        // get bar vertex matrix
        GetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // get bar right polygon buffer
        pCoords = &g_pBarVertices[stride];
        pColors = &g_pBarVertices[stride + 3];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

        // draw it
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // get block polygon buffer
    pCoords = &g_pBlockVertices[0];
    pColors = &g_pBlockVertices[3];

    // connect buffer to shader
    glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
    glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

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
        GetTranslateMatrix(&t, &modelViewMatrix);

        // connect block model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // disconnect slots from shader
    glDisableVertexAttribArray(g_PositionSlot);
    glDisableVertexAttribArray(g_ColorSlot);
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
    g_Bar.m_Geometry.m_Pos.m_X += (x - prev_x) / 10.0f;
    g_Bar.m_Geometry.m_Pos.m_Y  = g_Screen.m_Bottom + 1.0f;

    // is bar out of bounds?
    if (g_Bar.m_Geometry.m_Pos.m_X >= g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width / 2.0f))
        g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width / 2.0f);
    else
    if (g_Bar.m_Geometry.m_Pos.m_X <= g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width / 2.0f))
        g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width / 2.0f);
}
//------------------------------------------------------------------------------
#ifdef IOS
    void on_GLES2_DeviceRotate(int orientation)
    {}
#endif
//------------------------------------------------------------------------------

