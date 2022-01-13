/****************************************************************************
 * ==> Landscape game demo -------------------------------------------------*
 ****************************************************************************
 * Description : A base for a game using a landscape and a model            *
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
#include "SDK/CSR_Sound.h"

#include <ccr.h>

// NOTE the mdl model was extracted from the Quake game package
#define MDL_FILE "Resources/wizard.mdl"

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
CSR_OpenGLShader* g_pShader         = 0;
CSR_Mesh*         g_pMesh           = 0;
CSR_AABBNode*     g_pTree           = 0;
CSR_MDL*          g_pModel          = 0;
double            g_TextureLastTime = 0.0;
double            g_ModelLastTime   = 0.0;
double            g_MeshLastTime    = 0.0;
float             g_MapHeight       = 3.0f;
float             g_MapScale        = 0.2f;
float             g_LandscapeAngle  = 0.0f;
float             g_TouchAngle      = 0.0f;
float             g_RotationSpeed   = 0.02f;
float             g_StepTime        = 0.0f;
float             g_StepInterval    = 300.0f;
const float       g_PosVelocity     = 10.0f;
const float       g_DirVelocity     = 10.0f;
const float       g_ControlRadius   = 40.0f;
const unsigned    g_FPS             = 10;
size_t            g_AnimIndex       = 0;
size_t            g_TextureIndex    = 0;
size_t            g_ModelIndex      = 0;
size_t            g_MeshIndex       = 0;
CSR_Sphere        g_BoundingSphere;
CSR_Matrix4       g_ViewMatrix;
CSR_Matrix4       g_ModelMatrix;
CSR_Vector2       g_TouchOrigin;
CSR_Vector2       g_TouchPosition;
CSR_Vector3       g_GroundDir;
CSR_Color         g_Color;
ALCdevice*        g_pOpenALDevice  = 0;
ALCcontext*       g_pOpenALContext = 0;
CSR_Sound*        g_pSound         = 0;
CSR_OpenGLID      g_ID[2];
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    return g_pShader;
}
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
void* OnGetID(const void* pKey)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < 2; ++i)
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
    for (i = 0; i < 2; ++i)
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
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.1f;
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
    CSR_VertexFormat vertexFormat;
    CSR_Material     material;
    CSR_PixelBuffer* pPixelBuffer = 0;

    g_Color.m_R = 0.1f;
    g_Color.m_G = 0.65f;
    g_Color.m_B = 0.9f;
    g_Color.m_A = 1.0f;

    g_GroundDir.m_X =  0.0f;
    g_GroundDir.m_Y = -1.0f;
    g_GroundDir.m_Z =  0.0f;

    // set the viewpoint bounding sphere default position
    g_BoundingSphere.m_Center.m_X = 0.0f;
    g_BoundingSphere.m_Center.m_Y = 0.0f;
    g_BoundingSphere.m_Center.m_Z = 2.5f;
    g_BoundingSphere.m_Radius     = 0.15f;

    // compile, link and use shader
    g_pShader = csrOpenGLShaderLoadFromStr(&g_VSTextured[0],
                                            sizeof(g_VSTextured),
                                           &g_FSTextured[0],
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
    pPixelBuffer = csrPixelBufferFromBitmapFile(LANDSCAPE_DATA_FILE);

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
    pPixelBuffer       = csrPixelBufferFromBitmapFile(LANDSCAPE_TEXTURE_FILE);
    g_ID[0].m_pKey     = &g_pMesh->m_Skin.m_Texture;
    g_ID[0].m_ID       = csrOpenGLTextureFromPixelBuffer(pPixelBuffer);
    g_ID[0].m_UseCount = 1;

    // landscape texture will no longer be used
    csrPixelBufferRelease(pPixelBuffer);

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    // load the MDL model
    g_pModel = csrMDLOpen(MDL_FILE, 0, &vertexFormat, 0, 0, 0, OnApplySkin, OnDeleteTexture);

    csrSoundInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // load step sound file
    g_pSound = csrSoundOpenWavFile(g_pOpenALDevice, g_pOpenALContext, PLAYER_STEP_SOUND_FILE);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the landscape
    csrMeshRelease(g_pMesh, OnDeleteTexture);
    g_pMesh = 0;

    // delete shader
    csrOpenGLShaderRelease(g_pShader);
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

    // calculate next model indexes to show
    csrMDLUpdateIndex(g_pModel,
                      g_FPS,
                      g_AnimIndex,
                     &g_TextureIndex,
                     &g_ModelIndex,
                     &g_MeshIndex,
                     &g_TextureLastTime,
                     &g_ModelLastTime,
                     &g_MeshLastTime,
                      timeStep_sec);

    // if screen isn't touched, do nothing
    if (!g_TouchOrigin.m_X || !g_TouchOrigin.m_Y)
    {
        g_AnimIndex  = 0;
        return;
    }

    if (!(g_TouchPosition.m_X - g_TouchOrigin.m_X) && !(g_TouchPosition.m_Y - g_TouchOrigin.m_Y))
    {
        g_AnimIndex  = 0;
        return;
    }

    g_AnimIndex  = 1;
    //g_TouchAngle = atanf((g_TouchPosition.m_Y - g_TouchOrigin.m_Y) / (g_TouchPosition.m_X - g_TouchOrigin.m_X));

    CSR_Vector3 touchVec;
    touchVec.m_X = g_TouchPosition.m_X - g_TouchOrigin.m_X;
    touchVec.m_Y = g_TouchPosition.m_Y - g_TouchOrigin.m_Y;
    touchVec.m_Z = 0.0f;

    float touchLength;
    csrVec3Length(&touchVec, &touchLength);

    g_TouchAngle = acosf(touchVec.m_X / touchLength);

    if (touchVec.m_Y > 0.0f)
        g_TouchAngle = 0.0f - g_TouchAngle;

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
    g_LandscapeAngle -= dirVelocity * timeStep_sec;

    // validate it
    if (g_LandscapeAngle > M_PI * 2.0f)
        g_LandscapeAngle -= M_PI * 2.0f;
    else
    if (g_LandscapeAngle < 0.0f)
        g_LandscapeAngle += M_PI * 2.0f;

    // calculate the next player position
    g_BoundingSphere.m_Center.m_Z += posVelocity * timeStep_sec;

    csrMathClamp(g_BoundingSphere.m_Center.m_Z, 0.0f, 3.0f, &g_BoundingSphere.m_Center.m_Z);

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
    float       posY;
    float       angle;
    float       determinant;
    CSR_Vector3 t;
    CSR_Vector3 r;
    CSR_Vector3 factor;
    CSR_Matrix4 translateMatrix;
    CSR_Matrix4 rotateMatrixX;
    CSR_Matrix4 rotateMatrixY;
    CSR_Matrix4 rotateMatrixZ;
    CSR_Matrix4 scaleMatrix;
    CSR_Matrix4 combinedRotMatrix;
    CSR_Matrix4 combinedRotTransMatrix;
    CSR_Matrix4 modelViewMatrix;
    CSR_Matrix4 invertMatrix;
    CSR_Sphere  sphere;

    csrDrawBegin(&g_Color);

    // set translation
    t.m_X = 0.0f;
    t.m_Y = 0.0f;
    t.m_Z = 0.0f;

    csrMat4Translate(&t, &translateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    csrMat4Rotate(g_LandscapeAngle, &r, &rotateMatrixY);

    // build model view matrix
    csrMat4Multiply(&rotateMatrixY, &translateMatrix, &g_ModelMatrix);

    // build model view matrix
    csrMat4Identity(&g_ViewMatrix);
    g_ViewMatrix.m_Table[3][1] = -2.0f;
    g_ViewMatrix.m_Table[3][2] = -5.0f;

    // connect the view matrix to shader
    const GLint viewSlot = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uView");
    glUniformMatrix4fv(viewSlot, 1, 0, &g_ViewMatrix.m_Table[0][0]);

    // connect model view matrix to shader
    const GLint modelSlot = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uModel");
    glUniformMatrix4fv(modelSlot, 1, 0, &g_ModelMatrix.m_Table[0][0]);

    // draw the landscape
    csrDrawMesh(g_pMesh, g_pShader, 0, OnGetID);

    // inverse the model matrix
    csrMat4Inverse(&g_ModelMatrix, &invertMatrix, &determinant);

    // put the bounding sphere into the model coordinate system (at the location where the
    // collision should be checked)
    csrMat4Transform(&invertMatrix, &g_BoundingSphere.m_Center, &sphere.m_Center);

    sphere.m_Radius = g_BoundingSphere.m_Radius;
    posY            = M_CSR_NoGround;

    // calculate the y position where to place the point of view
    csrGroundPosY(&sphere, g_pTree, &g_GroundDir, 0, &posY);

    // set translation
    t.m_X = g_BoundingSphere.m_Center.m_X;
    t.m_Y = posY; 
    t.m_Z = g_BoundingSphere.m_Center.m_Z;

    csrMat4Translate(&t, &translateMatrix);

    // set rotation axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.5;

    csrMat4Rotate(angle, &r, &rotateMatrixX);

    // set rotation axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    // set rotation angle
    angle = g_TouchAngle;//-M_PI * 0.25;

    csrMat4Rotate(angle, &r, &rotateMatrixY);

    // set scale factor
    factor.m_X = 0.005f;
    factor.m_Y = 0.005f;
    factor.m_Z = 0.005f;

    csrMat4Scale(&factor, &scaleMatrix);

    // calculate model view matrix
    csrMat4Multiply(&scaleMatrix,            &rotateMatrixX,   &combinedRotMatrix);
    csrMat4Multiply(&combinedRotMatrix,      &rotateMatrixY,   &combinedRotTransMatrix);
    csrMat4Multiply(&combinedRotTransMatrix, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    const GLint modelviewUniform = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uModel");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the model
    csrDrawMDL(g_pModel, g_pShader, 0, g_TextureIndex, g_ModelIndex, g_MeshIndex, OnGetID);

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
