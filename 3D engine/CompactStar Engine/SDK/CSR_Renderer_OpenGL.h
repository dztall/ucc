/****************************************************************************
 * ==> CSR_Renderer_OpenGL -------------------------------------------------*
 ****************************************************************************
 * Description : This module provides an OpenGL renderer                    *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2019, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

#ifndef CSR_Renderer_OpenGLH
#define CSR_Renderer_OpenGLH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"
#include "CSR_Renderer.h"

// openGL
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include <gles2.h>
    #include <gles2ext.h>
#elif defined(__APPLE__)
    #define CSR_OPENGL_2_ONLY
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#elif defined(__CODEGEARC__) || defined(__GNUC__) || defined(_MSC_VER)
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #define GLEW_STATIC
    #include <gl/glew.h>
    #include <gl/gl.h>

    // missing in RAD studio OpenGL header
    #define GL_CLAMP_TO_EDGE 0x812F
#else
    #error "The OpenGL renderer isn't implemented for this platform"
#endif

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* OpenGL identifier
*/
typedef struct
{
    void*  m_pKey;
    size_t m_UseCount;
    GLint  m_ID;
} CSR_OpenGLID;

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
    GLint  m_CubemapSlot;
    GLint  m_ColorSlot;
    GLint  m_ModelSlot;
} CSR_OpenGLShader;

/**
* Static buffer, it's a buffer which the content was moved to a shader, i.e. on the GPU side
*/
typedef struct
{
    GLuint m_BufferID;
    size_t m_Stride;
} CSR_OpenGLStaticBuffer;

/**
* Multisampling antialiasing
*/
#ifndef CSR_OPENGL_2_ONLY
    typedef struct
    {
        CSR_OpenGLShader*       m_pShader;
        CSR_OpenGLStaticBuffer* m_pStaticBuffer;
        GLuint                  m_FrameBufferID;
        GLuint                  m_RenderBufferID;
        GLuint                  m_TextureBufferID;
        GLuint                  m_TextureID;
        size_t                  m_Width;
        size_t                  m_Height;
        size_t                  m_Factor;
    } CSR_OpenGLMSAA;
