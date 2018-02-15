/*****************************************************************************
 * ==> Rover demo -----------------------------------------------------------*
 *****************************************************************************
 * Description : A simple rover model, press the left or right arrow keys to *
 *               increase or decrease the rotation speed                     *
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

//---------------------------------------------------------------------------
TMainForm* MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* pOwner) :
    TForm(pOwner),
    m_ShaderProgram(0),
    m_pVertices(0),
    m_VertexCount(0),
    m_pMdlCmds(0),
    m_pIndexes(0),
    m_IndexCount(0),
    m_Angle(0.0f),
    m_RotationSpeed(0.1f),
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
    }
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
    const float zFar   = 100.0f;
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
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot  = glGetAttribLocation(m_ShaderProgram, "mini_vColor");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // create the rover
    miniCreateRover(&m_VertexFormat,
                    &m_pVertices,
                    &m_VertexCount,
                    &m_pMdlCmds,
                    &m_pIndexes,
                    &m_IndexCount);
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    // delete model commands
    if (m_pMdlCmds)
    {
        free(m_pMdlCmds);
        m_pMdlCmds = 0;
    }

    // delete buffer index table
    if (m_pIndexes)
    {
        free(m_pIndexes);
        m_pIndexes = 0;
    }

    // delete vertices
    if (m_pVertices)
    {
        free(m_pVertices);
        m_pVertices = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
    {
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    // calculate next rotation angle
    m_Angle += (m_RotationSpeed * elapsedTime * 10.0f);

    // is rotating angle out of bounds?
    while (m_Angle >= 6.28f)
        m_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    MINI_Vector3 t;
    MINI_Vector3 axis;
    MINI_Vector3 factor;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrixX;
    MINI_Matrix  rotateMatrixY;
    MINI_Matrix  scaleMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angleX;
    float        angleY;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =   0.0f;
    t.m_Y =   0.0f;
    t.m_Z = -10.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angleX = 0.0f;

    miniGetRotateMatrix(&angleX, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    angleY = m_Angle;

    miniGetRotateMatrix(&angleY, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 1.0f;
    factor.m_Y = 1.0f;
    factor.m_Z = 1.0f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniMatrixMultiply(&rotateMatrixY,   &rotateMatrixX,   &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the rover model
    miniDrawRover(m_pVertices,
                  m_VertexCount,
                  m_pMdlCmds,
                  m_pIndexes,
                  m_IndexCount,
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
