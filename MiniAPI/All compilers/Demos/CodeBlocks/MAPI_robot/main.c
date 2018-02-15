/*****************************************************************************
 * ==> Quake II (MD2) model demo --------------------------------------------*
 *****************************************************************************
 * Description : A Quake II (MD2) model showing a robot. Press the left or   *
 *               right arrow key to change the animation                     *
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
#include "MiniModels.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

// windows
#include <Windowsx.h>

// resources
#include "resources.rh"

// NOTE the md2 model was found on this site:
// http://leileilol.mancubus.net/garyacordsucks/64.246.6.138/_gcsgames.com/GCSenemy/md2.htm
#define MD2_FILE         "..\\..\\..\\Resources\\chip.md2"
#define MD2_TEXTURE_FILE "..\\..\\..\\Resources\\chipskin.bmp"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} MINI_MD2Animation;
//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
MINI_VertexFormat  g_ModelFormat;
MINI_MD2Model*     g_pModel           = 0;
unsigned int       g_MeshIndex        = 0;
GLuint             g_ShaderProgram    = 0;
GLuint             g_TextureIndex     = GL_INVALID_VALUE;
GLuint             g_PositionSlot     = 0;
GLuint             g_ColorSlot        = 0;
GLuint             g_TexCoordSlot     = 0;
GLuint             g_TexSamplerSlot   = 0;
float              g_Time             = 0.0f;
float              g_Interval         = 0.0f;
float              g_ScreenWidth      = 0.0f;
int                g_SceneInitialized = 0;
const unsigned int g_FPS              = 15;
unsigned int       g_AnimIndex        = 0; // can only be 0 (robot walks) or 1 (robot dies)
MINI_MD2Animation  g_Animation[2];
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

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;

                case VK_LEFT:
                    if (g_AnimIndex == 0)
                        g_AnimIndex = 2;

                    --g_AnimIndex;
                    break;

                case VK_RIGHT:
                    ++g_AnimIndex;

                    if (g_AnimIndex > 1)
                        g_AnimIndex = 0;

                    break;
            }

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
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    // get the screen width
    g_ScreenWidth = w;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(g_ShaderProgram);

    // configure the shader slots
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram, "mini_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "mini_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram, "mini_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram, "mini_sColorMap");

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

    g_ModelFormat.m_UseNormals  = 0;
    g_ModelFormat.m_UseTextures = 1;
    g_ModelFormat.m_UseColors   = 1;

    // load MD2 file and create mesh to draw
    miniLoadMD2Model(MD2_FILE, &g_ModelFormat, 0xFFFFFFFF, &g_pModel);

    // load MD2 texture
    g_TextureIndex = miniLoadTexture(MD2_TEXTURE_FILE);

    // create MD2 animation list
    g_Animation[0].m_Range[0] = 0;  g_Animation[0].m_Range[1] = 20; // robot walks
    g_Animation[1].m_Range[0] = 21; g_Animation[1].m_Range[1] = 29; // robot dies

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    miniReleaseMD2Model(g_pModel);
    g_pModel = 0;

    if (g_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_TextureIndex);

    g_TextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
    {
        glDeleteProgram(g_ShaderProgram);
        g_ShaderProgram = 0;
    }
}
//------------------------------------------------------------------------------
void UpdateScene(float timeStep_sec)
{
    unsigned int frameCount = 0;
    unsigned int deltaRange = g_Animation[g_AnimIndex].m_Range[1] -
                              g_Animation[g_AnimIndex].m_Range[0];

    // calculate next time
    g_Time += (timeStep_sec * 1000.0f);

    // count frames
    while (g_Time > g_Interval)
    {
        g_Time -= g_Interval;
        ++frameCount;
    }

    // calculate next mesh index to show. Index should always be between animation range
    g_MeshIndex = ((g_MeshIndex + frameCount) % deltaRange);
}
//------------------------------------------------------------------------------
void DrawScene()
{
    MINI_Vector3 t;
    MINI_Vector3 axis;
    MINI_Vector3 factor;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrix;
    MINI_Matrix  scaleMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angle;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -75.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = 0.0f;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrix);

    // set scale factor
    factor.m_X = 0.02f;
    factor.m_Y = 0.02f;
    factor.m_Z = 0.02f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniGetIdentity(&modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &rotateMatrix,    &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // draw the model
    miniDrawMD2(g_pModel,
                &g_Shader,
                (int)(g_Animation[g_AnimIndex].m_Range[0] + g_MeshIndex));

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
    wcex.lpszClassName = "MAPI_robot";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_robot",
                          "MiniAPI Robot (Quake II model) Demo",
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

    // initialize the timer
    unsigned __int64 previousTime = GetTickCount();

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
            // calculate time interval
            const unsigned __int64 now          =  GetTickCount();
            const double           elapsedTime  = (now - previousTime) / 1000.0;
                                   previousTime =  now;

            UpdateScene(elapsedTime);
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
