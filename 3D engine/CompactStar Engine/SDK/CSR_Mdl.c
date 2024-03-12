/****************************************************************************
 * ==> CSR_Mdl -------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a Quake 1 model (.mdl) reader         *
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
#include "CSR_Mdl.h"

// std
#include <stdlib.h>
#include <string.h>

// visual studio specific code
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <math.h>
#else
    #include <math.h>
#endif

// this code is EXPERIMENTAL and should be STRONGLY TESTED on big endian machines before be activated
#define CONVERT_ENDIANNESS

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_MDL_Mesh_File_Version 6
#define M_MDL_ID                (('O' << 24) + ('P' << 16) + ('D' << 8) + 'I')
//---------------------------------------------------------------------------
// Global values
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
unsigned char g_ColorTable[] =
{
    0,   0,   0,   15,  15,  15,  31,  31,  31,
    47,  47,  47,  63,  63,  63,  75,  75,  75,
    91,  91,  91,  107, 107, 107, 123, 123, 123,
    139, 139, 139, 155, 155, 155, 171, 171, 171,
    187, 187, 187, 203, 203, 203, 219, 219, 219,
    235, 235, 235, 15,  11,  7,   23,  15,  11,
    31,  23,  11,  39,  27,  15,  47,  35,  19,
    55,  43,  23,  63,  47,  23,  75,  55,  27,
    83,  59,  27,  91,  67,  31,  99,  75,  31,
    107, 83,  31,  115, 87,  31,  123, 95,  35,
    131, 103, 35,  143, 111, 35,  11,  11,  15,
    19,  19,  27,  27,  27,  39,  39,  39,  51,
    47,  47,  63,  55,  55,  75,  63,  63,  87,
    71,  71,  103, 79,  79,  115, 91,  91,  127,
    99,  99,  139, 107, 107, 151, 115, 115, 163,
    123, 123, 175, 131, 131, 187, 139, 139, 203,
    0,   0,   0,   7,   7,   0,   11,  11,  0,
    19,  19,  0,   27,  27,  0,   35,  35,  0,
    43,  43,  7,   47,  47,  7,   55,  55,  7,
    63,  63,  7,   71,  71,  7,   75,  75,  11,
    83,  83,  11,  91,  91,  11,  99,  99,  11,
    107, 107, 15,  7,   0,   0,   15,  0,   0,
    23,  0,   0,   31,  0,   0,   39,  0,   0,
    47,  0,   0,   55,  0,   0,   63,  0,   0,
    71,  0,   0,   79,  0,   0,   87,  0,   0,
    95,  0,   0,   103, 0,   0,   111, 0,   0,
    119, 0,   0,   127, 0,   0,   19,  19,  0,
    27,  27,  0,   35,  35,  0,   47,  43,  0,
    55,  47,  0,   67,  55,  0,   75,  59,  7,
    87,  67,  7,   95,  71,  7,   107, 75,  11,
    119, 83,  15,  131, 87,  19,  139, 91,  19,
    151, 95,  27,  163, 99,  31,  175, 103, 35,
    35,  19,  7,   47,  23,  11,  59,  31,  15,
    75,  35,  19,  87,  43,  23,  99,  47,  31,
    115, 55,  35,  127, 59,  43,  143, 67,  51,
    159, 79,  51,  175, 99,  47,  191, 119, 47,
    207, 143, 43,  223, 171, 39,  239, 203, 31,
    255, 243, 27,  11,  7,   0,   27,  19,  0,
    43,  35,  15,  55,  43,  19,  71,  51,  27,
    83,  55,  35,  99,  63,  43,  111, 71,  51,
    127, 83,  63,  139, 95,  71,  155, 107, 83,
    167, 123, 95,  183, 135, 107, 195, 147, 123,
    211, 163, 139, 227, 179, 151, 171, 139, 163,
    159, 127, 151, 147, 115, 135, 139, 103, 123,
    127, 91,  111, 119, 83,  99,  107, 75,  87,
    95,  63,  75,  87,  55,  67,  75,  47,  55,
    67,  39,  47,  55,  31,  35,  43,  23,  27,
    35,  19,  19,  23,  11,  11,  15,  7,   7,
    187, 115, 159, 175, 107, 143, 163, 95,  131,
    151, 87,  119, 139, 79,  107, 127, 75,  95,
    115, 67,  83,  107, 59,  75,  95,  51,  63,
    83,  43,  55,  71,  35,  43,  59,  31,  35,
    47,  23,  27,  35,  19,  19,  23,  11,  11,
    15,  7,   7,   219, 195, 187, 203, 179, 167,
    191, 163, 155, 175, 151, 139, 163, 135, 123,
    151, 123, 111, 135, 111, 95,  123, 99,  83,
    107, 87,  71,  95,  75,  59,  83,  63,  51,
    67,  51,  39,  55,  43,  31,  39,  31,  23,
    27,  19,  15,  15,  11,  7,   111, 131, 123,
    103, 123, 111, 95,  115, 103, 87,  107, 95,
    79,  99,  87,  71,  91,  79,  63,  83,  71,
    55,  75,  63,  47,  67,  55,  43,  59,  47,
    35,  51,  39,  31,  43,  31,  23,  35,  23,
    15,  27,  19,  11,  19,  11,  7,   11,  7,
    255, 243, 27,  239, 223, 23,  219, 203, 19,
    203, 183, 15,  187, 167, 15,  171, 151, 11,
    155, 131, 7,   139, 115, 7,   123, 99,  7,
    107, 83,  0,   91,  71,  0,   75,  55,  0,
    59,  43,  0,   43,  31,  0,   27,  15,  0,
    11,  7,   0,   0,   0,   255, 11,  11,  239,
    19,  19,  223, 27,  27,  207, 35,  35,  191,
    43,  43,  175, 47,  47,  159, 47,  47,  143,
    47,  47,  127, 47,  47,  111, 47,  47,  95,
    43,  43,  79,  35,  35,  63,  27,  27,  47,
    19,  19,  31,  11,  11,  15,  43,  0,   0,
    59,  0,   0,   75,  7,   0,   95,  7,   0,
    111, 15,  0,   127, 23,  7,   147, 31,  7,
    163, 39,  11,  183, 51,  15,  195, 75,  27,
    207, 99,  43,  219, 127, 59,  227, 151, 79,
    231, 171, 95,  239, 191, 119, 247, 211, 139,
    167, 123, 59,  183, 155, 55,  199, 195, 55,
    231, 227, 87,  127, 191, 255, 171, 231, 255,
    215, 255, 255, 103, 0,   0,   139, 0,   0,
    179, 0,   0,   215, 0,   0,   255, 0,   0,
    255, 243, 147, 255, 247, 199, 255, 255, 255,
    159, 91,  83
};

//---------------------------------------------------------------------------
// Private structures
//---------------------------------------------------------------------------

/**
* MDL header
*/
typedef struct
{
    unsigned m_ID;
    unsigned m_Version;
    float    m_Scale[3];
    float    m_Translate[3];
    float    m_BoundingRadius;
    float    m_EyePosition[3];
    unsigned m_SkinCount;
    unsigned m_SkinWidth;
    unsigned m_SkinHeight;
    unsigned m_VertexCount;
    unsigned m_PolygonCount;
    unsigned m_FrameCount;
    unsigned m_SyncType;
    unsigned m_Flags;
    float    m_Size;
} CSR_MDLHeader;

