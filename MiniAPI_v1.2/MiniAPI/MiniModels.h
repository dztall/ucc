/*****************************************************************************
 * ==> Minimal models library -----------------------------------------------*
 *****************************************************************************
 * Description : Minimal models library                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015, this file is part of the Minimal API. You are free    *
 *               to copy or redistribute this file, modify it, or use it for *
 *               your own projects, commercial or not                        *
 *****************************************************************************/

#ifndef MiniModelsH
#define MiniModelsH

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"

//-----------------------------------------------------------------------------
// Global defines
//-----------------------------------------------------------------------------

#define M_MD2_Mesh_File_Version 8
#define M_MD2_ID                (('2' << 24) + ('P' << 16) + ('D' << 8) + 'I')

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

/**
* Vertex index
*/
typedef struct
{
    int m_Start;
    int m_Length;
    int m_GlCmd;
} MM_Index;

/**
* MD2 header
*/
typedef struct
{
    unsigned int m_ID;
    unsigned int m_Version;
    unsigned int m_SkinWidth;
    unsigned int m_SkinHeight;
    unsigned int m_FrameSize;
    unsigned int m_SkinCount;
    unsigned int m_VertexCount;
    unsigned int m_TextureCoordCount;
    unsigned int m_PolygonCount;
    unsigned int m_GlCmdsCount;
    unsigned int m_FrameCount;
    unsigned int m_SkinOffset;
    unsigned int m_TextureCoordOffset;
    unsigned int m_PolygonOffset;
    unsigned int m_FrameOffset;
    unsigned int m_GlCmdsOffset;
    unsigned int m_EndOffset;
} MM_MD2Header;

/**
* MD2 model skin
*/
typedef struct
{
    char m_Name[64];
} MM_MD2Skin;

/**
* MD2 vertex
*/
typedef struct
{
    unsigned char m_Vertex[3];
    unsigned char m_NormalIndex;
} MM_MD2Vertex;

/**
* MD2 texture coordinates
*/
typedef struct
{
    unsigned short m_U;
    unsigned short m_V;
} MM_MD2TextureCoord;

/**
* MD2 frame
*/
typedef struct
{
    char          m_Name[16];
    float         m_Scale[3];
    float         m_Translate[3];
    MM_MD2Vertex* m_pVertex;
} MM_MD2Frame;

/**
* MD2 polygon
*/
typedef struct
{
    unsigned short m_VertexIndex[3];
    unsigned short m_TextureCoordIndex[3];
} MM_MD2Polygon;

/**
* Model mesh
*/
typedef struct
{
    float*       m_pVertexBuffer;
    unsigned int m_VertexCount;
    int          m_IsTriangleStrip;
} MM_Mesh;

/**
* Model frame
*/
typedef struct
{
    MM_Mesh*     m_pMesh;
    unsigned int m_MeshCount;
} MM_Frame;

/**
* MD2 model
*/
typedef struct
{
    MV_VertexFormat* m_pVertexFormat;
    MM_Frame*        m_pFrame;
    unsigned int     m_FrameCount;
} MM_MD2Model;

//------------------------------------------------------------------------------
// Normal table
//------------------------------------------------------------------------------

