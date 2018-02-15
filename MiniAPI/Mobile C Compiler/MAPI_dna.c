/*****************************************************************************
 * ==> MAPI_dna -------------------------------------------------------------*
 *****************************************************************************
 * Description : A dna spiral                                                *
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
#include "MiniAPI/MiniCommon.h"
#include "MiniAPI/MiniGeometry.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniShapes.h"
#include "MiniAPI/MiniShader.h"
#include "MiniAPI/MiniRenderer.h"

#define DNA_TEXTURE_FILE "Resources/texture_dna.bmp"

#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    #include <ccr.h>
#endif

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader        g_Shader;
GLuint             g_ShaderProgram  = 0;
float*             g_pVertices      = 0;
unsigned           g_VertexCount    = 0;
MINI_Index*        g_pIndexes       = 0;
unsigned           g_IndexCount     = 0;
float              g_Pos            = 0.0f;
float              g_Velocity       = 5.0f;
float              g_Angle          = 2.0f * M_PI;
GLuint             g_TextureIndex   = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot = 0;
GLuint             g_AlphaSlot      = 0;
MINI_VertexFormat  g_VertexFormat;
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
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    // renderer buffers should no more be generated since CCR version 1.1
    #if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
        #ifndef _OS_ANDROID_
            // generate and bind in memory frame buffers to render to
            glGenRenderbuffers(1, &g_Renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, g_Renderbuffer);
            glGenFramebuffers(1,&g_Framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_RENDERBUFFER,
                                      g_Renderbuffer);
        #endif
    #endif

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSTexAlpha(), miniGetFSTexAlpha());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram,  "mini_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram,  "mini_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram,  "mini_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram,  "mini_sColorMap");
    g_AlphaSlot             = glGetUniformLocation(g_ShaderProgram, "mini_uAlpha");

    // disable OpenGL depth testing
    glDisable(GL_DEPTH_TEST);

    // disable culling
    glDisable(GL_CULL_FACE);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // configure the vertex format for the spiral shape
    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;
    miniCalculateStride(&g_VertexFormat);

    // create the spiral shape
    miniCreateSpiral(0.0f,
                     0.0f,
                     1.0f,
                     2.2f,
                     0.0f,
                     0.0f,
                     g_Velocity / 36.0f,
                     25,
                     36,
                     0xFFFFFFFF,
                     0x404040FF,
                    &g_VertexFormat,
                    &g_pVertices,
                    &g_VertexCount,
                    &g_pIndexes,
                    &g_IndexCount);

    // load dna texture
    g_TextureIndex = miniLoadTexture(DNA_TEXTURE_FILE);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete indices
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertices
    if (g_pVertices)
    {
        free(g_pVertices);
        g_pVertices = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    ApplyMatrix(view_w, view_h);
    glViewport(0, 0, view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // rotate the spiral
    g_Angle -= 0.5f * timeStep_sec;

    // is angle out of boumds?
    if (g_Angle < 0.0f)
        g_Angle += 2.0f * M_PI;

    // calculate the camera movement to follow the spiral
    g_Pos = (g_Angle * g_Velocity) / (2.0f * M_PI);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrix;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // populate translation vector
    t.m_X = -0.5f;
    t.m_Y =  0.0f;
    t.m_Z = -(-7.5f + g_Pos);

    // get translation matrix
    miniGetTranslateMatrix(&t, &translateMatrix);

    // populate rotation vector
    r.m_X = 0.0f;
    r.m_Y = 0.0f;
    r.m_Z = 1.0f;

    // get rotation matrix
    miniGetRotateMatrix(&g_Angle, &r, &rotateMatrix);

    // build the final view matrix
    miniMatrixMultiply(&rotateMatrix, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // set alpha transparency level to draw sphere
    glUniform1f(g_AlphaSlot, 0.75f);

    // draw the spiral
    miniDrawSpiral(g_pVertices,
                   g_VertexCount,
                   g_pIndexes,
                   g_IndexCount,
                  &g_VertexFormat,
                  &g_Shader);

    miniEndScene();
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
