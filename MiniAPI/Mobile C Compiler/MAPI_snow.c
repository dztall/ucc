/*****************************************************************************
 * ==> Snow demo ------------------------------------------------------------*
 *****************************************************************************
 * Description : Snow particle system demo                                   *
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
#include "MiniAPI/MiniParticles.h"
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
GLuint            g_ShaderProgram = 0;
float*            g_pVertexBuffer = 0;
unsigned int      g_VertexCount   = 0;
float             g_Radius        = 0.02f;
float             g_Angle         = 0.0f;
float             g_RotationSpeed = 0.1f;
const unsigned    g_ParticleCount = 100;
int               g_Initialized   = 0;
MINI_Particles    g_Particles;
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

    srand((unsigned)time(0));

    g_Particles.m_Count = 0;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "mini_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "mini_vColor");

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
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate disk
    miniCreateDisk(0.0f,
                   0.0f,
                   g_Radius,
                   5,
                   0xFFFFFFFF,
                  &g_VertexFormat,
                  &g_pVertexBuffer,
                  &g_VertexCount);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    miniClearParticles(&g_Particles);

    // delete vertices
    if (g_pVertexBuffer)
    {
        free(g_pVertexBuffer);
        g_pVertexBuffer = 0;
    }

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
    unsigned       i;
    MINI_Vector3   startPos;
    MINI_Vector3   startDir;
    MINI_Vector3   startVelocity;
    MINI_Particle* pNewParticle;

    startPos.m_X      =  0.0f; // between -2.2 to 2.2
    startPos.m_Y      =  2.0f;
    startPos.m_Z      = -3.0f; // between -1.0 to -5.0
    startDir.m_X      =  1.0f;
    startDir.m_Y      = -1.0f;
    startDir.m_Z      =  0.0f;
    startVelocity.m_X =  0.0f;
    startVelocity.m_Y =  0.05f;
    startVelocity.m_Z =  0.0f;

    // iterate through particles to update
    for (i = 0; i < g_ParticleCount; ++i)
    {
        // emit a new particle
        if (pNewParticle = miniEmitParticle(&g_Particles,
                                            &startPos,
                                            &startDir,
                                            &startVelocity,
                                            g_ParticleCount))
        {
            // initialize default values
            pNewParticle->m_Position.m_X =  ((rand() % 44) - 22.0f) * 0.1f; // between -2.2 to  2.2
            pNewParticle->m_Position.m_Z = -((rand() % 40) + 10.0f) * 0.1f; // between -1.0 to -5.0
            pNewParticle->m_Velocity.m_X =  ((rand() % 4)  - 2.0f)  * 0.01f;
            pNewParticle->m_Velocity.m_Y =  ((rand() % 4)  + 2.0f)  * 0.01f;

            // select a random start height the first time particles are emitted
            if (!g_Initialized)
                pNewParticle->m_Position.m_Y = 2.0f + ((rand() % 200) * 0.01f);
        }

        // no particles to show? (e.g all were removed in this loop)
        if (!g_Particles.m_Count)
            continue;

        // move particle
        if (i >= g_Particles.m_Count)
            miniMoveParticle(&g_Particles.m_pParticles[g_Particles.m_Count - 1],
                              timeStep_sec * 20.0f);
        else
            miniMoveParticle(&g_Particles.m_pParticles[i], timeStep_sec * 20.0f);

        // is particle out of screen?
        if (g_Particles.m_pParticles[i].m_Position.m_Y <= -2.0f ||
            g_Particles.m_pParticles[i].m_Position.m_X <= -4.0f ||
            g_Particles.m_pParticles[i].m_Position.m_X >=  4.0f)
        {
            // delete it from system
            miniDeleteParticle(&g_Particles, i);
            continue;
        }
    }

    g_Initialized = 1;
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    unsigned     i;
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.1f, 0.35f, 0.66f, 1.0f);

    // iterate through particles to draw
    for (i = 0; i < g_Particles.m_Count; ++i)
    {
        // set translation
        t.m_X = g_Particles.m_pParticles[i].m_Position.m_X;
        t.m_Y = g_Particles.m_pParticles[i].m_Position.m_Y;
        t.m_Z = g_Particles.m_pParticles[i].m_Position.m_Z;

        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect model view matrix to shader
        GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the particle
        miniDrawDisk(g_pVertexBuffer, g_VertexCount, &g_VertexFormat, &g_Shader);
    }

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
