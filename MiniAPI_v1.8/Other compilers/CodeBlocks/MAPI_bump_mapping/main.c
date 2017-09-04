/*****************************************************************************
 * ==> Bump mapping demo ----------------------------------------------------*
 *****************************************************************************
 * Description : A bump mapped stone wall with diffuse light, hover with     *
 *               mouse on the wall to modify the light position              *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <math.h>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

// windows
#include <Windowsx.h>

// resources
#include "resources.rh"

// NOTE the texture was found here: http://opengameart.org/content/stone-texture-bump
#define STONE_TEXTURE_FILE "..\\..\\..\\Resources\\stone.bmp"
#define STONE_BUMPMAP_FILE "..\\..\\..\\Resources\\stone_bump.bmp"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
GLuint             g_ShaderProgram      = 0;
float*             g_pSurfaceVB         = 0;
unsigned           g_SurfaceVertexCount = 0;
int                g_SceneInitialized   = 0;
const float        g_SurfaceWidth       = 10.0f;
const float        g_SurfaceHeight      = 12.5f;
GLuint             g_TextureIndex       = GL_INVALID_VALUE;
GLuint             g_BumpMapIndex       = GL_INVALID_VALUE;
GLuint             g_LightPos           = 0;
GLuint             g_TexSamplerSlot     = 0;
GLuint             g_BumpMapSamplerSlot = 0;
MINI_Size          g_View;
MINI_VertexFormat  g_VertexFormat;
//------------------------------------------------------------------------------
const char* g_pVSDiffuseBumpMap =
    "precision mediump float;"
    "attribute vec4 qr_vPosition;"
    "attribute vec4 qr_vColor;"
    "attribute vec2 qr_vTexCoord;"
    "uniform   vec3 qr_vLightPos;"
    "uniform   mat4 qr_uProjection;"
    "uniform   mat4 qr_uModelview;"
    "varying   vec4 qr_fColor;"
    "varying   vec2 qr_fTexCoord;"
    "varying   vec3 qr_fLightPos;"
    "void main(void)"
    "{"
    "    qr_fColor    = qr_vColor;"
    "    qr_fTexCoord = qr_vTexCoord;"
    "    qr_fLightPos = qr_vLightPos;"
    "    gl_Position  = qr_uProjection * qr_uModelview * qr_vPosition;"
    "}";
//------------------------------------------------------------------------------
// NOTE this shader was written on the base of the following article:
// http://www.swiftless.com/tutorials/glsl/8_bump_mapping.html
const char* g_pFSDiffuseBumpMap =
    "precision mediump float;"
    "uniform sampler2D qr_sColorMap;"
    "uniform sampler2D qr_sBumpMap;"
    "varying lowp vec4 qr_fColor;"
    "varying      vec2 qr_fTexCoord;"
    "varying      vec3 qr_fLightPos;"
    "void main(void)"
    "{"
    "    vec3  normal  = normalize(texture2D(qr_sBumpMap, qr_fTexCoord).rgb * 2.0 - 1.0);"
    "    float diffuse = clamp(dot(normal, qr_fLightPos), 0.0, 2.5);"
    "    vec3  color   = diffuse * texture2D(qr_sColorMap, qr_fTexCoord).rgb;"
    "    gl_FragColor  = vec4(color, 1.0);"
    "}";
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_DESTROY:
            return 0;

        case WM_MOUSEMOVE:
        {
            const float x = GET_X_LPARAM(lParam);
            const float y = GET_Y_LPARAM(lParam);

            const float maxX = 2.0f;
            const float maxY = 2.0f;

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);

            // convert screen coordinates to light world coordinate and notify shader about new light position
            glUniform3f(g_LightPos,
                        ((x * maxX) / (clientRect.right - clientRect.left)) - 1.0f,
                        1.0f - ((y * maxY) / (clientRect.bottom - clientRect.top)),
                        2.0f);

            break;
        }

        case WM_SIZE:
        {
            if (!g_SceneInitialized)
                break;

            const int width  = ((int)(short)LOWORD(lParam));
            const int height = ((int)(short)HIWORD(lParam));

            CreateViewport(width, height);
            break;
        }

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}
//------------------------------------------------------------------------------
void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    // get the device context (DC)
    *hDC = GetDC(hwnd);

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
void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
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
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(g_pVSDiffuseBumpMap, g_pFSDiffuseBumpMap);
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram,  "qr_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram,  "qr_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram,  "qr_vTexCoord");
    g_LightPos              = glGetUniformLocation(g_ShaderProgram, "qr_vLightPos");
    g_TexSamplerSlot        = glGetUniformLocation(g_ShaderProgram, "qr_sColorMap");
    g_BumpMapSamplerSlot    = glGetUniformLocation(g_ShaderProgram, "qr_sBumpMap");

    // notify shader about default light position
    glUniform3f(g_LightPos, 0.0f, 0.0f, 2.0f);

    // create the viewport
    CreateViewport(w, h);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // generate surface
    miniCreateSurface(&g_SurfaceWidth,
                      &g_SurfaceHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);

    // load wall texture and his bump map
    g_TextureIndex = miniLoadTexture(STONE_TEXTURE_FILE);
    g_BumpMapIndex = miniLoadTexture(STONE_BUMPMAP_FILE);

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    if (g_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_TextureIndex);

    g_TextureIndex = GL_INVALID_VALUE;

    if (g_BumpMapIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_BumpMapIndex);

    g_BumpMapIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void DrawScene()
{
    int          stride;
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // calculate vertex stride
    stride = g_VertexFormat.m_Stride;

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -15.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // enable OpenGL texturing engine
    glEnable(GL_TEXTURE_2D);

    // connect texture to shader
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, 0);
    glBindTexture(GL_TEXTURE_2D, g_TextureIndex);

    // connect bump map to shader
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(g_BumpMapSamplerSlot, 1);
    glBindTexture(GL_TEXTURE_2D, g_BumpMapIndex);

    // draw the bump mapped image
    miniDrawSurface(g_pSurfaceVB,
                    g_SurfaceVertexCount,
                    &g_VertexFormat,
                    &g_Shader);

    miniEndScene();
}
//------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    // try to load application icon from resources
    HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL),
                                   MAKEINTRESOURCE(IDI_MAIN_ICON),
                                   IMAGE_ICON,
                                   16,
                                   16,
                                   0);

    WNDCLASSEX wcex;
    HWND       hWnd;
    HDC        hDC;
    HGLRC      hRC;
    MSG        msg;
    BOOL       bQuit = FALSE;

    // register the window class
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_OWNDC;
    wcex.lpfnWndProc   = WindowProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm       = hIcon;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = "MAPI_bump_mapping";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_bump_mapping",
                          "MiniAPI Bump Mapping Demo",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          400,
                          400,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    // enable OpenGL for the window
    EnableOpenGL(hWnd, &hDC, &hRC);

    // stop GLEW crashing on OSX :-/
    glewExperimental = GL_TRUE;

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        // shutdown OpenGL
        DisableOpenGL(hWnd, hDC, hRC);

        // destroy the window explicitly
        DestroyWindow(hWnd);

        return 0;
    }

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    // initialize the scene
    InitScene(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    // application main loop
    while (!bQuit)
    {
        // check for messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // handle or dispatch messages
            if (msg.message == WM_QUIT)
                bQuit = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            DrawScene();
            SwapBuffers(hDC);
            Sleep (1);
        }
    }

    // delete the scene
    DeleteScene();

    // shutdown OpenGL
    DisableOpenGL(hWnd, hDC, hRC);

    // destroy the window explicitly
    DestroyWindow(hWnd);

    return msg.wParam;
}
//------------------------------------------------------------------------------