float g_NormalTable[] =
{
    -0.525731f,  0.000000f,  0.850651f,
    -0.442863f,  0.238856f,  0.864188f,
    -0.295242f,  0.000000f,  0.955423f,
    -0.309017f,  0.500000f,  0.809017f,
    -0.162460f,  0.262866f,  0.951056f,
     0.000000f,  0.000000f,  1.000000f,
     0.000000f,  0.850651f,  0.525731f,
    -0.147621f,  0.716567f,  0.681718f,
     0.147621f,  0.716567f,  0.681718f,
     0.000000f,  0.525731f,  0.850651f,
     0.309017f,  0.500000f,  0.809017f,
     0.525731f,  0.000000f,  0.850651f,
     0.295242f,  0.000000f,  0.955423f,
     0.442863f,  0.238856f,  0.864188f,
     0.162460f,  0.262866f,  0.951056f,
    -0.681718f,  0.147621f,  0.716567f,
    -0.809017f,  0.309017f,  0.500000f,
    -0.587785f,  0.425325f,  0.688191f,
    -0.850651f,  0.525731f,  0.000000f,
    -0.864188f,  0.442863f,  0.238856f,
    -0.716567f,  0.681718f,  0.147621f,
    -0.688191f,  0.587785f,  0.425325f,
    -0.500000f,  0.809017f,  0.309017f,
    -0.238856f,  0.864188f,  0.442863f,
    -0.425325f,  0.688191f,  0.587785f,
    -0.716567f,  0.681718f, -0.147621f,
    -0.500000f,  0.809017f, -0.309017f,
    -0.525731f,  0.850651f,  0.000000f,
     0.000000f,  0.850651f, -0.525731f,
    -0.238856f,  0.864188f, -0.442863f,
     0.000000f,  0.955423f, -0.295242f,
    -0.262866f,  0.951056f, -0.162460f,
     0.000000f,  1.000000f,  0.000000f,
     0.000000f,  0.955423f,  0.295242f,
    -0.262866f,  0.951056f,  0.162460f,
     0.238856f,  0.864188f,  0.442863f,
     0.262866f,  0.951056f,  0.162460f,
     0.500000f,  0.809017f,  0.309017f,
     0.238856f,  0.864188f, -0.442863f,
     0.262866f,  0.951056f, -0.162460f,
     0.500000f,  0.809017f, -0.309017f,
     0.850651f,  0.525731f,  0.000000f,
     0.716567f,  0.681718f,  0.147621f,
     0.716567f,  0.681718f, -0.147621f,
     0.525731f,  0.850651f,  0.000000f,
     0.425325f,  0.688191f,  0.587785f,
     0.864188f,  0.442863f,  0.238856f,
     0.688191f,  0.587785f,  0.425325f,
     0.809017f,  0.309017f,  0.500000f,
     0.681718f,  0.147621f,  0.716567f,
     0.587785f,  0.425325f,  0.688191f,
     0.955423f,  0.295242f,  0.000000f,
     1.000000f,  0.000000f,  0.000000f,
     0.951056f,  0.162460f,  0.262866f,
     0.850651f, -0.525731f,  0.000000f,
     0.955423f, -0.295242f,  0.000000f,
     0.864188f, -0.442863f,  0.238856f,
     0.951056f, -0.162460f,  0.262866f,
     0.809017f, -0.309017f,  0.500000f,
     0.681718f, -0.147621f,  0.716567f,
     0.850651f,  0.000000f,  0.525731f,
     0.864188f,  0.442863f, -0.238856f,
     0.809017f,  0.309017f, -0.500000f,
     0.951056f,  0.162460f, -0.262866f,
     0.525731f,  0.000000f, -0.850651f,
     0.681718f,  0.147621f, -0.716567f,
     0.681718f, -0.147621f, -0.716567f,
     0.850651f,  0.000000f, -0.525731f,
     0.809017f, -0.309017f, -0.500000f,
     0.864188f, -0.442863f, -0.238856f,
     0.951056f, -0.162460f, -0.262866f,
     0.147621f,  0.716567f, -0.681718f,
     0.309017f,  0.500000f, -0.809017f,
     0.425325f,  0.688191f, -0.587785f,
     0.442863f,  0.238856f, -0.864188f,
     0.587785f,  0.425325f, -0.688191f,
     0.688191f,  0.587785f, -0.425325f,
    -0.147621f,  0.716567f, -0.681718f,
    -0.309017f,  0.500000f, -0.809017f,
     0.000000f,  0.525731f, -0.850651f,
    -0.525731f,  0.000000f, -0.850651f,
    -0.442863f,  0.238856f, -0.864188f,
    -0.295242f,  0.000000f, -0.955423f,
    -0.162460f,  0.262866f, -0.951056f,
     0.000000f,  0.000000f, -1.000000f,
     0.295242f,  0.000000f, -0.955423f,
     0.162460f,  0.262866f, -0.951056f,
    -0.442863f, -0.238856f, -0.864188f,
    -0.309017f, -0.500000f, -0.809017f,
    -0.162460f, -0.262866f, -0.951056f,
     0.000000f, -0.850651f, -0.525731f,
    -0.147621f, -0.716567f, -0.681718f,
     0.147621f, -0.716567f, -0.681718f,
     0.000000f, -0.525731f, -0.850651f,
     0.309017f, -0.500000f, -0.809017f,
     0.442863f, -0.238856f, -0.864188f,
     0.162460f, -0.262866f, -0.951056f,
     0.238856f, -0.864188f, -0.442863f,
     0.500000f, -0.809017f, -0.309017f,
     0.425325f, -0.688191f, -0.587785f,
     0.716567f, -0.681718f, -0.147621f,
     0.688191f, -0.587785f, -0.425325f,
     0.587785f, -0.425325f, -0.688191f,
     0.000000f, -0.955423f, -0.295242f,
     0.000000f, -1.000000f,  0.000000f,
     0.262866f, -0.951056f, -0.162460f,
     0.000000f, -0.850651f,  0.525731f,
     0.000000f, -0.955423f,  0.295242f,
     0.238856f, -0.864188f,  0.442863f,
     0.262866f, -0.951056f,  0.162460f,
     0.500000f, -0.809017f,  0.309017f,
     0.716567f, -0.681718f,  0.147621f,
     0.525731f, -0.850651f,  0.000000f,
    -0.238856f, -0.864188f, -0.442863f,
    -0.500000f, -0.809017f, -0.309017f,
    -0.262866f, -0.951056f, -0.162460f,
    -0.850651f, -0.525731f,  0.000000f,
    -0.716567f, -0.681718f, -0.147621f,
    -0.716567f, -0.681718f,  0.147621f,
    -0.525731f, -0.850651f,  0.000000f,
    -0.500000f, -0.809017f,  0.309017f,
    -0.238856f, -0.864188f,  0.442863f,
    -0.262866f, -0.951056f,  0.162460f,
    -0.864188f, -0.442863f,  0.238856f,
    -0.809017f, -0.309017f,  0.500000f,
    -0.688191f, -0.587785f,  0.425325f,
    -0.681718f, -0.147621f,  0.716567f,
    -0.442863f, -0.238856f,  0.864188f,
    -0.587785f, -0.425325f,  0.688191f,
    -0.309017f, -0.500000f,  0.809017f,
    -0.147621f, -0.716567f,  0.681718f,
    -0.425325f, -0.688191f,  0.587785f,
    -0.162460f, -0.262866f,  0.951056f,
     0.442863f, -0.238856f,  0.864188f,
     0.162460f, -0.262866f,  0.951056f,
     0.309017f, -0.500000f,  0.809017f,
     0.147621f, -0.716567f,  0.681718f,
     0.000000f, -0.525731f,  0.850651f,
     0.425325f, -0.688191f,  0.587785f,
     0.587785f, -0.425325f,  0.688191f,
     0.688191f, -0.587785f,  0.425325f,
    -0.955423f,  0.295242f,  0.000000f,
    -0.951056f,  0.162460f,  0.262866f,
    -1.000000f,  0.000000f,  0.000000f,
    -0.850651f,  0.000000f,  0.525731f,
    -0.955423f, -0.295242f,  0.000000f,
    -0.951056f, -0.162460f,  0.262866f,
    -0.864188f,  0.442863f, -0.238856f,
    -0.951056f,  0.162460f, -0.262866f,
    -0.809017f,  0.309017f, -0.500000f,
    -0.864188f, -0.442863f, -0.238856f,
    -0.951056f, -0.162460f, -0.262866f,
    -0.809017f, -0.309017f, -0.500000f,
    -0.681718f,  0.147621f, -0.716567f,
    -0.681718f, -0.147621f, -0.716567f,
    -0.850651f,  0.000000f, -0.525731f,
    -0.688191f,  0.587785f, -0.425325f,
    -0.587785f,  0.425325f, -0.688191f,
    -0.425325f,  0.688191f, -0.587785f,
    -0.425325f, -0.688191f, -0.587785f,
    -0.587785f, -0.425325f, -0.688191f,
    -0.688191f, -0.587785f, -0.425325f
};

