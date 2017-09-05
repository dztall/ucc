/*****************************************************************************
 * ==> MD2 (.md2) robot demo ------------------------------------------------*
 *****************************************************************************
 * Description : MD2 robot demo.                                             *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

// supported platforms check (for now, only supports iOS and Android devices.
// NOTE Android support is theorical, never tested on a such device)
#if !defined(IOS) && !defined(ANDROID)
    #error "Not supported platform!"
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef ANDROID
    #include <gles2/gl2.h>
    #include <gles2/gl2ext.h>
#endif
#ifdef IOS
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#endif

// NOTE the md2 model was found on this site:
// http://leileilol.mancubus.net/garyacordsucks/64.246.6.138/_gcsgames.com/GCSenemy/md2.htm
#ifdef ANDROID
    #define MD2_FILE         "/sdcard/C++ Compiler/chip.md2"
    #define MD2_TEXTURE_FILE "/sdcard/C++ Compiler/chipskin.bmp"
#else
    #define MD2_FILE         "Resources/chip.md2"
    #define MD2_TEXTURE_FILE "Resources/chipskin.bmp"
#endif

// mini API
#include "MiniAPI/MiniModels.h"
#include "MiniAPI/MiniShader.h"

//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} QR_MD2Animation;
//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef ANDROID
        GLuint g_Framebuffer, g_Renderbuffer;
    #endif
#endif
MV_VertexFormat    g_ModelFormat;
MM_MD2Model*       g_pModel         = 0;
unsigned int       g_MeshIndex      = 0;
GLuint             g_ShaderProgram  = 0;
GLuint             g_TextureIndex   = GL_INVALID_VALUE;
GLuint             g_PositionSlot   = 0;
GLuint             g_ColorSlot      = 0;
GLuint             g_TexCoordSlot   = 0;
GLuint             g_TexSamplerSlot = 0;
float              g_Time           = 0.0f;
float              g_Interval       = 0.0f;
const unsigned int g_FPS            = 15;
const unsigned int g_AnimIndex      = 0; // Can only be 0 (robot walks) or 1 (robot dies)
QR_MD2Animation    g_Animation[2];
//------------------------------------------------------------------------------
void ApplyProjection(float width, float height) const
{
    // get orthogonal matrix
    float left   = -width  * 0.5f;
    float right  =  width  * 0.5f;
    float bottom = -height * 0.5f;
    float top    =  height * 0.5f;
    float near   =  1.0f;
    float far    =  100.0f;

    MG_Matrix ortho;
    GetOrtho(&left, &right, &bottom, &top, &near, &far, &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    // renderer buffers should no more be generated since CCR version 1.1
    #if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
        #ifndef ANDROID
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

    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(g_pVSTextured, g_pFSTextured);
    glUseProgram(g_ShaderProgram);

    g_ModelFormat.m_UseNormals  = 0;
    g_ModelFormat.m_UseTextures = 1;
    g_ModelFormat.m_UseColors   = 1;

    // load MD2 file and create mesh to draw
    LoadMD2Model(MD2_FILE, &g_ModelFormat, 0xFFFFFFFF, &g_pModel);

    // load MD2 texture
    g_TextureIndex = LoadTexture(MD2_TEXTURE_FILE);

    g_PositionSlot   = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_ColorSlot      = glGetAttribLocation(g_ShaderProgram, "qr_vColor");
    g_TexCoordSlot   = glGetAttribLocation(g_ShaderProgram, "qr_vTexCoord");
    g_TexSamplerSlot = glGetAttribLocation(g_ShaderProgram, "qr_sColorMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // create MD2 animation list
    g_Animation[0].m_Range[0] = 0;  g_Animation[0].m_Range[1] = 20; // robot walks
    g_Animation[1].m_Range[0] = 21; g_Animation[1].m_Range[1] = 29; // robot dies

    // calculate frame interval
    g_Interval = 1000.0f / g_FPS;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    ReleaseMD2Model(g_pModel);

    glDeleteTextures(1, &g_TextureIndex);

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
    glViewport(0, 0, view_w, view_h);
    ApplyProjection(24.0f, 24.0f * 1.12f);
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
    MG_Vector3 t;
    MG_Vector3 axis;
    MG_Vector3 factor;
    MG_Matrix  translateMatrix;
    MG_Matrix  rotateMatrix;
    MG_Matrix  scaleMatrix;
    MG_Matrix  modelViewMatrix;
    MM_Frame*  pFrame;
    MM_Mesh*   pMesh;
    float      angle;
    float*     pVB;
    GLvoid*    pCoords;
    GLvoid*    pColors;
    GLvoid*    pTexCoords;
    GLsizei    stride;
    GLint      modelviewUniform;

    // clear scene background and depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // enable position and color slots
    glEnableVertexAttribArray(g_PositionSlot);
    glEnableVertexAttribArray(g_TexCoordSlot);
    glEnableVertexAttribArray(g_ColorSlot);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -20.0f;

    GetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = 0.0f;

    GetRotateMatrix(&angle, &axis, &rotateMatrix);

    // set scale factor
    factor.m_X = 0.5f;
    factor.m_Y = 0.5f;
    factor.m_Z = 0.5f;

    GetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    GetIdentity(&modelViewMatrix);
    MatrixMultiply(&modelViewMatrix, &rotateMatrix,    &modelViewMatrix);
    MatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);
    MatrixMultiply(&modelViewMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

     // calculate vertex stride
    stride = g_pModel->m_pVertexFormat->m_Stride;

    // get current frame
    pFrame = &g_pModel->m_pFrame[g_Animation[g_AnimIndex].m_Range[0] + g_MeshIndex];

    // iterate through vertices to draw
    for (unsigned int i = 0; i < pFrame->m_MeshCount; ++i)
    {
        // get current vertices
        pMesh = &pFrame->m_pMesh[i];
        pVB   = &pMesh->m_pVertexBuffer[0];

        pCoords    = &pVB[0];
        pTexCoords = &pVB[3];
        pColors    = &pVB[5];

       // connect object to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
        glVertexAttribPointer(g_TexCoordSlot, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), pTexCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

        // draw vertices
        if (pMesh->m_IsTriangleStrip)
            glDrawArrays(GL_TRIANGLE_STRIP, 0, pMesh->m_VertexCount);
        else
            glDrawArrays(GL_TRIANGLE_FAN, 0, pMesh->m_VertexCount);
    }

    // disconnect slots from shader
    glDisableVertexAttribArray(g_PositionSlot);
    glDisableVertexAttribArray(g_TexCoordSlot);
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
#ifdef IOS
    void on_GLES2_DeviceRotate(int orientation)
    {}
#endif
//------------------------------------------------------------------------------
