/*****************************************************************************
 * ==> Shader viewer --------------------------------------------------------*
 *****************************************************************************
 * Description : A shader viewer tool                                        *
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

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram      =  0;
float*            g_pSurfaceVB         =  0;
unsigned int      g_SurfaceVertexCount =  0;
const float       g_SurfaceWidth       =  10.0f;
const float       g_SurfaceHeight      =  12.5f;
const float       g_MaxTime            =  12.0f * M_PI;
float             g_Time               =  0.0f;
GLuint            g_TimeSlot           =  0;
GLuint            g_SizeSlot           =  0;
GLuint            g_ResolutionSlot     =  0;
GLuint            g_MouseSlot          =  0;
char              g_ShaderFile[128]    = {0};
unsigned          g_ShaderFileSize     =  0;
MINI_Vector2      g_Resolution;
MINI_Vector2      g_MousePos;
MINI_VertexFormat g_VertexFormat;
//------------------------------------------------------------------------------
// generic vertex shader program
const char* g_pVertexShader =
    "precision mediump float;"
    "attribute vec3  mini_aPosition;"
    "uniform   float mini_uTime;"
    "uniform   vec2  mini_uResolution;"
    "uniform   vec2  mini_uMouse;"
    "varying   float iTime;"
    "varying   vec2  iResolution;"
    "varying   vec2  iMouse;"
    ""
    "void main(void)"
    "{"
    "    iResolution = mini_uResolution;"
    "    iTime       = mini_uTime;"
    "    iMouse      = mini_uMouse;"
    "    gl_Position = vec4(mini_aPosition, 1.0);"
    "}";
//------------------------------------------------------------------------------
void ReadShader(const char* pFileName, long fileSize, char** pShader)
{
    // open shader file
    FILE* pFile = fopen(pFileName, "rb");

    // succeeded?
    if (!pFile)
        return;

    // reserve memory for the shader program
    *pShader = (char*)malloc(fileSize + 1);

    // succeeded?
    if (*pShader)
    {
        // read the shader program from the file
        fread(*pShader, 1, fileSize, pFile);
        (*pShader)[fileSize] = '\0';
    }

    // close the file
    fclose(pFile);
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

    char* pShader = 0;

    // read the shader content
    ReadShader(g_ShaderFile, g_ShaderFileSize, &pShader);

    // failed?
    if (!pShader)
        printf("ERROR - Failed to open the file - %s", g_ShaderFile);

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(g_pVertexShader, pShader);
    glUseProgram(g_ShaderProgram);

    free(pShader);

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram,  "mini_aPosition");
    g_TimeSlot            = glGetUniformLocation(g_ShaderProgram, "mini_uTime");
    g_ResolutionSlot      = glGetUniformLocation(g_ShaderProgram, "mini_uResolution");
    g_MouseSlot           = glGetUniformLocation(g_ShaderProgram, "mini_uMouse");

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
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 0;

    // generate surface
    miniCreateSurface(&g_SurfaceWidth,
                      &g_SurfaceHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);
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

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    // create the viewport
    glViewport(0, 0, view_w, view_h);

    // get the screen resolution
    g_Resolution.m_X = view_w;
    g_Resolution.m_Y = view_h;

    // notify shader about screen size
    glUniform2f(g_ResolutionSlot, view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // calculate next time
    g_Time += timeStep_sec * 0.5f;

    // is time out of bounds?
    if (g_Time > g_MaxTime)
        g_Time -= g_MaxTime;

    // notify shader about elapsed time
    glUniform1f(g_TimeSlot, g_Time);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // draw the surface on which the shader will be executed
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
    // notify shader about mouse (or finger) position
    glUniform2f(g_MouseSlot, g_Resolution.m_X - x, y);
}
//------------------------------------------------------------------------------
#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    int main()
    {
        printf("Enter the shader file name to load (e.g. Shaders/file.shader)\n");

        do
        {
            printf("> ");
            scanf("%s", g_ShaderFile);

            g_ShaderFileSize = miniGetFileSize(g_ShaderFile);

            if (g_ShaderFileSize)
                break;

            printf("ERROR - cannot open file - %s\n", g_ShaderFile);
        } while(1);

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
