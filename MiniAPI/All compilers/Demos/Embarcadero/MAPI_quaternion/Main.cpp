/*****************************************************************************
 * ==> Quaternion demo ------------------------------------------------------*
 *****************************************************************************
 * Description : An animation using a quaternion                             *
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
    m_pVertexBuffer(0),
    m_VertexCount(0),
    m_pIndexes(0),
    m_IndexCount(0),
    m_pCylinderVB(0),
    m_CylinderVBCount(0),
    m_CircleRadius(0.1f),
    m_CylinderRadius(0.1f),
    m_CylinderHeight(2.0f),
    m_Angle(0.0f),
    m_CylFaceCount(12),
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
    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 20.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &m_ProjectionMatrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &m_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
{
    m_View.m_Width  = 0.0f;
    m_View.m_Height = 0.0f;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot  = glGetAttribLocation(m_ShaderProgram, "mini_vColor");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // disable culling
    glDisable(GL_CULL_FACE);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&m_CircleRadius,
                     5,
                     12,
                     0x0000FFFF,
                     &m_VertexFormat,
                     &m_pVertexBuffer,
                     &m_VertexCount,
                     &m_pIndexes,
                     &m_IndexCount);

    // generate cylinder
    miniCreateCylinder(&m_CylinderRadius,
                       &m_CylinderHeight,
                       m_CylFaceCount,
                       0xFF0000FF,
                       &m_VertexFormat,
                       &m_pCylinderVB,
                       &m_CylinderVBCount);
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    // delete buffer index table
    if (m_pIndexes)
    {
        free(m_pIndexes);
        m_pIndexes = 0;
    }

    // delete vertices
    if (m_pVertexBuffer)
    {
        free(m_pVertexBuffer);
        m_pVertexBuffer = 0;
    }

    // delete cylinder vertices
    if (m_pCylinderVB)
    {
        free(m_pCylinderVB);
        m_pCylinderVB = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    // calculate next angle value, limit to 2 * PI
    m_Angle = std::fmod(m_Angle + (3.0f * elapsedTime), M_PI * 2.0f);
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    float           angle;
    MINI_Vector3    t;
    MINI_Vector3    axis;
    MINI_Quaternion quatX;
    MINI_Quaternion quatZ;
    MINI_Matrix     translateMatrix;
    MINI_Matrix     rotTransMatrix;
    MINI_Matrix     rotMatrixX;
    MINI_Matrix     rotMatrixZ;
    MINI_Matrix     combMatrix1;
    MINI_Matrix     combMatrix2;
    MINI_Matrix     modelMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    t.m_X =  0.95f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the ball translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    t.m_X = 0.2f;
    t.m_Y = 0.0f;
    t.m_Z = 0.0f;

    // set the ball animation translation
    miniGetTranslateMatrix(&t, &rotTransMatrix);

    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    angle    = m_Angle;

    // calculate the x axis rotation
    miniQuatFromAxis(&angle, &axis, &quatX);
    miniGetMatrix(&quatX, &rotMatrixX);

    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;
    angle    = M_PI / 2.0f;

    // calculate the z axis rotation
    miniQuatFromAxis(&angle, &axis, &quatZ);
    miniGetMatrix(&quatZ, &rotMatrixZ);

    // build the ball final model matrix
    miniMatrixMultiply(&rotTransMatrix, &rotMatrixZ,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1,    &rotMatrixX,      &combMatrix2);
    miniMatrixMultiply(&combMatrix2,    &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the sphere
    miniDrawSphere(m_pVertexBuffer,
                   m_VertexCount,
                   m_pIndexes,
                   m_IndexCount,
                   &m_VertexFormat,
                   &m_Shader);

    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the cylinder animation translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    // build the cylinder final model matrix
    miniMatrixMultiply(&rotMatrixZ,  &rotMatrixX,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1, &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the cylinder
    miniDrawCylinder(m_pCylinderVB, m_CylFaceCount, &m_VertexFormat, &m_Shader);

    miniEndScene();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnIdle(TObject* pSender, bool& done)
{
    FormPaint(pSender);
    done = false;
}
//---------------------------------------------------------------------------
