/*****************************************************************************
 * ==> Quake (MDL) model demo -----------------------------------------------*
 *****************************************************************************
 * Description : A Quake (MDL) model showing a wizard. Press the left or     *
 *               right arrow key to change the animation                     *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#include <vcl.h>
#pragma hdrstop
#include "Main.h"

#pragma package(smart_init)
#ifdef __llvm__
    #pragma link "glewSL.a"
#else
    #pragma link "glewSL.lib"
#endif
#pragma resource "*.dfm"

// NOTE the mdl model was extracted from the Quake game package
#define MDL_FILE "..\\..\\..\\..\\..\\Resources\\wizard.mdl"

//---------------------------------------------------------------------------
TMainForm* MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* pOwner) :
    TForm(pOwner),
    m_pModel(NULL),
    m_MeshIndex(),
    m_ShaderProgram(),
    m_TextureIndex(GL_INVALID_VALUE),
    m_TexSamplerSlot(0),
    m_Time(0.0f),
    m_Interval(0.0f),
    m_FPS(15),
    m_AnimIndex(0),
    m_PreviousTime(0)
{
    EnableOpenGL(Handle, &m_hDC, &m_hRC);

    // stop GLEW crashing on OSX :-/
    glewExperimental = GL_TRUE;

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        // shutdown OpenGL
        DisableOpenGL(Handle, m_hDC, m_hRC);

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
    InitScene(ClientWidth, ClientHeight);

    // initialize the timer
    m_PreviousTime = ::GetTickCount();

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

    UpdateScene(elapsedTime);
    DrawScene();

    SwapBuffers(m_hDC);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormKeyUp(TObject* pSender, WORD& key, TShiftState shift)
{
    switch (key)
    {
        case VK_LEFT:
            if (m_AnimIndex == 0)
                m_AnimIndex = 5;

            --m_AnimIndex;
            break;

        case VK_RIGHT:
            ++m_AnimIndex;

            if (m_AnimIndex > 4)
                m_AnimIndex = 0;

            break;
    }
}
//------------------------------------------------------------------------------
void TMainForm::EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    // get the device context (DC)
    *hDC = ::GetDC(hWnd);

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
    const float zFar   = 1000.0f;
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
    MINI_Texture texture;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(m_ShaderProgram);

    // configure the shader slots
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram, "qr_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram, "qr_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram, "qr_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram, "qr_sColorMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    m_ModelFormat.m_UseNormals  = 0;
    m_ModelFormat.m_UseTextures = 1;
    m_ModelFormat.m_UseColors   = 1;

    // load MDL file and create mesh to draw
    miniLoadMDLModel((unsigned char*)MDL_FILE,
                     &m_ModelFormat,
                     0xFFFFFFFF,
                     &m_pModel,
                     &texture);

    // create new OpenGL texture
    glGenTextures(1, &m_TextureIndex);
    glBindTexture(GL_TEXTURE_2D, m_TextureIndex);

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

    // create MDL animation list
    m_Animation[0].m_Range[0] = 0;  m_Animation[0].m_Range[1] = 14; // wizard hovers
    m_Animation[1].m_Range[0] = 15; m_Animation[1].m_Range[1] = 28; // wizard flies
    m_Animation[2].m_Range[0] = 29; m_Animation[2].m_Range[1] = 42; // wizard attacks
    m_Animation[3].m_Range[0] = 43; m_Animation[3].m_Range[1] = 47; // wizard feels pain
    m_Animation[4].m_Range[0] = 48; m_Animation[4].m_Range[1] = 54; // wizard dies

    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    miniReleaseMDLModel(m_pModel);
    m_pModel = 0;

    if (m_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_TextureIndex);

    m_TextureIndex = GL_INVALID_VALUE;

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
    unsigned int frameCount = 0;
    unsigned int deltaRange =
            m_Animation[m_AnimIndex].m_Range[1] - m_Animation[m_AnimIndex].m_Range[0];

    // calculate next time
    m_Time += (elapsedTime * 1000.0f);

    // count frames
    while (m_Time > m_Interval)
    {
        m_Time -= m_Interval;
        ++frameCount;
    }

    // calculate next mesh index to show. Index should always be between animation range
    m_MeshIndex = ((m_MeshIndex + frameCount) % deltaRange);
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
    MINI_Matrix  combinedRotMatrix;
    MINI_Matrix  combinedRotTransMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angle;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y = -20.0f;
    t.m_Z = -150.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.5;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.25;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 0.025f;
    factor.m_Y = 0.025f;
    factor.m_Z = 0.025f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniMatrixMultiply(&rotateMatrixX,          &rotateMatrixY,   &combinedRotMatrix);
    miniMatrixMultiply(&combinedRotMatrix,      &translateMatrix, &combinedRotTransMatrix);
    miniMatrixMultiply(&combinedRotTransMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // draw the model
    miniDrawMDL(m_pModel,
                &m_Shader,
                (int)(m_Animation[m_AnimIndex].m_Range[0] + m_MeshIndex));

    miniEndScene();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnIdle(TObject* pSender, bool& done)
{
    FormPaint(pSender);
    done = false;
}
//---------------------------------------------------------------------------
