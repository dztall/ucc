/****************************************************************************
 * ==> CSR_Renderer --------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the draw functions                    *
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

#ifndef CSR_RendererH
#define CSR_RendererH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"
#include "CSR_Wavefront.h"
#include "CSR_Mdl.h"
#include "CSR_X.h"
#include "CSR_Collada.h"

// graphics library
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #define CSR_USE_OPENGL

    // the Mobile C Compiler cannot use the OpenGL 3.0 and higher functionalities required by
    // several functions. For that reason, its usage is limited to OpenGL 2.0
    #define CSR_OPENGL_2_ONLY
#elif defined(__APPLE__)
    // OpenGL is (unfortunately) deprecated, from now Metal should be used instead. To do that,
    // please define CSR_USE_METAL in the global preprocessor macros and use the metal renderer
    #ifndef CSR_USE_METAL
        #define CSR_USE_OPENGL
    #endif
#elif defined(__CODEGEARC__) || defined(__GNUC__) || defined(_MSC_VER)
    #define CSR_USE_OPENGL
#else
    #error "The graphics library to use in unknown for this system."
#endif

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when a resource identifier should be get from a key
*@param pKey - key for which the resource identifier should be get
*@return identifier, 0 on error or if not found
*@note For Collada and DirectX models, the key cannot be used directly to
*      retrieve the associated resource, because it is get from a local copy
*      of the source mesh. In this case, the key should be casted to a
*      CSR_Texture object, and the m_pFileName parameter content should be
*      used instead as a key
*/
typedef void* (*CSR_fOnGetID)(const void* pKey);

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Shader functions
        //-------------------------------------------------------------------

        /**
        * Enables a shader (i.e. notify that from now this shader will be used)
        *@param pShader - shader to enable, disable any previously enabled shader if 0
        */
        void csrShaderEnable(const void* pShader);

        /**
        * Connects a projection matrix to a shader
        *@param pShader - shader to which the matrix should be connected
        *@param pMatrix - matrix to connect
        */
        void csrShaderConnectProjectionMatrix(const void* pShader, const CSR_Matrix4* pMatrix);

        /**
        * Connects a view matrix to a shader
        *@param pShader - shader to which the matrix should be connected
        *@param pMatrix - matrix to connect
        */
        void csrShaderConnectViewMatrix(const void* pShader, const CSR_Matrix4* pMatrix);

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
        * Draws a line
        *@param pLine - line to draw
        *@param pShader - shader that will be used to draw the line
        */
        void csrDrawLine(const CSR_Line* pLine, const void* pShader);

        /**
        * Draws a vertex buffer in a scene
        *@param pVB - vertex buffer to draw
        *@param pShader - shader to use to draw the vertex buffer
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@note The shader must be first enabled with the csrShaderEnable() function
        */
        void csrDrawVertexBuffer(const CSR_VertexBuffer* pVB,
                                 const void*             pShader,
                                 const CSR_Array*        pMatrixArray);

        /**
        * Draws a mesh in a scene
        *@param pMesh - mesh to draw
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrDrawMesh(const CSR_Mesh*    pMesh,
                         const void*        pShader,
                         const CSR_Array*   pMatrixArray,
                         const CSR_fOnGetID fOnGetID);

        /**
        * Draws a model in a scene
        *@param pModel - model to draw
        *@param index - model mesh index
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrDrawModel(const CSR_Model*   pModel,
                                size_t       index,
                          const void*        pShader,
                          const CSR_Array*   pMatrixArray,
                          const CSR_fOnGetID fOnGetID);

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
        void csrDrawMDL(const CSR_MDL*     pMDL,
                        const void*        pShader,
                        const CSR_Array*   pMatrixArray,
                              size_t       skinIndex,
                              size_t       modelIndex,
                              size_t       meshIndex,
                        const CSR_fOnGetID fOnGetID);

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
        void csrDrawX(const CSR_X*       pX,
                      const void*        pShader,
                      const CSR_Array*   pMatrixArray,
                            size_t       animSetIndex,
                            size_t       frameIndex,
                      const CSR_fOnGetID fOnGetID);

        /**
        * Draws a Collada model in a scene
        *@param pCollada - Collada model to draw
        *@param pShader - shader to use to draw the model
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param animSetIndex - animation set index, ignored if model isn't animated
        *@param frameIndex - frame index, ignored if model isn't animated
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrDrawCollada(const CSR_Collada* pCollada,
                            const void*        pShader,
                            const CSR_Array*   pMatrixArray,
                                  size_t       animSetIndex,
                                  size_t       frameIndex,
                            const CSR_fOnGetID fOnGetID);

        //-------------------------------------------------------------------
        // State functions
        //-------------------------------------------------------------------

        /**
        * Enables or disables the depth mask (i.e. the depth buffer writing)
        *@param value - if 0 the depth mask is disabled, otherwise enabled
        */
        void csrStateEnableDepthMask(int value);

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
