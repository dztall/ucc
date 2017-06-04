/*****************************************************************************
 * ==> Minimal models library -----------------------------------------------*
 *****************************************************************************
 * Description : Minimal models library                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not                 *
 *****************************************************************************/

#ifndef MiniModelsH
#define MiniModelsH

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <memory.h>

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

/**
* Landscape model
*/
typedef struct
{
    float*       m_pVertexBuffer;
    unsigned int m_VertexCount;
} ML_Mesh;

/**
* Rover model OpenGL commands
*/
typedef struct
{
    int m_GLCmd;    // 0 = TRIANGLE_STRIP, 1 = TRIANGLE_FAN
    int m_CullMode; // 0 = DISABLED, 1 = ENABLED
} MR_MdlCmds;

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
// MD2 functions
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

//-----------------------------------------------------------------------------
// Landscape creation functions
//-----------------------------------------------------------------------------

/**
* Loads data to generate landscape from model image
*@param pFileName - bitmap file name to load from
*@param[out] pData - data to use to generate landscape
*@param[out] width - landscape width, in bytes
*@param[out] height - landscape height, in bytes
*@return 1 on success, otherwise 0
*@note For now only most common bitmaps are loaded, some bitmap types may be unsupported
*/
int LoadLandscape(const unsigned char*  pFileName,
                        unsigned char** pData,
                        int*            pWidth,
                        int*            pHeight)
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

//-----------------------------------------------------------------------------
// Rover creation functions
//-----------------------------------------------------------------------------

