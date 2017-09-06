/*****************************************************************************
 * ==> Quake II (MD2) model demo --------------------------------------------*
 *****************************************************************************
 * Description : A Quake II (MD2) model showing a robot. Tap on the left or  *
 *               right to change the animation                               *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
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

// NOTE the md2 model was found on this site:
// http://leileilol.mancubus.net/garyacordsucks/64.246.6.138/_gcsgames.com/GCSenemy/md2.htm
#define MD2_FILE         "Resources/chip.md2"
#define MD2_TEXTURE_FILE "Resources/chipskin.bmp"

// mini API
#include "MiniAPI/MiniCommon.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniModels.h"
#include "MiniAPI/MiniShader.h"
#include "MiniAPI/MiniRenderer.h"

#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    #include <ccr.h>
#endif

//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} MINI_MD2Animation;
//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Framebuffer, g_Renderbuffer;
    #endif
#endif
MINI_Shader        g_Shader;
MINI_VertexFormat  g_ModelFormat;
MINI_MD2Model*     g_pModel         = 0;
unsigned int       g_MeshIndex      = 0;
GLuint             g_ShaderProgram  = 0;
GLuint             g_TextureIndex   = GL_INVALID_VALUE;
GLuint             g_PositionSlot   = 0;
GLuint             g_ColorSlot      = 0;
GLuint             g_TexCoordSlot   = 0;
GLuint             g_TexSamplerSlot = 0;
float              g_Time           = 0.0f;
float              g_Interval       = 0.0f;
float              g_ScreenWidth    = 0.0f;
const unsigned int g_FPS            = 15;
unsigned int       g_AnimIndex      = 0; // can only be 0 (robot walks) or 1 (robot dies)
MINI_MD2Animation  g_Animation[2];
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 100.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
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

    // get the screen width
    g_ScreenWidth = view_w;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(g_ShaderProgram);

    // configure the shader slots
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram, "qr_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram, "qr_sColorMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    g_ModelFormat.m_UseNormals  = 0;
    g_ModelFormat.m_UseTextures = 1;
    g_ModelFormat.m_UseColors   = 1;

    // load MD2 file and create mesh to draw
    miniLoadMD2Model(MD2_FILE, &g_ModelFormat, 0xFFFFFFFF, &g_pModel);

    // load MD2 texture
    g_TextureIndex = miniLoadTexture(MD2_TEXTURE_FILE);

    // create MD2 animation list
    g_Animation[0].m_Range[0] = 0;  g_Animation[0].m_Range[1] = 20; // robot walks
    g_Animation[1].m_Range[0] = 21; g_Animation[1].m_Range[1] = 29; // robot dies

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    miniReleaseMD2Model(g_pModel);
    g_pModel = 0;

    if (g_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_TextureIndex);

    g_TextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
    {
        glDeleteProgram(g_ShaderProgram);
        g_ShaderProgram = 0;
    }
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
    unsigned int frameCount = 0;
    unsigned int deltaRange = g_Animation[g_AnimIndex].m_Range[1] -
                              g_Animation[g_AnimIndex].m_Range[0];

    // calculate next time
    g_Time += (timeStep_sec * 1000.0f);

    // count frames
    while (g_Time > g_Interval)
    {
        g_Time -= g_Interval;
        ++frameCount;
    }

    // calculate next mesh index to show. Index should always be between animation range
    g_MeshIndex = ((g_MeshIndex + frameCount) % deltaRange);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    MINI_Vector3 t;
    MINI_Vector3 axis;
    MINI_Vector3 factor;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrix;
    MINI_Matrix  scaleMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angle;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -75.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = 0.0f;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrix);

    // set scale factor
    factor.m_X = 0.02f;
    factor.m_Y = 0.02f;
    factor.m_Z = 0.02f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniGetIdentity(&modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &rotateMatrix,    &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // draw the model
    miniDrawMD2(g_pModel,
                &g_Shader,
                (int)(g_Animation[g_AnimIndex].m_Range[0] + g_MeshIndex));

    miniEndScene();
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

        if (g_AnimIndex > 1)
            g_AnimIndex = 0;
    }
    else
    {
        if (g_AnimIndex == 0)
            g_AnimIndex = 2;

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