#endif

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when static vertex buffers may be linked to the shader
*@param pShader - shader about to be linked
*@param pCustomData - custom data
*/
typedef void (*CSR_fOnLinkStaticVB)(const CSR_OpenGLShader* pShader, const void* pCustomData);

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Texture functions
        //-------------------------------------------------------------------

        /**
        * Loads a texture contained in a pixel buffer on the GPU
        *@param pPixelBuffer - pixel buffer containing the texture to load
        *@return loaded texture index, M_CSR_Error_Code on error
        */
        GLuint csrOpenGLTextureFromPixelBuffer(const CSR_PixelBuffer* pPixelBuffer);

        //-------------------------------------------------------------------
        // Cubemap functions
        //-------------------------------------------------------------------

        /**
        * Loads a cubemap texture from 6 images containing each faces on the GPU
        *@param pFileNames - face textures file names
        *@return loaded texture index, M_CSR_Error_Code on error
        */
        GLuint csrOpenGLCubemapLoad(const char** pFileNames);

        //-------------------------------------------------------------------
        // Identifier functions
        //-------------------------------------------------------------------

        /**
        * Creates an identifier
        *@return newly created identifier, 0 on error
        *@note The identifier must be released when no longer used, see csrOpenGLIDRelease()
        */
        CSR_OpenGLID* csrOpenGLIDCreate(void);

        /**
        * Releases an identifier
        *@param[in, out] pID - identifier to release
        */
        void csrOpenGLIDRelease(CSR_OpenGLID* pID);

        /**
        * Initializes an identifier structure
        *@param[in, out] pID - identifier to initialize
        */
        void csrOpenGLIDInit(CSR_OpenGLID* pID);

        //-------------------------------------------------------------------
        // Shader functions
        //-------------------------------------------------------------------

        /**
        * Creates a shader
        *@return newly created shader, 0 on error
        *@note The shader must be released when no longer used, see csrShaderRelease()
        */
        CSR_OpenGLShader* csrOpenGLShaderCreate(void);

        /**
        * Releases a shader
        *@param[in, out] pShader - shader to release
        */
        void csrOpenGLShaderRelease(CSR_OpenGLShader* pShader);

        /**
        * Initializes a shader structure
        *@param[in, out] pShader - shader to initialize
        */
        void csrOpenGLShaderInit(CSR_OpenGLShader* pShader);

        /**
        * Loads, compiles and links a shader from vertex and fragment files
        *@param pVertex - vertex shader program file name
        *@param pFragment - fragment shader program file name
        *@param fOnLinkStaticVB - get link static VB callback function to use, 0 if not used
        *@param pCustomData - custom data to send to fOnLinkStaticVB, 0 if not used
        *@return newly created shader, 0 on error
        *@note The shader must be released when no longer used, see csrShaderRelease()
        */
        CSR_OpenGLShader* csrOpenGLShaderLoadFromFile(const char*               pVertex,
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
        CSR_OpenGLShader* csrOpenGLShaderLoadFromStr(const char*               pVertex,
                                                           size_t              vertexLength,
                                                     const char*               pFragment,
                                                           size_t              fragmentLength,
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
        CSR_OpenGLShader* csrOpenGLShaderLoadFromBuffer(const CSR_Buffer*         pVertex,
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
        int csrOpenGLShaderCompile(const CSR_Buffer*       pSource,
                                         GLenum            shaderType,
                                         CSR_OpenGLShader* pShader);

        /**
        * Links the shader
        *@param[in, out] pShader - shader to link, linked shader if function ends with success
        *@return 1 on success, otherwise 0
        */
        int csrOpenGLShaderLink(CSR_OpenGLShader* pShader);

        /**
        * Enables a shader (i.e. notify that from now this shader will be used)
        *@param pShader - shader to enable, disable any previously enabled shader if 0
        */
        void csrOpenGLShaderEnable(const CSR_OpenGLShader* pShader);

        /**
        * Connects a projection matrix to a shader
        *@param pShader - shader to which the matrix should be connected
        *@param pMatrix - matrix to connect
        */
        void csrOpenGLShaderConnectProjectionMatrix(const CSR_OpenGLShader* pShader,
                                                    const CSR_Matrix4*      pMatrix);

        /**
        * Connects a view matrix to a shader
        *@param pShader - shader to which the matrix should be connected
        *@param pMatrix - matrix to connect
        */
        void csrOpenGLShaderConnectViewMatrix(const CSR_OpenGLShader* pShader,
                                              const CSR_Matrix4*      pMatrix);

        //-------------------------------------------------------------------
        // Static buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates a static buffer
        *@param pShader - shader that will contain the buffer
        *@param pBuffer - buffer to make static
        *@return newly created static buffer, 0 on error
        *@note Once the static buffer is created, the source buffer may be deleted or reused for
        *      another task
        *@note The static buffer must be released when no longer used, see csrStaticBufferRelease()
        */
        CSR_OpenGLStaticBuffer* csrOpenGLStaticBufferCreate(const CSR_OpenGLShader* pShader,
                                                            const CSR_Buffer*       pBuffer);

        /**
        * Releases a static buffer
        *@param[in, out] pSB - static buffer to release
        */
        void csrOpenGLStaticBufferRelease(CSR_OpenGLStaticBuffer* pSB);

        /**
        * Initializes a static buffer structure
        *@param[in, out] pSB - static buffer to initialize
        */
        void csrOpenGLStaticBufferInit(CSR_OpenGLStaticBuffer* pSB);

        //-------------------------------------------------------------------
        // Multisampling antialiasing functions
        //-------------------------------------------------------------------

        /**
        * Creates a multisample antialiasing
        *@param width - viewport width on which the antialiasing will be applied
        *@param height - viewport height on which the antialiasing will be applied
        *@param factor - antialiasing factor to apply, may be 2x, 4x or 8x
        *@return newly created multisample antialiasing, 0 on error
        *@note The multisample antialiasing must be released when no longer used, see csrMSAARelease()
        */
        #ifndef CSR_OPENGL_2_ONLY
            CSR_OpenGLMSAA* csrOpenGLMSAACreate(size_t width, size_t height, size_t factor);
        #endif

        /**
        * Releases a multisample antialiasing
        *@param[in, out] pMSAA - multisample antialiasing to release
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrOpenGLMSAARelease(CSR_OpenGLMSAA* pMSAA);
        #endif

        /**
        * Initializes a multisample antialiasing structure
        *@param width - viewport width on which the antialiasing will be applied
        *@param height - viewport height on which the antialiasing will be applied
        *@param factor - antialiasing factor to apply, may be 2x, 4x or 8x
        *@param[in, out] pMSAA - multisample antialiasing to initialize
        *@return 1 on success, otherwise 0
        */
        #ifndef CSR_OPENGL_2_ONLY
            int csrOpenGLMSAAInit(size_t width, size_t height, size_t factor, CSR_OpenGLMSAA* pMSAA);
        #endif

        /**
        * Changes the size of a multisample antialiasing
        *@param width - new width to apply
        *@param height - new height to apply
        *@param[in, out] pMSAA - multisample antialiasing for which the size should change
        *@return 1 on success, otherwise 0
        */
        #ifndef CSR_OPENGL_2_ONLY
            int csrOpenGLMSAAChangeSize(size_t width, size_t height, CSR_OpenGLMSAA* pMSAA);
        #endif

        /**
        * Begins to draw with antialiasing
        *@param pColor - scene background color
        *@param pMSAA - multisample antialiasing to apply
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrOpenGLMSAADrawBegin(const CSR_Color* pColor, const CSR_OpenGLMSAA* pMSAA);
        #endif

        /**
        * Ends to draw with antialiasing
        *@param pMSAA - applied multisample antialiasing
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrOpenGLMSAADrawEnd(const CSR_OpenGLMSAA* pMSAA);
        #endif

        //-------------------------------------------------------------------
        // Draw functions
        //-------------------------------------------------------------------

        /**
        * Begins to draw
        *@param pColor - scene background color
        */
        void csrOpenGLDrawBegin(const CSR_Color* pColor);

        /**
        * Ends to draw
        */
        void csrOpenGLDrawEnd(void);

        /**
        * Draws a line
        *@param pLine - line to draw
        *@param pShader - shader that will be used to draw the line
        */
        void csrOpenGLDrawLine(const CSR_Line* pLine, const CSR_OpenGLShader* pShader);

        /**
        * Draws a vertex buffer in a scene
        *@param pVB - vertex buffer to draw
        *@param pShader - shader to use to draw the vertex buffer
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@note The shader must be first enabled with the csrShaderEnable() function
        */
        void csrOpenGLDrawVertexBuffer(const CSR_VertexBuffer* pVB,
                                       const CSR_OpenGLShader* pShader,
                                       const CSR_Array*        pMatrixArray);

        /**
        * Draws a mesh in a scene
        *@param pMesh - mesh to draw
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrOpenGLDrawMesh(const CSR_Mesh*         pMesh,
                               const CSR_OpenGLShader* pShader,
                               const CSR_Array*        pMatrixArray,
                               const CSR_fOnGetID      fOnGetID);

        /**
        * Draws a model in a scene
        *@param pModel - model to draw
        *@param index - model mesh index
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrOpenGLDrawModel(const CSR_Model*        pModel,
                                      size_t            index,
                                const CSR_OpenGLShader* pShader,
                                const CSR_Array*        pMatrixArray,
                                const CSR_fOnGetID      fOnGetID);

        /**
        * Draws a MDL model in a scene
        *@param pMDL - MDL model to draw
        *@param pShader - shader to use to draw the model
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param skinIndex - skin index
        *@param modelIndex - model index
        *@param meshIndex - mesh index
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrOpenGLDrawMDL(const CSR_MDL*          pMDL,
                              const CSR_OpenGLShader* pShader,
                              const CSR_Array*        pMatrixArray,
                                    size_t            skinIndex,
                                    size_t            modelIndex,
                                    size_t            meshIndex,
                              const CSR_fOnGetID      fOnGetID);

        /**
        * Draws a X model in a scene
        *@param pX - X model to draw
        *@param pShader - shader to use to draw the model
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param animSetIndex - animation set index, ignored if model isn't animated
        *@param frameIndex - frame index, ignored if model isn't animated
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrOpenGLDrawX(const CSR_X*            pX,
                            const CSR_OpenGLShader* pShader,
                            const CSR_Array*        pMatrixArray,
                                  size_t            animSetIndex,
                                  size_t            frameIndex,
                            const CSR_fOnGetID      fOnGetID);

        //-------------------------------------------------------------------
        // State functions
        //-------------------------------------------------------------------

        /**
        * Enables or disables the depth mask (i.e. the depth buffer writing)
        *@param value - if 0 the depth mask is disabled, otherwise enabled
        */
        void csrOpenGLStateEnableDepthMask(int value);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Renderer_OpenGL.c"
#endif

#endif
