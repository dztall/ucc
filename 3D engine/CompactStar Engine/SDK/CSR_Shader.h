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

#ifndef CSR_ShaderH
#define CSR_ShaderH

// compactStar engine
#include "CSR_Common.h"

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Shader
*/
typedef struct
{
    GLuint m_ProgramID;
    GLuint m_VertexID;
    GLuint m_FragmentID;
    GLint  m_VertexSlot;
    GLint  m_NormalSlot;
    GLint  m_TexCoordSlot;
    GLint  m_TextureSlot;
    GLint  m_BumpMapSlot;
    GLint  m_ColorSlot;
    GLint  m_ModelSlot;
} CSR_Shader;

/**
* Shader item
*/
typedef struct
{
    char*       m_pFileName;
    char*       m_pContent;
    CSR_Shader* m_pShader;
} CSR_ShaderItem;

/**
* Shader array
*/
typedef struct
{
    CSR_ShaderItem* m_pItem;
    size_t          m_Count;
} CSR_ShaderArray;

/**
* Static buffer, it's a buffer which the content was moved to a shader, i.e. on the GPU side
*/
typedef struct
{
    GLuint m_BufferID;
    size_t m_Stride;
} CSR_StaticBuffer;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when static vertex buffers may be linked to the shader
*@param pShader - shader about to be linked
*@param pCustomData - custom data
*/
typedef void (*CSR_fOnLinkStaticVB)(const CSR_Shader* pShader, const void* pCustomData);

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Shader functions
        //-------------------------------------------------------------------

        /**
        * Creates a shader
        *@return newly created shader, 0 on error
        *@note The shader must be released when no longer used, see csrShaderRelease()
        */
        CSR_Shader* csrShaderCreate(void);

        /**
        * Releases a shader
        *@param[in, out] pShader - shader to release
        */
        void csrShaderRelease(CSR_Shader* pShader);

        /**
        * Initializes a shader structure
        *@param[in, out] pShader - shader to initialize
        */
        void csrShaderInit(CSR_Shader* pShader);

        /**
        * Loads, compiles and links a shader from vertex and fragment files
        *@param pVertex - vertex shader program file name
        *@param pFragment - fragment shader program file name
        *@param fOnLinkStaticVB - get link static VB callback function to use, 0 if not used
        *@param pCustomData - custom data to send to fOnLinkStaticVB, 0 if not used
        *@return newly created shader, 0 on error
        *@note The shader must be released when no longer used, see csrShaderRelease()
        */
        CSR_Shader* csrShaderLoadFromFile(const char*               pVertex,
                                          const char*               pFragment,
                                          const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                          const void*               pCustomData);

        /**
        * Loads, compiles and links a shader from strings containing the vertex and fragment programs
        *@param pVertex - string containing the vertex shader program to load
        *@param vertexLength - vertex shader program string length
        *@param pFragment - string containing the fragment shader program to load
        *@param fragmentLength - fragment shader program string length
        *@param fOnLinkStaticVB - get link static VB callback function to use, 0 if not used
        *@param pCustomData - custom data to send to fOnLinkStaticVB, 0 if not used
        *@return newly created shader, 0 on error
        *@note The shader must be released when no longer used, see csrShaderRelease()
        */
        CSR_Shader* csrShaderLoadFromStr(const char*               pVertex,
                                         size_t                    vertexLength,
                                         const char*               pFragment,
                                         size_t                    fragmentLength,
                                         const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                         const void*               pCustomData);

        /**
        * Loads, compiles and links a shader from vertex and fragment buffers
        *@param pVertex - buffer containing the vertex shader program to load
        *@param pFragment - buffer containing the fragment shader program to load
        *@param fOnLinkStaticVB - get link static VB callback function to use, 0 if not used
        *@param pCustomData - custom data to send to fOnLinkStaticVB, 0 if not used
        *@return newly created shader, 0 on error
        *@note The shader must be released when no longer used, see csrShaderRelease()
        */
        CSR_Shader* csrShaderLoadFromBuffer(const CSR_Buffer*         pVertex,
                                            const CSR_Buffer*         pFragment,
                                            const CSR_fOnLinkStaticVB fOnLinkStaticVB,
                                            const void*               pCustomData);

        /**
        * Compiles a shader program
        *@param pSource - source shader program to compile
        *@param shaderType - shader type, parameters are:
        *                    -> GL_VERTEX_SHADER for vertex shader
        *                    -> GL_FRAGMENT_SHADER for fragment shader
        *@param[in, out] pShader - shader that will contain the compiled program
        *@return 1 on success, otherwise 0
        */
        int csrShaderCompile(const CSR_Buffer* pSource, GLenum shaderType, CSR_Shader* pShader);

        /**
        * Links the shader
        *@param[in, out] pShader - shader to link, linked shader if function ends with success
        *@return 1 on success, otherwise 0
        */
        int csrShaderLink(CSR_Shader* pShader);

        /**
        * Enables a shader (i.e. notify that from now this shader will be used)
        *@param pShader - shader to enable, disable any previously enabled shader if 0
        */
        void csrShaderEnable(const CSR_Shader* pShader);

        //-------------------------------------------------------------------
        // Shader item functions
        //-------------------------------------------------------------------

        /**
        * Creates a shader item
        *@return newly created shader item, 0 on error
        *@note The shader item must be released when no longer used, see csrShaderItemContentRelease()
        */
        CSR_ShaderItem* csrShaderItemCreate(void);

        /**
        * Releases a shader item content
        *@param[in, out] pTI - shader item for which the content should be released
        *@note Only the item content is released, the item itself is not released
        */
        void csrShaderItemContentRelease(CSR_ShaderItem* pSI);

        /**
        * Initializes a shader item structure
        *@param[in, out] pTI - shader item to initialize
        */
        void csrShaderItemInit(CSR_ShaderItem* pSI);

        //-------------------------------------------------------------------
        // Shader array functions
        //-------------------------------------------------------------------

        /**
        * Creates a shader array
        *@return newly created shader array, 0 on error
        *@note The shader array must be released when no longer used, see csrShaderArrayRelease()
        */
        CSR_ShaderArray* csrShaderArrayCreate(void);

        /**
        * Releases a shader array
        *@param[in, out] pSA - shader array to release
        */
        void csrShaderArrayRelease(CSR_ShaderArray* pSA);

        /**
        * Initializes a shader array
        *@param[in, out] pSA - shader list to initialize
        */
        void csrShaderArrayInit(CSR_ShaderArray* pSA);

        //-------------------------------------------------------------------
        // Static buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates a static buffer
        *@param pShader - shader that will contain the buffer
        *@param pSA - shader attributes
        *@param pBuffer - buffer to make static
        *@return newly created static buffer, 0 on error
        *@note Once the static buffer is created, the source buffer may be deleted or reused for
        *      another task
        *@note The static buffer must be released when no longer used, see csrStaticBufferRelease()
        */
        CSR_StaticBuffer* csrStaticBufferCreate(const CSR_Shader* pShader, const CSR_Buffer* pBuffer);

        /**
        * Releases a static buffer
        *@param[in, out] pSB - static buffer to release
        */
        void csrStaticBufferRelease(CSR_StaticBuffer* pSB);

        /**
        * Initializes a static buffer structure
        *@param[in, out] pSB - static buffer to initialize
        */
        void csrStaticBufferInit(CSR_StaticBuffer* pSB);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Shader.c"
#endif

#endif
