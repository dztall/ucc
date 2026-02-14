/****************************************************************************
 * ==> Nebulus tower -------------------------------------------------------*
 ****************************************************************************
 * Description : Nebulus tower demo                                         *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2026, this file is part of the CompactStar Engine.  *
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
#include "SDK/CSR_Iqm.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_AI.h"
#include "SDK/CSR_Sound.h"
#include "SDK/CSR_DebugHelper.h"

#include <ccr.h>

// resource files to load
#define SKYBOX_LEFT          "Resources/nebulus_skybox_left.bmp"
#define SKYBOX_TOP           "Resources/nebulus_skybox_top.bmp"
#define SKYBOX_RIGHT         "Resources/nebulus_skybox_right.bmp"
#define SKYBOX_BOTTOM        "Resources/nebulus_skybox_bottom.bmp"
#define SKYBOX_FRONT         "Resources/nebulus_skybox_front.bmp"
#define SKYBOX_BACK          "Resources/nebulus_skybox_back.bmp"
#define MDL_FILE             "Resources/Robot.mdl"
#define TOWER_TEXTURE_FILE   "Resources/NebulusTowerBlue.bmp"
#define WATER_SOUND_FILE     "Resources/water.wav"
#define FOOTSTEPS_SOUND_FILE "Resources/footsteps_metal.wav"

#define TEXTURE_COUNT 3

//----------------------------------------------------------------------------
const char* pCubemapFileNames[6] =
{
    SKYBOX_RIGHT,
    SKYBOX_LEFT,
    SKYBOX_TOP,
    SKYBOX_BOTTOM,
    SKYBOX_FRONT,
    SKYBOX_BACK
};
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
const char g_VSSkybox[] =
    "precision mediump float;"
    "attribute vec3 csr_aVertices;"
    "uniform   mat4 csr_uProjection;"
    "uniform   mat4 csr_uView;"
    "varying   vec3 csr_vTexCoord;"
    "void main()"
    "{"
    "    csr_vTexCoord = csr_aVertices;"
    "    gl_Position   = csr_uProjection * csr_uView * vec4(csr_aVertices, 1.0);"
    "}";
//------------------------------------------------------------------------------
const char g_FSSkybox[] =
    "precision mediump float;"
    "uniform samplerCube csr_sCubemap;"
    "varying vec3        csr_vTexCoord;"
    "void main()"
    "{"
    "    gl_FragColor = textureCube(csr_sCubemap, csr_vTexCoord);"
    "}";
//------------------------------------------------------------------------------
CSR_Scene*                   g_pScene          = 0;
CSR_OpenGLShader*            g_pSkyboxShader   = 0;
CSR_OpenGLShader*            g_pTextureShader  = 0;
CSR_OpenGLShader*            g_pColorShader    = 0;
CSR_SceneContext             g_SceneContext;
CSR_ArcBall                  g_Arcball;
ALCdevice*                   g_pOpenALDevice   = 0;
ALCcontext*                  g_pOpenALContext  = 0;
CSR_IQM*                     g_pPlayerModel    = 0;
CSR_Mesh*                    g_pTowerMesh      = 0;
CSR_Collider*                g_pPlayerCollider = 0;
CSR_Sound*                   g_pFootstepsSound = 0;
CSR_Matrix4                  g_PlayerMatrix;
CSR_Matrix4                  g_PlayerColliderMatrix;
CSR_Matrix4                  g_TowerMatrix;
size_t                       g_FPS             = 120;
size_t                       g_AnimIndex       = 0;
size_t                       g_TextureIndex    = 0;
size_t                       g_ModelIndex      = 0;
size_t                       g_MeshIndex       = 0;
const size_t                 g_PlatformCount   = 22;
float                        g_xPos            = 0.0f;
float                        g_yPos            = 0.0f;
float                        g_zPos            = 0.0f;
float                        g_Distance        = 1.4f;
float                        g_PlayerShift     = 0.25f;
float                        g_WalkOffset      = 1.0f;
float                        g_Angle           = 0.0f;
float                        g_Velocity        = 0.75f;
float                        g_Gravity         = 0.5f;
float                        g_WipeOffset      = 0.0f;
double                       g_ModelWalkTime   = 0.0;
double                       g_TextureLastTime = 0.0;
double                       g_ModelLastTime   = 0.0;
double                       g_MeshLastTime    = 0.0;
int                          g_Walking         = 0;
CSR_Matrix4                  g_PlatformMatrix      [g_PlatformCount];
CSR_Matrix4                  g_PlatformMatrixTop   [g_PlatformCount];
CSR_Matrix4                  g_PlatformMatrixBottom[g_PlatformCount];
CSR_Collider*                g_PlatformCollider    [g_PlatformCount];
CSR_Mesh*                    g_pPlatformBodyMesh   [g_PlatformCount];
CSR_Mesh*                    g_pPlatformTopMesh    [g_PlatformCount];
CSR_Mesh*                    g_pPlatformBottomMesh [g_PlatformCount];
CSR_OpenGLID                 g_ID[TEXTURE_COUNT];
//---------------------------------------------------------------------------
void OnApplySkin(size_t index, const CSR_Skin* pSkin, int* pCanRelease)
{
    // should not be hardcoded, however there is only 1 model which will use this function in this demo,
    // so it is safe to do that
    g_ID[0].m_pKey     = (void*)(&pSkin->m_Texture);
    g_ID[0].m_ID       = csrOpenGLTextureFromPixelBuffer(pSkin->m_Texture.m_pBuffer);
    g_ID[0].m_UseCount = 1;

    // from now the source texture will no longer be used
    if (pCanRelease)
        *pCanRelease = 1;
}
//---------------------------------------------------------------------------
void* OnGetID(const void* pKey)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < TEXTURE_COUNT; ++i)
        // found the texture to get?
        if (pKey == g_ID[i].m_pKey)
            return &g_ID[i];

    return 0;
}
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    size_t i;

    if (pModel == g_pPlayerModel || pModel == g_pTowerMesh)
        return g_pTextureShader;
    else
    if (pModel == g_pScene->m_pSkybox)
        return g_pSkyboxShader;

    for (i = 0; i < g_PlatformCount; ++i)
        if (pModel == g_pPlatformBodyMesh[i] || pModel == g_pPlatformTopMesh[i] || pModel == g_pPlatformBottomMesh[i] )
            return g_pColorShader;

    return g_pTextureShader;
}
//---------------------------------------------------------------------------
void OnGetMDLIndex(const CSR_MDL* pMDL,
                         size_t*  pSkinIndex,
                         size_t*  pModelIndex,
                         size_t*  pMeshIndex)
{
    *pSkinIndex  = g_TextureIndex;
    *pModelIndex = g_ModelIndex;
    *pMeshIndex  = g_MeshIndex;
}
//---------------------------------------------------------------------------
void OnDeleteTexture(const CSR_Texture* pTexture)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < TEXTURE_COUNT; ++i)
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
//---------------------------------------------------------------------------
void BuildModelMatrix(CSR_Matrix4* pMatrix, int isCollider)
{
    CSR_Vector3 axis   = {};
    CSR_Vector3 factor = {};
    CSR_Matrix4 rotateXMatrix;
    CSR_Matrix4 rotateYMatrix;
    CSR_Matrix4 scaleMatrix;
    CSR_Matrix4 intermediateMatrix;

    csrMat4Identity(pMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // create the rotation matrix
    csrMat4Rotate(-((float)M_PI / 2.0), &axis, &rotateXMatrix);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // create the rotation matrix
    csrMat4Rotate(-g_Angle - ((float)M_PI / 2.0f) * g_WalkOffset, &axis, &rotateYMatrix);

    // set scale factor
    factor.m_X = isCollider ? 1.0f : 0.1f;
    factor.m_Y = isCollider ? 1.0f : 0.1f;
    factor.m_Z = isCollider ? 1.0f : 0.1f;

    // create the scale matrix
    csrMat4Scale(&factor, &scaleMatrix);

    // build the model matrix
    csrMat4Multiply(&scaleMatrix,        &rotateXMatrix, &intermediateMatrix);
    csrMat4Multiply(&intermediateMatrix, &rotateYMatrix, pMatrix);

    // place it in the world
    pMatrix->m_Table[3][0] = g_xPos - (g_Distance * sinf(g_Angle));
    pMatrix->m_Table[3][1] = g_yPos -  g_PlayerShift;
    pMatrix->m_Table[3][2] = g_zPos + (g_Distance * cosf(g_Angle));
}
//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.1f;
    const float zFar   = 100.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    csrMat4Perspective(fov, aspect, zNear, zFar, &g_pScene->m_ProjectionMatrix);
}
//---------------------------------------------------------------------------
void OnSceneBegin(const CSR_Scene* pScene, const CSR_SceneContext* pContext)
{
    csrDrawBegin(&pScene->m_Color);
}
//---------------------------------------------------------------------------
void OnSceneEnd(const CSR_Scene* pScene, const CSR_SceneContext* pContext)
{
    csrDrawEnd();
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_VertexFormat  vertexFormat;
    CSR_VertexCulling vertexCulling;
    CSR_Material      material;
    CSR_PixelBuffer*  pPixelBuffer = 0;
    float             step;
    float             heightStep;
    size_t            i;
    GLint             uniformSlot;

    // initialize OpenAL
    csrSoundInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // initialize the scene
    g_pScene = csrSceneCreate();

    // configure the scene color background
    g_pScene->m_Color.m_R = 0.08f;
    g_pScene->m_Color.m_G = 0.12f;
    g_pScene->m_Color.m_B = 0.17f;
    g_pScene->m_Color.m_A = 1.0f;

    // configure the scene view matrix
    csrMat4Identity(&g_pScene->m_ViewMatrix);

    // configure the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnGetShader     = OnGetShader;
    g_SceneContext.m_fOnGetID         = OnGetID;
    g_SceneContext.m_fOnGetMDLIndex   = OnGetMDLIndex;
    g_SceneContext.m_fOnDeleteTexture = OnDeleteTexture;
    g_SceneContext.m_fOnSceneBegin    = OnSceneBegin;
    g_SceneContext.m_fOnSceneEnd      = OnSceneEnd;

    // initialize the arcball
    g_Arcball.m_Position.m_X = g_xPos;
    g_Arcball.m_Position.m_Y = g_yPos;
    g_Arcball.m_Position.m_Z = g_zPos;
    g_Arcball.m_AngleX       = 0.0f;
    g_Arcball.m_AngleY       = g_Angle;
    g_Arcball.m_Radius       = 4.0f;

    // load the skybox shader
    g_pSkyboxShader = csrOpenGLShaderLoadFromStr(&g_VSSkybox[0],
                                                  sizeof(g_VSSkybox),
                                                 &g_FSSkybox[0],
                                                  sizeof(g_FSSkybox),
                                                  0,
                                                  0);

    // succeeded?
    if (!g_pSkyboxShader)
    {
        // show the error message to the user
        printf("Failed to load the skybox shader.\n");
        return;
    }

    // enable the shader program
    csrShaderEnable(g_pSkyboxShader);

    // connect projection matrix to shader
    uniformSlot = glGetUniformLocation(g_pSkyboxShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(uniformSlot, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);

    // connect view matrix to shader
    uniformSlot = glGetUniformLocation(g_pSkyboxShader->m_ProgramID, "csr_uView");
    glUniformMatrix4fv(uniformSlot, 1, 0, &g_pScene->m_ViewMatrix.m_Table[0][0]);

    // get shader attributes
    g_pSkyboxShader->m_VertexSlot  = glGetAttribLocation (g_pSkyboxShader->m_ProgramID, "csr_aVertices");
    g_pSkyboxShader->m_CubemapSlot = glGetUniformLocation(g_pSkyboxShader->m_ProgramID, "csr_sCubemap");

    // create the skybox
    g_pScene->m_pSkybox = csrSkyboxCreate(1.0f, 1.0f, 1.0f);

    // succeeded?
    if (!g_pScene->m_pSkybox)
    {
        // show the error message to the user
        printf("Failed to create the skybox.\n");
        return;
    }

    // load the cubemap texture
    g_ID[1].m_pKey     = &g_pScene->m_pSkybox->m_Skin.m_CubeMap;
    g_ID[1].m_ID       = csrOpenGLCubemapLoad(pCubemapFileNames);
    g_ID[1].m_UseCount = 1;

    // load the texture shader
    g_pTextureShader = csrOpenGLShaderLoadFromStr(&g_VSTextured[0],
                                                  sizeof(g_VSTextured),
                                                 &g_FSTextured[0],
                                                  sizeof(g_FSTextured),
                                                  0,
                                                  0);

    // succeeded?
    if (!g_pTextureShader)
    {
        // show the error message to the user
        printf("Failed to load the texture shader.\n");
        return;
    }

    // enable the shader program
    csrShaderEnable(g_pTextureShader);

    // connect projection matrix to shader
    uniformSlot = glGetUniformLocation(g_pTextureShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(uniformSlot, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);

    // connect view matrix to shader
    uniformSlot = glGetUniformLocation(g_pTextureShader->m_ProgramID, "csr_uView");
    glUniformMatrix4fv(uniformSlot, 1, 0, &g_pScene->m_ViewMatrix.m_Table[0][0]);

    // get shader attributes
    g_pTextureShader->m_VertexSlot   = glGetAttribLocation (g_pTextureShader->m_ProgramID, "csr_aVertices");
    g_pTextureShader->m_ColorSlot    = glGetAttribLocation (g_pTextureShader->m_ProgramID, "csr_aColor");
    g_pTextureShader->m_TexCoordSlot = glGetAttribLocation (g_pTextureShader->m_ProgramID, "csr_aTexCoord");
    g_pTextureShader->m_TextureSlot  = glGetUniformLocation(g_pTextureShader->m_ProgramID, "csr_sTexture");

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    // set vertex culling
    vertexCulling.m_Face = CSR_CF_CW;
    vertexCulling.m_Type = CSR_CT_Back;

    // configure the material
    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;
    material.m_Wireframe   = 0;
    material.m_uScale      = 1.0f;
    material.m_vScale      = 1.0f;

    // load the IQM model
    g_pPlayerModel = csrMDLOpen(MDL_FILE, 0, &vertexFormat, &vertexCulling, &material, 0, OnApplySkin, OnDeleteTexture);

    // build the player model matrix
    BuildModelMatrix(&g_PlayerMatrix,         0);
    BuildModelMatrix(&g_PlayerColliderMatrix, 1);

    // create the player collider
    g_pPlayerCollider               = csrColliderCreate();
    g_pPlayerCollider->m_State      = CSR_CS_Dynamic;
    g_pPlayerCollider->m_Pos.m_X    = g_xPos;
    g_pPlayerCollider->m_Pos.m_Y    = g_yPos;
    g_pPlayerCollider->m_Pos.m_Z    = g_zPos;
    g_pPlayerCollider->m_Radius     = 0.15f;
    g_pPlayerCollider->m_TopY       = 0.15f;
    g_pPlayerCollider->m_BottomY    = 0.0f;
    g_pPlayerCollider->m_fOnSupport = csrGJKSupportCapsule;
    csrColliderSetRS(&g_PlayerColliderMatrix, g_pPlayerCollider);

    // add the model to the scene
    CSR_SceneItem* pSceneItem   = csrSceneAddMDL(g_pScene, g_pPlayerModel, 0, 0);
    pSceneItem->m_CollisionType = CSR_CO_GJK;
    pSceneItem->m_pCollider     = g_pPlayerCollider;
    csrSceneAddModelMatrix(g_pScene, g_pPlayerModel, &g_PlayerMatrix);

    // load the player footsteps sound file
    g_pFootstepsSound = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, FOOTSTEPS_SOUND_FILE);

    // loop the sound
    csrSoundLoop(g_pFootstepsSound, 1);

    // change the volume
    csrSoundChangeVolume(g_pFootstepsSound, 1.0f);

    // stop the sound by default
    csrSoundStop(g_pFootstepsSound);

    // set vertex format for textured models
    vertexFormat.m_Type              = CSR_VT_Triangles;
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    // set vertex culling
    vertexCulling.m_Face = CSR_CF_CCW;

    // create material
    material.m_Color  = 0xFFFFFFFF;
    material.m_uScale = 10.0f;
    material.m_vScale = 10.0f;

    // create the tower
    g_pTowerMesh = csrShapeCreateCylinder(1.2f, 1.2f, 10.0f, 50, &vertexFormat, &vertexCulling, &material, 0);

    // load tower texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(TOWER_TEXTURE_FILE);
    g_ID[2].m_pKey     = &g_pTowerMesh->m_Skin.m_Texture;
    g_ID[2].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[2].m_UseCount = 1;

    // set texture wrapping mode
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // ball texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // configure the tower matrix
    csrMat4Identity(&g_TowerMatrix);

    // add the tower item to the scene
    csrSceneAddMesh(g_pScene, g_pTowerMesh, 0, 0);
    csrSceneAddModelMatrix(g_pScene, g_pTowerMesh, &g_TowerMatrix);

    // load the color shader
    g_pColorShader = csrOpenGLShaderLoadFromStr(&g_VSColored[0],
                                                 sizeof(g_VSColored),
                                                &g_FSColored[0],
                                                 sizeof(g_FSColored),
                                                 0,
                                                 0);

    // succeeded?
    if (!g_pColorShader)
    {
        // show the error message to the user
        printf("Failed to load the color shader.\n");
        return;
    }

    // enable the shader program
    csrShaderEnable(g_pColorShader);

    // connect projection matrix to shader
    uniformSlot = glGetUniformLocation(g_pColorShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(uniformSlot, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);

    // connect view matrix to shader
    uniformSlot = glGetUniformLocation(g_pColorShader->m_ProgramID, "csr_uView");
    glUniformMatrix4fv(uniformSlot, 1, 0, &g_pScene->m_ViewMatrix.m_Table[0][0]);

    // get shader attributes
    g_pColorShader->m_VertexSlot = glGetAttribLocation(g_pColorShader->m_ProgramID, "csr_aVertices");
    g_pColorShader->m_ColorSlot  = glGetAttribLocation(g_pColorShader->m_ProgramID, "csr_aColor");

    // set vertex format for colored models
    vertexFormat.m_Type              = CSR_VT_Triangles;
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 0;
    vertexFormat.m_HasPerVertexColor = 1;

    step       = ((float)M_PI * 2.0f) / (float)g_PlatformCount;
    heightStep = 0.05f;

    // create first platform row
    for (i = 0; i < g_PlatformCount; ++i)
    {
        CSR_Vector3 axis;

        // calculate the platform position
        csrMat4Identity(&g_PlatformMatrix[i]);
        g_PlatformMatrix[i].m_Table[3][0] =  g_Distance * sinf(i * step);
        g_PlatformMatrix[i].m_Table[3][1] = -0.44f + (i >= (g_PlatformCount >> 1) ? (g_PlatformCount - i) * heightStep : i * heightStep);
        g_PlatformMatrix[i].m_Table[3][2] =  g_Distance * cosf(i * step);

        // create material
        material.m_Color = 0x69615eff;

        // create the tower platform model
        g_pPlatformBodyMesh[i] = csrShapeCreateCylinder(0.2f, 0.2f, 0.1f, 15, &vertexFormat, &vertexCulling, &material, 0);

        // create the platform collider
        g_PlatformCollider[i]               =  csrColliderCreate();
        g_PlatformCollider[i]->m_State      =  CSR_CS_Static;
        g_PlatformCollider[i]->m_Pos.m_X    =  g_PlatformMatrix[i].m_Table[3][0];
        g_PlatformCollider[i]->m_Pos.m_Y    =  g_PlatformMatrix[i].m_Table[3][1];
        g_PlatformCollider[i]->m_Pos.m_Z    =  g_PlatformMatrix[i].m_Table[3][2];
        g_PlatformCollider[i]->m_Radius     =  0.2f;
        g_PlatformCollider[i]->m_TopY       =  0.05f;
        g_PlatformCollider[i]->m_BottomY    = -0.05f;
        g_PlatformCollider[i]->m_fOnSupport =  csrGJKSupportCylinder;
        csrColliderSetRS(&g_PlatformMatrix[i], g_PlatformCollider[i]);

        // add the platform body item to the scene
        CSR_SceneItem* pSceneItem   = csrSceneAddMesh(g_pScene, g_pPlatformBodyMesh[i], 0, 0);
        pSceneItem->m_CollisionType = CSR_CO_GJK;
        pSceneItem->m_pCollider     = g_PlatformCollider[i];
        csrSceneAddModelMatrix(g_pScene, g_pPlatformBodyMesh[i], &g_PlatformMatrix[i]);

        // set rotation axis
        axis.m_X = 1.0f;
        axis.m_Y = 0.0f;
        axis.m_Z = 0.0f;

        // initialize the platform top matrix
        csrMat4Rotate(-((float)M_PI / 2.0), &axis, &g_PlatformMatrixTop[i]);

        // place the platform top closure in the world
        g_PlatformMatrixTop[i].m_Table[3][0] = g_PlatformMatrix[i].m_Table[3][0];
        g_PlatformMatrixTop[i].m_Table[3][1] = g_PlatformMatrix[i].m_Table[3][1] + 0.05f;
        g_PlatformMatrixTop[i].m_Table[3][2] = g_PlatformMatrix[i].m_Table[3][2];

        // create material
        material.m_Color = 0xcfc7c4ff;

        // create the tower platform model
        g_pPlatformTopMesh[i] = csrShapeCreateDisk(0.0f, 0.0f, 0.2f, 15, &vertexFormat, &vertexCulling, &material, 0);

        // add the platform closure item to the scene
        csrSceneAddMesh(g_pScene, g_pPlatformTopMesh[i], 0, 0);
        csrSceneAddModelMatrix(g_pScene, g_pPlatformTopMesh[i], &g_PlatformMatrixTop[i]);

        // initialize the platform bottom matrix
        csrMat4Rotate(((float)M_PI / 2.0), &axis, &g_PlatformMatrixBottom[i]);

        // place the platform bottom closure in the world
        g_PlatformMatrixBottom[i].m_Table[3][0] = g_PlatformMatrix[i].m_Table[3][0];
        g_PlatformMatrixBottom[i].m_Table[3][1] = g_PlatformMatrix[i].m_Table[3][1] - 0.05f;
        g_PlatformMatrixBottom[i].m_Table[3][2] = g_PlatformMatrix[i].m_Table[3][2];

        // create the tower platform model
        g_pPlatformBottomMesh[i] = csrShapeCreateDisk(0.0f, 0.0f, 0.2f, 15, &vertexFormat, &vertexCulling, &material, 0);

        // add the platform closure item to the scene
        csrSceneAddMesh(g_pScene, g_pPlatformBottomMesh[i], 0, 0);
        csrSceneAddModelMatrix(g_pScene, g_pPlatformBottomMesh[i], &g_PlatformMatrixBottom[i]);
    }
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);
    g_pScene = 0;

    // delete color shader
    csrOpenGLShaderRelease(g_pColorShader);
    g_pColorShader = 0;

    // delete texture shader
    csrOpenGLShaderRelease(g_pTextureShader);
    g_pTextureShader = 0;

    // delete skybox shader
    csrOpenGLShaderRelease(g_pSkyboxShader);
    g_pSkyboxShader = 0;

    // stop running sound, if needed
    csrSoundStop(g_pFootstepsSound);

    // release OpenAL interface
    csrSoundRelease(g_pFootstepsSound);

    // release OpenAL
    csrSoundReleaseOpenAL(g_pOpenALDevice, g_pOpenALContext);
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
    CSR_CollisionInput   colInput;
    CSR_CollisionOutput* pColOutput;
    float                camX;
    float                camY;
    float                camZ;

    // angle changed?
    if (g_WipeOffset > 0.0f)
    {
        g_Walking    = 1;
        g_WalkOffset = 1.0f;
    }
    else
    if (g_WipeOffset < 0.0f)
    {
        g_Walking    =  1;
        g_WalkOffset = -1.0f;
    }
    else
        g_Walking = 0;

    // apply the gravity to the player
    g_yPos -= g_Gravity * timeStep_sec;

    // is player walking?
    if (g_Walking)
    {
        // calculate the next position
        g_Angle += g_Velocity * (float)g_WalkOffset * timeStep_sec;

        // calculate next model indexes to show
        csrMDLUpdateIndex(g_pPlayerModel,
                          g_FPS,
                          g_AnimIndex,
                         &g_TextureIndex,
                         &g_ModelIndex,
                         &g_MeshIndex,
                         &g_TextureLastTime,
                         &g_ModelLastTime,
                         &g_MeshLastTime,
                          timeStep_sec);

        // play the walk sound, if not playing
        if (!csrSoundIsPlaying(g_pFootstepsSound))
            csrSoundPlay(g_pFootstepsSound);
    }
    else
    if (csrSoundIsPlaying(g_pFootstepsSound))
        // stop the walk sound
        csrSoundStop(g_pFootstepsSound);

    // rebuild the model matrix
    BuildModelMatrix(&g_PlayerMatrix,         0);
    BuildModelMatrix(&g_PlayerColliderMatrix, 1);

    // update player collider position
    g_pPlayerCollider->m_Pos.m_X = g_xPos - (g_Distance * sinf(g_Angle));
    g_pPlayerCollider->m_Pos.m_Y = g_yPos -  g_PlayerShift;
    g_pPlayerCollider->m_Pos.m_Z = g_zPos + (g_Distance * cosf(g_Angle));

    csrCollisionInputInit(&colInput);
    pColOutput = csrCollisionOutputCreate();

    // detect the collisions in the scene
    csrSceneDetectCollision(g_pScene, &colInput, pColOutput, 0);

    // found a collision?
    if (pColOutput->m_pColliders && pColOutput->m_pColliders->m_Count)
    {
        // update the player position
        g_yPos += pColOutput->m_MinTransVec.m_Y;

        // update player position
        g_PlayerMatrix.m_Table[3][0] = g_xPos - (g_Distance * sinf(g_Angle));
        g_PlayerMatrix.m_Table[3][1] = g_yPos -  g_PlayerShift;
        g_PlayerMatrix.m_Table[3][2] = g_zPos + (g_Distance * cosf(g_Angle));

        // update player collider position
        g_pPlayerCollider->m_Pos.m_X = g_xPos - (g_Distance * sinf(g_Angle));
        g_pPlayerCollider->m_Pos.m_Y = g_yPos -  g_PlayerShift;
        g_pPlayerCollider->m_Pos.m_Z = g_zPos + (g_Distance * cosf(g_Angle));
    }

    // delete the output
    csrCollisionOutputRelease(pColOutput);

    // update the arcball
    g_Arcball.m_Position.m_X =  g_xPos;
    g_Arcball.m_Position.m_Y = -g_yPos;
    g_Arcball.m_Position.m_Z =  g_zPos;
    g_Arcball.m_AngleY       =  g_Angle;

    // get the view matrix matching with the arcball
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
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    g_WipeOffset = 0.0f;
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
    g_WipeOffset = (x - prev_x);
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
