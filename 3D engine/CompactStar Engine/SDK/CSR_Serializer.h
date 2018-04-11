/****************************************************************************
 * ==> CSR_Serializer ------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the functions required for the        *
 *               serialization of models and scenes                         *
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

#ifndef CSR_SerializerH
#define CSR_SerializerH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"
#include "CSR_Renderer.h"
#include "CSR_Scene.h"

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Global header options
*/
typedef enum
{
    CSR_HO_None = 0x0,
} CSR_EHeaderOptions;

/**
* Scene file data type
*/
typedef enum
{
    CSR_DT_Unknown = 0,
    CSR_DT_Vertices,
    CSR_DT_TimeStamp,
    CSR_DT_ShaderIndex,
    CSR_DT_TextureIndex,
    CSR_DT_BumpMapIndex
} CSR_ESceneDataType;

/**
* Scene file header options
*/
typedef enum
{
    CSR_SO_None           = 0x0,
    CSR_SO_DoGenerateAABB = 0x1,
    CSR_SO_Transparent    = 0x2
} CSR_ESceneItemOptions;

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

// Read context prototype
typedef struct CSR_ReadContext CSR_ReadContext;

// Write context prototype
typedef struct CSR_WriteContext CSR_WriteContext;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Scene file header
*/
typedef struct
{
    char     m_ID[4];      // chunk identifier
    unsigned m_HeaderSize; // size of the header, in bytes
    unsigned m_ChunkSize;  // size of the chunk (i.e. header + content), in bytes
    unsigned m_Options;    // chunk options (depend of each chunk)
} CSR_SceneFileHeader;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when a texture should be created for the scene
*@param pPixelBuffer - texture pixel buffer
*/
typedef void (*CSR_fOnCreateTexture)(const CSR_PixelBuffer* pPixelBuffer);

/**
* Called when a shader should be created for the scene
*@param pContent - shader content to create
*@param size - shader content size
*/
typedef void (*CSR_fOnCreateShader)(const char* pContent, size_t size);

/**
* Called when a model should receive a texture index to save
*@param pModel - model for which the texture index should be get
*@param index - model texture index, in case the model contains several textures
*@param bumpMap - if 1, the needed texture should be a bump map, normal texture if 0
*@return texture index from a referenced texture list, if -1 no texture will be linked
*/
typedef int (*CSR_fOnGetTextureIndex)(const void* pModel, size_t index, int bumpMap);

/**
* Called when a texture index should be set to a model
*@param pModel - model for which the texture index should be set
*@param index - texture index
*@param modelTexIndex - texture index to set in the model
*@param bumpMap - if 1, the texture is a bump map, normal texture if 0
*/
typedef void (*CSR_fOnSetTextureIndex)(void* pModel, size_t index, size_t modelTexIndex, int bumpMap);

/**
* Called when a model should receive a shader index to save
*@param pModel - model for which the shader index should be get
*@return shader index from a referenced shader list, if -1 no shader will be linked
*/
typedef int (*CSR_fOnGetShaderIndex)(const void* pModel);

/**
* Called when a shader should be set to a model
*@param pModel - model for which the shader index should be set
*@param index - shader index
*/
typedef int (*CSR_fOnSetShaderIndex)(void* pModel, size_t index);

//---------------------------------------------------------------------------
// Implementation
//---------------------------------------------------------------------------

/**
* Read context
*/
struct CSR_ReadContext
{
    CSR_fOnCreateTexture   m_fOnCreateTexture;
    CSR_fOnCreateShader    m_fOnCreateShader;
    CSR_fOnSetTextureIndex m_fOnSetTextureIndex;
    CSR_fOnSetShaderIndex  m_fOnSetShaderIndex;
};

