/*****************************************************************************
 * ==> Transparency demo ----------------------------------------------------*
 *****************************************************************************
 * Description : A transparency effect, swipe left/right to increase or      *
 *               decrease the rotation speed, swipe up/down to increase or   *
 *               decrease the transparency                                   *
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

#define GLASS_TEXTURE_FILE "Resources/frosted_glass.bmp"
#define CLOUD_TEXTURE_FILE "Resources/cloud.bmp"

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader        g_Shader;
GLuint             g_ShaderProgram      = 0;
float*             g_pSphereVB          = 0;
unsigned int       g_SphereVertexCount  = 0;
MINI_Index*        g_pSphereIndexes     = 0;
unsigned int       g_SphereIndexCount   = 0;
float*             g_pSurfaceVB         = 0;
unsigned int       g_SurfaceVertexCount = 0;
const float        g_SurfaceWidth       = 20.0f;
const float        g_SurfaceHeight      = 20.0f;
const float        g_SphereRadius       = 1.0f;
float              g_Angle              = 0.0f;
float              g_RotationSpeed      = 0.1f;
float              g_AlphaLevel         = 0.5f;
float              g_Time               = 0.0f;
float              g_Interval           = 0.0f;
const unsigned int g_FPS                = 15;
GLuint             g_GlassTextureIndex  = GL_INVALID_VALUE;
GLuint             g_CloudTextureIndex  = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot     = 0;
GLuint             g_AlphaSlot          = 0;
GLuint             g_ModelviewUniform   = 0;
MINI_VertexFormat  g_VertexFormat;
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
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram,  "qr_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram,  "qr_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram,  "qr_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram,  "qr_sColorMap");
    g_AlphaSlot             = glGetUniformLocation(g_ShaderProgram, "qr_uAlpha");
    g_ModelviewUniform      = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // generate surface
    miniCreateSurface(&g_SurfaceWidth,
                      &g_SurfaceHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);

    // generate sphere
    miniCreateSphere(&g_SphereRadius,
                     10,
                     24,
                     0xFFFFFFFF,
                     &g_VertexFormat,
                     &g_pSphereVB,
                     &g_SphereVertexCount,
                     &g_pSphereIndexes,
                     &g_SphereIndexCount);

    // load textures
    g_GlassTextureIndex = miniLoadTexture(GLASS_TEXTURE_FILE);
    g_CloudTextureIndex = miniLoadTexture(CLOUD_TEXTURE_FILE);

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete sphere buffer index table
    if (g_pSphereIndexes)
    {
        free(g_pSphereIndexes);
        g_pSphereIndexes = 0;
    }

    // delete sphere vertices
    if (g_pSphereVB)
    {
        free(g_pSphereVB);
        g_pSphereVB = 0;
    }

    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    if (g_GlassTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_GlassTextureIndex);

    g_GlassTextureIndex = GL_INVALID_VALUE;

    if (g_CloudTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_CloudTextureIndex);

    g_CloudTextureIndex = GL_INVALID_VALUE;

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
    unsigned int frameCount = 0;

    // calculate next time
    g_Time += (timeStep_sec * 1000.0f);

    // count frames to skip
    while (g_Time > g_Interval)
    {
        g_Time -= g_Interval;
        ++frameCount;
    }

    // calculate next rotation angle
    g_Angle += (g_RotationSpeed * frameCount);

    // is rotating angle out of bounds?
    while (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    float        xAngle;
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  xRotateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure OpenGL to draw opaque objects (NOTE all opaque object should be drawn before,
    // because depth test should be deactivated later to allow alpha blending to work correctly)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -10.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(g_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // bind cloud texture
    glBindTexture(GL_TEXTURE_2D, g_CloudTextureIndex);

    // set alpha transparency level to draw surface (NOTE surface doesn't use transparency)
    glUniform1f(g_AlphaSlot, 1.0f);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // draw background image
    miniDrawSurface(g_pSurfaceVB,
                    g_SurfaceVertexCount,
                    &g_VertexFormat,
                    &g_Shader);

    // bind glass texture
    glBindTexture(GL_TEXTURE_2D, g_GlassTextureIndex);

    // populate sphere translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 90 degrees
    xAngle = 1.57075;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&g_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&xRotateMatrix,   &yRotateMatrix,   &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(g_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // set alpha transparency level to draw sphere
    glUniform1f(g_AlphaSlot, g_AlphaLevel);

    // configure OpenGL to draw transparency (NOTE all opaque objects should be drawn before,
    // because depth test should be disabled to allow alpha blending to work correctly)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw the transparent sphere
    miniDrawSphere(g_pSphereVB,
                   g_SphereVertexCount,
                   g_pSphereIndexes,
                   g_SphereIndexCount,
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
{
    // increase or decrease rotation speed
    g_RotationSpeed += (x - prev_x) * 0.001f;

    // increase or decrease alpha level
    g_AlphaLevel += (y - prev_y) * -0.001f;

    // limit alpha level to min and max bounds
    if (g_AlphaLevel > 1.0f)
        g_AlphaLevel = 1.0f;
    else
    if (g_AlphaLevel < 0.0f)
        g_AlphaLevel = 0.0f;
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
