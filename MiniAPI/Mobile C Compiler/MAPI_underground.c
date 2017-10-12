/*****************************************************************************
 * ==> Underground demo -----------------------------------------------------*
 *****************************************************************************
 * Description : An underground level demo based on ray casting engines like *
 *               Doom or Wolfenstein. Swipe up or down to walk, and left or  *
 *               right to rotate                                             *
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
#include "MiniAPI/MiniCollision.h"
#include "MiniAPI/MiniVertex.h"
#include "MiniAPI/MiniShapes.h"
#include "MiniAPI/MiniShader.h"
#include "MiniAPI/MiniLevel.h"
#include "MiniAPI/MiniRenderer.h"

#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    #include <ccr.h>
#endif

// map mode, used for debugging
//#define MAP_MODE

#define SOIL_TEXTURE_FILE "Resources/Soil_25_256x256.bmp"
#define WALL_TEXTURE_FILE "Resources/Wall_Tilleable_64_256x256.bmp"
#define CEIL_TEXTURE_FILE "Resources/Soil_2_25_256x256.bmp"

#ifdef MAP_MODE
    #define SPHERE_TEXTURE_FILE "Resources/cloud.bmp"
#endif

//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef _OS_ANDROID_
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
//------------------------------------------------------------------------------
const char* g_pLevelMap =
    "***   ***   ******"
    "********* *   *  *"
    "********* ** **  *"
    "*** * ************"
    "*** * ********* **"
    "***   ***  *    **"
    "********* *** ****"
    "********* *** ****"
    "***   *** *** ****";
//------------------------------------------------------------------------------
const char* miniVSTextured2 =
    "precision mediump float;"
    "attribute vec4 qr_vPosition;"
    "attribute vec4 qr_vColor;"
    "attribute vec2 qr_vTexCoord;"
    "uniform   mat4 qr_uProjection;"
    "uniform   mat4 qr_uView;"
    "uniform   mat4 qr_uModelview;"
    "varying   vec4 qr_fColor;"
    "varying   vec2 qr_fTexCoord;"
    "void main(void)"
    "{"
    "    qr_fColor    = qr_vColor;"
    "    qr_fTexCoord = qr_vTexCoord;"
    "    gl_Position  = qr_uProjection * qr_uView * qr_uModelview * qr_vPosition;"
    "}";
//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
MINI_LevelItem*    g_pLevel             = 0;
GLuint             g_ShaderProgram      = 0;
float*             g_pSurfaceVB         = 0;
unsigned int       g_SurfaceVertexCount = 0;
const float        g_LevelItemWidth     = 0.05f;
const float        g_LevelItemHeight    = 0.05f;
const float        g_LevelItemLength    = 0.05f;
const float        g_PosVelocity        = 1.0f;
const float        g_DirVelocity        = 20.0f;
const float        g_ControlRadius      = 40.0f;
float              g_Angle              = 0.0f;
const unsigned int g_MapWidth           = 18;
const unsigned int g_MapHeight          = 9;
const unsigned int g_MapLength          = g_MapWidth * g_MapHeight;
MINI_Sphere        g_Player;
MINI_Vector2       g_TouchOrigin;
MINI_Vector2       g_TouchPosition;
GLuint             g_SoilTextureIndex   = GL_INVALID_VALUE;
GLuint             g_WallTextureIndex   = GL_INVALID_VALUE;
GLuint             g_CeilTextureIndex   = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot     = 0;
GLuint             g_ViewUniform        = 0;
GLuint             g_ModelviewUniform   = 0;
MINI_VertexFormat  g_VertexFormat;

#ifdef MAP_MODE
    float             g_SphereRadius;
    float*            g_pSphereVertices     = 0;
    unsigned int      g_SphereVertexCount;
    unsigned int      g_SphereIndexCount;
    MINI_Index*       g_pSphereIndexes      = 0;
    MINI_VertexFormat g_SphereVertexFormat;
    GLuint            g_SphereTextureIndex  = GL_INVALID_VALUE;
#endif
//------------------------------------------------------------------------------
void ApplyMatrix(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.001f;
    const float zFar   = 1000.0f;
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

    // initialize the player
    g_Player.m_Pos.m_X = 0.0f;
    g_Player.m_Pos.m_Y = 0.0f;
    g_Player.m_Pos.m_Z = 0.0f;
    g_Player.m_Radius  = 0.01f;

    g_Angle = 0.0f;

    // initialize the touch
    g_TouchOrigin.m_X   = 0.0f;
    g_TouchOrigin.m_Y   = 0.0f;
    g_TouchPosition.m_X = 0.0f;
    g_TouchPosition.m_Y = 0.0f;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniVSTextured2, miniGetFSTextured());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram, "qr_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram, "qr_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram, "qr_sColorMap");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // generate the level
    miniGenerateLevel(g_pLevelMap,
                      g_MapWidth,
                      g_MapHeight,
                      &g_LevelItemWidth,
                      &g_LevelItemHeight,
                      &g_pLevel);

    #ifdef MAP_MODE
        g_SphereRadius                     = g_Player.m_Radius;
        g_SphereVertexFormat.m_UseNormals  = 0;
        g_SphereVertexFormat.m_UseTextures = 1;
        g_SphereVertexFormat.m_UseColors   = 1;

        // generate sphere
        miniCreateSphere(&g_SphereRadius,
                         5,
                         5,
                         0xFFFFFFFF,
                         &g_SphereVertexFormat,
                         &g_pSphereVertices,
                         &g_SphereVertexCount,
                         &g_pSphereIndexes,
                         &g_SphereIndexCount);
    #endif

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // calculate the stride
    miniCalculateStride(&g_VertexFormat);

    // generate surface
    miniCreateSurface(&g_LevelItemWidth,
                      &g_LevelItemHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);

    // load textures
    g_SoilTextureIndex = miniLoadTexture(SOIL_TEXTURE_FILE);
    g_WallTextureIndex = miniLoadTexture(WALL_TEXTURE_FILE);
    g_CeilTextureIndex = miniLoadTexture(CEIL_TEXTURE_FILE);

    #ifdef MAP_MODE
        g_SphereTextureIndex = miniLoadTexture(SPHERE_TEXTURE_FILE);
    #endif
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete objects used in map mode
    #ifdef MAP_MODE
        // delete sphere vertex buffer
        if (g_pSphereVertices)
        {
            free(g_pSphereVertices);
            g_pSphereVertices = 0;
        }

        // delete sphere index buffer
        if (g_pSphereIndexes)
        {
            free(g_pSphereIndexes);
            g_pSphereIndexes = 0;
        }
    #endif

    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    // delete the level
    if (g_pLevel)
    {
        free(g_pLevel);
        g_pLevel = 0;
    }

    // delete textures
    if (g_SoilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_SoilTextureIndex);

    g_SoilTextureIndex = GL_INVALID_VALUE;

    if (g_WallTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_WallTextureIndex);

    g_WallTextureIndex = GL_INVALID_VALUE;

    if (g_CeilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_CeilTextureIndex);

    g_CeilTextureIndex = GL_INVALID_VALUE;

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
    float        angle;
    MINI_Vector3 newPos;

    // if screen isn't touched, do nothing
    if (!g_TouchOrigin.m_X || !g_TouchOrigin.m_Y)
        return;

    newPos = g_Player.m_Pos;

    // calculate the angle formed by the touch gesture x and y distances
    if ((g_TouchPosition.m_X < g_TouchOrigin.m_X || g_TouchPosition.m_Y < g_TouchOrigin.m_Y) &&
       !(g_TouchPosition.m_X < g_TouchOrigin.m_X && g_TouchPosition.m_Y < g_TouchOrigin.m_Y))
        angle = -atanf((g_TouchPosition.m_Y - g_TouchOrigin.m_Y) / (g_TouchPosition.m_X - g_TouchOrigin.m_X));
    else
        angle =  atanf((g_TouchPosition.m_Y - g_TouchOrigin.m_Y) / (g_TouchPosition.m_X - g_TouchOrigin.m_X));

    // calculate the possible min and max values for each axis
    float minX = g_TouchOrigin.m_X - (cosf(angle) * g_ControlRadius);
    float maxX = g_TouchOrigin.m_X + (cosf(angle) * g_ControlRadius);
    float minY = g_TouchOrigin.m_Y - (sinf(angle) * g_ControlRadius);
    float maxY = g_TouchOrigin.m_Y + (sinf(angle) * g_ControlRadius);

    // limit the touch gesture in a radius distance
    if (g_TouchPosition.m_X > maxX)
        g_TouchPosition.m_X = maxX;
    else
    if (g_TouchPosition.m_X < minX)
        g_TouchPosition.m_X = minX;

    if (g_TouchPosition.m_Y > maxY)
        g_TouchPosition.m_Y = maxY;
    else
    if (g_TouchPosition.m_Y < minY)
        g_TouchPosition.m_Y = minY;

    // calculate the final pos and dir velocity
    float posVelocity = (g_PosVelocity * ((g_TouchPosition.m_Y - g_TouchOrigin.m_Y) / g_TouchOrigin.m_Y));
    float dirVelocity = (g_DirVelocity * ((g_TouchPosition.m_X - g_TouchOrigin.m_X) / g_TouchOrigin.m_X));

    // calculate the next player direction
    g_Angle += dirVelocity * timeStep_sec;

    // validate it
    if (g_Angle > M_PI * 2.0f)
        g_Angle -= M_PI * 2.0f;
    else
    if (g_Angle < 0.0f)
        g_Angle += M_PI * 2.0f;

    // calculate the next player position
    newPos.m_X += posVelocity * cosf(g_Angle + (M_PI * 0.5f)) * timeStep_sec;
    newPos.m_Z += posVelocity * sinf(g_Angle + (M_PI * 0.5f)) * timeStep_sec;

    // validate and apply it
    miniValidateNextPos(g_pLevel,
                        g_LevelItemWidth,
                        g_LevelItemHeight,
                        g_MapLength,
                       &g_Player,
                       &newPos);

    g_Player.m_Pos = newPos;
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    MINI_LevelDrawInfo drawInfo;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // configure the draw info
    drawInfo.m_pSurfaceVB         =  g_pSurfaceVB;
    drawInfo.m_SurfaceVertexCount =  g_SurfaceVertexCount;
    drawInfo.m_pVertexFormat      = &g_VertexFormat;
    drawInfo.m_pShader            = &g_Shader;
    drawInfo.m_ShaderProgram      =  g_ShaderProgram;
    drawInfo.m_SoilTextureIndex   =  g_SoilTextureIndex;
    drawInfo.m_WallTextureIndex   =  g_WallTextureIndex;
    drawInfo.m_CeilTextureIndex   =  g_CeilTextureIndex;

    #ifdef MAP_MODE
        drawInfo.m_pSphereVB           =  g_pSphereVertices;
        drawInfo.m_SphereRadius        =  g_SphereRadius;
        drawInfo.m_pSphereIndexes      =  g_pSphereIndexes;
        drawInfo.m_SphereVertexCount   =  g_SphereVertexCount;
        drawInfo.m_SphereIndexCount    =  g_SphereIndexCount;
        drawInfo.m_pSphereShader       = &g_Shader;
        drawInfo.m_pSphereVertexFormat = &g_SphereVertexFormat;
        drawInfo.m_SphereTextureIndex  =  g_SphereTextureIndex;
        drawInfo.m_MapMode             =  1;
    #else
        drawInfo.m_MapMode = 0;
    #endif

    // draw the level
    miniDrawLevel(g_pLevel,
                  g_LevelItemWidth,
                  g_LevelItemHeight,
                  g_LevelItemLength,
                  g_MapLength,
                 &g_Player,
                  g_Angle,
                 &drawInfo);

    miniEndScene();
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{
    // initialize the position
    g_TouchOrigin.m_X   = x;
    g_TouchOrigin.m_Y   = y;
    g_TouchPosition.m_X = x;
    g_TouchPosition.m_Y = y;
}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    // reset the position
    g_TouchOrigin.m_X   = 0;
    g_TouchOrigin.m_Y   = 0;
    g_TouchPosition.m_X = 0;
    g_TouchPosition.m_Y = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
    // get the next position
    g_TouchPosition.m_X = x;
    g_TouchPosition.m_Y = y;
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
