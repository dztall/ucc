/****************************************************************************
 * ==> Bot demo ------------------------------------------------------------*
 ****************************************************************************
 * Description : A small demo about artificial intelligence in a game       *
 *               context. The bot will search the player, and he will       *
 *               attack if he sees it. If the screen become gray, then the  *
 *               player is under attack. On the other hand, the player will *
 *               win if he can hit the bot without being seen.              *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2022, this file is part of the CompactStar Engine.  *
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
#include "SDK/CSR_Mdl.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_AI.h"
#include "SDK/CSR_Sound.h"

#include <ccr.h>

#define LANDSCAPE_TEXTURE_FILE     "Resources/grass.bmp"
#define LANDSCAPE_DATA_FILE        "Resources/bot_level.bmp"
#define FADER_TEXTURE_FILE         "Resources/blank.bmp"
#define MDL_FILE                   "Resources/wizard.mdl"
#define SKYBOX_LEFT                "Resources/skybox_left_small.bmp"
#define SKYBOX_TOP                 "Resources/skybox_top_small.bmp"
#define SKYBOX_RIGHT               "Resources/skybox_right_small.bmp"
#define SKYBOX_BOTTOM              "Resources/skybox_bottom_small.bmp"
#define SKYBOX_FRONT               "Resources/skybox_front_small.bmp"
#define SKYBOX_BACK                "Resources/skybox_back_small.bmp"
#define FOOT_STEP_LEFT_SOUND_FILE  "Resources/footstep_left.wav"
#define FOOT_STEP_RIGHT_SOUND_FILE "Resources/footstep_right.wav"
#define HIT_SOUND_FILE             "Resources/hit.wav"

//----------------------------------------------------------------------------
#define M_Texture_Count 4
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
const char g_VSTextured[] =
    "precision mediump float;"
    "attribute vec3  csr_aVertices;"
    "attribute vec4  csr_aColor;"
    "attribute vec2  csr_aTexCoord;"
    "uniform   float csr_uAlpha;"
    "uniform   int   csr_uGrayscale;"
    "uniform   mat4  csr_uProjection;"
    "uniform   mat4  csr_uView;"
    "uniform   mat4  csr_uModel;"
    "varying   vec4  csr_vColor;"
    "varying   vec2  csr_vTexCoord;"
    "varying   float csr_fGrayscale;"
    "varying   float csr_fAlpha;"
    "void main(void)"
    "{"
    "    csr_vColor    = csr_aColor;"
    "    csr_vTexCoord = csr_aTexCoord;"
    "    gl_Position   = csr_uProjection * csr_uView * csr_uModel * vec4(csr_aVertices, 1.0);"
    ""
    "    if (csr_uGrayscale == 1)"
    "        csr_fGrayscale = 1.0;"
    "    else"
    "        csr_fGrayscale = 0.0;"
    ""
    "    csr_fAlpha = csr_uAlpha;"
    "}";
//----------------------------------------------------------------------------
const char g_FSTextured[] =
    "precision mediump float;"
    "uniform sampler2D  csr_sColorMap;"
    "varying lowp vec4  csr_vColor;"
    "varying      vec2  csr_vTexCoord;"
    "varying      float csr_fGrayscale;"
    "varying      float csr_fAlpha;"
    "void main(void)"
    "{"
    "    float csr_fFadeFactor = 1.0;"
    "    vec4 color            = csr_vColor * texture2D(csr_sColorMap, csr_vTexCoord);"
    ""
    "    if (csr_fGrayscale > 0.5)"
    "    {"
    "        float grayscaleVal = (color.x * 0.3 + color.y * 0.59 + color.z * 0.11);"
    "        gl_FragColor       = vec4(grayscaleVal, grayscaleVal, grayscaleVal, csr_fAlpha);"
    "    }"
    "    else"
    "        gl_FragColor = vec4(color.x * csr_fFadeFactor, color.y * csr_fFadeFactor, color.z * csr_fFadeFactor, csr_fAlpha);"
    "}";
//------------------------------------------------------------------------------
const char g_VSSkybox[] =
    "precision mediump float;"
    "attribute vec3  csr_aVertices;"
    "uniform   mat4  csr_uProjection;"
    "uniform   mat4  csr_uView;"
    "varying   vec3  csr_vTexCoord;"
    "varying   float csr_fRedFilter;"
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
/**
* Artificial intelligence tasks enumeration
*/
typedef enum
{
    E_BT_Watching,
    E_BT_Searching,
    E_BT_Attacking,
    E_BT_Dying
}
CSR_EBotTask;
//------------------------------------------------------------------------------
typedef enum
{
    E_DS_None,
    E_DS_Dying,
    E_DS_FadeOut,
    E_DS_FadeIn
}
CSR_EDyingSequence;
//------------------------------------------------------------------------------
/**
* Bot
*/
typedef struct
{
    CSR_MDL*           m_pModel;
    CSR_Sphere         m_Geometry;
    CSR_Vector2        m_StartPosition;
    CSR_Vector2        m_EndPosition;
    CSR_Vector3        m_Dir;
    CSR_Matrix4        m_Matrix;
    CSR_EDyingSequence m_DyingSequence;
    float              m_Angle;
    float              m_Velocity;
    float              m_MovePos;
} CSR_Bot;
//------------------------------------------------------------------------------
/**
* Terrain limits
*/
typedef struct
{
    CSR_Vector2 m_Min;
    CSR_Vector2 m_Max;
} CSR_TerrainLimits;
//------------------------------------------------------------------------------
CSR_Scene*        g_pScene              = 0;
CSR_TaskManager*  g_pTaskManager        = 0;
CSR_OpenGLShader* g_pShader             = 0;
CSR_OpenGLShader* g_pSkyboxShader       = 0;
CSR_Mesh*         g_pFader              = 0;
void*             g_pLandscapeKey       = 0;
float             g_MapHeight           = 3.0f;
float             g_MapScale            = 0.2f;
float             g_Angle               = 0.0f;
float             g_RotationSpeed       = 0.02f;
float             g_StepTime            = 0.0f;
float             g_StepInterval        = 300.0f;
const float       g_PosVelocity         = 10.0f;
const float       g_DirVelocity         = 30.0f;
const float       g_ControlRadius       = 40.0f;
float             g_BotAlpha            = 1.0f;
float             g_FaderAlpha          = 0.0f;
float             g_Time                = 0.0f;
float             g_Interval            = 0.0f;
double            g_TextureLastTime     = 0.0;
double            g_ModelLastTime       = 0.0;
double            g_MeshLastTime        = 0.0;
double            g_WatchingTime        = 0.0;
double            g_ElapsedTime         = 0.0;
const unsigned    g_FPS                 = 10;
size_t            g_AnimIndex           = 0;
size_t            g_TextureIndex        = 0;
size_t            g_ModelIndex          = 0;
size_t            g_MeshIndex           = 0;
size_t            g_LastKnownIndex      = 0;
int               g_AlternateStep       = 0;
int               g_BotShowPlayer       = 0;
int               g_BotHitPlayer        = 0;
int               g_BotDying            = 0;
int               g_PlayerDying         = 0;
CSR_SceneContext  g_SceneContext;
CSR_TaskContext   g_TaskContext;
CSR_Bot           g_Bot;
CSR_Sphere        g_BoundingSphere;
CSR_Matrix4       g_LandscapeMatrix;
CSR_Matrix4       g_FaderMatrix;
CSR_Vector2       g_TouchOrigin;
CSR_Vector2       g_TouchPosition;
CSR_Color         g_Color;
CSR_TerrainLimits g_TerrainLimits;
ALCdevice*        g_pOpenALDevice       = 0;
ALCcontext*       g_pOpenALContext      = 0;
GLuint            g_GrayscaleSlot       = 0;
GLuint            g_AlphaSlot           = 0;
CSR_Sound*        g_pFootStepLeftSound  = 0;
CSR_Sound*        g_pFootStepRightSound = 0;
CSR_Sound*        g_pHitSound           = 0;
CSR_OpenGLID      g_ID[M_Texture_Count];
//---------------------------------------------------------------------------
void OnApplySkin(size_t index, const CSR_Skin* pSkin, int* pCanRelease)
{
    // should not be hardcoded, however there is only 1 model which will use this function in this demo,
    // so it is safe to do that
    g_ID[1].m_pKey     = (void*)(&pSkin->m_Texture);
    g_ID[1].m_ID       = csrOpenGLTextureFromPixelBuffer(pSkin->m_Texture.m_pBuffer);
    g_ID[1].m_UseCount = 1;

    // from now the source texture will no longer be used
    if (pCanRelease)
        *pCanRelease = 1;
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
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    if (pModel == g_pScene->m_pSkybox)
        return g_pSkyboxShader;

    csrShaderEnable(g_pShader);

    // bot or fader?
    if (pModel == g_Bot.m_pModel)
        glUniform1f(g_AlphaSlot, g_BotAlpha);
    else
    if (pModel == g_pFader)
        glUniform1f(g_AlphaSlot, g_FaderAlpha);
    else
        glUniform1f(g_AlphaSlot, 1.0f);

    return g_pShader;
}
//---------------------------------------------------------------------------
void* OnGetID(const void* pKey)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < M_Texture_Count - 1; ++i)
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
    for (i = 0; i < M_Texture_Count - 1; ++i)
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
int LoadLandscapeFromBitmap(const char* fileName)
{
    CSR_Material      material;
    CSR_VertexCulling vc;
    CSR_VertexFormat  vf;
    CSR_Model*        pModel;
    CSR_PixelBuffer*  pBitmap;
    CSR_SceneItem*    pSceneItem;

    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;
    material.m_Wireframe   = 0;

    vc.m_Type = CSR_CT_None;
    vc.m_Face = CSR_CF_CW;

    vf.m_HasNormal         = 0;
    vf.m_HasTexCoords      = 1;
    vf.m_HasPerVertexColor = 1;

    // create a model to contain the landscape
    pModel = csrModelCreate();

    // succeeded?
    if (!pModel)
        return 0;

    // load a default grayscale bitmap from which a landscape will be generated
    pBitmap = csrPixelBufferFromBitmapFile(fileName);

    // succeeded?
    if (!pBitmap)
    {
        csrModelRelease(pModel, OnDeleteTexture);
        return 0;
    }

    // load the landscape mesh from the grayscale bitmap
    pModel->m_pMesh     = csrLandscapeCreate(pBitmap, 3.0f, 0.2f, &vf, &vc, &material, 0);
    pModel->m_MeshCount = 1;

    csrPixelBufferRelease(pBitmap);

    csrMat4Identity(&g_LandscapeMatrix);

    // add the model to the scene
    pSceneItem = csrSceneAddModel(g_pScene, pModel, 0, 1);
    csrSceneAddModelMatrix(g_pScene, pModel, &g_LandscapeMatrix);

    // succeeded?
    if (pSceneItem)
        pSceneItem->m_CollisionType = CSR_CO_Ground;

    // keep the key
    g_pLandscapeKey = pModel;

    return 1;
}
//---------------------------------------------------------------------------
void BuildBotMatrix()
{
    CSR_Vector3 axis;
    CSR_Vector3 factor;
    CSR_Matrix4 rotateXMatrix;
    CSR_Matrix4 rotateYMatrix;
    CSR_Matrix4 scaleMatrix;
    CSR_Matrix4 intermediateMatrix;

    csrMat4Identity(&g_Bot.m_Matrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // create the rotation matrix
    csrMat4Rotate(-M_PI / 2.0f, &axis, &rotateXMatrix);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // create the rotation matrix
    csrMat4Rotate(g_Bot.m_Angle, &axis, &rotateYMatrix);

    // set scale factor
    factor.m_X = 0.005f;
    factor.m_Y = 0.005f;
    factor.m_Z = 0.005f;

    // create the scale matrix
    csrMat4Scale(&factor, &scaleMatrix);

    // build the model matrix
    csrMat4Multiply(&scaleMatrix,        &rotateXMatrix, &intermediateMatrix);
    csrMat4Multiply(&intermediateMatrix, &rotateYMatrix, &g_Bot.m_Matrix);
}
//---------------------------------------------------------------------------
void BuildFaderMatrix()
{
    CSR_Vector3 translation;
    CSR_Vector3 axis;
    CSR_Matrix4 translateMatrix;
    CSR_Matrix4 rotateYMatrix;
    float       angle;

    csrMat4Identity(&g_FaderMatrix);

    // calculate the fader angle
    angle = (M_PI * 2.0f) - g_Angle;

    // calculate the fader position
    translation.m_X = g_BoundingSphere.m_Center.m_X - (0.05 * sin(angle));
    translation.m_Y = g_BoundingSphere.m_Center.m_Y;
    translation.m_Z = g_BoundingSphere.m_Center.m_Z - (0.05 * cos(angle));

    // create the translate matrix
    csrMat4Translate(&translation, &translateMatrix);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // create the rotation matrix
    csrMat4Rotate(angle, &axis, &rotateYMatrix);

    // build the final matrix
    csrMat4Multiply(&rotateYMatrix, &translateMatrix, &g_FaderMatrix);
}
//---------------------------------------------------------------------------
int ApplyGroundCollision(const CSR_Sphere* pBoundingSphere, CSR_Matrix4* pMatrix)
{
    if (!g_pScene)
        return 0;

    // validate the inputs
    if (!pBoundingSphere || !pMatrix)
        return 0;

    CSR_CollisionInput collisionInput;
    csrCollisionInputInit(&collisionInput);
    collisionInput.m_BoundingSphere.m_Radius = pBoundingSphere->m_Radius;

    CSR_Camera camera;

    // calculate the camera position in the 3d world, without the ground value
    camera.m_Position.m_X = -pBoundingSphere->m_Center.m_X;
    camera.m_Position.m_Y =  0.0f;
    camera.m_Position.m_Z = -pBoundingSphere->m_Center.m_Z;
    camera.m_xAngle       =  0.0f;
    camera.m_yAngle       =  g_Angle;
    camera.m_zAngle       =  0.0f;
    camera.m_Factor.m_X   =  1.0f;
    camera.m_Factor.m_Y   =  1.0f;
    camera.m_Factor.m_Z   =  1.0f;
    camera.m_MatCombType  =  IE_CT_Translate_Scale_Rotate;

    // get the view matrix matching with the camera
    csrSceneCameraToMatrix(&camera, pMatrix);

    CSR_Vector3 modelCenter;

    // get the model center
    modelCenter.m_X = 0.0f;
    modelCenter.m_Y = 0.0f;
    modelCenter.m_Z = 0.0f;

    CSR_Matrix4 invertView;
    float       determinant;

    // calculate the current camera position above the landscape
    csrMat4Inverse(pMatrix, &invertView, &determinant);
    csrMat4Transform(&invertView, &modelCenter, &collisionInput.m_BoundingSphere.m_Center);
    collisionInput.m_CheckPos = collisionInput.m_BoundingSphere.m_Center;

    CSR_CollisionOutput collisionOutput;

    // calculate the collisions in the whole scene
    csrSceneDetectCollision(g_pScene, &collisionInput, &collisionOutput, 0);

    // update the ground position directly inside the matrix (this is where the final value is required)
    pMatrix->m_Table[3][1] = -collisionOutput.m_GroundPos;

    if (collisionOutput.m_Collision & CSR_CO_Ground)
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
int CheckPlayerVisible()
{
    CSR_SceneItem*     pSceneItem;
    CSR_Vector3        botToPlayer;
    CSR_Vector3        botToPlayerDir;
    CSR_Polygon3Buffer polygons;
    CSR_Ray3           ray;
    float              angle;

    // calculate the bot to player vector
    botToPlayer.m_X = g_Bot.m_Geometry.m_Center.m_X - g_BoundingSphere.m_Center.m_X;
    botToPlayer.m_Y = g_Bot.m_Geometry.m_Center.m_Y - g_BoundingSphere.m_Center.m_Y;
    botToPlayer.m_Z = g_Bot.m_Geometry.m_Center.m_Z - g_BoundingSphere.m_Center.m_Z;

    // calculate the angle between the bot dir and the bot to player dir
    csrVec3Normalize(&botToPlayer, &botToPlayerDir);
    csrVec3Dot(&botToPlayerDir, &g_Bot.m_Dir, &angle);

    // reset the bot detection signal
    csrShaderEnable(g_pShader);
    glUniform1i(g_GrayscaleSlot, 0);

    // is bot showing the player?
    if (angle > -0.707f)
        return 0;

    // set the bot detection signal
    csrShaderEnable(g_pShader);
    glUniform1i(g_GrayscaleSlot, 1);

    return 1;
}
//------------------------------------------------------------------------------
int CheckBotHitPlayer()
{
    CSR_Figure3 firstSphere;
    CSR_Figure3 secondSphere;

    // get the player bounding object
    firstSphere.m_Type    = CSR_F3_Sphere;
    firstSphere.m_pFigure = &g_BoundingSphere;

    // get the bot bounding object
    secondSphere.m_Type    = CSR_F3_Sphere;
    secondSphere.m_pFigure = &g_Bot.m_Geometry;

    return csrIntersect3(&firstSphere, &secondSphere, 0, 0, 0);
}
//------------------------------------------------------------------------------
void OnPrepareWatching()
{
    // calculate a new sleep time and reset the elapsed time
    g_WatchingTime  = (double)(rand() % 200) / 100.0;
    g_ElapsedTime   = 0.0;
    g_BotShowPlayer = 0;
}
//------------------------------------------------------------------------------
int OnWatching(double elapsedTime)
{
    if (g_BotAlpha < 1.0f)
        g_BotAlpha += 1.0f * elapsedTime;

    if (g_BotAlpha >= 1.0f)
    {
        g_BotAlpha            = 1.0f;
        g_Bot.m_DyingSequence = E_DS_None;
    }

    // is player alive?
    if (!g_PlayerDying)
    {
        // player hit bot?
        if (g_BotHitPlayer)
        {
            g_BotDying = 1;

            // play hit sound
            csrSoundStop(g_pHitSound);
            csrSoundPlay(g_pHitSound);

            return 1;
        }

        // is bot showing player?
        if (CheckPlayerVisible())
        {
            g_BotShowPlayer = 1;
            return 1;
        }
    }

    // watching time elapsed?
    if (g_ElapsedTime >= g_WatchingTime)
        return 1;

    g_ElapsedTime += elapsedTime;
    return 0;
}
//------------------------------------------------------------------------------
void OnPrepareSearching()
{
    CSR_Vector3 dir;
    CSR_Vector3 refDir;
    CSR_Vector3 refNormal;
    float       angle;

    // build the reference dir
    refDir.m_X = 1.0f;
    refDir.m_Y = 0.0f;
    refDir.m_Z = 0.0f;

    // build the reference dir
    refNormal.m_X = 0.0f;
    refNormal.m_Y = 0.0f;
    refNormal.m_Z = 1.0f;

    // keep the current position as the start one
    g_Bot.m_StartPosition.m_X = g_Bot.m_Geometry.m_Center.m_X;
    g_Bot.m_StartPosition.m_Y = g_Bot.m_Geometry.m_Center.m_Z;

    // get a new position to move to
    g_Bot.m_EndPosition.m_X = -3.0f + ((rand() % 600) / 100.0f);
    g_Bot.m_EndPosition.m_Y = -3.0f + ((rand() % 600) / 100.0f);

    // calculate the bot direction
    dir.m_X = g_Bot.m_EndPosition.m_X - g_Bot.m_StartPosition.m_X;
    dir.m_Y = 0.0f;
    dir.m_Z = g_Bot.m_EndPosition.m_Y - g_Bot.m_StartPosition.m_Y;
    csrVec3Normalize(&dir, &g_Bot.m_Dir);

    // calculate the bot direction angle
    csrVec3Dot(&refDir, &g_Bot.m_Dir, &angle);
    g_Bot.m_Angle = acosf(angle);

    // calculate the bot direction normal angle
    csrVec3Dot(&refNormal, &g_Bot.m_Dir, &angle);

    // calculate the final bot direction angle
    if (angle < 0.0f)
        g_Bot.m_Angle = fmod(g_Bot.m_Angle, M_PI * 2.0);
    else
        g_Bot.m_Angle = (M_PI * 2.0f) - fmod(g_Bot.m_Angle, M_PI * 2.0);

    g_Bot.m_MovePos = 0.0f;
    g_BotShowPlayer = 0;
}
//------------------------------------------------------------------------------
int OnSearching(double elapsedTime)
{
    if (g_BotAlpha < 1.0f)
        g_BotAlpha += 1.0f * elapsedTime;

    if (g_BotAlpha >= 1.0f)
    {
        g_BotAlpha            = 1.0f;
        g_Bot.m_DyingSequence = E_DS_None;
    }

    // calculate the next position
    g_Bot.m_Geometry.m_Center.m_X = g_Bot.m_StartPosition.m_X + ((g_Bot.m_EndPosition.m_X - g_Bot.m_StartPosition.m_X) * g_Bot.m_MovePos);
    g_Bot.m_Geometry.m_Center.m_Z = g_Bot.m_StartPosition.m_Y + ((g_Bot.m_EndPosition.m_Y - g_Bot.m_StartPosition.m_Y) * g_Bot.m_MovePos);

    // calculate the next move
    g_Bot.m_MovePos += g_Bot.m_Velocity * elapsedTime;

    // is player alive?
    if (!g_PlayerDying)
    {
        // player hit bot?
        if (g_BotHitPlayer)
        {
            g_BotDying = 1;

            // play hit sound
            csrSoundStop(g_pHitSound);
            csrSoundPlay(g_pHitSound);

            return 1;
        }

        // is bot showing player?
        if (CheckPlayerVisible())
        {
            g_BotShowPlayer = 1;
            return 1;
        }
    }

    // check if move end was reached
    return g_Bot.m_MovePos >= 1.0f;
}
//------------------------------------------------------------------------------
void OnPrepareAttacking()
{
    CSR_Vector3 dir;
    CSR_Vector3 refDir;
    CSR_Vector3 refNormal;
    float       angle;

    // build the reference dir
    refDir.m_X = 1.0f;
    refDir.m_Y = 0.0f;
    refDir.m_Z = 0.0f;

    // build the reference dir
    refNormal.m_X = 0.0f;
    refNormal.m_Y = 0.0f;
    refNormal.m_Z = 1.0f;

    // keep the current position as the start one
    g_Bot.m_StartPosition.m_X = g_Bot.m_Geometry.m_Center.m_X;
    g_Bot.m_StartPosition.m_Y = g_Bot.m_Geometry.m_Center.m_Z;

    // get the player position as destination
    g_Bot.m_EndPosition.m_X = g_BoundingSphere.m_Center.m_X;
    g_Bot.m_EndPosition.m_Y = g_BoundingSphere.m_Center.m_Z;

    // calculate the bot direction
    dir.m_X = g_Bot.m_EndPosition.m_X - g_Bot.m_StartPosition.m_X;
    dir.m_Y = 0.0f;
    dir.m_Z = g_Bot.m_EndPosition.m_Y - g_Bot.m_StartPosition.m_Y;
    csrVec3Normalize(&dir, &g_Bot.m_Dir);

    // calculate the bot direction angle
    csrVec3Dot(&refDir, &g_Bot.m_Dir, &angle);
    g_Bot.m_Angle = acosf(angle);

    // calculate the bot direction normal angle
    csrVec3Dot(&refNormal, &g_Bot.m_Dir, &angle);

    // calculate the final bot direction angle
    if (angle < 0.0f)
        g_Bot.m_Angle = fmod(g_Bot.m_Angle, M_PI * 2.0);
    else
        g_Bot.m_Angle = (M_PI * 2.0f) - fmod(g_Bot.m_Angle, M_PI * 2.0);

    g_Bot.m_MovePos = 0.0f;
    g_BotShowPlayer = 0;
}
//------------------------------------------------------------------------------
int OnAttacking(double elapsedTime)
{
    if (g_BotAlpha < 1.0f)
        g_BotAlpha += 1.0f * elapsedTime;

    if (g_BotAlpha >= 1.0f)
    {
        g_BotAlpha            = 1.0f;
        g_Bot.m_DyingSequence = E_DS_None;
    }

    // calculate the next position
    g_Bot.m_Geometry.m_Center.m_X = g_Bot.m_StartPosition.m_X + ((g_Bot.m_EndPosition.m_X - g_Bot.m_StartPosition.m_X) * g_Bot.m_MovePos);
    g_Bot.m_Geometry.m_Center.m_Z = g_Bot.m_StartPosition.m_Y + ((g_Bot.m_EndPosition.m_Y - g_Bot.m_StartPosition.m_Y) * g_Bot.m_MovePos);

    // calculate the next move
    g_Bot.m_MovePos += g_Bot.m_Velocity * elapsedTime;

    // to keep the correct visual
    CheckPlayerVisible();

    // the bot hit the player?
    if (g_BotHitPlayer)
    {
        g_PlayerDying = 1;

        // play hit sound
        csrSoundStop(g_pHitSound);
        csrSoundPlay(g_pHitSound);

        return 1;
    }

    // check if move end was reached
    return (g_Bot.m_MovePos >= 1.0f);
}
//------------------------------------------------------------------------------
void OnPrepareDying()
{
    g_Bot.m_DyingSequence = E_DS_Dying;
    g_LastKnownIndex      = 0;
}
//------------------------------------------------------------------------------
int OnDying(double elapsedTime)
{
    // execute the dying sequence
    switch (g_Bot.m_DyingSequence)
    {
        case E_DS_Dying:
            // dying animation end reached?
            if (g_LastKnownIndex && g_ModelIndex < g_LastKnownIndex)
            {
                g_ModelIndex          = g_LastKnownIndex;
                g_Bot.m_DyingSequence = E_DS_FadeOut;
            }
            else
                g_LastKnownIndex = g_ModelIndex;

            break;

        case E_DS_FadeOut:
            // fade out the bot
            g_BotAlpha -= 1.0f * elapsedTime;

            // fade out end reached?
            if (g_BotAlpha <= 0.0f)
            {
                g_BotAlpha            = 0.0f;
                g_Bot.m_DyingSequence = E_DS_FadeIn;

                // ...thus a new position will be calculated
                OnPrepareSearching();

                // apply it immediately
                g_Bot.m_Geometry.m_Center.m_X = g_Bot.m_EndPosition.m_X;
                g_Bot.m_Geometry.m_Center.m_Z = g_Bot.m_EndPosition.m_Y;
            }

            break;

        case E_DS_FadeIn:
            g_BotShowPlayer = 0;
            g_BotDying      = 0;
            return 1;
    }

    return 0;
}
//------------------------------------------------------------------------------
void OnTaskChange(CSR_Task* pTask, double elapsedTime)
{
    // is bot dyimg?
    if (g_BotDying)
    {
        OnPrepareDying();

        g_pTaskManager->m_pTask->m_Action = E_BT_Dying;
        g_AnimIndex                       = 4;
        return;
    }

    // do bot attack the player?
    if (g_BotShowPlayer && !g_PlayerDying && pTask->m_Action != E_BT_Dying)
    {
        OnPrepareAttacking();

        g_AnimIndex     = 1;
        pTask->m_Action = E_BT_Attacking;
        return;
    }

    // prepare the next task
    switch (pTask->m_Action)
    {
        case E_BT_Watching:
            OnPrepareSearching();

            g_AnimIndex     = 1;
            pTask->m_Action = E_BT_Searching;
            break;

        case E_BT_Searching:
        case E_BT_Attacking:
        case E_BT_Dying:
            OnPrepareWatching();

            g_AnimIndex     = 0;
            pTask->m_Action = E_BT_Watching;
            break;
    }
}
//------------------------------------------------------------------------------
int OnTaskRun(CSR_Task* pTask, double elapsedTime)
{
    switch (pTask->m_Action)
    {
        case E_BT_Watching:  return OnWatching(elapsedTime);
        case E_BT_Searching: return OnSearching(elapsedTime);
        case E_BT_Attacking: return OnAttacking(elapsedTime);
        case E_BT_Dying:     return OnDying(elapsedTime);
    }

    return 0;
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
    CSR_VertexFormat vertexFormat;
    CSR_Material     material;
    CSR_PixelBuffer* pPixelBuffer = 0;
    CSR_Mesh*        pMesh;
    CSR_Model*       pModel;

    srand(0);

    // initialize the terrain limits
    g_TerrainLimits.m_Min.m_X = -3.0f;
    g_TerrainLimits.m_Min.m_Y = -3.0f;
    g_TerrainLimits.m_Max.m_X =  3.0f;
    g_TerrainLimits.m_Max.m_Y =  3.0f;

    // initialize the scene
    g_pScene = csrSceneCreate();

    // configure the scene background color
    g_pScene->m_Color.m_R = 0.45f;
    g_pScene->m_Color.m_G = 0.8f;
    g_pScene->m_Color.m_B = 1.0f;
    g_pScene->m_Color.m_A = 1.0f;

    // configure the scene ground direction
    g_pScene->m_GroundDir.m_X =  0.0f;
    g_pScene->m_GroundDir.m_Y = -1.0f;
    g_pScene->m_GroundDir.m_Z =  0.0f;

    // configure the scene view matrix
    csrMat4Identity(&g_pScene->m_ViewMatrix);

    // set the viewpoint bounding sphere default position
    g_BoundingSphere.m_Center.m_X = 0.0f;
    g_BoundingSphere.m_Center.m_Y = 0.0f;
    g_BoundingSphere.m_Center.m_Z = 0.0f;
    g_BoundingSphere.m_Radius     = 0.1f;

    // configure the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnGetShader     = OnGetShader;
    g_SceneContext.m_fOnGetID         = OnGetID;
    g_SceneContext.m_fOnGetMDLIndex   = OnGetMDLIndex;
    g_SceneContext.m_fOnDeleteTexture = OnDeleteTexture;

    // compile, link and use shader
    g_pShader = csrOpenGLShaderLoadFromStr(&g_VSTextured[0],
                                            sizeof(g_VSTextured),
                                           &g_FSTextured[0],
                                            sizeof(g_FSTextured),
                                            0,
                                            0);

    // succeeded?
    if (!g_pShader)
    {
        // show the error message to the user
        printf("Failed to load the scene shader.\n");
        return;
    }

    csrShaderEnable(g_pShader);

    // get shader attributes
    g_pShader->m_VertexSlot   = glGetAttribLocation (g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_ColorSlot    = glGetAttribLocation (g_pShader->m_ProgramID, "csr_aColor");
    g_pShader->m_TexCoordSlot = glGetAttribLocation (g_pShader->m_ProgramID, "csr_aTexCoord");
    g_pShader->m_TextureSlot  = glGetAttribLocation (g_pShader->m_ProgramID, "csr_sColorMap");
    g_GrayscaleSlot           = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uGrayscale");
    g_AlphaSlot               = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uAlpha");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // load the landscape
    if (!LoadLandscapeFromBitmap(LANDSCAPE_DATA_FILE))
    {
        // show the error message to the user
        printf("The landscape could not be loaded.\n");
        return;
    }

    // get back the scene item containing the model
    CSR_SceneItem* pItem = csrSceneGetItem(g_pScene, g_pLandscapeKey);

    // found it?
    if (!pItem)
    {
        // show the error message to the user
        printf("The landscape was not found in the scene.\n");
        return;
    }

    // load landscape texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(LANDSCAPE_TEXTURE_FILE);
    g_ID[0].m_pKey     = &((CSR_Model*)(pItem->m_pModel))->m_pMesh[0].m_Skin.m_Texture;
    g_ID[0].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[0].m_UseCount = 1;

    // landscape texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    g_Bot.m_Angle         = 0.0f;
    g_Bot.m_Velocity      = 0.5f;
    g_Bot.m_DyingSequence = E_DS_None;

    BuildBotMatrix();

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    // configure the material
    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 1;
    material.m_Wireframe   = 0;

    // load the MDL model
    g_Bot.m_pModel = csrMDLOpen(MDL_FILE, 0, &vertexFormat, 0, &material, 0, OnApplySkin, OnDeleteTexture);

    // add the model to the scene
    CSR_SceneItem* pSceneItem = csrSceneAddMDL(g_pScene, g_Bot.m_pModel, 1, 0);
    csrSceneAddModelMatrix(g_pScene, g_Bot.m_pModel, &g_Bot.m_Matrix);

    // succeeded?
    if (pSceneItem)
        pSceneItem->m_CollisionType = CSR_CO_Ground;

    // configure bot geometry
    g_Bot.m_Geometry.m_Center.m_X = 3.0f;
    g_Bot.m_Geometry.m_Center.m_Y = 0.0f;
    g_Bot.m_Geometry.m_Center.m_Z = 0.0f;
    g_Bot.m_Geometry.m_Radius     = 0.125f;

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    // configure the material
    material.m_Color       = 0xFF0000FF;
    material.m_Transparent = 1;
    material.m_Wireframe   = 0;

    // create a surface for the fader
    g_pFader = csrShapeCreateSurface(100.0f, 100.0f, &vertexFormat, 0, &material, 0);

    // add the mesh to the scene
    pSceneItem = csrSceneAddMesh(g_pScene, g_pFader, 1, 0);
    csrSceneAddModelMatrix(g_pScene, g_pFader, &g_FaderMatrix);

    // create a resource for the fader texture
    g_ID[2].m_pKey     = g_pFader;
    g_ID[2].m_UseCount = 1;

    // load fader texture
    pPixelBuffer = csrPixelBufferFromBitmapFile(FADER_TEXTURE_FILE);
    g_ID[2].m_ID = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);

    // fader texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // initialize the fader matrix
    BuildFaderMatrix();

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
    g_ID[3].m_pKey     = &g_pScene->m_pSkybox->m_Skin.m_CubeMap;
    g_ID[3].m_ID       = csrOpenGLCubemapLoad(pCubemapFileNames);
    g_ID[3].m_UseCount = 1;

    csrSoundInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // load the sound files
    g_pFootStepLeftSound  = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, FOOT_STEP_LEFT_SOUND_FILE);
    g_pFootStepRightSound = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, FOOT_STEP_RIGHT_SOUND_FILE);
    g_pHitSound           = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, HIT_SOUND_FILE);

    // change the volume
    csrSoundChangeVolume(g_pFootStepLeftSound,  0.2f);
    csrSoundChangeVolume(g_pFootStepRightSound, 0.2f);

    // initialize the IA task manager
    g_pTaskManager = csrTaskManagerCreate();
    csrTaskManagerInit(g_pTaskManager);
    g_pTaskManager->m_pTask = csrTaskCreate();
    g_pTaskManager->m_Count = 1;

    // initialize the bot task
    csrTaskInit(g_pTaskManager->m_pTask);
    g_pTaskManager->m_pTask->m_Action = E_BT_Watching;

    // initialize the IA task manager context
    g_TaskContext.m_fOnTaskChange = OnTaskChange;
    g_TaskContext.m_fOnTaskRun    = OnTaskRun;

    // prepare the first bot task
    OnPrepareWatching();
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the task manager
    csrTaskManagerRelease(g_pTaskManager);

    // delete the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);
    g_pScene = 0;

    // delete scene shader
    csrOpenGLShaderRelease(g_pShader);
    g_pShader = 0;

    // delete skybox shader
    csrOpenGLShaderRelease(g_pSkyboxShader);
    g_pSkyboxShader = 0;

    // stop running step sound, if needed
    csrSoundStop(g_pFootStepLeftSound);
    csrSoundStop(g_pFootStepRightSound);
    csrSoundStop(g_pHitSound);

    // release OpenAL interface
    csrSoundRelease(g_pFootStepLeftSound);
    csrSoundRelease(g_pFootStepRightSound);
    csrSoundRelease(g_pHitSound);
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
    float          posY;
    float          angle;
    CSR_Sphere     prevSphere;
    CSR_SceneItem* pSceneItem;

    // check if the bot hit the player
    g_BotHitPlayer = CheckBotHitPlayer();

    // run the IA
    csrTaskManagerExecute(g_pTaskManager, &g_TaskContext, timeStep_sec);

    // rebuild the bot matrix
    BuildBotMatrix();

    pSceneItem = csrSceneGetItem(g_pScene, g_pLandscapeKey);
    posY       = M_CSR_NoGround;

    // calculate the y position where to place the bot
    csrGroundPosY(&g_Bot.m_Geometry, pSceneItem->m_pAABBTree, &g_pScene->m_GroundDir, 0, &posY);

    // set the bot y position directly inside the matrix
    g_Bot.m_Matrix.m_Table[3][0] = g_Bot.m_Geometry.m_Center.m_X;
    g_Bot.m_Matrix.m_Table[3][1] = posY;
    g_Bot.m_Matrix.m_Table[3][2] = g_Bot.m_Geometry.m_Center.m_Z;

    // calculate next model indexes to show
    if (g_Bot.m_DyingSequence != E_DS_FadeOut)
        csrMDLUpdateIndex(g_Bot.m_pModel,
                          g_FPS,
                          g_AnimIndex,
                         &g_TextureIndex,
                         &g_ModelIndex,
                         &g_MeshIndex,
                         &g_TextureLastTime,
                         &g_ModelLastTime,
                         &g_MeshLastTime,
                          timeStep_sec);

    // is player dying?
    if (g_PlayerDying)
    {
        // reset the bot data
        g_pTaskManager->m_pTask->m_Action = E_BT_Watching;
        g_BotShowPlayer                   = 0;
        glUniform1i(g_GrayscaleSlot, 0);

        // fade to red
        g_FaderAlpha += 1.0f * timeStep_sec;

        // fader max value reached?
        if (g_FaderAlpha >= 1.0f)
        {
            g_FaderAlpha = 1.0f;

            // get a new player position
            g_BoundingSphere.m_Center.m_X = -3.0f + ((rand() % 600) / 100.0f);
            g_BoundingSphere.m_Center.m_Z = -3.0f + ((rand() % 600) / 100.0f);

            // update the fader position
            BuildFaderMatrix();

            g_PlayerDying = 0;
        }

        return;
    }

    // fade out
    if (g_FaderAlpha > 0.0f)
        g_FaderAlpha -= 1.0f * timeStep_sec;
    else
        g_FaderAlpha = 0.0f;

    // if screen isn't touched, do nothing
    if (!g_TouchOrigin.m_X || !g_TouchOrigin.m_Y)
        return;

    prevSphere = g_BoundingSphere;

    // calculate the angle formed by the touch gesture x and y distances
    if ((g_TouchPosition.m_X < g_TouchOrigin.m_X || g_TouchPosition.m_Y < g_TouchOrigin.m_Y) &&
       !(g_TouchPosition.m_X < g_TouchOrigin.m_X && g_TouchPosition.m_Y < g_TouchOrigin.m_Y))
        angle = -atanf((g_TouchPosition.m_Y - g_TouchOrigin.m_Y) / (g_TouchPosition.m_X - g_TouchOrigin.m_X));
    else
        angle =  atanf((g_TouchPosition.m_Y - g_TouchOrigin.m_Y) / (g_TouchPosition.m_X - g_TouchOrigin.m_X));

    // calculate the possible min and max values for each axis
    float minX = g_TouchOrigin.m_X - (cosf(angle) * g_ControlRadius);
    float maxX = g_TouchOrigin.m_X + (cosf(angle) * g_ControlRadius);
    float minY = g_TouchOrigin.m_Y - (sinf(angle) * g_ControlRadius);
    float maxY = g_TouchOrigin.m_Y + (sinf(angle) * g_ControlRadius);

    // limit the touch gesture in a radius distance
    if (g_TouchPosition.m_X > maxX)
        g_TouchPosition.m_X = maxX;
    else
    if (g_TouchPosition.m_X < minX)
        g_TouchPosition.m_X = minX;

    if (g_TouchPosition.m_Y > maxY)
        g_TouchPosition.m_Y = maxY;
    else
    if (g_TouchPosition.m_Y < minY)
        g_TouchPosition.m_Y = minY;

    // calculate the final pos and dir velocity
    float posVelocity = (g_PosVelocity * ((g_TouchPosition.m_Y - g_TouchOrigin.m_Y) / g_TouchOrigin.m_Y));
    float dirVelocity = (g_DirVelocity * ((g_TouchPosition.m_X - g_TouchOrigin.m_X) / g_TouchOrigin.m_X));

    // calculate the next player direction
    g_Angle += dirVelocity * timeStep_sec;

    // validate it
    if (g_Angle > M_PI * 2.0f)
        g_Angle -= M_PI * 2.0f;
    else
    if (g_Angle < 0.0f)
        g_Angle += M_PI * 2.0f;

    // calculate the next player position
    g_BoundingSphere.m_Center.m_X += posVelocity * cosf(g_Angle + (M_PI * 0.5f)) * timeStep_sec;
    g_BoundingSphere.m_Center.m_Z += posVelocity * sinf(g_Angle + (M_PI * 0.5f)) * timeStep_sec;

    // calculate the ground position and check if next position is valid
    if (!ApplyGroundCollision(&g_BoundingSphere, &g_pScene->m_ViewMatrix))
    {
        // invalid next position, get the scene item (just one for this scene)
        const CSR_SceneItem* pItem = csrSceneGetItem(g_pScene, g_pLandscapeKey);

        // found it?
        if (pItem)
        {
            // check if the x position is out of bounds, and correct it if yes
            if (g_BoundingSphere.m_Center.m_X <= pItem->m_pAABBTree->m_pBox->m_Min.m_X ||
                g_BoundingSphere.m_Center.m_X >= pItem->m_pAABBTree->m_pBox->m_Max.m_X)
                g_BoundingSphere.m_Center.m_X = prevSphere.m_Center.m_X;

            // do the same thing with the z position. Doing that separately for each axis will make
            // the point of view to slide against the landscape border (this is possible because the
            // landscape is axis-aligned)
            if (g_BoundingSphere.m_Center.m_Z <= pItem->m_pAABBTree->m_pBox->m_Min.m_Z ||
                g_BoundingSphere.m_Center.m_Z >= pItem->m_pAABBTree->m_pBox->m_Max.m_Z)
                g_BoundingSphere.m_Center.m_Z = prevSphere.m_Center.m_Z;
        }
        else
            // failed to get the scene item, just revert the position
            g_BoundingSphere.m_Center = prevSphere.m_Center;

        // recalculate the ground value (this time the collision result isn't tested, because the
        // previous position is always considered as valid)
        ApplyGroundCollision(&g_BoundingSphere, &g_pScene->m_ViewMatrix);
    }

    BuildFaderMatrix();

    // calculate next time where the step sound should be played
    g_StepTime += (timeStep_sec * 1000.0f);

    // count frames
    while (g_StepTime > g_StepInterval)
    {
        // do play the left or right footstep sound?
        if (!(g_AlternateStep % 2))
        {
            csrSoundStop(g_pFootStepLeftSound);
            csrSoundPlay(g_pFootStepLeftSound);
        }
        else
        {
            csrSoundStop(g_pFootStepRightSound);
            csrSoundPlay(g_pFootStepRightSound);
        }

        g_StepTime = 0.0f;

        // next time the other footstep sound will be played
        g_AlternateStep = (g_AlternateStep + 1) & 1;
    }
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    // finalize the view matrix
    ApplyGroundCollision(&g_BoundingSphere, &g_pScene->m_ViewMatrix);

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
