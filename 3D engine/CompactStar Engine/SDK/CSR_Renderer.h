/****************************************************************************
 * ==> CSR_Renderer --------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the draw functions                    *
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

#ifndef CSR_RendererH
#define CSR_RendererH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"
#include "CSR_Shader.h"

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Multisampling antialiasing
*/
#ifndef CSR_OPENGL_2_ONLY
    typedef struct
    {
        CSR_Shader*       m_pShader;
        CSR_StaticBuffer* m_pStaticBuffer;
        GLuint            m_FrameBufferID;
        GLuint            m_RenderBufferID;
        GLuint            m_TextureBufferID;
        GLuint            m_TextureID;
        size_t            m_Width;
        size_t            m_Height;
        size_t            m_Factor;
    } CSR_MSAA;
#endif

#ifdef __cplusplus
    extern "C"
    {
#endif
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
            CSR_MSAA* csrMSAACreate(size_t width, size_t height, size_t factor);
        #endif

        /**
        * Releases a multisample antialiasing
        *@param[in, out] pMSAA - multisample antialiasing to release
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrMSAARelease(CSR_MSAA* pMSAA);
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
            int csrMSAAInit(size_t width, size_t height, size_t factor, CSR_MSAA* pMSAA);
        #endif

        /**
        * Changes the size of a multisample antialiasing
        *@param width - new width to apply
        *@param height - new height to apply
        *@param[in, out] pMSAA - multisample antialiasing for which the size should change
        *@return 1 on success, otherwise 0
        */
        #ifndef CSR_OPENGL_2_ONLY
            int csrMSAAChangeSize(size_t width, size_t height, CSR_MSAA* pMSAA);
        #endif

        /**
        * Begins to draw with antialiasing
        *@param pColor - scene background color
        *@param pMSAA - multisample antialiasing to apply
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrMSAADrawBegin(const CSR_Color* pColor, const CSR_MSAA* pMSAA);
        #endif

        /**
        * Ends to draw with antialiasing
        *@param pMSAA - applied multisample antialiasing
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrMSAADrawEnd(const CSR_MSAA* pMSAA);
        #endif

        //-------------------------------------------------------------------
        // Draw functions
        //-------------------------------------------------------------------

        /**
        * Begins to draw
        *@param pColor - scene background color
        */
        void csrDrawBegin(const CSR_Color* pColor);

        /**
        * Ends to draw
        */
        void csrDrawEnd(void);

        /**
        * Draws a vertex buffer in a scene
        *@param pVB - vertex buffer to draw
        *@param pShader - shader to use to draw the vertex buffer
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@note The shader must be first enabled with the csrShaderEnable() function
        */
        void csrDrawVertexBuffer(const CSR_VertexBuffer* pVB,
                                 const CSR_Shader*       pShader,
                                 const CSR_Array*        pMatrixArray);

        /**
        * Draws a mesh in a scene
        *@param pMesh - mesh to draw
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        */
        void csrDrawMesh(const CSR_Mesh*   pMesh,
                         const CSR_Shader* pShader,
                         const CSR_Array*  pMatrixArray);

        /**
        * Draws a model in a scene
        *@param pModel - model to draw
        *@param index - model mesh index
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        */
        void csrDrawModel(const CSR_Model*  pModel,
                                size_t      index,
                          const CSR_Shader* pShader,
                          const CSR_Array*  pMatrixArray);

        /**
        * Draws a MDL model in a scene
        *@param pMDL - MDL model to draw
        *@param pShader - shader to use to draw the model
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param textureIndex - texture index
        *@param modelIndex - model index
        *@param meshIndex - mesh index
        */
        void csrDrawMDL(const CSR_MDL*    pMDL,
                        const CSR_Shader* pShader,
                        const CSR_Array*  pMatrixArray,
                              size_t      textureIndex,
                              size_t      modelIndex,
                              size_t      meshIndex);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Renderer.c"
#endif

#endif
