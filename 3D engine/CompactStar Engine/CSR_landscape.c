/*****************************************************************************
 * ==> Landscape and ground collision demo ----------------------------------*
 *****************************************************************************
 * Description : A landscape generator based on a grayscale image, with      *
 *               ground collision to walk on his surface. Swipe up or down   *
 *               to walk, and left or right to rotate                        *
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
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Geometry.h"
#include "SDK/CSR_Collision.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Shader.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_Sound.h"

#include <ccr.h>

#define LANDSCAPE_TEXTURE_FILE "Resources/grass.bmp"
#define LANDSCAPE_DATA_FILE    "Resources/the_face.bmp"
#define PLAYER_STEP_SOUND_FILE "Resources/human_walk_grass_step.wav"

//----------------------------------------------------------------------------
const char g_VSTextured[] =
    "precision mediump float;"
    "attribute vec4 csr_aVertices;"
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
    "    gl_Position   = csr_uProjection * csr_uView * csr_uModel * csr_aVertices;"
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
CSR_Shader*   g_pShader       = 0;
CSR_Mesh*     g_pMesh         = 0;
CSR_AABBNode* g_pTree         = 0;
float         g_MapHeight     = 3.0f;
float         g_MapScale      = 0.2f;
float         g_Angle         = 0.0f;
float         g_RotationSpeed = 0.02f;
float         g_StepTime      = 0.0f;
float         g_StepInterval  = 350.0f;
const float   g_PosVelocity   = 10.0f;
const float   g_DirVelocity   = 30.0f;
const float   g_ControlRadius = 40.0f;
CSR_Sphere    g_BoundingSphere;
CSR_Matrix4   g_ViewMatrix;
CSR_Matrix4   g_ModelMatrix;
CSR_Vector2   g_TouchOrigin;
CSR_Vector2   g_TouchPosition;
CSR_Color     g_Color;
ALCdevice*    g_pOpenALDevice  = 0;
ALCcontext*   g_pOpenALContext = 0;
CSR_Sound*    g_pSound         = 0;
//---------------------------------------------------------------------------
void ApplyGroundCollision(const CSR_Sphere*   pBoundingSphere,
                          const CSR_AABBNode* pTree,
                                CSR_Matrix4*  pMatrix)
{
    // validate the inputs
    if (!pBoundingSphere || !pTree || !pMatrix)
        return;

    CSR_Sphere transformedSphere = *pBoundingSphere;
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

    CSR_Vector3 groundDir;

    // get the ground direction
    groundDir.m_X =  0.0f;
    groundDir.m_Y = -1.0f;
    groundDir.m_Z =  0.0f;

    CSR_Vector3 modelCenter;

    // get the model center
    modelCenter.m_X = 0.0f;
    modelCenter.m_Y = 0.0f;
    modelCenter.m_Z = 0.0f;

    CSR_Matrix4 invertView;
    float       determinant;

    // calculate the current camera position above the landscape
    csrMat4Inverse(pMatrix, &invertView, &determinant);
    csrMat4Transform(&invertView, &modelCenter, &transformedSphere.m_Center);

    float posY = -transformedSphere.m_Center.m_Y;

    // calculate the y position where to place the point of view
    csrGroundPosY(&transformedSphere, pTree, &groundDir, 0, &posY);

    // update the ground position inside the view matrix
    pMatrix->m_Table[3][1] = -posY;
}
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.01f;
    const float zFar   = 100.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    CSR_Matrix4 matrix;
    csrMat4Perspective(fov, aspect, zNear, zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    unsigned         playerStepSoundFileLen;
    unsigned         playerFireSoundFileLen;
    unsigned char*   pPlayerStepSndBuffer;
    CSR_VertexFormat vertexFormat;
    CSR_Material     material;
    CSR_PixelBuffer* pPixelBuffer = 0;

    g_Color.m_R = 0.1f;
    g_Color.m_G = 0.65f;
    g_Color.m_B = 0.9f;
    g_Color.m_A = 1.0f;

    // set the viewpoint bounding sphere default position
    g_BoundingSphere.m_Center.m_X = 0.0f;
    g_BoundingSphere.m_Center.m_Y = 0.0f;
    g_BoundingSphere.m_Center.m_Z = 0.0f;
    g_BoundingSphere.m_Radius     = 0.1f;

    // compile, link and use shader
    g_pShader = csrShaderLoadFromStr(&g_VSTextured,
                                      sizeof(g_VSTextured),
                                     &g_FSTextured,
                                      sizeof(g_FSTextured),
                                      0,
                                      0);

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

    // load landscape data from grayscale image model
    pPixelBuffer = csrPixelBufferFromBitmap(LANDSCAPE_DATA_FILE);

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    // configure the material
    material.m_Color       = 0xFFFFFFFF;
    material.m_Transparent = 0;

    // generate landscape
    g_pMesh = csrLandscapeCreate(pPixelBuffer,
                                 g_MapHeight,
                                 g_MapScale,
                                &vertexFormat,
                                 0,
                                &material,
                                 0);

    // landscape image data will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // create the AABB tree for the mountain model
    g_pTree = csrAABBTreeFromMesh(g_pMesh);

    // load landscape texture
    pPixelBuffer                  = csrPixelBufferFromBitmap(LANDSCAPE_TEXTURE_FILE);
    g_pMesh->m_Shader.m_TextureID = csrTextureFromPixelBuffer(pPixelBuffer);

    // landscape texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    csrSoundInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // load step sound file
    g_pSound = csrSoundOpen(g_pOpenALDevice, g_pOpenALContext, PLAYER_STEP_SOUND_FILE, 44100);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the landscape
    csrMeshRelease(g_pMesh);
    g_pMesh = 0;

    // delete shader
    csrShaderRelease(g_pShader);
    g_pShader = 0;

    // stop running step sound, if needed
    csrSoundStop(g_pSound);

    // release OpenAL interface
    csrSoundRelease(g_pSound);
    csrSoundReleaseOpenAL(g_pOpenALDevice, g_pOpenALContext);
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
    ApplyMatrix(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    float angle;

    // if screen isn't touched, do nothing
    if (!g_TouchOrigin.m_X || !g_TouchOrigin.m_Y)
        return;

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

    // calculate next time where the step sound should be played
    g_StepTime += (timeStep_sec * 1000.0f);

    // count frames
    while (g_StepTime > g_StepInterval)
    {
        csrSoundStop(g_pSound);
        csrSoundPlay(g_pSound);
        g_StepTime = 0.0f;
    }
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    csrDrawBegin(&g_Color);

    ApplyGroundCollision(&g_BoundingSphere, g_pTree, &g_ViewMatrix);

    // connect the view matrix to shader
    const GLint viewSlot = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uView");
    glUniformMatrix4fv(viewSlot, 1, 0, &g_ViewMatrix.m_Table[0][0]);

    // build model view matrix
    csrMat4Identity(&g_ModelMatrix);

    // connect model view matrix to shader
    const GLint modelSlot = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uModel");
    glUniformMatrix4fv(modelSlot, 1, 0, &g_ModelMatrix.m_Table[0][0]);

    // draw the landscape
    csrDrawMesh(g_pMesh, g_pShader, 0);

    csrDrawEnd();
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