/**
* MDL skin
*/
typedef struct
{
    unsigned       m_Group;
    unsigned       m_Count;
    unsigned       m_TexLen;
    float*         m_pTime;
    unsigned char* m_pData;
} CSR_MDLSkin;

/**
* MDL texture coordinate
*/
typedef struct
{
    unsigned m_OnSeam;
    unsigned m_U;
    unsigned m_V;
} CSR_MDLTextureCoord;

/**
* MDL polygon
*/
typedef struct
{
    unsigned m_FacesFront;
    unsigned m_VertexIndex[3];
} CSR_MDLPolygon;

/**
* MDL vertex
*/
typedef struct
{
    unsigned char m_Vertex[3];
    unsigned char m_NormalIndex;
} CSR_MDLVertex;

/**
* MDL frame
*/
typedef struct
{
    CSR_MDLVertex  m_BoundingBoxMin;
    CSR_MDLVertex  m_BoundingBoxMax;
    char           m_Name[16];
    CSR_MDLVertex* m_pVertex;
} CSR_MDLFrame;

/**
* MDL frame group
*/
typedef struct
{
    unsigned char m_Type;
    unsigned char m_Count;
    CSR_MDLVertex m_BoundingBoxMin;
    CSR_MDLVertex m_BoundingBoxMax;
    float*        m_pTime;
    CSR_MDLFrame* m_pFrame;
} CSR_MDLFrameGroup;

