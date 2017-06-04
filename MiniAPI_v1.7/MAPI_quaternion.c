/*****************************************************************************
 * ==> Quaternion demo ------------------------------------------------------*
 *****************************************************************************
 * Description : An animation using a quaternion                             *
 * Developer   : Jean-Milost Reymond                                         *
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
#include "MiniAPI/MiniGeometry.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniShapes.h"
#include "MiniAPI/MiniCollision.h"
#include "MiniAPI/MiniShader.h"

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
GLuint             g_ShaderProgram   = 0;
float*             g_pVertexBuffer   = 0;
int                g_VertexCount     = 0;
MV_Index*          g_pIndexes        = 0;
int                g_IndexCount      = 0;
float*             g_pCylinderVB     = 0;
int                g_CylinderVBCount = 0;
float              g_CircleRadius    = 0.1f;
float              g_CylinderRadius  = 0.1f;
float              g_CylinderHeight  = 2.0f;
float              g_Angle           = 0.0f;
unsigned           g_CylFaceCount    = 12;
GLuint             g_PositionSlot    = 0;
GLuint             g_ColorSlot       = 0;
MG_Size            g_View;
MV_VertexFormat    g_VertexFormat;
MG_Matrix          g_ProjectionMatrix;
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float near   = 1.0f;
    const float far    = 20.0f;
    const float fov    = 45.0f;
    const float aspect = (GLfloat)w/(GLfloat)h;

    GetPerspective(&fov, &aspect, &near, &far, &g_ProjectionMatrix);

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

    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(g_pVSColored, g_pFSColored);
    glUseProgram(g_ShaderProgram);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere
    CreateSphere(&g_CircleRadius,
                 5,
                 12,
                 0x0000FFFF,
                 &g_VertexFormat,
                 &g_pVertexBuffer,
                 &g_VertexCount,
                 &g_pIndexes,
                 &g_IndexCount);

    // generate sphere
    CreateCylinder(&g_CylinderRadius,
                   &g_CylinderHeight,
                   g_CylFaceCount,
                   0xFF0000FF,
                   &g_VertexFormat,
                   &g_pCylinderVB,
                   &g_CylinderVBCount);

    // get shader attributes
    g_PositionSlot = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_vColor");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // disable culling
    glDisable(GL_CULL_FACE);
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
    unsigned      i;
    unsigned      j;
    int           stride;
    float         angle;
    MG_Vector3    t;
    MG_Vector3    axis;
    MG_Quaternion quatX;
    MG_Quaternion quatZ;
    MG_Matrix     translateMatrix;
    MG_Matrix     rotTransMatrix;
    MG_Matrix     rotMatrixX;
    MG_Matrix     rotMatrixZ;
    MG_Matrix     combMatrix1;
    MG_Matrix     combMatrix2;
    MG_Matrix     modelMatrix;
    GLvoid*       pCoords;
    GLvoid*       pColors;

    // clear scene background and depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    t.m_X =  0.95f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the ball translation
    GetTranslateMatrix(&t, &translateMatrix);

    t.m_X = 0.2f;
    t.m_Y = 0.0f;
    t.m_Z = 0.0f;

    // set the ball animation translation
    GetTranslateMatrix(&t, &rotTransMatrix);

    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    angle    = g_Angle;

    // calculate the x axis rotation
    QuatFromAxis(&angle, &axis, &quatX);
    GetMatrix(&quatX, &rotMatrixX);

    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;
    angle    = M_PI / 2.0f;

    // calculate the z axis rotation
    QuatFromAxis(&angle, &axis, &quatZ);
    GetMatrix(&quatZ, &rotMatrixZ);

    // build the ball final model matrix
    MatrixMultiply(&rotTransMatrix, &rotMatrixZ,      &combMatrix1);
    MatrixMultiply(&combMatrix1,    &rotMatrixX,      &combMatrix2);
    MatrixMultiply(&combMatrix2,    &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // calculate vertex stride
    stride = g_VertexFormat.m_Stride;

    // enable position and color slots
    glEnableVertexAttribArray(g_PositionSlot);
    glEnableVertexAttribArray(g_ColorSlot);

    // iterate through vertex fan buffers to draw
    for (int i = 0; i < g_IndexCount; ++i)
    {
        // get next vertices fan buffer
        pCoords = &g_pVertexBuffer[g_pIndexes[i].m_Start];
        pColors = &g_pVertexBuffer[g_pIndexes[i].m_Start + 3];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

        // draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, g_pIndexes[i].m_Length / stride);
    }

    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the cylinder animation translation
    GetTranslateMatrix(&t, &translateMatrix);

    // build the cylinder final model matrix
    MatrixMultiply(&rotMatrixZ,  &rotMatrixX,      &combMatrix1);
    MatrixMultiply(&combMatrix1, &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // get next vertices fan buffer
    pCoords = &g_pCylinderVB[0];
    pColors = &g_pCylinderVB[3];

    // connect buffer to shader
    glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
    glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

    // draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (g_CylFaceCount + 1) * 2);

    // disconnect slots from shader
    glDisableVertexAttribArray(g_PositionSlot);
    glDisableVertexAttribArray(g_ColorSlot);
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
