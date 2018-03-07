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
#include "SDK/CSR_Shader.h"
#include "SDK/CSR_Renderer.h"

// NOTE the mdl model was extracted from the Quake game package
#define MDL_FILE "Resources/wizard.mdl"

#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    #include <ccr.h>
#endif

//----------------------------------------------------------------------------
const char g_VSTextured[] =
    "precision mediump float;"
    "attribute vec4 mini_vPosition;"
    "attribute vec4 mini_vColor;"
    "attribute vec2 mini_vTexCoord;"
    "uniform   mat4 mini_uProjection;"
    "uniform   mat4 mini_uModelview;"
    "varying   vec4 mini_fColor;"
    "varying   vec2 mini_fTexCoord;"
    "void main(void)"
    "{"
    "    mini_fColor    = mini_vColor;"
    "    mini_fTexCoord = mini_vTexCoord;"
    "    gl_Position    = mini_uProjection * mini_uModelview * mini_vPosition;"
    "}";
//----------------------------------------------------------------------------
const char g_FSTextured[] =
    "precision mediump float;"
    "uniform sampler2D mini_sColorMap;"
    "varying lowp vec4 mini_fColor;"
    "varying      vec2 mini_fTexCoord;"
    "void main(void)"
    "{"
    "    gl_FragColor = mini_fColor * texture2D(mini_sColorMap, mini_fTexCoord);"
    "}";
//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Framebuffer, g_Renderbuffer;
    #endif
#endif
CSR_Shader*    g_pShader         = 0;
CSR_MDL*       g_pModel          = 0;
float          g_ScreenWidth     = 0.0f;
float          g_Time            = 0.0f;
float          g_Interval        = 0.0f;
double         g_TextureLastTime = 0.0;
double         g_ModelLastTime   = 0.0;
double         g_MeshLastTime    = 0.0;
const unsigned g_FPS             = 10;
size_t         g_AnimIndex       = 0;
size_t         g_TextureIndex    = 0;
size_t         g_ModelIndex      = 0;
size_t         g_MeshIndex       = 0;
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
    GLint projectionUniform = glGetUniformLocation(g_pShader->m_ProgramID, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    // renderer buffers should no more be generated since CCR version 1.1
    #if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
        #ifndef _OS_ANDROID_
            // generate and bind in memory frame buffers to render to
            glGenFramebuffers(1, &g_Framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer);

            glGenRenderbuffers(1, &g_Renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, g_Renderbuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_RENDERBUFFER,
                                      g_Renderbuffer);
        #endif
    #endif

    CSR_VertexFormat vertexFormat;
    CSR_Material     material;

    // get the screen width
    g_ScreenWidth = view_w;

    // compile, link and use shader
    g_pShader = csrShaderLoadFromStr(&g_VSTextured,
                                      sizeof(g_VSTextured),
                                     &g_FSTextured,
                                      sizeof(g_FSTextured),
                                      0,
                                      0);
    glUseProgram(g_pShader->m_ProgramID);

    // configure the shader slots
    g_pShader->m_VertexSlot   = glGetAttribLocation(g_pShader->m_ProgramID, "mini_vPosition");
    g_pShader->m_ColorSlot    = glGetAttribLocation(g_pShader->m_ProgramID, "mini_vColor");
    g_pShader->m_TexCoordSlot = glGetAttribLocation(g_pShader->m_ProgramID, "mini_vTexCoord");
    g_pShader->m_TextureSlot  = glGetAttribLocation(g_pShader->m_ProgramID, "mini_sColorMap");

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
    g_pModel = csrMDLOpen(MDL_FILE, 0, &vertexFormat, 0, 0, 0, 0);

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the model
    csrMDLRelease(g_pModel);
    g_pModel = 0;

    // delete shader program
    csrShaderRelease(g_pShader);
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

    csrSceneBegin(0.0f, 0.0f, 0.0f, 1.0f);

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
    modelviewUniform = glGetUniformLocation(g_pShader->m_ProgramID, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the model
    csrSceneDrawMDL(g_pModel, g_pShader, g_TextureIndex, g_ModelIndex, g_MeshIndex);

    csrSceneEnd();
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
