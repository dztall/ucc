/*****************************************************************************
 * ==> Aligned-axis bounding box ray picking demo ---------------------------*
 *****************************************************************************
 * Description : A ray picking demo with aligned-axis bounding box. Move the *
 *               mouse above the sphere to select a polygon, press the left  *
 *               or right arrow keys to rotate the sphere                    *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <math.h>

// mini API
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniCollision.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

// windows
#include <Windowsx.h>

// resources
#include "resources.rh"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
GLuint             g_ShaderProgram        = 0;
float*             g_pVertexBuffer        = 0;
unsigned int       g_VertexCount          = 0;
MINI_Index*        g_pIndexes             = 0;
unsigned int       g_IndexCount           = 0;
MINI_AABBNode*     g_pAABBRoot            = 0;
MINI_Polygon*      g_pCollidePolygons     = 0;
unsigned int       g_CollidePolygonsCount = 0;
float              g_Radius               = 1.0f;
float              g_RayX                 = 2.0f;
float              g_RayY                 = 2.0f;
int                g_SceneInitialized     = 0;
float              g_Angle                = 0.0f;
float              g_RotationSpeed        = 0.0f;
float              g_Time                 = 0.0f;
float              g_Interval             = 0.0f;
const unsigned int g_FPS                  = 15;
MINI_VertexFormat  g_VertexFormat;
MINI_Matrix        g_ProjectionMatrix;
MINI_Matrix        g_ViewMatrix;
float              g_PolygonArray[21];
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
            switch (wParam)
            {
                case VK_LEFT:  g_RotationSpeed -= 0.005; break;
                case VK_RIGHT: g_RotationSpeed += 0.005; break;
            }

            break;

        case WM_MOUSEMOVE:
        {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);

            g_RayX = -1.0f + ((x * 2.0f) / (clientRect.right  - clientRect.left));
            g_RayY =  1.0f - ((y * 2.0f) / (clientRect.bottom - clientRect.top));
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
    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // calculate matrix items
    const float zNear  =  1.0f;
    const float zFar   =  20.0f;
    const float aspect =  w / h;
    const float left   = -aspect;
    const float right  =  aspect;
    const float top    =  1.0f;
    const float bottom = -1.0f;

    miniGetFrustum(&left, &right, &bottom, &top, &zNear, &zFar, &g_ProjectionMatrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    unsigned i;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "qr_vColor");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // create the viewport
    CreateViewport(w, h);

    miniGetIdentity(&g_ViewMatrix);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&g_Radius,
                     20,
                     20,
                     0x0000FFFF,
                     &g_VertexFormat,
                     &g_pVertexBuffer,
                     &g_VertexCount,
                     &g_pIndexes,
                     &g_IndexCount);

    // iterate through vertex indexes
    for (i = 0; i < g_IndexCount; ++i)
        // get collide polygons
        miniGetPolygonsFromVB(&g_pVertexBuffer[g_pIndexes[i].m_Start],
                              g_pIndexes[i].m_Length,
                              1,
                              g_VertexFormat.m_Stride,
                              &g_pCollidePolygons,
                              &g_CollidePolygonsCount);

    // create Aligned-Axis bounding box tree
    g_pAABBRoot = (MINI_AABBNode*)malloc(sizeof(MINI_AABBNode));
    miniPopulateTree(g_pAABBRoot, g_pCollidePolygons, g_CollidePolygonsCount);

    // fill polygon array colors
    g_PolygonArray[3]  = 1.0f;
    g_PolygonArray[4]  = 0.0f;
    g_PolygonArray[5]  = 0.0f;
    g_PolygonArray[6]  = 1.0f;
    g_PolygonArray[10] = 0.8f;
    g_PolygonArray[11] = 0.0f;
    g_PolygonArray[12] = 0.2f;
    g_PolygonArray[13] = 1.0f;
    g_PolygonArray[17] = 1.0f;
    g_PolygonArray[18] = 0.12f;
    g_PolygonArray[19] = 0.2f;
    g_PolygonArray[20] = 1.0f;

    g_SceneInitialized = 1;

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete aabb tree
    if (g_pAABBRoot)
        miniReleaseTree(g_pAABBRoot);

    g_pAABBRoot = 0;

    // delete collide polygons
    if (g_pCollidePolygons)
        miniReleasePolygons(g_pCollidePolygons);

    g_pCollidePolygons = 0;

    // delete buffer index table
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertices
    if (g_pVertexBuffer)
    {
        free(g_pVertexBuffer);
        g_pVertexBuffer = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime)
{
    unsigned int frameCount = 0;

    // calculate next time
    g_Time += (elapsedTime * 1000.0f);

    // count frames to skip
    while (g_Time > g_Interval)
    {
        g_Time -= g_Interval;
        ++frameCount;
    }

    // calculate next rotation angle
    g_Angle += (g_RotationSpeed * frameCount);

    // is rotating angle out of bounds?
    while (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void DrawScene()
{
    MINI_Polygon* pPolygonList;
    MINI_Polygon* pPolygonsToDraw;
    unsigned      polygonsCount;
    unsigned      polygonsToDrawCount;
    unsigned      i;
    unsigned      j;
    float         determinant;
    float         xAngle;
    MINI_Vector3  t;
    MINI_Vector3  r;
    MINI_Vector3  rayPos;
    MINI_Vector3  rayDir;
    MINI_Matrix   translateMatrix;
    MINI_Matrix   xRotateMatrix;
    MINI_Matrix   yRotateMatrix;
    MINI_Matrix   rotateMatrix;
    MINI_Matrix   modelMatrix;
    MINI_Matrix   invModelMatrix;
    MINI_Ray      ray;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -2.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 90 degrees
    xAngle = 1.57075f;

    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    miniGetRotateMatrix(&g_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&xRotateMatrix, &yRotateMatrix,   &rotateMatrix);
    miniMatrixMultiply(&rotateMatrix,  &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // set ray in 3d world
    rayPos.m_X =  g_RayX;
    rayPos.m_Y =  g_RayY;
    rayPos.m_Z =  0.0f;
    rayDir.m_X =  g_RayX;
    rayDir.m_Y =  g_RayY;
    rayDir.m_Z = -1.0f;

    miniNormalize(&rayDir, &rayDir);

    // put the ray in the 3d world coordinates
    miniUnproject(&g_ProjectionMatrix,
                  &g_ViewMatrix,
                  &rayPos,
                  &rayDir);

    // put the ray in the model coordinates
    miniInverse(&modelMatrix, &invModelMatrix, &determinant);
    miniApplyMatrixToVector(&invModelMatrix, &rayPos, &ray.m_Pos);
    miniApplyMatrixToNormal(&invModelMatrix, &rayDir, &ray.m_Dir);
    miniNormalize(&ray.m_Dir, &ray.m_Dir);

    // calculate inverted ray dir. NOTE in C language, a division by 0 in this context
    // means infinity, that is needed, so don't worry about ray.m_Dir equals to 0
    ray.m_InvDir.m_X = 1.0f / ray.m_Dir.m_X;
    ray.m_InvDir.m_Y = 1.0f / ray.m_Dir.m_Y;
    ray.m_InvDir.m_Z = 1.0f / ray.m_Dir.m_Z;

    pPolygonList  = 0;
    polygonsCount = 0;

    // resolve aligned-axis bounding box tree
    miniResolveTree(&ray, g_pAABBRoot, &pPolygonList, &polygonsCount);

    pPolygonsToDraw     = 0;
    polygonsToDrawCount = 0;

    // iterate through polygons to check
    for (i = 0; i < polygonsCount; ++i)
        // is polygon intersecting ray?
        if (miniRayPolygonIntersect(&ray, &pPolygonList[i]))
        {
            // add new polygon to draw list
            if (!polygonsToDrawCount)
            {
                pPolygonsToDraw     = (MINI_Polygon*)malloc(sizeof(MINI_Polygon));
                polygonsToDrawCount = 1;
            }
            else
            {
                ++polygonsToDrawCount;
                pPolygonsToDraw = (MINI_Polygon*)realloc(pPolygonsToDraw,
                                                         polygonsToDrawCount * sizeof(MINI_Polygon));
            }

            // copy polygon
            for (j = 0; j < 3; ++j)
                miniCopy(&pPolygonList[i].m_v[j], &pPolygonsToDraw[polygonsToDrawCount - 1].m_v[j]);
        }

    // delete found polygons (no more needed from now)
    if (polygonsCount)
        free(pPolygonList);

    // draw the sphere
    miniDrawSphere(g_pVertexBuffer,
                   g_VertexCount,
                   g_pIndexes,
                   g_IndexCount,
                   &g_VertexFormat,
                   &g_Shader);

    // enable position and color slots
    glEnableVertexAttribArray(g_Shader.m_VertexSlot);
    glEnableVertexAttribArray(g_Shader.m_ColorSlot);

    // found collide polygons to draw?
    for (i = 0; i < polygonsToDrawCount; ++i)
    {
        // set vertex 1 in vertex buffer
        g_PolygonArray[0]  = pPolygonsToDraw[i].m_v[0].m_X;
        g_PolygonArray[1]  = pPolygonsToDraw[i].m_v[0].m_Y;
        g_PolygonArray[2]  = pPolygonsToDraw[i].m_v[0].m_Z;

        // set vertex 2 in vertex buffer
        g_PolygonArray[7]  = pPolygonsToDraw[i].m_v[1].m_X;
        g_PolygonArray[8]  = pPolygonsToDraw[i].m_v[1].m_Y;
        g_PolygonArray[9]  = pPolygonsToDraw[i].m_v[1].m_Z;

        // set vertex 3 in vertex buffer
        g_PolygonArray[14] = pPolygonsToDraw[i].m_v[2].m_X;
        g_PolygonArray[15] = pPolygonsToDraw[i].m_v[2].m_Y;
        g_PolygonArray[16] = pPolygonsToDraw[i].m_v[2].m_Z;

        // draw the polygon
        miniDrawBuffer(g_PolygonArray,
                       3,
                       E_Triangles,
                       &g_VertexFormat,
                       &g_Shader);
    }

    if (polygonsToDrawCount)
        free(pPolygonsToDraw);

    // disconnect slots from shader
    glDisableVertexAttribArray(g_Shader.m_VertexSlot);
    glDisableVertexAttribArray(g_Shader.m_ColorSlot);

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
    wcex.hCursor       = LoadCursor(NULL, IDC_CROSS);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = "MAPI_aabbtree";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_aabbtree",
                          "MiniAPI Aligned-Axis Bounding Box Demo",
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