//---------------------------------------------------------------------------
// MDL model private functions
//---------------------------------------------------------------------------
int csrMDLReadHeader(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_MDLHeader* pHeader)
{
    int success = 1;

    // read header from buffer
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_ID);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_Version);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(pHeader->m_Scale),       1, &pHeader->m_Scale);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(pHeader->m_Translate),   1, &pHeader->m_Translate);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(float),                  1, &pHeader->m_BoundingRadius);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(pHeader->m_EyePosition), 1, &pHeader->m_EyePosition);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_SkinCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_SkinWidth);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_SkinHeight);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_VertexCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_PolygonCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_FrameCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_SyncType);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),               1, &pHeader->m_Flags);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(float),                  1, &pHeader->m_Size);

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (success && csrMemoryEndianness() == CSR_E_BigEndian)
        {
            // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pHeader->m_ID,             sizeof(unsigned));
            csrMemorySwap(&pHeader->m_Version,        sizeof(unsigned));
            csrMemorySwap(&pHeader->m_Scale[0],       sizeof(float));
            csrMemorySwap(&pHeader->m_Scale[1],       sizeof(float));
            csrMemorySwap(&pHeader->m_Scale[2],       sizeof(float));
            csrMemorySwap(&pHeader->m_Translate[0],   sizeof(float));
            csrMemorySwap(&pHeader->m_Translate[1],   sizeof(float));
            csrMemorySwap(&pHeader->m_Translate[2],   sizeof(float));
            csrMemorySwap(&pHeader->m_BoundingRadius, sizeof(float));
            csrMemorySwap(&pHeader->m_EyePosition[0], sizeof(float));
            csrMemorySwap(&pHeader->m_EyePosition[1], sizeof(float));
            csrMemorySwap(&pHeader->m_EyePosition[2], sizeof(float));
            csrMemorySwap(&pHeader->m_SkinCount,      sizeof(unsigned));
            csrMemorySwap(&pHeader->m_SkinWidth,      sizeof(unsigned));
            csrMemorySwap(&pHeader->m_SkinHeight,     sizeof(unsigned));
            csrMemorySwap(&pHeader->m_VertexCount,    sizeof(unsigned));
            csrMemorySwap(&pHeader->m_PolygonCount,   sizeof(unsigned));
            csrMemorySwap(&pHeader->m_FrameCount,     sizeof(unsigned));
            csrMemorySwap(&pHeader->m_SyncType,       sizeof(unsigned));
            csrMemorySwap(&pHeader->m_Flags,          sizeof(unsigned));
            csrMemorySwap(&pHeader->m_Size,           sizeof(float));
        }
    #endif

    return success;
}
//---------------------------------------------------------------------------
int csrMDLReadSkin(const CSR_Buffer*    pBuffer,
                         size_t*        pOffset,
                   const CSR_MDLHeader* pHeader,
                         CSR_MDLSkin*   pSkin)
{
    size_t i;

    // calculate texture size
    pSkin->m_TexLen = pHeader->m_SkinWidth * pHeader->m_SkinHeight;

    // read the skin group flag
    if (!csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pSkin->m_Group))
        return 0;

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (csrMemoryEndianness() == CSR_E_BigEndian)
            // swap the read value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pSkin->m_Group, sizeof(unsigned));
    #endif

    pSkin->m_pTime = 0;

    // is a group of textures?
    if (!pSkin->m_Group)
    {
        pSkin->m_Count = 1;

        // create memory for texture
        pSkin->m_pData = (unsigned char*)malloc(pSkin->m_TexLen);

        // read texture from buffer. NOTE 8 bit array, same in all endianness
        return csrBufferRead(pBuffer, pOffset, pSkin->m_TexLen, 1, pSkin->m_pData);
    }

    // read the skin count
    csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pSkin->m_Count);

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (csrMemoryEndianness() == CSR_E_BigEndian)
            // swap the read value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pSkin->m_Count, sizeof(unsigned));
    #endif

    // no skin to read?
    if (!pSkin->m_Count)
        return 1;

    // create memory for time table
    pSkin->m_pTime = (float*)malloc(pSkin->m_Count * sizeof(float));

    // read time table from buffer
    if (!csrBufferRead(pBuffer, pOffset, sizeof(float), pSkin->m_Count, pSkin->m_pTime))
        return 0;

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (csrMemoryEndianness() == CSR_E_BigEndian)
            // iterate through time values to swap
            for (i = 0; i < pSkin->m_Count; ++i)
                // swap the value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                csrMemorySwap(&pSkin->m_pTime[i], sizeof(float));
    #endif

    // create memory for texture
    pSkin->m_pData = (unsigned char*)malloc((size_t)(pSkin->m_TexLen * pSkin->m_Count));

    // read texture from buffer. NOTE 8 bit array, same in all endianness
    return csrBufferRead(pBuffer, pOffset, pSkin->m_TexLen, pSkin->m_Count, pSkin->m_pData);
}
//---------------------------------------------------------------------------
int csrMDLReadTextureCoord(const CSR_Buffer*          pBuffer,
                                 size_t*              pOffset,
                                 CSR_MDLTextureCoord* pTexCoord)
{
    int success = 1;

    // read texture coordinates from buffer
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pTexCoord->m_OnSeam);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pTexCoord->m_U);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pTexCoord->m_V);

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (success && csrMemoryEndianness() == CSR_E_BigEndian)
        {
            // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pTexCoord->m_OnSeam, sizeof(unsigned));
            csrMemorySwap(&pTexCoord->m_U,      sizeof(unsigned));
            csrMemorySwap(&pTexCoord->m_V,      sizeof(unsigned));
        }
    #endif

    return success;
}
//---------------------------------------------------------------------------
int csrMDLReadPolygon(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_MDLPolygon* pPolygon)
{
    int success = 1;

    // read polygon from buffer
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),                1, &pPolygon->m_FacesFront);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(pPolygon->m_VertexIndex), 1, &pPolygon->m_VertexIndex);

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (success && csrMemoryEndianness() == CSR_E_BigEndian)
        {
            // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pPolygon->m_FacesFront,     sizeof(unsigned));
            csrMemorySwap(&pPolygon->m_VertexIndex[0], sizeof(unsigned));
            csrMemorySwap(&pPolygon->m_VertexIndex[1], sizeof(unsigned));
            csrMemorySwap(&pPolygon->m_VertexIndex[2], sizeof(unsigned));
        }
    #endif

    return success;
}
//---------------------------------------------------------------------------
int csrMDLReadVertex(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_MDLVertex* pVertex)
{
    int success = 1;

    // read vertex from buffer. NOTE 8 bit values, same in all endianness
    success &= csrBufferRead(pBuffer, pOffset, sizeof(pVertex->m_Vertex), 1, &pVertex->m_Vertex);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned char),     1, &pVertex->m_NormalIndex);

    return success;
}
//---------------------------------------------------------------------------
int csrMDLReadFrame(const CSR_Buffer*    pBuffer,
                          size_t*        pOffset,
                    const CSR_MDLHeader* pHeader,
                          CSR_MDLFrame*  pFrame)
{
    unsigned i;
    int      success = 1;

    // read frame bounding box
    success &= csrMDLReadVertex(pBuffer, pOffset, &pFrame->m_BoundingBoxMin);
    success &= csrMDLReadVertex(pBuffer, pOffset, &pFrame->m_BoundingBoxMax);

    // succeeded?
    if (!success)
        return 0;

    // read frame name. NOTE 8 bit array, same in all endianness
    if (!csrBufferRead(pBuffer, pOffset, sizeof(char), 16, &pFrame->m_Name))
        return 0;

    // create frame vertex buffer
    pFrame->m_pVertex = (CSR_MDLVertex*)malloc(sizeof(CSR_MDLVertex) * pHeader->m_VertexCount);

    // read frame vertices
    for (i = 0; i < pHeader->m_VertexCount; ++i)
        if (!csrMDLReadVertex(pBuffer, pOffset, &pFrame->m_pVertex[i]))
            return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrMDLReadFrameGroup(const CSR_Buffer*        pBuffer,
                               size_t*            pOffset,
                         const CSR_MDLHeader*     pHeader,
                               CSR_MDLFrameGroup* pFrameGroup)
{
    int i;

    // read the group type
    if (!csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pFrameGroup->m_Type))
        return 0;

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (csrMemoryEndianness() == CSR_E_BigEndian)
            // swap the read value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pFrameGroup->m_Type, sizeof(unsigned));
    #endif

    // is a single frame or a group of frames?
    if (!pFrameGroup->m_Type)
    {
        pFrameGroup->m_Count = 1;

        // create frame and time buffers
        pFrameGroup->m_pFrame = (CSR_MDLFrame*)malloc(sizeof(CSR_MDLFrame) * pFrameGroup->m_Count);
        pFrameGroup->m_pTime  = (float*)malloc(sizeof(float) * pFrameGroup->m_Count);

        // succeeded?
        if (!pFrameGroup->m_pFrame || !pFrameGroup->m_pTime)
            return 0;

        // read the frame
        if (!csrMDLReadFrame(pBuffer, pOffset, pHeader, pFrameGroup->m_pFrame))
            return 0;

        // for 1 frame there is no time
        pFrameGroup->m_pTime[0] = 0.0f;

        // get the group bounding box (for 1 frame, the group has the same box as the frame)
        pFrameGroup->m_BoundingBoxMin = pFrameGroup->m_pFrame[0].m_BoundingBoxMin;
        pFrameGroup->m_BoundingBoxMax = pFrameGroup->m_pFrame[0].m_BoundingBoxMax;

        return 1;
    }

    // frame group count from buffer
    if (!csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pFrameGroup->m_Count))
        return 0;

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (csrMemoryEndianness() == CSR_E_BigEndian)
            // swap the value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pFrameGroup->m_Count, sizeof(unsigned));
    #endif

    // read the group bounding box min frame
    if (!csrMDLReadVertex(pBuffer, pOffset, &pFrameGroup->m_BoundingBoxMin))
        return 0;

    // read the group bounding box max frame
    if (!csrMDLReadVertex(pBuffer, pOffset, &pFrameGroup->m_BoundingBoxMax))
        return 0;

    // create frame and time buffers
    pFrameGroup->m_pFrame = (CSR_MDLFrame*)malloc(sizeof(CSR_MDLFrame) * pFrameGroup->m_Count);
    pFrameGroup->m_pTime  = (float*)malloc(sizeof(float) * pFrameGroup->m_Count);

    // read the time table from buffer
    if (!csrBufferRead(pBuffer, pOffset, sizeof(float), pFrameGroup->m_Count, pFrameGroup->m_pTime))
        return 0;

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (csrMemoryEndianness() == CSR_E_BigEndian)
            // iterate through time values to swap
            for (i = 0; i < pFrameGroup->m_Count; ++i)
                // swap the value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                csrMemorySwap(&pFrameGroup->m_pTime[i], sizeof(float));
    #endif

    // read the frames
    for (i = 0; i < pFrameGroup->m_Count; ++i)
        if (!csrMDLReadFrame(pBuffer, pOffset, pHeader, &pFrameGroup->m_pFrame[i]))
            return 0;

    return 1;
}
//---------------------------------------------------------------------------
CSR_PixelBuffer* csrMDLUncompressTexture(const CSR_MDLSkin* pSkin,
                                         const CSR_Buffer*  pPalette,
                                               size_t       width,
                                               size_t       height,
                                               size_t       index)
{
    size_t           offset;
    size_t           i;
    CSR_PixelBuffer* pPB;
    unsigned char*   pTexPal;
    unsigned         bpp = 3;

    if (!pSkin)
        return 0;

    if (!pSkin->m_TexLen)
        return 0;

    // create a new pixel buffer
    pPB = csrPixelBufferCreate();

    // succeeded?
    if (!pPB)
        return 0;

    // populate the pixel buffer and calculate the start offset
    pPB->m_ImageType    = CSR_IT_Raw;
    pPB->m_PixelType    = CSR_PT_RGB;
    pPB->m_Width        = (unsigned)width;
    pPB->m_Height       = (unsigned)height;
    pPB->m_BytePerPixel = bpp;
    pPB->m_Stride       = (unsigned)(width * pPB->m_BytePerPixel);
    pPB->m_DataLength   = sizeof(unsigned char) * pSkin->m_TexLen * 3;
    offset              = pSkin->m_TexLen * index;

    // allocate memory for the pixels
    pPB->m_pData = (unsigned char*)malloc(pPB->m_DataLength);

    // do use the default palette?
    if (!pPalette || pPalette->m_Length != sizeof(g_ColorTable))
        pTexPal = g_ColorTable;
    else
        pTexPal = pPalette->m_pData;

    // convert indexed 8 bits texture to RGB 24 bits
    for (i = 0; i < pSkin->m_TexLen; ++i)
    {
        ((unsigned char*)pPB->m_pData)[(i * bpp)]     = pTexPal[(pSkin->m_pData[offset + i] * bpp)];
        ((unsigned char*)pPB->m_pData)[(i * bpp) + 1] = pTexPal[(pSkin->m_pData[offset + i] * bpp) + 1];
        ((unsigned char*)pPB->m_pData)[(i * bpp) + 2] = pTexPal[(pSkin->m_pData[offset + i] * bpp) + 2];
    }

    return pPB;
}
//---------------------------------------------------------------------------
void csrMDLUncompressVertex(const CSR_MDLHeader* pHeader,
                            const CSR_MDLVertex* pVertex,
                            CSR_Vector3* pResult)
{
    #ifdef _MSC_VER
        unsigned i;
        float    vertex[3] = {0};
    #else
        unsigned i;
        float    vertex[3];
    #endif

    // iterate through vertex coordinates
    for (i = 0; i < 3; ++i)
        // uncompress vertex using frame scale and translate values
        vertex[i] = (pHeader->m_Scale[i] * pVertex->m_Vertex[i]) + pHeader->m_Translate[i];

    // copy decompressed vertex to result
    pResult->m_X = vertex[0];
    pResult->m_Y = vertex[1];
    pResult->m_Z = vertex[2];
}
//---------------------------------------------------------------------------
void csrMDLPopulateModel(const CSR_MDLHeader*        pHeader,
                         const CSR_MDLFrameGroup*    pFrameGroup,
                         const CSR_MDLPolygon*       pPolygon,
                         const CSR_MDLTextureCoord*  pTexCoord,
                         const CSR_VertexFormat*     pVertFormat,
                         const CSR_VertexCulling*    pVertCulling,
                         const CSR_Material*         pMaterial,
                         const CSR_fOnGetVertexColor fOnGetVertexColor,
                               CSR_Model*            pModel)
{
    #ifdef _MSC_VER
        int            i;
        size_t         j;
        size_t         k;
        CSR_Vector3    vertex        = {0};
        CSR_Vector3    normal        = {0};
        CSR_Vector2    uv            = {0};
        CSR_MDLVertex* pSrcVertex;
        double         lastKnownTime = 0.0;
    #else
        int            i;
        size_t         j;
        size_t         k;
        CSR_Vector3    vertex;
        CSR_Vector3    normal;
        CSR_Vector2    uv;
        CSR_MDLVertex* pSrcVertex;
        double         lastKnownTime = 0.0;
    #endif

    // any MDL source is missing?
    if (!pHeader || !pFrameGroup || !pPolygon || !pTexCoord)
        return;

    // model contains no frame?
    if (!pHeader->m_FrameCount)
        return;

    // no frame group?
    if (!pFrameGroup->m_Count)
        return;

    // no model to populate?
    if (!pModel)
        return;

    // initialize the model and create all the meshes required to contain the MDL group frames
    pModel->m_MeshCount = pFrameGroup->m_Count;
    pModel->m_pMesh     = (CSR_Mesh*)malloc(pFrameGroup->m_Count * sizeof(CSR_Mesh));
    pModel->m_Time      = 0.0;

    // succeeded?
    if (!pModel->m_pMesh)
        return;

    // iterate through sub-frames contained in group
    for (i = 0; i < pFrameGroup->m_Count; ++i)
    {
        // create the vertex buffers required for the sub-frames
        pModel->m_pMesh[i].m_Count = 1;
        pModel->m_pMesh[i].m_pVB   = (CSR_VertexBuffer*)malloc(sizeof(CSR_VertexBuffer));

        // prepare the next vertex buffer format
        csrVertexBufferInit(pModel->m_pMesh[i].m_pVB);

        // apply the user wished vertex format
        if (pVertFormat)
            pModel->m_pMesh[i].m_pVB->m_Format = *pVertFormat;
        else
        {
            // otherwise configure the default vertex format
            pModel->m_pMesh[i].m_pVB->m_Format.m_HasNormal    = 1;
            pModel->m_pMesh[i].m_pVB->m_Format.m_HasTexCoords = 1;
        }

        // apply the user wished vertex culling
        if (pVertCulling)
            pModel->m_pMesh[i].m_pVB->m_Culling = *pVertCulling;
        else
            // otherwise configure the default culling
            pModel->m_pMesh[i].m_pVB->m_Culling.m_Face = CSR_CF_CW;

        // apply the user wished material
        if (pMaterial)
            pModel->m_pMesh[i].m_pVB->m_Material = *pMaterial;

        // set the vertex format type
        pModel->m_pMesh[i].m_pVB->m_Format.m_Type = CSR_VT_Triangles;

        // calculate the vertex stride
        csrVertexFormatCalculateStride(&pModel->m_pMesh[i].m_pVB->m_Format);

        // configure the model texture
        csrTextureInit(&pModel->m_pMesh[i].m_Skin.m_Texture);
        csrTextureInit(&pModel->m_pMesh[i].m_Skin.m_BumpMap);
        csrTextureInit(&pModel->m_pMesh[i].m_Skin.m_CubeMap);

        // configure the frame time
        if (pFrameGroup->m_pTime)
        {
            pModel->m_pMesh[i].m_Time = pFrameGroup->m_pTime[i] - lastKnownTime;
            lastKnownTime = pFrameGroup->m_pTime[i];
        }
        else
            pModel->m_pMesh[i].m_Time = 0.0;

        // iterate through polygons to process
        for (j = 0; j < pHeader->m_PolygonCount; ++j)
            // iterate through polygon vertices
            for (k = 0; k < 3; ++k)
            {
                // get source vertex
                pSrcVertex = &pFrameGroup->m_pFrame[i].m_pVertex[pPolygon[j].m_VertexIndex[k]];

                // uncompress vertex
                csrMDLUncompressVertex(pHeader, pSrcVertex, &vertex);

                // get normal
                normal.m_X = g_NormalTable[pSrcVertex->m_NormalIndex];
                normal.m_Y = g_NormalTable[pSrcVertex->m_NormalIndex + 1];
                normal.m_Z = g_NormalTable[pSrcVertex->m_NormalIndex + 2];

                // get vertex texture coordinates
                uv.m_X = (float)pTexCoord[pPolygon[j].m_VertexIndex[k]].m_U;
                uv.m_Y = (float)pTexCoord[pPolygon[j].m_VertexIndex[k]].m_V;

                // is texture coordinate on the back face?
                if (!pPolygon[j].m_FacesFront && pTexCoord[pPolygon[j].m_VertexIndex[k]].m_OnSeam)
                    // correct the texture coordinate to put it on the back face
                    uv.m_X += pHeader->m_SkinWidth * 0.5f;

                // scale s and t to range from 0.0 to 1.0
                uv.m_X = (uv.m_X + 0.5f) / pHeader->m_SkinWidth;
                uv.m_Y = (uv.m_Y + 0.5f) / pHeader->m_SkinHeight;

                // add vertex to frame buffer
                if (!csrVertexBufferAdd(&vertex,
                                        &normal,
                                        &uv,
                                        (j * 3) + k,
                                        fOnGetVertexColor,
                                        pModel->m_pMesh[i].m_pVB))
                    return;
            }
    }
}
//---------------------------------------------------------------------------
void csrMDLReleaseObjects(CSR_MDLHeader*       pHeader,
                          CSR_MDLFrameGroup*   pFrameGroup,
                          CSR_MDLSkin*         pSkin,
                          CSR_MDLTextureCoord* pTexCoord,
                          CSR_MDLPolygon*      pPolygon)
{
    size_t i;
    int    j;

    // release frame group content
    if (pHeader && pFrameGroup)
        // iterate through frame groups for which the content should be released
        for (i = 0; i < pHeader->m_FrameCount; ++i)
        {
            // frame group contains frame to release?
            if (pFrameGroup[i].m_pFrame)
            {
                // release frame vertices
                for (j = 0; j < pFrameGroup[i].m_Count; ++j)
                    free(pFrameGroup[i].m_pFrame[j].m_pVertex);

                // release frame
                free(pFrameGroup[i].m_pFrame);
            }

            // release time table
            if (pFrameGroup[i].m_pTime)
                free(pFrameGroup[i].m_pTime);
        }

    // release skin content
    if (pSkin)
    {
        // delete skin time table
        if (pSkin->m_pTime)
            free(pSkin->m_pTime);

        // delete skin data
        if (pSkin->m_pData)
            free(pSkin->m_pData);
    }

    // delete MDL structures
    free(pHeader);
    free(pSkin);
    free(pTexCoord);
    free(pPolygon);
    free(pFrameGroup);
}
//---------------------------------------------------------------------------
// MDL model functions
//---------------------------------------------------------------------------
CSR_MDL* csrMDLCreate(const CSR_Buffer*           pBuffer,
                      const CSR_Buffer*           pPalette,
                      const CSR_VertexFormat*     pVertFormat,
                      const CSR_VertexCulling*    pVertCulling,
                      const CSR_Material*         pMaterial,
                      const CSR_fOnGetVertexColor fOnGetVertexColor,
                      const CSR_fOnApplySkin      fOnApplySkin,
                      const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    CSR_MDLHeader*       pHeader;
    CSR_MDLSkin*         pSkin;
    CSR_MDLTextureCoord* pTexCoord;
    CSR_MDLPolygon*      pPolygon;
    CSR_MDLFrameGroup*   pFrameGroup;
    CSR_MDL*             pMDL;
    CSR_Animation_Frame* pAnimation;
    char                 skinName[16];
    char                 prevSkinName[16];
    unsigned             animationStartIndex;
    unsigned             skinNameIndex;
    const size_t         skinNameLength = sizeof(skinName);
    size_t               i;
    size_t               j;
    size_t               offset        = 0;
    double               lastKnownTime = 0.0;

    // no buffer to read from?
    if (!pBuffer)
        return 0;

    // create a MDL model
    pMDL = (CSR_MDL*)malloc(sizeof(CSR_MDL));

    // succeeded?
    if (!pMDL)
        return 0;

    // initialize it
    csrMDLInit(pMDL);

    // create mdl header
    pHeader = (CSR_MDLHeader*)malloc(sizeof(CSR_MDLHeader));

    // succeeded?
    if (!pHeader)
    {
        csrMDLRelease(pMDL, fOnDeleteTexture);
        return 0;
    }

    // read file header
    csrMDLReadHeader(pBuffer, &offset, pHeader);

    // is mdl file and version correct?
    if ((pHeader->m_ID != M_MDL_ID) || ((float)pHeader->m_Version != M_MDL_Mesh_File_Version))
    {
        free(pHeader);
        csrMDLRelease(pMDL, fOnDeleteTexture);
        return 0;
    }

    // read skins
    if (pHeader->m_SkinCount)
    {
        pSkin = (CSR_MDLSkin*)malloc(sizeof(CSR_MDLSkin) * pHeader->m_SkinCount);

        for (i = 0; i < pHeader->m_SkinCount; ++i)
            if (!csrMDLReadSkin(pBuffer, &offset, pHeader, &pSkin[i]))
            {
                // release the used memory
                csrMDLReleaseObjects(pHeader, 0, pSkin, 0, 0);
                csrMDLRelease(pMDL, fOnDeleteTexture);
                return 0;
            }
    }
    else
        pSkin = 0;

    // read texture coordinates
    if (pHeader->m_VertexCount)
    {
        pTexCoord = (CSR_MDLTextureCoord*)malloc(sizeof(CSR_MDLTextureCoord) * pHeader->m_VertexCount);

        for (i = 0; i < pHeader->m_VertexCount; ++i)
            if (!csrMDLReadTextureCoord(pBuffer, &offset, &pTexCoord[i]))
            {
                // release the used memory
                csrMDLReleaseObjects(pHeader, 0, pSkin, pTexCoord, 0);
                csrMDLRelease(pMDL, fOnDeleteTexture);
                return 0;
            }
    }
    else
        pTexCoord = 0;

    // read polygons
    if (pHeader->m_PolygonCount)
    {
        pPolygon = (CSR_MDLPolygon*)malloc(sizeof(CSR_MDLPolygon) * pHeader->m_PolygonCount);

        for (i = 0; i < pHeader->m_PolygonCount; ++i)
            if (!csrMDLReadPolygon(pBuffer, &offset, &pPolygon[i]))
            {
                // release the used memory
                csrMDLReleaseObjects(pHeader, 0, pSkin, pTexCoord, pPolygon);
                csrMDLRelease(pMDL, fOnDeleteTexture);
                return 0;
            }
    }
    else
        pPolygon = 0;

    // read frames
    if (pHeader->m_FrameCount)
    {
        pFrameGroup = (CSR_MDLFrameGroup*)malloc(sizeof(CSR_MDLFrameGroup) * pHeader->m_FrameCount);

        for (i = 0; i < pHeader->m_FrameCount; ++i)
            if (!csrMDLReadFrameGroup(pBuffer, &offset, pHeader, &pFrameGroup[i]))
            {
                // release the used memory
                csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);
                csrMDLRelease(pMDL, fOnDeleteTexture);
                return 0;
            }
    }
    else
        pFrameGroup = 0;

    // do generate skin?
    if (pSkin && (!pVertFormat || pVertFormat->m_HasTexCoords))
    {
        // assign the memory to contain the skin
        pMDL->m_pSkin     = (CSR_Skin*)malloc(sizeof(CSR_Skin) * pSkin->m_Count);
        pMDL->m_SkinCount = pSkin->m_Count;

        // succeeded?
        if (!pMDL->m_pSkin)
        {
            // release the MDL object used for the loading
            csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

            // release the model
            csrMDLRelease(pMDL, fOnDeleteTexture);

            return 0;
        }

        // iterate through textures to extract
        for (i = 0; i < pSkin->m_Count; ++i)
        {
            int canRelease = 0;

            // initialize the skin values
            csrSkinInit(&pMDL->m_pSkin[i]);

            // extract texture from model
            pMDL->m_pSkin[i].m_Texture.m_pBuffer =
                    csrMDLUncompressTexture(pSkin,
                                            pPalette,
                                            pHeader->m_SkinWidth,
                                            pHeader->m_SkinHeight,
                                            i);

            // is a default texture?
            if (pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_DataLength <= 48)
            {
                unsigned color;

                free(pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_pData);

                // recreate a 4 * 4 * 3 pixel buffer
                pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_DataLength = 48;
                pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_pData      =
                        (unsigned char*)malloc(pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_DataLength);

                // succeeded?
                if (!pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_pData)
                {
                    // release the MDL object used for the loading
                    csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

                    // release the model
                    csrMDLRelease(pMDL, fOnDeleteTexture);

                    return 0;
                }

                // get the texture color from material
                if (pMaterial)
                    color = pMaterial->m_Color;
                else
                {
                    // create a default material (because the model vertex buffer is still not created)
                    CSR_Material material;
                    csrMaterialInit(&material);

                    color = material.m_Color;
                }

                // initialize the buffer
                for (j = 0; j < 16; ++j)
                {
                    // set color data
                    ((unsigned char*)pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_pData)[ j * 3]      = ((color >> 24) & 0xFF);
                    ((unsigned char*)pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_pData)[(j * 3) + 1] = ((color >> 16) & 0xFF);
                    ((unsigned char*)pMDL->m_pSkin[i].m_Texture.m_pBuffer->m_pData)[(j * 3) + 2] = ((color >> 8)  & 0xFF);
                }
            }

            // skin texture was read successfully?
            if (!pMDL->m_pSkin[i].m_Texture.m_pBuffer)
            {
                // release the MDL object used for the loading
                csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

                // release the model
                csrMDLRelease(pMDL, fOnDeleteTexture);

                return 0;
            }

            // notify that the model skin should be applied
            if (fOnApplySkin)
                fOnApplySkin(i, &pMDL->m_pSkin[i], &canRelease);

            // can release the skin content? (NOTE the skin texture, bump map and cubemap members can
            // still be used as keys even after their content is released)
            if (canRelease)
            {
                // release the skin content
                csrTextureContentRelease(&pMDL->m_pSkin[i].m_Texture);
                csrTextureContentRelease(&pMDL->m_pSkin[i].m_BumpMap);
                csrTextureContentRelease(&pMDL->m_pSkin[i].m_CubeMap);

                // clear their values
                csrSkinInit(&pMDL->m_pSkin[i]);
            }

            // also get the animation time
            if (pSkin->m_pTime)
            {
                pMDL->m_pSkin[i].m_Time = pSkin->m_pTime[i] - lastKnownTime;
                lastKnownTime           = pSkin->m_pTime[i];
            }
            else
                pMDL->m_pSkin[i].m_Time = 0.0;
        }
    }

    // create the models required to keep the frame groups content
    pMDL->m_pModel     = (CSR_Model*)malloc(pHeader->m_FrameCount * sizeof(CSR_Model));
    pMDL->m_ModelCount =  pHeader->m_FrameCount;

    // initialize the previous skin name (needed to detect the animations)
    memset(prevSkinName, 0x0, sizeof(prevSkinName));

    // initialize the animation start index
    animationStartIndex = 0;

    // iterate through MDL model frame group
    for (i = 0; i < pHeader->m_FrameCount; ++i)
    {
        // the frame group contains at least 1 sub-frame?
        if (pFrameGroup[i].m_Count > 0)
        {
            memset(skinName, 0x0, skinNameLength);

            // get the skin name
            #ifdef _MSC_VER
                if (skinName)
                    strcpy_s(skinName, skinNameLength, pFrameGroup[i].m_pFrame[0].m_Name);

                // reset the array content to 0x0 after the string end mark (required to find the animation group below)
                for (j = 0; j < skinNameLength; ++j)
                {
                    // calculate the skin name index
                    skinNameIndex = (unsigned)((skinNameLength - 1) - j);

                    if (skinName[skinNameIndex] != 0x0)
                    {
                        skinName[skinNameIndex] = 0x0;
                        continue;
                    }

                    break;
                }
            #else
                strcpy(skinName, pFrameGroup[i].m_pFrame[0].m_Name);
            #endif

            // revert iterate through the skin name and remove all the trailing numbers
            for (j = 0; j < skinNameLength; ++j)
            {
                // calculate the skin name index
                skinNameIndex = (unsigned)((skinNameLength - 1) - j);

                // is char empty or is a number?
                if (skinName[skinNameIndex] == 0x0 ||
                   (skinName[skinNameIndex] >= '0' && skinName[skinNameIndex] <= '9'))
                {
                    // erase it
                    skinName[skinNameIndex] = 0x0;
                    continue;
                }

                break;
            }

            // is previous skin name already initialized?
            if (prevSkinName[0] == 0x0)
                // no, initialize it
                memcpy(prevSkinName, skinName, skinNameLength);

            // do begin a new animation?
            if (i == (size_t)pHeader->m_FrameCount - 1 || memcmp(skinName, prevSkinName, skinNameLength) != 0)
            {
                // increase the memory to contain the new animation
                pAnimation = (CSR_Animation_Frame*)csrMemoryAlloc(pMDL->m_pAnimation,
                                                                  sizeof(CSR_Animation_Frame),
                                                                  pMDL->m_AnimationCount + 1);

                // succeeded?
                if (!pAnimation)
                {
                    // release the MDL object used for the loading
                    csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

                    // release the model
                    csrMDLRelease(pMDL, fOnDeleteTexture);

                    return 0;
                }

                // update the model
                pMDL->m_pAnimation = pAnimation;
                ++pMDL->m_AnimationCount;

                // get the animation name
                memcpy(pMDL->m_pAnimation[pMDL->m_AnimationCount - 1].m_Name,
                       prevSkinName,
                       skinNameLength);

                // only one frame?
                if (pHeader->m_FrameCount == 1)
                {
                    // populate the animation
                    pMDL->m_pAnimation[pMDL->m_AnimationCount - 1].m_Start = 0;
                    pMDL->m_pAnimation[pMDL->m_AnimationCount - 1].m_End   = 0;
                }
                else
                {
                    // populate the animation
                    pMDL->m_pAnimation[pMDL->m_AnimationCount - 1].m_Start = animationStartIndex;
                    pMDL->m_pAnimation[pMDL->m_AnimationCount - 1].m_End   = i - 1;
                }

                // prepare the values for the next animation
                animationStartIndex = (unsigned)i;
                memset(prevSkinName, 0x0, skinNameLength);
            }
        }

        // extract model from file content
        csrMDLPopulateModel(pHeader,
                           &pFrameGroup[i],
                            pPolygon,
                            pTexCoord,
                            pVertFormat,
                            pVertCulling,
                            pMaterial,
                            fOnGetVertexColor,
                           &pMDL->m_pModel[i]);
    }

    // release the MDL object used for the loading
    csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

    return pMDL;
}
//---------------------------------------------------------------------------
void csrMDLInit(CSR_MDL* pMDL)
{
    // no MDL model to initialize?
    if (!pMDL)
        return;

    // initialize the MDL model content
    pMDL->m_pModel = 0;
    pMDL->m_ModelCount = 0;
    pMDL->m_pAnimation = 0;
    pMDL->m_AnimationCount = 0;
    pMDL->m_pSkin = 0;
    pMDL->m_SkinCount = 0;
}
//---------------------------------------------------------------------------
CSR_MDL* csrMDLOpen(const char*                 pFileName,
                    const CSR_Buffer*           pPalette,
                    const CSR_VertexFormat*     pVertFormat,
                    const CSR_VertexCulling*    pVertCulling,
                    const CSR_Material*         pMaterial,
                    const CSR_fOnGetVertexColor fOnGetVertexColor,
                    const CSR_fOnApplySkin      fOnApplySkin,
                    const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    CSR_Buffer* pBuffer;
    CSR_MDL*    pMDL;

    // open the model file
    pBuffer = csrFileOpen(pFileName);

    // succeeded?
    if (!pBuffer || !pBuffer->m_Length)
    {
        csrBufferRelease(pBuffer);
        return 0;
    }

    // create the MDL model from the file content
    pMDL = csrMDLCreate(pBuffer,
                        pPalette,
                        pVertFormat,
                        pVertCulling,
                        pMaterial,
                        fOnGetVertexColor,
                        fOnApplySkin,
                        fOnDeleteTexture);

    // release the file buffer (no longer required)
    csrBufferRelease(pBuffer);

    return pMDL;
}
//---------------------------------------------------------------------------
void csrMDLRelease(CSR_MDL* pMDL, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t i;
    size_t j;
    size_t k;

    // no MDL model to release?
    if (!pMDL)
        return;

    // do free the skins?
    if (pMDL->m_pSkin)
    {
        // delete each skin
        for (i = 0; i < pMDL->m_SkinCount; ++i)
            csrSkinContentRelease(&pMDL->m_pSkin[i], fOnDeleteTexture);

        // free the skin container
        free(pMDL->m_pSkin);
    }

    // delete the animations
    if (pMDL->m_pAnimation)
        free(pMDL->m_pAnimation);

    // do free the models content?
    if (pMDL->m_pModel)
    {
        // iterate through models to free
        for (i = 0; i < pMDL->m_ModelCount; ++i)
            // do free the model content?
            if (pMDL->m_pModel[i].m_pMesh)
            {
                // iterate through meshes to free
                for (j = 0; j < pMDL->m_pModel[i].m_MeshCount; ++j)
                    // do free the mesh vertex buffer?
                    if (pMDL->m_pModel[i].m_pMesh[j].m_pVB)
                    {
                        // free the mesh vertex buffer content
                        for (k = 0; k < pMDL->m_pModel[i].m_pMesh[j].m_Count; ++k)
                            if (pMDL->m_pModel[i].m_pMesh[j].m_pVB[k].m_pData)
                                free(pMDL->m_pModel[i].m_pMesh[j].m_pVB[k].m_pData);

                        // free the mesh vertex buffer
                        free(pMDL->m_pModel[i].m_pMesh[j].m_pVB);
                    }

                // free the mesh
                free(pMDL->m_pModel[i].m_pMesh);
            }

        // free the models
        free(pMDL->m_pModel);
    }

    // free the MDL model
    free(pMDL);
}
//---------------------------------------------------------------------------
void csrMDLUpdateIndex(const CSR_MDL* pMDL,
                             size_t   fps,
                             size_t   animationIndex,
                             size_t*  pSkinIndex,
                             size_t*  pModelIndex,
                             size_t*  pMeshIndex,
                             double*  pTextureLastTime,
                             double*  pModelLastTime,
                             double*  pMeshLastTime,
                             double   elapsedTime)
{
    size_t    animLength;
    size_t    animIndex;
    double    interval;
    CSR_Mesh* pMesh;

    // no MDL model to calculate from?
    if (!pMDL)
    {
        // reset all values
        *pSkinIndex       = 0;
        *pModelIndex      = 0;
        *pMeshIndex       = 0;
        *pTextureLastTime = 0.0;
        *pModelLastTime   = 0.0;
        *pMeshLastTime    = 0.0;
        return;
    }

    // are skins animated?
    if (pMDL->m_SkinCount > 1)
    {
        // apply the elapsed time
        *pTextureLastTime += elapsedTime;

        // certify that the skin index is inside the limits
        *pSkinIndex = (*pSkinIndex % pMDL->m_SkinCount);

        // do get the next skin?
        while (*pTextureLastTime >= (pMDL->m_pSkin[*pSkinIndex].m_Time))
        {
            // decrease the counted time
            *pTextureLastTime -= pMDL->m_pSkin[*pSkinIndex].m_Time;

            // go to next index
            *pSkinIndex = ((*pSkinIndex + 1) % pMDL->m_SkinCount);
        }
    }

    // get the current model mesh for which the index should be updated
    pMesh = csrMDLGetMesh(pMDL, *pModelIndex, *pMeshIndex);

    // found it?
    if (!pMesh)
    {
        // reset all values
        *pSkinIndex       = 0;
        *pModelIndex      = 0;
        *pMeshIndex       = 0;
        *pTextureLastTime = 0.0;
        *pModelLastTime   = 0.0;
        *pMeshLastTime    = 0.0;
        return;
    }

    // do animate current model frames? (NOTE the modelIndex value was indirectly validated while
    // csrMDLGetMesh() was executed)
    if (pMDL->m_pModel[*pModelIndex].m_MeshCount > 1 && pMesh->m_Time)
    {
        // apply the elapsed time
        *pMeshLastTime += elapsedTime;

        // certify that the mesh index is inside the limits
        *pMeshIndex = (*pMeshIndex % pMDL->m_pModel[*pModelIndex].m_MeshCount);

        // do get the next mesh?
        while (*pMeshLastTime >= pMesh->m_Time)
        {
            // decrease the counted time
            *pMeshLastTime -= pMesh->m_Time;

            // go to next index
            *pMeshIndex = ((*pMeshIndex + 1) % pMDL->m_pModel[*pModelIndex].m_MeshCount);
        }

        return;
    }

    // is animation index out of bounds?
    if (animationIndex >= pMDL->m_AnimationCount)
    {
        // reset all values
        *pSkinIndex       = 0;
        *pModelIndex      = 0;
        *pMeshIndex       = 0;
        *pTextureLastTime = 0.0;
        *pModelLastTime   = 0.0;
        *pMeshLastTime    = 0.0;
        return;
    }

    // no fps?
    if (!fps)
    {
        // reset all values
        *pSkinIndex       = 0;
        *pModelIndex      = 0;
        *pMeshIndex       = 0;
        *pTextureLastTime = 0.0;
        *pModelLastTime   = 0.0;
        *pMeshLastTime    = 0.0;
        return;
    }

    // calculate the running animation length
    animLength = pMDL->m_pAnimation[animationIndex].m_End - pMDL->m_pAnimation[animationIndex].m_Start;

    // is animation empty?
    if (!animLength)
    {
        // reset all values
        *pSkinIndex       = 0;
        *pModelIndex      = 0;
        *pMeshIndex       = 0;
        *pTextureLastTime = 0.0;
        *pModelLastTime   = 0.0;
        *pMeshLastTime    = 0.0;
        return;
    }

    // apply the elapsed time
    *pModelLastTime += elapsedTime;

    // calculate the frame interval
    interval = 1.0 / fps;

    // calculate the model animation index, and certify that is it inside the limits
    animIndex = ((*pModelIndex - pMDL->m_pAnimation[animationIndex].m_Start) % animLength);

    // do get the next model?
    while (*pModelLastTime >= interval)
    {
        // decrease the counted time
        *pModelLastTime -= interval;

        // go to next index
        animIndex = ((animIndex + 1) % animLength);
    }

    *pModelIndex = pMDL->m_pAnimation[animationIndex].m_Start + animIndex;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrMDLGetMesh(const CSR_MDL* pMDL, size_t modelIndex, size_t meshIndex)
{
    // no MDL model?
    if (!pMDL)
        return 0;

    // is model index valid?
    if (modelIndex >= pMDL->m_ModelCount)
        return 0;

    // determine how many meshes the model contains
    if (!pMDL->m_pModel[modelIndex].m_MeshCount)
        // no mesh, nothing to do
        return 0;
    else
    if (pMDL->m_pModel[modelIndex].m_MeshCount == 1)
        // one mesh, return it
        return pMDL->m_pModel[modelIndex].m_pMesh;

    // several meshes (i.e. meshes are animated), check if mesh index is out of bounds
    if (meshIndex >= pMDL->m_pModel[modelIndex].m_MeshCount)
        return 0;

    // draw the model mesh
    return &pMDL->m_pModel[modelIndex].m_pMesh[meshIndex];
}
//---------------------------------------------------------------------------
