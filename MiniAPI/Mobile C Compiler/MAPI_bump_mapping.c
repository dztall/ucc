/*****************************************************************************
 * ==> Bump mapping demo ----------------------------------------------------*
 *****************************************************************************
 * Description : A bump mapped stone wall with diffuse light, swipe on       *
 *               screen to modify the light position                         *
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

// NOTE the texture was found here: http://opengameart.org/content/stone-texture-bump
#define STONE_TEXTURE_FILE "Resources/stone.bmp"
#define STONE_BUMPMAP_FILE "Resources/stone_bump.bmp"

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram      = 0;
float*            g_pSurfaceVB         = 0;
unsigned          g_SurfaceVertexCount = 0;
const float       g_SurfaceWidth       = 10.0f;
const float       g_SurfaceHeight      = 12.5f;
GLuint            g_TextureIndex       = GL_INVALID_VALUE;
GLuint            g_BumpMapIndex       = GL_INVALID_VALUE;
GLuint            g_LightPos           = 0;
GLuint            g_TexSamplerSlot     = 0;
GLuint            g_BumpMapSamplerSlot = 0;
MINI_Size         g_View;
MINI_VertexFormat g_VertexFormat;
//------------------------------------------------------------------------------
const char* g_pVSDiffuseBumpMap =
    "precision mediump float;"
    "attribute vec4 qr_vPosition;"
    "attribute vec4 qr_vColor;"
    "attribute vec2 qr_vTexCoord;"
    "uniform   vec3 qr_vLightPos;"
    "uniform   mat4 qr_uProjection;"
    "uniform   mat4 qr_uModelview;"
    "varying   vec4 qr_fColor;"
    "varying   vec2 qr_fTexCoord;"
    "varying   vec3 qr_fLightPos;"
    "void main(void)"
    "{"
    "    qr_fColor    = qr_vColor;"
    "    qr_fTexCoord = qr_vTexCoord;"
    "    qr_fLightPos = qr_vLightPos;"
    "    gl_Position  = qr_uProjection * qr_uModelview * qr_vPosition;"
    "}";
//------------------------------------------------------------------------------
// NOTE this shader was written on the base of the following article:
// http://www.swiftless.com/tutorials/glsl/8_bump_mapping.html
const char* g_pFSDiffuseBumpMap =
    "precision mediump float;"
    "uniform sampler2D qr_sColorMap;"
    "uniform sampler2D qr_sBumpMap;"
    "varying lowp vec4 qr_fColor;"
    "varying      vec2 qr_fTexCoord;"
    "varying      vec3 qr_fLightPos;"
    "void main(void)"
    "{"
    "    vec3  normal  = normalize(texture2D(qr_sBumpMap, qr_fTexCoord).rgb * 2.0 - 1.0);"
    "    float diffuse = clamp(dot(normal, qr_fLightPos), 0.0, 2.5);"
    "    vec3  color   = diffuse * texture2D(qr_sColorMap, qr_fTexCoord).rgb;"
    "    gl_FragColor  = vec4(color, 1.0);"
    "}";
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
    g_ShaderProgram = miniCompileShaders(g_pVSDiffuseBumpMap, g_pFSDiffuseBumpMap);
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram,  "qr_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram,  "qr_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram,  "qr_vTexCoord");
    g_LightPos              = glGetUniformLocation(g_ShaderProgram, "qr_vLightPos");
    g_TexSamplerSlot        = glGetUniformLocation(g_ShaderProgram, "qr_sColorMap");
    g_BumpMapSamplerSlot    = glGetUniformLocation(g_ShaderProgram, "qr_sBumpMap");

    // notify shader about default light position
    glUniform3f(g_LightPos, 0.0f, 0.0f, 2.0f);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

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

    // load wall texture and his bump map
    g_TextureIndex = miniLoadTexture(STONE_TEXTURE_FILE);
    g_BumpMapIndex = miniLoadTexture(STONE_BUMPMAP_FILE);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    if (g_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_TextureIndex);

    g_TextureIndex = GL_INVALID_VALUE;

    if (g_BumpMapIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_BumpMapIndex);

    g_BumpMapIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    // get view size
    g_View.m_Width  = view_w;
    g_View.m_Height = view_h;

    glViewport(0, 0, view_w, view_h);
    ApplyMatrix(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    int          stride;
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // calculate vertex stride
    stride = g_VertexFormat.m_Stride;

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -15.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // enable OpenGL texturing engine
    glEnable(GL_TEXTURE_2D);

    // connect texture to shader
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, 0);
    glBindTexture(GL_TEXTURE_2D, g_TextureIndex);

    // connect bump map to shader
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(g_BumpMapSamplerSlot, 1);
    glBindTexture(GL_TEXTURE_2D, g_BumpMapIndex);

    // draw the bump mapped image
    miniDrawSurface(g_pSurfaceVB,
                    g_SurfaceVertexCount,
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
    const float maxX = 2.0f;
    const float maxY = 2.0f;

    // convert screen coordinates to light world coordinate and notify shader about new light position
    glUniform3f(g_LightPos,
                ((x * maxX) / g_View.m_Width) - 1.0f,
                1.0f - ((y * maxY) / g_View.m_Height),
                2.0f);
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
