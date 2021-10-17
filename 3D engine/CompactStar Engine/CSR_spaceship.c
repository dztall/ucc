/****************************************************************************
 * ==> Spaceship game demo -------------------------------------------------*
 ****************************************************************************
 * Description : A spaceship game demo                                      *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2019, this file is part of the CompactStar Engine.  *
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
#include "SDK/CSR_Particles.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_Physics.h"
#include "SDK/CSR_Sound.h"
#include "SDK/CSR_MobileC_Debug.h"

#include <ccr.h>

#define BACKGROUND_TEXTURE "Resources/background.bmp"
#define STAR_TEXTURE       "Resources/star.bmp"
#define SPACESHIP_MODEL    "Resources/spaceship.mdl"
#define METEORE_MODEL      "Resources/meteore.mdl"
#define FIRE_SOUND         "Resources/fire.wav"
#define EXPLOSION_SOUND    "Resources/explosion.wav"

#define TEXTURE_COUNT 4
#define STAR_COUNT    50
#define METEORE_COUNT 20

//----------------------------------------------------------------------------
const char g_VSColorAlpha[] =
    "precision mediump float;"
    "attribute vec3  csr_aVertices;"
    "attribute vec4  csr_aColor;"
    "uniform   float csr_uAlpha;"
    "uniform   mat4  csr_uProjection;"
    "uniform   mat4  csr_uView;"
    "uniform   mat4  csr_uModel;"
    "varying   vec4  csr_vColor;"
    "varying   float csr_fAlpha;"
    "void main(void)"
    "{"
    "    csr_vColor  = csr_aColor;"
    "    csr_fAlpha  = csr_uAlpha;"
    "    gl_Position = csr_uProjection * csr_uView * csr_uModel * vec4(csr_aVertices, 1.0);"
    "}";
//----------------------------------------------------------------------------
const char g_FSColorAlpha[] =
    "precision mediump float;"
    "varying lowp vec4  csr_vColor;"
    "varying      float csr_fAlpha;"
    "void main(void)"
    "{"
    "    gl_FragColor = vec4(csr_vColor.x, csr_vColor.y, csr_vColor.z, csr_fAlpha);"
    "}";
//----------------------------------------------------------------------------
const char g_VSTextured[] =
    "precision mediump float;"
    "attribute vec3  csr_aVertices;"
    "attribute vec4  csr_aColor;"
    "attribute vec2  csr_aTexCoord;"
    "uniform   mat4  csr_uProjection;"
    "uniform   mat4  csr_uView;"
    "uniform   mat4  csr_uModel;"
    "uniform   float csr_uAlpha;"
    "varying   vec4  csr_vColor;"
    "varying   vec2  csr_vTexCoord;"
    "varying   float csr_fAlpha;"
    "void main(void)"
    "{"
    "    csr_vColor    = csr_aColor;"
    "    csr_fAlpha    = csr_uAlpha;"
    "    csr_vTexCoord = csr_aTexCoord;"
    "    gl_Position   = csr_uProjection * csr_uView * csr_uModel * vec4(csr_aVertices, 1.0);"
    "}";
//----------------------------------------------------------------------------
const char g_FSTextured[] =
    "precision mediump float;"
    "uniform sampler2D  csr_sColorMap;"
    "varying lowp vec4  csr_vColor;"
    "varying      vec2  csr_vTexCoord;"
    "varying      float csr_fAlpha;"
    "void main(void)"
    "{"
    "    vec4 color   = vec4(csr_vColor.x, csr_vColor.y, csr_vColor.z, csr_fAlpha);"
    "    gl_FragColor = color * texture2D(csr_sColorMap, csr_vTexCoord);"
    "}";
//------------------------------------------------------------------------------
typedef struct
{
    void*       m_pKey;
    CSR_Matrix4 m_Matrix;
    CSR_Rect    m_Bounds;
} CSR_Spaceship;
//------------------------------------------------------------------------------
typedef struct
{
    CSR_Vector3 m_StartPos;
    CSR_Vector3 m_Scaling;
    CSR_Vector3 m_Rotation;
    CSR_Vector3 m_RotationVelocity;
} CSR_Meteore;
//------------------------------------------------------------------------------
CSR_Scene*        g_pScene                  = 0;
CSR_OpenGLShader* g_pShader                 = 0;
CSR_OpenGLShader* g_pFlameShader            = 0;
float             g_Angle                   = 0.0f;
float             g_Alpha                   = 0.3f;
float             g_MinAlpha                = 0.25f;
float             g_MaxAlpha                = 0.35f;
float             g_AlphaOffset             = 1.0f;
float             g_ShipAlpha               = 1.0f;
float             g_LaserTime               = 0.0f;
float             g_LaserInterval           = 0.5f;
float             g_Aspect                  = 1.0f;
int               g_LaserVisible            = 0;
int               g_TextureIndex            = 0;
int               g_ShipDying               = 0;
CSR_Spaceship     g_Spaceship;
CSR_MDL*          g_pMeteore                = 0;
CSR_Mesh*         g_pFlame                  = 0;
CSR_Mesh*         g_pLaser                  = 0;
CSR_Mesh*         g_pStar                   = 0;
CSR_Box           g_StarBox;
CSR_Particles*    g_pStars                  = 0;
CSR_Particles*    g_pMeteores               = 0;
CSR_SceneContext  g_SceneContext;
CSR_Circle        g_SpaceshipBoundingCircle;
CSR_Circle        g_MeteoreBoundingCircle;
CSR_Ray2          g_LaserRay;
CSR_Vector2       g_ScreenOrigin;
CSR_Matrix4       g_FlameMatrix;
CSR_Matrix4       g_LaserMatrix;
CSR_Matrix4       g_Background;
GLuint            g_AlphaSlot               = 0;
GLuint            g_TexAlphaSlot            = 0;
ALCdevice*        g_pOpenALDevice           = 0;
ALCcontext*       g_pOpenALContext          = 0;
CSR_Sound*        g_pFireSound              = 0;
CSR_Sound*        g_pExplosionSound         = 0;
CSR_OpenGLID      g_ID[TEXTURE_COUNT];
CSR_Meteore       g_Meteores[METEORE_COUNT];
//---------------------------------------------------------------------------
int CheckLaserCollision(const CSR_Circle* pMeteoreBoundingCircle, const CSR_Ray2* pLaserRay)
{
    CSR_Figure2 figure1;
    CSR_Figure2 figure2;

    figure1.m_Type    = CSR_F2_Ray;
    figure1.m_pFigure = pLaserRay;

    figure2.m_Type    = CSR_F2_Circle;
    figure2.m_pFigure = pMeteoreBoundingCircle;

    return csrIntersect2(&figure1, &figure2, 0, 0);
}
//---------------------------------------------------------------------------
int CheckSpaceshipCollision(const CSR_Circle* pMeteoreBoundingCircle, const CSR_Circle* pSpaceshipBoundingCircle)
{
    CSR_Figure2 figure1;
    CSR_Figure2 figure2;
    CSR_Vector2 p1;
    CSR_Vector2 p2;

    figure1.m_Type    = CSR_F2_Circle;
    figure1.m_pFigure = pMeteoreBoundingCircle;

    figure2.m_Type    = CSR_F2_Circle;
    figure2.m_pFigure = pSpaceshipBoundingCircle;

    return csrIntersect2(&figure1, &figure2, &p1, &p2);
}
//---------------------------------------------------------------------------
void BuildSpaceshipMatrix(float angle)
{
    CSR_Vector3 translation;
    translation.m_X =  0.0f;
    translation.m_Y =  0.0f;
    translation.m_Z = -1.75f;

    CSR_Matrix4 translationMatrix;

    // apply translation to spaceship matrix
    csrMat4Translate(&translation, &translationMatrix);

    CSR_Vector3 axis;
    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;

    CSR_Matrix4 rzMatrix;

    // apply rotation on z axis to spaceship matrix
    csrMat4Rotate(angle, &axis, &rzMatrix);

    CSR_Vector3 factor;
    factor.m_X = 0.05f;
    factor.m_Y = 0.05f;
    factor.m_Z = 0.05f;

    CSR_Matrix4 scaleMatrix;

    // apply scaling to spaceship matrix
    csrMat4Scale(&factor, &scaleMatrix);

    CSR_Matrix4 buildMatrix;

    // build the spaceship model matrix
    csrMat4Multiply(&scaleMatrix, &rzMatrix, &buildMatrix);
    csrMat4Multiply(&buildMatrix, &translationMatrix, &g_Spaceship.m_Matrix);
}
//---------------------------------------------------------------------------
void BuildFlameMatrix(float angle)
{
    CSR_Vector3 translation;
    translation.m_X =  0.0f;
    translation.m_Y = -0.2f;
    translation.m_Z = -1.75f;

    CSR_Matrix4 translationMatrix;

    // apply translation to spaceship rear flame matrix
    csrMat4Translate(&translation, &translationMatrix);

    CSR_Vector3 axis;
    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;

    CSR_Matrix4 rzMatrix;

    // apply rotation on z axis to spaceship rear flame matrix
    csrMat4Rotate(angle, &axis, &rzMatrix);

    // build the spaceship rear flame matrix
    csrMat4Multiply(&translationMatrix, &rzMatrix, &g_FlameMatrix);
}
//---------------------------------------------------------------------------
void BuildMeteoreMatrix(const CSR_Vector3* pTranslation,
                        const CSR_Vector3* pRotation,
                        const CSR_Vector3* pScaling,
                              CSR_Matrix4* pMeteoreMatrix)
{
    CSR_Matrix4 translationMatrix;

    // apply translation to meteore matrix
    csrMat4Translate(pTranslation, &translationMatrix);

    CSR_Vector3 axis;
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    CSR_Matrix4 rxMatrix;

    // apply rotation on x axis to meteore matrix
    csrMat4Rotate(pRotation->m_X, &axis, &rxMatrix);

    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    CSR_Matrix4 ryMatrix;

    // apply rotation on y axis to meteore matrix
    csrMat4Rotate(pRotation->m_Y, &axis, &ryMatrix);

    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;

    CSR_Matrix4 rzMatrix;

    // apply rotation on z axis to meteore matrix
    csrMat4Rotate(pRotation->m_Z, &axis, &rzMatrix);

    CSR_Matrix4 scaleMatrix;

    // apply scaling to meteore matrix
    csrMat4Scale(pScaling, &scaleMatrix);

    CSR_Matrix4 intermediateMatrix1;
    CSR_Matrix4 intermediateMatrix2;
    CSR_Matrix4 intermediateMatrix3;

    // build the meteore matrix
    csrMat4Multiply(&scaleMatrix,         &rxMatrix,          &intermediateMatrix1);
    csrMat4Multiply(&intermediateMatrix1, &ryMatrix,          &intermediateMatrix2);
    csrMat4Multiply(&intermediateMatrix2, &rzMatrix,          &intermediateMatrix3);
    csrMat4Multiply(&intermediateMatrix3, &translationMatrix,  pMeteoreMatrix);
}
//---------------------------------------------------------------------------
void BuildLaserMatrix(float angle)
{
    float posOffset;

    if (g_LaserVisible)
        posOffset = 5.15f;
    else
        posOffset = 99999.0f;

    CSR_Vector3 translation;
    translation.m_X =  posOffset * -sinf(g_Angle);
    translation.m_Y =  posOffset *  cosf(g_Angle);
    translation.m_Z = -1.75f;

    CSR_Matrix4 translationMatrix;

    // apply translation to laser matrix
    csrMat4Translate(&translation, &translationMatrix);

    CSR_Vector3 axis;
    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;

    CSR_Matrix4 rzMatrix;

    // apply rotation on z axis to laser matrix
    csrMat4Rotate(angle, &axis, &rzMatrix);

    // build the laser model matrix
    csrMat4Multiply(&rzMatrix, &translationMatrix, &g_LaserMatrix);
}
//---------------------------------------------------------------------------
void OnCalculateStarMotion(const CSR_Particles* pParticles,
                                 CSR_Particle*  pParticle,
                                 float          elapsedTime)
{
    pParticle->m_pBody->m_Velocity.m_X =  0.5f * sinf(g_Angle);
    pParticle->m_pBody->m_Velocity.m_Y = -0.5f * cosf(g_Angle);

    // calculate next star position
    pParticle->m_pMatrix->m_Table[3][0] += pParticle->m_pBody->m_Velocity.m_X * elapsedTime;
    pParticle->m_pMatrix->m_Table[3][1] += pParticle->m_pBody->m_Velocity.m_Y * elapsedTime;
    pParticle->m_pMatrix->m_Table[3][2] += pParticle->m_pBody->m_Velocity.m_Z * elapsedTime;

    // limit x pos inside the star box
    if (pParticle->m_pMatrix->m_Table[3][0] <= g_StarBox.m_Min.m_X)
        pParticle->m_pMatrix->m_Table[3][0] += (g_StarBox.m_Max.m_X - g_StarBox.m_Min.m_X);
    else
    if (pParticle->m_pMatrix->m_Table[3][0] >= g_StarBox.m_Max.m_X)
        pParticle->m_pMatrix->m_Table[3][0] -= (g_StarBox.m_Max.m_X - g_StarBox.m_Min.m_X);

    // limit y pos inside the star box
    if (pParticle->m_pMatrix->m_Table[3][1] <= g_StarBox.m_Min.m_Y)
        pParticle->m_pMatrix->m_Table[3][1] += (g_StarBox.m_Max.m_Y - g_StarBox.m_Min.m_Y);
    else
    if (pParticle->m_pMatrix->m_Table[3][1] >= g_StarBox.m_Max.m_Y)
        pParticle->m_pMatrix->m_Table[3][1] -= (g_StarBox.m_Max.m_Y - g_StarBox.m_Min.m_Y);

    // limit z pos inside the star box
    if (pParticle->m_pMatrix->m_Table[3][2] <= g_StarBox.m_Min.m_Z)
        pParticle->m_pMatrix->m_Table[3][2] += (g_StarBox.m_Max.m_Z - g_StarBox.m_Min.m_Z);
    else
    if (pParticle->m_pMatrix->m_Table[3][2] >= g_StarBox.m_Max.m_Z)
        pParticle->m_pMatrix->m_Table[3][2] -= (g_StarBox.m_Max.m_Z - g_StarBox.m_Min.m_Z);
}
//---------------------------------------------------------------------------
void OnCalculateMeteoreMotion(const CSR_Particles* pParticles,
                                    CSR_Particle*  pParticle,
                                    float          elapsedTime)
{
    CSR_Vector3 position;
    size_t      index;

    // get particle index (mass was used for that)
    index = (size_t)pParticle->m_pBody->m_Mass;

    // calculate velocity on x and y axis
    pParticle->m_pBody->m_Velocity.m_X =  0.5f * sinf(g_Angle);
    pParticle->m_pBody->m_Velocity.m_Y = -0.5f * cosf(g_Angle);

    // calculate next meteore position
    position.m_X = pParticle->m_pMatrix->m_Table[3][0] + pParticle->m_pBody->m_Velocity.m_X * elapsedTime;
    position.m_Y = pParticle->m_pMatrix->m_Table[3][1] + pParticle->m_pBody->m_Velocity.m_Y * elapsedTime;
    position.m_Z = pParticle->m_pMatrix->m_Table[3][2] + pParticle->m_pBody->m_Velocity.m_Z * elapsedTime;

    // calculate next meteore rotation
    g_Meteores[index].m_Rotation.m_X += g_Meteores[index].m_RotationVelocity.m_X * elapsedTime;
    g_Meteores[index].m_Rotation.m_Y += g_Meteores[index].m_RotationVelocity.m_Y * elapsedTime;
    g_Meteores[index].m_Rotation.m_Z += g_Meteores[index].m_RotationVelocity.m_Z * elapsedTime;

    // build meteore matrix
    BuildMeteoreMatrix(&position,
                       &g_Meteores[index].m_Rotation,
                       &g_Meteores[index].m_Scaling,
                        pParticle->m_pMatrix);

    // put the meteore bounding circle to the location to test
    g_MeteoreBoundingCircle.m_Center.m_X = pParticle->m_pMatrix->m_Table[3][0];
    g_MeteoreBoundingCircle.m_Center.m_Y = pParticle->m_pMatrix->m_Table[3][1];

    // hit by a laser?
    if (g_LaserVisible && CheckLaserCollision(&g_MeteoreBoundingCircle, &g_LaserRay))
    {
    	// sort new positions
        float newPosX = (((float)(rand() % (int)200.0f)) * 0.01f);
        float newPosY = (((float)(rand() % (int)200.0f)) * 0.01f);

        // recalculate a new position out of the screen, on the left or on the right
        if (newPosX >= 1.0f)
            pParticle->m_pMatrix->m_Table[3][0] = g_StarBox.m_Max.m_X - (newPosX - 1.0f);
        else
            pParticle->m_pMatrix->m_Table[3][0] = g_StarBox.m_Min.m_X + newPosX;

        // recalculate a new position out of the screen, on the top or on the bottom
        if (newPosY >= 1.0f)
            pParticle->m_pMatrix->m_Table[3][1] = g_StarBox.m_Max.m_Y - (newPosY - 1.0f);
        else
            pParticle->m_pMatrix->m_Table[3][1] = g_StarBox.m_Min.m_Y + newPosY;

        pParticle->m_pMatrix->m_Table[3][2] = g_Meteores[index].m_StartPos.m_Z;
    }

    // hitting the spaceship?
    if (!g_ShipDying && CheckSpaceshipCollision(&g_MeteoreBoundingCircle, &g_SpaceshipBoundingCircle))
    {
        g_ShipDying = 1;
        csrSoundPlay(g_pExplosionSound);
    }

    // limit x pos inside the star box
    if (pParticle->m_pMatrix->m_Table[3][0] <= g_StarBox.m_Min.m_X)
        pParticle->m_pMatrix->m_Table[3][0] += (g_StarBox.m_Max.m_X - g_StarBox.m_Min.m_X);
    else
    if (pParticle->m_pMatrix->m_Table[3][0] >= g_StarBox.m_Max.m_X)
        pParticle->m_pMatrix->m_Table[3][0] -= (g_StarBox.m_Max.m_X - g_StarBox.m_Min.m_X);

    // limit y pos inside the star box
    if (pParticle->m_pMatrix->m_Table[3][1] <= g_StarBox.m_Min.m_Y)
        pParticle->m_pMatrix->m_Table[3][1] += (g_StarBox.m_Max.m_Y - g_StarBox.m_Min.m_Y);
    else
    if (pParticle->m_pMatrix->m_Table[3][1] >= g_StarBox.m_Max.m_Y)
        pParticle->m_pMatrix->m_Table[3][1] -= (g_StarBox.m_Max.m_Y - g_StarBox.m_Min.m_Y);

    // limit z pos inside the star box
    if (pParticle->m_pMatrix->m_Table[3][2] <= g_StarBox.m_Min.m_Z)
        pParticle->m_pMatrix->m_Table[3][2] += (g_StarBox.m_Max.m_Z - g_StarBox.m_Min.m_Z);
    else
    if (pParticle->m_pMatrix->m_Table[3][2] >= g_StarBox.m_Max.m_Z)
        pParticle->m_pMatrix->m_Table[3][2] -= (g_StarBox.m_Max.m_Z - g_StarBox.m_Min.m_Z);
}
//---------------------------------------------------------------------------
void OnApplySkin(size_t index, const CSR_Skin* pSkin, int* pCanRelease)
{
    // should not be hardcoded, however there is only 2 models which will use this function in this demo,
    // so it is safe to do that
    g_ID[g_TextureIndex].m_pKey     = (void*)(&pSkin->m_Texture);
    g_ID[g_TextureIndex].m_ID       = csrOpenGLTextureFromPixelBuffer(pSkin->m_Texture.m_pBuffer);
    g_ID[g_TextureIndex].m_UseCount = 1;

    // from now the source texture will no longer be used
    if (pCanRelease)
        *pCanRelease = 1;
}
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    // flame or laser?
    if (pModel == g_pFlame || pModel == g_pLaser)
    {
        csrShaderEnable(g_pFlameShader);

        // set alpha transparency level
        if (pModel == g_pFlame)
            glUniform1f(g_AlphaSlot, g_Alpha * g_ShipAlpha);
        else
            glUniform1f(g_AlphaSlot, g_Alpha + 0.5f);

        return g_pFlameShader;
    }

    csrShaderEnable(g_pShader);

    // spaceship?
    if (pModel == g_Spaceship.m_pKey)
        glUniform1f(g_TexAlphaSlot, g_ShipAlpha);
    else
        glUniform1f(g_TexAlphaSlot, 1.0f);

    return g_pShader;
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

    csrShaderEnable(g_pFlameShader);

    // connect projection matrix to shader
    projectionUniform = glGetUniformLocation(g_pFlameShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_VertexFormat vertexFormat;
    CSR_Material     material;
    CSR_PixelBuffer* pPixelBuffer = 0;
    CSR_Mesh*        pMesh;
    CSR_MDL*         pMDL;
    CSR_SceneItem*   pSceneItem;
    size_t           i;

    // initialize the scene
    g_pScene = csrSceneCreate();

    // configure the scene background color
    g_pScene->m_Color.m_R = 0.0f;
    g_pScene->m_Color.m_G = 0.0f;
    g_pScene->m_Color.m_B = 0.0f;
    g_pScene->m_Color.m_A = 1.0f;

    // configure the scene ground direction
    g_pScene->m_GroundDir.m_X =  0.0f;
    g_pScene->m_GroundDir.m_Y = -1.0f;
    g_pScene->m_GroundDir.m_Z =  0.0f;

    // configure the scene view matrix
    csrMat4Identity(&g_pScene->m_ViewMatrix);

    // configure the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnGetShader     = OnGetShader;
    g_SceneContext.m_fOnGetID         = OnGetID;
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
    g_pShader->m_VertexSlot   = glGetAttribLocation(g_pShader->m_ProgramID,  "csr_aVertices");
    g_pShader->m_ColorSlot    = glGetAttribLocation(g_pShader->m_ProgramID,  "csr_aColor");
    g_pShader->m_TexCoordSlot = glGetAttribLocation(g_pShader->m_ProgramID,  "csr_aTexCoord");
    g_pShader->m_TextureSlot  = glGetAttribLocation(g_pShader->m_ProgramID,  "csr_sColorMap");
    g_TexAlphaSlot            = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uAlpha");

    // compile, link and use flame shader
    g_pFlameShader = csrOpenGLShaderLoadFromStr(&g_VSColorAlpha[0],
                                                 sizeof(g_VSColorAlpha),
                                                &g_FSColorAlpha[0],
                                                 sizeof(g_FSColorAlpha),
                                                 0,
                                                 0);

    // succeeded?
    if (!g_pFlameShader)
    {
        // show the error message to the user
        printf("Failed to load the flame shader.\n");
        return;
    }

    csrShaderEnable(g_pFlameShader);

    // get flame shader attributes
    g_pFlameShader->m_VertexSlot = glGetAttribLocation(g_pFlameShader->m_ProgramID,  "csr_aVertices");
    g_pFlameShader->m_ColorSlot  = glGetAttribLocation(g_pFlameShader->m_ProgramID,  "csr_aColor");
    g_AlphaSlot                  = glGetUniformLocation(g_pFlameShader->m_ProgramID, "csr_uAlpha");

    csrShaderEnable(g_pShader);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasPerVertexColor = 1;
    vertexFormat.m_HasTexCoords      = 1;

    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 1;
    material.m_Wireframe   = 0;

    // IMPORTANT this will map the model with the correct texture resource
    g_TextureIndex = 0;

    // create the spaceship
    pMDL = csrMDLOpen(SPACESHIP_MODEL,
                      0,
                     &vertexFormat,
                      0,
                     &material,
                      0,
                      OnApplySkin,
                      OnDeleteTexture);

    g_Spaceship.m_pKey = pMDL;

    // build the spaceship matrix
    BuildSpaceshipMatrix(0.0f);

    // add the model to the scene
    pSceneItem = csrSceneAddMDL(g_pScene, pMDL, 1, 0);
    csrSceneAddModelMatrix(g_pScene, pMDL, &g_Spaceship.m_Matrix);

    // configure spaceship bounding sphere
    g_SpaceshipBoundingCircle.m_Center.m_X = 0.0f;
    g_SpaceshipBoundingCircle.m_Center.m_Y = 0.0f;
    g_SpaceshipBoundingCircle.m_Radius     = 0.15f;

    material.m_Color       = 0x00B4FFFF;
    material.m_Transparent = 1;

    // build the rear flame model
    g_pFlame = csrShapeCreateCylinder(0.001f,
                                      0.025f,
                                      0.125f,
                                      10,
                                     &vertexFormat,
                                      0,
                                     &material,
                                      0);

    // initialize the rear flame model matrix
    BuildFlameMatrix(0.0f);

    // add the model to the scene
    pSceneItem = csrSceneAddMesh(g_pScene, g_pFlame, 1, 0);
    csrSceneAddModelMatrix(g_pScene, g_pFlame, &g_FlameMatrix);

    material.m_Color = 0xFF1122FF;

    // build the laser model
    g_pLaser = csrShapeCreateCylinder(0.01f,
                                      0.01f,
                                      10.0f,
                                      10,
                                     &vertexFormat,
                                      0,
                                     &material,
                                      0);

    // initialize the laser model matrix
    BuildLaserMatrix(0.0f);

    // add the model to the scene
    pSceneItem = csrSceneAddMesh(g_pScene, g_pLaser, 1, 0);
    csrSceneAddModelMatrix(g_pScene, g_pLaser, &g_LaserMatrix);

    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;

    // create the background surface
    pMesh = csrShapeCreateSurface(11.5f,
                                  11.5f,
                                 &vertexFormat,
                                  0,
                                 &material,
                                  0);

    // initialize background matrix
    csrMat4Identity(&g_Background);
    g_Background.m_Table[3][2] = -5.0f;

    // add the mesh to the scene
    pSceneItem = csrSceneAddMesh(g_pScene, pMesh, 0, 0);
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_Background);

    // load background texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(BACKGROUND_TEXTURE);
    g_ID[2].m_pKey     = &pMesh->m_Skin.m_Texture;
    g_ID[2].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[2].m_UseCount = 1;

    // background texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // create the mesh to use for particles
    g_pStar = csrShapeCreateDisk(0.0f,
                                 0.0f,
                                 0.01f,
                                 6,
                                &vertexFormat,
                                 0,
                                &material,
                                 0);

    // add the mesh to the scene
    pSceneItem = csrSceneAddMesh(g_pScene, g_pStar, 0, 0);

    g_Aspect = view_w / view_h;

    // configure the star box
    g_StarBox.m_Min.m_X = -0.5f * 4.0f * g_Aspect;
    g_StarBox.m_Min.m_Y = -0.5f * 4.0f;
    g_StarBox.m_Min.m_Z = -4.0f;
    g_StarBox.m_Max.m_X =  0.5f * 4.0f * g_Aspect;
    g_StarBox.m_Max.m_Y =  0.5f * 4.0f;
    g_StarBox.m_Max.m_Z = -2.0f;

    g_pStars = csrParticlesCreate();
    g_pStars->m_fOnCalculateMotion = OnCalculateStarMotion;

    // iterate through the particles to create
    for (i = 0; i < STAR_COUNT; ++i)
    {
        // add a new particle, ignore the mass
        CSR_Particle* pParticle    = csrParticlesAdd(g_pStars);
        pParticle->m_pBody->m_Mass = 0.0f;

        // calculate the particle start position
        const float x = g_StarBox.m_Min.m_X + (((float)(rand() % (int)400.0f)) * 0.01f * g_Aspect);
        const float y = g_StarBox.m_Min.m_Y + (((float)(rand() % (int)400.0f)) * 0.01f);
        const float z = g_StarBox.m_Min.m_Z + (((float)(rand() % (int)200.0f)) * 0.01f);

        // calculate the particle initial force
        pParticle->m_pBody->m_Velocity.m_X =  0.0f;
        pParticle->m_pBody->m_Velocity.m_Y = -0.5f;
        pParticle->m_pBody->m_Velocity.m_Z =  0.0f;

        // configure the particle matrix (was set to identity while the particle was created)
        pParticle->m_pMatrix->m_Table[3][0] = x;
        pParticle->m_pMatrix->m_Table[3][1] = y;
        pParticle->m_pMatrix->m_Table[3][2] = z;

        // add it to the scene
        pSceneItem = csrSceneAddModelMatrix(g_pScene, g_pStar, pParticle->m_pMatrix);

        // keep the particle model key
        pParticle->m_pKey = pSceneItem->m_pModel;
    }

    // load star texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(STAR_TEXTURE);
    g_ID[3].m_pKey     = &g_pStar->m_Skin.m_Texture;
    g_ID[3].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[3].m_UseCount = 1;

    // star texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // scene should contain meteores?
    if (METEORE_COUNT)
    {
        material.m_Transparent = 0;

        // IMPORTANT this will map the model with the correct texture resource
        g_TextureIndex = 1;

        // create the meteore
        g_pMeteore = csrMDLOpen(METEORE_MODEL,
                                0,
                               &vertexFormat,
                                0,
                               &material,
                                0,
                                OnApplySkin,
                                OnDeleteTexture);

        // add the model to the scene
        pSceneItem = csrSceneAddMDL(g_pScene, g_pMeteore, 0, 0);

        g_pMeteores = csrParticlesCreate();
        g_pMeteores->m_fOnCalculateMotion = OnCalculateMeteoreMotion;

        // iterate through the particles to create
        for (i = 0; i < METEORE_COUNT; ++i)
        {
            float rotationAngle;

            // add a new particle, ignore the mass
            CSR_Particle* pParticle    = csrParticlesAdd(g_pMeteores);
            pParticle->m_pBody->m_Mass = i; // NOTE here use the mass as particle index

            // calculate the particle start position
            const float x =  g_StarBox.m_Min.m_X + (((float)(rand() % (int)400.0f)) * 0.01f * g_Aspect);
            const float y =  g_StarBox.m_Min.m_Y + (((float)(rand() % (int)400.0f)) * 0.01f);
            const float z = -0.1f;

            // calculate the particle initial force
            pParticle->m_pBody->m_Velocity.m_X =  0.0f;
            pParticle->m_pBody->m_Velocity.m_Y = -0.5f;
            pParticle->m_pBody->m_Velocity.m_Z =  0.0f;

            // configure the particle matrix (was set to identity while the particle was created)
            pParticle->m_pMatrix->m_Table[3][0] = x;
            pParticle->m_pMatrix->m_Table[3][1] = y;
            pParticle->m_pMatrix->m_Table[3][2] = z;

            // keep the meteore start position
            g_Meteores[i].m_StartPos.m_X = x;
            g_Meteores[i].m_StartPos.m_Y = g_StarBox.m_Min.m_Y;
            g_Meteores[i].m_StartPos.m_Z = z;

            // set meteore rotation and scaling
            rotationAngle                        = ((float)(rand() % (int)628.0f)) * 0.01f;
            g_Meteores[i].m_Rotation.m_X         = sinf(rotationAngle);
            g_Meteores[i].m_Rotation.m_Y         = cosf(rotationAngle);
            g_Meteores[i].m_Rotation.m_Z         = cosf(rotationAngle);
            g_Meteores[i].m_RotationVelocity.m_X = ((float)(rand() % (int)100.0f)) * 0.01f;
            g_Meteores[i].m_RotationVelocity.m_Y = ((float)(rand() % (int)100.0f)) * 0.01f;
            g_Meteores[i].m_RotationVelocity.m_Z = ((float)(rand() % (int)100.0f)) * 0.01f;
            g_Meteores[i].m_Scaling.m_X          = 0.1f;
            g_Meteores[i].m_Scaling.m_Y          = 0.1f;
            g_Meteores[i].m_Scaling.m_Z          = 0.1f;

            // add it to the scene
            pSceneItem = csrSceneAddModelMatrix(g_pScene, g_pMeteore, pParticle->m_pMatrix);

            // keep the particle model key
            pParticle->m_pKey = pSceneItem->m_pModel;
        }

        g_MeteoreBoundingCircle.m_Radius = 0.125f;
    }

    csrSoundInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // load the sound files
    g_pFireSound      = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, FIRE_SOUND);
    g_pExplosionSound = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, EXPLOSION_SOUND);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the particle systems
    csrParticlesRelease(g_pStars);
    csrParticlesRelease(g_pMeteores);

    // delete the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);
    g_pScene = 0;

    // delete scene shader
    csrOpenGLShaderRelease(g_pShader);
    g_pShader = 0;

    // delete flame shader
    csrOpenGLShaderRelease(g_pFlameShader);
    g_pShader = 0;

    // stop running sounds, if needed
    csrSoundStop(g_pFireSound);
    csrSoundStop(g_pExplosionSound);

    // release OpenAL interface
    csrSoundRelease(g_pFireSound);
    csrSoundRelease(g_pExplosionSound);
    csrSoundReleaseOpenAL(g_pOpenALDevice, g_pOpenALContext);
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    g_Aspect = view_w / view_h;

    g_ScreenOrigin.m_X = view_w * 0.5f;
    g_ScreenOrigin.m_Y = view_h * 0.5f;

    glViewport(0, 0, view_w, view_h);
    CreateViewport(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // animate the star particles
    csrParticlesAnimate(g_pStars, timeStep_sec);

    // animate the meteore particles
    csrParticlesAnimate(g_pMeteores, timeStep_sec);

    // rebuild the spaceship matrix
    BuildSpaceshipMatrix(g_Angle);

    // rebuild the flame matrix
    BuildFlameMatrix(g_Angle);

    // initialize the laser model matrix
    BuildLaserMatrix(g_Angle);

    // calculate next alpha value
    g_Alpha += g_AlphaOffset * timeStep_sec;

    // is alpha out of bounds?
    if (g_Alpha < g_MinAlpha)
    {
        g_Alpha       =  g_MinAlpha;
        g_AlphaOffset = -g_AlphaOffset;
    }
    else
    if (g_Alpha > g_MaxAlpha)
    {
        g_Alpha       =  g_MaxAlpha;
        g_AlphaOffset = -g_AlphaOffset;
    }

    if (g_LaserVisible)
    {
        // calculate laser show elapsed time
        g_LaserTime += (timeStep_sec * 1000.0f);

        // count frames
        if (g_LaserTime > g_LaserInterval)
        {
            g_LaserTime    = 0.0f;
            g_LaserVisible = 0;
        }
    }

    if (g_ShipDying)
    {
        g_ShipAlpha -= timeStep_sec * 0.5f;

        if (g_ShipAlpha <= 0.0f)
        {
            g_ShipAlpha = 1.0f;
            g_ShipDying = 0;
        }
    }
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
    CSR_Vector3 touchDir;
    CSR_Vector3 touchDirN;
    CSR_Vector3 touchRef;
    CSR_Vector2 laserPos;
    CSR_Vector2 laserDir;
    float       angle;

    if (g_ShipDying)
        return;

    touchDir.m_X = x - g_ScreenOrigin.m_X;
    touchDir.m_Y = y - g_ScreenOrigin.m_Y;
    touchDir.m_Z = 0.0f;
    csrVec3Normalize(&touchDir, &touchDirN);

    touchRef.m_X = 1.0f;
    touchRef.m_Y = 0.0f;
    touchRef.m_Z = 0.0f;

    csrVec3Dot(&touchDirN, &touchRef, &angle);

    g_Angle = acosf(angle) + (M_PI * 0.5f);

    if (touchDir.m_Y > 0.0f)
        g_Angle = (M_PI * 2.0f) - (g_Angle + M_PI);

    g_Angle += M_PI;

    laserPos.m_X =  0.0f;
    laserPos.m_X =  0.0f;
    laserDir.m_X = -sinf(g_Angle);
    laserDir.m_Y =  cosf(g_Angle);

    // compute laser ray
    csrRay2FromPointDir(&laserPos, &laserDir, &g_LaserRay);

    // play laser sound
    if (g_pFireSound)
        csrSoundPlay(g_pFireSound);

    // show laser
    g_LaserVisible = 1;
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{}
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
