/****************************************************************************
 * ==> Wild soccer game demo -----------------------------------------------*
 ****************************************************************************
 * Description : A wild soccer game demo. Swipe up or down to walk, and     *
 *               left or right to rotate. Tap to shoot the ball             *
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
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_Physics.h"
#include "SDK/CSR_Sound.h"
#include "SDK/CSR_MobileC_Debug.h"

#include <ccr.h>

#define YOU_WON_TEXTURE_FILE       "Resources/you_won.bmp"
#define LANDSCAPE_TEXTURE_FILE     "Resources/soccer_grass.bmp"
#define BALL_TEXTURE_FILE          "Resources/soccer_ball.bmp"
#define SOCCER_GOAL_TEXTURE_FILE   "Resources/soccer_goal.bmp"
#define SOCCER_GOAL_MODEL          "Resources/soccer_goal.obj"
#define LANDSCAPE_DATA_FILE        "Resources/level.bmp"
#define SKYBOX_LEFT                "Resources/skybox_left_small.bmp"
#define SKYBOX_TOP                 "Resources/skybox_top_small.bmp"
#define SKYBOX_RIGHT               "Resources/skybox_right_small.bmp"
#define SKYBOX_BOTTOM              "Resources/skybox_bottom_small.bmp"
#define SKYBOX_FRONT               "Resources/skybox_front_small.bmp"
#define SKYBOX_BACK                "Resources/skybox_back_small.bmp"
#define FOOT_STEP_LEFT_SOUND_FILE  "Resources/footstep_left.wav"
#define FOOT_STEP_RIGHT_SOUND_FILE "Resources/footstep_right.wav"
#define BALL_KICK_SOUND_FILE       "Resources/soccer_ball_kick.wav"

// energy factor for the shoot. NOTE should be 23.0 for iPhone 6s and 26.0 for iPhone 11
#define M_ShootEnergyFactor 26.0f

#define TEXTURE_COUNT 5

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
typedef struct
{
    void*       m_pKey;
    CSR_Matrix4 m_Matrix;
    CSR_Sphere  m_Geometry;
    CSR_Body    m_Body;
} CSR_Ball;
//------------------------------------------------------------------------------
typedef struct 
{
    void*       m_pKey;
    CSR_Matrix4 m_Matrix;
    CSR_Rect    m_Bounds;
} CSR_Goal;
//------------------------------------------------------------------------------
CSR_Scene*        g_pScene             = 0;
CSR_OpenGLShader* g_pShader            = 0;
CSR_OpenGLShader* g_pSkyboxShader      = 0;
void*             g_pLandscapeKey      = 0;
float             g_Angle              = M_PI / -4.0f;
float             g_RollAngle          = 0.0f;
float             g_BallDirAngle       = 0.0f;
float             g_BallOffset         = 0.0f;
float             g_StepTime           = 0.0f;
float             g_StepInterval       = 300.0f;
const float       g_PosVelocity        = 10.0f;
const float       g_DirVelocity        = 30.0f;
const float       g_ControlRadius      = 40.0f;
int               g_AlternateStep      = 0;
CSR_Ball          g_Ball;
CSR_Goal          g_Goal;
CSR_SceneContext  g_SceneContext;
CSR_Sphere        g_ViewSphere;
CSR_Matrix4       g_LandscapeMatrix;
CSR_Matrix4       g_YouWonMatrix;
CSR_Vector2       g_TouchOrigin;
CSR_Vector2       g_TouchPosition;
ALCdevice*        g_pOpenALDevice       = 0;
ALCcontext*       g_pOpenALContext      = 0;
CSR_Sound*        g_pFootStepLeftSound  = 0;
CSR_Sound*        g_pFootStepRightSound = 0;
CSR_Sound*        g_pBallKickSound      = 0;
CSR_OpenGLID      g_ID[TEXTURE_COUNT];
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    if (pModel == g_pScene->m_pSkybox)
        return g_pSkyboxShader;

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
//---------------------------------------------------------------------------
int LoadLandscapeFromBitmap(const char* fileName, CSR_fOnDeleteTexture fOnDeleteTexture)
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
        csrModelRelease(pModel, fOnDeleteTexture);
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
int CheckForGoal(CSR_Ball* pBall, const CSR_Vector3* pOldPos, const CSR_Vector3* pDir)
{
    if (!pBall || !pDir)
        return 0;

    if (!pDir->m_X && !pDir->m_Y && !pDir->m_Z)
        return 0;

    // is ball hitting the goal?
    if (pBall->m_Geometry.m_Center.m_X >= g_Goal.m_Bounds.m_Min.m_X &&
        pBall->m_Geometry.m_Center.m_X <= g_Goal.m_Bounds.m_Max.m_X &&
        pBall->m_Geometry.m_Center.m_Z >= g_Goal.m_Bounds.m_Min.m_Y &&
        pBall->m_Geometry.m_Center.m_Z <= g_Goal.m_Bounds.m_Max.m_Y)
    {
        // player hit the goal
        // a       b
        // |-------|
        // |       |
        // |       |
        // |       |
        // |       |
        // |       |
        // |       |
        // |       |
        // |-------|
        // d       c
        CSR_Segment3 ab;
        CSR_Segment3 bc;
        CSR_Segment3 cd;
        CSR_Segment3 da;

        // build the ab segment
        ab.m_Start.m_X = g_Goal.m_Bounds.m_Min.m_X;
        ab.m_Start.m_Y = 0.0f;
        ab.m_Start.m_Z = g_Goal.m_Bounds.m_Min.m_Y;
        ab.m_End.m_X   = g_Goal.m_Bounds.m_Max.m_X;
        ab.m_End.m_Y   = 0.0f;
        ab.m_End.m_Z   = g_Goal.m_Bounds.m_Min.m_Y;

        // build the bc segment
        bc.m_Start.m_X = g_Goal.m_Bounds.m_Max.m_X;
        bc.m_Start.m_Y = 0.0f;
        bc.m_Start.m_Z = g_Goal.m_Bounds.m_Min.m_Y;
        bc.m_End.m_X   = g_Goal.m_Bounds.m_Max.m_X;
        bc.m_End.m_Y   = 0.0f;
        bc.m_End.m_Z   = g_Goal.m_Bounds.m_Max.m_Y;

        // build the cd segment
        cd.m_Start.m_X = g_Goal.m_Bounds.m_Max.m_X;
        cd.m_Start.m_Y = 0.0f;
        cd.m_Start.m_Z = g_Goal.m_Bounds.m_Max.m_Y;
        cd.m_End.m_X   = g_Goal.m_Bounds.m_Min.m_X;
        cd.m_End.m_Y   = 0.0f;
        cd.m_End.m_Z   = g_Goal.m_Bounds.m_Max.m_Y;

        // build the da segment
        da.m_Start.m_X = g_Goal.m_Bounds.m_Min.m_X;
        da.m_Start.m_Y = 0.0f;
        da.m_Start.m_Z = g_Goal.m_Bounds.m_Max.m_Y;
        da.m_End.m_X   = g_Goal.m_Bounds.m_Min.m_X;
        da.m_End.m_Y   = 0.0f;
        da.m_End.m_Z   = g_Goal.m_Bounds.m_Min.m_Y;

        CSR_Vector3 ptAB;
        CSR_Vector3 ptBC;
        CSR_Vector3 ptCD;
        CSR_Vector3 ptDA;

        // calculate the closest point from previous position to each of the segments
        csrSeg3ClosestPoint(&ab, pOldPos, &ptAB);
        csrSeg3ClosestPoint(&bc, pOldPos, &ptBC);
        csrSeg3ClosestPoint(&cd, pOldPos, &ptCD);
        csrSeg3ClosestPoint(&da, pOldPos, &ptDA);

        CSR_Vector3 PPtAB;
        CSR_Vector3 PPtBC;
        CSR_Vector3 PPtCD;
        CSR_Vector3 PPtDA;

        // calculate each distances between the previous point and each points found on segments
        csrVec3Sub(&ptAB, pOldPos, &PPtAB);
        csrVec3Sub(&ptBC, pOldPos, &PPtBC);
        csrVec3Sub(&ptCD, pOldPos, &PPtCD);
        csrVec3Sub(&ptDA, pOldPos, &PPtDA);

        float lab;
        float lbc;
        float lcd;
        float lda;

        // calculate each lengths between the previous point and each points found on segments
        csrVec3Length(&PPtAB, &lab);
        csrVec3Length(&PPtBC, &lbc);
        csrVec3Length(&PPtCD, &lcd);
        csrVec3Length(&PPtDA, &lda);

        // find on which side the player is hitting the goal
        if (lab < lbc && lab < lcd && lab < lda)
            pBall->m_Body.m_Velocity.m_Z = -pBall->m_Body.m_Velocity.m_Z;
        else
        if (lbc < lab && lbc < lcd && lbc < lda)
            pBall->m_Body.m_Velocity.m_X = -pBall->m_Body.m_Velocity.m_X;
        else
        if (lcd < lab && lcd < lbc && lcd < lda)
        {
            pBall->m_Body.m_Velocity.m_Z = -pBall->m_Body.m_Velocity.m_Z;
            return 1;
        }
        else
        if (lda < lab && lda < lbc && lda < lcd)
            pBall->m_Body.m_Velocity.m_X = -pBall->m_Body.m_Velocity.m_X;
        else
        {
            pBall->m_Body.m_Velocity.m_X = -pBall->m_Body.m_Velocity.m_X;
            pBall->m_Body.m_Velocity.m_Z = -pBall->m_Body.m_Velocity.m_Z;
        }
    }

    return 0;
}
//---------------------------------------------------------------------------
int ApplyGroundCollision(const CSR_Sphere*  pBoundingSphere,
                               float        dir,
                               CSR_Matrix4* pMatrix,
                               CSR_Plane*   pGroundPlane)
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
    camera.m_yAngle       =  dir;
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

    CSR_Matrix4 invertMatrix;
    float       determinant;

    // calculate the current camera position above the landscape
    csrMat4Inverse(pMatrix, &invertMatrix, &determinant);
    csrMat4Transform(&invertMatrix, &modelCenter, &collisionInput.m_BoundingSphere.m_Center);
    collisionInput.m_CheckPos = collisionInput.m_BoundingSphere.m_Center;

    CSR_CollisionOutput collisionOutput;

    // calculate the collisions in the whole scene
    csrSceneDetectCollision(g_pScene, &collisionInput, &collisionOutput, 0);

    // update the ground position directly inside the matrix (this is where the final value is required)
    pMatrix->m_Table[3][1] = -collisionOutput.m_GroundPos;

    // get the resulting plane
    *pGroundPlane = collisionOutput.m_GroundPlane;

    if (collisionOutput.m_Collision & CSR_CO_Ground)
        return 1;

    return 0;
}
//---------------------------------------------------------------------------
void ApplyPhysics(float elapsedTime)
{
    CSR_Plane   groundPlane;
    CSR_Vector3 planeNormal;
    CSR_Vector3 prevCenter;
    CSR_Vector3 ballDir;
    CSR_Vector3 ballDirN;

    // apply the ground collision on the current position and get the ground polygon
    ApplyGroundCollision(&g_Ball.m_Geometry, 0.0f, &g_Ball.m_Matrix, &groundPlane);

    // get the normal of the plane
    planeNormal.m_X = groundPlane.m_A;
    planeNormal.m_Y = groundPlane.m_B;
    planeNormal.m_Z = groundPlane.m_C;

    // calculate the next ball roll position
    csrPhysicsRoll(&planeNormal, g_Ball.m_Body.m_Mass, 0.005f, elapsedTime, &g_Ball.m_Body.m_Velocity);

    // keep the previous ball position
    prevCenter = g_Ball.m_Geometry.m_Center;

    // calculate the new position ( using the formula pos = pos + (v * dt))
    g_Ball.m_Geometry.m_Center.m_X += g_Ball.m_Body.m_Velocity.m_X * elapsedTime;
    g_Ball.m_Geometry.m_Center.m_Y += g_Ball.m_Body.m_Velocity.m_Y * elapsedTime;
    g_Ball.m_Geometry.m_Center.m_Z += g_Ball.m_Body.m_Velocity.m_Z * elapsedTime;

    // check if the new position is valid
    if (!ApplyGroundCollision(&g_Ball.m_Geometry, 0.0f, &g_Ball.m_Matrix, &groundPlane))
    {
        // do perform a rebound on the x or z axis?
        const int xRebound = g_Ball.m_Geometry.m_Center.m_X <= -3.08f || g_Ball.m_Geometry.m_Center.m_X >= 3.08f;
        const int zRebound = g_Ball.m_Geometry.m_Center.m_Z <= -3.08f || g_Ball.m_Geometry.m_Center.m_Z >= 3.08f;

        // reset the ball to the previous position
        g_Ball.m_Geometry.m_Center = prevCenter;

        // do perform a rebound on the x axis?
        if (xRebound)
            g_Ball.m_Body.m_Velocity.m_X = -g_Ball.m_Body.m_Velocity.m_X;

        // do perform a rebound on the z axis?
        if (zRebound)
            g_Ball.m_Body.m_Velocity.m_Z = -g_Ball.m_Body.m_Velocity.m_Z;
    }
    else
    {
        float       distance;
        CSR_Matrix4 rxMatrix;
        CSR_Matrix4 ryMatrix;
        CSR_Matrix4 ballMatrix;
        CSR_Vector3 rollDistance;
        CSR_Vector3 rollDir;
        CSR_Vector3 axis;

        // calculate the rolling angle (depends on the distance the ball moved)
        csrVec3Sub(&g_Ball.m_Geometry.m_Center, &prevCenter, &rollDistance);
        csrVec3Length(&rollDistance, &distance);
        g_RollAngle = fmod(g_RollAngle + (distance * 10.0f), M_PI * 2.0f);

        axis.m_X = 1.0f;
        axis.m_Y = 0.0f;
        axis.m_Z = 0.0f;

        // the ball moved since the last frame?
        if (distance)
        {
            // calculate the new ball direction angle
            csrVec3Normalize(&rollDistance, &rollDir);
            csrVec3Dot(&rollDir, &axis, &g_BallDirAngle);
        }

        // calculate the rotation matrix on the x axis
        csrMat4Rotate(g_RollAngle, &axis, &rxMatrix);

        axis.m_X = 0.0f;
        axis.m_Y = 1.0f;
        axis.m_Z = 0.0f;

        // ball moved on the z axis since the last frame?
        if (rollDistance.m_Z)
            // calculate the offset to apply to the ball direction
            g_BallOffset = (rollDistance.m_Z > 0.0f ? 1.0f : -1.0f);

        // calculate the rotation matrix on the y axis
        csrMat4Rotate((M_PI * 2.0f) - ((acos(g_BallDirAngle) * g_BallOffset) - (M_PI / 2.0f)),
                      &axis,
                      &ryMatrix);

        // build the final matrix
        csrMat4Multiply(&rxMatrix, &ryMatrix, &ballMatrix);

        // replace the ball in the model coordinate system (do that directly on the matrix)
        ballMatrix.m_Table[3][0] = -g_Ball.m_Matrix.m_Table[3][0];
        ballMatrix.m_Table[3][1] = -g_Ball.m_Matrix.m_Table[3][1];
        ballMatrix.m_Table[3][2] = -g_Ball.m_Matrix.m_Table[3][2];

        g_Ball.m_Matrix = ballMatrix;
    }

    // calculate the ball direction
    csrVec3Sub(&g_Ball.m_Geometry.m_Center, &prevCenter, &ballDir);
    csrVec3Normalize(&ballDir, &ballDirN);

    // check if the goal was hit
    if (CheckForGoal(&g_Ball, &prevCenter, &ballDirN))
        g_YouWonMatrix.m_Table[3][1] = 1.375f;
}
//---------------------------------------------------------------------------
void Shoot()
{
    CSR_Circle  playerCircle;
    CSR_Circle  ballCircle;
    CSR_Figure2 f1;
    CSR_Figure2 f2;

    // get the player position as a circle (i.e. ignore the y axis)
    playerCircle.m_Center.m_X = g_ViewSphere.m_Center.m_X;
    playerCircle.m_Center.m_Y = g_ViewSphere.m_Center.m_Z;
    playerCircle.m_Radius     = g_ViewSphere.m_Radius + 0.15f;

    // get the ball position as a circle (i.e. ignore the y axis)
    ballCircle.m_Center.m_X = g_Ball.m_Geometry.m_Center.m_X;
    ballCircle.m_Center.m_Y = g_Ball.m_Geometry.m_Center.m_Z;
    ballCircle.m_Radius     = g_Ball.m_Geometry.m_Radius;

    f1.m_Type    = CSR_F2_Circle;
    f1.m_pFigure = &playerCircle;

    f2.m_Type    = CSR_F2_Circle;
    f2.m_pFigure = &ballCircle;

    // check if the player is closer enough to the ball to shoot it
    if (csrIntersect2(&f1, &f2, 0, 0))
    {
        // calculate the direction and intensity of the shoot
        CSR_Vector2 distance;
        csrVec2Sub(&ballCircle.m_Center, &playerCircle.m_Center, &distance);

        // shoot the ball
        g_Ball.m_Body.m_Velocity.m_X = M_ShootEnergyFactor * distance.m_X;
        g_Ball.m_Body.m_Velocity.m_Y = 0.0f;
        g_Ball.m_Body.m_Velocity.m_Z = M_ShootEnergyFactor * distance.m_Y;

        // play the kick sound
        csrSoundStop(g_pBallKickSound);
        csrSoundPlay(g_pBallKickSound);
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
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_VertexFormat vertexFormat;
    CSR_Material     material;
    CSR_PixelBuffer* pPixelBuffer = 0;
    CSR_Mesh*        pMesh;
    CSR_SceneItem*   pSceneItem;

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
    g_ViewSphere.m_Center.m_X = 3.08f;
    g_ViewSphere.m_Center.m_Y = 0.0f;
    g_ViewSphere.m_Center.m_Z = 3.08f;
    g_ViewSphere.m_Radius     = 0.1f;

    // set the ball bounding sphere default position
    g_Ball.m_Geometry.m_Center.m_X = 0.0f;
    g_Ball.m_Geometry.m_Center.m_Y = 0.0f;
    g_Ball.m_Geometry.m_Center.m_Z = 0.0f;
    g_Ball.m_Geometry.m_Radius     = 0.025f;
    csrBodyInit(&g_Ball.m_Body);

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
    g_pShader->m_VertexSlot   = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_ColorSlot    = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aColor");
    g_pShader->m_TexCoordSlot = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aTexCoord");
    g_pShader->m_TextureSlot  = glGetAttribLocation(g_pShader->m_ProgramID, "csr_sColorMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // load the landscape
    if (!LoadLandscapeFromBitmap(LANDSCAPE_DATA_FILE, OnDeleteTexture))
    {
        // show the error message to the user
        printf("The landscape could not be loaded.\n");
        return;
    }

    // get back the scene item containing the model
    pSceneItem = csrSceneGetItem(g_pScene, g_pLandscapeKey);

    // found it?
    if (!pSceneItem)
    {
        // show the error message to the user
        printf("The landscape was not found in the scene.\n");
        return;
    }

    // load landscape texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(LANDSCAPE_TEXTURE_FILE);
    g_ID[0].m_pKey     = &((CSR_Model*)(pSceneItem->m_pModel))->m_pMesh[0].m_Skin.m_Texture;
    g_ID[0].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[0].m_UseCount = 1;

    // landscape texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasPerVertexColor = 1;
    vertexFormat.m_HasTexCoords      = 1;

    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;
    material.m_Wireframe   = 0;

    // create the ball
    pMesh = csrShapeCreateSphere(g_Ball.m_Geometry.m_Radius, 
                                 20,
                                 20,
                                &vertexFormat,
                                 0,
                                &material,
                                 0);

    // load ball texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(BALL_TEXTURE_FILE);
    g_ID[1].m_pKey     = &pMesh->m_Skin.m_Texture;
    g_ID[1].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[1].m_UseCount = 1;

    // ball texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // add the mesh to the scene
    pSceneItem = csrSceneAddMesh(g_pScene, pMesh, 0, 1);
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_Ball.m_Matrix);

    // configure the ball particle
    g_Ball.m_pKey        = pSceneItem->m_pModel;
    g_Ball.m_Body.m_Mass = 0.3f;

    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasPerVertexColor = 1;
    vertexFormat.m_HasTexCoords      = 1;

    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;
    material.m_Wireframe   = 0;

    // create the goal
    CSR_Model* pModel = csrWaveFrontOpen(SOCCER_GOAL_MODEL,
                                        &vertexFormat,
                                         0,
                                        &material,
                                         0,
                                         0,
                                         0);

    CSR_Vector3 translation;
    translation.m_X =  0.0f;
    translation.m_Y =  1.375f;
    translation.m_Z = -1.75f;

    CSR_Matrix4 translationMatrix;

    // apply translation to goal
    csrMat4Translate(&translation, &translationMatrix);

    CSR_Vector3 axis;
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    CSR_Matrix4 ryMatrix;

    // apply rotation on y axis to goal
    csrMat4Rotate(M_PI, &axis, &ryMatrix);

    CSR_Vector3 factor;
    factor.m_X = 0.0025f;
    factor.m_Y = 0.0025f;
    factor.m_Z = 0.0025f;

    CSR_Matrix4 scaleMatrix;

    // apply scaling to goal
    csrMat4Scale(&factor, &scaleMatrix);

    CSR_Matrix4 buildMatrix;

    // build the goal model matrix
    csrMat4Multiply(&ryMatrix, &scaleMatrix, &buildMatrix);
    csrMat4Multiply(&buildMatrix, &translationMatrix, &g_Goal.m_Matrix);

    // add the model to the scene
    pSceneItem = csrSceneAddModel(g_pScene, pModel, 0, 1);
    csrSceneAddModelMatrix(g_pScene, pModel, &g_Goal.m_Matrix);

    // load goal texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(SOCCER_GOAL_TEXTURE_FILE);
    g_ID[2].m_pKey     = &pMesh->m_Skin.m_Texture;
    g_ID[2].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[2].m_UseCount = 1;

    // goal texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    CSR_Box goalBox;

    // transform the goal bounding box in his local system coordinates
    csrMat4ApplyToVector(&g_Goal.m_Matrix, &pSceneItem->m_pAABBTree[0].m_pBox->m_Min, &goalBox.m_Min);
    csrMat4ApplyToVector(&g_Goal.m_Matrix, &pSceneItem->m_pAABBTree[0].m_pBox->m_Max, &goalBox.m_Max);

    // configure the goal
    g_Goal.m_pKey = pSceneItem->m_pModel;
    csrMathMin(goalBox.m_Min.m_X, goalBox.m_Max.m_X, &g_Goal.m_Bounds.m_Min.m_X);
    csrMathMin(goalBox.m_Min.m_Z, goalBox.m_Max.m_Z, &g_Goal.m_Bounds.m_Min.m_Y);
    csrMathMax(goalBox.m_Min.m_X, goalBox.m_Max.m_X, &g_Goal.m_Bounds.m_Max.m_X);
    csrMathMax(goalBox.m_Min.m_Z, goalBox.m_Max.m_Z, &g_Goal.m_Bounds.m_Max.m_Y );

    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasPerVertexColor = 1;
    vertexFormat.m_HasTexCoords      = 1;

    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;
    material.m_Wireframe   = 0;

    // create the You Won surface
    pMesh = csrShapeCreateSurface(0.6f, 0.2f, &vertexFormat, 0, &material, 0);

    // load the You Won texture
    pPixelBuffer       = csrPixelBufferFromBitmapFile(YOU_WON_TEXTURE_FILE);
    g_ID[3].m_pKey     = &pMesh->m_Skin.m_Texture;
    g_ID[3].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[3].m_UseCount = 1;

    // the You Won texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // initialize the You Won matrix
    csrMat4Identity(&g_YouWonMatrix);
    g_YouWonMatrix.m_Table[3][0] =  0.0f;
    g_YouWonMatrix.m_Table[3][1] =  99999.0f;
    g_YouWonMatrix.m_Table[3][2] = -1.65f;

    // add the mesh to the scene
    pSceneItem = csrSceneAddMesh(g_pScene, pMesh, 0, 1);
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_YouWonMatrix);

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
    g_ID[4].m_pKey     = &g_pScene->m_pSkybox->m_Skin.m_CubeMap;
    g_ID[4].m_ID       = csrOpenGLCubemapLoad(pCubemapFileNames);
    g_ID[4].m_UseCount = 1;

    csrSoundInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // load the sound files
    g_pFootStepLeftSound  = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, FOOT_STEP_LEFT_SOUND_FILE);
    g_pFootStepRightSound = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, FOOT_STEP_RIGHT_SOUND_FILE);
    g_pBallKickSound      = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, BALL_KICK_SOUND_FILE);

    // change the volume
    csrSoundChangeVolume(g_pFootStepLeftSound,  0.2f);
    csrSoundChangeVolume(g_pFootStepRightSound, 0.2f);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);
    g_pScene = 0;

    // delete scene shader
    csrOpenGLShaderRelease(g_pShader);
    g_pShader = 0;

    // delete skybox shader
    csrOpenGLShaderRelease(g_pSkyboxShader);
    g_pSkyboxShader = 0;

    // stop running sounds, if needed
    csrSoundStop(g_pFootStepLeftSound);
    csrSoundStop(g_pFootStepRightSound);
    csrSoundStop(g_pBallKickSound);

    // release OpenAL interface
    csrSoundRelease(g_pFootStepLeftSound);
    csrSoundRelease(g_pFootStepRightSound);
    csrSoundRelease(g_pBallKickSound);
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
    float      angle;
    CSR_Sphere prevSphere;
    CSR_Plane  groundPlane;

    ApplyPhysics(timeStep_sec);

    // if screen isn't touched, do nothing
    if (!g_TouchOrigin.m_X || !g_TouchOrigin.m_Y)
        return;

    prevSphere = g_ViewSphere;

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
    g_ViewSphere.m_Center.m_X += posVelocity * cosf(g_Angle + (M_PI * 0.5f)) * timeStep_sec;
    g_ViewSphere.m_Center.m_Z += posVelocity * sinf(g_Angle + (M_PI * 0.5f)) * timeStep_sec;

    // calculate the ground position and check if next position is valid
    if (!ApplyGroundCollision(&g_ViewSphere, g_Angle, &g_pScene->m_ViewMatrix, &groundPlane))
    {
        // invalid next position, get the scene item (just one for this scene)
        const CSR_SceneItem* pItem = csrSceneGetItem(g_pScene, g_pLandscapeKey);

        // found it?
        if (pItem)
        {
            // check if the x position is out of bounds, and correct it if yes
            if (g_ViewSphere.m_Center.m_X <= pItem->m_pAABBTree->m_pBox->m_Min.m_X ||
                g_ViewSphere.m_Center.m_X >= pItem->m_pAABBTree->m_pBox->m_Max.m_X)
                g_ViewSphere.m_Center.m_X = prevSphere.m_Center.m_X;

            // do the same thing with the z position. Doing that separately for each axis will make
            // the point of view to slide against the landscape border (this is possible because the
            // landscape is axis-aligned)
            if (g_ViewSphere.m_Center.m_Z <= pItem->m_pAABBTree->m_pBox->m_Min.m_Z ||
                g_ViewSphere.m_Center.m_Z >= pItem->m_pAABBTree->m_pBox->m_Max.m_Z)
                g_ViewSphere.m_Center.m_Z = prevSphere.m_Center.m_Z;
        }
        else
            // failed to get the scene item, just revert the position
            g_ViewSphere.m_Center = prevSphere.m_Center;

        // recalculate the ground value (this time the collision result isn't tested, because the
        // previous position is always considered as valid)
        ApplyGroundCollision(&g_ViewSphere, g_Angle, &g_pScene->m_ViewMatrix, &groundPlane);
    }
    else
    {
        float       groundAngle;
        CSR_Vector3 slopeDir;

        // get the slope direction
        slopeDir.m_X = groundPlane.m_A;
        slopeDir.m_Y = groundPlane.m_B;
        slopeDir.m_Z = groundPlane.m_C;

        // calculate the slope angle
        csrVec3Dot(&g_pScene->m_GroundDir, &slopeDir, &groundAngle);

        // is the slope too inclined to allow the player to walk on it?
        if (fabs(groundAngle) < 0.5f)
            // revert the position
            g_ViewSphere.m_Center = prevSphere.m_Center;
        else
        if (g_ViewSphere.m_Center.m_X >= g_Goal.m_Bounds.m_Min.m_X && g_ViewSphere.m_Center.m_X <= g_Goal.m_Bounds.m_Max.m_X &&
            g_ViewSphere.m_Center.m_Z >= g_Goal.m_Bounds.m_Min.m_Y && g_ViewSphere.m_Center.m_Z <= g_Goal.m_Bounds.m_Max.m_Y)
        {
            // player hit the goal
            // a       b
            // |-------|
            // |       |
            // |       |
            // |       |
            // |       |
            // |       |
            // |       |
            // |       |
            // |-------|
            // d       c
            CSR_Segment3 ab;
            CSR_Segment3 bc;
            CSR_Segment3 cd;
            CSR_Segment3 da;

            // build the ab segment
            ab.m_Start.m_X = g_Goal.m_Bounds.m_Min.m_X;
            ab.m_Start.m_Y = 0.0f;
            ab.m_Start.m_Z = g_Goal.m_Bounds.m_Min.m_Y;
            ab.m_End.m_X   = g_Goal.m_Bounds.m_Max.m_X;
            ab.m_End.m_Y   = 0.0f;
            ab.m_End.m_Z   = g_Goal.m_Bounds.m_Min.m_Y;

            // build the bc segment
            bc.m_Start.m_X = g_Goal.m_Bounds.m_Max.m_X;
            bc.m_Start.m_Y = 0.0f;
            bc.m_Start.m_Z = g_Goal.m_Bounds.m_Min.m_Y;
            bc.m_End.m_X   = g_Goal.m_Bounds.m_Max.m_X;
            bc.m_End.m_Y   = 0.0f;
            bc.m_End.m_Z   = g_Goal.m_Bounds.m_Max.m_Y;

            // build the cd segment
            cd.m_Start.m_X = g_Goal.m_Bounds.m_Max.m_X;
            cd.m_Start.m_Y = 0.0f;
            cd.m_Start.m_Z = g_Goal.m_Bounds.m_Max.m_Y;
            cd.m_End.m_X   = g_Goal.m_Bounds.m_Min.m_X;
            cd.m_End.m_Y   = 0.0f;
            cd.m_End.m_Z   = g_Goal.m_Bounds.m_Max.m_Y;

            // build the da segment
            da.m_Start.m_X = g_Goal.m_Bounds.m_Min.m_X;
            da.m_Start.m_Y = 0.0f;
            da.m_Start.m_Z = g_Goal.m_Bounds.m_Max.m_Y;
            da.m_End.m_X   = g_Goal.m_Bounds.m_Min.m_X;
            da.m_End.m_Y   = 0.0f;
            da.m_End.m_Z   = g_Goal.m_Bounds.m_Min.m_Y;

            CSR_Vector3 ptAB;
            CSR_Vector3 ptBC;
            CSR_Vector3 ptCD;
            CSR_Vector3 ptDA;

            // calculate the closest point from previous position to each of the segments
            csrSeg3ClosestPoint(&ab, &prevSphere.m_Center, &ptAB);
            csrSeg3ClosestPoint(&bc, &prevSphere.m_Center, &ptBC);
            csrSeg3ClosestPoint(&cd, &prevSphere.m_Center, &ptCD);
            csrSeg3ClosestPoint(&da, &prevSphere.m_Center, &ptDA);

            CSR_Vector3 PPtAB;
            CSR_Vector3 PPtBC;
            CSR_Vector3 PPtCD;
            CSR_Vector3 PPtDA;

            // calculate each distances between the previous point and each points found on segments
            csrVec3Sub(&ptAB, &prevSphere.m_Center, &PPtAB);
            csrVec3Sub(&ptBC, &prevSphere.m_Center, &PPtBC);
            csrVec3Sub(&ptCD, &prevSphere.m_Center, &PPtCD);
            csrVec3Sub(&ptDA, &prevSphere.m_Center, &PPtDA);

            float lab;
            float lbc;
            float lcd;
            float lda;

            // calculate each lengths between the previous point and each points found on segments
            csrVec3Length(&PPtAB, &lab);
            csrVec3Length(&PPtBC, &lbc);
            csrVec3Length(&PPtCD, &lcd);
            csrVec3Length(&PPtDA, &lda);

            // find on which side the player is hitting the goal
            if (lab < lbc && lab < lcd && lab < lda)
                g_ViewSphere.m_Center.m_Z = prevSphere.m_Center.m_Z;
            else
            if (lbc < lab && lbc < lcd && lbc < lda)
                g_ViewSphere.m_Center.m_X = prevSphere.m_Center.m_X;
            else
            if (lcd < lab && lcd < lbc && lcd < lda)
                g_ViewSphere.m_Center.m_Z = prevSphere.m_Center.m_Z;
            else
            if (lda < lab && lda < lbc && lda < lcd)
                g_ViewSphere.m_Center.m_X = prevSphere.m_Center.m_X;
            else
            {
                g_ViewSphere.m_Center.m_X = prevSphere.m_Center.m_X;
                g_ViewSphere.m_Center.m_Z = prevSphere.m_Center.m_Z;
            }

            // recalculate the ground value (this time the collision result isn't tested, because the
            // previous position is always considered as valid)
            ApplyGroundCollision(&g_ViewSphere, g_Angle, &g_pScene->m_ViewMatrix, &groundPlane);
        }
    }

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
    CSR_Plane groundPlane;

    // finalize the view matrix
    ApplyGroundCollision(&g_ViewSphere, g_Angle, &g_pScene->m_ViewMatrix, &groundPlane);

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
    if (g_TouchPosition.m_X == g_TouchOrigin.m_X && g_TouchPosition.m_Y == g_TouchOrigin.m_Y)
        Shoot();

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
