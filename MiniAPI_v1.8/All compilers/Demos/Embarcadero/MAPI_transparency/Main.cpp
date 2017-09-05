/*****************************************************************************
 * ==> Transparency demo ----------------------------------------------------*
 *****************************************************************************
 * Description : A transparency effect, press the left/right arrow keys to   *
 *               increase or decrease the rotation speed, press the up/down  *
 *               arrow keys to increase or decrease the transparency         *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
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

#define GLASS_TEXTURE_FILE "..\\..\\..\\..\\..\\Resources\\frosted_glass.bmp"
#define CLOUD_TEXTURE_FILE "..\\..\\..\\..\\..\\Resources\\cloud.bmp"

//---------------------------------------------------------------------------
TMainForm* MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* pOwner) :
    TForm(pOwner),
    m_ShaderProgram(0),
    m_pSphereVB(0),
    m_SphereVertexCount(0),
    m_pSphereIndexes(0),
    m_SphereIndexCount(0),
    m_pSurfaceVB(0),
    m_SurfaceVertexCount(0),
    m_SurfaceWidth(20.0f),
    m_SurfaceHeight(20.0f),
    m_SphereRadius(1.0f),
    m_Angle(0.0f),
    m_RotationSpeed(0.1f),
    m_AlphaLevel(0.5f),
    m_Time(0.0f),
    m_Interval(0.0f),
    m_FPS(15),
    m_GlassTextureIndex(GL_INVALID_VALUE),
    m_CloudTextureIndex(GL_INVALID_VALUE),
    m_TexSamplerSlot(0),
    m_AlphaSlot(0),
    m_ModelviewUniform(0),
    m_PreviousTime(0)
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
    switch (key)
    {
        case VK_LEFT:  m_RotationSpeed -= 0.005; break;
        case VK_RIGHT: m_RotationSpeed += 0.005; break;
        case VK_UP:    m_AlphaLevel    += 0.005; break;
        case VK_DOWN:  m_AlphaLevel    -= 0.005; break;
    }

    // limit alpha level to min and max bounds
    if (m_AlphaLevel > 1.0f)
        m_AlphaLevel = 1.0f;
    else
    if (m_AlphaLevel < 0.0f)
        m_AlphaLevel = 0.0f;
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
void TMainForm::CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 20.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
{
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSTexAlpha(), miniGetFSTexAlpha());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram,  "qr_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram,  "qr_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram,  "qr_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram,  "qr_sColorMap");
    m_AlphaSlot             = glGetUniformLocation(m_ShaderProgram, "qr_uAlpha");
    m_ModelviewUniform      = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 1;
    m_VertexFormat.m_UseColors   = 1;

    // generate surface
    miniCreateSurface(&m_SurfaceWidth,
                      &m_SurfaceHeight,
                      0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);

    // generate sphere
    miniCreateSphere(&m_SphereRadius,
                     10,
                     24,
                     0xFFFFFFFF,
                     &m_VertexFormat,
                     &m_pSphereVB,
                     &m_SphereVertexCount,
                     &m_pSphereIndexes,
                     &m_SphereIndexCount);

    // load textures
    m_GlassTextureIndex = miniLoadTexture(GLASS_TEXTURE_FILE);
    m_CloudTextureIndex = miniLoadTexture(CLOUD_TEXTURE_FILE);

    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    // delete sphere buffer index table
    if (m_pSphereIndexes)
    {
        free(m_pSphereIndexes);
        m_pSphereIndexes = 0;
    }

    // delete sphere vertices
    if (m_pSphereVB)
    {
        free(m_pSphereVB);
        m_pSphereVB = 0;
    }

    // delete surface vertices
    if (m_pSurfaceVB)
    {
        free(m_pSurfaceVB);
        m_pSurfaceVB = 0;
    }

    if (m_GlassTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_GlassTextureIndex);

    m_GlassTextureIndex = GL_INVALID_VALUE;

    if (m_CloudTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_CloudTextureIndex);

    m_CloudTextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    unsigned int frameCount = 0;

    // calculate next time
    m_Time += (elapsedTime * 1000.0f);

    // count frames to skip
    while (m_Time > m_Interval)
    {
        m_Time -= m_Interval;
        ++frameCount;
    }

    // calculate next rotation angle
    m_Angle += (m_RotationSpeed * frameCount);

    // is rotating angle out of bounds?
    while (m_Angle >= 6.28f)
        m_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    float        xAngle;
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  xRotateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure OpenGL to draw opaque objects (NOTE all opaque object should be drawn before,
    // because depth test should be deactivated later to allow alpha blending to work correctly)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -10.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(m_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // bind cloud texture
    glBindTexture(GL_TEXTURE_2D, m_CloudTextureIndex);

    // set alpha transparency level to draw surface (NOTE surface doesn't use transparency)
    glUniform1f(m_AlphaSlot, 1.0f);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // draw background image
    miniDrawSurface(m_pSurfaceVB,
                    m_SurfaceVertexCount,
                    &m_VertexFormat,
                    &m_Shader);

    // bind glass texture
    glBindTexture(GL_TEXTURE_2D, m_GlassTextureIndex);

    // populate sphere translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 90 degrees
    xAngle = 1.57075;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&m_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&xRotateMatrix,   &yRotateMatrix,   &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(m_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // set alpha transparency level to draw sphere
    glUniform1f(m_AlphaSlot, m_AlphaLevel);

    // configure OpenGL to draw transparency (NOTE all opaque objects should be drawn before,
    // because depth test should be disabled to allow alpha blending to work correctly)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw the transparent sphere
    miniDrawSphere(m_pSphereVB,
                   m_SphereVertexCount,
                   m_pSphereIndexes,
                   m_SphereIndexCount,
                   &m_VertexFormat,
                   &m_Shader);

    miniEndScene();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnIdle(TObject* pSender, bool& done)
{
    FormPaint(pSender);
    done = false;
}
//---------------------------------------------------------------------------
