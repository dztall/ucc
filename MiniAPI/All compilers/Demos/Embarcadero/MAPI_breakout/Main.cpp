/*****************************************************************************
 * ==> Breakout game demo ---------------------------------------------------*
 *****************************************************************************
 * Description : A simple breakout game                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#include <vcl.h>
#pragma hdrstop
#include "Main.h"

// std
#include <math.h>
#include <algorithm>

#pragma package(smart_init)
#ifdef __llvm__
    #pragma link "glewSL.a"
#else
    #pragma link "glewSL.lib"
#endif
#pragma link "OpenAL32E.lib"
#pragma resource "*.dfm"

// resources
#define BALL_REBOUND_SOUND_FILE "..\\..\\..\\..\\..\\Resources\\ball_rebound.wav"
#define BAR_EXPLODE_SOUND_FILE  "..\\..\\..\\..\\..\\Resources\\bar_explode.wav"

//------------------------------------------------------------------------------
#define M_BALL_VELOCITY_X 0.005f
#define M_BALL_VELOCITY_Y 0.005f
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
//---------------------------------------------------------------------------
TMainForm* MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* pOwner) :
    TForm(pOwner),
    m_pBarVertices(0),
    m_BarVerticesCount(0),
    m_pBlockVertices(0),
    m_BlockVerticesCount(0),
    m_pBallVertices(0),
    m_BallVerticesCount(0),
    m_BlockColumns(5),
    m_BlockLines(3),
    m_Level(0),
    m_pOpenALDevice(0),
    m_pOpenALContext(0)
{
    // enable OpenGL
    EnableOpenGL(Handle, &m_hDC, &m_hRC);

    // stop GLEW crashing on OSX :-/
    glewExperimental = GL_TRUE;

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        // shutdown OpenGL
        DisableOpenGL(Handle, m_hDC, m_hRC);

        // close the app
        Application->Terminate();
    }
}
//---------------------------------------------------------------------------
__fastcall TMainForm::~TMainForm()
{
    DeleteScene();
    DisableOpenGL(Handle, m_hDC, m_hRC);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    // initialize the scene
    InitScene(ClientWidth, ClientHeight);

    // initialize the timer
    m_PreviousTime = ::GetTickCount();

    // listen the application idle
    Application->OnIdle = OnIdle;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject* pSender)
{
    // update the viewport
    CreateViewport(ClientWidth, ClientHeight);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormPaint(TObject* pSender)
{
    // calculate time interval
    const unsigned __int64 now            = ::GetTickCount();
    const double           elapsedTime    = (now - m_PreviousTime) / 1000.0;
                           m_PreviousTime =  now;

    // update and draw the scene
    UpdateScene(elapsedTime);
    DrawScene();

    ::SwapBuffers(m_hDC);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormKeyDown(TObject* pSender, WORD& key, TShiftState shift)
{
    float velocity = 0.0f;

    // is bar exploding?
    if (m_Bar.m_Exploding)
        return;

    switch (key)
    {
        case VK_LEFT:  velocity = -0.05f; break;
        case VK_RIGHT: velocity =  0.05f; break;
    }

    // calculate bar next position
    m_Bar.m_Geometry.m_Pos.m_X += velocity;

    // is bar out of bounds?
    if (m_Bar.m_Geometry.m_Pos.m_X >= m_Screen.m_Right - (m_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        m_Bar.m_Geometry.m_Pos.m_X = m_Screen.m_Right - (m_Bar.m_Geometry.m_Size.m_Width * 0.5f);
    else
    if (m_Bar.m_Geometry.m_Pos.m_X <= m_Screen.m_Left + (m_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        m_Bar.m_Geometry.m_Pos.m_X = m_Screen.m_Left + (m_Bar.m_Geometry.m_Size.m_Width * 0.5f);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormMouseMove(TObject* pSender, TShiftState shift, int x, int y)
{
    // is bar exploding?
    if (m_Bar.m_Exploding)
        return;

    // calculate bar next position (NOTE use client height, because the viewport is always uses the
    // height as reference)
    m_Bar.m_Geometry.m_Pos.m_X = m_Screen.m_Left + (float(x) / float(ClientHeight));

    // is bar out of bounds?
    if (m_Bar.m_Geometry.m_Pos.m_X >= m_Screen.m_Right - (m_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        m_Bar.m_Geometry.m_Pos.m_X = m_Screen.m_Right - (m_Bar.m_Geometry.m_Size.m_Width * 0.5f);
    else
    if (m_Bar.m_Geometry.m_Pos.m_X <= m_Screen.m_Left + (m_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        m_Bar.m_Geometry.m_Pos.m_X = m_Screen.m_Left + (m_Bar.m_Geometry.m_Size.m_Width * 0.5f);
}
//---------------------------------------------------------------------------
void TMainForm::EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    // get the device context
    *hDC = ::GetDC(hWnd);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    // set the pixel format for the device context
    iFormat = ChoosePixelFormat(*hDC, &pfd);
    SetPixelFormat(*hDC, iFormat, &pfd);

    // create and enable the OpenGL render context
    *hRC = wglCreateContext(*hDC);
    wglMakeCurrent(*hDC, *hRC);
}
//------------------------------------------------------------------------------
void TMainForm::DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
//------------------------------------------------------------------------------
void TMainForm::GetScreen(float width, float height, IScreen& screen)
{
    // transform the width and height to keep the correct aspect ratio
    width  /= height;
    height /= height;

    // calculate the screen bounds (in the OpenGL view)
    screen.m_Left   = -(width  * 0.5f);
    screen.m_Right  =  (width  * 0.5f);
    screen.m_Top    =  (height * 0.5f);
    screen.m_Bottom = -(height * 0.5f);

    // calculate bar start y position
    screen.m_BarY = m_Screen.m_Bottom + 0.05f;
}
//------------------------------------------------------------------------------
void TMainForm::CreateViewport(float w, float h)
{
    const float       zNear = -1.0f;
    const float       zFar  =  1.0f;
          MINI_Matrix ortho;

    // configure screen
    GetScreen(w, h, m_Screen);

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // get orthogonal projection matrix
    miniGetOrtho(&m_Screen.m_Left,
                 &m_Screen.m_Right,
                 &m_Screen.m_Bottom,
                 &m_Screen.m_Top,
                 &zNear,
                 &zFar,
                 &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
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

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    // get vertex and color slots
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot  = glGetAttribLocation(m_ShaderProgram, "mini_vColor");

    // configure screen
    CreateViewport(w, h);

    // initialize ball data
    m_Ball.m_Geometry.m_Pos.m_X = 0.0f;
    m_Ball.m_Geometry.m_Pos.m_Y = 0.0f;
    m_Ball.m_Geometry.m_Radius  = 0.015f;
    m_Ball.m_Offset.m_X         = M_BALL_VELOCITY_X;
    m_Ball.m_Offset.m_Y         = M_BALL_VELOCITY_Y;
    m_Ball.m_Inc.m_X            = 0.001f;
    m_Ball.m_Inc.m_Y            = 0.0015f;
    m_Ball.m_Max.m_X            = 0.3f;
    m_Ball.m_Max.m_Y            = 0.3f;
    m_Ball.m_SoundID            = M_OPENAL_ERROR_ID;

    // initialize bar data
    m_Bar.m_Geometry.m_Pos.m_X       = 0.0f;
    m_Bar.m_Geometry.m_Pos.m_Y       = m_Screen.m_Bottom + m_Ball.m_Max.m_Y;
    m_Bar.m_Geometry.m_Size.m_Width  = 0.06f;
    m_Bar.m_Geometry.m_Size.m_Height = 0.0125f;
    m_Bar.m_R.m_X                    = 0.0f;
    m_Bar.m_R.m_Y                    = 0.0f;
    m_Bar.m_L.m_X                    = 0.0f;
    m_Bar.m_L.m_Y                    = 0.0f;
    m_Bar.m_ExpROffset.m_X           = 0.01f;
    m_Bar.m_ExpROffset.m_Y           = 0.0125f;
    m_Bar.m_ExpLOffset.m_X           = 0.01f;
    m_Bar.m_ExpLOffset.m_Y           = 0.015f;
    m_Bar.m_SoundID                  = M_OPENAL_ERROR_ID;

    // calculate block width (block width * block count + inter space width * block count - 1)
    blockWidth = (m_BlockColumns * 0.06f) + ((m_BlockColumns - 1) * 0.0075f);
    blockTop   =  m_Screen.m_Top - 0.15f;

    // iterate through block lines
    for (j = 0; j < m_BlockLines; ++j)
    {
        // iterate through block columns
        for (i = 0; i < m_BlockColumns; ++i)
        {
            index = (j * m_BlockColumns) + i;

            m_Blocks[index].m_Geometry.m_Pos.m_X       = -(blockWidth * 0.5f) + 0.03f + (i * 0.065f);
            m_Blocks[index].m_Geometry.m_Pos.m_Y       =   blockTop - (j * 0.045f);
            m_Blocks[index].m_Geometry.m_Size.m_Width  =   0.06f;
            m_Blocks[index].m_Geometry.m_Size.m_Height =   0.0125f;
            m_Blocks[index].m_Visible                  =   1;
        }
    }

    // populate vertex format
    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate disk to draw
    miniCreateDisk(0.0f,
                   0.0f,
                   m_Ball.m_Geometry.m_Radius,
                   20,
                   0xFFFF00FF,
                  &m_VertexFormat,
                  &m_pBallVertices,
                  &m_BallVerticesCount);

    surfaceWidth  = 0.06f;
    surfaceHeight = 0.0125f;

    // create a surface for the bar
    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                       0xFF0033FF,
                      &m_VertexFormat,
                      &m_pBarVertices,
                      &m_BarVerticesCount);

    surfaceWidth  = 0.06f;
    surfaceHeight = 0.025f;

    // update some colors
    m_pBarVertices[11] = 0.4f;
    m_pBarVertices[12] = 0.0f;
    m_pBarVertices[18] = 0.2f;
    m_pBarVertices[19] = 0.4f;
    m_pBarVertices[25] = 0.2f;
    m_pBarVertices[26] = 0.4f;

    // create a surface for the blocks
    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                      0x0000FFFF,
                      &m_VertexFormat,
                      &m_pBlockVertices,
                      &m_BlockVerticesCount);

    // update some colors
    m_pBlockVertices[10] = 0.2f;
    m_pBlockVertices[11] = 0.2f;
    m_pBlockVertices[12] = 0.9f;
    m_pBlockVertices[17] = 0.0f;
    m_pBlockVertices[18] = 0.1f;
    m_pBlockVertices[19] = 0.3f;
    m_pBlockVertices[24] = 0.3f;
    m_pBlockVertices[25] = 0.5f;
    m_pBlockVertices[26] = 0.8f;

    miniInitializeOpenAL(&m_pOpenALDevice, &m_pOpenALContext);

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
    miniCreateSound(m_pOpenALDevice,
                    m_pOpenALContext,
                    pBallSndBuffer,
                    ballSoundFileLen,
                    48000,
                    &m_Ball.m_BufferID,
                    &m_Ball.m_SoundID);

    // create bar explode sound file
    miniCreateSound(m_pOpenALDevice,
                    m_pOpenALContext,
                    pBarSndBuffer,
                    barSoundFileLen,
                    48000,
                    &m_Bar.m_BufferID,
                    &m_Bar.m_SoundID);

    // delete ball sound resource
    if (pBallSndBuffer)
        free(pBallSndBuffer);

    // delete bar sound resource
    if (pBarSndBuffer)
        free(pBarSndBuffer);
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    // delete ball vertices
    if (m_pBallVertices)
    {
        free(m_pBallVertices);
        m_pBallVertices = 0;
    }

    // delete bar vertices
    if (m_pBarVertices)
    {
        free(m_pBarVertices);
        m_pBarVertices = 0;
    }

    // delete block vertices
    if (m_pBlockVertices)
    {
        free(m_pBlockVertices);
        m_pBlockVertices = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;

    // stop running ball rebound sound, if needed
    if (miniIsSoundPlaying(m_Ball.m_SoundID))
        miniStopSound(m_Ball.m_SoundID);

    // stop running bar explode sound, if needed
    if (miniIsSoundPlaying(m_Bar.m_SoundID))
        miniStopSound(m_Bar.m_SoundID);

    // release OpenAL interface
    miniReleaseSound(m_Ball.m_BufferID, m_Ball.m_SoundID);
    miniReleaseSound(m_Bar.m_BufferID,  m_Bar.m_SoundID);
    miniReleaseOpenAL(m_pOpenALDevice, m_pOpenALContext);
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    int   i;
    int   collisionX;
    int   collisionY;
    int   allBlocksBroken;
    int   blockCount;
    int   rebuildLevel;
    int   doPlaySound;
    float left;
    float right;
    float top;
    float roundedLeft;
    float roundedRight;
    float roundedTop;

    // set bar y position (may change if the screen change)
    m_Bar.m_Geometry.m_Pos.m_Y = m_Screen.m_BarY;

    // is bar exploding?
    if (m_Bar.m_Exploding)
    {
        // move bar polygons
        m_Bar.m_R.m_X += m_Bar.m_ExpROffset.m_X * (elapsedTime * 50.0f);
        m_Bar.m_R.m_Y += m_Bar.m_ExpROffset.m_Y * (elapsedTime * 50.0f);
        m_Bar.m_L.m_X -= m_Bar.m_ExpLOffset.m_X * (elapsedTime * 50.0f);
        m_Bar.m_L.m_Y += m_Bar.m_ExpLOffset.m_Y * (elapsedTime * 50.0f);

        // explosion ends?
        if (m_Bar.m_L.m_Y > m_Screen.m_Top)
        {
            // reset values
            m_Bar.m_R.m_X     = 0.0f;
            m_Bar.m_R.m_Y     = 0.0f;
            m_Bar.m_L.m_X     = 0.0f;
            m_Bar.m_L.m_Y     = 0.0f;
            m_Bar.m_Exploding = 0;

            // reset x offset velocity
            if (m_Ball.m_Offset.m_X < 0.0f)
                m_Ball.m_Offset.m_X = -M_BALL_VELOCITY_X;
            else
                m_Ball.m_Offset.m_X =  M_BALL_VELOCITY_X;

            // reset y offset velocity
            if (m_Ball.m_Offset.m_Y < 0.0f)
                m_Ball.m_Offset.m_Y = -M_BALL_VELOCITY_Y;
            else
                m_Ball.m_Offset.m_Y =  M_BALL_VELOCITY_Y;
        }
    }

    // move ball
    m_Ball.m_Geometry.m_Pos.m_X += m_Ball.m_Offset.m_X * (elapsedTime * 50.0f);
    m_Ball.m_Geometry.m_Pos.m_Y += m_Ball.m_Offset.m_Y * (elapsedTime * 50.0f);

    collisionX      = 0;
    collisionY      = 0;
    allBlocksBroken = 1;
    blockCount      = m_BlockColumns * m_BlockLines;

    // iterate through blocks, and check each block to find a collision
    for (i = 0; i < blockCount; ++i)
    {
        // ignore broken blocks
        if (!m_Blocks[i].m_Visible)
            continue;

        // at least 1 block is visible
        allBlocksBroken = 0;

        // is ball in collision with block?
        if (miniCircleRectIntersect(&m_Ball.m_Geometry, &m_Blocks[i].m_Geometry))
        {
            // break the block
            m_Blocks[i].m_Visible = 0;

            // rebound happened on the left or right edge?
            if (m_Ball.m_Geometry.m_Pos.m_Y < m_Blocks[i].m_Geometry.m_Pos.m_Y &&
                m_Ball.m_Geometry.m_Pos.m_Y > m_Blocks[i].m_Geometry.m_Pos.m_Y - m_Blocks[i].m_Geometry.m_Size.m_Height)
                collisionX = 1;
            else
                collisionY = 1;
        }
    }

    rebuildLevel = 0;

    // calculate the rounded value of the screen edges, otherwise rounding errors may drive to
    // incorrect collision detections
    left  = m_Screen.m_Left  + (m_Ball.m_Geometry.m_Radius * 2.0f);
    right = m_Screen.m_Right - (m_Ball.m_Geometry.m_Radius * 2.0f);
    top   = m_Screen.m_Top   - (m_Ball.m_Geometry.m_Radius * 2.5f);
    miniRoundToExp(&left,  3, &roundedLeft);
    miniRoundToExp(&right, 3, &roundedRight);
    miniRoundToExp(&top,   3, &roundedTop);

    // left or right edge reached?
    if (m_Ball.m_Geometry.m_Pos.m_X > roundedRight)
    {
        m_Ball.m_Geometry.m_Pos.m_X = roundedRight;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (m_Ball.m_Geometry.m_Pos.m_X < roundedLeft)
    {
        m_Ball.m_Geometry.m_Pos.m_X = roundedLeft;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }

    // ball was moving down and is colliding with bar?
    if (m_Ball.m_Offset.m_Y < 0.0f && miniCircleRectIntersect(&m_Ball.m_Geometry, &m_Bar.m_Geometry))
    {
        collisionY = 1;

        // can increase x velocity?
        if (m_Ball.m_Offset.m_X < m_Ball.m_Max.m_X)
            // increase x velocity
            if (m_Ball.m_Offset.m_X > 0.0f)
                m_Ball.m_Offset.m_X += m_Ball.m_Inc.m_X;
            else
                m_Ball.m_Offset.m_X -= m_Ball.m_Inc.m_X;

        // can increase y velocity?
        if (m_Ball.m_Offset.m_X < m_Ball.m_Max.m_Y)
            // increase y velocity
            if (m_Ball.m_Offset.m_Y > 0.0f)
                m_Ball.m_Offset.m_Y += m_Ball.m_Inc.m_Y;
            else
                m_Ball.m_Offset.m_Y -= m_Ball.m_Inc.m_Y;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    // top edge reached?
    if (m_Ball.m_Geometry.m_Pos.m_Y > roundedTop)
    {
        m_Ball.m_Geometry.m_Pos.m_Y = roundedTop;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    // bottom edge reached?
    if (m_Ball.m_Geometry.m_Pos.m_Y < m_Screen.m_Bottom)
    {
        m_Ball.m_Geometry.m_Pos.m_Y = m_Screen.m_Bottom;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;

        // bottom reached? Game over...
        m_Bar.m_Exploding = 1;

        miniPlaySound(m_Bar.m_SoundID);
    }

    // rebuild level, if needed
    if (rebuildLevel)
    {
        // go to next level
        ++m_Level;

        // last level reached?
        if (m_Level >= 5)
            m_Level = 0;

        // iterate through blocks to regenerate
        for (i = 0; i < blockCount; ++i)
            switch (m_Level)
            {
                case 0:
                m_Blocks[i].m_Visible = g_Level1[i];
                break;

                case 1:
                m_Blocks[i].m_Visible = g_Level2[i];
                break;

                case 2:
                m_Blocks[i].m_Visible = g_Level3[i];
                break;

                case 3:
                m_Blocks[i].m_Visible = g_Level4[i];
                break;

                case 4:
                m_Blocks[i].m_Visible = g_Level5[i];
                break;
            }
    }

    doPlaySound = 0;

    // collision on the x axis?
    if (collisionX)
    {
        m_Ball.m_Offset.m_X = -m_Ball.m_Offset.m_X;
        doPlaySound         =  1;
    }

    // collision on the y axis?
    if (collisionY)
    {
        m_Ball.m_Offset.m_Y = -m_Ball.m_Offset.m_Y;

        // is bar already exploding?
        if (!m_Bar.m_Exploding)
            doPlaySound = 1;
    }

    // play ball rebound sound
    if (doPlaySound == 1)
        miniPlaySound(m_Ball.m_SoundID);
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;
    int          i;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set ball position
    t.m_X = m_Ball.m_Geometry.m_Pos.m_X;
    t.m_Y = m_Ball.m_Geometry.m_Pos.m_Y;
    t.m_Z = 0.0f;

    // calculate model view matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the ball
    miniDrawDisk(m_pBallVertices, m_BallVerticesCount, &m_VertexFormat, &m_Shader);

    // is bar currently exploding?
    if (!m_Bar.m_Exploding)
    {
        // set bar position
        t.m_X = m_Bar.m_Geometry.m_Pos.m_X;
        t.m_Y = m_Bar.m_Geometry.m_Pos.m_Y;

        // get bar matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the bar
        miniDrawSurface(m_pBarVertices,
                        m_BarVerticesCount,
                        &m_VertexFormat,
                        &m_Shader);
    }
    else
    {
        // connect vertex buffer slots to shader
        glEnableVertexAttribArray(m_Shader.m_VertexSlot);
        glEnableVertexAttribArray(m_Shader.m_ColorSlot);

        // set bar left vertex position
        t.m_X = m_Bar.m_Geometry.m_Pos.m_X + m_Bar.m_L.m_X;
        t.m_Y = m_Bar.m_Geometry.m_Pos.m_Y + m_Bar.m_L.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the first polygon composing the broken bar
        miniDrawBuffer(m_pBarVertices,
                       3,
                       E_Triangles,
                       &m_VertexFormat,
                       &m_Shader);

        // set bar right vertex position
        t.m_X = m_Bar.m_Geometry.m_Pos.m_X + m_Bar.m_R.m_X;
        t.m_Y = m_Bar.m_Geometry.m_Pos.m_Y + m_Bar.m_R.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the second polygon composing the broken bar
        miniDrawBuffer(m_pBarVertices + m_VertexFormat.m_Stride,
                       3,
                       E_Triangles,
                       &m_VertexFormat,
                       &m_Shader);

        // disconnect vertex buffer slots from shader
        glDisableVertexAttribArray(m_Shader.m_VertexSlot);
        glDisableVertexAttribArray(m_Shader.m_ColorSlot);
    }

    // iterate through blocks to draw
    for (i = 0; i < m_BlockColumns * m_BlockLines; ++i)
    {
        // is block visible?
        if (!m_Blocks[i].m_Visible)
            continue;

        // set block position
        t.m_X = m_Blocks[i].m_Geometry.m_Pos.m_X;
        t.m_Y = m_Blocks[i].m_Geometry.m_Pos.m_Y;

        // get block matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect block model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the block
        miniDrawSurface(m_pBlockVertices,
                        m_BlockVerticesCount,
                        &m_VertexFormat,
                        &m_Shader);
    }

    miniEndScene();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnIdle(TObject* pSender, bool& done)
{
    FormPaint(pSender);
    done = false;
}
//---------------------------------------------------------------------------
