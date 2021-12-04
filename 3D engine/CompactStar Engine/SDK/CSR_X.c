/****************************************************************************
 * ==> CSR_X ---------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a DirectX (.x) reader                 *
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

#include "CSR_X.h"

// std
#include <stdlib.h>
#include <math.h>
#include <string.h>

// visual studio specific code
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif

//---------------------------------------------------------------------------
// X model private functions
//---------------------------------------------------------------------------
CSR_Dataset_Generic_X* csrXCreateGenericDataset(void)
{
    // create the dataset
    CSR_Dataset_Generic_X* pData = malloc(sizeof(CSR_Dataset_Generic_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName = 0;

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_Header_X* csrXCreateHeaderDataset(void)
{
    // create the dataset
    CSR_Dataset_Header_X* pData = malloc(sizeof(CSR_Dataset_Header_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName        = 0;
    pData->m_Major        = 0;
    pData->m_Minor        = 0;
    pData->m_Flags        = 0;
    pData->m_ReadValCount = 0;

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_Matrix_X* csrXCreateMatrixDataset(void)
{
    // create the dataset
    CSR_Dataset_Matrix_X* pData = malloc(sizeof(CSR_Dataset_Matrix_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName        = 0;
    pData->m_ReadValCount = 0;
    csrMat4Identity(&pData->m_Matrix);

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_VertexBuffer_X* csrXCreateVertexBufferDataset(void)
{
    // create the dataset
    CSR_Dataset_VertexBuffer_X* pData = malloc(sizeof(CSR_Dataset_VertexBuffer_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName        = 0;
    pData->m_pVertices    = 0;
    pData->m_VerticeCount = 0;
    pData->m_VerticeTotal = 0;
    pData->m_pIndices     = 0;
    pData->m_IndiceCount  = 0;
    pData->m_IndiceTotal  = 0;

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_TexCoords_X* csrXCreateTexCoordsDataset(void)
{
    // create the dataset
    CSR_Dataset_TexCoords_X* pData = malloc(sizeof(CSR_Dataset_TexCoords_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName   = 0;
    pData->m_pUV     = 0;
    pData->m_UVCount = 0;
    pData->m_UVTotal = 0;

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_MaterialList_X* csrXCreateMaterialListDataset(void)
{
    // create the dataset
    CSR_Dataset_MaterialList_X* pData = malloc(sizeof(CSR_Dataset_MaterialList_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName               = 0;
    pData->m_MaterialCount       = 0;
    pData->m_pMaterialIndices    = 0;
    pData->m_MaterialIndiceCount = 0;
    pData->m_MaterialIndiceTotal = 0;

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_Material_X* csrXCreateMaterialDataset(void)
{
    // create the dataset
    CSR_Dataset_Material_X* pData = malloc(sizeof(CSR_Dataset_Material_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName        = 0;
    pData->m_SpecularExp  = 0.0f;
    pData->m_ReadValCount = 0;
    csrRGBAToColor(0xFFFFFFFF, &pData->m_Color);
    csrRGBAToColor(0xFFFFFFFF, &pData->m_SpecularColor);
    csrRGBAToColor(0xFFFFFFFF, &pData->m_EmisiveColor);

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_Texture_X* csrXCreateTextureDataset(void)
{
    // create the dataset
    CSR_Dataset_Texture_X* pData = malloc(sizeof(CSR_Dataset_Texture_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName     = 0;
    pData->m_pFileName = 0;

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_SkinWeights_X* csrXCreateSkinWeightsDataset(void)
{
    // create the dataset
    CSR_Dataset_SkinWeights_X* pData = malloc(sizeof(CSR_Dataset_SkinWeights_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName        = 0;
    pData->m_pBoneName    = 0;
    pData->m_ItemCount    = 0;
    pData->m_pIndices     = 0;
    pData->m_IndiceCount  = 0;
    pData->m_pWeights     = 0;
    pData->m_WeightCount  = 0;
    pData->m_ReadValCount = 0;
    pData->m_BoneIndex    = 0;
    pData->m_MeshIndex    = 0;
    csrMat4Identity(&pData->m_Matrix);

    return pData;
}
//---------------------------------------------------------------------------
CSR_Dataset_AnimationKeys_X* csrXCreateAnimationKeysDataset(void)
{
    // create the dataset
    CSR_Dataset_AnimationKeys_X* pData = malloc(sizeof(CSR_Dataset_AnimationKeys_X));

    // succeeded?
    if (!pData)
        return 0;

    // configure it
    pData->m_pName        = 0;
    pData->m_Type         = CSR_KT_Unknown;
    pData->m_pKeys        = 0;
    pData->m_KeyCount     = 0;
    pData->m_KeyTotal     = 0;
    pData->m_KeyIndex     = 0;
    pData->m_ReadValCount = 0;

    return pData;
}
//---------------------------------------------------------------------------
void csrXInitItem(CSR_Item_X* pItem)
{
    // initialize the item content
    pItem->m_ID            = CSR_XI_Unknown;
    pItem->m_pParent       = 0;
    pItem->m_pChildren     = 0;
    pItem->m_ChildrenCount = 0;
    pItem->m_pData         = 0;
    pItem->m_Opened        = 0;
    pItem->m_ContentRead   = 0;
}
//---------------------------------------------------------------------------
CSR_EDataStructID_X csrXGetDataStructureID(const char* pWord)
{
    if (!strcmp(pWord, "template"))
        return CSR_XI_Template_ID;
    else
    if (!strcmp(pWord, "Header"))
        return CSR_XI_Header_ID;
    else
    if (!strcmp(pWord, "Frame"))
        return CSR_XI_Frame_ID;
    else
    if (!strcmp(pWord, "FrameTransformMatrix"))
        return CSR_XI_Frame_Transform_Matrix_ID;
    else
    if (!strcmp(pWord, "Mesh"))
        return CSR_XI_Mesh_ID;
    else
    if (!strcmp(pWord, "MeshTextureCoords"))
        return CSR_XI_Mesh_Texture_Coords_ID;
    else
    if (!strcmp(pWord, "MeshMaterialList"))
        return CSR_XI_Mesh_Material_List_ID;
    else
    if (!strcmp(pWord, "Material"))
        return CSR_XI_Material_ID;
    else
    if (!strcmp(pWord, "XSkinMeshHeader"))
        return CSR_XI_Skin_Mesh_Header_ID;
    else
    if (!strcmp(pWord, "SkinWeights"))
        return CSR_XI_Skin_Weights_ID;
    else
    if (!strcmp(pWord, "TextureFilename"))
        return CSR_XI_Texture_Filename_ID;
    else
    if (!strcmp(pWord, "MeshNormals"))
        return CSR_XI_Mesh_Normals_ID;
    else
    if (!strcmp(pWord, "AnimationSet"))
        return CSR_XI_Animation_Set_ID;
    else
    if (!strcmp(pWord, "Animation"))
        return CSR_XI_Animation_ID;
    else
    if (!strcmp(pWord, "AnimationKey"))
        return CSR_XI_Animation_Key_ID;

    return CSR_XI_Unknown;
}
//---------------------------------------------------------------------------
char* csrXGetText(const CSR_Buffer* pBuffer, size_t startOffset, size_t endOffset)
{
    char*  pText;
    size_t length;

    // calculate text length
    length = endOffset - startOffset;

    if (!length)
        return "";

    pText = (char*)malloc(length + 1);

    if (!pText)
        return "";

    // get the text
    memcpy(pText, (char*)pBuffer->m_pData + startOffset, length);
    pText[length] = '\0';

    return pText;
}
//---------------------------------------------------------------------------
void csrXSkipCRLF(const CSR_Buffer* pBuffer, size_t* pOffset)
{
    // skip all carriage return and line feed
    while ((*pOffset < pBuffer->m_Length)              &&
          (((char*)pBuffer->m_pData)[*pOffset] == '\r' ||
           ((char*)pBuffer->m_pData)[*pOffset] == '\n'))
        *pOffset = *pOffset + 1;
}
//---------------------------------------------------------------------------
void csrXSkipSeparators(const CSR_Buffer* pBuffer, size_t* pOffset)
{
    // skip all special chars
    while (*pOffset < pBuffer->m_Length)
        switch (((char*)pBuffer->m_pData)[*pOffset])
        {
            case '\r':
            case '\n':
            case '\t':
            case ' ':
            case ',':
            case ';':
                *pOffset = *pOffset + 1;
                continue;

            default:
                return;
        }
}
//---------------------------------------------------------------------------
void csrXSkipLine(const CSR_Buffer* pBuffer, size_t* pOffset)
{
    // skip all the remaining line content
    while ((*pOffset < pBuffer->m_Length)             &&
          ((char*)pBuffer->m_pData)[*pOffset] != '\r' &&
          ((char*)pBuffer->m_pData)[*pOffset] != '\n')
        *pOffset = *pOffset + 1;

    // also skip the CRLF itself
    csrXSkipCRLF(pBuffer, pOffset);
}
//---------------------------------------------------------------------------
int csrXTranslateWord(const CSR_Buffer* pBuffer, size_t startOffset, size_t endOffset)
{
    char*  pWord;
    int    i;
    int    foundOpeningBrace;
    int    foundClosingBrace;
    int    itemType = CSR_XT_Unknown;
    size_t offset   = startOffset;

    // is word empty or contains just 1 char?
    if (endOffset <= startOffset)
        return 0;
    else
    if ((endOffset - startOffset) == 1)
        if (((char*)pBuffer->m_pData)[startOffset] == '{')
            return CSR_XT_Open_Brace;
        else
        if (((char*)pBuffer->m_pData)[startOffset] == '}')
            return CSR_XT_Close_Brace;

    // is a string?
    if (((char*)pBuffer->m_pData)[startOffset]   == '\"' &&
        ((char*)pBuffer->m_pData)[endOffset - 1] == '\"')
        return CSR_XT_String;

    itemType = CSR_XT_Unknown;
    offset   = startOffset;

    // iterate through word chars to determine if word is a number
    while (offset < endOffset)
    {
        // found end of line?
        if (((char*)pBuffer->m_pData)[offset] == '\0')
            break;

        // char contains a number or negative symbol?
        if ((((char*)pBuffer->m_pData)[offset] >= '0'  &&
             ((char*)pBuffer->m_pData)[offset] <= '9') ||
             ((char*)pBuffer->m_pData)[offset] == '-')
        {
            // already detected as a float?
            if (itemType != CSR_XT_Float)
                // set it as an integer
                itemType = CSR_XT_Integer;

            ++offset;
            continue;
        }

        // char contains a floating point separator?
        if (((char*)pBuffer->m_pData)[offset] == '.')
        {
            // set it as a float
            itemType = CSR_XT_Float;
            ++offset;
            continue;
        }

        itemType = CSR_XT_Unknown;
        break;
    }

    // is a number?
    if (itemType == CSR_XT_Float || itemType == CSR_XT_Integer)
        return itemType;

    foundOpeningBrace = 0;
    foundClosingBrace = 0;

    // check if the word is immediately preceded by a opening brace
    for (i = (int)startOffset - 1; i >= 0; --i)
    {
        switch (((char*)pBuffer->m_pData)[i])
        {
            case '\r':
            case '\n':
            case '\t':
            case ' ':                         continue;
            case '{':  foundOpeningBrace = 1; break;
            default:                          break;
        }

        break;
    }

    // check if the word is immediately followed by a closing brace
    for (i = (int)endOffset; (size_t)i < pBuffer->m_Length; ++i)
    {
        switch (((char*)pBuffer->m_pData)[i])
        {
            case '\r':
            case '\n':
            case '\t':
            case ' ':                         continue;
            case '}':  foundClosingBrace = 1; break;
            default:                          break;
        }

        break;
    }

    // is a reference name?
    if (foundOpeningBrace && foundClosingBrace)
        return CSR_XT_Name;

    // extract word from data
    pWord = csrXGetText(pBuffer, startOffset, endOffset);

    // get template identifier
    itemType = csrXGetDataStructureID(pWord);

    // free the word
    free(pWord);

    // is a known template identifier?
    if (itemType != CSR_XI_Unknown)
        return itemType;

    // unknown name or identifier
    return 0;
}
//---------------------------------------------------------------------------
int csrXReadDatasetName(const CSR_Buffer*  pBuffer,
                              size_t       startOffset,
                              size_t       endOffset,
                              CSR_Item_X*  pItem)
{
    switch (pItem->m_ID)
    {
        case CSR_XI_Template_ID:
        case CSR_XI_Frame_ID:
        case CSR_XI_Animation_Set_ID:
        case CSR_XI_Animation_ID:
        {
            // get item data
            CSR_Dataset_Generic_X* pData = (CSR_Dataset_Generic_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Header_ID:
        case CSR_XI_Skin_Mesh_Header_ID:
        {
            // get item data
            CSR_Dataset_Header_X* pData = (CSR_Dataset_Header_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Frame_Transform_Matrix_ID:
        {
            // get item data
            CSR_Dataset_Matrix_X* pData = (CSR_Dataset_Matrix_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Mesh_ID:
        case CSR_XI_Mesh_Normals_ID:
        {
            // get item data data
            CSR_Dataset_VertexBuffer_X* pData =
                    (CSR_Dataset_VertexBuffer_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Mesh_Texture_Coords_ID:
        {
            // get item data
            CSR_Dataset_TexCoords_X* pData = (CSR_Dataset_TexCoords_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Mesh_Material_List_ID:
        {
            // get item data data
            CSR_Dataset_MaterialList_X* pData =
                    (CSR_Dataset_MaterialList_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Material_ID:
        {
            // get item data
            CSR_Dataset_Material_X* pData = (CSR_Dataset_Material_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Skin_Weights_ID:
        {
            // get item data
            CSR_Dataset_SkinWeights_X* pData =
                    (CSR_Dataset_SkinWeights_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Texture_Filename_ID:
        {
            // get item data
            CSR_Dataset_Texture_X* pData = (CSR_Dataset_Texture_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        case CSR_XI_Animation_Key_ID:
        {
            // get item data
            CSR_Dataset_AnimationKeys_X* pData = (CSR_Dataset_AnimationKeys_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // was item data name already attributed?
            if (pData->m_pName)
                return 0;

            // get the item data name
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

            // succeeded?
            if (!pData->m_pName)
                return 0;

            return 1;
        }

        default:
            // unknown word, just ignore it
            return 1;
    }
}
//---------------------------------------------------------------------------
CSR_Item_X* csrXAddChild(CSR_Item_X* pItem, int id, void* pData)
{
    int         index;
    CSR_Item_X* pChildren;
    CSR_Item_X* pChild;

    // no item to add to?
    if (!pItem)
        return 0;

    // add a new child item
    pChildren = csrMemoryAlloc(pItem->m_pChildren, sizeof(CSR_Item_X), pItem->m_ChildrenCount + 1);

    // succeeded?
    if (!pChildren)
        return 0;

    // keep the item index
    index = (int)pItem->m_ChildrenCount;

    // update children
      pItem->m_pChildren = pChildren;
    ++pItem->m_ChildrenCount;

    // get newly created item and initialize it
    pChild = &pChildren[index];
    csrXInitItem(pChild);

    // configure it
    pChild->m_ID      = id;
    pChild->m_pParent = pItem;
    pChild->m_pData   = pData;

    return pChild;
}
//---------------------------------------------------------------------------
CSR_Item_X* csrXGetMaterial(const CSR_Item_X* pItem, size_t index)
{
    // is index out of bounds?
    if (index >= pItem->m_ChildrenCount)
        return 0;

    // return the material matching with the index. NOTE assume that the material list object only
    // contains materials as children and that the read order was the correct one
    return &pItem->m_pChildren[index];
}
//---------------------------------------------------------------------------
int csrXBuildVertex(const CSR_Item_X*                 pItem,
                          CSR_X*                      pX,
                          CSR_Mesh*                   pMesh,
                          size_t                      meshIndex,
                          size_t                      vertexIndex,
                          size_t                      matListIndex,
                          unsigned                    prevColor,
                    const CSR_Dataset_VertexBuffer_X* pMeshDataset,
                    const CSR_Dataset_VertexBuffer_X* pNormalsDataset,
                    const CSR_Dataset_TexCoords_X*    pUVDataset,
                    const CSR_Item_X*                 pMatList,
                    const CSR_Dataset_MaterialList_X* pMatListDataset,
                    const CSR_fOnGetVertexColor       fOnGetVertexColor)
{
    #ifdef _MSC_VER
        size_t       i;
        size_t       j;
        size_t       vbIndex;
        size_t       weightIndex;
        CSR_Vector3  vertex  = {0};
        CSR_Vector3  normal  = {0};
        CSR_Vector2  uv      = {0};
        CSR_Vector3* pNormal =  0;
        CSR_Vector2* pUV     =  0;
    #else
        size_t       i;
        size_t       j;
        size_t       vbIndex;
        size_t       weightIndex;
        CSR_Vector3  vertex;
        CSR_Vector3  normal;
        CSR_Vector2  uv;
        CSR_Vector3* pNormal = 0;
        CSR_Vector2* pUV     = 0;
    #endif

    // calculate the vertex index from the indices table
    const size_t indiceIndex = pMeshDataset->m_pIndices[vertexIndex] * 3;

    // is index out of bounds?
    if (indiceIndex >= pMeshDataset->m_VerticeCount)
        return 0;

    // build the vertex
    vertex.m_X = pMeshDataset->m_pVertices[indiceIndex];
    vertex.m_Y = pMeshDataset->m_pVertices[indiceIndex + 1];
    vertex.m_Z = pMeshDataset->m_pVertices[indiceIndex + 2];

    // mesh contains normals?
    if (pMesh->m_pVB->m_Format.m_HasNormal && pNormalsDataset)
    {
        // calculate the normal index from the indices table
        const size_t nIndiceIndex = pNormalsDataset->m_pIndices[vertexIndex] * 3;

        // is index out of bounds?
        if (nIndiceIndex >= pNormalsDataset->m_IndiceCount)
            return 0;

        // build the normal
        normal.m_X = pNormalsDataset->m_pVertices[nIndiceIndex];
        normal.m_Y = pNormalsDataset->m_pVertices[nIndiceIndex + 1];
        normal.m_Z = pNormalsDataset->m_pVertices[nIndiceIndex + 2];

        pNormal = &normal;
    }

    // mesh contains texture coordinates?
    if (pMesh->m_pVB->m_Format.m_HasTexCoords && pUVDataset)
    {
        // calculate the uv index from the indices table
        const size_t uvIndex = pMeshDataset->m_pIndices[vertexIndex] * 2;

        // is index out of bounds?
        if (uvIndex >= pUVDataset->m_UVCount)
            return 0;

        // build the texture coordinate. NOTE several files contain negative values, force them to
        // be positive (assume that the user will correct its texture in this case)
        #ifdef __CODEGEARC__
            uv.m_X = fabs(pUVDataset->m_pUV[uvIndex]);
            uv.m_Y = fabs(pUVDataset->m_pUV[uvIndex + 1]);
        #else
            uv.m_X = fabsf(pUVDataset->m_pUV[uvIndex]);
            uv.m_Y = fabsf(pUVDataset->m_pUV[uvIndex + 1]);
        #endif

        pUV = &uv;
    }

    // do apply a material to the vertex?
    if (pMatList && pMatListDataset)
    {
        CSR_Item_X* pMaterialItem;
        size_t      materialIndex;

        if (pMatListDataset->m_pMaterialIndices)
        {
            // get the material index to apply to this vertex
            if (pMatListDataset->m_MaterialCount == 1)
                materialIndex = pMatListDataset->m_pMaterialIndices[0];
            else
                materialIndex = pMatListDataset->m_pMaterialIndices[matListIndex];

            // get the material item
            pMaterialItem = csrXGetMaterial(pMatList, materialIndex);
        }
        else
            pMaterialItem = 0;

        // succeeded?
        if (pMaterialItem && pMaterialItem->m_ID == CSR_XI_Material_ID)
        {
            // get the material dataset
            CSR_Dataset_Material_X* pMaterialDataset =
                    (CSR_Dataset_Material_X*)pMaterialItem->m_pData;

            // found it?
            if (pMaterialDataset)
                // change the vertex color to match with the material one
                pX->m_pMesh[meshIndex].m_pVB->m_Material.m_Color =
                        csrColorToRGBA(&pMaterialDataset->m_Color);
        }
    }

    // keep the newly added vertex index
    vbIndex = pX->m_pMesh[meshIndex].m_pVB->m_Count;

    // add the next vertex to the buffer
    if (!csrVertexBufferAdd(&vertex,
                             pNormal,
                             pUV,
                             0,
                             fOnGetVertexColor,
                             pX->m_pMesh[meshIndex].m_pVB))
        return 0;

    // reset the previous vertex color
    pX->m_pMesh[meshIndex].m_pVB->m_Material.m_Color = prevColor;

    // do draw mesh only, nothing more is required
    if (pX->m_MeshOnly)
        return 1;

    weightIndex = 0;

    // link the newly added vertices to the mesh skin weights
    for (i = 0; i < pItem->m_ChildrenCount; ++i)
        switch (pItem->m_pChildren[i].m_ID)
        {
            case CSR_XI_Skin_Weights_ID:
            {
                // found the skin weights dataset, get it
                CSR_Dataset_SkinWeights_X* pSkinWeightsDataset =
                        (CSR_Dataset_SkinWeights_X*)pItem->m_pChildren[i].m_pData;

                // succeeded?
                if (!pSkinWeightsDataset)
                    return 0;

                // iterate through the indices to link to mesh vertices
                for (j = 0; j < pSkinWeightsDataset->m_IndiceCount; ++j)
                    // is current vertex index matching with one in the current skin weights?
                    if (pSkinWeightsDataset->m_pIndices[j] == pMeshDataset->m_pIndices[vertexIndex])
                    {
                        // allocate memory for the new indices to add
                        size_t* pWeightIndices =
                                (size_t*)csrMemoryAlloc
                                        (pX->m_pMeshWeights[meshIndex].m_pSkinWeights[weightIndex].m_pIndexTable[j].m_pData,
                                         sizeof(size_t),
                                         pX->m_pMeshWeights[meshIndex].m_pSkinWeights[weightIndex].m_pIndexTable[j].m_Count + 1);

                        // succeeded?
                        if (!pWeightIndices)
                            return 0;

                        // set the vertex index to keep
                        pWeightIndices[pX->m_pMeshWeights[meshIndex].m_pSkinWeights[weightIndex].m_pIndexTable[j].m_Count] = vbIndex;

                        // update the weight indices
                        pX->m_pMeshWeights[meshIndex].m_pSkinWeights[weightIndex].m_pIndexTable[j].m_pData = pWeightIndices;
                        ++pX->m_pMeshWeights[meshIndex].m_pSkinWeights[weightIndex].m_pIndexTable[j].m_Count;
                    }

                ++weightIndex;
                continue;
            }

            default:
                continue;
        }

    return 1;
}
//---------------------------------------------------------------------------
int csrXBuildMesh(const CSR_Item_X*           pItem,
                        CSR_X*                pX,
                        CSR_Bone*             pBone,
                  const CSR_VertexFormat*     pVertFormat,
                  const CSR_VertexCulling*    pVertCulling,
                  const CSR_Material*         pMaterial,
                  const CSR_fOnGetVertexColor fOnGetVertexColor,
                  const CSR_fOnLoadTexture    fOnLoadTexture,
                  const CSR_fOnApplySkin      fOnApplySkin,
                  const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    size_t                      i;
    size_t                      j;
    size_t                      k;
    size_t                      index;
    size_t                      meshWeightsIndex;
    size_t                      materialIndex;
    unsigned                    prevColor;
    int                         hasTexture;
    CSR_Mesh*                   pMesh;
    CSR_MeshSkinWeights_X*      pMeshWeights;
    CSR_Item_X*                 pMatListItem;
    CSR_MeshBoneItem_X*         pMeshBoneItem;
    CSR_Dataset_VertexBuffer_X* pNormalsDataset;
    CSR_Dataset_TexCoords_X*    pUVDataset;
    CSR_Dataset_MaterialList_X* pMatListDataset;

    // get the dataset containing the mesh
    CSR_Dataset_VertexBuffer_X* pMeshDataset = (CSR_Dataset_VertexBuffer_X*)pItem->m_pData;

    // found them?
    if (!pMeshDataset)
        return 0;

    // allocate memory for the new mesh
    pMesh = (CSR_Mesh*)csrMemoryAlloc(pX->m_pMesh, sizeof(CSR_Mesh), pX->m_MeshCount + 1);

    // succeeded?
    if (!pMesh)
        return 0;

    // keep the newly added mesh index
    index = pX->m_MeshCount;

    // update the model mesh data
    pX->m_pMesh = pMesh;
    ++pX->m_MeshCount;

    // initialize the mesh
    csrMeshInit(&pX->m_pMesh[index]);

    // is model supporting animations?
    if (!pX->m_MeshOnly)
    {
        // add a new mesh skin weights to the model
        pMeshWeights = csrMemoryAlloc(pX->m_pMeshWeights,
                                      sizeof(CSR_MeshSkinWeights_X),
                                      pX->m_MeshWeightsCount + 1);

        // succeeded?
        if (!pMeshWeights)
            return 0;

        // keep the mesh weights index
        meshWeightsIndex = pX->m_MeshWeightsCount;

        // update the model
        pX->m_pMeshWeights = pMeshWeights;
        ++pX->m_MeshWeightsCount;

        // initialize the mesh skin weights item
        pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights = 0;
        pX->m_pMeshWeights[meshWeightsIndex].m_Count        = 0;
    }
    else
        meshWeightsIndex = 0;

    pMatListItem    = 0;
    pNormalsDataset = 0;
    pUVDataset      = 0;
    pMatListDataset = 0;

    // search in the mesh children which are useful and should be used to build the mesh
    for (i = 0; i < pItem->m_ChildrenCount; ++i)
        switch (pItem->m_pChildren[i].m_ID)
        {
            case CSR_XI_Mesh_Normals_ID:
                // found the normals dataset, get it
                pNormalsDataset = (CSR_Dataset_VertexBuffer_X*)pItem->m_pChildren[i].m_pData;

                // succeeded?
                if (!pNormalsDataset)
                    return 0;

                continue;

            case CSR_XI_Mesh_Texture_Coords_ID:
                // found the texture coordinates dataset, get it
                pUVDataset = (CSR_Dataset_TexCoords_X*)pItem->m_pChildren[i].m_pData;

                // succeeded?
                if (!pUVDataset)
                    return 0;

                continue;

            case CSR_XI_Mesh_Material_List_ID:
                // found the material list, get it
                pMatListItem = &pItem->m_pChildren[i];

                // and get its dataset
                pMatListDataset = (CSR_Dataset_MaterialList_X*)pMatListItem->m_pData;

                // succeeded?
                if (!pMatListDataset)
                    return 0;

                continue;

            case CSR_XI_Skin_Weights_ID:
            {
                size_t                     length;
                size_t                     weightIndex;
                size_t                     weightsLength;
                CSR_Skin_Weights*          pSkinWeights;
                CSR_Dataset_SkinWeights_X* pSkinWeightsDataset;

                // mesh only should be drawn, ignore the skin weights
                if (pX->m_MeshOnly)
                    continue;

                // found the skin weights dataset, get it
                pSkinWeightsDataset = (CSR_Dataset_SkinWeights_X*)pItem->m_pChildren[i].m_pData;

                // succeeded?
                if (!pSkinWeightsDataset)
                    return 0;

                // add a new skin weights to the mesh skin weights
                pSkinWeights = csrMemoryAlloc(pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights,
                                              sizeof(CSR_Skin_Weights),
                                              pX->m_pMeshWeights[meshWeightsIndex].m_Count + 1);

                // succeeded?
                if (!pSkinWeights)
                    return 0;

                // keep the mesh weight index
                weightIndex = pX->m_pMeshWeights[meshWeightsIndex].m_Count;

                // update the model mesh skin weights list
                pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights = pSkinWeights;
                ++pX->m_pMeshWeights[meshWeightsIndex].m_Count;

                // initialize the skin weights
                csrSkinWeightsInit(&pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex]);

                // get the bone link name
                if (pSkinWeightsDataset->m_pBoneName)
                {
                    // allocate memory for the bone link name
                    length                                                                       = strlen(pSkinWeightsDataset->m_pBoneName);
                    pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pBoneName = (char*)malloc(length + 1);

                    // succeeded?
                    if (pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pBoneName)
                    {
                        // get the bone name to link to
                        #ifdef _MSC_VER
                            if (pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pBoneName)
                                strcpy_s(pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pBoneName,
                                         length + 1,
                                         pSkinWeightsDataset->m_pBoneName);
                        #else
                            strcpy(pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pBoneName,
                                   pSkinWeightsDataset->m_pBoneName);
                            pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pBoneName[length] = '\0';
                        #endif
                    }
                }
                else
                    // orphan skin weights? This should be a bug...
                    return 0;

                // get the matrix
                pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_Matrix = pSkinWeightsDataset->m_Matrix;

                // allocate memory for the weights table
                weightsLength                                                               = pSkinWeightsDataset->m_WeightCount * sizeof(float);
                pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pWeights = (float*)malloc(weightsLength);

                // succeeded
                if (!pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pWeights)
                    return 0;

                // get the weights
                memcpy(pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pWeights,
                       pSkinWeightsDataset->m_pWeights,
                       weightsLength);
                pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_WeightCount =
                        pSkinWeightsDataset->m_WeightCount;

                // allocate memory for the vertex index table
                pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pIndexTable =
                        (CSR_Skin_Weight_Index_Table*)calloc(pSkinWeightsDataset->m_IndiceCount,
                                                             sizeof(CSR_Skin_Weight_Index_Table));

                // succeeded?
                if (!pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_pIndexTable)
                    return 0;

                // set the vertex table item count
                pX->m_pMeshWeights[meshWeightsIndex].m_pSkinWeights[weightIndex].m_IndexTableCount =
                        pSkinWeightsDataset->m_IndiceCount;

                continue;
            }

            default:
                continue;
        }

    // model contains bones?
    if (pBone)
    {
        // allocate memory for the new mesh-to-bone dictionary item
        pMeshBoneItem = (CSR_MeshBoneItem_X*)csrMemoryAlloc(pX->m_pMeshToBoneDict,
                                                            sizeof(CSR_MeshBoneItem_X),
                                                            pX->m_MeshToBoneDictCount + 1);

        // succeeded?
        if (!pMeshBoneItem)
            return 0;

        // update the model mesh data
        pX->m_pMeshToBoneDict = pMeshBoneItem;
        ++pX->m_MeshToBoneDictCount;

        // get memory for the bone dictionary item content
        pBone->m_pCustomData = malloc(sizeof(size_t));

        // succeeded?
        if (!pBone->m_pCustomData)
            return 0;

        // link the mesh to the bone owning it
        pX->m_pMeshToBoneDict[index].m_MeshIndex = index;
        *((size_t*)pBone->m_pCustomData)         = index;
    }

    // create a new vertex buffer for the mesh
    pX->m_pMesh[index].m_pVB = csrVertexBufferCreate();

    // succeeded?
    if (!pX->m_pMesh[index].m_pVB)
        return 0;

    pX->m_pMesh[index].m_Count = 1;

    // apply the user wished vertex format
    if (pVertFormat)
        pX->m_pMesh[index].m_pVB->m_Format = *pVertFormat;

    // apply the user wished vertex culling
    if (pVertCulling)
        pX->m_pMesh[index].m_pVB->m_Culling = *pVertCulling;

    // apply the user wished material
    if (pMaterial)
        pX->m_pMesh[index].m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pX->m_pMesh[index].m_pVB->m_Format.m_Type = CSR_VT_Triangles;

    // calculate the stride
    csrVertexFormatCalculateStride(&pX->m_pMesh[index].m_pVB->m_Format);

    hasTexture = 0;

    // mesh contains materials?
    if (pMatListItem)
        // load all the material textures
        for (i = 0; i < pMatListItem->m_ChildrenCount; ++i)
        {
            // get the next material item
            CSR_Item_X* pMaterialItem = csrXGetMaterial(pMatListItem, i);

            // found it and is really a material?
            if (!pMaterialItem || pMaterialItem->m_ID != CSR_XI_Material_ID)
                return 0;

            // iterate through material children
            for (j = 0; j < pMaterialItem->m_ChildrenCount; ++j)
                // has a texture?
                if (pMaterialItem->m_pChildren[j].m_ID == CSR_XI_Texture_Filename_ID)
                {
                    int    canRelease;
                    size_t length;

                    // get the texture dataset
                    CSR_Dataset_Texture_X* pTextureDataset =
                            (CSR_Dataset_Texture_X*)pMaterialItem->m_pChildren[j].m_pData;

                    // found it?
                    if (!pTextureDataset)
                        return 0;

                    // allocate memory for the texture file name
                    length                                          = strlen(pTextureDataset->m_pFileName);
                    pX->m_pMesh[index].m_Skin.m_Texture.m_pFileName = (char*)malloc(length + 1);

                    // succeeded?
                    if (!pX->m_pMesh[index].m_Skin.m_Texture.m_pFileName)
                        return 0;

                    // get the file name
                    #ifdef _MSC_VER
                        if (pX->m_pMesh[index].m_Skin.m_Texture.m_pFileName)
                            strcpy_s(pX->m_pMesh[index].m_Skin.m_Texture.m_pFileName, length + 1, pTextureDataset->m_pFileName);
                    #else
                        strcpy(pX->m_pMesh[index].m_Skin.m_Texture.m_pFileName, pTextureDataset->m_pFileName);
                        pX->m_pMesh[index].m_Skin.m_Texture.m_pFileName[length] = '\0';
                    #endif

                    // load the texture
                    if (fOnLoadTexture)
                        pX->m_pMesh[index].m_Skin.m_Texture.m_pBuffer = fOnLoadTexture(pTextureDataset->m_pFileName);

                    canRelease = 0;

                    // apply the skin
                    if (fOnApplySkin)
                        fOnApplySkin(0, &pX->m_pMesh[index].m_Skin, &canRelease);

                    // can release the texture buffer?
                    if (canRelease)
                    {
                        csrPixelBufferRelease(pX->m_pMesh[index].m_Skin.m_Texture.m_pBuffer);
                        pX->m_pMesh[index].m_Skin.m_Texture.m_pBuffer = 0;
                    }

                    // normally each material should contain only one texture
                    hasTexture = 1;
                    break;
                }

                // for now only one texture is allowed per mesh. NOTE don't know if it's a
                // limitation, should be monitored in the future
                if (hasTexture)
                    break;
        }

    // keep the previous color, it may change while the mesh is created
    prevColor     = pX->m_pMesh[index].m_pVB->m_Material.m_Color;
    materialIndex = 0;

    // iterate through indices table
    for (i = 0; i < pMeshDataset->m_IndiceCount; i += pMeshDataset->m_pIndices[i] + 1)
    {
        // iterate through source vertices
        for (j = 0; j < pMeshDataset->m_pIndices[i] - 1; ++j)
        {
            // calculate the next polygon index
            const size_t polygonIndex = i + j + 1;

            // build the polygon
            for (k = 0; k < 3; ++k)
            {
                size_t vertIndex;

                // calculate the next vertex index
                if (!k)
                    vertIndex = i + 1;
                else
                    vertIndex = polygonIndex + (k - 1);

                    // build the vertex
                if (!csrXBuildVertex(pItem,
                                     pX,
                                     pMesh,
                                     index,
                                     vertIndex,
                                     materialIndex,
                                     prevColor,
                                     pMeshDataset,
                                     pNormalsDataset,
                                     pUVDataset,
                                     pMatListItem,
                                     pMatListDataset,
                                     fOnGetVertexColor))
                    return 0;
            }
        }

        ++materialIndex;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrXBuildAnimationSet(const CSR_Item_X* pItem, CSR_X* pX)
{
    size_t i;
    size_t j;
    size_t k;
    size_t index;

    // allocate memory for a new animation set
    CSR_AnimationSet_Bone* pAnimationSet =
            (CSR_AnimationSet_Bone*)csrMemoryAlloc(pX->m_pAnimationSet,
                                                   sizeof(CSR_AnimationSet_Bone),
                                                   pX->m_AnimationSetCount + 1);

    // succeeded?
    if (!pAnimationSet)
        return 0;

    // keep the animation set index
    index = pX->m_AnimationSetCount;

    // update the model animation sets
    pX->m_pAnimationSet = pAnimationSet;
    ++pX->m_AnimationSetCount;

    // initialize the animation set content
    csrBoneAnimSetInit(&pX->m_pAnimationSet[index]);

    // iterate through source animations
    for (i = 0; i < pItem->m_ChildrenCount; ++i)
    {
        // allocate memory for a new animation
        CSR_Animation_Bone* pAnimation =
                (CSR_Animation_Bone*)csrMemoryAlloc(pX->m_pAnimationSet[index].m_pAnimation,
                                                    sizeof(CSR_Animation_Bone),
                                                    pX->m_pAnimationSet[index].m_Count + 1);

        // succeeded?
        if (!pAnimation)
            return 0;

        // update the model animations
        pX->m_pAnimationSet[index].m_pAnimation = pAnimation;
        ++pX->m_pAnimationSet[index].m_Count;

        // initialize the animation content
        csrBoneAnimInit(&pX->m_pAnimationSet[index].m_pAnimation[i]);

        // iterate through source animation keys
        for (j = 0; j < pItem->m_pChildren[i].m_ChildrenCount; ++j)
        {
            CSR_AnimationKeys*           pAnimationKeys;
            CSR_Dataset_AnimationKeys_X* pData;

            // is a link?
            if (pItem->m_pChildren[i].m_pChildren[j].m_ID == CSR_XI_Link_ID)
            {
                size_t nameLength;

                // get the dataset containing the animation keys
                CSR_Dataset_Generic_X* pData =
                        (CSR_Dataset_Generic_X* )pItem->m_pChildren[i].m_pChildren[j].m_pData;

                if (!pData)
                    return 0;

                // get the bone name to link with
                if (pData->m_pName)
                {
                    nameLength                                             = strlen(pData->m_pName) + 1;
                    pX->m_pAnimationSet[index].m_pAnimation[i].m_pBoneName = (char*)malloc(nameLength);
                    #ifdef _MSC_VER
                        if (pX->m_pAnimationSet[index].m_pAnimation[i].m_pBoneName)
                            strcpy_s(pX->m_pAnimationSet[index].m_pAnimation[i].m_pBoneName, nameLength, pData->m_pName);
                    #else
                        strcpy(pX->m_pAnimationSet[index].m_pAnimation[i].m_pBoneName, pData->m_pName);
                        pX->m_pAnimationSet[index].m_pAnimation[i].m_pBoneName[nameLength - 1] = '\0';
                    #endif
                }

                continue;
            }

            // allocate memory for a new animation keys
            pAnimationKeys =
                    (CSR_AnimationKeys*)csrMemoryAlloc(pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys,
                                                       sizeof(CSR_AnimationKeys),
                                                       pX->m_pAnimationSet[index].m_pAnimation[i].m_Count + 1);

            // succeeded?
            if (!pAnimationKeys)
                return 0;

            // update the model animation keys
            pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys = pAnimationKeys;
            ++pX->m_pAnimationSet[index].m_pAnimation[i].m_Count;

            // initialize the animation keys content
            csrAnimKeysInit(&pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j]);

            // get the dataset containing the animation keys
            pData = (CSR_Dataset_AnimationKeys_X*)pItem->m_pChildren[i].m_pChildren[j].m_pData;

            // succeeded?
            if (!pData)
                return 0;

            // update the animation key type
            pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_Type = pData->m_Type;

            // iterate through keys
            for (k = 0; k < pData->m_KeyCount; ++k)
            {
                // allocate memory for a new animation key
                CSR_AnimationKey* pAnimationKey =
                        (CSR_AnimationKey*)csrMemoryAlloc(pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_pKey,
                                                          sizeof(CSR_AnimationKeys),
                                                          pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_Count + 1);

                // succeeded?
                if (!pAnimationKey)
                    return 0;

                // update the animation keys
                pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_pKey = pAnimationKey;
                ++pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_Count;

                // get the key frame and assign memory for values
                pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_pKey[k].m_Frame   = pData->m_pKeys[k].m_Frame;
                pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_pKey[k].m_pValues =
                        (float*)malloc(pData->m_pKeys[k].m_Count * sizeof(float));

                if (!pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_pKey[k].m_pValues)
                    return 0;

                // get the key values
                memcpy(pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_pKey[k].m_pValues,
                       pData->m_pKeys[k].m_pValues,
                       pData->m_pKeys[k].m_Count * sizeof(float));

                // get the key count
                pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j].m_pKey[k].m_Count = pData->m_pKeys[k].m_Count;
            }
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrXBuildParentHierarchy(CSR_Bone* pBone, CSR_Bone* pParent, CSR_X* pX)
{
    size_t i;

    // set bone parent
    pBone->m_pParent = pParent;

    // link the bone to the mesh
    if (pBone->m_pCustomData)
        for (i = 0; i < pX->m_MeshToBoneDictCount; ++i)
            if (pX->m_pMeshToBoneDict[i].m_MeshIndex == *((size_t*)pBone->m_pCustomData))
                pX->m_pMeshToBoneDict[i].m_pBone = pBone;

    // build children hierarchy
    for (i = 0; i < pBone->m_ChildrenCount; ++i)
        csrXBuildParentHierarchy(&pBone->m_pChildren[i], pBone, pX);
}
//---------------------------------------------------------------------------
// X model functions
//---------------------------------------------------------------------------
CSR_X* csrXCreate(const CSR_Buffer*           pBuffer,
                  const CSR_VertexFormat*     pVertFormat,
                  const CSR_VertexCulling*    pVertCulling,
                  const CSR_Material*         pMaterial,
                        int                   meshOnly,
                        int                   poseOnly,
                  const CSR_fOnGetVertexColor fOnGetVertexColor,
                  const CSR_fOnLoadTexture    fOnLoadTexture,
                  const CSR_fOnApplySkin      fOnApplySkin,
                  const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    CSR_X*       pX;
    CSR_Header_X header;
    size_t       offset;
    CSR_Item_X*  pRoot;
    CSR_Item_X*  pLocalRoot;

    // is buffer valid?
    if (!pBuffer || !pBuffer->m_Length)
        return 0;

    offset = 0;

    // read the header
    csrBufferRead(pBuffer, &offset, sizeof(CSR_Header_X), 1, &header);

    // is a .x file?
    if (header.m_Magic != M_X_FORMAT_MAGIC)
        return 0;

    // only 3.2 or 3.3 versions are supported
    if ((header.m_Major_Version != M_X_FORMAT_VERSION03) ||
        (header.m_Minor_Version != M_X_FORMAT_VERSION03) &&
        (header.m_Minor_Version != M_X_FORMAT_VERSION02))
        return 0;

    // is .x file containing text?
    if (header.m_Format != M_X_FORMAT_TEXT)
        return 0;

    // create the root item
    pRoot = (CSR_Item_X*)malloc(sizeof(CSR_Item_X));

    // succeeded?
    if (!pRoot)
        return 0;

    // initialize it
    csrXInitItem(pRoot);

    // as the root pointer itself may change while parsing, keep a local copy of the root pointer
    pLocalRoot = pRoot;

    // parse the file content
    if (!csrXParse(pBuffer, &offset, &pRoot))
    {
        csrXReleaseItems(pLocalRoot, 0);
        return 0;
    }

    // create the x model
    pX = (CSR_X*)malloc(sizeof(CSR_X));

    // succeeded?
    if (!pX)
    {
        csrXReleaseItems(pLocalRoot, 0);
        return 0;
    }

    csrXInit(pX);

    // configure it
    pX->m_MeshOnly = meshOnly;
    pX->m_PoseOnly = poseOnly;

    // convert the read item hierarchy to an x model
    if (!csrXItemToModel(pLocalRoot,
                         pX,
                         0,
                         pVertFormat,
                         pVertCulling,
                         pMaterial,
                         fOnGetVertexColor,
                         fOnLoadTexture,
                         fOnApplySkin,
                         fOnDeleteTexture))
    {
        csrXReleaseItems(pLocalRoot, 0);
        csrXRelease(pX, fOnDeleteTexture);
        return 0;
    }

    // build the bones parent hierarchy (could not simply keep the pointer while hierarchy was built
    // because the bone pointers may change several time while their hierarchy is built)
    if (pX->m_pSkeleton)
    {
        csrXBuildParentHierarchy(pX->m_pSkeleton, 0, pX);

        // skin weights?
        if (pX->m_pMeshWeights)
        {
            size_t i;
            size_t j;

            // retrieve the bone linked with each skin weights
            for (i = 0; i < pX->m_MeshWeightsCount; ++i)
                for (j = 0; j < pX->m_pMeshWeights[i].m_Count; ++j)
                    pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone =
                            csrBoneFind(pX->m_pSkeleton, pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBoneName);
        }

        // animation set?
        if (!pX->m_PoseOnly && pX->m_pAnimationSet)
        {
            size_t i;
            size_t j;

            // find each bone linked to animation sets
            for (i = 0; i < pX->m_AnimationSetCount; ++i)
                for (j = 0; j < pX->m_pAnimationSet[i].m_Count; ++j)
                    pX->m_pAnimationSet[i].m_pAnimation[j].m_pBone =
                            csrBoneFind(pX->m_pSkeleton, pX->m_pAnimationSet[i].m_pAnimation[j].m_pBoneName);
        }
    }

    // release the parsed items (since now no longer used)
    csrXReleaseItems(pLocalRoot, 0);

    return pX;
}
//---------------------------------------------------------------------------
CSR_X* csrXOpen(const char*                 pFileName,
                const CSR_VertexFormat*     pVertFormat,
                const CSR_VertexCulling*    pVertCulling,
                const CSR_Material*         pMaterial,
                      int                   meshOnly,
                      int                   poseOnly,
                const CSR_fOnGetVertexColor fOnGetVertexColor,
                const CSR_fOnLoadTexture    fOnLoadTexture,
                const CSR_fOnApplySkin      fOnApplySkin,
                const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    CSR_Buffer* pBuffer;
    CSR_X*      pX;

    // open the model file
    pBuffer = csrFileOpen(pFileName);

    // succeeded?
    if (!pBuffer || !pBuffer->m_Length)
    {
        csrBufferRelease(pBuffer);
        return 0;
    }

    // create the X model from the file content
    pX = csrXCreate(pBuffer,
                    pVertFormat,
                    pVertCulling,
                    pMaterial,
                    meshOnly,
                    poseOnly,
                    fOnGetVertexColor,
                    fOnLoadTexture,
                    fOnApplySkin,
                    fOnDeleteTexture);

    // release the file buffer (no longer required)
    csrBufferRelease(pBuffer);

    return pX;
}
//---------------------------------------------------------------------------
void csrXRelease(CSR_X* pX, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t i;
    size_t j;

    // no X model to release?
    if (!pX)
        return;

    // do free the meshes content?
    if (pX->m_pMesh)
    {
        // iterate through meshes to free
        for (i = 0; i < pX->m_MeshCount; ++i)
        {
            // delete the skin
            csrSkinContentRelease(&pX->m_pMesh[i].m_Skin, fOnDeleteTexture);

            // do free the mesh vertex buffer?
            if (pX->m_pMesh[i].m_pVB)
            {
                // free the mesh vertex buffer content
                for (j = 0; j < pX->m_pMesh[i].m_Count; ++j)
                    if (pX->m_pMesh[i].m_pVB[j].m_pData)
                        free(pX->m_pMesh[i].m_pVB[j].m_pData);

                // free the mesh vertex buffer
                free(pX->m_pMesh[i].m_pVB);
            }
        }

        // free the meshes
        free(pX->m_pMesh);
    }

    // release the weights
    if (pX->m_pMeshWeights)
    {
        // release the mesh weights content
        for (i = 0; i < pX->m_MeshWeightsCount; ++i)
        {
            // release the mesh skin weights content
            for (j = 0; j < pX->m_pMeshWeights[i].m_Count; ++j)
                csrSkinWeightsRelease(&pX->m_pMeshWeights[i].m_pSkinWeights[j], 1);

            // free the mesh skin weights
            free(pX->m_pMeshWeights[i].m_pSkinWeights);
        }

        // free the mesh weights
        free(pX->m_pMeshWeights);
    }

    // release the mesh-to-bone dictionary
    if (pX->m_pMeshToBoneDict)
        free(pX->m_pMeshToBoneDict);

    // release the bones
    csrBoneRelease(pX->m_pSkeleton, 0, 1);

    // release the animation sets
    if (pX->m_pAnimationSet)
    {
        // release the animation set content
        for (i = 0; i < pX->m_AnimationSetCount; ++i)
            csrBoneAnimSetRelease(&pX->m_pAnimationSet[i], 1);

        // free the animation sets
        free(pX->m_pAnimationSet);
    }

    // release the model
    free(pX);
}
//---------------------------------------------------------------------------
void csrXInit(CSR_X* pX)
{
    // no X model to initialize?
    if (!pX)
        return;

    // initialize the x model
    pX->m_pMesh               = 0;
    pX->m_MeshCount           = 0;
    pX->m_pMeshWeights        = 0;
    pX->m_MeshWeightsCount    = 0;
    pX->m_pMeshToBoneDict     = 0;
    pX->m_MeshToBoneDictCount = 0;
    pX->m_pSkeleton           = 0;
    pX->m_pAnimationSet       = 0;
    pX->m_AnimationSetCount   = 0;
    pX->m_MeshOnly            = 0;
    pX->m_PoseOnly            = 0;
}
//---------------------------------------------------------------------------
int csrXParse(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_Item_X** pItem)
{
    size_t wordOffset    = *pOffset;
    int    readingString = 0;

    /*
    xof 0303txt 0032

    Frame Root {
      FrameTransformMatrix {
         1.000000, 0.000000, 0.000000, 0.000000,
         0.000000,-0.000000, 1.000000, 0.000000,
         0.000000, 1.000000, 0.000000, 0.000000,
         0.000000, 0.000000, 0.000000, 1.000000;;
      }
      Frame Cube {
        FrameTransformMatrix {
           1.000000, 0.000000, 0.000000, 0.000000,
           0.000000, 1.000000, 0.000000, 0.000000,
           0.000000, 0.000000, 1.000000, 0.000000,
           0.000000, 0.000000, 0.000000, 1.000000;;
        }
        Mesh { // Cube mesh
          8;
           1.000000; 1.000000;-1.000000;,
           1.000000;-1.000000;-1.000000;,
          -1.000000;-1.000000;-1.000000;,
          -1.000000; 1.000000;-1.000000;,
           1.000000; 0.999999; 1.000000;,
           0.999999;-1.000001; 1.000000;,
          -1.000000;-1.000000; 1.000000;,
          -1.000000; 1.000000; 1.000000;;
          6;
          4;3,2,1,0;,
          4;5,6,7,4;,
          4;1,5,4,0;,
          4;2,6,5,1;,
          4;3,7,6,2;,
          4;7,3,0,4;;
          MeshNormals { // Cube normals
            6;
             0.000000; 0.000000;-1.000000;,
             0.000000;-0.000000; 1.000000;,
             1.000000;-0.000000; 0.000000;,
            -0.000000;-1.000000;-0.000000;,
            -1.000000; 0.000000;-0.000000;,
             0.000000; 1.000000; 0.000000;;
            6;
            4;0,0,0,0;,
            4;1,1,1,1;,
            4;2,2,2,2;,
            4;3,3,3,3;,
            4;4,4,4,4;,
            4;5,5,5,5;;
          } // End of Cube normals
          MeshMaterialList { // Cube material list
            1;
            6;
            0,
            0,
            0,
            0,
            0,
            0;
            Material Material {
               0.640000; 0.640000; 0.640000; 1.000000;;
               96.078431;
               0.500000; 0.500000; 0.500000;;
               0.000000; 0.000000; 0.000000;;
            }
          } // End of Cube material list
        } // End of Cube mesh
      } // End of Cube
    } // End of Root
    */
    while (*pOffset < pBuffer->m_Length)
        switch (((char*)pBuffer->m_pData)[*pOffset])
        {
            case '\r':
            case '\n':
            case '\t':
            case ' ':
            case ',':
            case ';':
                // reading a string?
                if (readingString)
                {
                    // ignore it in this case
                    ++(*pOffset);
                    continue;
                }

                // parse the next word
                if (*pOffset > wordOffset)
                    csrXParseWord(pBuffer, wordOffset, *pOffset, pItem);

                // skip the following separators since the current offset
                csrXSkipSeparators(pBuffer, pOffset);

                // set the next word start offset
                wordOffset = *pOffset;
                continue;

            case '{':
            case '}':
                // reading a string?
                if (readingString)
                {
                    // ignore it in this case
                    ++(*pOffset);
                    continue;
                }

                // parse the next word
                if (*pOffset > wordOffset)
                    csrXParseWord(pBuffer, wordOffset, *pOffset, pItem);

                // parse the opening or closing brace
                csrXParseWord(pBuffer, *pOffset, *pOffset + 1, pItem);

                // go to next char
                ++(*pOffset);

                // skip the following separators since the current offset
                csrXSkipSeparators(pBuffer, pOffset);

                // set the next word start offset
                wordOffset = *pOffset;
                continue;

            case '/':
                // reading a string?
                if (readingString)
                {
                    // ignore it in this case
                    ++(*pOffset);
                    continue;
                }

                // parse the next word
                if (*pOffset > wordOffset)
                    csrXParseWord(pBuffer, wordOffset, *pOffset, pItem);

                // next char should also be a slash, otherwise it's an error
                if ((*pOffset + 1) >= pBuffer->m_Length || ((char*)pBuffer->m_pData)[*pOffset + 1] != '/')
                    return 0;

                // skip the text until next line
                csrXSkipLine(pBuffer, pOffset);

                // set the next word start offset
                wordOffset = *pOffset;
                continue;

            case '#':
                // reading a string?
                if (readingString)
                {
                    // ignore it in this case
                    ++(*pOffset);
                    continue;
                }

                // parse the next word
                if (*pOffset > wordOffset)
                    csrXParseWord(pBuffer, wordOffset, *pOffset, pItem);

                // skip the text until next line
                csrXSkipLine(pBuffer, pOffset);

                // set the next word start offset
                wordOffset = *pOffset;
                continue;

            case '\"':
                // begin or end to read a string
                if (readingString)
                    readingString = 0;
                else
                    readingString = 1;

                ++(*pOffset);
                continue;

            default:
                ++(*pOffset);
                continue;
        }

    return 1;
}
//---------------------------------------------------------------------------
int csrXParseWord(const CSR_Buffer* pBuffer, size_t startOffset, size_t endOffset, CSR_Item_X** pItem)
{
    CSR_Item_X* pChild;

    // should always have an item defined
    if (!pItem || !(*pItem))
        return 0;

    // translate the word
    switch (csrXTranslateWord(pBuffer, startOffset, endOffset))
    {
        case CSR_XI_Template_ID:
        {
            CSR_Dataset_Generic_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateGenericDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Template_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Header_ID:
        {
            CSR_Dataset_Header_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateHeaderDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Header_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Frame_ID:
        {
            CSR_Dataset_Generic_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateGenericDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Frame_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Frame_Transform_Matrix_ID:
        {
            CSR_Dataset_Matrix_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateMatrixDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Frame_Transform_Matrix_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Mesh_ID:
        {
            CSR_Dataset_VertexBuffer_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateVertexBufferDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Mesh_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Mesh_Texture_Coords_ID:
        {
            CSR_Dataset_TexCoords_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateTexCoordsDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Mesh_Texture_Coords_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Mesh_Material_List_ID:
        {
            CSR_Dataset_MaterialList_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateMaterialListDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Mesh_Material_List_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Material_ID:
        {
            CSR_Dataset_Material_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateMaterialDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Material_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Skin_Mesh_Header_ID:
        {
            CSR_Dataset_Header_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateHeaderDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Skin_Mesh_Header_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Skin_Weights_ID:
        {
            CSR_Dataset_SkinWeights_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateSkinWeightsDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Skin_Weights_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Texture_Filename_ID:
        {
            CSR_Dataset_Texture_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateTextureDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Texture_Filename_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Mesh_Normals_ID:
        {
            CSR_Dataset_VertexBuffer_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateVertexBufferDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Mesh_Normals_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Animation_Set_ID:
        {
            CSR_Dataset_Generic_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateGenericDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Animation_Set_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Animation_ID:
        {
            CSR_Dataset_Generic_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateGenericDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Animation_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XI_Animation_Key_ID:
        {
            CSR_Dataset_AnimationKeys_X* pData;

            // sometimes dataset name may be the same as another known dataset. Check if it's the
            // case here and read the template name if yes
            if (*pItem                              &&
              !(*pItem)->m_Opened                   &&
               (*pItem)->m_ID >= CSR_XI_Template_ID &&
               (*pItem)->m_ID <= CSR_XI_Animation_Key_ID)
                return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // create the dataset
            pData = csrXCreateAnimationKeysDataset();

            // succeeded?
            if (!pData)
                return 0;

            // add a new template child item
            pChild = csrXAddChild(*pItem, CSR_XI_Animation_Key_ID, pData);

            // succeeded?
            if (!pChild)
            {
                free(pData);
                return 0;
            }

            // set the newly added child item as the current one
            *pItem = pChild;

            return 1;
        }

        case CSR_XT_Open_Brace:
            // found another open brace in an open dataset?
            if ((*pItem)->m_Opened)
            {
                // maybe an anonymous dataset or a link. Create the data
                CSR_Dataset_Generic_X* pData = csrXCreateGenericDataset();

                // succeeded?
                if (!pData)
                    return 0;

                // add a new child item
                pChild = csrXAddChild(*pItem, CSR_XI_Unknown, pData);

                // succeeded?
                if (!pChild)
                    return 0;

                // set the newly added child item as the current one
                *pItem = pChild;
            }

            (*pItem)->m_Opened = 1;
            return 1;

        case CSR_XT_Close_Brace:
            // close the dataset
            (*pItem)->m_Opened      = 0;
            (*pItem)->m_ContentRead = 1;

            // get the parent back
            *pItem = (*pItem)->m_pParent;

            return 1;

        case CSR_XT_String:
            // was the item opened?
            if ((*pItem)->m_Opened)
                switch ((*pItem)->m_ID)
                {
                    case CSR_XI_Texture_Filename_ID:
                    {
                        // get item data
                        CSR_Dataset_Texture_X* pData = (CSR_Dataset_Texture_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // get the texture file name (without the quotes)
                        pData->m_pFileName = csrXGetText(pBuffer, startOffset + 1, endOffset - 1);

                        return 1;
                    }

                    case CSR_XI_Skin_Weights_ID:
                    {
                        // get item data
                        CSR_Dataset_SkinWeights_X* pData = (CSR_Dataset_SkinWeights_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // get the skin weight linked bone name (without the quotes)
                        pData->m_pBoneName = csrXGetText(pBuffer, startOffset + 1, endOffset - 1);

                        return 1;
                    }

                    default:
                        return 1;
                }

                return 1;

        case CSR_XT_Float:
            // was the item opened?
            if ((*pItem)->m_Opened)
                switch ((*pItem)->m_ID)
                {
                    case CSR_XI_Frame_Transform_Matrix_ID:
                    {
                        // get item data
                        CSR_Dataset_Matrix_X* pData = (CSR_Dataset_Matrix_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // read values exceeded?
                        if (pData->m_ReadValCount < 16)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            if (!pValue)
                                return 0;

                            // convert value
                            pData->m_Matrix.m_Table[pData->m_ReadValCount / 4][pData->m_ReadValCount % 4] = (float)atof(pValue);

                            free(pValue);

                            ++pData->m_ReadValCount;
                        }

                        return 1;
                    }

                    case CSR_XI_Mesh_ID:
                    case CSR_XI_Mesh_Normals_ID:
                    {
                        // get item data
                        CSR_Dataset_VertexBuffer_X* pData =
                                (CSR_Dataset_VertexBuffer_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read a new vertex?
                        if (pData->m_VerticeCount < pData->m_VerticeTotal * 3)
                        {
                            char*  pValue;
                            float* pVertices = (float*)csrMemoryAlloc(pData->m_pVertices,
                                                                      sizeof(float),
                                                                      pData->m_VerticeCount + 1);

                            if (!pVertices)
                                return 0;

                            // get the value to convert
                            pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            if (!pValue)
                                return 0;

                            // convert value
                            pVertices[pData->m_VerticeCount] = (float)atof(pValue);

                            free(pValue);

                            // update the vertices
                            pData->m_pVertices = pVertices;
                            ++pData->m_VerticeCount;
                        }

                        return 1;
                    }

                    case CSR_XI_Mesh_Texture_Coords_ID:
                    {
                        // get item data
                        CSR_Dataset_TexCoords_X* pData = (CSR_Dataset_TexCoords_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read a new texture coordinate?
                        if (pData->m_UVCount < pData->m_UVTotal * 2)
                        {
                            char*  pValue;
                            float* pUV = (float*)csrMemoryAlloc(pData->m_pUV,
                                                                sizeof(float),
                                                                pData->m_UVCount + 1);

                            if (!pUV)
                                return 0;

                            // get the value to convert
                            pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            if (!pValue)
                                return 0;

                            // convert value
                            pUV[pData->m_UVCount] = (float)atof(pValue);

                            free(pValue);

                            // update the texture coordinates
                            pData->m_pUV = pUV;
                            ++pData->m_UVCount;
                        }

                        return 1;
                    }

                    case CSR_XI_Material_ID:
                    {
                        char* pValue;

                        // get item data
                        CSR_Dataset_Material_X* pData = (CSR_Dataset_Material_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // get the value to convert
                        pValue = csrXGetText(pBuffer, startOffset, endOffset);

                        if (!pValue)
                            return 0;

                        // convert the next value
                        switch (pData->m_ReadValCount)
                        {
                            case 0:  pData->m_Color.m_R         = (float)atof(pValue); break;
                            case 1:  pData->m_Color.m_G         = (float)atof(pValue); break;
                            case 2:  pData->m_Color.m_B         = (float)atof(pValue); break;
                            case 3:  pData->m_Color.m_A         = (float)atof(pValue); break;
                            case 4:  pData->m_SpecularExp       = (float)atof(pValue); break;
                            case 5:  pData->m_SpecularColor.m_R = (float)atof(pValue); break;
                            case 6:  pData->m_SpecularColor.m_G = (float)atof(pValue); break;
                            case 7:  pData->m_SpecularColor.m_B = (float)atof(pValue); break;
                            case 8:  pData->m_EmisiveColor.m_R  = (float)atof(pValue); break;
                            case 9:  pData->m_EmisiveColor.m_G  = (float)atof(pValue); break;
                            case 10: pData->m_EmisiveColor.m_B  = (float)atof(pValue); break;
                        }

                        free(pValue);

                        ++pData->m_ReadValCount;

                        return 1;
                    }

                    case CSR_XI_Skin_Weights_ID:
                    {
                        char* pValue;

                        // get item data
                        CSR_Dataset_SkinWeights_X* pData =
                                (CSR_Dataset_SkinWeights_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read a new skin weight?
                        if (pData->m_WeightCount < pData->m_ItemCount)
                        {
                            float* pWeights = (float*)csrMemoryAlloc(pData->m_pWeights,
                                                                     sizeof(float),
                                                                     pData->m_WeightCount + 1);

                            if (!pWeights)
                                return 0;

                            // get the value to convert
                            pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            if (!pValue)
                                return 0;

                            // convert value
                            pWeights[pData->m_WeightCount] = (float)atof(pValue);

                            free(pValue);

                            // update the texture coordinates
                            pData->m_pWeights = pWeights;
                            ++pData->m_WeightCount;
                        }
                        else
                        if (pData->m_ReadValCount < 16)
                        {
                            // get the value to convert
                            pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            if (!pValue)
                                return 0;

                            // convert value
                            pData->m_Matrix.m_Table[pData->m_ReadValCount / 4][pData->m_ReadValCount % 4] = (float)atof(pValue);

                            free(pValue);

                            ++pData->m_ReadValCount;
                        }

                        return 1;
                    }

                    case CSR_XI_Animation_Key_ID:
                    {
                        float* pValues;
                        char*  pValue;

                        // get item data
                        CSR_Dataset_AnimationKeys_X* pData =
                                (CSR_Dataset_AnimationKeys_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        pValues = (float*)csrMemoryAlloc(pData->m_pKeys[pData->m_KeyIndex].m_pValues,
                                                         sizeof(float),
                                                         pData->m_pKeys[pData->m_KeyIndex].m_Count + 1);

                        if (!pValues)
                            return 0;

                        // get the value to convert
                        pValue = csrXGetText(pBuffer, startOffset, endOffset);

                        if (!pValue)
                            return 0;

                        // convert value
                        pValues[pData->m_pKeys[pData->m_KeyIndex].m_Count] = (float)atof(pValue);

                        free(pValue);

                        // update the texture coordinates
                        pData->m_pKeys[pData->m_KeyIndex].m_pValues = pValues;
                        ++pData->m_pKeys[pData->m_KeyIndex].m_Count;

                        // if all data were read, go to next item
                        if (pData->m_pKeys[pData->m_KeyIndex].m_Count == pData->m_pKeys[pData->m_KeyIndex].m_Total)
                            ++pData->m_KeyIndex;

                        return 1;
                    }

                    default:
                        return 1;
                }

                return 1;

        case CSR_XT_Integer:
            // was the item opened?
            if ((*pItem)->m_Opened)
                switch ((*pItem)->m_ID)
                {
                    case CSR_XI_Header_ID:
                    case CSR_XI_Skin_Mesh_Header_ID:
                    {
                        char* pValue;

                        // get item data
                        CSR_Dataset_Header_X* pData = (CSR_Dataset_Header_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // get the value to convert
                        pValue = csrXGetText(pBuffer, startOffset, endOffset);

                        // convert value
                        switch (pData->m_ReadValCount)
                        {
                            case 0: pData->m_Major = atoi(pValue); break;
                            case 1: pData->m_Minor = atoi(pValue); break;
                            case 2: pData->m_Flags = atoi(pValue); break;
                        }

                        ++pData->m_ReadValCount;

                        free(pValue);

                        return 1;
                    }

                    case CSR_XI_Mesh_ID:
                    case CSR_XI_Mesh_Normals_ID:
                    {
                        // get item data
                        CSR_Dataset_VertexBuffer_X* pData =
                                (CSR_Dataset_VertexBuffer_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read the vertices or indices count, or a new index?
                        if (!pData->m_VerticeTotal)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_VerticeTotal = atoi(pValue);

                            free(pValue);
                        }
                        else
                        if (!pData->m_IndiceTotal)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_IndiceTotal = atoi(pValue);

                            free(pValue);
                        }
                        else
                        {
                            char*   pValue;
                            size_t* pIndices = (size_t*)csrMemoryAlloc(pData->m_pIndices,
                                                                       sizeof(size_t),
                                                                       pData->m_IndiceCount + 1);

                            if (!pIndices)
                                return 0;

                            // get the value to convert
                            pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pIndices[pData->m_IndiceCount] = atoi(pValue);

                            free(pValue);

                            // update the indices
                            pData->m_pIndices = pIndices;
                            ++pData->m_IndiceCount;
                        }

                        return 1;
                    }

                    case CSR_XI_Mesh_Texture_Coords_ID:
                    {
                        // get item data
                        CSR_Dataset_TexCoords_X* pData = (CSR_Dataset_TexCoords_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read the texture coordinate count?
                        if (!pData->m_UVTotal)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_UVTotal = atoi(pValue);

                            free(pValue);
                        }

                        return 1;
                    }

                    case CSR_XI_Mesh_Material_List_ID:
                    {
                        // get item data
                        CSR_Dataset_MaterialList_X* pData =
                                (CSR_Dataset_MaterialList_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read the material count, the material indices count, or a new indice?
                        if (!pData->m_MaterialCount)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_MaterialCount = atoi(pValue);

                            free(pValue);
                        }
                        else
                        if (!pData->m_MaterialIndiceTotal)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_MaterialIndiceTotal = atoi(pValue);

                            free(pValue);
                        }
                        else
                        if (pData->m_MaterialIndiceCount < pData->m_MaterialIndiceTotal)
                        {
                            char*   pValue;
                            size_t* pIndices = (size_t*)csrMemoryAlloc(pData->m_pMaterialIndices,
                                                                       sizeof(size_t),
                                                                       pData->m_MaterialIndiceCount + 1);

                            if (!pIndices)
                                return 0;

                            // get the value to convert
                            pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pIndices[pData->m_MaterialIndiceCount] = atoi(pValue);

                            free(pValue);

                            // update the indices
                            pData->m_pMaterialIndices = pIndices;
                            ++pData->m_MaterialIndiceCount;
                        }

                        return 1;
                    }

                    case CSR_XI_Skin_Weights_ID:
                    {
                        // get item data
                        CSR_Dataset_SkinWeights_X* pData =
                                (CSR_Dataset_SkinWeights_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read the skin weights item count, or a new index?
                        if (!pData->m_ItemCount)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_ItemCount = atoi(pValue);

                            free(pValue);
                        }
                        else
                        if (pData->m_IndiceCount < pData->m_ItemCount)
                        {
                            char*   pValue;
                            size_t* pIndices = (size_t*)csrMemoryAlloc(pData->m_pIndices,
                                                                       sizeof(size_t),
                                                                       pData->m_IndiceCount + 1);

                            if (!pIndices)
                                return 0;

                            // get the value to convert
                            pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pIndices[pData->m_IndiceCount] = atoi(pValue);

                            free(pValue);

                            // update the indices
                            pData->m_pIndices = pIndices;
                            ++pData->m_IndiceCount;
                        }

                        return 1;
                    }

                    case CSR_XI_Animation_Key_ID:
                    {
                        // get item data
                        CSR_Dataset_AnimationKeys_X* pData =
                                (CSR_Dataset_AnimationKeys_X*)(*pItem)->m_pData;

                        // found it?
                        if (!pData)
                            return 0;

                        // do read the key total, the key frame or the key value count?
                        if (pData->m_Type == CSR_KT_Unknown)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_Type = (CSR_EAnimKeyType)atoi(pValue);

                            free(pValue);
                        }
                        else
                        if (!pData->m_KeyTotal)
                        {
                            // get the value to convert
                            char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                            // convert value
                            pData->m_KeyTotal = atoi(pValue);

                            free(pValue);

                            // reserve the memory for the keys and initialize them to 0
                            pData->m_pKeys = (CSR_Dataset_AnimationKey_X*)calloc(pData->m_KeyTotal,
                                                                                 sizeof(CSR_Dataset_AnimationKey_X));

                            // succeeded?
                            if (!pData->m_pKeys)
                                return 0;

                            pData->m_KeyCount     = pData->m_KeyTotal;
                            pData->m_KeyIndex     = 0;
                            pData->m_ReadValCount = 0;
                        }
                        else
                        {
                            // search for value to read
                            switch (pData->m_ReadValCount)
                            {
                                // read the key frame
                                case 0:
                                {
                                    // get the value to convert
                                    char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                                    // convert value
                                    pData->m_pKeys[pData->m_KeyIndex].m_Frame = atoi(pValue);

                                    free(pValue);

                                    ++pData->m_ReadValCount;

                                    break;
                                }

                                // read the key value count
                                case 1:
                                {
                                    // get the value to convert
                                    char* pValue = csrXGetText(pBuffer, startOffset, endOffset);

                                    // convert value
                                    pData->m_pKeys[pData->m_KeyIndex].m_Total = atoi(pValue);

                                    free(pValue);

                                    pData->m_ReadValCount = 0;

                                    break;
                                }
                            }
                        }

                        return 1;
                    }

                    default:
                        return 1;
                }

                return 1;

        case CSR_XT_Name:
        {
            // get item data
            CSR_Dataset_Generic_X* pData = (CSR_Dataset_Generic_X*)(*pItem)->m_pData;

            // found it?
            if (!pData)
                return 0;

            // the name is a special dataset which contains the linked name to something else
            pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);
            (*pItem)->m_ID = CSR_XI_Link_ID;

            return 1;
        }

        default:
            // is the word the name of a dataset?
            if (*pItem && !(*pItem)->m_Opened)
                // item content was already read or is root node?
                if ((*pItem)->m_ContentRead || !(*pItem)->m_pParent)
                {
                    // probably the name of an unknown dataset. Create the data
                    CSR_Dataset_Generic_X* pData = csrXCreateGenericDataset();

                    // succeeded?
                    if (!pData)
                        return 0;

                    // add a new child item
                    pChild = csrXAddChild(*pItem, CSR_XI_Unknown, pData);

                    // succeeded?
                    if (!pChild)
                    {
                        free(pData);
                        return 0;
                    }

                    // keep the current word as name for the unknown dataset
                    pData->m_pName = csrXGetText(pBuffer, startOffset, endOffset);

                    // set the newly added child item as the current one
                    *pItem = pChild;

                    return 1;
                }
                else
                    return csrXReadDatasetName(pBuffer, startOffset, endOffset, *pItem);

            // if item is already opened, then the word isn't a dataset name but something else
            return 1;
    }
}
//---------------------------------------------------------------------------
int csrXItemToModel(const CSR_Item_X*           pItem,
                          CSR_X*                pX,
                          CSR_Bone*             pBone,
                    const CSR_VertexFormat*     pVertFormat,
                    const CSR_VertexCulling*    pVertCulling,
                    const CSR_Material*         pMaterial,
                    const CSR_fOnGetVertexColor fOnGetVertexColor,
                    const CSR_fOnLoadTexture    fOnLoadTexture,
                    const CSR_fOnApplySkin      fOnApplySkin,
                    const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    size_t    i;
    CSR_Bone* pCurrent = pBone;

    // must have a root item...
    if (!pItem)
        return 0;

    // ...and a destination x model
    if (!pX)
        return 0;

    switch (pItem->m_ID)
    {
        case CSR_XI_Frame_ID:
        {
            // get item data
            CSR_Dataset_Generic_X* pData = (CSR_Dataset_Generic_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // no current bone?
            if (!pBone)
            {
                // then the skeleton should also be not initialized
                if (pX->m_pSkeleton)
                    return 0;

                // create the root bone
                pX->m_pSkeleton = (CSR_Bone*)malloc(sizeof(CSR_Bone));

                // succeeded?
                if (!pX->m_pSkeleton)
                    return 0;

                // initialize it
                csrBoneInit(pX->m_pSkeleton);

                // set it as the current bone
                pCurrent = pX->m_pSkeleton;
            }
            else
            {
                size_t    index;
                CSR_Bone* pChildren;

                // create a new bone in the parent's children
                pChildren = csrMemoryAlloc(pBone->m_pChildren,
                                           sizeof(CSR_Bone),
                                           pBone->m_ChildrenCount + 1);

                if (!pChildren)
                    return 0;

                // keep the parent bone
                index = pBone->m_ChildrenCount;

                // initialize the newly created bone
                csrBoneInit(&pChildren[index]);

                // update the parent's children
                pBone->m_pChildren = pChildren;
                ++pBone->m_ChildrenCount;

                // set newly added bone as the current one
                pCurrent = &pChildren[index];
            }

            // from now current bone should always exist
            if (!pCurrent)
                return 0;

            // get the bone name
            if (pData->m_pName)
            {
                const size_t length = strlen(pData->m_pName);
                pCurrent->m_pName   = (char*)malloc(length + 1);

                #ifdef _MSC_VER
                if (pCurrent->m_pName)
                    strcpy_s(pCurrent->m_pName, length + 1, pData->m_pName);
                #else
                    strcpy(pCurrent->m_pName, pData->m_pName);
                    pCurrent->m_pName[length] = '\0';
                #endif
            }

            break;
        }

        case CSR_XI_Frame_Transform_Matrix_ID:
        {
            // get item data
            CSR_Dataset_Matrix_X* pData = (CSR_Dataset_Matrix_X*)pItem->m_pData;

            // found it?
            if (!pData)
                return 0;

            // should always have a parent bone
            if (!pBone)
                return 0;

            // get the bone matrix
            pBone->m_Matrix = pData->m_Matrix;
            return 1;
        }

        case CSR_XI_Mesh_ID:
            // build the mesh
            return csrXBuildMesh(pItem,
                                 pX,
                                 pBone,
                                 pVertFormat,
                                 pVertCulling,
                                 pMaterial,
                                 fOnGetVertexColor,
                                 fOnLoadTexture,
                                 fOnApplySkin,
                                 fOnDeleteTexture);

        case CSR_XI_Animation_Set_ID:
            // don't load the animation data if pose only is required
            if (pX->m_PoseOnly)
                return 1;

            // build the animation set
            return csrXBuildAnimationSet(pItem, pX);

        default:
            break;
    }

    // convert all children items
    for (i = 0; i < pItem->m_ChildrenCount; ++i)
        if (!csrXItemToModel(&pItem->m_pChildren[i],
                              pX,
                              pCurrent,
                              pVertFormat,
                              pVertCulling,
                              pMaterial,
                              fOnGetVertexColor,
                              fOnLoadTexture,
                              fOnApplySkin,
                              fOnDeleteTexture))
            return 0;

    return 1;
}
//---------------------------------------------------------------------------
void csrXReleaseItems(CSR_Item_X* pItem, int contentOnly)
{
    size_t i;

    // no item to release?
    if (!pItem)
        return;

    // release the dataset
    switch (pItem->m_ID)
    {
        case CSR_XI_Template_ID:
        case CSR_XI_Frame_ID:
        case CSR_XI_Animation_Set_ID:
        case CSR_XI_Animation_ID:
        case CSR_XI_Link_ID:
        case CSR_XI_Unknown:
        {
            // get the dataset
            CSR_Dataset_Generic_X* pData = (CSR_Dataset_Generic_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Header_ID:
        {
            // get the dataset
            CSR_Dataset_Header_X* pData = (CSR_Dataset_Header_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Frame_Transform_Matrix_ID:
        {
            // get the dataset
            CSR_Dataset_Matrix_X* pData = (CSR_Dataset_Matrix_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Mesh_ID:
        case CSR_XI_Mesh_Normals_ID:
        {
            // get the dataset
            CSR_Dataset_VertexBuffer_X* pData = (CSR_Dataset_VertexBuffer_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                if (pData->m_pVertices)
                    free(pData->m_pVertices);

                if (pData->m_pIndices)
                    free(pData->m_pIndices);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Mesh_Texture_Coords_ID:
        {
            // get the dataset
            CSR_Dataset_TexCoords_X* pData = (CSR_Dataset_TexCoords_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                if (pData->m_pUV)
                    free(pData->m_pUV);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Mesh_Material_List_ID:
        {
            // get the dataset
            CSR_Dataset_MaterialList_X* pData = (CSR_Dataset_MaterialList_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                if (pData->m_pMaterialIndices)
                    free(pData->m_pMaterialIndices);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Material_ID:
        {
            // get the dataset
            CSR_Dataset_Material_X* pData = (CSR_Dataset_Material_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Skin_Mesh_Header_ID:
        {
            // get the dataset
            CSR_Dataset_Header_X* pData = (CSR_Dataset_Header_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Skin_Weights_ID:
        {
            // get the dataset
            CSR_Dataset_SkinWeights_X* pData = (CSR_Dataset_SkinWeights_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                if (pData->m_pBoneName)
                    free(pData->m_pBoneName);

                if (pData->m_pIndices)
                    free(pData->m_pIndices);

                if (pData->m_pWeights)
                    free(pData->m_pWeights);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Texture_Filename_ID:
        {
            // get the dataset
            CSR_Dataset_Texture_X* pData = (CSR_Dataset_Texture_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                if (pData->m_pFileName)
                    free(pData->m_pFileName);

                free(pItem->m_pData);
            }

            break;
        }

        case CSR_XI_Animation_Key_ID:
        {
            // get the dataset
            CSR_Dataset_AnimationKeys_X* pData = (CSR_Dataset_AnimationKeys_X*)pItem->m_pData;

            // succeeded?
            if (pData)
            {
                // release its content
                if (pData->m_pName)
                    free(pData->m_pName);

                if (pData->m_pKeys)
                {
                    for (i = 0; i < pData->m_KeyCount; ++i)
                        free(pData->m_pKeys[i].m_pValues);

                    free(pData->m_pKeys);
                }

                free(pItem->m_pData);
            }

            break;
        }
    }

    // release the item children content
    for (i = 0; i < pItem->m_ChildrenCount; ++i)
        csrXReleaseItems(&pItem->m_pChildren[i], 1);

    // release the item children
    if (pItem->m_pChildren)
        free(pItem->m_pChildren);

    // release the item
    if (!contentOnly)
        free(pItem);
}
//---------------------------------------------------------------------------
