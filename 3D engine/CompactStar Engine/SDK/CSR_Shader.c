/****************************************************************************
 * ==> CSR_Shader ----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the shader functions and types        *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2018, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

#include "CSR_Shader.h"

// std
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
// Shader functions
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    CSR_Shader* csrShaderCreate(void)
    {
        // create a new shader
        CSR_Shader* pShader = (CSR_Shader*)malloc(sizeof(CSR_Shader));

        // succeeded?
        if (!pShader)
            return 0;

        // initialize the shader content
        csrShaderInit(pShader);

        return pShader;
    }
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrShaderRelease(CSR_Shader* pShader)
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
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrShaderInit(CSR_Shader* pShader)
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
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    CSR_Shader* csrShaderLoadFromFile(const char*               pVertex,
                                      const char*               pFragment,
                                      const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                      const void*               pCustomData)
    {
        CSR_Buffer* pVertexProgram;
        CSR_Buffer* pFragmentProgram;
        CSR_Shader* pShader;

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
        pShader = csrShaderLoadFromBuffer(pVertexProgram, pFragmentProgram, fOnLinkStaticVB, pCustomData);

        // release the program buffers
        csrBufferRelease(pVertexProgram);
        csrBufferRelease(pFragmentProgram);

        return pShader;
    }
#endif
//------------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    CSR_Shader* csrShaderLoadFromStr(const char*               pVertex,
                                     size_t                    vertexLength,
                                     const char*               pFragment,
                                     size_t                    fragmentLength,
                                     const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                     const void*               pCustomData)
    {
        CSR_Buffer* pVS;
        CSR_Buffer* pFS;
        CSR_Shader* pShader;

        // validate the inputs
        if (!pVertex || !vertexLength || !pFragment || !fragmentLength)
            return 0;

        // create buffers to contain vertex and fragment programs
        pVS = csrBufferCreate();
        pFS = csrBufferCreate();

        // copy the vertex program to read
        pVS->m_Length = vertexLength;
        pVS->m_pData  = (unsigned char*)malloc(pVS->m_Length + 1);
        memcpy(pVS->m_pData, pVertex, pVS->m_Length);
        ((unsigned char*)pVS->m_pData)[pVS->m_Length] = 0x0;

        // copy the fragment program to read
        pFS->m_Length = fragmentLength;
        pFS->m_pData  = (unsigned char*)malloc(pFS->m_Length + 1);
        memcpy(pFS->m_pData, pFragment, pFS->m_Length);
        ((unsigned char*)pFS->m_pData)[pFS->m_Length] = 0x0;

        // compile and build the shader
        pShader = csrShaderLoadFromBuffer(pVS, pFS, fOnLinkStaticVB, pCustomData);

        // release the buffers
        csrBufferRelease(pVS);
        csrBufferRelease(pFS);

        return pShader;
    }
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    CSR_Shader* csrShaderLoadFromBuffer(const CSR_Buffer*         pVertex,
                                        const CSR_Buffer*         pFragment,
                                        const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                        const void*               pCustomData)
    {
        CSR_Shader* pShader;

        // source vertex or fragment program is missing?
        if (!pVertex || !pFragment)
            return 0;

        // create a new shader
        pShader = csrShaderCreate();

        // succeeded?
        if (!pShader)
            return 0;

        // create a new shader program
        pShader->m_ProgramID = glCreateProgram();

        // succeeded?
        if (!pShader->m_ProgramID)
        {
            csrShaderRelease(pShader);
            return 0;
        }

        // create and compile the vertex shader
        if (!csrShaderCompile(pVertex, GL_VERTEX_SHADER, pShader))
        {
            csrShaderRelease(pShader);
            return 0;
        }

        // create and compile the fragment shader
        if (!csrShaderCompile(pFragment, GL_FRAGMENT_SHADER, pShader))
        {
            csrShaderRelease(pShader);
            return 0;
        }

        // attach compiled programs with shader
        glAttachShader(pShader->m_ProgramID, pShader->m_VertexID);
        glAttachShader(pShader->m_ProgramID, pShader->m_FragmentID);

        // let the user attach static vertex buffers if he wants
        if (fOnLinkStaticVB)
            fOnLinkStaticVB(pShader, pCustomData);

        // link shader
        if (!csrShaderLink(pShader))
        {
            csrShaderRelease(pShader);
            return 0;
        }

        return pShader;
    }
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    int csrShaderCompile(const CSR_Buffer* pSource, GLenum shaderType, CSR_Shader* pShader)
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
        glShaderSource(shaderID, 1, (GLchar**)&pSource->m_pData, 0);
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
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    int csrShaderLink(CSR_Shader* pShader)
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
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrShaderEnable(const CSR_Shader* pShader)
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
#endif
//---------------------------------------------------------------------------
// Shader item functions
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    CSR_ShaderItem* csrShaderItemCreate(void)
    {
        // create a new shader item
        CSR_ShaderItem* pSI = (CSR_ShaderItem*)malloc(sizeof(CSR_ShaderItem));

        // succeeded?
        if (!pSI)
            return 0;

        // initialize the shader item content
        csrShaderItemInit(pSI);

        return pSI;
    }
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrShaderItemRelease(CSR_ShaderItem* pSI)
    {
        // no shader item to release?
        if (!pSI)
            return;

        // do release the file name?
        if (pSI->m_pFileName)
            free(pSI->m_pFileName);

        // do release the shader content?
        if (pSI->m_pContent)
            free(pSI->m_pContent);

        // do release the shader?
        if (pSI->m_pShader)
            csrShaderRelease(pSI->m_pShader);
    }
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrShaderItemInit(CSR_ShaderItem* pSI)
    {
        // no shader item to initialize?
        if (!pSI)
            return;

        // initialize the shader item content
        pSI->m_pFileName = 0;
        pSI->m_pContent  = 0;
        pSI->m_pShader   = 0;
    }
#endif
//---------------------------------------------------------------------------
// Shader array functions
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    CSR_ShaderArray* csrShaderArrayCreate(void)
    {
        // create a new shader array
        CSR_ShaderArray* pSA = (CSR_ShaderArray*)malloc(sizeof(CSR_ShaderArray));

        // succeeded?
        if (!pSA)
            return 0;

        // initialize the shader array content
        csrShaderArrayInit(pSA);

        return pSA;
    }
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrShaderArrayRelease(CSR_ShaderArray* pSA)
    {
        size_t i;

        // no shader array to release?
        if (!pSA)
            return;

        // do free the shader items?
        if (pSA->m_pItem)
        {
            // iterate through shader items and release their content
            for (i = 0; i < pSA->m_Count; ++i)
                csrShaderItemRelease(&pSA->m_pItem[i]);

            // free the shader items
            free(pSA->m_pItem);
        }

        // free the shader array
        free(pSA);
    }
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrShaderArrayInit(CSR_ShaderArray* pSA)
    {
        // no shader array to initialize?
        if (!pSA)
            return;

        // initialize the shader array content
        pSA->m_pItem = 0;
        pSA->m_Count = 0;
    }
#endif
//---------------------------------------------------------------------------
// Static buffer functions
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    CSR_StaticBuffer* csrStaticBufferCreate(const CSR_Shader* pShader, const CSR_Buffer* pBuffer)
    {
        CSR_StaticBuffer* pSB;

        // no shader?
        if (!pShader)
            return 0;

        // validate the inputs
        if (!pBuffer || !pBuffer->m_Length)
            return 0;

        // create a static buffer
        pSB = (CSR_StaticBuffer*)malloc(sizeof(CSR_StaticBuffer));

        // succeeded?
        if (!pSB)
            return 0;

        // initialize the static buffer
        csrStaticBufferInit(pSB);

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
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrStaticBufferRelease(CSR_StaticBuffer* pSB)
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
#endif
//---------------------------------------------------------------------------
#ifdef CSR_USE_OPENGL
    void csrStaticBufferInit(CSR_StaticBuffer* pSB)
    {
        // no static buffer to initialize?
        if (!pSB)
            return;

        // initialize the static buffer content
        pSB->m_BufferID = M_CSR_Error_Code;
        pSB->m_Stride   = 0;
    }
#endif
//---------------------------------------------------------------------------
