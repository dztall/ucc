/****************************************************************************
 * ==> DirectX model demo --------------------------------------------------*
 ****************************************************************************
 * Description : A DirectX model showing an animated sparrow                *
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// opengl
#include <gles2.h>
#include <gles2ext.h>

// compactStar engine
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_X.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"

// NOTE the x model was get from the DirectX SDK samples
//#define X_FILE "Resources/tiny_4anim.x"
#define X_FILE "Resources/flying_sparrow.x"

// libraries
#include <ccr.h>

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
CSR_OpenGLShader* g_pShader         = 0;
CSR_Scene*        g_pScene          = 0;
CSR_X*            g_pModel          = 0;
float             g_ScreenWidth     = 0.0f;
float             g_Angle           = 0.0f;
float             g_ElapsedTime     = 0.0f;
CSR_Matrix4       g_ModelMatrix;
CSR_SceneContext  g_SceneContext;
CSR_OpenGLID      g_ID[1];
//---------------------------------------------------------------------------
CSR_PixelBuffer* OnLoadTexture(const char* pTextureName)
{
    char* pFileName = (char*)malloc(10 + strlen(pTextureName));
    strcpy(pFileName,      "Resources/\0");
    strcpy(pFileName + 10, pTextureName);

    return csrPixelBufferFromBitmapFile(pFileName);
}
//---------------------------------------------------------------------------
void OnGetXIndex(const CSR_X* pX, size_t* pAnimSetIndex, size_t* pFrameIndex)
{
    *pAnimSetIndex = 0;
    *pFrameIndex   = (*pFrameIndex + (size_t)(g_ElapsedTime * 10.0f)) % 11;
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
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    return g_pShader;
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
void CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
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
    CSR_VertexFormat  vf;
    CSR_VertexCulling vc;

    // create the default scene
    g_pScene = csrSceneCreate();

    // create the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnSceneBegin = OnSceneBegin;
    g_SceneContext.m_fOnSceneEnd   = OnSceneEnd;
    g_SceneContext.m_fOnGetShader  = OnGetShader;
    g_SceneContext.m_fOnGetID      = OnGetID;
    g_SceneContext.m_fOnGetXIndex  = OnGetXIndex;

    // configure the scene color background
    g_pScene->m_Color.m_R = 0.08f;
    g_pScene->m_Color.m_G = 0.12f;
    g_pScene->m_Color.m_B = 0.17f;
    g_pScene->m_Color.m_A = 1.0f;

    // initialize the matrices
    csrMat4Identity(&g_pScene->m_ProjectionMatrix);
    csrMat4Identity(&g_pScene->m_ViewMatrix);
    csrMat4Identity(&g_ModelMatrix);

    // get the screen width
    g_ScreenWidth = view_w;

    // compile, link and use shader
    g_pShader = csrOpenGLShaderLoadFromStr(&g_VSTextured[0],
                                            sizeof(g_VSTextured),
                                           &g_FSTextured[0],
                                            sizeof(g_FSTextured),
                                            0,
                                            0);
    csrShaderEnable(g_pShader);

    // configure the shader slots
    g_pShader->m_VertexSlot   = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_ColorSlot    = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aColor");
    g_pShader->m_TexCoordSlot = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aTexCoord");
    g_pShader->m_TextureSlot  = glGetAttribLocation(g_pShader->m_ProgramID, "csr_sTexture");

    CreateViewport(view_w, view_h);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // configure the vertex format
    vf.m_HasNormal         = 0;
    vf.m_HasTexCoords      = 1;
    vf.m_HasPerVertexColor = 1;

    vc.m_Type = CSR_CT_Back;
    vc.m_Face = CSR_CF_CCW;

    // load the X model
    g_pModel = csrXOpen(X_FILE, &vf, &vc, 0, 0, 0, 0, OnLoadTexture, OnApplySkin, OnDeleteTexture);

    // add it to the scene
    csrSceneAddX(g_pScene, g_pModel, 0, 0);
    csrSceneAddModelMatrix(g_pScene, g_pModel, &g_ModelMatrix);

    CSR_Matrix4 rotMat;

    CSR_Vector3 axis;
    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;

    csrMat4Rotate(0.0f, &axis, &rotMat);

    CSR_Matrix4 scaleMat;
    csrMat4Identity(&scaleMat);
    scaleMat.m_Table[0][0] = 0.1f;
    scaleMat.m_Table[1][1] = 0.1f;
    scaleMat.m_Table[2][2] = 0.1f;

    csrMat4Multiply(&scaleMat, &rotMat, &g_ModelMatrix);

    // initialize elapsed time
    g_ElapsedTime = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // release the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);

    // delete shader program
    csrOpenGLShaderRelease(g_pShader);
    g_pShader = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    // get the screen width
    g_ScreenWidth = view_w;

    glViewport(0, 0, view_w, view_h);
    CreateViewport(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // create a point of view from an arcball
    CSR_ArcBall arcball;
    arcball.m_AngleX = 0.0f;
    arcball.m_AngleY = g_Angle;
    arcball.m_Radius = 80.0f;
    csrSceneArcBallToMatrix(&arcball, &g_pScene->m_ViewMatrix);

    // rotate the view around the model
    g_Angle = fmod(g_Angle + timeStep_sec, M_PI * 2);

    g_ElapsedTime += timeStep_sec;
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
