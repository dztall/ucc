/*****************************************************************************
 * ==> Snow demo ------------------------------------------------------------*
 *****************************************************************************
 * Description : Snow particle system demo                                   *
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
    m_Radius(0.02f),
    m_Angle(0.0f),
    m_RotationSpeed(0.1f),
    m_ParticleInitialized(0),
    m_ParticleCount(1000),
    m_Initialized(0),
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

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
{
    std::srand(unsigned(std::time(0)));

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

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate disk
    miniCreateDisk(0.0f,
                   0.0f,
                   m_Radius,
                   5,
                   0xFFFFFFFF,
                  &m_VertexFormat,
                  &m_pVertexBuffer,
                  &m_VertexCount);

    m_Particles.m_Count = 0;
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    miniClearParticles(&m_Particles);

    // delete vertices
    if (m_pVertexBuffer)
    {
        free(m_pVertexBuffer);
        m_pVertexBuffer = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    unsigned       i;
    MINI_Vector3   startPos;
    MINI_Vector3   startDir;
    MINI_Vector3   startVelocity;
    MINI_Particle* pNewParticle;

    startPos.m_X      =  0.0f; // between -2.2 to 2.2
    startPos.m_Y      =  2.0f;
    startPos.m_Z      = -3.0f; // between -1.0 to -5.0
    startDir.m_X      =  1.0f;
    startDir.m_Y      = -1.0f;
    startDir.m_Z      =  0.0f;
    startVelocity.m_X =  0.0f;
    startVelocity.m_Y =  0.05f;
    startVelocity.m_Z =  0.0f;

    // iterate through particles to update
    for (i = 0; i < m_ParticleCount; ++i)
    {
        // emit a new particle
        if (pNewParticle = miniEmitParticle(&m_Particles,
                                            &startPos,
                                            &startDir,
                                            &startVelocity,
                                            m_ParticleCount))
        {
            // initialize default values
            pNewParticle->m_Position.m_X =  ((std::rand() % 44) - 22.0f) * 0.1f; // between -2.2 to  2.2
            pNewParticle->m_Position.m_Z = -((std::rand() % 40) + 10.0f) * 0.1f; // between -1.0 to -5.0
            pNewParticle->m_Velocity.m_X =  ((std::rand() % 4)  - 2.0f)  * 0.01f;
            pNewParticle->m_Velocity.m_Y =  ((std::rand() % 4)  + 2.0f)  * 0.01f;

            // select a random start height the first time particles are emitted
            if (!m_ParticleInitialized)
                pNewParticle->m_Position.m_Y = 2.0f + ((std::rand() % 200) * 0.01f);
        }

        // no particles to show? (e.g all were removed in this loop)
        if (!m_Particles.m_Count)
            continue;

        // move particle
        if (i >= m_Particles.m_Count)
            miniMoveParticle(&m_Particles.m_pParticles[m_Particles.m_Count - 1], elapsedTime * 20.0f);
        else
            miniMoveParticle(&m_Particles.m_pParticles[i], elapsedTime * 20.0f);

        // is particle out of screen?
        if (m_Particles.m_pParticles[i].m_Position.m_Y <= -2.0f ||
            m_Particles.m_pParticles[i].m_Position.m_X <= -4.0f ||
            m_Particles.m_pParticles[i].m_Position.m_X >=  4.0f)
        {
            // delete it from system
            miniDeleteParticle(&m_Particles, i);
            continue;
        }
    }

    m_ParticleInitialized = 1;
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    unsigned     i;
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.1f, 0.35f, 0.66f, 1.0f);

    // iterate through particles to draw
    for (i = 0; i < m_Particles.m_Count; ++i)
    {
        // set translation
        t.m_X = m_Particles.m_pParticles[i].m_Position.m_X;
        t.m_Y = m_Particles.m_pParticles[i].m_Position.m_Y;
        t.m_Z = m_Particles.m_pParticles[i].m_Position.m_Z;

        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect model view matrix to shader
        GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the particle
        miniDrawDisk(m_pVertexBuffer, m_VertexCount, &m_VertexFormat, &m_Shader);
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
