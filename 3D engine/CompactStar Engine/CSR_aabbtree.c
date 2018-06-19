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
#include <string.h>

// opengl
#include <gles2.h>
#include <gles2ext.h>

// mini API
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Geometry.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Collision.h"
#include "SDK/CSR_Shader.h"
#include "SDK/CSR_Renderer.h"

// libraries
#include <ccr.h>

//------------------------------------------------------------------------------
unsigned char g_VSProgram[] = "precision mediump float;"
                              "attribute vec4 csr_aVertices;"
                              "attribute vec4 csr_aColor;"
                              "uniform   mat4 csr_uProjection;"
                              "uniform   mat4 csr_uModelview;"
                              "varying   vec4 csr_vColor;"
                              "void main(void)"
                              "{"
                              "    csr_vColor  = csr_aColor;"
                              "    gl_Position = csr_uProjection * csr_uModelview * csr_aVertices;"
                              "}";
//----------------------------------------------------------------------------
unsigned char g_FSProgram[] = "precision mediump float;"
                              "varying lowp vec4 csr_vColor;"
                              "void main(void)"
                              "{"
                              "    gl_FragColor = csr_vColor;"
                              "}";
//------------------------------------------------------------------------------
CSR_Shader*        g_pShader              = 0;
CSR_Mesh*          g_pMesh                = 0;
CSR_AABBNode*      g_pAABBRoot            = 0;
float              g_Radius               = 1.0f;
float              g_RayX                 = 2.0f;
float              g_RayY                 = 2.0f;
float              g_Angle                = 0.0f;
float              g_RotationSpeed        = 0.0f;
float              g_Time                 = 0.0f;
float              g_Interval             = 0.0f;
const unsigned int g_FPS                  = 15;
CSR_Rect           g_View;
CSR_Color          g_Background;
CSR_Matrix4        g_ProjectionMatrix;
CSR_Matrix4        g_ViewMatrix;
float              g_PolygonArray[21];
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

    csrMat4Frustum(left, right, bottom, top, zNear, zFar, &g_ProjectionMatrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_Buffer*      pVSBuffer;
    CSR_Buffer*      pFSBuffer;
    unsigned char*   pVSProgram;
    unsigned char*   pFSProgram;
    CSR_VertexFormat vertexFormat;
    CSR_Material     material;

    // initialize the scene background color
    g_Background.m_R = 0.0f;
    g_Background.m_G = 0.0f;
    g_Background.m_B = 0.0f;
    g_Background.m_A = 1.0f;

    // initialize the viewbox
    g_View.m_Min.m_X = 0.0f;
    g_View.m_Min.m_Y = 0.0f;
    g_View.m_Max.m_X = 0.0f;
    g_View.m_Max.m_Y = 0.0f;

    csrMat4Identity(&g_ViewMatrix);

    // compile, link and use shader
    g_pShader = csrShaderLoadFromStr(&g_VSProgram,
                                      sizeof(g_VSProgram),
                                     &g_FSProgram,
                                      sizeof(g_FSProgram),
                                      0,
                                      0);

    // enable the shader
    csrShaderEnable(g_pShader);

    // get shader attributes
    g_pShader->m_VertexSlot = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_ColorSlot  = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aColor");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 0;
    vertexFormat.m_HasPerVertexColor = 1;

    // configure the material
    material.m_Color       = 0xFFFF;
    material.m_Transparent = 0;

    // create a sphere
    g_pMesh = csrShapeCreateSphere(g_Radius, 20, 24, &vertexFormat, 0, &material, 0);

    // extract the AABB tree from the sphere mesh
    g_pAABBRoot = csrAABBTreeFromMesh(g_pMesh);

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

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete aabb tree
    csrAABBTreeNodeRelease(g_pAABBRoot);
    g_pAABBRoot = 0;

    // delete mesh
    csrMeshRelease(g_pMesh);
    g_pMesh = 0;

    // delete shader
    csrShaderRelease(g_pShader);
    g_pShader = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    // get view size
    g_View.m_Max.m_X = view_w;
    g_View.m_Max.m_Y = view_h;

    glViewport(0, 0, view_w, view_h);
    ApplyMatrix(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    unsigned int frameCount = 0;

    // calculate next time
    g_Time += (timeStep_sec * 1000.0f);

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
void on_GLES2_Render()
{
    CSR_Polygon3Buffer polygons;
    CSR_Polygon3*      pPolygonsToDraw;
    CSR_Polygon3*      pNewPolygonsToDraw;
    unsigned           polygonsToDrawCount;
    unsigned           i;
    unsigned           j;
    float              determinant;
    float              xAngle;
    CSR_Vector3        t;
    CSR_Vector3        r;
    CSR_Vector3        rayPos;
    CSR_Vector3        rayDir;
    CSR_Vector3        rayDirN;
    CSR_Matrix4        translateMatrix;
    CSR_Matrix4        xRotateMatrix;
    CSR_Matrix4        yRotateMatrix;
    CSR_Matrix4        rotateMatrix;
    CSR_Matrix4        modelMatrix;
    CSR_Matrix4        invModelMatrix;
    CSR_Ray3           ray;
    CSR_Figure3        rayFigure;
    CSR_Figure3        polygonFigure;
    CSR_Mesh           polygonMesh;
    CSR_VertexBuffer   polygonVB;

    csrDrawBegin(&g_Background);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -2.0f;

    csrMat4Translate(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 90 degrees
    xAngle = M_PI / 2.0f;

    csrMat4Rotate(xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    csrMat4Rotate(g_Angle, &r, &yRotateMatrix);

    // build model view matrix
    csrMat4Multiply(&xRotateMatrix, &yRotateMatrix,   &rotateMatrix);
    csrMat4Multiply(&rotateMatrix,  &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelUniform = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uModelview");
    glUniformMatrix4fv(modelUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // get the ray from screen coordinates
    ray.m_Pos.m_X =  g_RayX;
    ray.m_Pos.m_Y =  g_RayY;
    ray.m_Pos.m_Z =  0.0f;
    ray.m_Dir.m_X =  g_RayX;
    ray.m_Dir.m_Y =  g_RayY;
    ray.m_Dir.m_Z = -1.0f;

    // put the ray in the 3d world coordinates
    csrMat4Unproject(&g_ProjectionMatrix, &g_ViewMatrix, &ray);

    // put the ray in the model coordinates
    csrMat4Inverse(&modelMatrix, &invModelMatrix, &determinant);
    csrMat4ApplyToVector(&invModelMatrix, &ray.m_Pos, &rayPos);
    csrMat4ApplyToNormal(&invModelMatrix, &ray.m_Dir, &rayDir);
    csrVec3Normalize(&rayDir, &rayDirN);

    // get the transformed tay
    csrRay3FromPointDir(&rayPos, &rayDirN, &ray);

    // resolve aligned-axis bounding box tree
    csrAABBTreeResolve(&ray, g_pAABBRoot, 0, &polygons);

    // get a figure from the ray
    rayFigure.m_Type    = CSR_F3_Ray;
    rayFigure.m_pFigure = &ray;

    pPolygonsToDraw     = 0;
    polygonsToDrawCount = 0;

    // iterate through polygons to check
    for (i = 0; i < polygons.m_Count; ++i)
    {
        // get a figure from the ray
        polygonFigure.m_Type    = CSR_F3_Polygon;
        polygonFigure.m_pFigure = &polygons.m_pPolygon[i];

        // is polygon intersecting ray?
        if (csrIntersect3(&rayFigure, &polygonFigure, 0, 0, 0))
        {
            // add new polygon to draw list
            ++polygonsToDrawCount;
            pNewPolygonsToDraw = (CSR_Polygon3*)csrMemoryAlloc(pPolygonsToDraw,
                                                               sizeof(CSR_Polygon3),
                                                               polygonsToDrawCount);

            // succeeded?
            if (!pNewPolygonsToDraw)
            {
                free(pPolygonsToDraw);
                csrDrawEnd();
                return;
            }

            // update the polygon buffer
            pPolygonsToDraw = pNewPolygonsToDraw;

            // copy polygon
            for (j = 0; j < 3; ++j)
                pPolygonsToDraw[polygonsToDrawCount - 1].m_Vertex[j] =
                        polygons.m_pPolygon[i].m_Vertex[j];
        }
    }

    // delete found polygons (no more needed from now)
    if (polygons.m_Count)
        free(polygons.m_pPolygon);

    // draw the sphere
    csrDrawMesh(g_pMesh, g_pShader, 0);

    // enable position and color slots
    glEnableVertexAttribArray(g_pShader->m_VertexSlot);
    glEnableVertexAttribArray(g_pShader->m_ColorSlot);

    // configure the polygon mesh
    polygonMesh.m_Shader.m_TextureID = GL_INVALID_VALUE;
    polygonMesh.m_Shader.m_BumpMapID = GL_INVALID_VALUE;
    polygonMesh.m_Time               = 0.0;
    polygonMesh.m_pVB                = &polygonVB;
    polygonMesh.m_Count              = 1;

    // configure the polygon vertex buffer
    polygonVB.m_Format.m_Type              = CSR_VT_Triangles;
    polygonVB.m_Format.m_HasNormal         = 0;
    polygonVB.m_Format.m_HasTexCoords      = 0;
    polygonVB.m_Format.m_HasPerVertexColor = 1;
    polygonVB.m_Format.m_Stride            = 7;
    polygonVB.m_Culling.m_Type             = CSR_CT_None;
    polygonVB.m_Culling.m_Face             = CSR_CF_CCW;
    polygonVB.m_Material.m_Color           = 0xFFFFFFFF;
    polygonVB.m_Material.m_Transparent     = 0;
    polygonVB.m_Time                       = 0.0;
    polygonVB.m_pData                      = &g_PolygonArray;
    polygonVB.m_Count                      = sizeof(g_PolygonArray) / sizeof(float);

    // found collide polygons to draw?
    for (i = 0; i < polygonsToDrawCount; ++i)
    {
        // set vertex 1 in vertex buffer
        g_PolygonArray[0]  = pPolygonsToDraw[i].m_Vertex[0].m_X;
        g_PolygonArray[1]  = pPolygonsToDraw[i].m_Vertex[0].m_Y;
        g_PolygonArray[2]  = pPolygonsToDraw[i].m_Vertex[0].m_Z;

        // set vertex 2 in vertex buffer
        g_PolygonArray[7]  = pPolygonsToDraw[i].m_Vertex[1].m_X;
        g_PolygonArray[8]  = pPolygonsToDraw[i].m_Vertex[1].m_Y;
        g_PolygonArray[9]  = pPolygonsToDraw[i].m_Vertex[1].m_Z;

        // set vertex 3 in vertex buffer
        g_PolygonArray[14] = pPolygonsToDraw[i].m_Vertex[2].m_X;
        g_PolygonArray[15] = pPolygonsToDraw[i].m_Vertex[2].m_Y;
        g_PolygonArray[16] = pPolygonsToDraw[i].m_Vertex[2].m_Z;

        // draw the polygon
        csrDrawMesh(&polygonMesh, g_pShader, 0);
    }

    if (polygonsToDrawCount)
        free(pPolygonsToDraw);

    // disconnect slots from shader
    glDisableVertexAttribArray(g_pShader->m_VertexSlot);
    glDisableVertexAttribArray(g_pShader->m_ColorSlot);

    csrDrawEnd();
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    // convert screen coordinates to ray world coordinate and get ray position
    g_RayX = -1.0f + ((x * 2.0f) / g_View.m_Max.m_X);
    g_RayY =  1.0f - ((y * 2.0f) / g_View.m_Max.m_Y);
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
