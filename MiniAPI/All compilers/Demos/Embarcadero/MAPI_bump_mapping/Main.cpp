/*****************************************************************************
 * ==> Bump mapping demo ----------------------------------------------------*
 *****************************************************************************
 * Description : A bump mapped stone wall with diffuse light, hover with     *
 *               mouse on the wall to modify the light position              *
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

// NOTE the texture was found here: http://opengameart.org/content/stone-texture-bump
#define STONE_TEXTURE_FILE "..\\..\\..\\..\\..\\Resources\\stone.bmp"
#define STONE_BUMPMAP_FILE "..\\..\\..\\..\\..\\Resources\\stone_bump.bmp"

//------------------------------------------------------------------------------
const char* g_pVSDiffuseBumpMap =
    "precision mediump float;"
    "attribute vec4 mini_vPosition;"
    "attribute vec4 mini_vColor;"
    "attribute vec2 mini_vTexCoord;"
    "uniform   vec3 mini_vLightPos;"
    "uniform   mat4 mini_uProjection;"
    "uniform   mat4 mini_uModelview;"
    "varying   vec4 mini_fColor;"
    "varying   vec2 mini_fTexCoord;"
    "varying   vec3 mini_fLightPos;"
    "void main(void)"
    "{"
    "    mini_fColor    = mini_vColor;"
    "    mini_fTexCoord = mini_vTexCoord;"
    "    mini_fLightPos = mini_vLightPos;"
    "    gl_Position  = mini_uProjection * mini_uModelview * mini_vPosition;"
    "}";
//------------------------------------------------------------------------------
// NOTE this shader was written on the base of the following article:
// http://www.swiftless.com/tutorials/glsl/8_bump_mapping.html
const char* g_pFSDiffuseBumpMap =
    "precision mediump float;"
    "uniform sampler2D mini_sColorMap;"
    "uniform sampler2D mini_sBumpMap;"
    "varying lowp vec4 mini_fColor;"
    "varying      vec2 mini_fTexCoord;"
    "varying      vec3 mini_fLightPos;"
    "void main(void)"
    "{"
    "    vec3  normal  = normalize(texture2D(mini_sBumpMap, mini_fTexCoord).rgb * 2.0 - 1.0);"
    "    float diffuse = clamp(dot(normal, mini_fLightPos), 0.0, 2.5);"
    "    vec3  color   = diffuse * texture2D(mini_sColorMap, mini_fTexCoord).rgb;"
    "    gl_FragColor  = vec4(color, 1.0);"
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
    m_TextureIndex(GL_INVALID_VALUE),
    m_BumpMapIndex(GL_INVALID_VALUE),
    m_LightPos(0),
    m_TexSamplerSlot(0),
    m_BumpMapSamplerSlot(0)
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
    // draw the scene
    DrawScene();

    ::SwapBuffers(m_hDC);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormMouseMove(TObject* pSender, TShiftState shift, int x, int y)
{
    const float maxX = 2.0f;
    const float maxY = 2.0f;

    // convert screen coordinates to light world coordinate and notify shader about new light position
    glUniform3f(m_LightPos,
                ((float(x) * maxX) / ClientWidth) - 1.0f,
                1.0f - ((float(y) * maxY) / ClientHeight),
                2.0f);
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
    m_ShaderProgram = miniCompileShaders(g_pVSDiffuseBumpMap, g_pFSDiffuseBumpMap);
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram,  "mini_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram,  "mini_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram,  "mini_vTexCoord");
    m_LightPos              = glGetUniformLocation(m_ShaderProgram, "mini_vLightPos");
    m_TexSamplerSlot        = glGetUniformLocation(m_ShaderProgram, "mini_sColorMap");
    m_BumpMapSamplerSlot    = glGetUniformLocation(m_ShaderProgram, "mini_sBumpMap");

    // notify shader about default light position
    glUniform3f(m_LightPos, 0.0f, 0.0f, 2.0f);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

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

    // load wall texture and his bump map
    m_TextureIndex = miniLoadTexture(STONE_TEXTURE_FILE);
    m_BumpMapIndex = miniLoadTexture(STONE_BUMPMAP_FILE);
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

    if (m_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_TextureIndex);

    m_TextureIndex = GL_INVALID_VALUE;

    if (m_BumpMapIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_BumpMapIndex);

    m_BumpMapIndex = GL_INVALID_VALUE;

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    int          stride;
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // calculate vertex stride
    stride = m_VertexFormat.m_Stride;

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -15.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // enable OpenGL texturing engine
    glEnable(GL_TEXTURE_2D);

    // connect texture to shader
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, 0);
    glBindTexture(GL_TEXTURE_2D, m_TextureIndex);

    // connect bump map to shader
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(m_BumpMapSamplerSlot, 1);
    glBindTexture(GL_TEXTURE_2D, m_BumpMapIndex);

    // draw the bump mapped image
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