//-----------------------------------------------------------------------------
// Models functions
//-----------------------------------------------------------------------------

/**
* Reads MD2 header
*@param pFile - MD2 file
*@param[out] pHeader - MD2 header structure to populate
*/
void ReadMD2Header(FILE* pFile, MM_MD2Header* pHeader)
{
    // read header from file
    MINI_FILE_READ(&pHeader->m_ID,                 sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_Version,            sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_SkinWidth,          sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_SkinHeight,         sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_FrameSize,          sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_SkinCount,          sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_VertexCount,        sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_TextureCoordCount,  sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_PolygonCount,       sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_GlCmdsCount,        sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_FrameCount,         sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_SkinOffset,         sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_TextureCoordOffset, sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_PolygonOffset,      sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_FrameOffset,        sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_GlCmdsOffset,       sizeof(unsigned int), 1, pFile);
    MINI_FILE_READ(&pHeader->m_EndOffset,          sizeof(unsigned int), 1, pFile);
}

/**
* Reads MD2 skin
*@param pFile - MD2 file
*@param[out] pSkin - MD2 skin structure to populate
*/
void ReadMD2Skin(FILE* pFile, MM_MD2Skin* pSkin)
{
    // read skin name from file
    MINI_FILE_READ(&pSkin->m_Name, sizeof(char), 64, pFile);
}

