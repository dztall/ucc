/*****************************************************************************
 * ==> Bump mapping demo ----------------------------------------------------*
 *****************************************************************************
 * Description : A bump mapped stone wall with diffuse light, swipe on       *
 *               screen to modify diffuse light properties                   *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

// supported platforms check. NOTE iOS only, but may works on other platforms
#if !defined(_OS_IOS_)
    #error "Not supported platform!"
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
#include "MiniAPI/MiniShader.h"

// NOTE the texture was found here: http://opengameart.org/content/stone-texture-bump
#ifdef ANDROID
    #define STONE_TEXTURE_FILE "/sdcard/C++ Compiler/stone.bmp"
    #define STONE_BUMPMAP_FILE "/sdcard/C++ Compiler/stone_bump.bmp"
#else
    #define STONE_TEXTURE_FILE "Resources/stone.bmp"
    #define STONE_BUMPMAP_FILE "Resources/stone_bump.bmp"
#endif

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef ANDROID
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
GLuint             g_ShaderProgram      = 0;
float*             g_pSurfaceVB         = 0;
int                g_SurfaceVertexCount = 0;
const float        g_SurfaceWidth       = 10.0f;
const float        g_SurfaceHeight      = 12.5f;
GLuint             g_TextureIndex       = GL_INVALID_VALUE;
GLuint             g_BumpMapIndex       = GL_INVALID_VALUE;
GLuint             g_PositionSlot       = 0;
GLuint             g_ColorSlot          = 0;
GLuint             g_TexCoordSlot       = 0;
GLuint             g_LightPos           = 0;
GLuint             g_TexSamplerSlot     = 0;
GLuint             g_BumpMapSamplerSlot = 0;
MG_Size            g_View;
MV_VertexFormat    g_VertexFormat;
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
    "    float diffuse = max(dot(normal, qr_fLightPos), 0.0);"
    "    vec3  color   = diffuse * texture2D(qr_sColorMap, qr_fTexCoord).rgb;"
    "    gl_FragColor  = vec4(color, 1.0);"
    "}";
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float near   = 1.0f;
    const float far    = 100.0f;
    const float fov    = 45.0f;
    const float aspect = (GLfloat)w/(GLfloat)h;

    MG_Matrix matrix;
    GetPerspective(&fov, &aspect, &near, &far, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    // renderer buffers should no more be generated since CCR version 1.1
    #if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
        #ifndef ANDROID
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

    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(g_pVSDiffuseBumpMap, g_pFSDiffuseBumpMap);
    glUseProgram(g_ShaderProgram);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // generate surface
    CreateSurface(&g_SurfaceWidth,
                  &g_SurfaceHeight,
                  0xFFFFFFFF,
                  &g_VertexFormat,
                  &g_pSurfaceVB,
                  &g_SurfaceVertexCount);

    // load earth texture
    g_TextureIndex = LoadTexture(STONE_TEXTURE_FILE);
    g_BumpMapIndex = LoadTexture(STONE_BUMPMAP_FILE);

    // get shader attributes
    g_PositionSlot       = glGetAttribLocation(g_ShaderProgram,  "qr_vPosition");
    g_ColorSlot          = glGetAttribLocation(g_ShaderProgram,  "qr_vColor");
    g_TexCoordSlot       = glGetAttribLocation(g_ShaderProgram,  "qr_vTexCoord");
    g_LightPos           = glGetUniformLocation(g_ShaderProgram, "qr_vLightPos");
    g_TexSamplerSlot     = glGetUniformLocation(g_ShaderProgram, "qr_sColorMap");
    g_BumpMapSamplerSlot = glGetUniformLocation(g_ShaderProgram, "qr_sBumpMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // notify shader about default light position
    glUniform3f(g_LightPos, 0.0f, 0.0f, 2.0f);
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
    int        stride;
    MG_Vector3 t;
    MG_Matrix  modelViewMatrix;
    GLvoid*    pCoords;
    GLvoid*    pTexCoords;
    GLvoid*    pColors;

    // clear scene background and depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // calculate vertex stride
    stride = g_VertexFormat.m_Stride;

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -15.0f;

    // get translation matrix
    GetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // enable position, texture and color slots
    glEnableVertexAttribArray(g_PositionSlot);
    glEnableVertexAttribArray(g_TexCoordSlot);
    glEnableVertexAttribArray(g_ColorSlot);

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

    // get next vertices buffer
    pCoords    = &g_pSurfaceVB[0];
    pTexCoords = &g_pSurfaceVB[3];
    pColors    = &g_pSurfaceVB[5];

    // connect buffer to shader
    glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), pCoords);
    glVertexAttribPointer(g_TexCoordSlot, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), pTexCoords);
    glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride * sizeof(float), pColors);

    // draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
#ifdef IOS
    void on_GLES2_DeviceRotate(int orientation)
    {}
#endif
//------------------------------------------------------------------------------
