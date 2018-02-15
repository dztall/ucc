/*****************************************************************************
 * ==> Saturn demo ----------------------------------------------------------*
 *****************************************************************************
 * Description : A textured sphere representing Saturn, swipe to left or     *
 *               right to increase or decrease the rotation speed            *
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

#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    #include <ccr.h>
#endif

#define SATURN_TEXTURE_FILE "Resources/texture_saturn.bmp"
#define RING_TEXTURE_FILE   "Resources/texture_saturn_ring.bmp"

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram     = 0;
float*            g_pRingVertexBuffer = 0;
unsigned          g_RingVertexCount   = 0;
float*            g_pVertexBuffer     = 0;
unsigned          g_VertexCount       = 0;
MINI_Index*       g_pIndexes          = 0;
unsigned          g_IndexCount        = 0;
float             g_Radius            = 1.0f;
float             g_Angle             = 0.0f;
float             g_RotationSpeed     = 0.1f;
GLuint            g_TextureIndex      = GL_INVALID_VALUE;
GLuint            g_RingTextureIndex  = GL_INVALID_VALUE;
GLuint            g_TexSamplerSlot    = 0;
GLuint            g_AlphaSlot         = 0;
MINI_VertexFormat g_VertexFormat;
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 20.0f;
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

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&g_Radius,
                      20,
                      48,
                      0xFFFFFFFF,
                     &g_VertexFormat,
                     &g_pVertexBuffer,
                     &g_VertexCount,
                     &g_pIndexes,
                     &g_IndexCount);

    // generate ring
    miniCreateRing(0.0f,
                   0.0f,
                   g_Radius + 0.1f,
                   g_Radius + 0.8f,
                   48,
                   0xFFFFFFFF,
                   0xFFFFFFFF,
                  &g_VertexFormat,
                  &g_pRingVertexBuffer,
                  &g_RingVertexCount);

    // load textures
    g_TextureIndex     = miniLoadTexture(SATURN_TEXTURE_FILE);
    g_RingTextureIndex = miniLoadTexture(RING_TEXTURE_FILE);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete ring vertices
    if (g_pRingVertexBuffer)
    {
        free(g_pRingVertexBuffer);
        g_pRingVertexBuffer = 0;
    }

    // delete buffer index table
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertices
    if (g_pVertexBuffer)
    {
        free(g_pVertexBuffer);
        g_pVertexBuffer = 0;
    }

    if (g_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_TextureIndex);

    g_TextureIndex = GL_INVALID_VALUE;

    if (g_RingTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_RingTextureIndex);

    g_RingTextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
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
    // calculate next rotation angle
    g_Angle += (g_RotationSpeed * timeStep_sec * 10.0f);

    // is angle out of bounds?
    while (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    float        xAngle;
    float        yAngle;
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  xRotateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  zRotateMatrix;
    MINI_Matrix  buildMatrix1;
    MINI_Matrix  buildMatrix2;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 45 degrees
    xAngle = -M_PI / 3.0f;

    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    // rotate 30 degrees
    yAngle = M_PI / 12.0f;

    miniGetRotateMatrix(&yAngle, &r, &yRotateMatrix);

    // set rotation on Z axis
    r.m_X = 0.0f;
    r.m_Y = 0.0f;
    r.m_Z = 1.0f;

    miniGetRotateMatrix(&g_Angle, &r, &zRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&zRotateMatrix, &yRotateMatrix,   &buildMatrix1);
    miniMatrixMultiply(&buildMatrix1,  &xRotateMatrix,   &buildMatrix2);
    miniMatrixMultiply(&buildMatrix2,  &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, g_TextureIndex);

    // configure the culling
    glEnable(GL_CULL_FACE);

    // configure OpenGL to draw transparency
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // set alpha transparency level to draw the sphere
    glUniform1f(g_AlphaSlot, 0.65f);

    // draw the sphere
    miniDrawSphere(g_pVertexBuffer,
                   g_VertexCount,
                   g_pIndexes,
                   g_IndexCount,
                   &g_VertexFormat,
                   &g_Shader);

    // prepare OpenGL to draw the ring
    glDisable(GL_CULL_FACE);

    // set alpha transparency level to draw the ring
    glUniform1f(g_AlphaSlot, 0.5f);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, g_RingTextureIndex);

    // draw the ring
    miniDrawRing(g_pRingVertexBuffer, g_RingVertexCount, &g_VertexFormat, &g_Shader);

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
{
    // increase or decrease rotation speed
    g_RotationSpeed += (x - prev_x) * 0.001f;
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
