/****************************************************************************
 * ==> CSR_Wavefront -------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a Wavefront (.obj) reader             *
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

#include "CSR_Wavefront.h"

// std
#include <stdlib.h>
#include <math.h>
#include <string.h>

// visual studio specific code
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif

//------------------------------------------------------------------------------
// WaveFront functions
//------------------------------------------------------------------------------
CSR_Model* csrWaveFrontCreate(const CSR_Buffer*           pBuffer,
                              const CSR_VertexFormat*     pVertFormat,
                              const CSR_VertexCulling*    pVertCulling,
                              const CSR_Material*         pMaterial,
                              const CSR_fOnGetVertexColor fOnGetVertexColor,
                              const CSR_fOnApplySkin      fOnApplySkin,
                              const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    size_t                 i;
    int                    objectChanging;
    int                    groupChanging;
    char                   ch;
    CSR_WavefrontVertex*   pVertex;
    CSR_WavefrontNormal*   pNormal;
    CSR_WavefrontTexCoord* pUV;
    CSR_WavefrontFace*     pFace;
    CSR_Model*             pModel;

    // validate the input
    if (!pBuffer)
        return 0;

    // create a model
    pModel = csrModelCreate();

    // succeeded?
    if (!pModel)
        return 0;

    pVertex          = (CSR_WavefrontVertex*)  malloc(sizeof(CSR_WavefrontVertex));
    pNormal          = (CSR_WavefrontNormal*)  malloc(sizeof(CSR_WavefrontNormal));
    pUV              = (CSR_WavefrontTexCoord*)malloc(sizeof(CSR_WavefrontTexCoord));
    pFace            = (CSR_WavefrontFace*)    malloc(sizeof(CSR_WavefrontFace));
    pVertex->m_pData = 0;
    pVertex->m_Count = 0;
    pNormal->m_pData = 0;
    pNormal->m_Count = 0;
    pUV->m_pData     = 0;
    pUV->m_Count     = 0;
    pFace->m_pData   = 0;
    pFace->m_Count   = 0;
    objectChanging   = 0;
    groupChanging    = 0;

    // iterate through wavefront chars
    for (i = 0; i < pBuffer->m_Length; ++i)
    {
        // get the next char
        ch = ((char*)pBuffer->m_pData)[i];

        // dispatch char
        switch (ch)
        {
            // found commented line
            case '#':
                csrWaveFrontReadComment(pBuffer, &ch, &i);
                continue;

            case 'v':
                // do begin to read a new wavefront object or group?
                if (objectChanging || groupChanging)
                {
                    objectChanging = 0;
                    groupChanging  = 0;
                }

                // check if line contains a normal or a texture coordinate
                if (i + 1 < pBuffer->m_Length)
                {
                    // line contains a normal
                    if (((char*)pBuffer->m_pData)[i + 1] == 'n')
                    {
                        ++i;
                        csrWaveFrontReadNormal(pBuffer, &ch, &i, pNormal);
                        continue;
                    }

                    // line contains a texture coordinate
                    if (((char*)pBuffer->m_pData)[i + 1] == 't')
                    {
                        ++i;
                        csrWaveFrontReadTextureCoordinate(pBuffer, &ch, &i, pUV);
                        continue;
                    }
                }

                // line contains a vertex
                csrWaveFrontReadVertex(pBuffer, &ch, &i, pVertex);
                continue;

            case 'f':
                // line contains a face
                csrWaveFrontReadFace(pBuffer, &ch, &i, pFace);

                // build the face
                if (!csrWaveFrontBuildFace(pVertex,
                                           pNormal,
                                           pUV,
                                           pFace,
                                           pVertFormat,
                                           pVertCulling,
                                           pMaterial,
                                           objectChanging,
                                           0,
                                           pModel,
                                           fOnGetVertexColor,
                                           fOnApplySkin))
                {
                    // free the model
                    csrModelRelease(pModel, fOnDeleteTexture);

                    // free the local buffers
                    free(pVertex->m_pData);
                    free(pNormal->m_pData);
                    free(pUV->m_pData);
                    free(pFace->m_pData);
                    free(pVertex);
                    free(pNormal);
                    free(pUV);
                    free(pFace);

                    return 0;
                }

                // free the local face buffer
                free(pFace->m_pData);

                // reset the values
                pFace->m_pData = 0;
                pFace->m_Count = 0;

                groupChanging = 1;
                continue;

            case 'o':
                // line contains an object
                csrWaveFrontReadUnknown(pBuffer, &ch, &i);
                objectChanging = 1;
                continue;

            case 'g':
                // line contains a polygon group
                csrWaveFrontReadUnknown(pBuffer, &ch, &i);
                groupChanging = 1;
                continue;

            default:
                // unknown line, skip it
                csrWaveFrontReadUnknown(pBuffer, &ch, &i);
                continue;
        }
    }

    // free the local buffers
    free(pVertex->m_pData);
    free(pNormal->m_pData);
    free(pUV->m_pData);
    free(pFace->m_pData);
    free(pVertex);
    free(pNormal);
    free(pUV);
    free(pFace);

    return pModel;
}
//------------------------------------------------------------------------------
CSR_Model* csrWaveFrontOpen(const char*                 pFileName,
                            const CSR_VertexFormat*     pVertFormat,
                            const CSR_VertexCulling*    pVertCulling,
                            const CSR_Material*         pMaterial,
                            const CSR_fOnGetVertexColor fOnGetVertexColor,
                            const CSR_fOnApplySkin      fOnApplySkin,
                            const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    CSR_Buffer* pBuffer;
    CSR_Model*  pModel;

    // open the model file
    pBuffer = csrFileOpen(pFileName);

    // succeeded?
    if (!pBuffer || !pBuffer->m_Length)
    {
        csrBufferRelease(pBuffer);
        return 0;
    }

    // create the mesh from the file content
    pModel = csrWaveFrontCreate(pBuffer,
                                pVertFormat,
                                pVertCulling,
                                pMaterial,
                                fOnGetVertexColor,
                                fOnApplySkin,
                                fOnDeleteTexture);

    // release the file buffer (no longer required)
    csrBufferRelease(pBuffer);

    return pModel;
}
//---------------------------------------------------------------------------
void csrWaveFrontReadComment(const CSR_Buffer* pBuffer, char* pChar, size_t* pIndex)
{
    // skip line
    while (*pChar != '\r' && *pChar != '\n' && *pIndex < pBuffer->m_Length)
    {
        // go to next char
        ++(*pIndex);
        *pChar = ((char*)pBuffer->m_pData)[*pIndex];
    }
}
//---------------------------------------------------------------------------
void csrWaveFrontReadVertex(const CSR_Buffer*          pBuffer,
                                  char*                pChar,
                                  size_t*              pIndex,
                                  CSR_WavefrontVertex* pVertex)
{
    #ifdef _MSC_VER
        char   line[256] = {0};
        size_t lineIndex;
        int    doExit;
    #else
        char   line[256];
        size_t lineIndex;
        int    doExit;
    #endif

    lineIndex = 0;
    doExit    = 0;

    // read the line
    while (*pIndex < pBuffer->m_Length)
    {
        // dispatch the next char
        switch (*pChar)
        {
            case '\r':
            case '\n':
                doExit = 1;

            case ' ':
                // something to parse?
                if (lineIndex)
                    csrWaveFrontConvertFloat(line, &pVertex->m_pData, &pVertex->m_Count);

                // do exit the loop?
                if (doExit)
                    return;

                lineIndex = 0;
                break;

            default:
                // keep only numeric values
                if ((*pChar >= '0' && *pChar <= '9') || *pChar == '-' || *pChar == '.')
                {
                    line[lineIndex]     = *pChar;
                    line[lineIndex + 1] = '\0';
                    ++lineIndex;
                }

                break;
        }

        // go to next char
        ++(*pIndex);
        *pChar = ((char*)pBuffer->m_pData)[*pIndex];
    }
}
//---------------------------------------------------------------------------
void csrWaveFrontReadNormal(const CSR_Buffer*          pBuffer,
                                  char*                pChar,
                                  size_t*              pIndex,
                                  CSR_WavefrontNormal* pNormal)
{
    #ifdef _MSC_VER
        char   line[256] = {0};
        size_t lineIndex;
        int    doExit;
    #else
        char   line[256];
        size_t lineIndex;
        int    doExit;
    #endif

    lineIndex = 0;
    doExit    = 0;

    // read the line
    while (*pIndex < pBuffer->m_Length)
    {
        // dispatch the next char
        switch (*pChar)
        {
            case '\r':
            case '\n':
                doExit = 1;

            case ' ':
                // something to parse?
                if (lineIndex)
                    csrWaveFrontConvertFloat(line, &pNormal->m_pData, &pNormal->m_Count);

                // do exit the loop?
                if (doExit)
                    return;

                lineIndex = 0;
                break;

            default:
                // keep only numeric values
                if ((*pChar >= '0' && *pChar <= '9') || *pChar == '-' || *pChar == '.')
                {
                    line[lineIndex]     = *pChar;
                    line[lineIndex + 1] = '\0';
                    ++lineIndex;
                }

                break;
        }

        // go to next char
        ++(*pIndex);
        *pChar = ((char*)pBuffer->m_pData)[*pIndex];
    }
}
//---------------------------------------------------------------------------
void csrWaveFrontReadTextureCoordinate(const CSR_Buffer*            pBuffer,
                                             char*                  pChar,
                                             size_t*                pIndex,
                                             CSR_WavefrontTexCoord* pTexCoord)
{
    #ifdef _MSC_VER
        char   line[256] = {0};
        size_t lineIndex;
        int    doExit;
    #else
        char   line[256];
        size_t lineIndex;
        int    doExit;
    #endif

    lineIndex = 0;
    doExit    = 0;

    // read the line
    while (*pIndex < pBuffer->m_Length)
    {
        // dispatch the next char
        switch (*pChar)
        {
            case '\r':
            case '\n':
                doExit = 1;

            case ' ':
                // something to parse?
                if (lineIndex)
                    csrWaveFrontConvertFloat(line, &pTexCoord->m_pData, &pTexCoord->m_Count);

                // do exit the loop?
                if (doExit)
                    return;

                lineIndex = 0;
                break;

            default:
                // keep only numeric values
                if ((*pChar >= '0' && *pChar <= '9') || *pChar == '-' || *pChar == '.')
                {
                    line[lineIndex]     = *pChar;
                    line[lineIndex + 1] = '\0';
                    ++lineIndex;
                }

                break;
        }

        // go to next char
        ++(*pIndex);
        *pChar = ((char*)pBuffer->m_pData)[*pIndex];
    }
}
//---------------------------------------------------------------------------
void csrWaveFrontReadFace(const CSR_Buffer*        pBuffer,
                                char*              pChar,
                                size_t*            pIndex,
                                CSR_WavefrontFace* pFace)
{
    #ifdef _MSC_VER
        char   line[256] = {0};
        size_t lineIndex;
        int    doExit;
    #else
        char   line[256];
        size_t lineIndex;
        int    doExit;
    #endif

    lineIndex = 0;
    doExit    = 0;

    // read the line
    while (*pIndex < pBuffer->m_Length)
    {
        // dispatch the next char
        switch (*pChar)
        {
            case '\r':
            case '\n':
                doExit = 1;

            case ' ':
            case '/':
                // something to parse?
                if (lineIndex)
                    csrWaveFrontConvertInt(line, &pFace->m_pData, &pFace->m_Count);

                // do exit the loop?
                if (doExit)
                    return;

                lineIndex = 0;
                break;

            default:
                // keep only numeric values
                if ((*pChar >= '0' && *pChar <= '9') || *pChar == '-')
                {
                    line[lineIndex]     = *pChar;
                    line[lineIndex + 1] = '\0';
                    ++lineIndex;
                }

                break;
        }

        // go to next char
        ++(*pIndex);
        *pChar = ((char*)pBuffer->m_pData)[*pIndex];
    }
}
//---------------------------------------------------------------------------
void csrWaveFrontReadUnknown(const CSR_Buffer* pBuffer, char* pChar, size_t* pIndex)
{
    // skip line
    while (*pChar != '\r' && *pChar != '\n' && *pIndex < pBuffer->m_Length)
    {
        // go to next char
        ++(*pIndex);
        *pChar = ((char*)pBuffer->m_pData)[*pIndex];
    }
}
//---------------------------------------------------------------------------
void csrWaveFrontConvertFloat(const char* pBuffer, float** pArray, size_t* pCount)
{
    size_t index;

    // allocate memory for new value in array
    float* pData = (float*)csrMemoryAlloc(*pArray, sizeof(float), *pCount + 1);

    // succeeded?
    if (!pData)
        return;

    // keep the data index
    index = *pCount;

    // update the array
    *pArray = pData;
    ++(*pCount);

    // convert string to float and add it to array
    (*pArray)[index] = (float)atof(pBuffer);
}
//---------------------------------------------------------------------------
void csrWaveFrontConvertInt(const char* pBuffer, int** pArray, size_t* pCount)
{
    size_t index;

    // allocate memory for new value in array
    int* pData = (int*)csrMemoryAlloc(*pArray, sizeof(int), *pCount + 1);

    // succeeded?
    if (!pData)
        return;

    // keep the data index
    index = *pCount;

    // update the array
    *pArray = pData;
    ++(*pCount);

    // convert string to float and add it to array
    (*pArray)[index] = atoi(pBuffer);
}
//---------------------------------------------------------------------------
int csrWaveFrontBuildFace(const CSR_WavefrontVertex*   pVertex,
                          const CSR_WavefrontNormal*   pNormal,
                          const CSR_WavefrontTexCoord* pUV,
                          const CSR_WavefrontFace*     pFace,
                          const CSR_VertexFormat*      pVertFormat,
                          const CSR_VertexCulling*     pVertCulling,
                          const CSR_Material*          pMaterial,
                                int                    objectChanging,
                                int                    groupChanging,
                                CSR_Model*             pModel,
                          const CSR_fOnGetVertexColor  fOnGetVertexColor,
                          const CSR_fOnApplySkin       fOnApplySkin)
{
    CSR_Mesh*         pMesh;
    CSR_VertexBuffer* pVB;

    pMesh = 0;
    pVB   = 0;

    // do build the previous group or object?
    if (pVertex->m_Count && pFace->m_Count)
    {
        // do create a new mesh?
        if (objectChanging || !pModel->m_MeshCount)
        {
            CSR_Mesh* pNewMesh;

            // create a new mesh in the model
            pNewMesh = (CSR_Mesh*)csrMemoryAlloc(pModel->m_pMesh,
                                                 sizeof(CSR_Mesh),
                                                 pModel->m_MeshCount + 1);

            // succeeded?
            if (!pNewMesh)
                return 0;

            // keep the current mesh to update
            pMesh = &pNewMesh[pModel->m_MeshCount];

            // update the model meshes
            pModel->m_pMesh = pNewMesh;
            ++pModel->m_MeshCount;

            // initialize the newly created mesh
            csrMeshInit(pMesh);
        }
        else
            pMesh = &pModel->m_pMesh[pModel->m_MeshCount - 1];

        // do create a new vertex buffer?
        if (groupChanging || !pMesh->m_pVB)
        {
            CSR_VertexBuffer* pNewVB;

            // create a new vertex buffer in the mesh
            pNewVB = (CSR_VertexBuffer*)csrMemoryAlloc(pMesh->m_pVB,
                                                       sizeof(CSR_VertexBuffer),
                                                       pMesh->m_Count + 1);

            // succeeded?
            if (!pNewVB)
                return 0;

            // keep the current vertex buffer to update
            pVB = &pNewVB[pMesh->m_Count];

            // update the model meshes
            pMesh->m_pVB = pNewVB;
            ++pMesh->m_Count;

            // initialize the newly created vertex buffer
            csrVertexBufferInit(pVB);

            // apply the user wished vertex format
            if (pVertFormat)
                pVB->m_Format = *pVertFormat;

            // apply the user wished vertex culling
            if (pVertCulling)
                pVB->m_Culling = *pVertCulling;
            else
            {
                // otherwise configure the default culling
                pVB->m_Culling.m_Type = CSR_CT_Back;
                pVB->m_Culling.m_Face = CSR_CF_CW;
            }

            // apply the user wished material
            if (pMaterial)
                pVB->m_Material = *pMaterial;

            // configure the vertex format type
            pVB->m_Format.m_Type         = CSR_VT_Triangles;
            pVB->m_Format.m_HasNormal    = pNormal->m_Count ? pVertFormat->m_HasNormal    : 0;
            pVB->m_Format.m_HasTexCoords = pUV->m_Count     ? pVertFormat->m_HasTexCoords : 0;

            // calculate the stride
            csrVertexFormatCalculateStride(&pVB->m_Format);
        }
        else
            pVB = &pMesh->m_pVB[pMesh->m_Count - 1];

        // build the next buffer
        csrWaveFrontBuildVertexBuffer(pVertex,
                                      pNormal,
                                      pUV,
                                      pFace,
                                      pVB,
                                      fOnGetVertexColor,
                                      fOnApplySkin);
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrWaveFrontBuildVertexBuffer(const CSR_WavefrontVertex*   pVertex,
                                   const CSR_WavefrontNormal*   pNormal,
                                   const CSR_WavefrontTexCoord* pUV,
                                   const CSR_WavefrontFace*     pFace,
                                         CSR_VertexBuffer*      pVB,
                                   const CSR_fOnGetVertexColor  fOnGetVertexColor,
                                   const CSR_fOnApplySkin       fOnApplySkin)
{
    size_t i;
    size_t faceStride;
    size_t normalOffset;
    size_t uvOffset;
    int    baseVertexIndex;
    int    baseNormalIndex;
    int    baseUVIndex;

    // calculate the normal and uv offsets. Be careful, the face values follows one each other in
    // the file, without distinction, so the correct format (v, v/n, v/f or v/n/f) should be
    // determined and used
    uvOffset     = (pUV     && pUV->m_Count)     ? 1            : 0;
    normalOffset = (pNormal && pNormal->m_Count) ? uvOffset + 1 : 0;

    // wavefront faces are organized as triangle fan, so get the first vertex
    // and build all others from it
    baseVertexIndex = (pFace->m_pData[0] - 1) * 3;
    faceStride      = 1;

    // get the first texture coordinate
    if (pUV->m_Count)
    {
        baseUVIndex = (pFace->m_pData[uvOffset] - 1) * 2;
        ++faceStride;
    }
    else
        baseUVIndex = 0;

    // get the first normal
    if (pNormal->m_Count)
    {
        baseNormalIndex = (pFace->m_pData[normalOffset] - 1) * 3;
        ++faceStride;
    }
    else
        baseNormalIndex = 0;

    // iterate through remaining indices
    for (i = 1; i <= (pFace->m_Count / faceStride) - 2; ++i)
    {
        #ifdef _MSC_VER
            CSR_Vector3 vertex = {0};
            CSR_Vector3 normal = {0};
            CSR_Vector2 uv     = {0};
        #else
            CSR_Vector3 vertex;
            CSR_Vector3 normal;
            CSR_Vector2 uv;
        #endif

        // build polygon vertex 1
        int vertexIndex =                                baseVertexIndex;
        int uvIndex     = pVB->m_Format.m_HasTexCoords ? baseUVIndex     : 0;
        int normalIndex = pVB->m_Format.m_HasNormal    ? baseNormalIndex : 0;

        // set vertex data
        vertex.m_X = pVertex->m_pData[vertexIndex];
        vertex.m_Y = pVertex->m_pData[vertexIndex + 1];
        vertex.m_Z = pVertex->m_pData[vertexIndex + 2];

        // vertex has a normal?
        if (pVB->m_Format.m_HasNormal)
        {
            // set normal data
            normal.m_X = pNormal->m_pData[normalIndex];
            normal.m_Y = pNormal->m_pData[normalIndex + 1];
            normal.m_Z = pNormal->m_pData[normalIndex + 2];
        }

        // vertex has UV texture coordinates?
        if (pVB->m_Format.m_HasTexCoords)
        {
            // set texture data
            uv.m_X = pUV->m_pData[uvIndex];
            uv.m_Y = pUV->m_pData[uvIndex + 1];
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex,
                           &normal,
                           &uv,
                            vertexIndex,
                            fOnGetVertexColor,
                            pVB);

        // build polygon vertex 2
        vertexIndex =                                (pFace->m_pData[ i * faceStride]                 - 1) * 3;
        uvIndex     = pVB->m_Format.m_HasTexCoords ? (pFace->m_pData[(i * faceStride) + uvOffset]     - 1) * 2 : 0;
        normalIndex = pVB->m_Format.m_HasNormal    ? (pFace->m_pData[(i * faceStride) + normalOffset] - 1) * 3 : 0;

        // set vertex data
        vertex.m_X = pVertex->m_pData[vertexIndex];
        vertex.m_Y = pVertex->m_pData[vertexIndex + 1];
        vertex.m_Z = pVertex->m_pData[vertexIndex + 2];

        // vertex has a normal?
        if (pVB->m_Format.m_HasNormal)
        {
            // set normal data
            normal.m_X = pNormal->m_pData[normalIndex];
            normal.m_Y = pNormal->m_pData[normalIndex + 1];
            normal.m_Z = pNormal->m_pData[normalIndex + 2];
        }

        // vertex has UV texture coordinates?
        if (pVB->m_Format.m_HasTexCoords)
        {
            // set texture data
            uv.m_X = pUV->m_pData[uvIndex];
            uv.m_Y = pUV->m_pData[uvIndex + 1];
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex,
                           &normal,
                           &uv,
                            vertexIndex,
                            fOnGetVertexColor,
                            pVB);

        // build polygon vertex 3
        vertexIndex =                                (pFace->m_pData[ (i + 1) * faceStride]                 - 1) * 3;
        uvIndex     = pVB->m_Format.m_HasTexCoords ? (pFace->m_pData[((i + 1) * faceStride) + uvOffset]     - 1) * 2 : 0;
        normalIndex = pVB->m_Format.m_HasNormal    ? (pFace->m_pData[((i + 1) * faceStride) + normalOffset] - 1) * 3 : 0;

        // set vertex data
        vertex.m_X = pVertex->m_pData[vertexIndex];
        vertex.m_Y = pVertex->m_pData[vertexIndex + 1];
        vertex.m_Z = pVertex->m_pData[vertexIndex + 2];

        // vertex has a normal?
        if (pVB->m_Format.m_HasNormal)
        {
            // set normal data
            normal.m_X = pNormal->m_pData[normalIndex];
            normal.m_Y = pNormal->m_pData[normalIndex + 1];
            normal.m_Z = pNormal->m_pData[normalIndex + 2];
        }

        // vertex has UV texture coordinates?
        if (pVB->m_Format.m_HasTexCoords)
        {
            // set texture data
            uv.m_X = pUV->m_pData[uvIndex];
            uv.m_Y = pUV->m_pData[uvIndex + 1];
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex,
                           &normal,
                           &uv,
                            vertexIndex,
                            fOnGetVertexColor,
                            pVB);
    }
}
//---------------------------------------------------------------------------
