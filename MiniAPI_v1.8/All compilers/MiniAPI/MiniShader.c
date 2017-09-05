/*****************************************************************************
 * ==> MiniShader -----------------------------------------------------------*
 *****************************************************************************
 * Description : This module provides the functions to compile and manage    *
 *               shader programs, and provides several common and simple     *
 *               ready-to-use programs                                       *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#include "MiniShader.h"

// std
#include <stdio.h>

//----------------------------------------------------------------------------
// Common shader programs
//----------------------------------------------------------------------------
const char* miniGetVSColored()
{
    return "precision mediump float;"
           "attribute vec4 qr_vPosition;"
           "attribute vec4 qr_vColor;"
           "uniform   mat4 qr_uProjection;"
           "uniform   mat4 qr_uModelview;"
           "varying   vec4 qr_fColor;"
           "void main(void)"
           "{"
           "    qr_fColor    = qr_vColor;"
           "    gl_Position  = qr_uProjection * qr_uModelview * qr_vPosition;"
           "}";
}
//----------------------------------------------------------------------------
const char* miniGetFSColored()
{
    return "precision mediump float;"
           "varying lowp vec4 qr_fColor;"
           "void main(void)"
           "{"
           "    gl_FragColor = qr_fColor;"
           "}";
}
//----------------------------------------------------------------------------
const char* miniGetVSTextured()
{
    return "precision mediump float;"
           "attribute vec4 qr_vPosition;"
           "attribute vec4 qr_vColor;"
           "attribute vec2 qr_vTexCoord;"
           "uniform   mat4 qr_uProjection;"
           "uniform   mat4 qr_uModelview;"
           "varying   vec4 qr_fColor;"
           "varying   vec2 qr_fTexCoord;"
           "void main(void)"
           "{"
           "    qr_fColor    = qr_vColor;"
           "    qr_fTexCoord = qr_vTexCoord;"
           "    gl_Position  = qr_uProjection * qr_uModelview * qr_vPosition;"
           "}";
}
//----------------------------------------------------------------------------
const char* miniGetFSTextured()
{
    return "precision mediump float;"
           "uniform sampler2D qr_sColorMap;"
           "varying lowp vec4 qr_fColor;"
           "varying      vec2 qr_fTexCoord;"
           "void main(void)"
           "{"
           "    gl_FragColor = qr_fColor * texture2D(qr_sColorMap, qr_fTexCoord);"
           "}";
}
//----------------------------------------------------------------------------
const char* miniGetVSTexAlpha()
{
    return "precision mediump float;"
           "attribute vec4  qr_vPosition;"
           "attribute vec4  qr_vColor;"
           "attribute vec2  qr_vTexCoord;"
           "uniform   mat4  qr_uProjection;"
           "uniform   mat4  qr_uModelview;"
           "uniform   float qr_uAlpha;"
           "varying   vec4  qr_fColor;"
           "varying   vec2  qr_fTexCoord;"
           "varying   float qr_fAlpha;"
           "void main(void)"
           "{"
           "    qr_fColor    = qr_vColor;"
           "    qr_fTexCoord = qr_vTexCoord;"
           "    qr_fAlpha    = qr_uAlpha;"
           "    gl_Position  = qr_uProjection * qr_uModelview * qr_vPosition;"
           "}";
}
//----------------------------------------------------------------------------
const char* miniGetFSTexAlpha()
{
    return "precision mediump float;"
           "uniform sampler2D  qr_sColorMap;"
           "varying lowp vec4  qr_fColor;"
           "varying      vec2  qr_fTexCoord;"
           "varying      float qr_fAlpha;"
           "void main(void)"
           "{"
           "    vec4 color   = qr_fColor * texture2D(qr_sColorMap, qr_fTexCoord);"
           "    gl_FragColor = vec4(color.x, color.y, color.z, qr_fAlpha);"
           "}";
}
//----------------------------------------------------------------------------
// Shader compilation and linking functions
//----------------------------------------------------------------------------
GLuint miniCreateAndCompileShader(const char* pSource, GLenum shaderType)
{
    GLuint shaderHandle;
    GLint  compileSuccess;
    GLchar messages[256];

    // create and compile shader program
    shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &pSource, 0);
    glCompileShader(shaderHandle);

    // get compiler result
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

    // succeeded?
    if (compileSuccess == GL_FALSE)
    {
        // show error message (in console)
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        printf("compile glsl error : %s\n", messages);
    }

    return shaderHandle;
}
//----------------------------------------------------------------------------
GLuint miniCompileShaders(const char* pVShader,const char* pFShader)
{
    GLuint programHandle;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint  linkSuccess;
    GLchar messages[256];

    // create and compile vertex and fragment shader programs
    vertexShader   = miniCreateAndCompileShader(pVShader, GL_VERTEX_SHADER);
    fragmentShader = miniCreateAndCompileShader(pFShader, GL_FRAGMENT_SHADER);

    // link shader programs
    programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);

    // get linker result
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);

    // succeeded?
    if (linkSuccess == GL_FALSE)
    {
        // show error message (in console)
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        printf("compile glsl error : %s\n", messages);
    }

    return programHandle;
}
//----------------------------------------------------------------------------
