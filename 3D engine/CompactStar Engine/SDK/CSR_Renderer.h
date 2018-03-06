/****************************************************************************
 * ==> CSR_Renderer --------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the functions to draw a scene         *
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
        * Begins to draw an antialiased scene
        *@param r - scene background color red component in percent (between 0.0f and 1.0f)
        *@param g - scene background color green component in percent (between 0.0f and 1.0f)
        *@param b - scene background color blue component in percent (between 0.0f and 1.0f)
        *@param a - scene background color alpha component in percent (between 0.0f and 1.0f)
        *@param pMSAA - multisample antialiasing to apply
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrMSAASceneBegin(float r, float g, float b, float a, const CSR_MSAA* pMSAA);
        #endif

        /**
        * Ends to draw an antialiased scene
        *@param pMSAA - applied multisample antialiasing
        */
        #ifndef CSR_OPENGL_2_ONLY
            void csrMSAASceneEnd(const CSR_MSAA* pMSAA);
        #endif

        //-------------------------------------------------------------------
        // Scene functions
        //-------------------------------------------------------------------

        /**
        * Begins to draw a scene
        *@param r - scene background color red component in percent (between 0.0f and 1.0f)
        *@param g - scene background color green component in percent (between 0.0f and 1.0f)
        *@param b - scene background color blue component in percent (between 0.0f and 1.0f)
        *@param a - scene background color alpha component in percent (between 0.0f and 1.0f)
        */
        void csrSceneBegin(float r, float g, float b, float a);

        /**
        * Ends to draw a scene
        */
        void csrSceneEnd(void);

        /**
        * Draws a mesh in a scene
        *@param pMesh - mesh to draw
        *@param pShader - shader to use to draw the mesh
        */
        void csrSceneDrawMesh(const CSR_Mesh* pMesh, CSR_Shader* pShader);

        /**
        * Draws a model in a scene
        *@param pModel - model to draw
        *@param index - model mesh index
        *@param pShader - shader to use to draw the mesh
        */
        void csrSceneDrawModel(const CSR_Model* pModel, size_t index, CSR_Shader* pShader);

        /**
        * Draws a MDL model in a scene
        *@param pMDL - MDL model to draw
        *@param pShader - shader to use to draw the model
        *@param textureIndex - texture index
        *@param modelIndex - model index
        *@param meshIndex - mesh index
        */
        void csrSceneDrawMDL(const CSR_MDL*    pMDL,
                                   CSR_Shader* pShader,
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
