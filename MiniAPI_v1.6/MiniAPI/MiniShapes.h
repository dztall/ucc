/*****************************************************************************
 * ==> Minimal shapes library -----------------------------------------------*
 *****************************************************************************
 * Description : Minimal shapes library                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015, this file is part of the Minimal API. You are free    *
 *               to copy or redistribute this file, modify it, or use it for *
 *               your own projects, commercial or not                        *
 *****************************************************************************/

#ifndef MiniShapesH
#define MiniShapesH

// std
#include <stdlib.h>
#include <math.h>

// mini API
#include "MiniVertex.h"

//-----------------------------------------------------------------------------
// Shapes creation functions
//-----------------------------------------------------------------------------

/**
* Creates a surface
*@param pWidth - surface width
*@param pHeight - surface height
*@param color - color in RGBA format
*@param [in, out] pVertexFormat - vertex format to use
*@param[out] pVertices - resulting vertex buffer
*@param[out] pVertexCount - vertex count in buffer
*/
int CreateSurface(const float*           pWidth,
                  const float*           pHeight,
                        unsigned int     color,
                        MV_VertexFormat* pVertexFormat,
                        float**          pVertices,
                        unsigned*        pVertexCount)
{
    int i;
    int index;
    int offset;

    // create a buffer template: false for negative values, true for positive
    int bufferTemplate[] =
    {
        0, 0,
        0, 1,
        1, 0,
        1, 1,
    };

    CalculateStride(pVertexFormat);

    *pVertexCount = 4;

    // generate vertices
    if (!(*pVertices))
        *pVertices = (float*)malloc(*pVertexCount * sizeof(float) * pVertexFormat->m_Stride);
    else
        *pVertices = (float*)realloc(*pVertices,
                                     *pVertexCount * sizeof(float) * pVertexFormat->m_Stride);

    offset = 0;

    // iterate through vertex to create
    for (i = 0; i < 4; ++i)
    {
        // calculate template buffer index
        index = i * 2;

        // populate vertex buffer
        if (bufferTemplate[index])
            (*pVertices)[offset] =   *pWidth  / 2.0f;
        else
            (*pVertices)[offset] = -(*pWidth) / 2.0f;

        if (bufferTemplate[index + 1])
            (*pVertices)[offset + 1] =   *pHeight  / 2.0f;
        else
            (*pVertices)[offset + 1] = -(*pHeight) / 2.0f;

        (*pVertices)[offset + 2] = 0.0f;

        offset += 3;

        // do include normals?
        if (pVertexFormat->m_UseNormals)
        {
            // calculate normals
            (*pVertices)[offset]     =  0.0f;
            (*pVertices)[offset + 1] =  0.0f;
            (*pVertices)[offset + 2] = -1.0f;

            offset += 3;
        }

        // do include texture?
        if (pVertexFormat->m_UseTextures)
        {
            // calculate texture u coordinate
            if (bufferTemplate[index])
                (*pVertices)[offset] = 1.0f;
            else
                (*pVertices)[offset] = 0.0f;

            // calculate texture v coordinate
            if (bufferTemplate[index + 1])
                (*pVertices)[offset + 1] = 1.0f;
            else
                (*pVertices)[offset + 1] = 0.0f;

            offset += 2;
        }

        // do include colors?
        if (pVertexFormat->m_UseColors)
        {
            // set color data
            (*pVertices)[offset]     = (float)((color >> 24) & 0xFF) / 255.0f;
            (*pVertices)[offset + 1] = (float)((color >> 16) & 0xFF) / 255.0f;
            (*pVertices)[offset + 2] = (float)((color >> 8)  & 0xFF) / 255.0f;
            (*pVertices)[offset + 3] = (float) (color        & 0xFF) / 255.0f;

            offset += 4;
        }
    }

    return 1;
}

