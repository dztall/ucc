/*****************************************************************************
 * ==> Plasma effect demo ---------------------------------------------------*
 *****************************************************************************
 * Description : An old school plasma effect that use shader for calculation *
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

//------------------------------------------------------------------------------
// plasma vertex shader program
const char* g_pVSPlasma =
    "precision mediump float;"
    "attribute vec4  qr_vPosition;"
    "uniform   float qr_uTime;"
    "uniform   vec2  qr_uSize;"
    "uniform   mat4  qr_uProjection;"
    "uniform   mat4  qr_uModelview;"
    "varying   float qr_fTime;"
    "varying   vec2  qr_fSize;"
    "void main(void)"
    "{"
    "    qr_fSize    = qr_uSize;"
    "    qr_fTime    = qr_uTime;"
    "    gl_Position = qr_uProjection * qr_uModelview * qr_vPosition;"
    "}";
//------------------------------------------------------------------------------
// plasma fragment shader program. NOTE the above used plasma algorithm is based on Bidouille.org
// oldscool plasma effect post, available here:
// http://www.bidouille.org/prog/plasma
const char* g_pFSPlasma =
    "precision mediump float;"
    "varying float qr_fTime;"
    "varying vec2  qr_fSize;"
    "void main(void)"
    "{"
    "    const float pi      = 3.1415926535897932384626433832795;"
    "    vec2        vK      = vec2(0.34, 0.25);"
    "    vec2        vCoords = vec2((gl_FragCoord.x / qr_fSize.x) * 100.0,"
    "                               (gl_FragCoord.y / qr_fSize.y) * 100.0);"
    "    float v             = 0.0;"
    "    vec2  c             = vCoords * (vK - (vK / 2.0));"
    "    v                  += sin((c.x + qr_fTime));"
    "    v                  += sin((c.y + qr_fTime) / 2.0);"
    "    v                  += sin((c.x + c.y + qr_fTime) / 2.0);"
    "    c                  += vK / 2.0 * vec2(sin(qr_fTime / 3.0), cos(qr_fTime / 2.0));"
    "    v                  += sin(sqrt(c.x * c.x + c.y * c.y + 1.0) + qr_fTime);"
    "    v                   = v / 2.0;"
    "    vec3  col           = vec3(1, sin(pi * v), cos(pi * v));"
    "    gl_FragColor        = vec4(col * 0.5 + 0.5, 1);"
    "}";
//---------------------------------------------------------------------------
TMainForm* MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* pOwner) :
    TForm(pOwner),
    m_ShaderProgram(0),
    m_pSurfaceVB(0),
    m_SurfaceVertexCount(0),
    m_SurfaceWidth(10.0f),
    m_SurfaceHeight(12.5f),
    m_MaxTime(12.0f * M_PI),
    m_Time(0.0f),
    m_TimeSlot(0),
    m_SizeSlot(0),
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
    // get orthogonal matrix
    float left;
    float right;
    float top;
    float bottom;
    float zNear;
    float zFar;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // transform the width and height to keep the correct aspect ratio
    w /= h;
    h /= h;

    // calculate the screen bounds (in the OpenGL view)
    left   = -(w * 0.5f);
    right  =  (w * 0.5f);
    top    =  (h * 0.5f);
    bottom = -(h * 0.5f);
    zNear  = -1.0f;
    zFar   =  1.0f;

    MINI_Matrix matrix;
    miniGetOrtho(&left, &right, &bottom, &top, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
{
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(g_pVSPlasma, g_pFSPlasma);
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram,  "qr_vPosition");
    m_TimeSlot            = glGetUniformLocation(m_ShaderProgram, "qr_uTime");
    m_SizeSlot            = glGetUniformLocation(m_ShaderProgram, "qr_uSize");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // create the viewport
    CreateViewport(w, h);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 0;

    // generate surface
    miniCreateSurface(&m_SurfaceWidth,
                      &m_SurfaceHeight,
                      0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);

    // notify shader about screen size
    glUniform2f(m_SizeSlot, w, h);
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    // delete surface vertices
    if (m_pSurfaceVB)
    {
        free(m_pSurfaceVB);
        m_pSurfaceVB = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    // calculate next time
    m_Time += elapsedTime * 5.0f;

    // is time out of bounds?
    if (m_Time > m_MaxTime)
        m_Time -= m_MaxTime;

    // notify shader about elapsed time
    glUniform1f(m_TimeSlot, m_Time);
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    MINI_Matrix modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // simply get the identity matrix
    miniGetIdentity(&modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the plasma
    miniDrawSurface(m_pSurfaceVB,
                    m_SurfaceVertexCount,
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
