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

// std
#include <math.h>

// mini API
#include "MiniCommon.h"
#include "MiniShader.h"
#include "MiniModels.h"
#include "MiniRenderer.h"

// resources
#include "resources.rh"

// NOTE the mdl model was extracted from the Quake game package
#define MDL_FILE "..\\..\\..\\Resources\\wizard.mdl"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} MINI_MDLAnimation;
//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
MINI_VertexFormat  g_ModelFormat;
MINI_MDLModel*     g_pModel           = 0;
unsigned int       g_MeshIndex        = 0;
GLuint             g_ShaderProgram    = 0;
GLuint             g_TextureIndex     = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot   = 0;
float              g_Time             = 0.0f;
float              g_Interval         = 0.0f;
int                g_SceneInitialized = 0;
const unsigned int g_FPS              = 15;
unsigned int       g_AnimIndex        = 0; // 0 = hover, 1 = fly, 2 = attack, 3 = pain, 4 = death
MINI_MDLAnimation  g_Animation[5];
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                        g_AnimIndex = 5;

                    --g_AnimIndex;
                    break;

                case VK_RIGHT:
                    ++g_AnimIndex;

                    if (g_AnimIndex > 4)
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
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 1000.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    MINI_Texture texture;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(g_ShaderProgram);

    // configure the shader slots
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram, "qr_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram, "qr_sColorMap");

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

    // load MDL file and create mesh to draw
    miniLoadMDLModel((unsigned char*)MDL_FILE,
                     &g_ModelFormat,
                     0xFFFFFFFF,
                     &g_pModel,
                     &texture);

    // create new OpenGL texture
    glGenTextures(1, &g_TextureIndex);
    glBindTexture(GL_TEXTURE_2D, g_TextureIndex);

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
    g_Animation[0].m_Range[0] = 0;  g_Animation[0].m_Range[1] = 14; // wizard hovers
    g_Animation[1].m_Range[0] = 15; g_Animation[1].m_Range[1] = 28; // wizard flies
    g_Animation[2].m_Range[0] = 29; g_Animation[2].m_Range[1] = 42; // wizard attacks
    g_Animation[3].m_Range[0] = 43; g_Animation[3].m_Range[1] = 47; // wizard feels pain
    g_Animation[4].m_Range[0] = 48; g_Animation[4].m_Range[1] = 54; // wizard dies

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    miniReleaseMDLModel(g_pModel);
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
void UpdateScene(float elapsedTime)
{
    unsigned int frameCount = 0;
    unsigned int deltaRange =
            g_Animation[g_AnimIndex].m_Range[1] - g_Animation[g_AnimIndex].m_Range[0];

    // calculate next time
    g_Time += (elapsedTime * 1000.0f);

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
    modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // draw the model
    miniDrawMDL(g_pModel,
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
    wcex.lpszClassName = "MAPI_wizard";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_wizard",
                          "MiniAPI Wizard (Quake I model) Demo",
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
