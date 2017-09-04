/*****************************************************************************
 * ==> Underground demo -----------------------------------------------------*
 *****************************************************************************
 * Description : An underground level demo based on ray casting engines like *
 *               Doom or Wolfenstein. Press the up or down arrow keys to     *
 *               walk, and the left or right arrow keys to rotate            *
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
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniLevel.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

// windows
#include <Windowsx.h>

// resources
#include "resources.rh"

// map mode, used for debugging
//#define MAP_MODE

#define SOIL_TEXTURE_FILE "..\\..\\..\\Resources\\Soil_25_256x256.bmp"
#define WALL_TEXTURE_FILE "..\\..\\..\\Resources\\Wall_Tilleable_64_256x256.bmp"
#define CEIL_TEXTURE_FILE "..\\..\\..\\Resources\\Soil_2_25_256x256.bmp"

#ifdef MAP_MODE
    #define SPHERE_TEXTURE_FILE "..\\..\\..\\Resources\\cloud.bmp"
#endif

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
const char* g_pLevelMap =
    "***   ***   ******"
    "********* *   *  *"
    "********* ** **  *"
    "*** * ************"
    "*** * ********* **"
    "***   ***  *    **"
    "********* *** ****"
    "********* *** ****"
    "***   *** *** ****";
//------------------------------------------------------------------------------
const char* miniVSTextured2 =
    "precision mediump float;"
    "attribute vec4 qr_vPosition;"
    "attribute vec4 qr_vColor;"
    "attribute vec2 qr_vTexCoord;"
    "uniform   mat4 qr_uProjection;"
    "uniform   mat4 qr_uView;"
    "uniform   mat4 qr_uModelview;"
    "varying   vec4 qr_fColor;"
    "varying   vec2 qr_fTexCoord;"
    "void main(void)"
    "{"
    "    qr_fColor    = qr_vColor;"
    "    qr_fTexCoord = qr_vTexCoord;"
    "    gl_Position  = qr_uProjection * qr_uView * qr_uModelview * qr_vPosition;"
    "}";
//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
MINI_LevelItem*    g_pLevel             = 0;
GLuint             g_ShaderProgram      = 0;
float*             g_pSurfaceVB         = 0;
unsigned int       g_SurfaceVertexCount = 0;
const float        g_LevelItemWidth     = 0.05f;
const float        g_LevelItemHeight    = 0.05f;
const float        g_LevelItemLength    = 0.05f;
float              g_PosVelocity        = 0.0f;
float              g_DirVelocity        = 0.0f;
float              g_Angle              = 0.0f;
const unsigned int g_MapWidth           = 18;
const unsigned int g_MapHeight          = 9;
unsigned int       g_MapLength;
int                g_SceneInitialized   = 0;
MINI_Sphere        g_Player;
MINI_Vector2       g_TouchOrigin;
MINI_Vector2       g_TouchPosition;
GLuint             g_SoilTextureIndex   = GL_INVALID_VALUE;
GLuint             g_WallTextureIndex   = GL_INVALID_VALUE;
GLuint             g_CeilTextureIndex   = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot     = 0;
GLuint             g_ViewUniform        = 0;
GLuint             g_ModelviewUniform   = 0;
MINI_VertexFormat  g_VertexFormat;

#ifdef MAP_MODE
    float             g_SphereRadius;
    float*            g_pSphereVertices     = 0;
    unsigned int      g_SphereVertexCount;
    unsigned int      g_SphereIndexCount;
    MINI_Index*       g_pSphereIndexes      = 0;
    MINI_VertexFormat g_SphereVertexFormat;
    GLuint            g_SphereTextureIndex  = GL_INVALID_VALUE;
#endif
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
                case VK_LEFT:  g_DirVelocity = -5.0f; break;
                case VK_RIGHT: g_DirVelocity =  5.0f; break;
                case VK_UP:    g_PosVelocity = -0.3f; break;
                case VK_DOWN:  g_PosVelocity =  0.3f; break;
            }

            break;
        }

        case WM_KEYUP:
        {
            switch (wParam)
            {
                case VK_LEFT:
                case VK_RIGHT: g_DirVelocity = 0.0f; break;

                case VK_UP:
                case VK_DOWN:  g_PosVelocity = 0.0f; break;
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
    const float zNear  = 0.001f;
    const float zFar   = 1000.0f;
    const float fov    = 45.0f;
    const float aspect = (GLfloat)w/(GLfloat)h;

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    g_MapLength = g_MapWidth * g_MapHeight;

    // initialize the player
    g_Player.m_Pos.m_X = 0.0f;
    g_Player.m_Pos.m_Y = 0.0f;
    g_Player.m_Pos.m_Z = 0.0f;
    g_Player.m_Radius  = 0.01f;

    g_Angle = 0.0f;

    // initialize the touch
    g_TouchOrigin.m_X   = 0.0f;
    g_TouchOrigin.m_Y   = 0.0f;
    g_TouchPosition.m_X = 0.0f;
    g_TouchPosition.m_Y = 0.0f;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniVSTextured2, miniGetFSTextured());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
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
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // generate the level
    miniGenerateLevel(g_pLevelMap,
                      g_MapWidth,
                      g_MapHeight,
                      &g_LevelItemWidth,
                      &g_LevelItemHeight,
                      &g_pLevel);

    #ifdef MAP_MODE
        g_SphereRadius                     = g_Player.m_Radius;
        g_SphereVertexFormat.m_UseNormals  = 0;
        g_SphereVertexFormat.m_UseTextures = 1;
        g_SphereVertexFormat.m_UseColors   = 1;

        // generate sphere
        miniCreateSphere(&g_SphereRadius,
                         5,
                         5,
                         0xFFFFFFFF,
                         &g_SphereVertexFormat,
                         &g_pSphereVertices,
                         &g_SphereVertexCount,
                         &g_pSphereIndexes,
                         &g_SphereIndexCount);
    #endif

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // calculate the stride
    miniCalculateStride(&g_VertexFormat);

    // generate surface
    miniCreateSurface(&g_LevelItemWidth,
                      &g_LevelItemHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);

    // load textures
    g_SoilTextureIndex = miniLoadTexture(SOIL_TEXTURE_FILE);
    g_WallTextureIndex = miniLoadTexture(WALL_TEXTURE_FILE);
    g_CeilTextureIndex = miniLoadTexture(CEIL_TEXTURE_FILE);

    #ifdef MAP_MODE
        g_SphereTextureIndex = miniLoadTexture(SPHERE_TEXTURE_FILE);
    #endif

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete objects used in map mode
    #ifdef MAP_MODE
        // delete sphere vertex buffer
        if (g_pSphereVertices)
        {
            free(g_pSphereVertices);
            g_pSphereVertices = 0;
        }

        // delete sphere index buffer
        if (g_pSphereIndexes)
        {
            free(g_pSphereIndexes);
            g_pSphereIndexes = 0;
        }
    #endif

    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    // delete the level
    if (g_pLevel)
    {
        free(g_pLevel);
        g_pLevel = 0;
    }

    // delete textures
    if (g_SoilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_SoilTextureIndex);

    g_SoilTextureIndex = GL_INVALID_VALUE;

    if (g_WallTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_WallTextureIndex);

    g_WallTextureIndex = GL_INVALID_VALUE;

    if (g_CeilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_CeilTextureIndex);

    g_CeilTextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime)
{
    MINI_Vector3 newPos;

    // no time elapsed?
    if (!elapsedTime)
        return;

    // is player rotating?
    if (g_DirVelocity)
    {
        // calculate the player direction
        g_Angle += g_DirVelocity * elapsedTime;

        // validate and apply it
        if (g_Angle > M_PI * 2.0f)
            g_Angle -= M_PI * 2.0f;
        else
        if (g_Angle < 0.0f)
            g_Angle += M_PI * 2.0f;
    }

    // is player moving?
    if (g_PosVelocity)
    {
        newPos = g_Player.m_Pos;

        // calculate the next player position
        newPos.m_X += g_PosVelocity * cosf(g_Angle + (M_PI * 0.5f)) * elapsedTime;
        newPos.m_Z += g_PosVelocity * sinf(g_Angle + (M_PI * 0.5f)) * elapsedTime;

        // validate and apply it
        miniValidateNextPos(g_pLevel, g_LevelItemWidth, g_LevelItemHeight, g_MapLength, &g_Player, &newPos);
        g_Player.m_Pos = newPos;
    }
}
//------------------------------------------------------------------------------
void DrawScene()
{
    MINI_LevelDrawInfo drawInfo;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // configure the draw info
    drawInfo.m_pSurfaceVB         =  g_pSurfaceVB;
    drawInfo.m_SurfaceVertexCount =  g_SurfaceVertexCount;
    drawInfo.m_pVertexFormat      = &g_VertexFormat;
    drawInfo.m_pShader            = &g_Shader;
    drawInfo.m_ShaderProgram      =  g_ShaderProgram;
    drawInfo.m_SoilTextureIndex   =  g_SoilTextureIndex;
    drawInfo.m_WallTextureIndex   =  g_WallTextureIndex;
    drawInfo.m_CeilTextureIndex   =  g_CeilTextureIndex;

    #ifdef MAP_MODE
        drawInfo.m_pSphereVB           =  g_pSphereVertices;
        drawInfo.m_SphereRadius        =  g_SphereRadius;
        drawInfo.m_pSphereIndexes      =  g_pSphereIndexes;
        drawInfo.m_SphereVertexCount   =  g_SphereVertexCount;
        drawInfo.m_SphereIndexCount    =  g_SphereIndexCount;
        drawInfo.m_pSphereShader       = &g_Shader;
        drawInfo.m_pSphereVertexFormat = &g_SphereVertexFormat;
        drawInfo.m_SphereTextureIndex  =  g_SphereTextureIndex;
        drawInfo.m_MapMode             =  1;
    #else
        drawInfo.m_MapMode = 0;
    #endif

    // draw the level
    miniDrawLevel(g_pLevel,
                  g_LevelItemWidth,
                  g_LevelItemHeight,
                  g_LevelItemLength,
                  g_MapLength,
                 &g_Player,
                  g_Angle,
                 &drawInfo);

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
    wcex.lpszClassName = "MAPI_underground";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_underground",
                          "MiniAPI Underground Demo",
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
