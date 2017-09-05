/*****************************************************************************
 * ==> Quaternion demo ------------------------------------------------------*
 *****************************************************************************
 * Description : An animation using a quaternion                             *
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

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram   = 0;
float*            g_pVertexBuffer   = 0;
unsigned int      g_VertexCount     = 0;
MINI_Index*       g_pIndexes        = 0;
unsigned int      g_IndexCount      = 0;
float*            g_pCylinderVB     = 0;
unsigned int      g_CylinderVBCount = 0;
float             g_CircleRadius    = 0.1f;
float             g_CylinderRadius  = 0.1f;
float             g_CylinderHeight  = 2.0f;
float             g_Angle           = 0.0f;
unsigned          g_CylFaceCount    = 12;
MINI_Size         g_View;
MINI_VertexFormat g_VertexFormat;
MINI_Matrix       g_ProjectionMatrix;
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 20.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &g_ProjectionMatrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_ProjectionMatrix.m_Table[0][0]);
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

    g_View.m_Width  = 0.0f;
    g_View.m_Height = 0.0f;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "qr_vColor");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // disable culling
    glDisable(GL_CULL_FACE);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&g_CircleRadius,
                     5,
                     12,
                     0x0000FFFF,
                     &g_VertexFormat,
                     &g_pVertexBuffer,
                     &g_VertexCount,
                     &g_pIndexes,
                     &g_IndexCount);

    // generate cylinder
    miniCreateCylinder(&g_CylinderRadius,
                       &g_CylinderHeight,
                       g_CylFaceCount,
                       0xFF0000FF,
                       &g_VertexFormat,
                       &g_pCylinderVB,
                       &g_CylinderVBCount);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
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

    // delete cylinder vertices
    if (g_pCylinderVB)
    {
        free(g_pCylinderVB);
        g_pCylinderVB = 0;
    }

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
{
    // calculate next angle value, limit to 2 * PI
    g_Angle = fmodf(g_Angle + (3.0f * timeStep_sec), M_PI * 2.0f);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    float           angle;
    MINI_Vector3    t;
    MINI_Vector3    axis;
    MINI_Quaternion quatX;
    MINI_Quaternion quatZ;
    MINI_Matrix     translateMatrix;
    MINI_Matrix     rotTransMatrix;
    MINI_Matrix     rotMatrixX;
    MINI_Matrix     rotMatrixZ;
    MINI_Matrix     combMatrix1;
    MINI_Matrix     combMatrix2;
    MINI_Matrix     modelMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    
    t.m_X =  0.95f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the ball translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    t.m_X = 0.2f;
    t.m_Y = 0.0f;
    t.m_Z = 0.0f;

    // set the ball animation translation
    miniGetTranslateMatrix(&t, &rotTransMatrix);

    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    angle    = g_Angle;

    // calculate the x axis rotation
    miniQuatFromAxis(&angle, &axis, &quatX);
    miniGetMatrix(&quatX, &rotMatrixX);

    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;
    angle    = M_PI / 2.0f;

    // calculate the z axis rotation
    miniQuatFromAxis(&angle, &axis, &quatZ);
    miniGetMatrix(&quatZ, &rotMatrixZ);

    // build the ball final model matrix
    miniMatrixMultiply(&rotTransMatrix, &rotMatrixZ,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1,    &rotMatrixX,      &combMatrix2);
    miniMatrixMultiply(&combMatrix2,    &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the sphere
    miniDrawSphere(g_pVertexBuffer,
                   g_VertexCount,
                   g_pIndexes,
                   g_IndexCount,
                   &g_VertexFormat,
                   &g_Shader);

    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the cylinder animation translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    // build the cylinder final model matrix
    miniMatrixMultiply(&rotMatrixZ,  &rotMatrixX,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1, &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the cylinder
    miniDrawCylinder(g_pCylinderVB, g_CylFaceCount, &g_VertexFormat, &g_Shader);

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