/**
* Creates a rover model
*@param[in, out] pVertexFormat - vertex format, generated format on function ends
*@param[out] pVertices - model vertices, should be deleted whenever useless
*@param[out] pVertexCount - vertex count
*@param[out] pMdlCmds - model commands, should be deleted whenever useless
*@param[out] pIndexes - index buffer, should be deleted whenever useless
*@param[out] pIndexCount - index count
*/
void CreateRover(MV_VertexFormat* pVertexFormat,
                 float**          pVertices,
                 unsigned*        pVertexCount,
                 MR_MdlCmds**     pMdlCmds,
                 MV_Index**       pIndexes,
                 unsigned*        pIndexCount)
{
    unsigned offset = 0;

    const float rover_frame_len_x   =  0.1f;
    const float rover_frame_len_y   =  1.5f;
    const float rover_frame_len     =  sqrtf(powf(rover_frame_len_x, 2.0f) + powf(rover_frame_len_y, 2.0f));
    const float rover_frame_angle   =  asinf(rover_frame_len_y / rover_frame_len);
          float rover_frame_angle_1 =  rover_frame_angle                                       + asinf(M_PI_2 / 3.0f);
          float rover_frame_angle_2 =  rover_frame_angle + (M_PI_2 - rover_frame_angle)        + asinf(M_PI_2 / 3.0f);
          float rover_frame_angle_3 =  rover_frame_angle +  M_PI                               + asinf(M_PI_2 / 3.0f);
          float rover_frame_angle_4 =  rover_frame_angle + (M_PI_2 - rover_frame_angle) + M_PI + asinf(M_PI_2 / 3.0f);
          float x                   =  1.25f;
          float y                   = -1.0f;

    const float wheel_angle_1 =  (M_PI * 2.0f)         / 8.0f;
    const float wheel_angle_2 = ((M_PI * 2.0f) * 2.0f) / 8.0f;
    const float wheel_angle_3 = ((M_PI * 2.0f) * 3.0f) / 8.0f;
    const float wheel_angle_4 = ((M_PI * 2.0f) * 4.0f) / 8.0f;
    const float wheel_angle_5 = ((M_PI * 2.0f) * 5.0f) / 8.0f;
    const float wheel_angle_6 = ((M_PI * 2.0f) * 6.0f) / 8.0f;
    const float wheel_angle_7 = ((M_PI * 2.0f) * 7.0f) / 8.0f;
    const float wheel_angle_8 =  (M_PI * 2.0f);
    const float wheel_length  =  0.5f;

    const float roverCockpit_1[] =
    {
        // x    y      z     r      g      b      a
        -2.0f, -1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
        -2.0f,  1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
         2.0f, -1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
         2.0f,  1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
         2.0f,  1.0f,  1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
        -2.0f,  1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
        -2.0f,  1.0f,  1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
        -2.0f, -1.0f,  1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
         2.0f,  1.0f,  1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
         2.0f, -1.0f,  1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
         2.0f, -1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
        -2.0f, -1.0f,  1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
        -2.0f, -1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
        -2.0f,  1.0f, -1.0f, 0.85f, 0.75f, 0.85f, 1.0f,
    };

    const float roverCockpit_2[] =
    {
        // x   y      z     r     g     b     a
        3.5f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        2.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        2.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        2.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        2.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        2.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    };

    const float roverCockpit_3[] =
    {
        // x    y      z     r     g     b     a
        -3.5f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -2.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -2.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -2.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -2.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -2.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    };

    const float roverCockpit_4[] =
    {
        // x    y    z      r      g      b      a
        1.0f, 1.0f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.0f, 1.5f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.5f, 1.0f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.5f, 1.5f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.5f, 1.5f,  0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.0f, 1.5f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.0f, 1.5f,  0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.0f, 1.0f,  0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.5f, 1.5f,  0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.5f, 1.0f,  0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.5f, 1.0f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.0f, 1.0f,  0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.0f, 1.0f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
        1.0f, 1.5f, -0.25f, 0.45f, 0.35f, 0.45f, 1.0f,
    };

    const float roverCockpit_5[] =
    {
        // x   y      z     r     g     b     a
        0.75f, 1.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 2.0f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.75f, 1.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.75f, 2.0f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.75f, 2.0f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 2.0f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 2.0f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 1.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.75f, 2.0f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.75f, 1.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.75f, 1.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 1.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 1.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.75f, 2.0f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    const float roverFrame_fl[] =
    {
        // x                                            y                                               z     r     g     b     a
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    const float roverFrame_fr[] =
    {
        // x                                            y                                                z     r     g     b     a
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x + rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    rover_frame_angle_1 =  rover_frame_angle                                       + asinf(M_PI_2 / 3.0f);
    rover_frame_angle_2 =  rover_frame_angle + (M_PI_2 - rover_frame_angle) + M_PI + asinf(M_PI_2 / 3.0f);
    rover_frame_angle_3 =  rover_frame_angle +  M_PI                               + asinf(M_PI_2 / 3.0f);
    rover_frame_angle_4 =  rover_frame_angle + (M_PI_2 - rover_frame_angle)        + asinf(M_PI_2 / 3.0f);
    x                   = -1.25f;

    const float roverFrame_rl[] =
    {
        // x                                            y                                               z     r     g     b     a
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    const float roverFrame_rr[] =
    {
        // x                                            y                                                z     r     g     b     a
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_1), y + rover_frame_len * sin(rover_frame_angle_1), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_4), y + rover_frame_len * sin(rover_frame_angle_4), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_3), y + rover_frame_len * sin(rover_frame_angle_3), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
        x - rover_frame_len * cos(rover_frame_angle_2), y + rover_frame_len * sin(rover_frame_angle_2), -1.2f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    x = -1.9f;
    y = -2.3f;

    const float roverWheel_fl[] =
    {
        // x                                   y                                      z     r     g     b     a
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
    };

    const float roverWheel_fr[] =
    {
        // x                                   y                                       z     r     g     b     a
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
    };

    x = 1.9f;

    const float roverWheel_rl[] =
    {
        // x                                   y                                      z     r     g     b     a
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), 2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
    };

    const float roverWheel_rr[] =
    {
        // x                                   y                                       z     r     g     b     a
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_2), y + wheel_length * sin(wheel_angle_2), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_3), y + wheel_length * sin(wheel_angle_3), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_4), y + wheel_length * sin(wheel_angle_4), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_5), y + wheel_length * sin(wheel_angle_5), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_6), y + wheel_length * sin(wheel_angle_6), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_7), y + wheel_length * sin(wheel_angle_7), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_8), y + wheel_length * sin(wheel_angle_8), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -1.2f, 0.3f, 0.3f, 0.3f, 1.0f,
        x + wheel_length * cos(wheel_angle_1), y + wheel_length * sin(wheel_angle_1), -2.2f, 0.3f, 0.3f, 0.3f, 1.0f,
    };

    // configure the vertex format, because data are pre-defined
    pVertexFormat->m_UseColors   = 1;
    pVertexFormat->m_UseNormals  = 0;
    pVertexFormat->m_UseTextures = 0;

    CalculateStride(pVertexFormat);

    // create memory for vertices and indexes
    *pVertexCount = 182;
    *pIndexCount  = 13;
    *pVertices    = (float*)malloc(*pVertexCount * pVertexFormat->m_Stride * sizeof(float));
    *pIndexes     = (MV_Index*)malloc(*pIndexCount * sizeof(MV_Index));
    *pMdlCmds     = (MR_MdlCmds*)malloc(*pIndexCount * sizeof(MR_MdlCmds));

    // set indexes and copy vertices
    (*pIndexes)[0].m_Start    = offset;
    (*pIndexes)[0].m_Length   = sizeof(roverCockpit_1) / sizeof(float);
    (*pMdlCmds)[0].m_GLCmd    = 0;
    (*pMdlCmds)[0].m_CullMode = 1;
    memcpy(*pVertices, roverCockpit_1, sizeof(roverCockpit_1));
    offset += sizeof(roverCockpit_1) / sizeof(float);

    (*pIndexes)[1].m_Start    = offset;
    (*pIndexes)[1].m_Length   = sizeof(roverCockpit_2) / sizeof(float);
    (*pMdlCmds)[1].m_GLCmd    = 1;
    (*pMdlCmds)[1].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverCockpit_2, sizeof(roverCockpit_2));
    offset += sizeof(roverCockpit_2) / sizeof(float);

    (*pIndexes)[2].m_Start    = offset;
    (*pIndexes)[2].m_Length   = sizeof(roverCockpit_3) / sizeof(float);
    (*pMdlCmds)[2].m_GLCmd    = 1;
    (*pMdlCmds)[2].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverCockpit_3, sizeof(roverCockpit_3));
    offset += sizeof(roverCockpit_3) / sizeof(float);

    (*pIndexes)[3].m_Start    = offset;
    (*pIndexes)[3].m_Length   = sizeof(roverCockpit_4) / sizeof(float);
    (*pMdlCmds)[3].m_GLCmd    = 0;
    (*pMdlCmds)[3].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverCockpit_4, sizeof(roverCockpit_4));
    offset += sizeof(roverCockpit_4) / sizeof(float);

    (*pIndexes)[4].m_Start    = offset;
    (*pIndexes)[4].m_Length   = sizeof(roverCockpit_5) / sizeof(float);
    (*pMdlCmds)[4].m_GLCmd    = 0;
    (*pMdlCmds)[4].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverCockpit_5, sizeof(roverCockpit_5));
    offset += sizeof(roverCockpit_5) / sizeof(float);

    (*pIndexes)[5].m_Start    = offset;
    (*pIndexes)[5].m_Length   = sizeof(roverFrame_fl) / sizeof(float);
    (*pMdlCmds)[5].m_GLCmd    = 0;
    (*pMdlCmds)[5].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverFrame_fl, sizeof(roverFrame_fl));
    offset += sizeof(roverFrame_fl) / sizeof(float);

    (*pIndexes)[6].m_Start    = offset;
    (*pIndexes)[6].m_Length   = sizeof(roverFrame_fr) / sizeof(float);
    (*pMdlCmds)[6].m_GLCmd    = 0;
    (*pMdlCmds)[6].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverFrame_fr, sizeof(roverFrame_fr));
    offset += sizeof(roverFrame_fr) / sizeof(float);

    (*pIndexes)[7].m_Start    = offset;
    (*pIndexes)[7].m_Length   = sizeof(roverFrame_rl) / sizeof(float);
    (*pMdlCmds)[7].m_GLCmd    = 0;
    (*pMdlCmds)[7].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverFrame_rl, sizeof(roverFrame_rl));
    offset += sizeof(roverFrame_rl) / sizeof(float);

    (*pIndexes)[8].m_Start    = offset;
    (*pIndexes)[8].m_Length   = sizeof(roverFrame_rr) / sizeof(float);
    (*pMdlCmds)[8].m_GLCmd    = 0;
    (*pMdlCmds)[8].m_CullMode = 1;
    memcpy((*pVertices) + offset, roverFrame_rr, sizeof(roverFrame_rr));
    offset += sizeof(roverFrame_rr) / sizeof(float);

    (*pIndexes)[9].m_Start    = offset;
    (*pIndexes)[9].m_Length   = sizeof(roverWheel_fl) / sizeof(float);
    (*pMdlCmds)[9].m_GLCmd    = 0;
    (*pMdlCmds)[9].m_CullMode = 0;
    memcpy((*pVertices) + offset, roverWheel_fl, sizeof(roverWheel_fl));
    offset += sizeof(roverWheel_fl) / sizeof(float);

    (*pIndexes)[10].m_Start    = offset;
    (*pIndexes)[10].m_Length   = sizeof(roverWheel_fr) / sizeof(float);
    (*pMdlCmds)[10].m_GLCmd    = 0;
    (*pMdlCmds)[10].m_CullMode = 0;
    memcpy((*pVertices) + offset, roverWheel_fr, sizeof(roverWheel_fr));
    offset += sizeof(roverWheel_fr) / sizeof(float);

    (*pIndexes)[11].m_Start    = offset;
    (*pIndexes)[11].m_Length   = sizeof(roverWheel_rl) / sizeof(float);
    (*pMdlCmds)[11].m_GLCmd    = 0;
    (*pMdlCmds)[11].m_CullMode = 0;
    memcpy((*pVertices) + offset, roverWheel_rl, sizeof(roverWheel_rl));
    offset += sizeof(roverWheel_rl) / sizeof(float);

    (*pIndexes)[12].m_Start    = offset;
    (*pIndexes)[12].m_Length   = sizeof(roverWheel_rr) / sizeof(float);
    (*pMdlCmds)[12].m_GLCmd    = 0;
    (*pMdlCmds)[12].m_CullMode = 0;
    memcpy((*pVertices) + offset, roverWheel_rr, sizeof(roverWheel_rr));
    offset += sizeof(roverWheel_rr) / sizeof(float);
}

#endif // MiniModelsH
