/*****************************************************************************
 * ==> Aligned-axis bounding box ray picking demo ---------------------------*
 *****************************************************************************
 * Description : A ray picking demo with aligned-axis bounding box. Tap      *
 *               anywhere on the sphere to select a polygon, swipe to the    *
 *               left or right to rotate the sphere                          *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// supported platforms check. NOTE iOS only, but may works on other platforms
#if !defined(_OS_IOS_) && !defined(_OS_ANDROID_) && !defined(_OS_WINDOWS_)
    #error "Not supported platform!"
#endif

#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// opengl
#include <gles2.h>
#include <gles2ext.h>

// mini API
#include "MiniAPI/MiniCommon.h"
#include "MiniAPI/MiniGeometry.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniShapes.h"
#include "MiniAPI/MiniCollision.h"
#include "MiniAPI/MiniShader.h"
#include "MiniAPI/MiniRenderer.h"

#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    #include <ccr.h>
#endif

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram        = 0;
float*            g_pVertexBuffer        = 0;
unsigned int      g_VertexCount          = 0;
MINI_Index*       g_pIndexes             = 0;
unsigned int      g_IndexCount           = 0;
MINI_AABBNode*    g_pAABBRoot            = 0;
MINI_Polygon*     g_pCollidePolygons     = 0;
unsigned          g_CollidePolygonsCount = 0;
float             g_Radius               = 1.0f;
float             g_RayX                 = 2.0f;
float             g_RayY                 = 2.0f;
float             g_Angle                = 0.0f;
float             g_RotationSpeed        = 0.0f;
MINI_Size         g_View;
MINI_VertexFormat g_VertexFormat;
MINI_Matrix       g_ProjectionMatrix;
MINI_Matrix       g_ViewMatrix;
float             g_PolygonArray[21];
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
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
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    // renderer buffers should no more be generated since CCR version 1.1
    #if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
        #ifndef _OS_ANDROID_
            // generate and bind in memory frame buffers to render to
            glGenRenderbuffers(1, &g_Renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, g_Renderbuffer);
            glGenFramebuffers(1,&g_Framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_RENDERBUFFER,
                                      g_Renderbuffer);
        #endif
    #endif

    g_View.m_Width  = 0.0f;
    g_View.m_Height = 0.0f;

    miniGetIdentity(&g_ViewMatrix);

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&g_Radius,
                     20,
                     24,
                     0x0000FFFF,
                     &g_VertexFormat,
                     &g_pVertexBuffer,
                     &g_VertexCount,
                     &g_pIndexes,
                     &g_IndexCount);

    // iterate through vertex indexes
    for (unsigned i = 0; i < g_IndexCount; ++i)
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

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "mini_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "mini_vColor");

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
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
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
void on_GLES2_Size(int view_w, int view_h)
{
    // get view size
    g_View.m_Width  = view_w;
    g_View.m_Height = view_h;

    glViewport(0, 0, view_w, view_h);
    ApplyMatrix(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // calculate next rotation angle
    g_Angle += (g_RotationSpeed * timeStep_sec * 10.0f);

    // is rotating angle out of bounds?
    while (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
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
    GLint modelUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
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
                pPolygonsToDraw[polygonsToDrawCount - 1].m_v[j] = pPolygonList[i].m_v[j];
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
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    // convert screen coordinates to ray world coordinate and get ray position
    g_RayX = -1.0f + ((x * 2.0f) / g_View.m_Width);
    g_RayY =  1.0f - ((y * 2.0f) / g_View.m_Height);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
    // increase or decrease rotation speed
    g_RotationSpeed += (x - prev_x) * 0.001f;
}
//------------------------------------------------------------------------------
#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    int main()
    {
        ccrSet_GLES2_Init_Callback(on_GLES2_Init);
        ccrSet_GLES2_Final_Callback(on_GLES2_Final);
        ccrSet_GLES2_Size_Callback(on_GLES2_Size);
        ccrSet_GLES2_Update_Callback(on_GLES2_Update);
        ccrSet_GLES2_Render_Callback(on_GLES2_Render);
        ccrSet_GLES2_TouchBegin_Callback(on_GLES2_TouchBegin);
        ccrSet_GLES2_TouchMove_Callback(on_GLES2_TouchMove);
        ccrSet_GLES2_TouchEnd_Callback(on_GLES2_TouchEnd);

        ccrBegin_GLES2_Drawing();

        while (ccrGetEvent(false) != CCR_EVENT_QUIT);

        ccrEnd_GLES2_Drawing();

        return 0;
    }
#endif
//------------------------------------------------------------------------------
