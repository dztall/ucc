/*****************************************************************************
 * ==> Minimal shader library -----------------------------------------------*
 *****************************************************************************
 * Description : Minimal shader library                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not                 *
 *****************************************************************************/

#ifndef MiniShaderH
#define MiniShaderH

#ifdef ANDROID
    #include <gles2/gl2.h>
    #include <gles2/gl2ext.h>
#endif
#ifdef IOS
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#endif

//-----------------------------------------------------------------------------
// Common shader programs
//-----------------------------------------------------------------------------

/**
* Vertex shader program containing slots for position and color
*/
const char* g_pVSColored =
    "precision mediump float;"
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

/**
* Fragment shader program containing slots for position and color
*/
const char* g_pFSColored =
    "precision mediump float;"
    "varying lowp vec4 qr_fColor;"
    "void main(void)"
    "{"
    "    gl_FragColor = qr_fColor;"
    "}";

/**
* Vertex shader program containing slots for position, color and texture
*/
const char* g_pVSTextured =
    "precision mediump float;"
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

/**
* Fragment shader program containing slots for position, color and texture
*/
const char* g_pFSTextured =
    "precision mediump float;"
    "uniform sampler2D qr_sColorMap;"
    "varying lowp vec4 qr_fColor;"
    "varying      vec2 qr_fTexCoord;"
    "void main(void)"
    "{"
    "    gl_FragColor = qr_fColor * texture2D(qr_sColorMap, qr_fTexCoord);"
    "}";

/**
* Vertex shader program containing slots for position, color, texture and alpha blending value
*/
const char* g_pVSTexAlpha =
    "precision mediump float;"
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

/**
* Fragment shader program containing slots for position, color, texture and alpha blending value
*/
const char* g_pFSTexAlpha =
    "precision mediump float;"
    "uniform sampler2D  qr_sColorMap;"
    "varying lowp vec4  qr_fColor;"
    "varying      vec2  qr_fTexCoord;"
    "varying      float qr_fAlpha;"
    "void main(void)"
    "{"
    "vec4 color   = qr_fColor * texture2D(qr_sColorMap, qr_fTexCoord);"
    "gl_FragColor = vec4(color.x, color.y, color.z, qr_fAlpha);"
    "}";

//-----------------------------------------------------------------------------
// Shader compilation and linking
//-----------------------------------------------------------------------------

/**
* Creates a new shader program and compile it
*@param pSource - source shader program to create and compile
*@param shaderType - shader type to create, parameters are:
                     -> GL_VERTEX_SHADER for vertex shader
                     -> GL_FRAGMENT_SHADER for fragment shader
*@return compiled shader program identifier
*@note This function logs the errors returned by OpenGL shader compiler
*/
GLuint CreateAndCompileShader(const char* pSource, GLenum shaderType)
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

/**
* Creates a shader program then uses it compiles and links vertex and fragment shaders
*@param pVShader - source vertex shader program to compile
*@param pFShader - source vertex shader program to compile
*@return newly created vertex program identifier
*@note This function logs the errors returned by OpenGL shader compiler
*@note Program should be deleted when useless by calling glDeleteProgram()
*/
GLuint CompileShaders(const char* pVShader,const char* pFShader)
{
    GLuint programHandle;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint  linkSuccess;
    GLchar messages[256];

    // create and compile vertex and fragment shaders programs
    vertexShader   = CreateAndCompileShader(pVShader, GL_VERTEX_SHADER);
    fragmentShader = CreateAndCompileShader(pFShader, GL_FRAGMENT_SHADER);

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

#endif
