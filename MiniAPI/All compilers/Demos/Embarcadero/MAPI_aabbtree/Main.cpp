/*****************************************************************************
 * ==> Aligned-axis bounding box ray picking demo ---------------------------*
 *****************************************************************************
 * Description : A ray picking demo with aligned-axis bounding box. Move the *
 *               mouse above the sphere to select a polygon, press the left  *
 *               or right arrow keys to rotate the sphere                    *
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
#include <limits>

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
    m_pAABBRoot(0),
    m_pCollidePolygons(0),
    m_CollidePolygonsCount(0),
    m_Radius(1.0f),
    m_RayX(2.0f),
    m_RayY(2.0f),
    m_Angle(0.0f),
    m_RotationSpeed(0.0f)
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
void __fastcall TMainForm::FormMouseMove(TObject* pSender, TShiftState shift, int x, int y)
{
    // calculate the next ray position in relation to the current mouse position
    m_RayX = -1.0f + ((x * 2.0f) / ClientWidth);
    m_RayY =  1.0f - ((y * 2.0f) / ClientHeight);
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
    const float zNear  =  1.0f;
    const float zFar   =  20.0f;
    const float aspect =  w / h;
    const float left   = -aspect;
    const float right  =  aspect;
    const float top    =  1.0f;
    const float bottom = -1.0f;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    miniGetFrustum(&left, &right, &bottom, &top, &zNear, &zFar, &m_ProjectionMatrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &m_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
{
    unsigned i;

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

    miniGetIdentity(&m_ViewMatrix);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&m_Radius,
                     20,
                     20,
                     0x0000FFFF,
                     &m_VertexFormat,
                     &m_pVertexBuffer,
                     &m_VertexCount,
                     &m_pIndexes,
                     &m_IndexCount);

    // iterate through vertex indexes
    for (i = 0; i < m_IndexCount; ++i)
        // get collide polygons
        miniGetPolygonsFromVB(&m_pVertexBuffer[m_pIndexes[i].m_Start],
                              m_pIndexes[i].m_Length,
                              1,
                              m_VertexFormat.m_Stride,
                              &m_pCollidePolygons,
                              &m_CollidePolygonsCount);

    // create Aligned-Axis bounding box tree
    m_pAABBRoot = (MINI_AABBNode*)malloc(sizeof(MINI_AABBNode));
    miniPopulateTree(m_pAABBRoot, m_pCollidePolygons, m_CollidePolygonsCount);

    // fill polygon array colors
    m_PolygonArray[3]  = 1.0f;
    m_PolygonArray[4]  = 0.0f;
    m_PolygonArray[5]  = 0.0f;
    m_PolygonArray[6]  = 1.0f;
    m_PolygonArray[10] = 0.8f;
    m_PolygonArray[11] = 0.0f;
    m_PolygonArray[12] = 0.2f;
    m_PolygonArray[13] = 1.0f;
    m_PolygonArray[17] = 1.0f;
    m_PolygonArray[18] = 0.12f;
    m_PolygonArray[19] = 0.2f;
    m_PolygonArray[20] = 1.0f;
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    // delete aabb tree
    if (m_pAABBRoot)
        miniReleaseTree(m_pAABBRoot);

    m_pAABBRoot = 0;

    // delete collide polygons
    if (m_pCollidePolygons)
        miniReleasePolygons(m_pCollidePolygons);

    m_pCollidePolygons = 0;

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

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
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

    miniGetRotateMatrix(&m_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&xRotateMatrix, &yRotateMatrix,   &rotateMatrix);
    miniMatrixMultiply(&rotateMatrix,  &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // set ray in 3d world
    rayPos.m_X =  m_RayX;
    rayPos.m_Y =  m_RayY;
    rayPos.m_Z =  0.0f;
    rayDir.m_X =  m_RayX;
    rayDir.m_Y =  m_RayY;
    rayDir.m_Z = -1.0f;

    miniNormalize(&rayDir, &rayDir);

    // put the ray in the 3d world coordinates
    miniUnproject(&m_ProjectionMatrix,
                  &m_ViewMatrix,
                  &rayPos,
                  &rayDir);

    // put the ray in the model coordinates
    miniInverse(&modelMatrix, &invModelMatrix, &determinant);
    miniApplyMatrixToVector(&invModelMatrix, &rayPos, &ray.m_Pos);
    miniApplyMatrixToNormal(&invModelMatrix, &rayDir, &ray.m_Dir);
    miniNormalize(&ray.m_Dir, &ray.m_Dir);

    // calculate inverted ray dir
    ray.m_InvDir.m_X = ray.m_Dir.m_X ? (1.0f / ray.m_Dir.m_X) : std::numeric_limits<float>::infinity();
    ray.m_InvDir.m_Y = ray.m_Dir.m_Y ? (1.0f / ray.m_Dir.m_Y) : std::numeric_limits<float>::infinity();
    ray.m_InvDir.m_Z = ray.m_Dir.m_Z ? (1.0f / ray.m_Dir.m_Z) : std::numeric_limits<float>::infinity();

    pPolygonList  = 0;
    polygonsCount = 0;

    // resolve aligned-axis bounding box tree
    miniResolveTree(&ray, m_pAABBRoot, &pPolygonList, &polygonsCount);

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
                pPolygonsToDraw[polygonsToDrawCount - 1].m_v[j] = pPolygonList[i].m_v[j];
        }

    // delete found polygons (no more needed from now)
    if (polygonsCount)
        free(pPolygonList);

    // draw the sphere
    miniDrawSphere(m_pVertexBuffer,
                   m_VertexCount,
                   m_pIndexes,
                   m_IndexCount,
                   &m_VertexFormat,
                   &m_Shader);

    // enable position and color slots
    glEnableVertexAttribArray(m_Shader.m_VertexSlot);
    glEnableVertexAttribArray(m_Shader.m_ColorSlot);

    // found collide polygons to draw?
    for (i = 0; i < polygonsToDrawCount; ++i)
    {
        // set vertex 1 in vertex buffer
        m_PolygonArray[0]  = pPolygonsToDraw[i].m_v[0].m_X;
        m_PolygonArray[1]  = pPolygonsToDraw[i].m_v[0].m_Y;
        m_PolygonArray[2]  = pPolygonsToDraw[i].m_v[0].m_Z;

        // set vertex 2 in vertex buffer
        m_PolygonArray[7]  = pPolygonsToDraw[i].m_v[1].m_X;
        m_PolygonArray[8]  = pPolygonsToDraw[i].m_v[1].m_Y;
        m_PolygonArray[9]  = pPolygonsToDraw[i].m_v[1].m_Z;

        // set vertex 3 in vertex buffer
        m_PolygonArray[14] = pPolygonsToDraw[i].m_v[2].m_X;
        m_PolygonArray[15] = pPolygonsToDraw[i].m_v[2].m_Y;
        m_PolygonArray[16] = pPolygonsToDraw[i].m_v[2].m_Z;

        // draw the polygon
        miniDrawBuffer(m_PolygonArray,
                       3,
                       E_Triangles,
                       &m_VertexFormat,
                       &m_Shader);
    }

    if (polygonsToDrawCount)
        free(pPolygonsToDraw);

    // disconnect slots from shader
    glDisableVertexAttribArray(m_Shader.m_VertexSlot);
    glDisableVertexAttribArray(m_Shader.m_ColorSlot);

    miniEndScene();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnIdle(TObject* pSender, bool& done)
{
    FormPaint(pSender);
    done = false;
}
//---------------------------------------------------------------------------
