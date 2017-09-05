/*****************************************************************************
 * ==> Aligned-Axis collision detection demo --------------------------------*
 *****************************************************************************
 * Description : Collision detection with aligned-axis bounding box tree     *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

// std
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef ANDROID
    #include <gles2/gl2.h>
    #include <gles2/gl2ext.h>
#endif
#ifdef IOS
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#endif

// mini API
#include "MiniAPI/MiniGeometry.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniShapes.h"
#include "MiniAPI/MiniCollision.h"
#include "MiniAPI/MiniShader.h"

#ifdef ANDROID
    #define EARTH_TEXTURE_FILE "/sdcard/C++ Compiler/earthmap.bmp"
#else
    #define EARTH_TEXTURE_FILE "Private/Resources/earthmap.bmp"
#endif

//------------------------------------------------------------------------------
#ifndef ANDROID
    GLuint g_Renderbuffer, g_Framebuffer;
#endif
GLuint             g_ShaderProgram        = 0;
float*             g_pVertexBuffer        = 0;
int                g_VertexCount          = 0;
MV_Index*          g_pIndexes             = 0;
int                g_IndexCount           = 0;
MC_AABBNode*       g_pAABBRoot            = 0;
MC_Polygon*        g_pCollidePolygons     = 0;
int                g_CollidePolygonsCount = 0;
float              g_Radius               = 5.0f;
float              g_RayX                 = 0.0f;
float              g_RayY                 = 0.0f;
GLuint             g_PositionSlot         = 0;
GLuint             g_ColorSlot            = 0;
MV_VertexFormat    g_VertexFormat;
float              g_PolygonArray[21];
//------------------------------------------------------------------------------
void ApplyOrtho(float maxX, float maxY) const
{
    // get orthogonal matrix
    float     left   = -5.0f;
    float     right  =  5.0f;
    float     bottom = -5.0f * 1.12f;
    float     top    =  5.0f * 1.12f;
    float     near   =  1.0f;
    float     far    =  20.0f;
    MG_Matrix ortho;
    GetOrtho(&left, &right, &bottom, &top, &near, &far, &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    #ifndef ANDROID
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

    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(g_pVSColored, g_pFSColored);
    glUseProgram(g_ShaderProgram);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere
    CreateSphere(&g_Radius,
                 5,
                 12,
                 0x0000FFFF,
                 &g_VertexFormat,
                 &g_pVertexBuffer,
                 &g_VertexCount,
                 &g_pIndexes,
                 &g_IndexCount);

    // iterate through vertex indexes
    for (unsigned i = 0; i < g_IndexCount; ++i)
        // get collide polygons
        GetPolygonsFromVB(&g_pVertexBuffer[g_pIndexes[i].m_Start],
                          g_pIndexes[i].m_Length,
                          1,
                          g_VertexFormat.m_Stride,
                          &g_pCollidePolygons,
                          &g_CollidePolygonsCount);

    // create Aligned-Axis bounding box tree
    g_pAABBRoot = (MC_AABBNode*)malloc(sizeof(MC_AABBNode));
    PopulateTree(g_pAABBRoot, g_pCollidePolygons, g_CollidePolygonsCount);

    // get shader attributes
    g_PositionSlot = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_vColor");

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

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // disable culling
    glDisable(GL_CULL_FACE);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete aabb tree
    if (g_pAABBRoot)
        ReleaseTree(g_pAABBRoot);

    // delete collide polygons
    if (g_pCollidePolygons)
        ReleasePolygons(g_pCollidePolygons);

    // delete buffer index table
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertexes
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
    glViewport(0, 0, view_w, view_h);
    ApplyOrtho(2.0f, 2.0f);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    MC_Polygon* pPolygonList;
    MC_Polygon* pPolygonsToDraw;
    unsigned    polygonsCount;
    unsigned    polygonsToDrawCount;
    unsigned    i;
    unsigned    j;
    int         stride;
    float       xAngle;
    MG_Vector3  r;
    MG_Matrix   xRotateMatrix;
    MG_Matrix   yRotateMatrix;
    MG_Matrix   modelViewMatrix;
    GLvoid*     pCoords;
    GLvoid*     pColors;
    MG_Ray      ray;

    // clear scene background and depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GetIdentity(&modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // set ray in 3d world
    ray.m_Pos.m_X =  g_RayX;
    ray.m_Pos.m_Y =  g_RayY;
    ray.m_Pos.m_Z = -10.0f;
    ray.m_Dir.m_X =  0.0f;
    ray.m_Dir.m_Y =  0.0f;
    ray.m_Dir.m_Z =  1.0f;

    // calculate inverted ray dir. NOTE in C language, a division by 0 in this context
    // means infinity, that is needed, so don't worry about ray.m_Dir equals to 0
    ray.m_InvDir.m_X = 1.0f / ray.m_Dir.m_X;
    ray.m_InvDir.m_Y = 1.0f / ray.m_Dir.m_Y;
    ray.m_InvDir.m_Z = 1.0f / ray.m_Dir.m_Z;

    pPolygonList  = 0;
    polygonsCount = 0;

    // resolve aligned-axis bounding box tree
    ResolveTree(&ray, g_pAABBRoot, &pPolygonList, &polygonsCount);

    pPolygonsToDraw     = 0;
    polygonsToDrawCount = 0;

    // iterate through polygons to check
    for (i = 0; i < polygonsCount; ++i)
        // is polygon intersecting ray?
        if (TriRayIntersect(&ray, &pPolygonList[i]))
        {
            // add new polygon to draw list
            if (!polygonsToDrawCount)
            {
                pPolygonsToDraw     = (MC_Polygon*)malloc(sizeof(MC_Polygon));
                polygonsToDrawCount = 1;
            }
            else
            {
                ++polygonsToDrawCount;
                pPolygonsToDraw = (MC_Polygon*)realloc(pPolygonsToDraw,
                                                       polygonsToDrawCount * sizeof(MC_Polygon));
            }

            // copy polygon
            for (j = 0; j < 3; ++j)
                Copy(&pPolygonList[i].m_v[j], &pPolygonsToDraw[polygonsToDrawCount - 1].m_v[j]);
        }

    // delete found polygons (no more needed from now)
    if (polygonsCount)
        free(pPolygonList);

    // calculate vertex stride
    stride = g_VertexFormat.m_Stride;

    // enable position and color slots
    glEnableVertexAttribArray(g_PositionSlot);
    glEnableVertexAttribArray(g_ColorSlot);

    // iterate through vertex fan buffers to draw
    for (int i = 0; i < g_IndexCount; ++i)
    {
        // get next vertexes fan buffer
        pCoords = &g_pVertexBuffer[g_pIndexes[i].m_Start];
        pColors = &g_pVertexBuffer[g_pIndexes[i].m_Start + 3];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

        // draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, g_pIndexes[i].m_Length / stride);
    }

    // found collide polygons to draw?
    for (i = 0; i < polygonsToDrawCount; ++i)
    {
        // draw vertex 1
        g_PolygonArray[0]  = pPolygonsToDraw[i].m_v[0].m_X;
        g_PolygonArray[1]  = pPolygonsToDraw[i].m_v[0].m_Y;
        g_PolygonArray[2]  = pPolygonsToDraw[i].m_v[0].m_Z;

        // draw vertex 2
        g_PolygonArray[7]  = pPolygonsToDraw[i].m_v[1].m_X;
        g_PolygonArray[8]  = pPolygonsToDraw[i].m_v[1].m_Y;
        g_PolygonArray[9]  = pPolygonsToDraw[i].m_v[1].m_Z;

        // draw vertex 3
        g_PolygonArray[14] = pPolygonsToDraw[i].m_v[2].m_X;
        g_PolygonArray[15] = pPolygonsToDraw[i].m_v[2].m_Y;
        g_PolygonArray[16] = pPolygonsToDraw[i].m_v[2].m_Z;

        // get next vertexes fan buffer
        pCoords = &g_PolygonArray[0];
        pColors = &g_PolygonArray[3];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), pColors);

        // draw it
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    if (polygonsToDrawCount)
        free(pPolygonsToDraw);

    // disconnect slots from shader
    glDisableVertexAttribArray(g_PositionSlot);
    glDisableVertexAttribArray(g_ColorSlot);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    const float maxX = 10.0f;
    const float maxY = 10.0f;

    // convert screen coordinates (0, 0, 320, 370) to ray world coordinate
    g_RayX = ((x * maxX) / 320.0f) - 5.0f;
    g_RayY = 5.0f - ((y * maxY) / 370.0f);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{}
//------------------------------------------------------------------------------
#ifdef IOS
    void on_GLES2_DeviceRotate(int orientation)
    {}
#endif
//------------------------------------------------------------------------------
