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

#include "CSR_Serializer.h"

// std
#include <stdlib.h>

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_CSR_Signature_Data           "data"
#define M_CSR_Signature_Color          "colr"
#define M_CSR_Signature_Matrix         "mat4"
#define M_CSR_Signature_Material       "matl"
#define M_CSR_Signature_Texture        "txtr"
#define M_CSR_Signature_BumpMap        "bump"
#define M_CSR_Signature_Vertex_Format  "vtfo"
#define M_CSR_Signature_Vertex_Culling "vtcl"
#define M_CSR_Signature_Vertex_Buffer  "vbuf"
#define M_CSR_Signature_Mesh           "mesh"
#define M_CSR_Signature_Model          "modl"
#define M_CSR_Signature_Model_Anim     "mani"
#define M_CSR_Signature_MDL            "mdlm"
#define M_CSR_Signature_Matrix_Array   "mxar"
#define M_CSR_Signature_Scene_Item     "scit"
#define M_CSR_Signature_Scene          "scne"
#define M_CSR_Signature_Texture        "txtr"
#define M_CSR_Signature_Texture_Array  "txar"
#define M_CSR_Signature_Shader         "shdr"
#define M_CSR_Signature_Shader_Array   "shar"
//---------------------------------------------------------------------------
// Serializer context functions
//---------------------------------------------------------------------------
void csrSerializerReadContextInit(CSR_ReadContext* pContext)
{
    // no context to initialize?
    if (!pContext)
        return;

    // initialize the context
    pContext->m_fOnCreateTexture   = 0;
    pContext->m_fOnCreateShader    = 0;
    pContext->m_fOnSetTextureIndex = 0;
    pContext->m_fOnSetShaderIndex  = 0;
}
//---------------------------------------------------------------------------
void csrSerializerWriteContextInit(CSR_WriteContext* pContext)
{
    // no context to initialize?
    if (!pContext)
        return;

    // initialize the context
    pContext->m_fOnGetTextureIndex = 0;
    pContext->m_fOnGetShaderIndex  = 0;
}
//---------------------------------------------------------------------------
// Read functions
//---------------------------------------------------------------------------
int csrSerializerReadHeader(const CSR_ReadContext*     pContext,
                            const CSR_Buffer*          pBuffer,
                                  size_t*              pOffset,
                                  CSR_SceneFileHeader* pHeader)
{
    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pHeader)
        return 0;

    // read the header signature
    return csrBufferRead(pBuffer, pOffset, sizeof(CSR_SceneFileHeader), 1, pHeader);
}
//---------------------------------------------------------------------------
int csrSerializerReadModelDependencies(const CSR_ReadContext* pContext,
                                       const CSR_Buffer*      pBuffer,
                                             size_t*          pOffset,
                                             size_t           size,
                                             void*            pModel,
                                             int              dataType,
                                             size_t           index)
{
    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pModel)
        return 0;

    // search for data type to read
    switch (dataType)
    {
        case CSR_DT_ShaderIndex:
        {
            int itemIndex;

            if (!pContext->m_fOnSetShaderIndex)
                break;

            // read the shader index
            if (!csrBufferRead(pBuffer, pOffset, sizeof(int), 1, &itemIndex))
                return 0;

            pContext->m_fOnSetShaderIndex(pModel, itemIndex);
            return 1;
        }

        case CSR_DT_TextureIndex:
        {
            int itemIndex;

            if (!pContext->m_fOnSetTextureIndex)
                break;

            // read the texture index
            if (!csrBufferRead(pBuffer, pOffset, sizeof(int), 1, &itemIndex))
                return 0;

            pContext->m_fOnSetTextureIndex(pModel, itemIndex, index, 0);
            return 1;
        }

        case CSR_DT_BumpMapIndex:
        {
            int itemIndex;

            if (!pContext->m_fOnSetTextureIndex)
                break;

            // read the bump map index
            if (!csrBufferRead(pBuffer, pOffset, sizeof(int), 1, &itemIndex))
                return 0;

            pContext->m_fOnSetTextureIndex(pModel, itemIndex, index, 1);
            return 1;
        }
    }

    return 0;
}
//---------------------------------------------------------------------------
int csrSerializerReadVB(const CSR_ReadContext*      pContext,
                        const CSR_Buffer*           pBuffer,
                              size_t*               pOffset,
                              size_t                size,
                              CSR_VertexBuffer*     pVB)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pVB)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Vertex_Format, 4))
        {
            // read the vertex format
            if (!csrBufferRead(pBuffer, pOffset, sizeof(CSR_VertexFormat), 1, &pVB->m_Format))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Vertex_Culling, 4))
        {
            // read the vertex culling
            if (!csrBufferRead(pBuffer, pOffset, sizeof(CSR_VertexCulling), 1, &pVB->m_Culling))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Material, 4))
        {
            // read the vertex material
            if (!csrBufferRead(pBuffer, pOffset, sizeof(CSR_Material), 1, &pVB->m_Material))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Data, 4))
        {
            // search for data type to read
            switch (pHeader->m_Options)
            {
                case CSR_DT_Vertices:
                {
                    size_t dataSize = pHeader->m_ChunkSize - pHeader->m_HeaderSize;

                    // reserve memory for the vertex data
                    pVB->m_Count = dataSize / sizeof(float);
                    pVB->m_pData = malloc(dataSize);

                    // read the vertex data
                    if (!csrBufferRead(pBuffer, pOffset, dataSize, 1, pVB->m_pData))
                    {
                        free(pHeader);
                        return 0;
                    }

                    break;
                }

                case CSR_DT_TimeStamp:
                    // read the animation timestamp
                    if (!csrBufferRead(pBuffer, pOffset, sizeof(double), 1, &pVB->m_Time))
                    {
                        free(pHeader);
                        return 0;
                    }

                    break;
            }

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadMesh(const CSR_ReadContext* pContext,
                          const CSR_Buffer*      pBuffer,
                                size_t*          pOffset,
                                size_t           size,
                                CSR_Mesh*        pMesh)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pMesh)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Vertex_Buffer, 4))
        {
            CSR_VertexBuffer* pVB;
            size_t            index;

            // add a new vertex buffer to the mesh
            pVB = (CSR_VertexBuffer*)csrMemoryAlloc(pMesh->m_pVB,
                                                    sizeof(CSR_VertexBuffer),
                                                    pMesh->m_Count + 1);

            // succeeded?
            if (!pVB)
            {
                free(pHeader);
                return 0;
            }

            // get the vertex buffer index
            index = pMesh->m_Count;

            // update the mesh
            pMesh->m_pVB = pVB;
            ++pMesh->m_Count;

            // initialize the vertex buffer
            csrVertexBufferInit(&pMesh->m_pVB[index]);

            // read the vertex buffer
            if (!csrSerializerReadVB(pContext,
                                     pBuffer,
                                     pOffset,
                                     pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                    &pMesh->m_pVB[index]))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Data, 4))
        {
            // search for data type to read
            switch (pHeader->m_Options)
            {
                case CSR_DT_TimeStamp:
                    // read the animation timestamp
                    if (!csrBufferRead(pBuffer, pOffset, sizeof(double), 1, &pMesh->m_Time))
                    {
                        free(pHeader);
                        return 0;
                    }

                    break;

                default:
                    // read the mesh dependencies
                    if (!csrSerializerReadModelDependencies(pContext,
                                                            pBuffer,
                                                            pOffset,
                                                            pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                                            pMesh,
                                                            pHeader->m_Options,
                                                            0))
                    {
                        free(pHeader);
                        return 0;
                    }

                    break;
            }

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadModel(const CSR_ReadContext* pContext,
                           const CSR_Buffer*      pBuffer,
                                 size_t*          pOffset,
                                 size_t           size,
                                 CSR_Model*       pModel)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pModel)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Mesh, 4))
        {
            CSR_Mesh* pMesh;
            size_t    index;

            // add a new mesh to the model
            pMesh = (CSR_Mesh*)csrMemoryAlloc(pModel->m_pMesh,
                                              sizeof(CSR_Mesh),
                                              pModel->m_MeshCount + 1);

            // succeeded?
            if (!pMesh)
            {
                free(pHeader);
                return 0;
            }

            // get the mesh index
            index = pModel->m_MeshCount;

            // update the model
            pModel->m_pMesh = pMesh;
            ++pModel->m_MeshCount;

            // initialize the mesh
            csrMeshInit(&pModel->m_pMesh[index]);

            // read the mesh
            if (!csrSerializerReadMesh(pContext,
                                       pBuffer,
                                       pOffset,
                                       pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                      &pModel->m_pMesh[index]))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Data, 4))
        {
            // search for data type to read
            switch (pHeader->m_Options)
            {
                case CSR_DT_TimeStamp:
                    // read the animation timestamp
                    if (!csrBufferRead(pBuffer, pOffset, sizeof(double), 1, &pModel->m_Time))
                    {
                        free(pHeader);
                        return 0;
                    }

                    break;

                default:
                    // read the model dependencies
                    if (!csrSerializerReadModelDependencies(pContext,
                                                            pBuffer,
                                                            pOffset,
                                                            pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                                            pModel,
                                                            pHeader->m_Options,
                                                            0))
                    {
                        free(pHeader);
                        return 0;
                    }

                    break;
            }

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadMDL(const CSR_ReadContext* pContext,
                         const CSR_Buffer*      pBuffer,
                               size_t*          pOffset,
                               size_t           size,
                               CSR_MDL*         pMDL)
{
    size_t               start;
    size_t               modelTexIndex;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pMDL)
        return 0;

    modelTexIndex = 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Model, 4))
        {
            CSR_Model* pModel;
            size_t     index;

            // add a new model to the MDL
            pModel = (CSR_Model*)csrMemoryAlloc(pMDL->m_pModel,
                                                sizeof(CSR_Model),
                                                pMDL->m_ModelCount + 1);

            // succeeded?
            if (!pModel)
            {
                free(pHeader);
                return 0;
            }

            // get the model index
            index = pMDL->m_ModelCount;

            // update the MDL
            pMDL->m_pModel = pModel;
            ++pMDL->m_ModelCount;

            // initialize the model
            csrModelInit(&pMDL->m_pModel[index]);

            // read the model
            if (!csrSerializerReadModel(pContext,
                                        pBuffer,
                                        pOffset,
                                        pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                       &pMDL->m_pModel[index]))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Model_Anim, 4))
        {
            CSR_ModelAnimation* pModelAnim;
            size_t              index;

            // add a new model animation to the MDL
            pModelAnim = (CSR_ModelAnimation*)csrMemoryAlloc(pMDL->m_pAnimation,
                                                             sizeof(CSR_ModelAnimation),
                                                             pMDL->m_AnimationCount + 1);

            // succeeded?
            if (!pModelAnim)
            {
                free(pHeader);
                return 0;
            }

            // get the model animation index
            index = pMDL->m_AnimationCount;

            // update the MDL
            pMDL->m_pAnimation = pModelAnim;
            ++pMDL->m_AnimationCount;

            // read the model animation
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(CSR_ModelAnimation),
                               1,
                              &pMDL->m_pAnimation[index]))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Data, 4))
        {
            // read the model dependencies
            if (!csrSerializerReadModelDependencies(pContext,
                                                    pBuffer,
                                                    pOffset,
                                                    pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                                    pMDL,
                                                    pHeader->m_Options,
                                                    modelTexIndex))
            {
                free(pHeader);
                return 0;
            }

            // increment the model texture index if needed
            switch (pHeader->m_Options)
            {
                case CSR_DT_TextureIndex:
                case CSR_DT_BumpMapIndex: ++modelTexIndex; break;
            }

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadMatrixArray(const CSR_ReadContext* pContext,
                                 const CSR_Buffer*      pBuffer,
                                       size_t*          pOffset,
                                       size_t           size,
                                       CSR_Array*       pMatrixArray)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pMatrixArray)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Matrix, 4))
        {
            CSR_ArrayItem* pMatrixItem;
            size_t         index;

            // add a new matrix item to the array
            pMatrixItem = (CSR_ArrayItem*)csrMemoryAlloc(pMatrixArray->m_pItem,
                                                         sizeof(CSR_ArrayItem),
                                                         pMatrixArray->m_Count + 1);

            // succeeded?
            if (!pMatrixItem)
            {
                free(pHeader);
                return 0;
            }

            // get the matrix item index
            index = pMatrixArray->m_Count;

            // update the array
            pMatrixArray->m_pItem = pMatrixItem;
            ++pMatrixArray->m_Count;

            // initialize the memory required to contain the matrix. This memory should be freed
            // while the array will be released
            pMatrixArray->m_pItem[index].m_pData    = malloc(sizeof(CSR_Matrix4));
            pMatrixArray->m_pItem[index].m_AutoFree = 1;

            // read the matrix item
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(CSR_ArrayItem),
                               1,
                               pMatrixArray->m_pItem[index].m_pData))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadSceneItem(const CSR_ReadContext*      pContext,
                               const CSR_Buffer*           pBuffer,
                                     size_t*               pOffset,
                                     size_t                size,
                                     CSR_ESceneItemOptions options,
                                     CSR_SceneItem*        pSceneItem)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pSceneItem)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Mesh, 4))
        {
            // normally the scene item can contain only one model
            if (pSceneItem->m_pModel)
            {
                free(pHeader);
                return 0;
            }

            // add a new mesh model in the scene item
            pSceneItem->m_pModel = malloc(sizeof(CSR_Mesh));
            pSceneItem->m_Type   = CSR_MT_Mesh;

            // succeeded?
            if (!pSceneItem->m_pModel)
            {
                free(pHeader);
                return 0;
            }

            // initialize the mesh content
            csrMeshInit((CSR_Mesh*)pSceneItem->m_pModel);

            // read the mesh
            if (!csrSerializerReadMesh(pContext,
                                       pBuffer,
                                       pOffset,
                                       pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                       (CSR_Mesh*)pSceneItem->m_pModel))
            {
                free(pHeader);
                return 0;
            }

            // do generate AABB trees for the mesh?
            if (options & CSR_SO_DoGenerateAABB)
            {
                // create the AABB tree from the mesh
                pSceneItem->m_pAABBTree = csrAABBTreeFromMesh((CSR_Mesh*)pSceneItem->m_pModel);

                // succeeded?
                if (!pSceneItem->m_pAABBTree)
                {
                    free(pHeader);
                    return 0;
                }

                pSceneItem->m_AABBTreeCount = 1;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Model, 4))
        {
            // normally the scene item can contain only one model
            if (pSceneItem->m_pModel)
            {
                free(pHeader);
                return 0;
            }

            // add a new model in the scene item
            pSceneItem->m_pModel = malloc(sizeof(CSR_Model));
            pSceneItem->m_Type   = CSR_MT_Model;

            // succeeded?
            if (!pSceneItem->m_pModel)
            {
                free(pHeader);
                return 0;
            }

            // initialize the model content
            csrModelInit((CSR_Model*)pSceneItem->m_pModel);

            // read the model
            if (!csrSerializerReadModel(pContext,
                                        pBuffer,
                                        pOffset,
                                        pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                        (CSR_Model*)pSceneItem->m_pModel))
            {
                free(pHeader);
                return 0;
            }

            // do generate AABB trees for the model?
            if (options & CSR_SO_DoGenerateAABB)
            {
                size_t     i;
                CSR_Model* pModel = (CSR_Model*)pSceneItem->m_pModel;

                pSceneItem->m_AABBTreeCount = 0;

                // iterate through the model meshes for which the AABB tree should be created
                for (i = 0; i < pModel->m_MeshCount; ++i)
                {
                    size_t        treeIndex;
                    CSR_AABBNode* pTree;
                    CSR_AABBNode* pMeshTree;

                    // add a new AABB tree in the scene item
                    pTree = (CSR_AABBNode*)csrMemoryAlloc(pSceneItem->m_pAABBTree,
                                                          sizeof(CSR_AABBNode),
                                                          pSceneItem->m_AABBTreeCount + 1);

                    // succeeded?
                    if (!pTree)
                    {
                        free(pHeader);
                        return 0;
                    }

                    // get the tree index to use
                    treeIndex = pSceneItem->m_AABBTreeCount;

                    // update the scene item content
                    pSceneItem->m_pAABBTree = pTree;
                    ++pSceneItem->m_AABBTreeCount;

                    // create the AABB tree from the model mesh
                    pMeshTree = csrAABBTreeFromMesh(&pModel->m_pMesh[i]);

                    // succeeded?
                    if (!pMeshTree)
                    {
                        free(pHeader);
                        return 0;
                    }

                    // copy the AABB tree content
                    memcpy(&pSceneItem->m_pAABBTree[treeIndex], pMeshTree, sizeof(CSR_AABBNode));
                }
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_MDL, 4))
        {
            // normally the scene item can contain only one model
            if (pSceneItem->m_pModel)
            {
                free(pHeader);
                return 0;
            }

            // add a new MDL model in the scene item
            pSceneItem->m_pModel = malloc(sizeof(CSR_MDL));
            pSceneItem->m_Type   = CSR_MT_MDL;

            // succeeded?
            if (!pSceneItem->m_pModel)
            {
                free(pHeader);
                return 0;
            }

            // initialize the MDL model content
            csrMDLInit((CSR_MDL*)pSceneItem->m_pModel);

            // read the MDL model
            if (!csrSerializerReadMDL(pContext,
                                      pBuffer,
                                      pOffset,
                                      pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                      (CSR_MDL*)pSceneItem->m_pModel))
            {
                free(pHeader);
                return 0;
            }

            // do generate AABB trees for the model?
            if (options & CSR_SO_DoGenerateAABB)
            {
                size_t     i;
                size_t     j;
                CSR_MDL* pMDL = (CSR_MDL*)pSceneItem->m_pModel;

                pSceneItem->m_AABBTreeCount = 0;

                // iterate through the frames for which the AABB tree should be created
                for (i = 0; i < pMDL->m_ModelCount; ++i)
                    for (j = 0; j < pMDL->m_pModel[i].m_MeshCount; ++j)
                    {
                        size_t        treeIndex;
                        CSR_AABBNode* pTree;
                        CSR_AABBNode* pMeshTree;

                        // add a new AABB tree in the scene item
                        pTree = (CSR_AABBNode*)csrMemoryAlloc(pSceneItem->m_pAABBTree,
                                                              sizeof(CSR_AABBNode),
                                                              pSceneItem->m_AABBTreeCount + 1);

                        // succeeded?
                        if (!pTree)
                        {
                            free(pHeader);
                            return 0;
                        }

                        // get the tree index to use
                        treeIndex = pSceneItem->m_AABBTreeCount;

                        // update the scene item content
                        pSceneItem->m_pAABBTree = pTree;
                        ++pSceneItem->m_AABBTreeCount;

                        // create the AABB tree from the model mesh
                        pMeshTree = csrAABBTreeFromMesh(&pMDL->m_pModel[i].m_pMesh[j]);

                        // succeeded?
                        if (!pMeshTree)
                        {
                            free(pHeader);
                            return 0;
                        }

                        // copy the AABB tree content
                        memcpy(&pSceneItem->m_pAABBTree[treeIndex], pMeshTree, sizeof(CSR_AABBNode));
                    }
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Matrix_Array, 4))
        {
            // normally the scene item can contain only one matrix array
            if (pSceneItem->m_pMatrixArray)
            {
                free(pHeader);
                return 0;
            }

            // add a new matrix array in the scene item
            pSceneItem->m_pMatrixArray = malloc(sizeof(CSR_Array));

            // succeeded?
            if (!pSceneItem->m_pMatrixArray)
            {
                free(pHeader);
                return 0;
            }

            // initialize the matrix array content
            pSceneItem->m_pMatrixArray->m_pItem = 0;
            pSceneItem->m_pMatrixArray->m_Count = 0;

            // read the matrix array
            if (!csrSerializerReadMatrixArray(pContext,
                                              pBuffer,
                                              pOffset,
                                              pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                              pSceneItem->m_pMatrixArray))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadScene(const CSR_ReadContext* pContext,
                           const CSR_Buffer*      pBuffer,
                                 size_t*          pOffset,
                                 size_t           size,
                                 CSR_Scene*       pScene)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pScene)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Color, 4))
        {
            // read the scene color
            if (!csrBufferRead(pBuffer, pOffset, sizeof(CSR_Color), 1, &pScene->m_Color))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Matrix, 4))
        {
            // read the scene matrix
            if (!csrBufferRead(pBuffer, pOffset, sizeof(CSR_Matrix4), 1, &pScene->m_Matrix))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Scene_Item, 4))
        {
            CSR_SceneItem* pItem;
            size_t         index;

            // do create a transparent item?
            if (pHeader->m_Options & CSR_SO_Transparent)
            {
                // add a new item to the transparent items
                pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pTransparentItem,
                                                       sizeof(CSR_SceneItem),
                                                       pScene->m_TransparentItemCount + 1);

                // succeeded?
                if (!pItem)
                {
                    free(pHeader);
                    return 0;
                }

                // get the item index
                index = pScene->m_TransparentItemCount;

                // update the scene
                pScene->m_pTransparentItem = pItem;
                ++pScene->m_TransparentItemCount;
            }
            else
            {
                // add a new item to the normal items
                pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pItem,
                                                       sizeof(CSR_SceneItem),
                                                       pScene->m_ItemCount + 1);

                // succeeded?
                if (!pItem)
                {
                    free(pHeader);
                    return 0;
                }

                // get the item index
                index = pScene->m_ItemCount;

                // update the scene
                pScene->m_pItem = pItem;
                ++pScene->m_ItemCount;
            }

            // initialize the scene item
            csrSceneItemInit(&pScene->m_pItem[index]);

            // read the scene item
            if (!csrSerializerReadSceneItem(pContext,
                                            pBuffer,
                                            pOffset,
                                            pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                            pHeader->m_Options,
                                           &pScene->m_pItem[index]))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadTextureArray(const CSR_ReadContext*  pContext,
                                  const CSR_Buffer*       pBuffer,
                                        size_t*           pOffset,
                                        size_t            size,
                                        CSR_TextureArray* pTextureArray)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pTextureArray)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Texture, 4))
        {
            CSR_TextureItem* pItem;
            size_t           index;

            // add a new texture item to the texture array
            pItem = (CSR_TextureItem*)csrMemoryAlloc(pTextureArray->m_pItem,
                                                     sizeof(CSR_TextureItem),
                                                     pTextureArray->m_Count + 1);

            // succeeded?
            if (!pItem)
            {
                free(pHeader);
                return 0;
            }

            // get the texture index
            index = pTextureArray->m_Count;

            // update the texture array
            pTextureArray->m_pItem = pItem;
            ++pTextureArray->m_Count;

            // initialize the texture item
            csrTextureItemInit(&pTextureArray->m_pItem[index]);

            // create memory for the texture pixel buffer
            pTextureArray->m_pItem[index].m_pBuffer = (CSR_PixelBuffer*)malloc(sizeof(CSR_PixelBuffer));

            // read the texture image type
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(CSR_EImageType),
                               1,
                              &pTextureArray->m_pItem[index].m_pBuffer->m_ImageType))
            {
                free(pHeader);
                return 0;
            }

            // read the texture pixel type
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(CSR_EPixelType),
                               1,
                              &pTextureArray->m_pItem[index].m_pBuffer->m_PixelType))
            {
                free(pHeader);
                return 0;
            }

            // read the texture width
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(unsigned),
                               1,
                              &pTextureArray->m_pItem[index].m_pBuffer->m_Width))
            {
                free(pHeader);
                return 0;
            }

            // read the texture height
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(unsigned),
                               1,
                              &pTextureArray->m_pItem[index].m_pBuffer->m_Height))
            {
                free(pHeader);
                return 0;
            }

            // read the texture stride
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(unsigned),
                               1,
                              &pTextureArray->m_pItem[index].m_pBuffer->m_Stride))
            {
                free(pHeader);
                return 0;
            }

            // read the texture byte per pixels
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(unsigned),
                               1,
                              &pTextureArray->m_pItem[index].m_pBuffer->m_BytePerPixel))
            {
                free(pHeader);
                return 0;
            }

            // read the texture data length
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               sizeof(unsigned),
                               1,
                              &pTextureArray->m_pItem[index].m_pBuffer->m_DataLength))
            {
                free(pHeader);
                return 0;
            }

            // read the texture data content
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               pTextureArray->m_pItem[index].m_pBuffer->m_DataLength,
                               1,
                               pTextureArray->m_pItem[index].m_pBuffer->m_pData))
            {
                free(pHeader);
                return 0;
            }

            // notify that a texture was read and should be created
            if (pContext->m_fOnCreateTexture)
                pContext->m_fOnCreateTexture(pTextureArray->m_pItem[index].m_pBuffer);

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadShaderList(const CSR_ReadContext* pContext,
                                const CSR_Buffer*      pBuffer,
                                      size_t*          pOffset,
                                      size_t           size,
                                      CSR_ShaderArray* pShaderArray)
{
    size_t               start;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pOffset || !pShaderArray)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // keep the start offset
    start = *pOffset;

    // read the file content
    while (*pOffset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (*pOffset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, pOffset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Shader, 4))
        {
            CSR_ShaderItem* pItem;
            size_t          index;

            // add a new shader item to the shader array
            pItem = (CSR_ShaderItem*)csrMemoryAlloc(pShaderArray->m_pItem,
                                                    sizeof(CSR_ShaderItem),
                                                    pShaderArray->m_Count + 1);

            // succeeded?
            if (!pItem)
            {
                free(pHeader);
                return 0;
            }

            // get the item index
            index = pShaderArray->m_Count;

            // update the shader array
            pShaderArray->m_pItem = pItem;
            ++pShaderArray->m_Count;

            // initialize the shader item
            csrShaderItemInit(&pShaderArray->m_pItem[index]);

            // create memory for the shader content
            pShaderArray->m_pItem[index].m_pContent = (unsigned char*)malloc(pHeader->m_ChunkSize);

            // read the shader content
            if (!csrBufferRead(pBuffer,
                               pOffset,
                               pHeader->m_ChunkSize,
                               1,
                               pShaderArray->m_pItem[index].m_pContent))
            {
                free(pHeader);
                return 0;
            }

            // notify that a shader was read and should be created
            if (pContext->m_fOnCreateShader)
                pContext->m_fOnCreateShader(pShaderArray->m_pItem[index].m_pContent, pHeader->m_ChunkSize);

            continue;
        }

        prevOffset = *pOffset;

        // nothing is matching, skip the data and continue with next chunk
        *pOffset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == *pOffset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerReadLevel(const CSR_ReadContext*  pContext,
                           const CSR_Buffer*       pBuffer,
                                 CSR_TextureArray* pTextureArray,
                                 CSR_ShaderArray*  pShaderArray,
                                 CSR_Scene*        pScene)
{
    size_t               start;
    size_t               offset;
    size_t               size;
    CSR_SceneFileHeader* pHeader;

    // validate the inputs
    if (!pContext || !pBuffer || !pTextureArray || !pShaderArray || !pScene)
        return 0;

    // create a header
    pHeader = (CSR_SceneFileHeader*)malloc(sizeof(CSR_SceneFileHeader));

    // initialize the read values
    start  = 0;
    offset = 0;
    size   = pBuffer->m_Length;

    // read the file content
    while (offset < start + size)
    {
        size_t prevOffset;

        // protect the loop against data corruption (offset cannot exceeds the buffer length)
        if (offset >= pBuffer->m_Length)
        {
            free(pHeader);
            return 0;
        }

        // read the next header
        if (!csrSerializerReadHeader(pContext, pBuffer, &offset, pHeader))
        {
            free(pHeader);
            return 0;
        }

        // search which chunk is reading
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Texture_Array, 4))
        {
            // read the texture list
            if (!csrSerializerReadTextureArray(pContext,
                                               pBuffer,
                                              &offset,
                                               pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                               pTextureArray))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Shader_Array, 4))
        {
            // read the shader list
            if (!csrSerializerReadShaderList(pContext,
                                             pBuffer,
                                            &offset,
                                             pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                             pShaderArray))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }
        else
        if (!memcmp(&pHeader->m_ID, M_CSR_Signature_Scene, 4))
        {
            // read the scene
            if (!csrSerializerReadScene(pContext,
                                        pBuffer,
                                       &offset,
                                        pHeader->m_ChunkSize - pHeader->m_HeaderSize,
                                        pScene))
            {
                free(pHeader);
                return 0;
            }

            continue;
        }

        prevOffset = offset;

        // nothing is matching, skip the data and continue with next chunk
        offset += pHeader->m_ChunkSize - pHeader->m_HeaderSize;

        // protect the loop against data corruption (offset must change after the ckunk was skipped)
        if (prevOffset == offset)
        {
            free(pHeader);
            return 0;
        }
    }

    // free the header structure
    free(pHeader);

    return 1;
}
//---------------------------------------------------------------------------
// Write functions
//---------------------------------------------------------------------------
int csrSerializerWriteHeader(const CSR_WriteContext* pContext,
                             const char*             pID,
                                   size_t            dataSize,
                                   unsigned          options,
                                   CSR_Buffer*       pBuffer)
{
    CSR_SceneFileHeader header;

    // validate the inputs
    if (!pContext || !pID || !pBuffer)
        return 0;

    // populate the header to write
    memcpy(header.m_ID, pID, 4);
    header.m_HeaderSize = sizeof(CSR_SceneFileHeader);
    header.m_ChunkSize  = header.m_HeaderSize + dataSize;
    header.m_Options    = options;

    // write the header
    if (!csrBufferWrite(pBuffer, &header, header.m_HeaderSize, 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteData(const CSR_WriteContext*  pContext,
                           const CSR_Buffer*        pData,
                                 CSR_ESceneDataType type,
                                 CSR_Buffer*        pBuffer)
{
    // validate the inputs
    if (!pContext || !pData || !pBuffer)
        return 0;

    // is data empty?
    if (!pData->m_pData || !pData->m_Length)
        return 1;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Data,
                                  pData->m_Length,
                                  type,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pData->m_pData, pData->m_Length, 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteColor(const CSR_WriteContext* pContext,
                            const CSR_Color*        pColor,
                                  CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pColor || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Color,
                                  sizeof(CSR_Color),
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pColor, sizeof(CSR_Color), 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteMatrix(const CSR_WriteContext* pContext,
                             const CSR_Matrix4*      pMatrix,
                                   CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pMatrix || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Matrix,
                                  sizeof(CSR_Matrix4),
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pMatrix, sizeof(CSR_Matrix4), 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteMaterial(const CSR_WriteContext* pContext,
                               const CSR_Material*     pMaterial,
                                     CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pMaterial || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Material,
                                  sizeof(CSR_Material),
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pMaterial, sizeof(CSR_Material), 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteTexture(const CSR_WriteContext* pContext,
                              const CSR_Buffer*       pTexture,
                                    CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pTexture || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Texture,
                                  pTexture->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pTexture->m_pData, pTexture->m_Length, 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteBumpMap(const CSR_WriteContext* pContext,
                              const CSR_Buffer*       pBumpMap,
                                    CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pBumpMap || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_BumpMap,
                                  pBumpMap->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pBumpMap->m_pData, pBumpMap->m_Length, 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteModelDependencies(const CSR_WriteContext* pContext,
                                        const void*             pModel,
                                              size_t            index,
                                              CSR_Buffer*       pBuffer)
{
    CSR_Buffer* pDataBuffer;
    int         indexToWrite;

    // validate the inputs
    if (!pContext || !pModel || !pBuffer)
        return 0;

    // no indexToWrite callback to call?
    if (!pContext->m_fOnGetShaderIndex && !pContext->m_fOnGetTextureIndex)
        return 1;

    // initialize the data buffer
    pDataBuffer = csrBufferCreate();

    // prepare the buffer to write the texture index
    pDataBuffer->m_Length = sizeof(int);
    pDataBuffer->m_pData  = malloc(pDataBuffer->m_Length);

    // do write a shader index?
    if (pContext->m_fOnGetShaderIndex && !index)
    {
        // get the shader index to write
        indexToWrite = pContext->m_fOnGetShaderIndex(pModel);

        // received a valid index?
        if (indexToWrite >= 0)
        {
            // copy the data to write
            memcpy(pDataBuffer->m_pData, &indexToWrite, pDataBuffer->m_Length);

            // write the shader index
            if (!csrSerializerWriteData(pContext, pDataBuffer, CSR_DT_ShaderIndex, pBuffer))
            {
                csrBufferRelease(pDataBuffer);
                return 0;
            }
        }
    }

    // get the texture index to write
    indexToWrite = pContext->m_fOnGetTextureIndex(pModel, index, 0);

    // received a valid index?
    if (indexToWrite >= 0)
    {
        // copy the data to write
        memcpy(pDataBuffer->m_pData, &indexToWrite, pDataBuffer->m_Length);

        // write the texture index
        if (!csrSerializerWriteData(pContext, pDataBuffer, CSR_DT_TextureIndex, pBuffer))
        {
            csrBufferRelease(pDataBuffer);
            return 0;
        }
    }

    // get the bump map index to write
    indexToWrite = pContext->m_fOnGetTextureIndex(pModel, index, 1);

    // received a valid index?
    if (indexToWrite >= 0)
    {
        // copy the data to write
        memcpy(pDataBuffer->m_pData, &indexToWrite, pDataBuffer->m_Length);

        // write the bump map index
        if (!csrSerializerWriteData(pContext, pDataBuffer, CSR_DT_BumpMapIndex, pBuffer))
        {
            csrBufferRelease(pDataBuffer);
            return 0;
        }
    }

    // release the buffer
    csrBufferRelease(pDataBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteVF(const CSR_WriteContext* pContext,
                         const CSR_VertexFormat* pVF,
                               CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pVF || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Vertex_Format,
                                  sizeof(CSR_VertexFormat),
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pVF, sizeof(CSR_VertexFormat), 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteVC(const CSR_WriteContext*  pContext,
                         const CSR_VertexCulling* pVC,
                               CSR_Buffer*        pBuffer)
{
    // validate the inputs
    if (!pContext || !pVC || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Vertex_Culling,
                                  sizeof(CSR_VertexCulling),
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pVC, sizeof(CSR_VertexCulling), 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteVB(const CSR_WriteContext* pContext,
                         const CSR_VertexBuffer* pVB,
                               CSR_Buffer*       pBuffer)
{
    CSR_Buffer* pChunkBuffer;
    CSR_Buffer* pDataBuffer;

    // validate the inputs
    if (!pContext || !pVB || !pBuffer)
        return 0;

    // initialize the local chunk buffer
    pChunkBuffer = csrBufferCreate();

    // write the vertex format
    if (!csrSerializerWriteVF(pContext, &pVB->m_Format, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write the vertex culling
    if (!csrSerializerWriteVC(pContext, &pVB->m_Culling, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write the vertex material
    if (!csrSerializerWriteMaterial(pContext, &pVB->m_Material, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // initialize a buffer to write the standalone data
    pDataBuffer = csrBufferCreate();

    // prepare the buffer to write the animation time
    pDataBuffer->m_Length = sizeof(double);
    pDataBuffer->m_pData  = malloc(pDataBuffer->m_Length);

    // succeeded?
    if (!pDataBuffer->m_pData)
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // copy the data to write
    memcpy(pDataBuffer->m_pData, &pVB->m_Time, pDataBuffer->m_Length);

    // write the animation time data
    if (!csrSerializerWriteData(pContext, pDataBuffer, CSR_DT_TimeStamp, pChunkBuffer))
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the buffer content
    free(pDataBuffer->m_pData);

    // prepare the buffer to write the vertices data content
    pDataBuffer->m_Length = (unsigned)pVB->m_Count * sizeof(float);
    pDataBuffer->m_pData  = malloc(pDataBuffer->m_Length);

    // succeeded?
    if (!pDataBuffer->m_pData)
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // copy the data to write
    memcpy(pDataBuffer->m_pData, pVB->m_pData, pDataBuffer->m_Length);

    // write the animation time data
    if (!csrSerializerWriteData(pContext, pDataBuffer, CSR_DT_Vertices, pChunkBuffer))
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the buffer
    csrBufferRelease(pDataBuffer);

    // write the vertex buffer header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Vertex_Buffer,
                                  pChunkBuffer->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write the vertex buffer data content
    if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the local data buffer
    csrBufferRelease(pChunkBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteMesh(const CSR_WriteContext* pContext,
                           const CSR_Mesh*         pMesh,
                                 CSR_Buffer*       pBuffer)
{
    size_t      i;
    CSR_Buffer* pChunkBuffer;
    CSR_Buffer* pDataBuffer;

    // validate the inputs
    if (!pContext || !pMesh || !pBuffer)
        return 0;

    // initialize the local chunk buffer
    pChunkBuffer = csrBufferCreate();

    // write all the vertex buffers the mesh contains
    for (i = 0; i < pMesh->m_Count; ++i)
        if (!csrSerializerWriteVB(pContext, &pMesh->m_pVB[i], pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    // write the model dependencies (shader, textures, ...)
    if (!csrSerializerWriteModelDependencies(pContext, pMesh, 0, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // initialize a buffer to write the standalone data
    pDataBuffer = csrBufferCreate();

    // prepare the buffer to write the animation time
    pDataBuffer->m_Length = sizeof(double);
    pDataBuffer->m_pData  = malloc(pDataBuffer->m_Length);

    // succeeded?
    if (!pDataBuffer->m_pData)
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // copy the data to write
    memcpy(pDataBuffer->m_pData, &pMesh->m_Time, pDataBuffer->m_Length);

    // write the animation time data
    if (!csrSerializerWriteData(pContext, pDataBuffer, CSR_DT_TimeStamp, pChunkBuffer))
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the buffer
    csrBufferRelease(pDataBuffer);

    // write the mesh header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Mesh,
                                  pChunkBuffer->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write mesh data content
    if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the local data buffer
    csrBufferRelease(pChunkBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteModel(const CSR_WriteContext* pContext,
                            const CSR_Model*        pModel,
                                  CSR_Buffer*       pBuffer)
{
    size_t      i;
    CSR_Buffer* pChunkBuffer;
    CSR_Buffer* pDataBuffer;

    // validate the inputs
    if (!pContext || !pModel || !pBuffer)
        return 0;

    // initialize the local chunk buffer
    pChunkBuffer = csrBufferCreate();

    // write all the meshes the model contains
    for (i = 0; i < pModel->m_MeshCount; ++i)
        if (!csrSerializerWriteMesh(pContext, &pModel->m_pMesh[i], pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    // write the model dependencies (shader, textures, ...)
    if (!csrSerializerWriteModelDependencies(pContext, pModel, 0, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // initialize a buffer to write the standalone data
    pDataBuffer = csrBufferCreate();

    // prepare the buffer to write the animation time
    pDataBuffer->m_Length = sizeof(double);
    pDataBuffer->m_pData  = malloc(pDataBuffer->m_Length);

    // succeeded?
    if (!pDataBuffer->m_pData)
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // copy the data to write
    memcpy(pDataBuffer->m_pData, &pModel->m_Time, pDataBuffer->m_Length);

    // write the animation time data
    if (!csrSerializerWriteData(pContext, pDataBuffer, CSR_DT_TimeStamp, pChunkBuffer))
    {
        csrBufferRelease(pDataBuffer);
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the buffer
    csrBufferRelease(pDataBuffer);

    // write the model header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Model,
                                  pChunkBuffer->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write model data content
    if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the local data buffer
    csrBufferRelease(pChunkBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteModelAnimation(const CSR_WriteContext*   pContext,
                                     const CSR_ModelAnimation* pModelAnim,
                                           CSR_Buffer*         pBuffer)
{
    // validate the inputs
    if (!pContext || !pModelAnim || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Model_Anim,
                                  sizeof(CSR_ModelAnimation),
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pModelAnim, sizeof(CSR_ModelAnimation), 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteMDL(const CSR_WriteContext* pContext,
                          const CSR_MDL*          pMDL,
                                CSR_Buffer*       pBuffer)
{
    size_t      i;
    CSR_Buffer* pChunkBuffer;

    // validate the inputs
    if (!pContext || !pMDL || !pBuffer)
        return 0;

    // initialize the local chunk buffer
    pChunkBuffer = csrBufferCreate();

    // write all the models the MDL contains
    for (i = 0; i < pMDL->m_ModelCount; ++i)
        if (!csrSerializerWriteModel(pContext, &pMDL->m_pModel[i], pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    // write all the animations the MDL contains
    for (i = 0; i < pMDL->m_AnimationCount; ++i)
        if (!csrSerializerWriteModelAnimation(pContext, &pMDL->m_pAnimation[i], pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    // the model contains textures?
    if (pMDL->m_TextureCount)
    {
        // iterate through the model textures to write
        for (i = 0; i < pMDL->m_TextureCount; ++i)
            // write the model dependencies (shader, textures, ...)
            if (!csrSerializerWriteModelDependencies(pContext, pMDL, i, pChunkBuffer))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }
    }
    else
    // write at least the other model dependencies (shader, ...)
    if (!csrSerializerWriteModelDependencies(pContext, pMDL, 0, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write the MDL header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_MDL,
                                  pChunkBuffer->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write MDL data content
    if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the local data buffer
    csrBufferRelease(pChunkBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteMatrixItem(const CSR_WriteContext* pContext,
                                 const CSR_ArrayItem*    pMatrixItem,
                                       CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pMatrixItem || !pBuffer)
        return 0;

    // write the header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Matrix,
                                  sizeof(CSR_Matrix4),
                                  CSR_HO_None,
                                  pBuffer))
        return 0;

    // write the data
    if (!csrBufferWrite(pBuffer, pMatrixItem->m_pData, sizeof(CSR_Matrix4), 1))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteMatrixArray(const CSR_WriteContext* pContext,
                                  const CSR_Array*        pMatrixArray,
                                        CSR_Buffer*       pBuffer)
{
    size_t      i;
    CSR_Buffer* pChunkBuffer;

    // validate the inputs
    if (!pContext || !pMatrixArray || !pBuffer)
        return 0;

    // initialize the local chunk buffer
    pChunkBuffer = csrBufferCreate();

    // write all the matrix items the array contains
    for (i = 0; i < pMatrixArray->m_Count; ++i)
        if (!csrSerializerWriteMatrixItem(pContext, &pMatrixArray->m_pItem[i], pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    // write the matrix array header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Matrix_Array,
                                  pChunkBuffer->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write mesh data content
    if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the local data buffer
    csrBufferRelease(pChunkBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteSceneItem(const CSR_WriteContext* pContext,
                                const CSR_SceneItem*    pSceneItem,
                                      int               transparent,
                                      CSR_Buffer*       pBuffer)
{
    CSR_Buffer*           pChunkBuffer;
    CSR_ESceneItemOptions options;

    // validate the inputs
    if (!pContext || !pSceneItem || !pBuffer)
        return 0;

    // initialize the local chunk buffer
    pChunkBuffer = csrBufferCreate();

    // write the model
    switch (pSceneItem->m_Type)
    {
        case CSR_MT_Mesh:
            // write the mesh
            if (!csrSerializerWriteMesh(pContext, (CSR_Mesh*)pSceneItem->m_pModel, pChunkBuffer))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            break;

        case CSR_MT_Model:
            // write the model
            if (!csrSerializerWriteModel(pContext, (CSR_Model*)pSceneItem->m_pModel, pChunkBuffer))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            break;

        case CSR_MT_MDL:
            // write the MDL model
            if (!csrSerializerWriteMDL(pContext, (CSR_MDL*)pSceneItem->m_pModel, pChunkBuffer))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            break;

        default:
            csrBufferRelease(pChunkBuffer);
            return 0;
    }

    // write the scene item matrix array
    if (pSceneItem->m_pMatrixArray)
        if (!csrSerializerWriteMatrixArray(pContext, pSceneItem->m_pMatrixArray, pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    options = CSR_SO_None;

    // set the transparent option if needed
    if (transparent)
        options |= CSR_SO_Transparent;

    // set the do generate AABB tree if the scene item contains one
    if (pSceneItem->m_pAABBTree)
        options |= CSR_SO_DoGenerateAABB;

    // write the scene item header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Scene_Item,
                                  pChunkBuffer->m_Length,
                                  options,
                                  pBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write scene item data content
    if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the local data buffer
    csrBufferRelease(pChunkBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteScene(const CSR_WriteContext* pContext,
                            const CSR_Scene*        pScene,
                                  CSR_Buffer*       pBuffer)
{
    size_t             i;
    CSR_Buffer*        pChunkBuffer;
    CSR_EHeaderOptions options;

    // validate the inputs
    if (!pContext || !pScene || !pBuffer)
        return 0;

    // initialize the local chunk buffer
    pChunkBuffer = csrBufferCreate();

    // write the scene color
    if (!csrSerializerWriteColor(pContext, &pScene->m_Color, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write the scene matrix
    if (!csrSerializerWriteMatrix(pContext, &pScene->m_Matrix, pChunkBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write all the items the scene contains
    for (i = 0; i < pScene->m_ItemCount; ++i)
        if (!csrSerializerWriteSceneItem(pContext,
                                        &pScene->m_pItem[i],
                                         0,
                                         pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    // write all the transparent items the scene contains
    for (i = 0; i < pScene->m_TransparentItemCount; ++i)
        if (!csrSerializerWriteSceneItem(pContext,
                                        &pScene->m_pTransparentItem[i],
                                         1,
                                         pChunkBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

    // write the scene header
    if (!csrSerializerWriteHeader(pContext,
                                  M_CSR_Signature_Scene,
                                  pChunkBuffer->m_Length,
                                  CSR_HO_None,
                                  pBuffer))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // write scene data content
    if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
    {
        csrBufferRelease(pChunkBuffer);
        return 0;
    }

    // release the local data buffer
    csrBufferRelease(pChunkBuffer);

    return 1;
}
//---------------------------------------------------------------------------
int csrSerializerWriteLevel(const CSR_WriteContext* pContext,
                            const CSR_Scene*        pScene,
                            const CSR_TextureArray* pTextureArray,
                            const CSR_ShaderArray*  pShaderArray,
                                  CSR_Buffer*       pBuffer)
{
    // validate the inputs
    if (!pContext || !pBuffer)
        return 0;

    // write the texture array
    if (pTextureArray)
    {
        size_t      i;
        CSR_Buffer* pChunkBuffer;

        // initialize the local chunk buffer
        pChunkBuffer = csrBufferCreate();

        // iterate through textures to write
        for (i = 0; i < pTextureArray->m_Count; ++i)
        {
            // write the texture header
            if (!csrSerializerWriteHeader(pContext,
                                          M_CSR_Signature_Texture,
                                          pTextureArray->m_pItem[i].m_pBuffer->m_DataLength,
                                          CSR_HO_None,
                                          pChunkBuffer))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture image type
            if (!csrBufferWrite(pChunkBuffer,
                               &pTextureArray->m_pItem[i].m_pBuffer->m_ImageType,
                                sizeof(CSR_EImageType),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture pixel type
            if (!csrBufferWrite(pChunkBuffer,
                               &pTextureArray->m_pItem[i].m_pBuffer->m_PixelType,
                                sizeof(CSR_EPixelType),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture width
            if (!csrBufferWrite(pChunkBuffer,
                               &pTextureArray->m_pItem[i].m_pBuffer->m_Width,
                                sizeof(unsigned),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture height
            if (!csrBufferWrite(pChunkBuffer,
                               &pTextureArray->m_pItem[i].m_pBuffer->m_Height,
                                sizeof(unsigned),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture stride
            if (!csrBufferWrite(pChunkBuffer,
                               &pTextureArray->m_pItem[i].m_pBuffer->m_Stride,
                                sizeof(unsigned),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture byte per pixels
            if (!csrBufferWrite(pChunkBuffer,
                               &pTextureArray->m_pItem[i].m_pBuffer->m_BytePerPixel,
                                sizeof(unsigned),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture data length
            if (!csrBufferWrite(pChunkBuffer,
                               &pTextureArray->m_pItem[i].m_pBuffer->m_DataLength,
                                sizeof(unsigned),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write texture data content
            if (!csrBufferWrite(pChunkBuffer,
                                pTextureArray->m_pItem[i].m_pBuffer->m_pData,
                                pTextureArray->m_pItem[i].m_pBuffer->m_DataLength,
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }
        }

        // write the texture array header
        if (!csrSerializerWriteHeader(pContext,
                                      M_CSR_Signature_Texture_Array,
                                      pChunkBuffer->m_Length,
                                      CSR_HO_None,
                                      pBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

        // write texture list data content
        if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

        // release the local data buffer
        csrBufferRelease(pChunkBuffer);
    }

    // write the shaders
    if (pShaderArray)
    {
        size_t      i;
        CSR_Buffer* pChunkBuffer;

        // initialize the local chunk buffer
        pChunkBuffer = csrBufferCreate();

        // iterate through shaders to write
        for (i = 0; i < pShaderArray->m_Count; ++i)
        {
            // write the shader item header
            if (!csrSerializerWriteHeader(pContext,
                                          M_CSR_Signature_Shader,
                                          sizeof(*pShaderArray->m_pItem[i].m_pContent),
                                          CSR_HO_None,
                                          pChunkBuffer))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }

            // write shader data content
            if (!csrBufferWrite(pChunkBuffer,
                                pShaderArray->m_pItem[i].m_pContent,
                                sizeof(*pShaderArray->m_pItem[i].m_pContent),
                                1))
            {
                csrBufferRelease(pChunkBuffer);
                return 0;
            }
        }

        // write the shader array header
        if (!csrSerializerWriteHeader(pContext,
                                      M_CSR_Signature_Shader_Array,
                                      pChunkBuffer->m_Length,
                                      CSR_HO_None,
                                      pBuffer))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

        // write shader array data content
        if (!csrBufferWrite(pBuffer, pChunkBuffer->m_pData, pChunkBuffer->m_Length, 1))
        {
            csrBufferRelease(pChunkBuffer);
            return 0;
        }

        // release the local data buffer
        csrBufferRelease(pChunkBuffer);
    }

    // write the scene
    if (pScene)
        if (!csrSerializerWriteScene(pContext, pScene, pBuffer))
            return 0;

    return 1;
}
//---------------------------------------------------------------------------