/**
* Reads MD2 vertex
*@param pFile - MD2 file
*@param[out] pVertex - MD2 vertex structure to populate
*/
void ReadMD2Vertex(FILE* pFile, MM_MD2Vertex* pVertex)
{
    // read vertex from file
    MINI_FILE_READ(&pVertex->m_Vertex,      sizeof(pVertex->m_Vertex), 1, pFile);
    MINI_FILE_READ(&pVertex->m_NormalIndex, sizeof(unsigned char),     1, pFile);
}

/**
* Reads MD2 texture coordinates
*@param pVertex - MD2 file
*@param[out] pTexCoord - MD2 texture coordinates structure to populate
*/
void ReadMD2TextureCoord(FILE* pFile, MM_MD2TextureCoord* pTexCoord)
{
    // read texture coordinates from file
    MINI_FILE_READ(&pTexCoord->m_U, sizeof(unsigned short), 1, pFile);
    MINI_FILE_READ(&pTexCoord->m_V, sizeof(unsigned short), 1, pFile);
}

/**
* Reads MD2 frame
*@param pFile - MD2 file
*@param pHeader - MD2 header
*@param[out] pFrame - MD2 frame structure to populate
*/
void ReadMD2Frame(FILE* pFile, const MM_MD2Header* pHeader, MM_MD2Frame* pFrame)
{
    unsigned int i;

    // read vertex transformations
    MINI_FILE_READ(&pFrame->m_Scale,     sizeof(pFrame->m_Scale),     1, pFile);
    MINI_FILE_READ(&pFrame->m_Translate, sizeof(pFrame->m_Translate), 1, pFile);

    // read frame name
    MINI_FILE_READ(&pFrame->m_Name, sizeof(char), 16, pFile);

    // create md2 vertex list
    pFrame->m_pVertex = (MM_MD2Vertex*)malloc(sizeof(MM_MD2Vertex) * pHeader->m_VertexCount);

    // read frame vertices
    for (i = 0; i < pHeader->m_VertexCount; ++i)
        ReadMD2Vertex(pFile, &pFrame->m_pVertex[i]);
}

/**
* Reads MD2 polygon
*@param pFile - MD2 file
*@param[out] pPolygon - MD2 polygon  structure to populate
*/
void ReadMD2Polygon(FILE* pFile, MM_MD2Polygon* pPolygon)
{
    // read polygon from file
    MINI_FILE_READ(&pPolygon->m_VertexIndex,       sizeof(pPolygon->m_VertexIndex),       1, pFile);
    MINI_FILE_READ(&pPolygon->m_TextureCoordIndex, sizeof(pPolygon->m_TextureCoordIndex), 1, pFile);
}

/**
* Uncompresses vertex
*@param pFrame - MD2 frame
*@param pVertex - MD2 vertex
*@param[out] pResult - resulting vector
*/
void UncompressVertex(const MM_MD2Frame* pFrame, const MM_MD2Vertex* pVertex, MG_Vector3* pResult)
{
    unsigned int i;
    float        vertex[3];

    // iterate through vertex coordinates
    for (i = 0; i < 3; ++i)
        // uncompress vertex using frame scale and translate values
        vertex[i] = (pFrame->m_Scale[i] * pVertex->m_Vertex[i]) + pFrame->m_Translate[i];

    // copy decompressed vertex to result
    pResult->m_X = vertex[0];
    pResult->m_Y = vertex[1];
    pResult->m_Z = vertex[2];
}