/**
* Write context
*/
struct CSR_WriteContext
{
    CSR_fOnGetTextureIndex m_fOnGetTextureIndex;
    CSR_fOnGetShaderIndex  m_fOnGetShaderIndex;
};

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Serializer context functions
        //-------------------------------------------------------------------

        /**
        * Initializes a serializer read context
        *@param[in, out] pContext - context to initialize
        */
        void csrSerializerReadContextInit(CSR_ReadContext* pContext);

        /**
        * Initializes a serializer write context
        *@param[in, out] pContext - context to initialize
        */
        void csrSerializerWriteContextInit(CSR_WriteContext* pContext);

        //-------------------------------------------------------------------
        // Read functions
        //-------------------------------------------------------------------

        /**
        * Reads a header from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param[in, out] pHeader - the header to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadHeader(const CSR_ReadContext*     pContext,
                                    const CSR_Buffer*          pBuffer,
                                          size_t*              pOffset,
                                          CSR_SceneFileHeader* pHeader);

        /**
        * Reads the dependencies (texture, shader, ...) a model requires
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pModel - model for which the links should be read
        *@param dataType - dependency data type to read
        *@param index - model texture index for MDL models, always 0 for other model types
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadModelDependencies(const CSR_ReadContext* pContext,
                                               const CSR_Buffer*      pBuffer,
                                                     size_t*          pOffset,
                                                     size_t           size,
                                                     void*            pModel,
                                                     int              dataType,
                                                     size_t           index);

        /**
        * Reads a vertex buffer from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pVB - the vertex buffer to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadVB(const CSR_ReadContext*  pContext,
                                const CSR_Buffer*       pBuffer,
                                      size_t*           pOffset,
                                      size_t            size,
                                      CSR_VertexBuffer* pVB);

        /**
        * Reads a mesh from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pMesh - the mesh to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadMesh(const CSR_ReadContext* pContext,
                                  const CSR_Buffer*      pBuffer,
                                        size_t*          pOffset,
                                        size_t           size,
                                        CSR_Mesh*        pMesh);

        /**
        * Reads a model from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pModel - the model to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadModel(const CSR_ReadContext* pContext,
                                   const CSR_Buffer*      pBuffer,
                                         size_t*          pOffset,
                                         size_t           size,
                                         CSR_Model*       pModel);

        /**
        * Reads a matrix array from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pMatrixArray - the matrix array to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadMatrixArray(const CSR_ReadContext* pContext,
                                         const CSR_Buffer*      pBuffer,
                                               size_t*          pOffset,
                                               size_t           size,
                                               CSR_Array*       pMatrixArray);

        /**
        * Reads a scene item from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param options - scene item options
        *@param[in, out] pSceneItem - the scene item to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadSceneItem(const CSR_ReadContext*      pContext,
                                       const CSR_Buffer*           pBuffer,
                                             size_t*               pOffset,
                                             size_t                size,
                                             CSR_ESceneItemOptions options,
                                             CSR_SceneItem*        pSceneItem);

        /**
        * Reads a scene from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pScene - the scene to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadScene(const CSR_ReadContext* pContext,
                                   const CSR_Buffer*      pBuffer,
                                         size_t*          pOffset,
                                         size_t           size,
                                         CSR_Scene*       pScene);

        /**
        * Reads a texture array from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pTextureArray - the texture array to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadTextureArray(const CSR_ReadContext*  pContext,
                                          const CSR_Buffer*       pBuffer,
                                                size_t*           pOffset,
                                                size_t            size,
                                                CSR_TextureArray* pTextureArray);

        /**
        * Reads a shader array from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param size - size of data to read in buffer
        *@param[in, out] pShaderArray - the shader array to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadShaderArray(const CSR_ReadContext* pContext,
                                         const CSR_Buffer*      pBuffer,
                                               size_t*          pOffset,
                                               size_t           size,
                                               CSR_ShaderArray* pShaderArray);

        /**
        * Reads a level from a buffer
        *@param pContext - read context, containing the read options
        *@param pBuffer - buffer to read from
        *@param[in, out] pTextureArray - the texture array to fill with data
        *@param[in, out] pShaderArray - the shader array to fill with data
        *@param[in, out] pScene - the scene to fill with data
        *@return 1 on success, otherwise 0
        */
        int csrSerializerReadLevel(const CSR_ReadContext*  pContext,
                                   const CSR_Buffer*       pBuffer,
                                         CSR_TextureArray* pTextureArray,
                                         CSR_ShaderArray*  pShaderArray,
                                         CSR_Scene*        pScene);

        //-------------------------------------------------------------------
        // Write functions
        //-------------------------------------------------------------------

        /**
        * Writes a header in the buffer
        *@param pContext - write context, containing the write options
        *@param pID - header identifier, must always be a string containing 4 chars
        *@param dataSize - size of data the chunk will contain, in bytes
        *@param options - header options
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteHeader(const CSR_WriteContext* pContext,
                                     const char*             pID,
                                           size_t            dataSize,
                                           unsigned          options,
                                           CSR_Buffer*       pBuffer);

        /**
        * Writes a data inside a buffer
        *@param pContext - write context, containing the write options
        *@param pData - data to write
        *@param type - data type
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteData(const CSR_WriteContext*  pContext,
                                   const CSR_Buffer*        pData,
                                         CSR_ESceneDataType type,
                                         CSR_Buffer*        pBuffer);

        /**
        * Writes a color inside a buffer
        *@param pContext - write context, containing the write options
        *@param pColor - color to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteColor(const CSR_WriteContext* pContext,
                                    const CSR_Color*        pColor,
                                          CSR_Buffer*       pBuffer);

        /**
        * Writes a matrix inside a buffer
        *@param pContext - write context, containing the write options
        *@param pMatrix - matrix to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteMatrix(const CSR_WriteContext* pContext,
                                     const CSR_Matrix4*      pMatrix,
                                           CSR_Buffer*       pBuffer);

        /**
        * Writes a material inside a buffer
        *@param pContext - write context, containing the write options
        *@param pMaterial - material to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteMaterial(const CSR_WriteContext* pContext,
                                       const CSR_Material*     pMaterial,
                                             CSR_Buffer*       pBuffer);

        /**
        * Writes a texture inside a buffer
        *@param pContext - write context, containing the write options
        *@param pTexture - texture to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteTexture(const CSR_WriteContext* pContext,
                                      const CSR_Buffer*       pTexture,
                                            CSR_Buffer*       pBuffer);

        /**
        * Writes a bump map inside a buffer
        *@param pContext - write context, containing the write options
        *@param pBumpMap - bump map texture to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteBumpMap(const CSR_WriteContext* pContext,
                                      const CSR_Buffer*       pBumpMap,
                                            CSR_Buffer*       pBuffer);

        /**
        * Writes the dependencies (texture, shader, ...) a model requires
        *@param pContext - write context, containing the write options
        *@param pModel - model for which the links should be written
        *@param index - model texture index for MDL models, always 0 for other model types
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteModelDependencies(const CSR_WriteContext* pContext,
                                                const void*             pModel,
                                                      size_t            index,
                                                      CSR_Buffer*       pBuffer);

        /**
        * Writes a vertex format inside a buffer
        *@param pContext - write context, containing the write options
        *@param pVF - vertex format to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteVF(const CSR_WriteContext* pContext,
                                 const CSR_VertexFormat* pVF,
                                       CSR_Buffer*       pBuffer);

        /**
        * Writes a vertex culling inside a buffer
        *@param pContext - write context, containing the write options
        *@param pVC - vertex culling to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteVC(const CSR_WriteContext*  pContext,
                                 const CSR_VertexCulling* pVC,
                                       CSR_Buffer*        pBuffer);

        /**
        * Writes a vertex buffer inside a buffer
        *@param pContext - write context, containing the write options
        *@param pVB - vertex buffer to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteVB(const CSR_WriteContext* pContext,
                                 const CSR_VertexBuffer* pVB,
                                       CSR_Buffer*       pBuffer);

        /**
        * Writes a mesh inside a buffer
        *@param pContext - write context, containing the write options
        *@param pMesh - mesh to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteMesh(const CSR_WriteContext* pContext,
                                   const CSR_Mesh*         pMesh,
                                         CSR_Buffer*       pBuffer);

        /**
        * Writes a model inside a buffer
        *@param pContext - write context, containing the write options
        *@param pModel - model to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteModel(const CSR_WriteContext* pContext,
                                    const CSR_Model*        pModel,
                                          CSR_Buffer*       pBuffer);

        /**
        * Writes a model animation inside a buffer
        *@param pContext - write context, containing the write options
        *@param pModelAnim - model animation to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteModelAnimation(const CSR_WriteContext*   pContext,
                                             const CSR_ModelAnimation* pModelAnim,
                                                   CSR_Buffer*         pBuffer);

        /**
        * Writes a MDL model inside a buffer
        *@param pContext - write context, containing the write options
        *@param pMDL - MDL model to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteMDL(const CSR_WriteContext* pContext,
                                  const CSR_MDL*          pMDL,
                                        CSR_Buffer*       pBuffer);

        /**
        * Writes a matrix item inside a buffer
        *@param pContext - write context, containing the write options
        *@param pMatrixItem - matrix item to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteMatrixItem(const CSR_WriteContext* pContext,
                                         const CSR_ArrayItem*    pMatrixItem,
                                               CSR_Buffer*       pBuffer);

        /**
        * Writes a matrix array inside a buffer
        *@param pContext - write context, containing the write options
        *@param pMatrixArray - matrix array to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteMatrixArray(const CSR_WriteContext* pContext,
                                          const CSR_Array*        pMatrixArray,
                                                CSR_Buffer*       pBuffer);

        /**
        * Writes a scene item inside a buffer
        *@param pContext - write context, containing the write options
        *@param pSceneItem - scene item to write
        *@param transparent - if 1, the scene item is transparent
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteSceneItem(const CSR_WriteContext* pContext,
                                        const CSR_SceneItem*    pSceneItem,
                                              int               transparent,
                                              CSR_Buffer*       pBuffer);

        /**
        * Writes a scene inside a buffer
        *@param pContext - write context, containing the write options
        *@param pScene - scene to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteScene(const CSR_WriteContext* pContext,
                                    const CSR_Scene*        pScene,
                                          CSR_Buffer*       pBuffer);

        /**
        * Writes a level inside a buffer
        *@param pContext - write context, containing the write options
        *@param pScene - scene to write
        *@param pTextureArray - texture array to write
        *@param pShaderArray - shader array to write
        *@param[in, out] pBuffer - buffer to write in
        *@return 1 on success, otherwise 0
        */
        int csrSerializerWriteLevel(const CSR_WriteContext* pContext,
                                    const CSR_Scene*        pScene,
                                    const CSR_TextureArray* pTextureArray,
                                    const CSR_ShaderArray*  pShaderArray,
                                          CSR_Buffer*       pBuffer);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Serializer.c"
#endif

#endif