/****************************************************************************
 * ==> CSR_Renderer_Metal --------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a base to write a Metal renderer      *
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

#ifndef CSR_Renderer_MetalH
#define CSR_Renderer_MetalH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"
#include "CSR_Collada.h"
#include "CSR_Mdl.h"
#include "CSR_Wavefront.h"
#include "CSR_X.h"
#include "CSR_Renderer.h"
#include "CSR_Scene.h"

// metal
#ifdef __OBJC__
    #import <MetalKit/MetalKit.h>
    #import "CSR_Renderer_Metal_Types.h"
#endif

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
        void csrMetalShaderEnable(const void* _Nullable pShader);

        /**
        * Connects a projection matrix to a shader
        *@param pShader - shader to which the matrix should be connected
        *@param pMatrix - matrix to connect
        */
        void csrMetalShaderConnectProjectionMatrix(const void*        _Nullable pShader,
                                                   const CSR_Matrix4* _Nullable pMatrix);

        /**
        * Connects a view matrix to a shader
        *@param pShader - shader to which the matrix should be connected
        *@param pMatrix - matrix to connect
        */
        void csrMetalShaderConnectViewMatrix(const void*        _Nullable pShader,
                                             const CSR_Matrix4* _Nullable pMatrix);

        //-------------------------------------------------------------------
        // Draw functions
        //-------------------------------------------------------------------

        /**
        * Begins to draw
        *@param pColor - scene background color
        */
        void csrMetalDrawBegin(const CSR_Color* _Nullable pColor);

        /**
        * Ends to draw
        */
        void csrMetalDrawEnd(void);

        /**
        * Draws a line
        *@param pLine - line to draw
        *@param pShader - shader that will be used to draw the line
        */
        void csrMetalDrawLine(const CSR_Line* _Nullable pLine, const void* _Nullable pShader);

        /**
        * Draws a vertex buffer in a scene
        *@param pVB - vertex buffer to draw
        *@param pShader - shader to use to draw the vertex buffer
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@note The shader must be first enabled with the csrMetalShaderEnable() function
        */
        void csrMetalDrawVertexBuffer(const CSR_VertexBuffer* _Nullable pVB,
                                      const void*             _Nullable pShader,
                                      const CSR_Array*        _Nullable pMatrixArray);

        /**
        * Draws a mesh in a scene
        *@param pMesh - mesh to draw
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrMetalDrawMesh(const CSR_Mesh*    _Nullable pMesh,
                              const void*        _Nullable pShader,
                              const CSR_Array*   _Nullable pMatrixArray,
                              const CSR_fOnGetID _Nullable fOnGetID);

        /**
        * Draws a model in a scene
        *@param pModel - model to draw
        *@param index - model mesh index
        *@param pShader - shader to use to draw the mesh
        *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
        *                      matrix currently connected in the shader will be used
        *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
        */
        void csrMetalDrawModel(const CSR_Model*   _Nullable pModel,
                                     size_t                 index,
                               const void*        _Nullable pShader,
                               const CSR_Array*   _Nullable pMatrixArray,
                               const CSR_fOnGetID _Nullable fOnGetID);

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
        void csrMetalDrawMDL(const CSR_MDL*     _Nullable pMDL,
                             const void*        _Nullable pShader,
                             const CSR_Array*   _Nullable pMatrixArray,
                                   size_t                 skinIndex,
                                   size_t                 modelIndex,
                                   size_t                 meshIndex,
                             const CSR_fOnGetID _Nullable fOnGetID);

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
        void csrMetalDrawX(const CSR_X*       _Nullable pX,
                           const void*        _Nullable pShader,
                           const CSR_Array*   _Nullable pMatrixArray,
                                 size_t                 animSetIndex,
                                 size_t                 frameIndex,
                           const CSR_fOnGetID _Nullable fOnGetID);

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
        void csrMetalDrawCollada(const CSR_Collada* _Nullable pCollada,
                                 const void*        _Nullable pShader,
                                 const CSR_Array*   _Nullable pMatrixArray,
                                       size_t                 animSetIndex,
                                       size_t                 frameIndex,
                                 const CSR_fOnGetID _Nullable fOnGetID);

        //-------------------------------------------------------------------
        // State functions
        //-------------------------------------------------------------------

        /**
        * Enables or disables the depth mask (i.e. the depth buffer writing)
        *@param value - if 0 the depth mask is disabled, otherwise enabled
        */
        void csrMetalStateEnableDepthMask(int value);

#ifdef __cplusplus
    }
#endif