/**
* Creates a sphere
*@param pRadius - sphere radius
*@param slices - slices (longitude) number
*@param stacks - stacks (latitude) number
*@param color - color in RGBA format
*@param [in, out] pVertexFormat - vertex format to use
*@param[out] pVertices - resulting vertex buffer
*@param[out] pVertexCount - vertex count in buffer
*@param[out] pIndexes - index list containing offset and length of each vertex buffer composing sphere
*@param[out] pIndexCount - resulting index count
*/
void CreateSphere(const float*           pRadius,
                        int              slices,
                        int              stacks,
                        unsigned         color,
                        MV_VertexFormat* pVertexFormat,
                        float**          pVertices,
                        unsigned*        pVertexCount,
                        MV_Index**       pIndexes,
                        unsigned*        pIndexCount)
{
    int   i;
    int   j;
    int   iIndex;
    int   fanLength;
    int   index;
    float majorStep;
    float minorStep;
    float a;
    float b;
    float r0;
    float r1;
    float z0;
    float z1;
    float c;
    float x;
    float y;

    const float pi = atan(1.0f) * 4.0f;

    CalculateStride(pVertexFormat);

    // initialize basic values
    majorStep     = (pi          / slices);
    minorStep     = ((2.0f * pi) / stacks);
    *pVertices    = 0;
    *pVertexCount = 0;
    *pIndexes     = 0;
    *pIndexCount  = 0;

    // iterate through vertex slices
    for (i = 0; i <= slices; ++i)
    {
        // calculate values for next slice
        a  = i * majorStep;
        b  = a + majorStep;
        r0 = *pRadius * sinf(a);
        r1 = *pRadius * sinf(b);
        z0 = *pRadius * cosf(a);
        z1 = *pRadius * cosf(b);

        // generate new index in table
        if (!(*pIndexes))
            *pIndexes = (MV_Index*)malloc(sizeof(MV_Index));
        else
            *pIndexes = (MV_Index*)realloc(*pIndexes,
                                          (*pIndexCount + 1) * sizeof(MV_Index));

        // calculate current index and slice fan length
        iIndex    = *pIndexCount;
        fanLength = (stacks + 1) * pVertexFormat->m_Stride * 2;

        // increase index count
        ++(*pIndexCount);

        // populate index
        (*pIndexes)[iIndex].m_Start  = *pVertexCount;
        (*pIndexes)[iIndex].m_Length = fanLength;

        // calculate new vertex buffer length
        *pVertexCount += fanLength;

        // generate vertices
        if (!(*pVertices))
            *pVertices = (float*)malloc(*pVertexCount * sizeof(float));
        else
            *pVertices = (float*)realloc(*pVertices,
                                         *pVertexCount * sizeof(float));

        index = (*pIndexes)[iIndex].m_Start;

        // iterate through vertex stacks
        for (j = 0; j <= stacks; ++j)
        {
            c = j * minorStep;
            x = cosf(c);
            y = sinf(c);

            // set vertex data
            (*pVertices)[index]     = x * r0;
            (*pVertices)[index + 1] = y * r0;
            (*pVertices)[index + 2] = z0;

            index += 3;

            // do generate normals?
            if (pVertexFormat->m_UseNormals)
            {
                // set normals
                (*pVertices)[index]     = (x * r0) / *pRadius;
                (*pVertices)[index + 1] = (y * r0) / *pRadius;
                (*pVertices)[index + 2] = z0       / *pRadius;

                index += 3;
            }

            // do generate texture coordinates?
            if (pVertexFormat->m_UseTextures)
            {
                // add texture coordinates data to buffer
                (*pVertices)[index]     = ((float)j / (float)stacks);
                (*pVertices)[index + 1] = ((float)i / (float)slices);

                index += 2;
            }

            // do generate colors?
            if (pVertexFormat->m_UseColors)
            {
                // set color data
                (*pVertices)[index]     = (float)((color >> 24) & 0xFF) / 255.0f;
                (*pVertices)[index + 1] = (float)((color >> 16) & 0xFF) / 255.0f;
                (*pVertices)[index + 2] = (float)((color >> 8)  & 0xFF) / 255.0f;
                (*pVertices)[index + 3] = (float) (color        & 0xFF) / 255.0f;

                index += 4;
            }

            (*pVertices)[index]     = x * r1;
            (*pVertices)[index + 1] = y * r1;
            (*pVertices)[index + 2] = z1;

            index += 3;

            // do generate normals?
            if (pVertexFormat->m_UseNormals)
            {
                // set normals
                (*pVertices)[index]     = (x * r1) / *pRadius;
                (*pVertices)[index + 1] = (y * r1) / *pRadius;
                (*pVertices)[index + 2] =  z1      / *pRadius;

                index += 3;
            }

            // do generate texture coordinates?
            if (pVertexFormat->m_UseTextures)
            {
                // add texture coordinates data to buffer
                (*pVertices)[index]     = ( (float)j         / (float)stacks);
                (*pVertices)[index + 1] = (((float)i + 1.0f) / (float)slices);

                index += 2;
            }

            // do generate colors?
            if (pVertexFormat->m_UseColors)
            {
                // set color data
                (*pVertices)[index]     = (float)((color >> 24) & 0xFF) / 255.0f;
                (*pVertices)[index + 1] = (float)((color >> 16) & 0xFF) / 255.0f;
                (*pVertices)[index + 2] = (float)((color >> 8)  & 0xFF) / 255.0f;
                (*pVertices)[index + 3] = (float) (color        & 0xFF) / 255.0f;

                index += 4;
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Landscape creation functions
//-----------------------------------------------------------------------------

/**
* Landscape model
*/
typedef struct
{
    float*       m_pVertexBuffer;
    unsigned int m_VertexCount;
} ML_Mesh;

/**
* Loads data to generate landscape from model image
*@param pFileName - bitmap file name to load from
*@param[out] pData - data to use to generate landscape
*@param[out] width - landscape width, in bytes
*@param[out] height - landscape height, in bytes
*@return 1 on success, otherwise 0
*@note For now only most common bitmaps are loaded, some bitmap types may be unsupported
*/
int LoadLandscape(const unsigned char* pFileName, unsigned char** pData, int* pWidth, int* pHeight)
{
    FILE*          pFile;
    GLuint         index;
    unsigned int   dataOffset;
    unsigned int   headerSize;
    unsigned int   x;
    unsigned int   y;
    unsigned int   bytesPerRow;
    unsigned int   bitmapSize;
    unsigned char  c;
    unsigned short bpp;
    unsigned short compressed;
    unsigned char  intBuffer[4];
    unsigned char  shortBuffer[2];

    // open bitmap file
    pFile = MINI_FILE_OPEN(pFileName, "rb");

    // succeeded?
    if (!pFile)
        return 0;

    // read bitmap signature
    MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);

    // is bitmap signature correct?
    if (shortBuffer[0] != 'B' || shortBuffer[1] != 'M')
        return 0;

    // skip 8 next bytes
    MINI_FILE_SEEK(pFile, 8, SEEK_CUR);

    // read data offset
    MINI_FILE_READ(&intBuffer[0], sizeof(unsigned char), 4, pFile);
    dataOffset = (unsigned int)(((unsigned char)intBuffer[3] << 24) |
                                ((unsigned char)intBuffer[2] << 16) |
                                ((unsigned char)intBuffer[1] << 8)  |
                                 (unsigned char)intBuffer[0]);

    // read header size
    MINI_FILE_READ(&intBuffer[0], sizeof(unsigned char), 4, pFile);
    headerSize = (unsigned int)(((unsigned char)intBuffer[3] << 24) |
                                ((unsigned char)intBuffer[2] << 16) |
                                ((unsigned char)intBuffer[1] << 8)  |
                                 (unsigned char)intBuffer[0]);

    // search for bitmap type
    switch (headerSize)
    {
        // V3
        case 40:
        {
            // read bitmap width
            MINI_FILE_READ(&intBuffer[0], sizeof(unsigned char), 4, pFile);
            *pWidth = (unsigned int)(((unsigned char)intBuffer[3] << 24) |
                                     ((unsigned char)intBuffer[2] << 16) |
                                     ((unsigned char)intBuffer[1] << 8)  |
                                      (unsigned char)intBuffer[0]);

            // read bitmap height
            MINI_FILE_READ(&intBuffer[0], sizeof(unsigned char), 4, pFile);
            *pHeight = (unsigned int)(((unsigned char)intBuffer[3] << 24) |
                                      ((unsigned char)intBuffer[2] << 16) |
                                      ((unsigned char)intBuffer[1] << 8)  |
                                       (unsigned char)intBuffer[0]);

            // skip next 2 bytes
            MINI_FILE_SEEK(pFile, 2, SEEK_CUR);

            // read bitmap bit per pixels
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            bpp = (unsigned short)(((unsigned char)shortBuffer[1] << 8) |
                                    (unsigned char)shortBuffer[0]);

            // is bpp supported?
            if (bpp != 24)
                return 0;

            // read bitmap compressed flag
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            compressed = (unsigned short)(((unsigned char)shortBuffer[1] << 8) |
                                           (unsigned char)shortBuffer[0]);

            // is compressed?
            if (compressed)
                return 0;

            break;
        }

        // OS/2 V1
        case 12:
        {
            // read bitmap width
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            *pWidth = (unsigned int)(((unsigned char)shortBuffer[1] << 8) |
                                      (unsigned char)shortBuffer[0]);

            // read bitmap height
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            *pHeight = (unsigned int)(((unsigned char)shortBuffer[1] << 8) |
                                       (unsigned char)shortBuffer[0]);

            // skip next 2 bytes
            MINI_FILE_SEEK(pFile, 2, SEEK_CUR);

            // read bitmap bit per pixels
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            bpp = (unsigned short)(((unsigned char)shortBuffer[1] << 8) |
                                    (unsigned char)shortBuffer[0]);

            // is bpp supported?
            if (bpp != 24)
                return 0;

            break;
        }

        // Windows V4
        case 108:
        {
            // read bitmap width
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            *pWidth = (unsigned int)(((unsigned char)shortBuffer[1] << 8) |
                                      (unsigned char)shortBuffer[0]);

            // skip next 2 bytes
            MINI_FILE_SEEK(pFile, 2, SEEK_CUR);

            // read bitmap height
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            *pHeight = (unsigned int)(((unsigned char)shortBuffer[1] << 8) |
                                       (unsigned char)shortBuffer[0]);

            // skip next 4 bytes
            MINI_FILE_SEEK(pFile, 4, SEEK_CUR);

            // read bitmap bit per pixels
            MINI_FILE_READ(&shortBuffer[0], sizeof(unsigned char), 2, pFile);
            bpp = (unsigned short)(((unsigned char)shortBuffer[1] << 8) |
                                    (unsigned char)shortBuffer[0]);

            // is bpp supported?
            if (bpp != 24)
                return 0;

            break;
        }

        default:
            // unsupported bitmap format
            return 0;
    }

    bytesPerRow = (((*pWidth) * 3 + 3) / 4) * 4 - ((*pWidth) * 3 % 4);
    bitmapSize  = bytesPerRow * (*pHeight);
    *pData      = (unsigned char*)malloc(sizeof(unsigned char) * bitmapSize);

    // read bitmap data
    MINI_FILE_SEEK(pFile, dataOffset, SEEK_SET);
    MINI_FILE_READ(*pData, sizeof(unsigned char), bitmapSize, pFile);

    // close file
    MINI_FILE_CLOSE(pFile);

    return 1;
}

/**
* Generates a landscape from a grayscale image
*@param pData - image data
*@param mapX - map width, same as image width in pixels
*@param mapZ - map deep, same as image height in pixels
*@param height - map height
*@param scale - scale factor
*@param[out] pMesh - mesh containing landscape
*@param[out] pMeshSize - size of landscape mesh
*/
void GenerateLandscapeMesh(unsigned char* pData,
                           int            mapX,
                           int            mapZ,
                           float          height,
                           float          scale,
                           MG_Vector3**   pMesh,
                           unsigned int*  pMeshSize)
{
    int          x;
    int          z;
    unsigned int index;
    float        scaleX;
    float        scaleZ;

    // calculate lansdcape data size and reserve memory for landscape mesh
    *pMeshSize = mapX * mapZ;
    *pMesh     = (MG_Vector3*)malloc(*pMeshSize * sizeof(MG_Vector3));

    // calculate scaling factor on x and z axis
    scaleX = -(((mapX - 1) * scale) / 2.0f);
    scaleZ =  (((mapZ - 1) * scale) / 2.0f);

    // loop through heightfield points and calculate coordinates for each point
    for (z = 0; z < mapZ; ++z)
        for (x = 0; x < mapX; ++x)
        {
            // calculate vertex index
            index = (z * mapZ) + x;

            // calculate landscape vertex
            (*pMesh)[index].m_X = scaleX + ((float)x * scale);
            (*pMesh)[index].m_Y = ((float)pData[index * 3] / 255.0f) * height;
            (*pMesh)[index].m_Z = scaleZ - ((float)z * scale);
        }
}

/**
* Creates a landscape mesh from a grayscale image
*@param pData - data image to use to generate landscape
*@param mapX - map width, same as image width in pixels
*@param mapZ - map deep, same as image height in pixels
*@param height - map height
*@param scale - scale factor
*@param pVertexFormat - landscape mesh vertex format
*@param color - landscape mesh color
*p@aram[out] pMesh - mesh containing landscape
*/
void CreateLandscape(unsigned char*   pData,
                     int              mapX,
                     int              mapZ,
                     float            height,
                     float            scale,
                     MV_VertexFormat* pVertexFormat,
                     unsigned int     color,
                     ML_Mesh**        pMesh)
{
    int          x;
    int          z;
    unsigned int index;
    unsigned int offset;
    unsigned int landscapeSize;
    MG_Vector3*  pLandscape;
    MG_Vector3   v1;
    MG_Vector3   v2;
    MG_Vector3   v3;
    MG_Vector3   v4;
    MG_Vector3   n1;
    MG_Vector3   n2;
    MG_Vector2   uv1;
    MG_Vector2   uv2;
    MG_Vector2   uv3;
    MG_Vector2   uv4;
    MG_Plane     p1;
    MG_Plane     p2;

    // by default stride is equals to 3 (for vertex XYZ position)
    pVertexFormat->m_Stride = 3;

    // do include normals?
    if (pVertexFormat->m_UseNormals)
        pVertexFormat->m_Stride += 3;

    // do include textures?
    if (pVertexFormat->m_UseTextures)
        pVertexFormat->m_Stride += 2;

    // do include colors?
    if (pVertexFormat->m_UseColors)
        pVertexFormat->m_Stride += 4;

    // generate landscape XYZ vertex from grayscale image
    GenerateLandscapeMesh(pData,
                          mapX,
                          mapZ,
                          height,
                          scale,
                          &pLandscape,
                          &landscapeSize);

    // create landscape mesh
    *pMesh = (ML_Mesh*)malloc(sizeof(ML_Mesh));
    (*pMesh)->m_pVertexBuffer = 0;
    (*pMesh)->m_VertexCount   = 0;

    // loop through landscape XYZ vertices and generate mesh polygons
    for (z = 0; z < mapZ - 1; ++z)
        for (x = 0; x < mapX - 1; ++x)
        {
            // polygons are generated in the following order:
            // v1 -- v2
            //     /
            //    /
            // v3 -- v4

            // calculate vertex index
            index = (z * mapZ) + x;

            // calculate first vertex
            v1.m_X = pLandscape[index].m_X;
            v1.m_Y = pLandscape[index].m_Y;
            v1.m_Z = pLandscape[index].m_Z;

            // calculate second vertex
            v2.m_X = pLandscape[index + 1].m_X;
            v2.m_Y = pLandscape[index + 1].m_Y;
            v2.m_Z = pLandscape[index + 1].m_Z;

            // calculate next vertex index
            index = ((z + 1) * mapZ) + x;

            // calculate third vertex
            v3.m_X = pLandscape[index].m_X;
            v3.m_Y = pLandscape[index].m_Y;
            v3.m_Z = pLandscape[index].m_Z;

            // calculate fourth vertex
            v4.m_X = pLandscape[index + 1].m_X;
            v4.m_Y = pLandscape[index + 1].m_Y;
            v4.m_Z = pLandscape[index + 1].m_Z;

            // do include normals?
            if (pVertexFormat->m_UseNormals)
            {
                // calculate polygons planes
                PlaneFromPoints(&v1, &v2, &v3, &p1);
                PlaneFromPoints(&v2, &v3, &v4, &p2);

                // get first normal
                n1.m_X = p1.m_A;
                n1.m_Y = p1.m_B;
                n1.m_Z = p1.m_C;

                // get second normal
                n2.m_X = p2.m_A;
                n2.m_Y = p2.m_B;
                n2.m_Z = p2.m_C;
            }

            // do include colors?
            if (pVertexFormat->m_UseColors)
            {
                // generate texture coordinates
                uv1.m_X = 0.0f;
                uv1.m_Y = 0.0f;
                uv2.m_X = 1.0f;
                uv2.m_Y = 0.0f;
                uv3.m_X = 0.0f;
                uv3.m_Y = 1.0f;
                uv4.m_X = 1.0f;
                uv4.m_Y = 1.0f;
            }

            // add first polygon first vertex to buffer
            AddVertexToVB(pVertexFormat,
                          &v1,
                          &n1,
                          &uv1,
                          color,
                          &(*pMesh)->m_pVertexBuffer,
                          &(*pMesh)->m_VertexCount);

            // add first polygon second vertex to buffer
            AddVertexToVB(pVertexFormat,
                          &v2,
                          &n1,
                          &uv2,
                          color,
                          &(*pMesh)->m_pVertexBuffer,
                          &(*pMesh)->m_VertexCount);

            // add first polygon third vertex to buffer
            AddVertexToVB(pVertexFormat,
                          &v3,
                          &n1,
                          &uv3,
                          color,
                          &(*pMesh)->m_pVertexBuffer,
                          &(*pMesh)->m_VertexCount);

            // add second polygon first vertex to buffer
            AddVertexToVB(pVertexFormat,
                          &v2,
                          &n2,
                          &uv2,
                          color,
                          &(*pMesh)->m_pVertexBuffer,
                          &(*pMesh)->m_VertexCount);

            // add second polygon second vertex to buffer
            AddVertexToVB(pVertexFormat,
                          &v3,
                          &n2,
                          &uv3,
                          color,
                          &(*pMesh)->m_pVertexBuffer,
                          &(*pMesh)->m_VertexCount);

            // add second polygon third vertex to buffer
            AddVertexToVB(pVertexFormat,
                          &v4,
                          &n2,
                          &uv4,
                          color,
                          &(*pMesh)->m_pVertexBuffer,
                          &(*pMesh)->m_VertexCount);
        }

    // delete landscape XYZ vertices (no longer used as copied in mesh)
    if (pLandscape)
    {
        free(pLandscape);
        pLandscape = 0;
    }
}

#endif // MiniShapesH
