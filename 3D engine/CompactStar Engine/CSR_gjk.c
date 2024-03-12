/****************************************************************************
 * ==> Gilbert-Johnson-Keerthi (GJK) demo ----------------------------------*
 ****************************************************************************
 * Description : A simple 3rd person demo using the Gilbert-Johnson-Keerthi *
 *               (GJK) algorithm for collision detection                    *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2023, this file is part of the CompactStar Engine.  *
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
#include "SDK/CSR_GJK.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"
#include "SDK/CSR_Scene.h"

#include <ccr.h>

#define BACKGROUND_TEXTURE_FILE "Resources/chessboard.bmp"

//----------------------------------------------------------------------------
const char g_VSColored[] =
    "precision mediump float;"
    "attribute    vec3 csr_aVertices;"
    "attribute    vec4 csr_aColor;"
    "uniform      mat4 csr_uProjection;"
    "uniform      mat4 csr_uView;"
    "uniform      mat4 csr_uModel;"
    "varying lowp vec4 csr_vColor;"
    "void main(void)"
    "{"
    "    csr_vColor   = csr_aColor;"
    "    gl_Position  = csr_uProjection * csr_uView * csr_uModel * vec4(csr_aVertices, 1.0);"
    "}";
//----------------------------------------------------------------------------
const char g_FSColored[] =
    "precision mediump float;"
    "varying lowp vec4 csr_vColor;"
    "void main(void)"
    "{"
    "    gl_FragColor = csr_vColor;"
    "}";
//----------------------------------------------------------------------------
const char g_VSTextured[] =
    "precision mediump float;"
    "attribute vec3 csr_aVertices;"
    "attribute vec4 csr_aColor;"
    "attribute vec2 csr_aTexCoord;"
    "uniform   mat4 csr_uProjection;"
    "uniform   mat4 csr_uView;"
    "uniform   mat4 csr_uModel;"
    "varying   vec4 csr_vColor;"
    "varying   vec2 csr_vTexCoord;"
    "void main(void)"
    "{"
    "    csr_vColor    = csr_aColor;"
    "    csr_vTexCoord = csr_aTexCoord;"
    "    gl_Position   = csr_uProjection * csr_uView * csr_uModel * vec4(csr_aVertices, 1.0);"
    "}";
//----------------------------------------------------------------------------
const char g_FSTextured[] =
    "precision mediump float;"
    "uniform sampler2D csr_sColorMap;"
    "varying lowp vec4 csr_vColor;"
    "varying      vec2 csr_vTexCoord;"
    "void main(void)"
    "{"
    "    gl_FragColor = csr_vColor * texture2D(csr_sColorMap, csr_vTexCoord);"
    "}";
//------------------------------------------------------------------------------
CSR_Scene*        g_pScene          = 0;
CSR_SceneContext  g_SceneContext;
CSR_OpenGLShader* g_pColShader      = 0;
CSR_OpenGLShader* g_pTexShader      = 0;
CSR_Mesh*         g_pBackgroundMesh = 0;
CSR_ArcBall       g_Arcball;
CSR_Collider*     g_pPlayerCollider;
CSR_Collider*     g_pModel1Collider;
CSR_Collider*     g_pModel2Collider;
CSR_Collider*     g_pModel3Collider;
CSR_Collider*     g_pModel4Collider;
CSR_Matrix4       g_BackgroundMatrix;
CSR_Matrix4       g_PlayerMatrix;
CSR_Matrix4       g_Model1Matrix;
CSR_Matrix4       g_Model2Matrix;
CSR_Matrix4       g_Model3Matrix;
CSR_Matrix4       g_Model4Matrix;
CSR_Vector2       g_TouchOrigin;
CSR_Vector2       g_TouchPosition;
float             g_xPos            = 0.0f;
float             g_zPos            = 0.0f;
float             g_Velocity        = 0.025f;
CSR_OpenGLID      g_ID[1];
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    // textured model?
    if (pModel == g_pBackgroundMesh)
    {
        csrShaderEnable(g_pTexShader);
        return g_pTexShader;
    }

    csrShaderEnable(g_pColShader);
    return g_pColShader;
}
//---------------------------------------------------------------------------
void* OnGetID(const void* pKey)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < 1; ++i)
        // found the texture to get?
        if (pKey == g_ID[i].m_pKey)
            return &g_ID[i];

    return 0;
}
//---------------------------------------------------------------------------
void OnDeleteTexture(const CSR_Texture* pTexture)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < 1; ++i)
        // found the texture to delete?
        if (pTexture == g_ID[i].m_pKey)
        {
            // unuse the texture
            if (g_ID[i].m_UseCount)
                --g_ID[i].m_UseCount;

            // is texture no longer used?
            if (g_ID[i].m_UseCount)
                return;

            // delete the texture from the GPU
            if (g_ID[i].m_ID != M_CSR_Error_Code)
            {
                glDeleteTextures(1, (GLuint*)(&g_ID[i].m_ID));
                g_ID[i].m_ID = M_CSR_Error_Code;
            }

            return;
        }
}
//------------------------------------------------------------------------------
void UpdatePos(CSR_ArcBall* pArcball, float elapsedTime)
{
    // calculate the new direction from last mouse move
    pArcball->m_AngleY -= fmodf(-(g_TouchPosition.m_X - g_TouchOrigin.m_X) * 0.001f, (float)M_PI * 2.0f);

    // get the pressed key, if any, and convert it to the matching player state
    if (g_TouchPosition.m_Y < g_TouchOrigin.m_Y)
    {
        // move player forward
        g_xPos += g_Velocity * cosf(pArcball->m_AngleY + (float)(M_PI * 0.5)) * (elapsedTime * 0.05f);
        g_zPos -= g_Velocity * sinf(pArcball->m_AngleY + (float)(M_PI * 0.5)) * (elapsedTime * 0.05f);
    }

    pArcball->m_Position.m_X =  g_xPos;
    pArcball->m_Position.m_Y = -0.5f;
    pArcball->m_Position.m_Z =  2.0f - g_zPos;
}
//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 100.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    csrMat4Perspective(fov, aspect, zNear, zFar, &g_pScene->m_ProjectionMatrix);

    csrShaderEnable(g_pColShader);

    // connect projection matrix to color shader
    GLint projectionUniform = glGetUniformLocation(g_pColShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);

    csrShaderEnable(g_pTexShader);

    // connect projection matrix to texture shader
    projectionUniform = glGetUniformLocation(g_pTexShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);
}
//---------------------------------------------------------------------------
void BuildMatrix(const CSR_Vector3* pPos, float roll, float pitch, float yaw, float scale, CSR_Matrix4* pMatrix)
{
    CSR_Vector3 axis;

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    CSR_Matrix4 rotateXMatrix;

    // create the rotation matrix on x axis
    csrMat4Rotate(roll, &axis, &rotateXMatrix);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    CSR_Matrix4 rotateYMatrix;

    // create the rotation matrix on y axis
    csrMat4Rotate(pitch, &axis, &rotateYMatrix);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;

    CSR_Matrix4 rotateZMatrix;

    // create the rotation matrix on z axis
    csrMat4Rotate(yaw, &axis, &rotateZMatrix);

    CSR_Vector3 factor;

    // set scale factor
    factor.m_X = scale;
    factor.m_Y = scale;
    factor.m_Z = scale;

    CSR_Matrix4 scaleMatrix;

    // create the scale matrix
    csrMat4Scale(&factor, &scaleMatrix);

    CSR_Matrix4 firstIntermediateMatrix;
    CSR_Matrix4 secondIntermediateMatrix;

    // build the model matrix
    csrMat4Multiply(&scaleMatrix,              &rotateXMatrix, &firstIntermediateMatrix);
    csrMat4Multiply(&firstIntermediateMatrix,  &rotateYMatrix, &secondIntermediateMatrix);
    csrMat4Multiply(&secondIntermediateMatrix, &rotateZMatrix, pMatrix);

    // place it in the world
    pMatrix->m_Table[3][0] = pPos->m_X;
    pMatrix->m_Table[3][1] = pPos->m_Y;
    pMatrix->m_Table[3][2] = pPos->m_Z;
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_VertexFormat  vertexFormat;
    CSR_VertexCulling vertexCulling;
    CSR_Material      material;
    CSR_SceneItem*    pSceneItem   = 0;
    CSR_PixelBuffer*  pPixelBuffer = 0;

    // initialize the scene
    g_pScene = csrSceneCreate();

    // configure the scene background color
    g_pScene->m_Color.m_R = 0.25f;
    g_pScene->m_Color.m_G = 0.4f;
    g_pScene->m_Color.m_B = 0.5f;
    g_pScene->m_Color.m_A = 1.0f;

    // configure the scene view matrix
    csrMat4Identity(&g_pScene->m_ViewMatrix);

    // configure the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnGetShader     = OnGetShader;
    g_SceneContext.m_fOnGetID         = OnGetID;
    g_SceneContext.m_fOnDeleteTexture = OnDeleteTexture;

    // load the shader
    g_pColShader = csrOpenGLShaderLoadFromStr(&g_VSColored[0],
                                              sizeof(g_VSColored),
                                              &g_FSColored[0],
                                              sizeof(g_FSColored),
                                              0,
                                              0);

    // succeeded?
    if (!g_pColShader)
    {
        // show the error message to the user
        printf("Failed to load the color shader.\n");
        return;
    }

    // get shader attributes
    g_pColShader->m_VertexSlot = glGetAttribLocation(g_pColShader->m_ProgramID, "csr_aVertices");
    g_pColShader->m_ColorSlot  = glGetAttribLocation(g_pColShader->m_ProgramID, "csr_aColor");

    // load the shader
    g_pTexShader = csrOpenGLShaderLoadFromStr(&g_VSTextured[0],
                                              sizeof(g_VSTextured),
                                              &g_FSTextured[0],
                                              sizeof(g_FSTextured),
                                              0,
                                              0);

    // succeeded?
    if (!g_pTexShader)
    {
        // show the error message to the user
        printf("Failed to load the texture shader.\n");
        return;
    }

    // get shader attributes
    g_pTexShader->m_VertexSlot   = glGetAttribLocation (g_pTexShader->m_ProgramID, "csr_aVertices");
    g_pTexShader->m_ColorSlot    = glGetAttribLocation (g_pTexShader->m_ProgramID, "csr_aColor");
    g_pTexShader->m_TexCoordSlot = glGetAttribLocation (g_pTexShader->m_ProgramID, "csr_aTexCoord");
    g_pTexShader->m_TextureSlot  = glGetUniformLocation(g_pTexShader->m_ProgramID, "csr_sTexture");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    CSR_Vector3 pos;

    // set background position
    pos.m_X = 0.0f;
    pos.m_Y = 0.0f;
    pos.m_Z = 0.0f;

    // place the background
    csrMat4Identity(&g_BackgroundMatrix);
    BuildMatrix(&pos, (float)M_PI / 2.0f, 0.0f, 0.0f, 1.0f, &g_BackgroundMatrix);

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    vertexCulling.m_Face = CSR_CF_CCW;
    vertexCulling.m_Type = CSR_CT_Back;

    // configure the material
    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;
    material.m_Wireframe   = 0;

    // create the background
    g_pBackgroundMesh = csrShapeCreateSurface(20.0f, 20.0f, &vertexFormat, &vertexCulling, &material, 0);
    pSceneItem        = csrSceneAddMesh(g_pScene, g_pBackgroundMesh, 0, 0);
    csrSceneAddModelMatrix(g_pScene, g_pBackgroundMesh, &g_BackgroundMatrix);

    // create a resource for the background texture
    g_ID[0].m_pKey     = &g_pBackgroundMesh->m_Skin.m_Texture;
    g_ID[0].m_UseCount = 1;

    // load landscape texture
    pPixelBuffer = csrPixelBufferFromBitmapFile(BACKGROUND_TEXTURE_FILE);
    g_ID[0].m_ID = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);

    // landscape texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    CSR_Mesh* pMesh;

    // set vertex culling
    vertexCulling.m_Type = CSR_CT_Front;

    // set mesh format and material
    vertexFormat.m_HasTexCoords = 0;
    material.m_Color = 0x0000FFFF;

    CSR_Matrix4 matrix;

    // create the player collider
    g_pPlayerCollider               = csrColliderCreate();
    g_pPlayerCollider->m_State      = CSR_CS_Dynamic;
    g_pPlayerCollider->m_Pos.m_X    = 0.0f;
    g_pPlayerCollider->m_Pos.m_Y    = 0.0f;
    g_pPlayerCollider->m_Pos.m_Z    = 0.0f;
    g_pPlayerCollider->m_Radius     = 0.17f;
    g_pPlayerCollider->m_TopY       = 0.85f;
    g_pPlayerCollider->m_BottomY    = 0.0f;
    g_pPlayerCollider->m_fOnSupport = csrGJKSupportCapsule;
    csrMat4Identity(&matrix);
    csrColliderSetRS(&matrix, g_pPlayerCollider);

    // build the player matrix
    csrMat4Identity(&g_PlayerMatrix);

    // create the player capsule
    pMesh                       = csrShapeCreateCapsule(0.85f, 0.17f, 16.0f, &vertexFormat, &vertexCulling, &material, 0);
    pSceneItem                  = csrSceneAddMesh(g_pScene, pMesh, 0, 0);
    pSceneItem->m_CollisionType = CSR_CO_GJK;
    pSceneItem->m_pCollider     = g_pPlayerCollider;
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_PlayerMatrix);

    // create the first model collider
    g_pModel1Collider               =  csrColliderCreate();
    g_pModel1Collider->m_State      =  CSR_CS_Static;
    g_pModel1Collider->m_Pos.m_X    =  5.0f;
    g_pModel1Collider->m_Pos.m_Y    =  0.0f;
    g_pModel1Collider->m_Pos.m_Z    = -2.0f;
    g_pModel1Collider->m_Radius     =  0.17f;
    g_pModel1Collider->m_TopY       =  0.85f;
    g_pModel1Collider->m_BottomY    =  0.0f;
    g_pModel1Collider->m_fOnSupport =  csrGJKSupportCapsule;
    csrMat4Identity(&matrix);
    csrColliderSetRS(&matrix, g_pModel1Collider);

    // set mesh material
    material.m_Color = 0xFF00FFFF;

    // build the first model matrix
    csrMat4Identity(&g_Model1Matrix);
    g_Model1Matrix.m_Table[3][0] = g_pModel1Collider->m_Pos.m_X;
    g_Model1Matrix.m_Table[3][1] = g_pModel1Collider->m_Pos.m_Y;
    g_Model1Matrix.m_Table[3][2] = g_pModel1Collider->m_Pos.m_Z;

    // create the capsule
    pMesh                       = csrShapeCreateCapsule(0.85f, 0.17f, 16.0f, &vertexFormat, &vertexCulling, &material, 0);
    pSceneItem                  = csrSceneAddMesh(g_pScene, pMesh, 0, 0);
    pSceneItem->m_CollisionType = CSR_CO_GJK;
    pSceneItem->m_pCollider     = g_pModel1Collider;
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_Model1Matrix);

    // create the second model collider
    g_pModel2Collider               =  csrColliderCreate();
    g_pModel2Collider->m_State      =  CSR_CS_Static;
    g_pModel2Collider->m_Pos.m_X    = -5.0f;
    g_pModel2Collider->m_Pos.m_Y    =  0.0f;
    g_pModel2Collider->m_Pos.m_Z    =  3.5f;
    g_pModel2Collider->m_Min.m_X    = -0.4f;
    g_pModel2Collider->m_Min.m_Y    = -1.7f;
    g_pModel2Collider->m_Min.m_Z    = -1.3f;
    g_pModel2Collider->m_Max.m_X    =  0.4f;
    g_pModel2Collider->m_Max.m_Y    =  1.7f;
    g_pModel2Collider->m_Max.m_Z    =  1.3f;
    g_pModel2Collider->m_fOnSupport =  csrGJKSupportBox;
    csrMat4Identity(&matrix);
    BuildMatrix(&pos, 0.0f, (float)(M_PI * 0.25), (float)(M_PI * 0.15), 1.0f, &matrix);
    csrColliderSetRS(&matrix, g_pModel2Collider);

    // set vertex culling
    vertexCulling.m_Type = CSR_CT_Back;

    // set mesh material
    material.m_Color = 0x00FF00FF;

    // build the second model matrix
    csrMat4Identity(&matrix);
    matrix.m_Table[3][0] = g_pModel2Collider->m_Pos.m_X;
    matrix.m_Table[3][1] = g_pModel2Collider->m_Pos.m_Y;
    matrix.m_Table[3][2] = g_pModel2Collider->m_Pos.m_Z;
    csrMat4Multiply(&g_pModel2Collider->m_MatRS, &matrix, &g_Model2Matrix);

    // create the box
    pMesh                       = csrShapeCreateBox(0.8f, 3.4f, 2.6f, 0, &vertexFormat, &vertexCulling, &material, 0);
    pSceneItem                  = csrSceneAddMesh(g_pScene, pMesh, 0, 0);
    pSceneItem->m_CollisionType = CSR_CO_GJK;
    pSceneItem->m_pCollider     = g_pModel2Collider;
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_Model2Matrix);

    // create the third model collider
    g_pModel3Collider               =  csrColliderCreate();
    g_pModel3Collider->m_State      =  CSR_CS_Static;
    g_pModel3Collider->m_Pos.m_X    = -5.0f;
    g_pModel3Collider->m_Pos.m_Y    =  0.2f;
    g_pModel3Collider->m_Pos.m_Z    = -3.5f;
    g_pModel3Collider->m_Radius     =  1.2f;
    g_pModel3Collider->m_fOnSupport =  csrGJKSupportSphere;
    csrMat4Identity(&matrix);
    csrColliderSetRS(&matrix, g_pModel3Collider);

    // set mesh material
    material.m_Color = 0xFFFF00FF;

    // build the third model matrix
    csrMat4Identity(&g_Model3Matrix);
    g_Model3Matrix.m_Table[3][0] = g_pModel3Collider->m_Pos.m_X;
    g_Model3Matrix.m_Table[3][1] = g_pModel3Collider->m_Pos.m_Y;
    g_Model3Matrix.m_Table[3][2] = g_pModel3Collider->m_Pos.m_Z;

    // create the sphere
    pMesh                       = csrShapeCreateSphere(1.2f, 20, 20, &vertexFormat, &vertexCulling, &material, 0);
    pSceneItem                  = csrSceneAddMesh(g_pScene, pMesh, 0, 0);
    pSceneItem->m_CollisionType = CSR_CO_GJK;
    pSceneItem->m_pCollider     = g_pModel3Collider;
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_Model3Matrix);

    // create the fourth model collider
    g_pModel4Collider               = csrColliderCreate();
    g_pModel4Collider->m_State      = CSR_CS_Static;
    g_pModel4Collider->m_Pos.m_X    = 5.0f;
    g_pModel4Collider->m_Pos.m_Y    = 0.2f;
    g_pModel4Collider->m_Pos.m_Z    = 4.1f;
    g_pModel4Collider->m_Radius     = 2.1f;
    g_pModel4Collider->m_TopY       = 1.5f;
    g_pModel4Collider->m_BottomY    = 0.0f;
    g_pModel4Collider->m_fOnSupport = csrGJKSupportCylinder;
    csrMat4Identity(&matrix);
    csrColliderSetRS(&matrix, g_pModel4Collider);

    // set mesh material
    material.m_Color = 0x00FFFFFF;

    // build the fourth model matrix
    csrMat4Identity(&g_Model4Matrix);
    g_Model4Matrix.m_Table[3][0] = g_pModel4Collider->m_Pos.m_X;
    g_Model4Matrix.m_Table[3][1] = g_pModel4Collider->m_Pos.m_Y;
    g_Model4Matrix.m_Table[3][2] = g_pModel4Collider->m_Pos.m_Z;

    // create the cylinder
    pMesh                       = csrShapeCreateCylinder(2.1f, 2.1f, 1.5f, 20, &vertexFormat, &vertexCulling, &material, 0);
    pSceneItem                  = csrSceneAddMesh(g_pScene, pMesh, 0, 0);
    pSceneItem->m_CollisionType = CSR_CO_GJK;
    pSceneItem->m_pCollider     = g_pModel4Collider;
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_Model4Matrix);

    // initialize the arcball
    g_Arcball.m_Radius = 2.0f;
    g_Arcball.m_AngleX = 0.25f;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);

    // delete scene shaders
    csrOpenGLShaderRelease(g_pColShader);
    csrOpenGLShaderRelease(g_pTexShader);
    g_pColShader = 0;
    g_pTexShader = 0;
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
	float elapsedTime = (timeStep_sec * 1000.0f);

    // update the arcball position
    UpdatePos(&g_Arcball, elapsedTime);

    // update the collider position
    g_pPlayerCollider->m_Pos.m_X = -g_Arcball.m_Position.m_X;
    g_pPlayerCollider->m_Pos.m_Y =  0.0f;
    g_pPlayerCollider->m_Pos.m_Z = -g_Arcball.m_Position.m_Z;

    CSR_CollisionInput colInput;
    csrCollisionInputInit(&colInput);

    CSR_CollisionOutput colOutput;
    csrCollisionOutputInit(&colOutput);

    // detect the collisions in the scene
    csrSceneDetectCollision(g_pScene, &colInput, &colOutput, 0);

    // found a collision?
    if (colOutput.m_pColliders && colOutput.m_pColliders->m_Count)
    {
        // update the player position
        g_xPos -= colOutput.m_MinTransVec.m_X;
        g_zPos += colOutput.m_MinTransVec.m_Z;

        // update the arcball position
        g_Arcball.m_Position.m_X += colOutput.m_MinTransVec.m_X;
        g_Arcball.m_Position.m_Z -= colOutput.m_MinTransVec.m_Z;

        // update the collider position
        g_pPlayerCollider->m_Pos.m_X = -g_Arcball.m_Position.m_X;
        g_pPlayerCollider->m_Pos.m_Z = -g_Arcball.m_Position.m_Z;
    }

    // update the player matrix
    g_PlayerMatrix.m_Table[3][0] = g_pPlayerCollider->m_Pos.m_X;
    g_PlayerMatrix.m_Table[3][1] = g_pPlayerCollider->m_Pos.m_Y;
    g_PlayerMatrix.m_Table[3][2] = g_pPlayerCollider->m_Pos.m_Z;

    // get the view matrix matching with the camera
    csrSceneArcBallToMatrix(&g_Arcball, &g_pScene->m_ViewMatrix);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    // draw the scene
    csrSceneDraw(g_pScene, &g_SceneContext);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{
    // initialize the position
    g_TouchOrigin.m_X   = x;
    g_TouchOrigin.m_Y   = y;
    g_TouchPosition.m_X = x;
    g_TouchPosition.m_Y = y;
}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    // reset the position
    g_TouchOrigin.m_X   = 0;
    g_TouchOrigin.m_Y   = 0;
    g_TouchPosition.m_X = 0;
    g_TouchPosition.m_Y = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
    // get the next position
    g_TouchPosition.m_X = x;
    g_TouchPosition.m_Y = y;
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