/**
* Creates mesh
*@param pHeader - MD2 header
*@param pFrames - MD2 frames
*@param pSkin - MD2 skin
*@param pTexCoord - MD2 texture coordinates
*@param pPolygon - MD2 polygon
*@param pGlCmds - MD2 OpenGL commands
*@param pVertexFormat - vertex format to use
*@param color - color in RGBA format
*@param[out] pMD2Model - MD2 model
*@return 1 on success, otherwise 0
*/
int CreateMesh(MM_MD2Header*       pHeader,
               MM_MD2Frame*        pFrames,
               MM_MD2Skin*         pSkin,
               MM_MD2TextureCoord* pTexCoord,
               MM_MD2Polygon*      pPolygon,
               int*                pGlCmds,
               MV_VertexFormat*    pVertexFormat,
               unsigned            color,
               MM_MD2Model**       pMD2Model)
{
    unsigned int  index;
    unsigned int  meshIndex;
    unsigned int  vertexIndex;
    unsigned int  normalIndex;
    unsigned int  offset;
    int           i;
    int*          pCurGlCmds;
    MM_MD2Frame*  pSrcFrame;
    MM_MD2Vertex* pSrcVertex;
    MM_Frame*     pMdlFrm;
    MM_Mesh*      pMdlMesh;
    MG_Vector3    vertex;

    // create MD2 model
    *pMD2Model                    = (MM_MD2Model*)    malloc(sizeof(MM_MD2Model));
    (*pMD2Model)->m_pVertexFormat = (MV_VertexFormat*)malloc(sizeof(MV_VertexFormat));
    (*pMD2Model)->m_pFrame        = (MM_Frame*)       malloc(sizeof(MM_Frame) * pHeader->m_FrameCount);
    (*pMD2Model)->m_FrameCount    = pHeader->m_FrameCount;

    // calculate stride
    CalculateStride(pVertexFormat);

    // copy vertex format
    *((*pMD2Model)->m_pVertexFormat) = *pVertexFormat;

    const unsigned int vertexLength = sizeof(float) * pVertexFormat->m_Stride;

    // iterate through frames to create
    for (index = 0; index < pHeader->m_FrameCount; ++index)
    {
        // get source frame from which meshes should be extracted
        pSrcFrame = &pFrames[index];

        // get current frame to populate
        pMdlFrm              = &(*pMD2Model)->m_pFrame[index];
        pMdlFrm->m_pMesh     = 0;
        pMdlFrm->m_MeshCount = 0;

        pCurGlCmds =  pGlCmds;
        i          = *pCurGlCmds;
        meshIndex  =  0;

        // iterate through OpenGL commands (negative value is for triangle fan,
        // positive value is for triangle strip, 0 means list end)
        while (i)
        {
            // the first command is the number of vertices to process, already processed,
            // so skip it
            ++pCurGlCmds;

            // set mesh count
            pMdlFrm->m_MeshCount = meshIndex + 1;

            // create new mesh
            if (!pMdlFrm->m_pMesh)
                pMdlFrm->m_pMesh = (MM_Mesh*)malloc(sizeof(MM_Mesh));
            else
                pMdlFrm->m_pMesh = (MM_Mesh*)realloc(pMdlFrm->m_pMesh,
                                                     sizeof(MM_Mesh) * pMdlFrm->m_MeshCount);

            // get current mesh to populate
            pMdlMesh = &pMdlFrm->m_pMesh[meshIndex];

            // populate newly created mesh
            pMdlMesh->m_pVertexBuffer = 0;
            pMdlMesh->m_VertexCount   = 0;

            // determine vertices type to draw
            if (i < 0)
            {
                pMdlMesh->m_IsTriangleStrip = 0;
                i = -i;
            }
            else
                pMdlMesh->m_IsTriangleStrip = 1;

            vertexIndex = 0;

            // iterate through OpenGL commands to process
            for (; i > 0; --i)
            {
                ++pMdlMesh->m_VertexCount;

                // add new vertex
                if (!pMdlMesh->m_pVertexBuffer)
                    pMdlMesh->m_pVertexBuffer = (float*)malloc(vertexLength);
                else
                    pMdlMesh->m_pVertexBuffer = (float*)realloc(pMdlMesh->m_pVertexBuffer,
                                                                pMdlMesh->m_VertexCount * vertexLength);

                // get source vertex
                pSrcVertex = &pSrcFrame->m_pVertex[pCurGlCmds[2]];

                // uncompress vertex
                UncompressVertex(pSrcFrame, pSrcVertex, &vertex);

                offset = vertexIndex;

                // populate vertex
                pMdlMesh->m_pVertexBuffer[offset]     = vertex.m_X;
                pMdlMesh->m_pVertexBuffer[offset + 1] = vertex.m_Y;
                pMdlMesh->m_pVertexBuffer[offset + 2] = vertex.m_Z;

                offset += 3;

                // do include normals?
                 if ((*pMD2Model)->m_pVertexFormat->m_UseNormals)
                {
                    // calculate normal index in table
                    normalIndex = pFrames[index].m_pVertex[pCurGlCmds[2]].m_NormalIndex * 3;

                    // extract normal
                    pMdlMesh->m_pVertexBuffer[offset]     = g_NormalTable[normalIndex];
                    pMdlMesh->m_pVertexBuffer[offset + 1] = g_NormalTable[normalIndex + 1];
                    pMdlMesh->m_pVertexBuffer[offset + 2] = g_NormalTable[normalIndex + 2];

                    offset += 3;
                }

                // do include texture coordinates?
                 if ((*pMD2Model)->m_pVertexFormat->m_UseTextures)
                {
                    // copy texture coordinates from source
                    pMdlMesh->m_pVertexBuffer[offset]     = ((float*)pCurGlCmds)[0];
                    pMdlMesh->m_pVertexBuffer[offset + 1] = ((float*)pCurGlCmds)[1];

                    offset += 2;
                }

                // do use colors?
                if ((*pMD2Model)->m_pVertexFormat->m_UseColors)
                {
                    pMdlMesh->m_pVertexBuffer[offset]     = (float)((color >> 24) & 0xFF) / 255.0f;
                    pMdlMesh->m_pVertexBuffer[offset + 1] = (float)((color >> 16) & 0xFF) / 255.0f;
                    pMdlMesh->m_pVertexBuffer[offset + 2] = (float)((color >> 8)  & 0xFF) / 255.0f;
                    pMdlMesh->m_pVertexBuffer[offset + 3] = (float)( color        & 0xFF) / 255.0f;
                }

                vertexIndex += pVertexFormat->m_Stride;

                // go to next OpenGL command
                pCurGlCmds += 3;
            }

            ++meshIndex;

            // get next OpenGL command
            i = *pCurGlCmds;
        }
    }

    return 1;
}

