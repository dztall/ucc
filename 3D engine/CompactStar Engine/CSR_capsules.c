/****************************************************************************
 * ==> Capsules demo -------------------------------------------------------*
 ****************************************************************************
 * Description : Capsules and collision demo                                *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2021, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

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

// compactStar engine
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Geometry.h"
#include "SDK/CSR_Collision.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_AI.h"
#include "SDK/CSR_Sound.h"

#include <ccr.h>

//------------------------------------------------------------------------------
const char g_VSColored[] = "precision mediump float;"
                           "attribute vec3  csr_aVertices;"
                           "attribute vec4  csr_aColor;"
                           "uniform   float csr_uAlpha;"
                           "uniform   float csr_uCollision;"
                           "uniform   mat4  csr_uProjection;"
                           "uniform   mat4  csr_uView;"
                           "uniform   mat4  csr_uModel;"
                           "varying   vec4  csr_vColor;"
                           "varying   float csr_fAlpha;"
                           "varying   float csr_fCollision;"
                           "void main(void)"
                           "{"
                           "    csr_vColor     = csr_aColor;"
                           "    csr_fAlpha     = csr_uAlpha;"
                           "    csr_fCollision = csr_uCollision;"
                           "    gl_Position    = csr_uProjection * csr_uView * csr_uModel * vec4(csr_aVertices, 1.0);"
                           "}";
//----------------------------------------------------------------------------
const char g_FSColored[] = "precision mediump float;"
                           "varying lowp vec4  csr_vColor;"
                           "varying      float csr_fAlpha;"
                           "varying      float csr_fCollision;"
                           "void main(void)"
                           "{"
                           "    if (csr_fCollision > 0.5)"
                           "        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);"
                           "    else"
                           "        gl_FragColor = vec4(csr_vColor.x, csr_vColor.y, csr_vColor.z, csr_fAlpha);"
                           "}";
//------------------------------------------------------------------------------
CSR_Scene*        g_pScene        = 0;
CSR_OpenGLShader* g_pShader       = 0;
CSR_Mesh*         g_pCapsule1Mesh = 0;
CSR_Mesh*         g_pCapsule2Mesh = 0;
CSR_SceneContext  g_SceneContext;
CSR_Capsule       g_Capsule1;
CSR_Capsule       g_Capsule2;
CSR_Matrix4       g_Capsule1Matrix;
CSR_Matrix4       g_Capsule2Matrix;
GLuint            g_AlphaSlot     = 0;
GLuint            g_CollisionSlot = 0;
size_t            g_FrameCount    = 0;
int               g_Collision     = 0;
float             g_DeltaX        = 0.5f;
float             g_Angle         = 0.0f;
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    csrShaderEnable(g_pShader);

    // are capsules colliding?
    if (g_Collision)
        glUniform1f(g_CollisionSlot, 1.0f);
    else
        glUniform1f(g_CollisionSlot, 0.0f);

    glUniform1f(g_AlphaSlot, 1.0f);

    return g_pShader;
}
//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.01f;
    const float zFar   = 100.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    csrMat4Perspective(fov, aspect, zNear, zFar, &g_pScene->m_ProjectionMatrix);

    csrShaderEnable(g_pShader);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_VertexFormat  vertexFormat;
    CSR_VertexCulling vertexCulling;
    CSR_Material      material;
    CSR_SceneItem*    pSceneItem;

    // initialize the scene
    g_pScene = csrSceneCreate();

    // configure the scene background color
    g_pScene->m_Color.m_R = 0.08f;
    g_pScene->m_Color.m_G = 0.12f;
    g_pScene->m_Color.m_B = 0.17f;
    g_pScene->m_Color.m_A = 1.0f;

    // configure the scene view matrix
    csrMat4Identity(&g_pScene->m_ViewMatrix);

    // configure the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnGetShader = OnGetShader;

    // compile, link and use shader
    g_pShader = csrOpenGLShaderLoadFromStr(&g_VSColored[0],
                                            sizeof(g_VSColored),
                                           &g_FSColored[0],
                                            sizeof(g_FSColored),
                                            0,
                                            0);

    // succeeded?
    if (!g_pShader)
    {
    	printf("FAILED to compile shader\n");
        return;
    }

    csrShaderEnable(g_pShader);

    // get shader attributes
    g_pShader->m_VertexSlot = glGetAttribLocation (g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_ColorSlot  = glGetAttribLocation (g_pShader->m_ProgramID, "csr_aColor");
    g_AlphaSlot             = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uAlpha");
    g_CollisionSlot         = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uCollision");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // initialize the first capsule
    g_Capsule1.m_Top.m_X    =  0.0f;
    g_Capsule1.m_Top.m_Y    =  1.0f;
    g_Capsule1.m_Top.m_Z    = -2.0f;
    g_Capsule1.m_Bottom.m_X =  0.0f;
    g_Capsule1.m_Bottom.m_Y =  0.0f;
    g_Capsule1.m_Bottom.m_Z = -2.0f;
    g_Capsule1.m_Radius     =  0.2f;

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 0;
    vertexFormat.m_HasPerVertexColor = 1;

    vertexCulling.m_Face = CSR_CF_CCW;
    vertexCulling.m_Type = CSR_CT_Back;

    // configure the material
    material.m_Color       = 0xFF0000FF;
    material.m_Transparent = 1;
    material.m_Wireframe   = 0;

    // initialize first capsule matrix
    csrMat4Identity(&g_Capsule1Matrix);
    g_Capsule1Matrix.m_Table[3][0] = g_Capsule1.m_Bottom.m_X + g_DeltaX;
    g_Capsule1Matrix.m_Table[3][1] = g_Capsule1.m_Bottom.m_Y;
    g_Capsule1Matrix.m_Table[3][2] = g_Capsule1.m_Bottom.m_Z;

    // create a capsule
    g_pCapsule1Mesh = csrShapeCreateCapsule(1.0f, 0.2f, 10.0f, &vertexFormat, 0, &material, 0);
    pSceneItem      = csrSceneAddMesh(g_pScene, g_pCapsule1Mesh, 1, 0);
    csrSceneAddModelMatrix(g_pScene, g_pCapsule1Mesh, &g_Capsule1Matrix);

    // initialize the second capsule
    g_Capsule2.m_Top.m_X    =  0.0f;
    g_Capsule2.m_Top.m_Y    =  1.0f;
    g_Capsule2.m_Top.m_Z    = -2.0f;
    g_Capsule2.m_Bottom.m_X =  0.0f;
    g_Capsule2.m_Bottom.m_Y =  0.0f;
    g_Capsule2.m_Bottom.m_Z = -2.0f;
    g_Capsule2.m_Radius     =  0.2f;

    // configure the material
    material.m_Color = 0x0000FFFF;

    // initialize second capsule matrix
    csrMat4Identity(&g_Capsule2Matrix);
    g_Capsule2Matrix.m_Table[3][0] = g_Capsule2.m_Bottom.m_X - g_DeltaX;
    g_Capsule2Matrix.m_Table[3][1] = g_Capsule2.m_Bottom.m_Y;
    g_Capsule2Matrix.m_Table[3][2] = g_Capsule2.m_Bottom.m_Z;

    // create a capsule
    g_pCapsule2Mesh = csrShapeCreateCapsule(1.0f, 0.2f, 10.0f, &vertexFormat, 0, &material, 0);
    pSceneItem      = csrSceneAddMesh(g_pScene, g_pCapsule2Mesh, 1, 0);
    csrSceneAddModelMatrix(g_pScene, g_pCapsule2Mesh, &g_Capsule2Matrix);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the scene
    csrSceneRelease(g_pScene, 0);
    g_pScene = 0;

    // delete scene shader
    csrOpenGLShaderRelease(g_pShader);
    g_pShader = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
    CreateViewport(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // create the X rotation matrix
    CSR_Matrix4 rotMatX;
    CSR_Vector3 axis;
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    csrMat4Rotate(0.0f, &axis, &rotMatX);

    // create the Y rotation matrix
    CSR_Matrix4 rotMatY;
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;
    csrMat4Rotate(0.0f, &axis, &rotMatY);

    // create the Y rotation matrix
    CSR_Matrix4 rotMatZ;
    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;
    csrMat4Rotate(g_Angle, &axis, &rotMatZ);

    CSR_Matrix4 buildMat;
    CSR_Matrix4 matrix;

    // combine the rotation matrices
    csrMat4Multiply(&rotMatX,  &rotMatY, &buildMat);
    csrMat4Multiply(&buildMat, &rotMatZ, &matrix);

    // update first capsule matrix
    g_Capsule1Matrix               =  matrix;
    g_Capsule1Matrix.m_Table[3][0] =  g_Capsule1.m_Bottom.m_X + g_DeltaX;
    g_Capsule1Matrix.m_Table[3][1] = -0.5f;
    g_Capsule1Matrix.m_Table[3][2] =  g_Capsule1.m_Bottom.m_Z;

    // combine the rotation matrices
    csrMat4Rotate  (-g_Angle,   &axis,    &rotMatZ);
    csrMat4Multiply( &buildMat, &rotMatZ, &matrix);

    // update second capsule matrix
    g_Capsule2Matrix               =  matrix;
    g_Capsule2Matrix.m_Table[3][0] =  g_Capsule2.m_Bottom.m_X - g_DeltaX;
    g_Capsule2Matrix.m_Table[3][1] = -0.5f;
    g_Capsule2Matrix.m_Table[3][2] =  g_Capsule2.m_Bottom.m_Z;

    CSR_Capsule capsule1;
    CSR_Capsule capsule2;

    CSR_Vector3 cap1Top;
    cap1Top.m_X = 0.0f;
    cap1Top.m_Y = g_Capsule1.m_Top.m_Y;
    cap1Top.m_Z = 0.0f;

    CSR_Vector3 cap1Bottom;
    cap1Bottom.m_X = 0.0f;
    cap1Bottom.m_Y = g_Capsule1.m_Bottom.m_Y;
    cap1Bottom.m_Z = 0.0f;

    CSR_Vector3 cap2Top;
    cap2Top.m_X = 0.0f;
    cap2Top.m_Y = g_Capsule2.m_Top.m_Y;
    cap2Top.m_Z = 0.0f;

    CSR_Vector3 cap2Bottom;
    cap2Bottom.m_X = 0.0f;
    cap2Bottom.m_Y = g_Capsule2.m_Bottom.m_Y;
    cap2Bottom.m_Z = 0.0f;

    csrMat4Transform(&g_Capsule1Matrix, &cap1Top,    &capsule1.m_Top);
    csrMat4Transform(&g_Capsule1Matrix, &cap1Bottom, &capsule1.m_Bottom);

    csrMat4Transform(&g_Capsule2Matrix, &cap2Top,    &capsule2.m_Top);
    csrMat4Transform(&g_Capsule2Matrix, &cap2Bottom, &capsule2.m_Bottom);

    capsule1.m_Radius = g_Capsule1.m_Radius;
    capsule2.m_Radius = g_Capsule2.m_Radius;

    CSR_Figure3 figure1;
    figure1.m_pFigure = &capsule1;
    figure1.m_Type    = CSR_F3_Capsule;

    CSR_Figure3 figure2;
    figure2.m_pFigure = &capsule2;
    figure2.m_Type    = CSR_F3_Capsule;

    // test collision between capsules
    g_Collision = csrIntersect3(&figure1, &figure2, 0, 0, 0);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    // draw the scene
    csrSceneDraw(g_pScene, &g_SceneContext);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
    // move capsules
    g_Capsule1.m_Top.m_X    += (x - prev_x) * 0.01f;
    g_Capsule2.m_Top.m_X    -= (x - prev_x) * 0.01f;
    g_Capsule1.m_Bottom.m_X += (x - prev_x) * 0.01f;
    g_Capsule2.m_Bottom.m_X -= (x - prev_x) * 0.01f;

    g_Angle += fmodf((y - prev_y) * 0.01f, (float)M_PI * 2.0f);
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
