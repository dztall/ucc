/*****************************************************************************
 * ==> Quake (MDL) model demo -----------------------------------------------*
 *****************************************************************************
 * Description : A Quake (MDL) model showing a wizard. Tap on the left or    *
 *               right to change the animation                               *
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// opengl
#include <gles2.h>
#include <gles2ext.h>

// mini API
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"

// NOTE the mdl model was extracted from the Quake game package
#define MDL_FILE "Resources/wizard.mdl"

// libraries
#include <ccr.h>

//----------------------------------------------------------------------------
const char g_VSTextured[] =
    "precision mediump float;"
    "attribute vec4 csr_aVertices;"
    "attribute vec4 csr_aColor;"
    "attribute vec2 csr_aTexCoord;"
    "uniform   mat4 csr_uProjection;"
    "uniform   mat4 csr_uModelview;"
    "varying   vec4 csr_vColor;"
    "varying   vec2 csr_vTexCoord;"
    "void main(void)"
    "{"
    "    csr_vColor    = csr_aColor;"
    "    csr_vTexCoord = csr_aTexCoord;"
    "    gl_Position   = csr_uProjection * csr_uModelview * csr_aVertices;"
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
CSR_MDL*          g_pModel          = 0;
float             g_ScreenWidth     = 0.0f;
float             g_Time            = 0.0f;
float             g_Interval        = 0.0f;
double            g_TextureLastTime = 0.0;
double            g_ModelLastTime   = 0.0;
double            g_MeshLastTime    = 0.0;
const unsigned    g_FPS             = 10;
size_t            g_AnimIndex       = 0;
size_t            g_TextureIndex    = 0;
size_t            g_ModelIndex      = 0;
size_t            g_MeshIndex       = 0;
CSR_Color         g_Background;
CSR_OpenGLID      g_ID[1];
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
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
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

    // initialize the scene background color
    g_Background.m_R = 0.0f;
    g_Background.m_G = 0.0f;
    g_Background.m_B = 0.0f;
    g_Background.m_A = 1.0f;

    // get the screen width
    g_ScreenWidth = view_w;

    // compile, link and use shader
    g_pShader = csrOpenGLShaderLoadFromStr(&g_VSTextured[0],
                                            sizeof(g_VSTextured),
                                           &g_FSTextured[0],
                                            sizeof(g_FSTextured),
                                            0,
                                            0);
    glUseProgram(g_pShader->m_ProgramID);

    // configure the shader slots
    g_pShader->m_VertexSlot   = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_ColorSlot    = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aColor");
    g_pShader->m_TexCoordSlot = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aTexCoord");
    g_pShader->m_TextureSlot  = glGetAttribLocation(g_pShader->m_ProgramID, "csr_sColorMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 1;

    // load the MDL model
    g_pModel = csrMDLOpen(MDL_FILE, 0, &vertexFormat, 0, 0, 0, OnApplySkin, OnDeleteTexture);

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the model
    csrMDLRelease(g_pModel, OnDeleteTexture);
    g_pModel = 0;

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
    ApplyMatrix(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
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
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    CSR_Vector3 t;
    CSR_Vector3 axis;
    CSR_Vector3 factor;
    CSR_Matrix4 translateMatrix;
    CSR_Matrix4 rotateMatrixX;
    CSR_Matrix4 rotateMatrixY;
    CSR_Matrix4 scaleMatrix;
    CSR_Matrix4 combinedRotMatrix;
    CSR_Matrix4 combinedRotTransMatrix;
    CSR_Matrix4 modelViewMatrix;
    float       angle;
    GLint       modelviewUniform;

    csrDrawBegin(&g_Background);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -150.0f;

    csrMat4Translate(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.5;

    csrMat4Rotate(angle, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.25;

    csrMat4Rotate(angle, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 0.02f;
    factor.m_Y = 0.02f;
    factor.m_Z = 0.02f;

    csrMat4Scale(&factor, &scaleMatrix);

    // calculate model view matrix
    csrMat4Multiply(&rotateMatrixX,          &rotateMatrixY,   &combinedRotMatrix);
    csrMat4Multiply(&combinedRotMatrix,      &translateMatrix, &combinedRotTransMatrix);
    csrMat4Multiply(&combinedRotTransMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the model
    csrDrawMDL(g_pModel, g_pShader, 0, g_TextureIndex, g_ModelIndex, g_MeshIndex, OnGetID);

    csrDrawEnd();
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    if (x > g_ScreenWidth * 0.5f)
    {
        ++g_AnimIndex;

        if (g_AnimIndex >= g_pModel->m_AnimationCount)
            g_AnimIndex = 0;
    }
    else
    {
        if (g_AnimIndex == 0)
            g_AnimIndex = g_pModel->m_AnimationCount;

        --g_AnimIndex;
    }
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