/**
* Loads MD2 model
*@param pName - MD2 file name
*@param pVertexFormat - vertex format to use
*@param color - color in RGBA format
*@param[out] pMD2Model - MD2 model
*@return 1 on success, otherwise 0
*/
int LoadMD2Model(const unsigned char*   pName,
                       MV_VertexFormat* pVertexFormat,
                       unsigned         color,
                       MM_MD2Model**    pMD2Model)
{
    FILE*               pFile;
    MM_MD2Header*       pHeader;
    MM_MD2Skin*         pSkin;
    MM_MD2TextureCoord* pTexCoord;
    MM_MD2Polygon*      pPolygon;
    int*                pGlCmds;
    MM_MD2Frame*        pFrame;
    unsigned int        i;
    int                 result = 0;

    // open md2 file
    pFile = MINI_FILE_OPEN(pName, "rb");

    // succeeded?
    if (!pFile)
        return 0;

    // create md2 header
    pHeader = (MM_MD2Header*)malloc(sizeof(MM_MD2Header));

    // read file header
    ReadMD2Header(pFile, pHeader);

    // is md2 file and version correct?
    if ((pHeader->m_ID != M_MD2_ID) || ((float)pHeader->m_Version != M_MD2_Mesh_File_Version))
    {
        free(pHeader);
        return 0;
    }

    // create mesh buffers
    pSkin     = (MM_MD2Skin*)        malloc(sizeof(MM_MD2Skin)         * pHeader->m_SkinCount);
    pTexCoord = (MM_MD2TextureCoord*)malloc(sizeof(MM_MD2TextureCoord) * pHeader->m_TextureCoordCount);
    pPolygon  = (MM_MD2Polygon*)     malloc(sizeof(MM_MD2Polygon)      * pHeader->m_PolygonCount);
    pGlCmds   = (int*)               malloc(sizeof(int)                * pHeader->m_GlCmdsCount);
    pFrame    = (MM_MD2Frame*)       malloc(sizeof(MM_MD2Frame)        * pHeader->m_FrameCount);

    // go to skin offset
    MINI_FILE_SEEK(pFile, pHeader->m_SkinOffset, SEEK_SET);

    // read skins
    for (i = 0; i < pHeader->m_SkinCount; ++i)
        ReadMD2Skin(pFile, &pSkin[i]);

    // go to texture coordinates offset
    MINI_FILE_SEEK(pFile, pHeader->m_TextureCoordOffset, SEEK_SET);

    // read texture coordinates
    for (i = 0; i < pHeader->m_TextureCoordCount; ++i)
        ReadMD2TextureCoord(pFile, &pTexCoord[i]);

    // go to polygons offset
    MINI_FILE_SEEK(pFile, pHeader->m_PolygonOffset, SEEK_SET);

    // read polygons
    for (i = 0; i < pHeader->m_PolygonCount; ++i)
        ReadMD2Polygon(pFile, &pPolygon[i]);

    // read OpenGL commands
    MINI_FILE_SEEK(pFile, pHeader->m_GlCmdsOffset, SEEK_SET);
    MINI_FILE_READ(pGlCmds, sizeof(unsigned int), pHeader->m_GlCmdsCount, pFile);

    // go to frames offset
    MINI_FILE_SEEK(pFile, pHeader->m_FrameOffset, SEEK_SET);

    // read frames
    for (i = 0; i < pHeader->m_FrameCount; ++i)
        ReadMD2Frame(pFile, pHeader, &pFrame[i]);

    // close MD2 file
    MINI_FILE_CLOSE(pFile);

    // create mesh from file content
    result = CreateMesh(pHeader,
                        pFrame,
                        pSkin,
                        pTexCoord,
                        pPolygon,
                        pGlCmds,
                        pVertexFormat,
                        color,
                        pMD2Model);

    // delete frame vertices
    for (i = 0; i < pHeader->m_FrameCount; ++i)
        if (pFrame[i].m_pVertex)
            free(pFrame[i].m_pVertex);

    // delete MD2 structures
    free(pHeader);
    free(pSkin);
    free(pTexCoord);
    free(pPolygon);
    free(pGlCmds);
    free(pFrame);

    return result;
}

/**
* Releases MD2 model
*@param pMD2Model - MD2 model to release
*/
void ReleaseMD2Model(MM_MD2Model* pMD2Model)
{
    unsigned int i;
    unsigned int j;

    // no model to delete?
    if (!pMD2Model)
        return;

    // iterate through meshes and delete each mesh
    for (i = 0; i < pMD2Model->m_FrameCount; ++i)
    {
        if (!pMD2Model->m_pFrame[i].m_pMesh)
            continue;

        // iterate through mesh vertex buffers and delete each vertex buffer
        for (j = 0; j < pMD2Model->m_pFrame[i].m_MeshCount; ++j)
            if (pMD2Model->m_pFrame[i].m_pMesh[j].m_pVertexBuffer)
                free(pMD2Model->m_pFrame[i].m_pMesh[j].m_pVertexBuffer);

        // delete meshes, if exist
        free(pMD2Model->m_pFrame[i].m_pMesh);
    }

    // delete frame list, if exists
    if (pMD2Model->m_pFrame)
        free(pMD2Model->m_pFrame);

    // delete vertex format, if exists
    if (pMD2Model->m_pVertexFormat)
        free(pMD2Model->m_pVertexFormat);

    free(pMD2Model);
}

#endif // MiniModelsH
