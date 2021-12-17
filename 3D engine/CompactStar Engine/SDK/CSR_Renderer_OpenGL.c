/****************************************************************************
 * ==> CSR_Renderer_OpenGL -------------------------------------------------*
 ****************************************************************************
 * Description : This module provides an OpenGL renderer                    *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2022, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

#include "CSR_Renderer_OpenGL.h"

// std
#include <stdlib.h>
#include <memory.h>

//---------------------------------------------------------------------------
// Texture functions
//---------------------------------------------------------------------------
GLuint csrOpenGLTextureFromPixelBuffer(const CSR_PixelBuffer* pPixelBuffer)
{
    unsigned char* pPixels = 0;
    size_t         x;
    size_t         y;
    size_t         c;
    GLint          pixelType;
    GLuint         index;

    // validate the input
    if (!pPixelBuffer || !pPixelBuffer->m_Width || !pPixelBuffer->m_Height || !pPixelBuffer->m_pData)
        return M_CSR_Error_Code;

    // select the correct pixel type to use
    switch (pPixelBuffer->m_BytePerPixel)
    {
        case 3:
            pixelType = GL_RGB;
            break;

        case 4:
            // actually the bitmaps are limited to 24 bit RGB (due to below calculation). For that
            // reason trying to create a texture from a RGBA bitmap is prohibited
            if (pPixelBuffer->m_ImageType == CSR_IT_Bitmap)
                return M_CSR_Error_Code;

            pixelType = GL_RGBA;
            break;

        default:
            return M_CSR_Error_Code;
    }

    // reorder the pixels if image is a bitmap
    if (pPixelBuffer->m_ImageType == CSR_IT_Bitmap)
    {
        const size_t bufferLength = sizeof(unsigned char)  *
                                    pPixelBuffer->m_Width  *
                                    pPixelBuffer->m_Height *
                                    3;

        pPixels = (unsigned char*)malloc(bufferLength);

        if (!pPixels || !pPixelBuffer->m_Stride)
            return M_CSR_Error_Code;

        // format bitmap data
        for (y = 0; y < pPixelBuffer->m_Height; ++y)
            for (x = 0; x < pPixelBuffer->m_Width; ++x)
                for (c = 0; c < 3; ++c)
                {
                    const size_t index = 3 * (pPixelBuffer->m_Width * y + x) + c;

                    if (index >= bufferLength)
                        continue;

                    pPixels[index] = ((unsigned char*)pPixelBuffer->m_pData)
                            [pPixelBuffer->m_Stride * y + 3 * (pPixelBuffer->m_Width - x - 1) + (2 - c)];
                }
    }
    else
        pPixels = (unsigned char*)pPixelBuffer->m_pData;

    // create new OpenGL texture
    glGenTextures(1, &index);
    glBindTexture(GL_TEXTURE_2D, index);

    // set texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set texture wrapping mode
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // generate texture from bitmap data
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 pixelType,
                 pPixelBuffer->m_Width,
                 pPixelBuffer->m_Height,
                 0,
                 pixelType,
                 GL_UNSIGNED_BYTE,
                 pPixels);

    // delete local pixel buffer
    if (pPixelBuffer->m_ImageType == CSR_IT_Bitmap)
        free(pPixels);

    return index;
}
//---------------------------------------------------------------------------
// Cubemap functions
//------------------------------------------------------------------------------
GLuint csrOpenGLCubemapLoad(const char** pFileNames)
{
    size_t i;

    // create a cubemap texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // iterate through cubemap faces to load
    for (i = 0; i < 6; ++i)
    {
        unsigned char* pPixels;
        int            doReleasePixels = 0;

        // load the texture content from file
        CSR_PixelBuffer* pPixelBuffer = csrPixelBufferFromBitmapFile(pFileNames[i]);

        // failed?
        if (!pPixelBuffer)
            continue;

        // reorder the pixels if image is a bitmap
        if (pPixelBuffer->m_ImageType == CSR_IT_Bitmap)
        {
            size_t x;
            size_t y;
            size_t c;

            const size_t bufferLength = sizeof(unsigned char)  *
                                        pPixelBuffer->m_Width  *
                                        pPixelBuffer->m_Height *
                                        3;

            doReleasePixels = 1;

            pPixels = (unsigned char*)malloc(bufferLength);

            if (!pPixels || !pPixelBuffer->m_Stride)
                continue;

            // get bitmap data into right format
            for (y = 0; y < pPixelBuffer->m_Height; ++y)
                for (x = 0; x < pPixelBuffer->m_Width; ++x)
                    for (c = 0; c < 3; ++c)
                    {
                        const size_t index = 3 * ((size_t)pPixelBuffer->m_Width * y + x) + c;

                        if (index >= bufferLength)
                            continue;

                        pPixels[index] = ((unsigned char*)pPixelBuffer->m_pData)
                                [pPixelBuffer->m_Stride * y + 3 * (pPixelBuffer->m_Width - x - 1) + (2 - c)];
                    }
        }
        else
            pPixels = (unsigned char*)pPixelBuffer->m_pData;

        // load the texture on the GPU
        glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i),
                     0,
                     GL_RGB,
                     pPixelBuffer->m_Width,
                     pPixelBuffer->m_Height,
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     pPixels);

        if (doReleasePixels)
            free(pPixels);

        // release the previously loaded bitmap
        csrPixelBufferRelease(pPixelBuffer);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    return textureID;
}
//---------------------------------------------------------------------------
// Identifier functions
//---------------------------------------------------------------------------
CSR_OpenGLID* csrOpenGLIDCreate(void)
{
    // create a new identifier
    CSR_OpenGLID* pID = (CSR_OpenGLID*)malloc(sizeof(CSR_OpenGLID));

    // succeeded?
    if (!pID)
        return 0;

    // initialize the shader content
    csrOpenGLIDInit(pID);

    return pID;
}
//---------------------------------------------------------------------------
void csrOpenGLIDRelease(CSR_OpenGLID* pID)
{
    // no identifier to release?
    if (!pID)
        return;

    // free the identifier
    free(pID);
}
//---------------------------------------------------------------------------
void csrOpenGLIDInit(CSR_OpenGLID* pID)
{
    // no identifier to initialize?
    if (!pID)
        return;

    // initialize the shader content
    pID->m_pKey     = 0;
    pID->m_UseCount = 0;
    pID->m_ID       = M_CSR_Error_Code;
}
//---------------------------------------------------------------------------
// Shader functions
//---------------------------------------------------------------------------
CSR_OpenGLShader* csrOpenGLShaderCreate(void)
{
    // create a new shader
    CSR_OpenGLShader* pShader = (CSR_OpenGLShader*)malloc(sizeof(CSR_OpenGLShader));

    // succeeded?
    if (!pShader)
        return 0;

    // initialize the shader content
    csrOpenGLShaderInit(pShader);

    return pShader;
}
//---------------------------------------------------------------------------
void csrOpenGLShaderRelease(CSR_OpenGLShader* pShader)
{
    // no shader to release?
    if (!pShader)
        return;

    // delete the fragment shader
    if (pShader->m_FragmentID)
        glDeleteShader(pShader->m_FragmentID);

    // delete the vertex shader
    if (pShader->m_VertexID)
        glDeleteShader(pShader->m_VertexID);

    // delete the shader program
    if (pShader->m_ProgramID)
        glDeleteProgram(pShader->m_ProgramID);

    // free the shader
    free(pShader);
}
//---------------------------------------------------------------------------
void csrOpenGLShaderInit(CSR_OpenGLShader* pShader)
{
    // no shader to initialize?
    if (!pShader)
        return;

    // initialize the shader content
    pShader->m_ProgramID    =  0;
    pShader->m_VertexID     =  0;
    pShader->m_FragmentID   =  0;
    pShader->m_VertexSlot   = -1;
    pShader->m_NormalSlot   = -1;
    pShader->m_TexCoordSlot = -1;
    pShader->m_TextureSlot  = -1;
    pShader->m_BumpMapSlot  = -1;
    pShader->m_CubemapSlot  = -1;
    pShader->m_ColorSlot    = -1;
    pShader->m_ModelSlot    = -1;
}
//---------------------------------------------------------------------------
CSR_OpenGLShader* csrOpenGLShaderLoadFromFile(const char*               pVertex,
                                              const char*               pFragment,
                                              const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                              const void*               pCustomData)
{
    CSR_Buffer*       pVertexProgram;
    CSR_Buffer*       pFragmentProgram;
    CSR_OpenGLShader* pShader;

    // open the vertex file to load
    pVertexProgram = csrFileOpen(pVertex);

    // succeeded?
    if (!pVertexProgram)
        return 0;

    // open the fragment file to load
    pFragmentProgram = csrFileOpen(pFragment);

    // succeeded?
    if (!pFragmentProgram)
    {
        csrBufferRelease(pVertexProgram);
        return 0;
    }

    // load the shader from opened vertex and fragment programs
    pShader = csrOpenGLShaderLoadFromBuffer(pVertexProgram,
                                            pFragmentProgram,
                                            fOnLinkStaticVB,
                                            pCustomData);

    // release the program buffers
    csrBufferRelease(pVertexProgram);
    csrBufferRelease(pFragmentProgram);

    return pShader;
}
//------------------------------------------------------------------------------
CSR_OpenGLShader* csrOpenGLShaderLoadFromStr(const char*               pVertex,
                                                   size_t              vertexLength,
                                             const char*               pFragment,
                                                   size_t              fragmentLength,
                                             const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                             const void*               pCustomData)
{
    CSR_Buffer*       pVS;
    CSR_Buffer*       pFS;
    CSR_OpenGLShader* pShader;

    // validate the inputs
    if (!pVertex || !vertexLength || !pFragment || !fragmentLength)
        return 0;

    // create buffers to contain vertex and fragment programs
    pVS = csrBufferCreate();
    pFS = csrBufferCreate();

    if (!pVS || !pFS)
        return 0;

    pVS->m_Length = vertexLength;
    pVS->m_pData  = (unsigned char*)malloc(pVS->m_Length + 1);

    if (!pVS->m_pData)
        return 0;

    // copy the vertex program to read
    memcpy(pVS->m_pData, pVertex, pVS->m_Length);
    ((unsigned char*)pVS->m_pData)[pVS->m_Length] = 0x0;

    pFS->m_Length = fragmentLength;
    pFS->m_pData  = (unsigned char*)malloc(pFS->m_Length + 1);

    if (!pFS->m_pData)
        return 0;

    // copy the fragment program to read
    memcpy(pFS->m_pData, pFragment, pFS->m_Length);
    ((unsigned char*)pFS->m_pData)[pFS->m_Length] = 0x0;

    // compile and build the shader
    pShader = csrOpenGLShaderLoadFromBuffer(pVS, pFS, fOnLinkStaticVB, pCustomData);

    // release the buffers
    csrBufferRelease(pVS);
    csrBufferRelease(pFS);

    return pShader;
}
//---------------------------------------------------------------------------
CSR_OpenGLShader* csrOpenGLShaderLoadFromBuffer(const CSR_Buffer*         pVertex,
                                                const CSR_Buffer*         pFragment,
                                                const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                                const void*               pCustomData)
{
    CSR_OpenGLShader* pShader;

    // source vertex or fragment program is missing?
    if (!pVertex || !pFragment)
        return 0;

    // create a new shader
    pShader = csrOpenGLShaderCreate();

    // succeeded?
    if (!pShader)
        return 0;

    // sometimes the OpenGL driver isn't compatible with Glew and this function isn't available.
    // Stop here if it's the case
    if (!glCreateProgram)
        return 0;

    // create a new shader program
    pShader->m_ProgramID = glCreateProgram();

    // succeeded?
    if (!pShader->m_ProgramID)
    {
        csrOpenGLShaderRelease(pShader);
        return 0;
    }

    // create and compile the vertex shader
    if (!csrOpenGLShaderCompile(pVertex, GL_VERTEX_SHADER, pShader))
    {
        csrOpenGLShaderRelease(pShader);
        return 0;
    }

    // create and compile the fragment shader
    if (!csrOpenGLShaderCompile(pFragment, GL_FRAGMENT_SHADER, pShader))
    {
        csrOpenGLShaderRelease(pShader);
        return 0;
    }

    // attach compiled programs with shader
    glAttachShader(pShader->m_ProgramID, pShader->m_VertexID);
    glAttachShader(pShader->m_ProgramID, pShader->m_FragmentID);

    // let the user attach static vertex buffers if he wants
    if (fOnLinkStaticVB)
        fOnLinkStaticVB(pShader, pCustomData);

    // link shader
    if (!csrOpenGLShaderLink(pShader))
    {
        csrOpenGLShaderRelease(pShader);
        return 0;
    }

    return pShader;
}
//---------------------------------------------------------------------------
int csrOpenGLShaderCompile(const CSR_Buffer* pSource, GLenum shaderType, CSR_OpenGLShader* pShader)
{
    GLuint shaderID;
    GLint  success;

    // no source data to compile?
    if (!pSource || !pSource->m_Length || !pSource->m_pData)
        return 0;

    // no shader to add result to?
    if (!pShader)
        return 0;

    // create a new shader program
    shaderID = glCreateShader(shaderType);

    // succeeded?
    if (!shaderID)
        return 0;

    // compile the shader program
    #ifdef __APPLE__
        glShaderSource(shaderID, 1, (const GLchar* const*)&pSource->m_pData, 0);
    #else
        glShaderSource(shaderID, 1, (GLchar**)&pSource->m_pData, 0);
    #endif
    glCompileShader(shaderID);

    // get compiler result
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

    // succeeded?
    if (success == GL_FALSE)
    {
        // delete the shader
        glDeleteShader(shaderID);
        return 0;
    }

    // assign the compiled shader to the correct identifier
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:   pShader->m_VertexID   = shaderID; break;
        case GL_FRAGMENT_SHADER: pShader->m_FragmentID = shaderID; break;
        default:                 glDeleteShader(shaderID);         return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrOpenGLShaderLink(CSR_OpenGLShader* pShader)
{
    GLint success;

    // no shader to link?
    if (!pShader || !pShader->m_ProgramID)
        return 0;

    // link the shader
    glLinkProgram(pShader->m_ProgramID);

    // get linker result
    glGetProgramiv(pShader->m_ProgramID, GL_LINK_STATUS, &success);

    // succeeded?
    if (success == GL_FALSE)
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
void csrOpenGLShaderEnable(const CSR_OpenGLShader* pShader)
{
    // no shader to enable?
    if (!pShader)
    {
        // disable all
        glUseProgram(0);
        return;
    }

    // enable the shader
    glUseProgram(pShader->m_ProgramID);
}
//---------------------------------------------------------------------------
void csrOpenGLShaderConnectProjectionMatrix(const CSR_OpenGLShader* pShader,
                                            const CSR_Matrix4*      pMatrix)
{
    GLint slot;

    // get the projection matrix slot from shader
    slot = glGetUniformLocation(pShader->m_ProgramID, "csr_uProjection");

    // connect the projection matrix to the shader
    if (slot >= 0)
        glUniformMatrix4fv(slot, 1, 0, &pMatrix->m_Table[0][0]);
}
//---------------------------------------------------------------------------
void csrOpenGLShaderConnectViewMatrix(const CSR_OpenGLShader* pShader,
                                      const CSR_Matrix4*      pMatrix)
{
    GLint slot;

    // get the view matrix slot from shader
    slot = glGetUniformLocation(pShader->m_ProgramID, "csr_uView");

    // connect the view matrix to the shader
    if (slot >= 0)
        glUniformMatrix4fv(slot, 1, 0, &pMatrix->m_Table[0][0]);
}
//---------------------------------------------------------------------------
// Static buffer functions
//---------------------------------------------------------------------------
CSR_OpenGLStaticBuffer* csrOpenGLStaticBufferCreate(const CSR_OpenGLShader* pShader,
                                                    const CSR_Buffer*       pBuffer)
{
    CSR_OpenGLStaticBuffer* pSB;

    // no shader?
    if (!pShader)
        return 0;

    // validate the inputs
    if (!pBuffer || !pBuffer->m_Length)
        return 0;

    // create a static buffer
    pSB = (CSR_OpenGLStaticBuffer*)malloc(sizeof(CSR_OpenGLStaticBuffer));

    // succeeded?
    if (!pSB)
        return 0;

    // initialize the static buffer
    csrOpenGLStaticBufferInit(pSB);

    // create a Vertex Buffer Object (VBO) on the GPU size
    glGenBuffers(1, &pSB->m_BufferID);

    // bind the newly created VBO
    glBindBuffer(GL_ARRAY_BUFFER, pSB->m_BufferID);

    // copy the buffer data content in the VBO
    glBufferData(GL_ARRAY_BUFFER,
                 pBuffer->m_Length * sizeof(float),
                 ((float*)pBuffer->m_pData),
                 GL_STATIC_DRAW);

    // unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return pSB;
}
//---------------------------------------------------------------------------
void csrOpenGLStaticBufferRelease(CSR_OpenGLStaticBuffer* pSB)
{
    // no static buffer to release?
    if (!pSB)
        return;

    // free the static buffer content
    if (pSB->m_BufferID != M_CSR_Error_Code)
        glDeleteBuffers(1, &pSB->m_BufferID);

    // free the static buffer
    free(pSB);
}
//---------------------------------------------------------------------------
void csrOpenGLStaticBufferInit(CSR_OpenGLStaticBuffer* pSB)
{
    // no static buffer to initialize?
    if (!pSB)
        return;

    // initialize the static buffer content
    pSB->m_BufferID = M_CSR_Error_Code;
    pSB->m_Stride   = 0;
}
//---------------------------------------------------------------------------
// Multisample antialiasing shader
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    const char g_MSAA_VertexProgram[] =
        "attribute vec2 csr_vVertex;"
        "attribute vec2 csr_vTexCoord;"
        "varying   vec2 csr_fTexCoord;"
        "void main()"
        "{"
        "    gl_Position   = vec4(csr_vVertex.x, csr_vVertex.y, 0.0f, 1.0f);"
        "    csr_fTexCoord = csr_vTexCoord;"
        "}";
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    const char g_MSAA_FragmentProgram[] =
        "uniform sampler2D csr_sTexture;"
        "varying vec2      csr_fTexCoord;"
        "void main()"
        "{"
        "    gl_FragColor = texture(csr_sTexture, csr_fTexCoord);"
        "}";
#endif
//---------------------------------------------------------------------------
// Multisample antialiasing constants
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    const float g_MSAA_VB[] =
    {
        // x,   y,    tu,   tv
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f
    };
#endif
//---------------------------------------------------------------------------
// Multisample antialiasing private functions
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrOpenGLMSAALinkStaticVB(const CSR_OpenGLShader* pShader, const void* pCustomData)
    {
        #ifdef _MSC_VER
            CSR_Buffer buffer = {0};
        #else
            CSR_Buffer buffer;
        #endif

        // get the multisampling antialiasing sent in custom data
        CSR_OpenGLMSAA* pMSAA = (CSR_OpenGLMSAA*)pCustomData;

        // found it?
        if (!pMSAA)
            return;

        // configure the vertex buffer
        buffer.m_pData  = (void*)g_MSAA_VB;
        buffer.m_Length = sizeof(g_MSAA_VB);

        // create the shape on which the final texture will be drawn
        pMSAA->m_pStaticBuffer           = csrOpenGLStaticBufferCreate(pShader, &buffer);
        pMSAA->m_pStaticBuffer->m_Stride = 4;
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    int csrOpenGLMSAAConfigure(size_t width, size_t height, size_t factor, CSR_OpenGLMSAA* pMSAA)
    {
        GLuint msTexture;

        // enable multisampling
        glEnable(GL_MULTISAMPLE);

        // create and bind the frame buffer
        glGenFramebuffers(1, &pMSAA->m_FrameBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, pMSAA->m_FrameBufferID);

        // create a new texture
        glGenTextures(1, &msTexture);

        // bind texture to sampler
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (GLsizei)factor, GL_RGB, (GLsizei)width, (GLsizei)height, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

        // add texture to the frame buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D_MULTISAMPLE,
                               msTexture,
                               0);

        // create and bind the render buffer for depth and stencil attachments
        glGenRenderbuffers(1, &pMSAA->m_RenderBufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, pMSAA->m_RenderBufferID);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                         (GLsizei)factor,
                                         GL_DEPTH24_STENCIL8,
                                         (GLsizei)width,
                                         (GLsizei)height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  pMSAA->m_RenderBufferID);

        // build the render buffer
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return 0;

        // unbind the frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // create multisampled output texture
        glGenTextures(1, &pMSAA->m_TextureID);
        glBindTexture(GL_TEXTURE_2D, pMSAA->m_TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLint)width, (GLint)height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

        // configure texture filters to use and bind texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // create and bind a multisampled texture buffer
        glGenFramebuffers(1, &pMSAA->m_TextureBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, pMSAA->m_TextureBufferID);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               pMSAA->m_TextureID,
                               0);

        // build the multisampled texture buffer
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return 0;

        // unbind the texture buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // set the viewport size
        pMSAA->m_Width  = width;
        pMSAA->m_Height = height;
        pMSAA->m_Factor = factor;

        return 1;
    }
#endif
//---------------------------------------------------------------------------
// Multisample antialiasing functions
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    CSR_OpenGLMSAA* csrOpenGLMSAACreate(size_t width, size_t height, size_t factor)
    {
        // create a new multisample antialiasing
        CSR_OpenGLMSAA* pMSAA = (CSR_OpenGLMSAA*)malloc(sizeof(CSR_OpenGLMSAA));

        // succeeded?
        if (!pMSAA)
            return 0;

        // initialize the multisample antialiasing content
        if (!csrOpenGLMSAAInit(width, height, factor, pMSAA))
        {
            csrOpenGLMSAARelease(pMSAA);
            return 0;
        }

        return pMSAA;
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrOpenGLMSAARelease(CSR_OpenGLMSAA* pMSAA)
    {
        // no multisampling antialiasing to release?
        if (!pMSAA)
            return;

        // enable the MSAA shader
        if (pMSAA->m_pShader)
            csrOpenGLShaderEnable(pMSAA->m_pShader);

        // delete the multisampled texture
        if (pMSAA->m_TextureID)
            glDeleteTextures(1, &pMSAA->m_TextureID);

        // delete the multisampled texture buffer
        if (pMSAA->m_TextureBufferID)
            glDeleteFramebuffers(1, &pMSAA->m_TextureBufferID);

        // delete the render buffer
        if (pMSAA->m_RenderBufferID)
            glDeleteRenderbuffers(1, &pMSAA->m_RenderBufferID);

        // delete the frame buffer
        if (pMSAA->m_FrameBufferID)
            glDeleteFramebuffers(1, &pMSAA->m_FrameBufferID);

        // delete the shader
        if (pMSAA->m_pShader)
            csrOpenGLShaderRelease(pMSAA->m_pShader);

        // delete the static buffer
        if (pMSAA->m_pStaticBuffer)
            csrOpenGLStaticBufferRelease(pMSAA->m_pStaticBuffer);

        // delete the multisampling antialiasing
        free(pMSAA);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    int csrOpenGLMSAAInit(size_t width, size_t height, size_t factor, CSR_OpenGLMSAA* pMSAA)
    {
        // no multisample antialiasing to initialize?
        if (!pMSAA)
            return 0;

        // validate input
        if (factor != 2 && factor != 4 && factor != 8)
            return 0;

        // load the shader to use for MSAA
        pMSAA->m_pShader = csrOpenGLShaderLoadFromStr(g_MSAA_VertexProgram,
                                                      sizeof(g_MSAA_VertexProgram),
                                                      g_MSAA_FragmentProgram,
                                                      sizeof(g_MSAA_FragmentProgram),
                                                      csrOpenGLMSAALinkStaticVB,
                                                      pMSAA);

        // succeeded?
        if (!pMSAA->m_pShader)
            return 0;

        // the static buffer containing the final surface to show was generated successfully?
        if (!pMSAA->m_pStaticBuffer)
            return 0;

        // enable the MSAA shader
        csrOpenGLShaderEnable(pMSAA->m_pShader);

        // get the vertex slot
        pMSAA->m_pShader->m_VertexSlot =
                glGetAttribLocation(pMSAA->m_pShader->m_ProgramID, "csr_vVertex");

        // found it?
        if (pMSAA->m_pShader->m_VertexSlot == -1)
            return 0;

        // get the texture coordinates slot
        pMSAA->m_pShader->m_TexCoordSlot =
                glGetAttribLocation(pMSAA->m_pShader->m_ProgramID, "csr_vTexCoord");

        // found it?
        if (pMSAA->m_pShader->m_TexCoordSlot == -1)
            return 0;

        // get the texture sampler slot
        pMSAA->m_pShader->m_TextureSlot =
                glGetUniformLocation(pMSAA->m_pShader->m_ProgramID, "csr_sTexture");

        // found it?
        if (pMSAA->m_pShader->m_TextureSlot == -1)
            return 0;

        // configure the multisample antialiasing
        return csrOpenGLMSAAConfigure(width, height, factor, pMSAA);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    int csrOpenGLMSAAChangeSize(size_t width, size_t height, CSR_OpenGLMSAA* pMSAA)
    {
        // validate the input
        if (!pMSAA)
            return 0;

        // delete the multisampled texture
        if (pMSAA->m_TextureID)
        {
            glDeleteTextures(1, &pMSAA->m_TextureID);
            pMSAA->m_TextureID = M_CSR_Error_Code;
        }

        // delete the multisampled texture buffer
        if (pMSAA->m_TextureBufferID)
        {
            glDeleteFramebuffers(1, &pMSAA->m_TextureBufferID);
            pMSAA->m_TextureBufferID = M_CSR_Error_Code;
        }

        // delete the render buffer
        if (pMSAA->m_RenderBufferID)
        {
            glDeleteRenderbuffers(1, &pMSAA->m_RenderBufferID);
            pMSAA->m_RenderBufferID = M_CSR_Error_Code;
        }

        // delete the frame buffer
        if (pMSAA->m_FrameBufferID)
        {
            glDeleteFramebuffers(1, &pMSAA->m_FrameBufferID);
            pMSAA->m_FrameBufferID = M_CSR_Error_Code;
        }

        // recreate the multisample antialiasing
        return csrOpenGLMSAAConfigure(width, height, pMSAA->m_Factor, pMSAA);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrOpenGLMSAADrawBegin(const CSR_Color* pColor, const CSR_OpenGLMSAA* pMSAA)
    {
        // do apply a multisample antialiasing?
        if (pMSAA && pMSAA->m_pShader)
        {
            // enable the MSAA shader
            csrOpenGLShaderEnable(pMSAA->m_pShader);

            // enable multisampling
            glEnable(GL_MULTISAMPLE);

            // bind the frame buffer on which the final rendering should be performed
            glBindFramebuffer(GL_FRAMEBUFFER, pMSAA->m_FrameBufferID);
        }

        // begin the draw
        csrOpenGLDrawBegin(pColor);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrOpenGLMSAADrawEnd(const CSR_OpenGLMSAA* pMSAA)
    {
        // end the draw
        csrOpenGLDrawEnd();

        // do finalize the multisampling antialiasing effect?
        if (pMSAA && pMSAA->m_pShader && pMSAA->m_pStaticBuffer)
        {
            // configure the culling
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glFrontFace(GL_CW);

            // disable the alpha blending
            glDisable(GL_BLEND);

            // set polygon mode to fill
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // enable the MSAA shader
            csrOpenGLShaderEnable(pMSAA->m_pShader);

            // blit the multisampled buffer containing the drawing to the output texture buffer
            glBindFramebuffer(GL_READ_FRAMEBUFFER, pMSAA->m_FrameBufferID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pMSAA->m_TextureBufferID);
            glBlitFramebuffer(0,
                              0,
                              (GLint)pMSAA->m_Width,
                              (GLint)pMSAA->m_Height,
                              0,
                              0,
                              (GLint)pMSAA->m_Width,
                              (GLint)pMSAA->m_Height,
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);

            // unbind the frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // configure the depth testing
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            // select the texture sampler to use (GL_TEXTURE0 for normal textures)
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(pMSAA->m_pShader->m_TextureSlot, GL_TEXTURE0);

            // bind the texture to use
            glBindTexture(GL_TEXTURE_2D, pMSAA->m_TextureID);

            // bind the VBO containing the shape to draw
            glBindBuffer(GL_ARRAY_BUFFER, pMSAA->m_pStaticBuffer->m_BufferID);

            // enable the vertices
            glEnableVertexAttribArray(pMSAA->m_pShader->m_VertexSlot);
            glVertexAttribPointer(pMSAA->m_pShader->m_VertexSlot,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  (GLsizei)(pMSAA->m_pStaticBuffer->m_Stride * sizeof(float)),
                                  0);

            // enable the texture coordinates
            glEnableVertexAttribArray(pMSAA->m_pShader->m_TexCoordSlot);
            glVertexAttribPointer(pMSAA->m_pShader->m_TexCoordSlot,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  (GLsizei)(pMSAA->m_pStaticBuffer->m_Stride * sizeof(float)),
                                  (void*)(2 * sizeof(float)));

            // draw the surface
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // disable the vertex attribute arrays
            glDisableVertexAttribArray(pMSAA->m_pShader->m_TexCoordSlot);
            glDisableVertexAttribArray(pMSAA->m_pShader->m_VertexSlot);

            // unbind the VBO
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
#endif
//---------------------------------------------------------------------------
// Draw private functions
//---------------------------------------------------------------------------
void csrOpenGLDrawArray(const CSR_VertexBuffer* pVB, size_t vertexCount)
{
    // search for array type to draw
    switch (pVB->m_Format.m_Type)
    {
        case CSR_VT_Triangles:     glDrawArrays(GL_TRIANGLES,      0, (GLsizei)vertexCount); return;
        case CSR_VT_TriangleStrip: glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vertexCount); return;
        case CSR_VT_TriangleFan:   glDrawArrays(GL_TRIANGLE_FAN,   0, (GLsizei)vertexCount); return;
        default:                                                                             return;
    }
}
//---------------------------------------------------------------------------
// Draw functions
//---------------------------------------------------------------------------
void csrOpenGLDrawBegin(const CSR_Color* pColor)
{
    // no background color?
    if (!pColor)
        return;

    // clear background and depth buffer
    glClearColor(pColor->m_R, pColor->m_G, pColor->m_B, pColor->m_A);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // configure the OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);
}
//---------------------------------------------------------------------------
void csrOpenGLDrawEnd(void)
{}
//---------------------------------------------------------------------------
void csrOpenGLDrawLine(const CSR_Line* pLine, const CSR_OpenGLShader* pShader)
{
    #ifdef _MSC_VER
        GLint  slot;
        size_t stride;
        float  lineVertex[14] = {0};
    #else
        GLint  slot;
        size_t stride;
        float  lineVertex[14];
    #endif

    // validate the inputs
    if (!pLine || !pShader || pLine->m_Width <= 0.0f)
        return;

    // set the line width to use
    glLineWidth(pLine->m_Width);

    #ifndef CSR_OPENGL_2_ONLY
        // do draw smooth lines?
        if (pLine->m_Smooth)
        {
            // enabled the line smoothing mode
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        }
    #endif

    // bind shader program
    csrOpenGLShaderEnable(pShader);

    // do use a default model matrix?
    if (!pLine->m_CustomModelMat)
    {
        // get the model matrix slot from shader
        slot = glGetUniformLocation(pShader->m_ProgramID, "csr_uModel");

        // found it?
        if (slot >= 0)
        {
            CSR_Matrix4 matrix;
            csrMat4Identity(&matrix);

            // connect default model matrix to shader
            glUniformMatrix4fv(slot, 1, GL_FALSE, &matrix.m_Table[0][0]);
        }
    }

    // generate the line vertex buffer
    lineVertex[0]  = pLine->m_Start.m_X;
    lineVertex[1]  = pLine->m_Start.m_Y;
    lineVertex[2]  = pLine->m_Start.m_Z;
    lineVertex[3]  = pLine->m_StartColor.m_R;
    lineVertex[4]  = pLine->m_StartColor.m_G;
    lineVertex[5]  = pLine->m_StartColor.m_B;
    lineVertex[6]  = pLine->m_StartColor.m_A;
    lineVertex[7]  = pLine->m_End.m_X;
    lineVertex[8]  = pLine->m_End.m_Y;
    lineVertex[9]  = pLine->m_End.m_Z;
    lineVertex[10] = pLine->m_EndColor.m_R;
    lineVertex[11] = pLine->m_EndColor.m_G;
    lineVertex[12] = pLine->m_EndColor.m_B;
    lineVertex[13] = pLine->m_EndColor.m_A;

    stride = 7;

    // found it?
    if (pShader->m_VertexSlot < 0)
        return;

    // found it?
    if (pShader->m_ColorSlot < 0)
        return;

    // enable shader slots
    glEnableVertexAttribArray(pShader->m_VertexSlot);
    glEnableVertexAttribArray(pShader->m_ColorSlot);

    // link the line buffer to the shader
    glVertexAttribPointer(pShader->m_VertexSlot,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          (GLsizei)(stride * sizeof(float)),
                          &lineVertex[0]);
    glVertexAttribPointer(pShader->m_ColorSlot,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          (GLsizei)(stride * sizeof(float)),
                          &lineVertex[3]);

    // draw the line
    glDrawArrays(GL_LINES, 0, 2);

    // disable shader slots
    glDisableVertexAttribArray(pShader->m_VertexSlot);
    glDisableVertexAttribArray(pShader->m_ColorSlot);

    // unbind shader program
    csrOpenGLShaderEnable(0);
}
//---------------------------------------------------------------------------
void csrOpenGLDrawVertexBuffer(const CSR_VertexBuffer* pVB,
                               const CSR_OpenGLShader* pShader,
                               const CSR_Array*        pMatrixArray)
{
    GLvoid* pCoords;
    GLvoid* pNormals;
    GLvoid* pTexCoords;
    GLvoid* pColors;
    size_t  i;
    size_t  offset;
    size_t  vertexCount;

    // no vertex buffer to draw?
    if (!pVB)
        return;

    // no shader?
    if (!pShader)
        return;

    // check if vertex buffer is empty, skip to next if yes
    if (!pVB->m_Count || !pVB->m_Format.m_Stride)
        return;

    // configure the culling
    switch (pVB->m_Culling.m_Type)
    {
        case CSR_CT_None:  glDisable(GL_CULL_FACE); glCullFace(GL_NONE);           break;
        case CSR_CT_Front: glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT);          break;
        case CSR_CT_Back:  glEnable(GL_CULL_FACE);  glCullFace(GL_BACK);           break;
        case CSR_CT_Both:  glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT_AND_BACK); break;
        default:           glDisable(GL_CULL_FACE); glCullFace(GL_NONE);           break;
    }

    // configure the culling face
    switch (pVB->m_Culling.m_Face)
    {
        case CSR_CF_CW:  glFrontFace(GL_CW);  break;
        case CSR_CF_CCW: glFrontFace(GL_CCW); break;
    }

    // configure the alpha blending
    if (pVB->m_Material.m_Transparent)
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    // configure the wireframe mode
    #ifndef CSR_OPENGL_2_ONLY
        if (pVB->m_Material.m_Wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    #endif

    // enable vertex slot
    glEnableVertexAttribArray(pShader->m_VertexSlot);

    // enable normal slot
    if (pVB->m_Format.m_HasNormal)
        glEnableVertexAttribArray(pShader->m_NormalSlot);

    // enable texture slot
    if (pVB->m_Format.m_HasTexCoords)
        glEnableVertexAttribArray(pShader->m_TexCoordSlot);

    // enable color slot
    if (pVB->m_Format.m_HasPerVertexColor)
        glEnableVertexAttribArray(pShader->m_ColorSlot);

    offset = 0;

    // send vertices to shader
    pCoords = &pVB->m_pData[offset];
    glVertexAttribPointer(pShader->m_VertexSlot,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          pVB->m_Format.m_Stride * sizeof(float),
                          pCoords);

    offset += 3;

    // vertices have normals?
    if (pVB->m_Format.m_HasNormal)
    {
        // send normals to shader
        pNormals = &pVB->m_pData[offset];
        glVertexAttribPointer(pShader->m_NormalSlot,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              pVB->m_Format.m_Stride * sizeof(float),
                              pNormals);

        offset += 3;
    }

    // vertices have UV texture coordinates?
    if (pVB->m_Format.m_HasTexCoords)
    {
        // send textures to shader
        pTexCoords = &pVB->m_pData[offset];
        glVertexAttribPointer(pShader->m_TexCoordSlot,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              pVB->m_Format.m_Stride * sizeof(float),
                              pTexCoords);

        offset += 2;
    }

    // vertices have per-vertex color?
    if (pVB->m_Format.m_HasPerVertexColor)
    {
        // send colors to shader
        pColors = &pVB->m_pData[offset];
        glVertexAttribPointer(pShader->m_ColorSlot,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              pVB->m_Format.m_Stride * sizeof(float),
                              pColors);
    }
    else
    if (pShader->m_ColorSlot != -1)
    {
        // get the color component values
        const float r = (float)((pVB->m_Material.m_Color >> 24) & 0xFF) / 255.0f;
        const float g = (float)((pVB->m_Material.m_Color >> 16) & 0xFF) / 255.0f;
        const float b = (float)((pVB->m_Material.m_Color >> 8)  & 0xFF) / 255.0f;
        const float a = (float) (pVB->m_Material.m_Color        & 0xFF) / 255.0f;

        // connect the vertex color to the shader
        glVertexAttrib4f(pShader->m_ColorSlot, r, g, b, a);
    }

    // calculate the vertex count
    vertexCount = pVB->m_Count / pVB->m_Format.m_Stride;

    // do draw the vertex buffer several times?
    if (pMatrixArray && pMatrixArray->m_Count)
    {
        // get the model matrix slot from shader
        const GLint slot = glGetUniformLocation(pShader->m_ProgramID, "csr_uModel");

        // found it?
        if (slot >= 0)
            // yes, iterate through each matrix to use to draw the vertex buffer
            for (i = 0; i < pMatrixArray->m_Count; ++i)
            {
                // connect the model matrix to the shader
                glUniformMatrix4fv(slot,
                                   1,
                                   0,
                                   &((CSR_Matrix4*)pMatrixArray->m_pItem[i].m_pData)->m_Table[0][0]);

                // draw the next buffer
                csrOpenGLDrawArray(pVB, vertexCount);
            }
    }
    else
        // no, simply draw the buffer without worrying about the model matrix
        csrOpenGLDrawArray(pVB, vertexCount);

    // disable vertices slots from shader
    glDisableVertexAttribArray(pShader->m_VertexSlot);

    // disable normal slot
    if (pVB->m_Format.m_HasNormal)
        glDisableVertexAttribArray(pShader->m_NormalSlot);

    // disable texture slot
    if (pVB->m_Format.m_HasTexCoords)
        glDisableVertexAttribArray(pShader->m_TexCoordSlot);

    // disable color slot
    if (pVB->m_Format.m_HasPerVertexColor)
        glDisableVertexAttribArray(pShader->m_ColorSlot);
}
//---------------------------------------------------------------------------
void csrOpenGLDrawMesh(const CSR_Mesh*         pMesh,
                       const CSR_OpenGLShader* pShader,
                       const CSR_Array*        pMatrixArray,
                       const CSR_fOnGetID      fOnGetID)
{
    size_t i;

    // no mesh to draw?
    if (!pMesh)
        return;

    // no shader?
    if (!pShader)
        return;

    // enable the shader to use for drawing
    csrOpenGLShaderEnable(pShader);

    // iterate through the vertex buffers composing the mesh to draw
    for (i = 0; i < pMesh->m_Count; ++i)
    {
        // in order to link textures, the OnGetID callback should be defined
        if (fOnGetID)
        {
            CSR_OpenGLID* pTextureID;
            CSR_OpenGLID* pBumpmapID;
            CSR_OpenGLID* pCubemapID;

            // vertices have UV texture coordinates?
            if (pMesh->m_pVB[i].m_Format.m_HasTexCoords)
            {
                // get the OpenGL texture and bump map resource identifiers for this mesh
                pTextureID = (CSR_OpenGLID*)fOnGetID(&pMesh->m_Skin.m_Texture);
                pBumpmapID = (CSR_OpenGLID*)fOnGetID(&pMesh->m_Skin.m_BumpMap);

                // a texture is defined for this mesh?
                if (pTextureID && (GLuint)pTextureID->m_ID != M_CSR_Error_Code)
                {
                    // select the texture sampler to use (GL_TEXTURE0 for normal textures)
                    glActiveTexture(GL_TEXTURE0);
                    glUniform1i(pShader->m_TextureSlot, GL_TEXTURE0);

                    // bind the texture to use
                    glBindTexture(GL_TEXTURE_2D, pTextureID->m_ID);
                }

                // a bump map is defined for this mesh?
                if (pBumpmapID && (GLuint)pBumpmapID->m_ID != M_CSR_Error_Code)
                {
                    // select the texture sampler to use (GL_TEXTURE1 for bump map textures)
                    glActiveTexture(GL_TEXTURE1);
                    glUniform1i(pShader->m_BumpMapSlot, GL_TEXTURE1);

                    // bind the texture to use
                    glBindTexture(GL_TEXTURE_2D, pBumpmapID->m_ID);
                }
            }

            // get the OpenGL cubemap resource identifier for this mesh
            pCubemapID = (CSR_OpenGLID*)fOnGetID(&pMesh->m_Skin.m_CubeMap);

            // a cube map is defined for this mesh?
            if (pCubemapID && (GLuint)pCubemapID->m_ID != M_CSR_Error_Code)
            {
                // select the texture sampler to use (GL_TEXTURE0 for cubemap textures)
                //glActiveTexture(GL_TEXTURE0);
                //glUniform1i(pShader->m_CubemapSlot, GL_TEXTURE0);

                // bind the cubemap texture to use
                glBindTexture(GL_TEXTURE_CUBE_MAP, pCubemapID->m_ID);
            }
        }

        // draw the next mesh vertex buffer
        csrOpenGLDrawVertexBuffer(&pMesh->m_pVB[i], pShader, pMatrixArray);
    }
}
//---------------------------------------------------------------------------
void csrOpenGLDrawModel(const CSR_Model*        pModel,
                              size_t            index,
                        const CSR_OpenGLShader* pShader,
                        const CSR_Array*        pMatrixArray,
                        const CSR_fOnGetID      fOnGetID)
{
    // no model to draw?
    if (!pModel)
        return;

    // draw the model mesh
    csrOpenGLDrawMesh(&pModel->m_pMesh[index % pModel->m_MeshCount],
                       pShader,
                       pMatrixArray,
                       fOnGetID);
}
//---------------------------------------------------------------------------
void csrOpenGLDrawMDL(const CSR_MDL*          pMDL,
                      const CSR_OpenGLShader* pShader,
                      const CSR_Array*        pMatrixArray,
                            size_t            skinIndex,
                            size_t            modelIndex,
                            size_t            meshIndex,
                      const CSR_fOnGetID      fOnGetID)
{
    // get the current model mesh to draw
    const CSR_Mesh* pMesh = csrMDLGetMesh(pMDL, modelIndex, meshIndex);

    // found it?
    if (!pMesh)
        return;

    // normally each mesh should contain only one vertex buffer
    if (pMesh->m_Count != 1)
        // unsupported if not (because cannot know which texture should be binded. If a such model
        // exists, a custom version of this function should also be written for it)
        return;

    // can use texture?
    if (fOnGetID && pMesh->m_pVB->m_Format.m_HasTexCoords && skinIndex < pMDL->m_SkinCount)
    {
        // get the OpenGL identifier matching with the texture
        const CSR_OpenGLID* pTextureID = (CSR_OpenGLID*)fOnGetID(&pMDL->m_pSkin[skinIndex].m_Texture);

        // found it?
        if (pTextureID && (GLuint)pTextureID->m_ID != M_CSR_Error_Code)
        {
            // select the texture sampler to use (GL_TEXTURE0 for normal textures)
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(pShader->m_TextureSlot, GL_TEXTURE0);

            // bind the texture to use
            glBindTexture(GL_TEXTURE_2D, pTextureID->m_ID);
        }
    }

    // draw the model mesh
    csrOpenGLDrawMesh(pMesh, pShader, pMatrixArray, fOnGetID);
}
//---------------------------------------------------------------------------
void csrOpenGLDrawX(const CSR_X*            pX,
                    const CSR_OpenGLShader* pShader,
                    const CSR_Array*        pMatrixArray,
                          size_t            animSetIndex,
                          size_t            frameIndex,
                    const CSR_fOnGetID      fOnGetID)
{
    size_t i;
    size_t j;
    size_t k;
    size_t l;

    // no model to draw?
    if (!pX || !pX->m_MeshCount)
        return;

    // do draw only the mesh and ignore all other data like bones?
    if (pX->m_MeshOnly)
    {
        // iterate through the meshes to draw
        for (i = 0; i < pX->m_MeshCount; ++i)
            // draw the model mesh
            csrOpenGLDrawMesh(&pX->m_pMesh[i], pShader, pMatrixArray, fOnGetID);

        return;
    }

    // iterate through the meshes to draw
    for (i = 0; i < pX->m_MeshCount; ++i)
    {
        int        useLocalMatrixArray;
        int        useSourceBuffer;
        CSR_Mesh*  pMesh;
        CSR_Mesh*  pLocalMesh;
        CSR_Array* pLocalMatrixArray;

        // if mesh has no skeleton, perform a simple draw
        if (!pX->m_pSkeleton)
        {
            // draw the model mesh
            csrOpenGLDrawMesh(&pX->m_pMesh[i], pShader, pMatrixArray, fOnGetID);
            return;
        }

        // get the current model mesh to draw
        pMesh = &pX->m_pMesh[i];

        // found it?
        if (!pMesh)
            continue;

        // normally each mesh should contain only one vertex buffer
        if (pMesh->m_Count != 1)
            // unsupported if not (because cannot know which texture should be binded. If a such model
            // exists, a custom version of this function should also be written for it)
            continue;

        // create a local mesh to contain the processed frame to draw
        pLocalMesh = csrMeshCreate();

        if (!pLocalMesh)
            continue;

        // bind the source mesh to the local one. Don't need to take care of copy the pointers, because
        // the source mesh will remain valid during the whole local mesh lifetime. Just don't delete
        // them on the loop end
        pLocalMesh->m_Skin = pMesh->m_Skin;
        pLocalMesh->m_Time = pMesh->m_Time;

        // mesh contains skin weights?
        if (pX->m_pMeshWeights[i].m_pSkinWeights)
        {
            useSourceBuffer = 0;

            // allocate memory for the final vertex buffer to draw
            pLocalMesh->m_pVB   = (CSR_VertexBuffer*)malloc(pMesh->m_Count * sizeof(CSR_VertexBuffer));
            pLocalMesh->m_Count = pMesh->m_Count;

            if (!pLocalMesh->m_pVB || !pLocalMesh->m_Count)
            {
                free(pLocalMesh);
                continue;
            }

            // bind the source vertex buffer to the local one
            pLocalMesh->m_pVB->m_Format   = pMesh->m_pVB->m_Format;
            pLocalMesh->m_pVB->m_Culling  = pMesh->m_pVB->m_Culling;
            pLocalMesh->m_pVB->m_Material = pMesh->m_pVB->m_Material;
            pLocalMesh->m_pVB->m_Time     = pMesh->m_pVB->m_Time;

            // allocate memory for the vertex buffer data
            pLocalMesh->m_pVB->m_pData = (float*)calloc(pMesh->m_pVB->m_Count, sizeof(float));
            pLocalMesh->m_pVB->m_Count = pMesh->m_pVB->m_Count;

            if (!pLocalMesh->m_pVB->m_pData || !pLocalMesh->m_pVB->m_Count)
            {
                free(pLocalMesh->m_pVB);
                free(pLocalMesh);
                continue;
            }

            // iterate through mesh skin weights
            for (j = 0; j < pX->m_pMeshWeights[i].m_Count; ++j)
            {
                CSR_Matrix4 boneMatrix;
                CSR_Matrix4 finalMatrix;

                // get the bone matrix
                if (pX->m_PoseOnly)
                    csrBoneGetMatrix(pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone, 0, &boneMatrix);
                else
                    csrBoneGetAnimMatrix(pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone,
                                        &pX->m_pAnimationSet[animSetIndex],
                                         frameIndex,
                                         0,
                                        &boneMatrix);

                // get the final matrix after bones transform
                csrMat4Multiply(&pX->m_pMeshWeights[i].m_pSkinWeights[j].m_Matrix,
                                &boneMatrix,
                                &finalMatrix);

                // apply the bone and its skin weights to each vertices
                for (k = 0; k < pX->m_pMeshWeights[i].m_pSkinWeights[j].m_IndexTableCount; ++k)
                    for (l = 0; l < pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_Count; ++l)
                    {
                        #ifdef _MSC_VER
                            size_t      iX;
                            size_t      iY;
                            size_t      iZ;
                            CSR_Vector3 inputVertex  = {0};
                            CSR_Vector3 outputVertex = {0};
                        #else
                            size_t      iX;
                            size_t      iY;
                            size_t      iZ;
                            CSR_Vector3 inputVertex;
                            CSR_Vector3 outputVertex;
                        #endif

                        // get the next vertex to which the next skin weight should be applied
                        iX = pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l];
                        iY = pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 1;
                        iZ = pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 2;

                        // get input vertex
                        inputVertex.m_X = pMesh->m_pVB->m_pData[iX];
                        inputVertex.m_Y = pMesh->m_pVB->m_pData[iY];
                        inputVertex.m_Z = pMesh->m_pVB->m_pData[iZ];

                        // apply bone transformation to vertex
                        csrMat4Transform(&finalMatrix, &inputVertex, &outputVertex);

                        // apply the skin weights and calculate the final output vertex
                        pLocalMesh->m_pVB->m_pData[iX] += (outputVertex.m_X * pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iY] += (outputVertex.m_Y * pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iZ] += (outputVertex.m_Z * pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);

                        // copy the remaining vertex data
                        if (pMesh->m_pVB->m_Format.m_Stride > 3)
                        {
                            const size_t copyIndex = iZ + 1;

                            memcpy(&pLocalMesh->m_pVB->m_pData[copyIndex],
                                   &pMesh->m_pVB->m_pData[copyIndex],
                                    ((size_t)pMesh->m_pVB->m_Format.m_Stride - 3) * sizeof(float));
                        }
                    }
            }
        }
        else
        {
            useSourceBuffer = 1;

            // no weights, just use the existing vertex buffer
            pLocalMesh->m_pVB   = pMesh->m_pVB;
            pLocalMesh->m_Count = pMesh->m_Count;
        }

        useLocalMatrixArray = 0;

        // has matrix array to transform, and model contain mesh bones?
        if (pMatrixArray && pMatrixArray->m_Count && pX->m_pMeshToBoneDict[i].m_pBone)
        {
            // create a new local matrix array
            pLocalMatrixArray = (CSR_Array*)malloc(sizeof(CSR_Array));
            csrArrayInit(pLocalMatrixArray);
            useLocalMatrixArray = 1;

            // create as array item as in the source matrix list
            pLocalMatrixArray->m_pItem =
                    (CSR_ArrayItem*)malloc(sizeof(CSR_ArrayItem) * pMatrixArray->m_Count);

            // succeeded?
            if (pLocalMatrixArray->m_pItem)
            {
                // update array count
                pLocalMatrixArray->m_Count = pMatrixArray->m_Count;

                // iterate through source model matrices
                for (j = 0; j < pMatrixArray->m_Count; ++j)
                {
                    // initialize the local matrix array item
                    pLocalMatrixArray->m_pItem[j].m_AutoFree = 1;
                    pLocalMatrixArray->m_pItem[j].m_pData    = malloc(sizeof(CSR_Matrix4));

                    // get the final matrix after bones transform
                    csrBoneGetMatrix(pX->m_pMeshToBoneDict[i].m_pBone,
                                     (CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData,
                                     (CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData);
                }
            }
        }
        else
            // no matrix array or no bone, keep the original array
            pLocalMatrixArray = (CSR_Array*)pMatrixArray;

        // draw the model mesh
        csrOpenGLDrawMesh(pLocalMesh, pShader, pLocalMatrixArray, fOnGetID);

        // delete the local vertex buffer
        if (!useSourceBuffer)
        {
            free(pLocalMesh->m_pVB->m_pData);
            free(pLocalMesh->m_pVB);
        }

        // delete the local mesh
        free(pLocalMesh);

        // release the transformed matrix list
        if (useLocalMatrixArray)
            csrArrayRelease(pLocalMatrixArray);
    }
}
//---------------------------------------------------------------------------
void csrOpenGLDrawCollada(const CSR_Collada*      pCollada,
                          const CSR_OpenGLShader* pShader,
                          const CSR_Array*        pMatrixArray,
                                size_t            animSetIndex,
                                size_t            frameIndex,
                          const CSR_fOnGetID      fOnGetID)
{
    size_t i;
    size_t j;
    size_t k;
    size_t l;

    // no model to draw?
    if (!pCollada || !pCollada->m_MeshCount)
        return;

    // do draw only the mesh and ignore all other data like bones?
    if (pCollada->m_MeshOnly || !pCollada->m_pMeshWeights)
    {
        // iterate through the meshes to draw
        for (i = 0; i < pCollada->m_MeshCount; ++i)
            // draw the model mesh
            csrOpenGLDrawMesh(&pCollada->m_pMesh[i], pShader, pMatrixArray, fOnGetID);

        return;
    }

    // iterate through the meshes to draw
    for (i = 0; i < pCollada->m_MeshCount; ++i)
    {
        int        useLocalMatrixArray;
        int        useSourceBuffer;
        CSR_Mesh*  pMesh;
        CSR_Mesh*  pLocalMesh;
        CSR_Array* pLocalMatrixArray;

        // if mesh has no skeleton, perform a simple draw
        if (!pCollada->m_pSkeleton)
        {
            // draw the model mesh
            csrOpenGLDrawMesh(&pCollada->m_pMesh[i], pShader, pMatrixArray, fOnGetID);
            return;
        }

        // get the current model mesh to draw
        pMesh = &pCollada->m_pMesh[i];

        // found it?
        if (!pMesh)
            continue;

        // normally each mesh should contain only one vertex buffer
        if (pMesh->m_Count != 1)
            // unsupported if not (because cannot know which texture should be binded. If a such model
            // exists, a custom version of this function should also be written for it)
            continue;

        // create a local mesh to contain the processed frame to draw
        pLocalMesh = csrMeshCreate();

        if (!pLocalMesh)
            continue;

        // bind the source mesh to the local one. Don't need to take care of copy the pointers, because
        // the source mesh will remain valid during the whole local mesh lifetime. Just don't delete
        // them on the loop end
        pLocalMesh->m_Skin = pMesh->m_Skin;
        pLocalMesh->m_Time = pMesh->m_Time;

        // mesh contains skin weights?
        if (pCollada->m_pMeshWeights[i].m_pSkinWeights)
        {
            useSourceBuffer = 0;

            // allocate memory for the final vertex buffer to draw
            pLocalMesh->m_pVB   = (CSR_VertexBuffer*)malloc(pMesh->m_Count * sizeof(CSR_VertexBuffer));
            pLocalMesh->m_Count = pMesh->m_Count;

            if (!pLocalMesh->m_pVB || !pLocalMesh->m_Count)
            {
                free(pLocalMesh);
                continue;
            }

            // bind the source vertex buffer to the local one
            pLocalMesh->m_pVB->m_Format   = pMesh->m_pVB->m_Format;
            pLocalMesh->m_pVB->m_Culling  = pMesh->m_pVB->m_Culling;
            pLocalMesh->m_pVB->m_Material = pMesh->m_pVB->m_Material;
            pLocalMesh->m_pVB->m_Time     = pMesh->m_pVB->m_Time;

            // allocate memory for the vertex buffer data
            pLocalMesh->m_pVB->m_pData = (float*)calloc(pMesh->m_pVB->m_Count, sizeof(float));
            pLocalMesh->m_pVB->m_Count = pMesh->m_pVB->m_Count;

            if (!pLocalMesh->m_pVB->m_pData || !pLocalMesh->m_pVB->m_Count)
            {
                free(pLocalMesh->m_pVB);
                free(pLocalMesh);
                continue;
            }

            // iterate through mesh skin weights
            for (j = 0; j < pCollada->m_pMeshWeights[i].m_Count; ++j)
            {
                CSR_Matrix4 boneMatrix;
                CSR_Matrix4 finalMatrix;

                // get the bone matrix
                if (pCollada->m_PoseOnly)
                    csrBoneGetMatrix(pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone, 0, &boneMatrix);
                else
                    csrBoneGetAnimMatrix(pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone,
                                        &pCollada->m_pAnimationSet[animSetIndex],
                                         frameIndex,
                                         0,
                                        &boneMatrix);

                // get the final matrix after bones transform
                csrMat4Multiply(&pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_Matrix,
                                &boneMatrix,
                                &finalMatrix);

                // apply the bone and its skin weights to each vertices
                for (k = 0; k < pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_IndexTableCount; ++k)
                    for (l = 0; l < pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_Count; ++l)
                    {
                        #ifdef _MSC_VER
                            size_t      iX;
                            size_t      iY;
                            size_t      iZ;
                            CSR_Vector3 inputVertex  = {0};
                            CSR_Vector3 outputVertex = {0};
                        #else
                            size_t      iX;
                            size_t      iY;
                            size_t      iZ;
                            CSR_Vector3 inputVertex;
                            CSR_Vector3 outputVertex;
                        #endif

                        // get the next vertex to which the next skin weight should be applied
                        iX = pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l];
                        iY = pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 1;
                        iZ = pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 2;

                        // get input vertex
                        inputVertex.m_X = pMesh->m_pVB->m_pData[iX];
                        inputVertex.m_Y = pMesh->m_pVB->m_pData[iY];
                        inputVertex.m_Z = pMesh->m_pVB->m_pData[iZ];

                        // apply bone transformation to vertex
                        csrMat4Transform(&finalMatrix, &inputVertex, &outputVertex);

                        // apply the skin weights and calculate the final output vertex
                        pLocalMesh->m_pVB->m_pData[iX] += (outputVertex.m_X * pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iY] += (outputVertex.m_Y * pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iZ] += (outputVertex.m_Z * pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);

                        // copy the remaining vertex data
                        if (pMesh->m_pVB->m_Format.m_Stride > 3)
                        {
                            const size_t copyIndex = iZ + 1;

                            memcpy(&pLocalMesh->m_pVB->m_pData[copyIndex],
                                   &pMesh->m_pVB->m_pData[copyIndex],
                                    ((size_t)pMesh->m_pVB->m_Format.m_Stride - 3) * sizeof(float));
                        }
                    }
            }
        }
        else
        {
            useSourceBuffer = 1;

            // no weights, just use the existing vertex buffer
            pLocalMesh->m_pVB   = pMesh->m_pVB;
            pLocalMesh->m_Count = pMesh->m_Count;
        }

        useLocalMatrixArray = 0;

        // has matrix array to transform, and model contain mesh bones?
        if (pMatrixArray && pMatrixArray->m_Count && pCollada->m_pMeshToBoneDict[i].m_pBone)
        {
            // create a new local matrix array
            pLocalMatrixArray = (CSR_Array*)malloc(sizeof(CSR_Array));
            csrArrayInit(pLocalMatrixArray);
            useLocalMatrixArray = 1;

            // create as array item as in the source matrix list
            pLocalMatrixArray->m_pItem =
                    (CSR_ArrayItem*)malloc(sizeof(CSR_ArrayItem) * pMatrixArray->m_Count);

            // succeeded?
            if (pLocalMatrixArray->m_pItem)
            {
                // update array count
                pLocalMatrixArray->m_Count = pMatrixArray->m_Count;

                // iterate through source model matrices
                for (j = 0; j < pMatrixArray->m_Count; ++j)
                {
                    // initialize the local matrix array item
                    pLocalMatrixArray->m_pItem[j].m_AutoFree = 1;
                    pLocalMatrixArray->m_pItem[j].m_pData    = malloc(sizeof(CSR_Matrix4));

                    // get the final matrix after bones transform
                    csrBoneGetMatrix(pCollada->m_pMeshToBoneDict[i].m_pBone,
                                     (CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData,
                                     (CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData);
                }
            }
        }
        else
            // no matrix array or no bone, keep the original array
            pLocalMatrixArray = (CSR_Array*)pMatrixArray;

        // draw the model mesh
        csrOpenGLDrawMesh(pLocalMesh, pShader, pLocalMatrixArray, fOnGetID);

        // delete the local vertex buffer
        if (!useSourceBuffer)
        {
            free(pLocalMesh->m_pVB->m_pData);
            free(pLocalMesh->m_pVB);
        }

        // delete the local mesh
        free(pLocalMesh);

        // release the transformed matrix list
        if (useLocalMatrixArray)
            csrArrayRelease(pLocalMatrixArray);
    }
}
//---------------------------------------------------------------------------
// State functions
//---------------------------------------------------------------------------
void csrOpenGLStateEnableDepthMask(int value)
{
    // do disable the depth buffer writing?
    if (!value)
    {
        // disable the depth buffer writing
        glDepthMask(GL_FALSE);
        return;
    }

    // enable the depth buffer writing again
    glDepthMask(GL_TRUE);
}
//---------------------------------------------------------------------------