#ifdef __OBJC__
    //-----------------------------------------------------------------------
    // Metal shader
    //-----------------------------------------------------------------------
    @interface CSR_MetalShader : NSObject
    {
        id<MTLFunction> m_pVertexFunction;
        id<MTLFunction> m_pFragmentFunction;
    }

    @property (readonly) id<MTLFunction> _Nonnull m_pVertexFunction;
    @property (readonly) id<MTLFunction> _Nonnull m_pFragmentFunction;

    /**
    * Initializes the shader
    *@param pLibrary - Metal library at which the shader will be linked
     *@param pVertexShaderName - vertex shader function name
     *@param pFragmentShaderName - fragment shader function name
    */
    - (nonnull instancetype) init :(id<MTLLibrary> _Nonnull)pLibrary
                                  :(NSString* _Nonnull)pVertexShaderName
                                  :(NSString* _Nonnull)pFragmentShaderName;

    /**
    * Releases the class
    */
    - (void) dealloc;

    @end

    static const NSUInteger g_ParallelBufferCount = 3;

    //-----------------------------------------------------------------------
    // Metal renderer
    //-----------------------------------------------------------------------
    @interface CSR_MetalBasicRenderer : NSObject<MTKViewDelegate>
    {
        id<MTLDevice>               m_pDevice;
        id<MTLRenderCommandEncoder> m_pRenderEncoder;
        CSR_Uniforms                m_Uniforms;
        dispatch_semaphore_t        m_Semaphore;
        uint8_t                     m_UniformBufferIndex;
     }

    /**
    * Initializes a Metal view
    *@param pView - view above with the Metal library will draw
    *@return metal view instance
    */
    - (nonnull instancetype) initWithMetalKitView :(nonnull MTKView*)pView;

    /**
    * Releases the class
    */
    - (void) dealloc;

    /**
    * Called when a per frame drawing is required
    *@param pView - view above with the Metal library will draw
    */
    - (void) drawInMTKView :(nonnull MTKView*)pView;

    /**
    * Called when the view orientation or size is about to change
    *@param pView - view for which orientation or size is about to change
    *@param size - new size
    */
    - (void) mtkView :(nonnull MTKView*)pView drawableSizeWillChange:(CGSize)size;

    /**
    * Enables a shader (i.e. notify that from now this shader will be used)
    *@param pShader - shader to enable, disable any previously enabled shader if 0
    */
    - (void) csrMetalShaderEnable :(const void* _Nullable)pShader;

    /**
    * Connects a projection matrix to a shader
    *@param pShader - shader to which the matrix should be connected
    *@param pMatrix - matrix to connect
    */
    - (void) csrMetalShaderConnectProjectionMatrix :(const void* _Nullable)pShader
                                                   :(const CSR_Matrix4* _Nullable)pMatrix;

    /**
    * Connects a view matrix to a shader
    *@param pShader - shader to which the matrix should be connected
    *@param pMatrix - matrix to connect
    */
    - (void) csrMetalShaderConnectViewMatrix :(const void* _Nullable)pShader
                                             :(const CSR_Matrix4* _Nullable)pMatrix;

    /**
    * Begins to draw
    *@param pColor - scene background color
    */
    - (void) csrMetalDrawBegin :(const CSR_Color* _Nullable)pColor;

    /**
    * Ends to draw
    */
    - (void) csrMetalDrawEnd;

    /**
    * Draws a line
    *@param pLine - line to draw
    *@param pShader - shader that will be used to draw the line
    */
    - (void) csrMetalDrawLine :(const CSR_Line* _Nullable)pLine :(const void* _Nullable)pShader;

    /**
    * Draws a vertex buffer in a scene
    *@param pVB - vertex buffer to draw
    *@param pShader - shader to use to draw the vertex buffer
    *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
    *                      matrix currently connected in the shader will be used
    *@note The shader must be first enabled with the csrMetalShaderEnable() function
    */
    - (void) csrMetalDrawVertexBuffer :(const CSR_VertexBuffer* _Nullable)pVB
                                      :(const void* _Nullable)pShader
                                      :(const CSR_Array* _Nullable)pMatrixArray;

    /**
    * Draws a mesh in a scene
    *@param pMesh - mesh to draw
    *@param pShader - shader to use to draw the mesh
    *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
    *                      matrix currently connected in the shader will be used
    *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
    */
    - (void) csrMetalDrawMesh :(const CSR_Mesh* _Nullable)pMesh
                              :(const void* _Nullable)pShader
                              :(const CSR_Array* _Nullable)pMatrixArray
                              :(const CSR_fOnGetID _Nullable)fOnGetID;

    /**
    * Draws a model in a scene
    *@param pModel - model to draw
    *@param index - model mesh index
    *@param pShader - shader to use to draw the mesh
    *@param pMatrixArray - matrices to use, one for each vertex buffer drawing. If 0, the model
    *                      matrix currently connected in the shader will be used
    *@param fOnGetID - callback function to get the OpenGL identifier matching with a key
    */
    -(void) csrMetalDrawModel :(const CSR_Model* _Nullable)pModel
                              :(size_t)index
                              :(const void* _Nullable)pShader
                              :(const CSR_Array* _Nullable)pMatrixArray
                              :(const CSR_fOnGetID _Nullable)fOnGetID;

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
    - (void) csrMetalDrawMDL :(const CSR_MDL* _Nullable)pMDL
                             :(const void* _Nullable)pShader
                             :(const CSR_Array* _Nullable)pMatrixArray
                             :(size_t)skinIndex
                             :(size_t)modelIndex
                             :(size_t)meshIndex
                             :(const CSR_fOnGetID _Nullable)fOnGetID;

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
    - (void) csrMetalDrawX :(const CSR_X* _Nullable)pX
                           :(const void* _Nullable)pShader
                           :(const CSR_Array* _Nullable)pMatrixArray
                           :(size_t)animSetIndex
                           :(size_t)frameIndex
                           :(const CSR_fOnGetID _Nullable)fOnGetID;

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
    - (void) csrMetalDrawCollada :(const CSR_Collada* _Nullable)pCollada
                                 :(const void* _Nullable)pShader
                                 :(const CSR_Array* _Nullable)pMatrixArray
                                 :(size_t)animSetIndex
                                 :(size_t)frameIndex
                                 :(const CSR_fOnGetID _Nullable)fOnGetID;

    /**
    * Enables or disables the depth mask (i.e. the depth buffer writing)
    *@param value - if 0 the depth mask is disabled, otherwise enabled
    */
    - (void) csrMetalStateEnableDepthMask :(int)value;

    /**
    * Creates a metal buffer for a MDL model
    *@param pMDL - MDL model for which the metal buffer should be created
    */
    - (void) CreateBufferFromMDL :(const CSR_MDL* _Nullable)pMDL;

    /**
    * Creates a metal buffer for a X model
    *@param pX - X model for which the metal buffer should be created
    */
    - (void) CreateBufferFromX :(const CSR_X* _Nullable)pX;

    /**
    * Creates a metal buffer for a Collada model
    *@param pCollada - Collada model for which the metal buffer should be created
    */
    - (void) CreateBufferFromCollada :(const CSR_Collada* _Nullable)pCollada;

    /**
    * Creates a metal buffer for a model
    *@param pModel - model for which the metal buffer should be created
    *@param shared - if true, the model will be shared between the GPU and CPU
    */
    - (void) CreateBufferFromModel :(const CSR_Model* _Nullable)pModel :(bool)shared;

    /**
    * Creates a metal buffer for a mesh
    *@param pMesh - mesh for which the metal buffer should be created
    *@param shared - if true, the mesh will be shared between the GPU and CPU
    */
    - (void) CreateBufferFromMesh :(const CSR_Mesh* _Nullable)pMesh :(bool)shared;

    /**
    * Creates a metal buffer for a vertex buffer
    *@param pVB - vertex buffer for which the metal buffer should be created
    *@param shared - if true, the vertex buffer will be shared between the GPU and CPU
    */
    - (void) CreateBufferFromVB :(const CSR_VertexBuffer* _Nullable)pVB :(bool)shared;

    /**
    * Creates a Metal texture
    *@param pKey - texture key to link to
    *@param pUrl - URL to an image file to load
    *@return true on success, otherwise false
    */
    - (bool) CreateTexture :(void* _Nullable)pKey :(nonnull NSURL*)pUrl;

    /**
    * Creates a Metal cubemap texture
    *@param pKey - texture key to link to
    *@param pImageNames - images composing the texture, in the following order:
    *                     1. the positive x image file
    *                     2. the negative x image file
    *                     3. the positive y image file
    *                     4. the negative y image file
    *                     5. the positive z image file
    *                     6. the negative z image file
    *@return true on success, otherwise false
    */
    - (bool) CreateCubemapTexture :(void* _Nullable)pKey :(NSArray* _Nullable)pImageNames;

    /**
    * Creates a Metal uniform
    *@param pKey - uniform key
    */
    - (void) CreateUniform :(const void* _Nullable)pKey;

    /**
    * Creates a Metal uniform to use for the skybox
    */
    - (void) CreateSkyboxUniform;

    /**
    * Creates a Metal render pipeline
    *@param pView - view
    *@param pKey - render pipeline key
    *@param pShader - shader to use for the pipeline
    *@return true on success, otherwise false
    */
    - (bool) CreateRenderPipeline :(nonnull MTKView*)pView
                                  :(nonnull NSString*)pKey
                                  :(CSR_MetalShader* _Nullable)pShader;

    /**
    * Gets a texture from a key
    *@param pKey - key for which the texture should be get
    *@return texture matching with key, nil on error or if not found
    */
    -(id<MTLTexture> _Nullable) GetTexture :(const void* _Nullable)pKey;

    /**
    * Gets a render pipeline from a key
    *@param pKey - key for which the render pipeline should be get
    *@return render pipeline matching with key, nil on error or if not found
    */
    -(id<MTLRenderPipelineState> _Nullable) GetRenderPipeline :(nonnull NSString*)pKey;

    /**
    * Converts a CompactStar 4x4 matrix to a SIMD 4x4 matrix
    *@param pMatrix - CompactStar matrix to convert
    *@return converted SIMD matrix
    */
    - (matrix_float4x4) CSRMat4ToSIMDMat4 :(const CSR_Matrix4* _Nullable)pMatrix;

    @end
#endif

#endif
