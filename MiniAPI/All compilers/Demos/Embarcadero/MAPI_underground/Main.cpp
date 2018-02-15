/*****************************************************************************
 * ==> Underground demo -----------------------------------------------------*
 *****************************************************************************
 * Description : An underground level demo based on ray casting engines like *
 *               Doom or Wolfenstein. Press the up or down arrow keys to     *
 *               walk, and the left or right arrow keys to rotate            *
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

#pragma package(smart_init)
#ifdef __llvm__
    #pragma link "glewSL.a"
#else
    #pragma link "glewSL.lib"
#endif
#pragma resource "*.dfm"

// resources
#define SOIL_TEXTURE_FILE "..\\..\\..\\..\\..\\Resources\\Soil_25_256x256.bmp"
#define WALL_TEXTURE_FILE "..\\..\\..\\..\\..\\Resources\\Wall_Tilleable_64_256x256.bmp"
#define CEIL_TEXTURE_FILE "..\\..\\..\\..\\..\\Resources\\Soil_2_25_256x256.bmp"

#ifdef MAP_MODE
    #define SPHERE_TEXTURE_FILE "..\\..\\..\\..\\..\\Resources\\cloud.bmp"
#endif

//---------------------------------------------------------------------------
const char* g_pLevelMap =
    "***   ***   ******"
    "********* *   *  *"
    "********* ** **  *"
    "*** * ************"
    "*** * ********* **"
    "***   ***  *    **"
    "********* *** ****"
    "********* *** ****"
    "***   *** *** ****";
//---------------------------------------------------------------------------
const char* miniVSTextured2 =
    "precision mediump float;"
    "attribute vec4 mini_vPosition;"
    "attribute vec4 mini_vColor;"
    "attribute vec2 mini_vTexCoord;"
    "uniform   mat4 mini_uProjection;"
    "uniform   mat4 mini_uView;"
    "uniform   mat4 mini_uModelview;"
    "varying   vec4 mini_fColor;"
    "varying   vec2 mini_fTexCoord;"
    "void main(void)"
    "{"
    "    mini_fColor    = mini_vColor;"
    "    mini_fTexCoord = mini_vTexCoord;"
    "    gl_Position  = mini_uProjection * mini_uView * mini_uModelview * mini_vPosition;"
    "}";
//---------------------------------------------------------------------------
TMainForm* MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* pOwner) :
    TForm(pOwner),
    m_pLevel(0),
    m_ShaderProgram(0),
    m_pSurfaceVB(0),
    m_SurfaceVertexCount(0),
    m_LevelItemWidth(0.05f),
    m_LevelItemHeight(0.05f),
    m_LevelItemLength(0.05f),
    m_PosVelocity(0.0f),
    m_DirVelocity(0.0f),
    m_Angle(0.0f),
    m_MapWidth(18),
    m_MapHeight(9),
    m_SoilTextureIndex(GL_INVALID_VALUE),
    m_WallTextureIndex(GL_INVALID_VALUE),
    m_CeilTextureIndex(GL_INVALID_VALUE),
    m_TexSamplerSlot(0),
    m_ViewUniform(0),
    m_ModelviewUniform(0),
    m_PreviousTime(0)
    #ifdef MAP_MODE
        ,
        m_pSphereVertices(0),
        m_pSphereIndexes(0),
        m_SphereTextureIndex(GL_INVALID_VALUE)
    #endif
{
    // calculate map length
    m_MapLength = m_MapWidth * m_MapHeight;

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
    switch (key)
    {
        case VK_LEFT:  m_DirVelocity = -5.0f; break;
        case VK_RIGHT: m_DirVelocity =  5.0f; break;
        case VK_UP:    m_PosVelocity = -0.3f; break;
        case VK_DOWN:  m_PosVelocity =  0.3f; break;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormKeyUp(TObject* pSender, WORD& key, TShiftState shift)
{
    switch (key)
    {
        case VK_LEFT:
        case VK_RIGHT: m_DirVelocity = 0.0f; break;

        case VK_UP:
        case VK_DOWN:  m_PosVelocity = 0.0f; break;
    }
}
//------------------------------------------------------------------------------
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
void TMainForm::CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.001f;
    const float zFar   = 1000.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
{
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniVSTextured2, miniGetFSTextured());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram, "mini_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram, "mini_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram, "mini_sColorMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_Player.m_Pos.m_X = 0.0f;
    m_Player.m_Pos.m_Y = 0.0f;
    m_Player.m_Pos.m_Z = 0.0f;
    m_Player.m_Radius  = 0.01f;

    m_Angle = 0.0f;

    miniGenerateLevel(g_pLevelMap,
                      m_MapWidth,
                      m_MapHeight,
                      &m_LevelItemWidth,
                      &m_LevelItemHeight,
                      &m_pLevel);

    #ifdef MAP_MODE
        m_SphereRadius                     = m_Player.m_Radius;
        m_SphereVertexFormat.m_UseNormals  = 0;
        m_SphereVertexFormat.m_UseTextures = 1;
        m_SphereVertexFormat.m_UseColors   = 1;

        miniCreateSphere(&m_SphereRadius,
                         5,
                         5,
                         0xFFFFFFFF,
                         &m_SphereVertexFormat,
                         &m_pSphereVertices,
                         &m_SphereVertexCount,
                         &m_pSphereIndexes,
                         &m_SphereIndexCount);
    #endif

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 1;
    m_VertexFormat.m_UseColors   = 1;

    // calculate the stride
    miniCalculateStride(&m_VertexFormat);

    // generate surface
    miniCreateSurface(&m_LevelItemWidth,
                      &m_LevelItemHeight,
                      0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);

    // load textures
    m_SoilTextureIndex = miniLoadTexture(SOIL_TEXTURE_FILE);
    m_WallTextureIndex = miniLoadTexture(WALL_TEXTURE_FILE);
    m_CeilTextureIndex = miniLoadTexture(CEIL_TEXTURE_FILE);

    #ifdef MAP_MODE
        m_SphereTextureIndex = miniLoadTexture(SPHERE_TEXTURE_FILE);
    #endif
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    #ifdef MAP_MODE
        // delete sphere vertex buffer
        if (m_pSphereVertices)
        {
            free(m_pSphereVertices);
            m_pSphereVertices = 0;
        }

        // delete sphere index buffer
        if (m_pSphereIndexes)
        {
            free(m_pSphereIndexes);
            m_pSphereIndexes = 0;
        }
    #endif

    // delete surface vertices
    if (m_pSurfaceVB)
    {
        free(m_pSurfaceVB);
        m_pSurfaceVB = 0;
    }

    // delete the level
    if (m_pLevel)
    {
        free(m_pLevel);
        m_pLevel = 0;
    }

    if (m_SoilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_SoilTextureIndex);

    m_SoilTextureIndex = GL_INVALID_VALUE;

    if (m_WallTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_WallTextureIndex);

    m_WallTextureIndex = GL_INVALID_VALUE;

    if (m_CeilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_CeilTextureIndex);

    m_CeilTextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    MINI_Vector3 newPos;

    // no time elapsed?
    if (!elapsedTime)
        return;

    // is player rotating?
    if (m_DirVelocity)
    {
        // calculate the player direction
        m_Angle += m_DirVelocity * elapsedTime;

        // validate and apply it
        if (m_Angle > M_PI * 2.0f)
            m_Angle -= M_PI * 2.0f;
        else
        if (m_Angle < 0.0f)
            m_Angle += M_PI * 2.0f;
    }

    // is player moving?
    if (m_PosVelocity)
    {
        newPos = m_Player.m_Pos;

        // calculate the next player position
        newPos.m_X += m_PosVelocity * cosf(m_Angle + (M_PI * 0.5f)) * elapsedTime;
        newPos.m_Z += m_PosVelocity * sinf(m_Angle + (M_PI * 0.5f)) * elapsedTime;

        // validate and apply it
        miniValidateNextPos(m_pLevel, m_LevelItemWidth, m_LevelItemHeight, m_MapLength, &m_Player, &newPos);
        m_Player.m_Pos = newPos;
    }
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    MINI_LevelDrawInfo drawInfo;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // configure the draw info
    drawInfo.m_pSurfaceVB         =  m_pSurfaceVB;
    drawInfo.m_SurfaceVertexCount =  m_SurfaceVertexCount;
    drawInfo.m_pVertexFormat      = &m_VertexFormat;
    drawInfo.m_pShader            = &m_Shader;
    drawInfo.m_ShaderProgram      =  m_ShaderProgram;
    drawInfo.m_SoilTextureIndex   =  m_SoilTextureIndex;
    drawInfo.m_WallTextureIndex   =  m_WallTextureIndex;
    drawInfo.m_CeilTextureIndex   =  m_CeilTextureIndex;

    #ifdef MAP_MODE
        drawInfo.m_pSphereVB           =  m_pSphereVertices;
        drawInfo.m_SphereRadius        =  m_SphereRadius;
        drawInfo.m_pSphereIndexes      =  m_pSphereIndexes;
        drawInfo.m_SphereVertexCount   =  m_SphereVertexCount;
        drawInfo.m_SphereIndexCount    =  m_SphereIndexCount;
        drawInfo.m_pSphereShader       = &m_Shader;
        drawInfo.m_pSphereVertexFormat = &m_SphereVertexFormat;
        drawInfo.m_SphereTextureIndex  =  m_SphereTextureIndex;
        drawInfo.m_MapMode             = 1;
    #else
        drawInfo.m_MapMode = 0;
    #endif

    // draw the level
    miniDrawLevel(m_pLevel,
                  m_LevelItemWidth,
                  m_LevelItemHeight,
                  m_LevelItemLength,
                  m_MapLength,
                 &m_Player,
                  m_Angle,
                 &drawInfo);

    miniEndScene();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnIdle(TObject* pSender, bool& done)
{
    FormPaint(pSender);
    done = false;
}
//---------------------------------------------------------------------------
