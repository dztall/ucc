/****************************************************************************
 * ==> CSR_Model -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the model functions and types         *
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

#include "CSR_Model.h"

// std
#include <stdlib.h>
#include <math.h>
#include <string.h>

// visual studio specific code
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif

// this code is EXPERIMENTAL and should be STRONGLY TESTED on big endian machines before be activated
#define CONVERT_ENDIANNESS

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
// Shape functions
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateSurface(float                 width,
                                float                 height,
                          const CSR_VertexFormat*     pVertFormat,
                          const CSR_VertexCulling*    pVertCulling,
                          const CSR_Material*         pMaterial,
                          const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        int         i;
        int         index;
        CSR_Mesh*   pMesh  = {0};
        CSR_Vector3 vertex = {0};
        CSR_Vector3 normal = {0};
        CSR_Vector2 uv     = {0};
    #else
        int         i;
        int         index;
        CSR_Mesh*   pMesh;
        CSR_Vector3 vertex;
        CSR_Vector3 normal;
        CSR_Vector2 uv;
    #endif

    // create a buffer template: 0 for negative values, 1 for positive
    const int bufferTemplate[] =
    {
        0, 0,
        0, 1,
        1, 0,
        1, 1,
    };

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a vertex buffer
    pMesh->m_pVB = csrVertexBufferCreate();

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh, 0);
        return 0;
    }

    pMesh->m_Count = 1;

    // apply the user wished vertex format
    if (pVertFormat)
        pMesh->m_pVB->m_Format = *pVertFormat;

    // apply the user wished vertex culling
    if (pVertCulling)
        pMesh->m_pVB->m_Culling = *pVertCulling;
    else
    {
        // otherwise configure the default culling
        pMesh->m_pVB->m_Culling.m_Type = CSR_CT_None;
        pMesh->m_pVB->m_Culling.m_Face = CSR_CF_CW;
    }

    // apply the user wished material
    if (pMaterial)
        pMesh->m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_TriangleStrip;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // iterate through vertex to create
    for (i = 0; i < 4; ++i)
    {
        // calculate template buffer index
        index = i * 2;

        // populate vertex buffer
        if (bufferTemplate[index])
            vertex.m_X =  width / 2.0f;
        else
            vertex.m_X = -width / 2.0f;

        if (bufferTemplate[index + 1])
            vertex.m_Y =  height / 2.0f;
        else
            vertex.m_Y = -height / 2.0f;

        vertex.m_Z = 0.0f;

        // vertex has a normal?
        if (pMesh->m_pVB[0].m_Format.m_HasNormal)
        {
            normal.m_X =  0.0f;
            normal.m_Y =  0.0f;
            normal.m_Z = -1.0f;
        }

        // vertex has UV texture coordinates?
        if (pMesh->m_pVB[0].m_Format.m_HasTexCoords)
        {
            // calculate texture u coordinate
            if (bufferTemplate[index])
                uv.m_X = 1.0f;
            else
                uv.m_X = 0.0f;

            // calculate texture v coordinate
            if (bufferTemplate[index + 1])
                uv.m_Y = 1.0f;
            else
                uv.m_Y = 0.0f;
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex, &normal, &uv, i, fOnGetVertexColor, pMesh->m_pVB);
    }

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateBox(float                 width,
                            float                 height,
                            float                 depth,
                            int                   repeatTexOnEachFace,
                      const CSR_VertexFormat*     pVertFormat,
                      const CSR_VertexCulling*    pVertCulling,
                      const CSR_Material*         pMaterial,
                      const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        size_t      i;
        CSR_Vector3 vertices[8]   = {0};
        CSR_Vector3 normals[6]    = {0};
        CSR_Vector2 texCoords[24] = {0};
        CSR_Mesh*   pMesh;
    #else
        size_t      i;
        CSR_Vector3 vertices[8];
        CSR_Vector3 normals[6];
        CSR_Vector2 texCoords[24];
        CSR_Mesh*   pMesh;
    #endif

    // calculate half values
    const float halfX = width  / 2.0f;
    const float halfY = height / 2.0f;
    const float halfZ = depth  / 2.0f;

    // create a new mesh for the box
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a vertex buffer for each box edges
    pMesh->m_pVB = (CSR_VertexBuffer*)csrMemoryAlloc(0, sizeof(CSR_VertexBuffer), 6);

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh, 0);
        return 0;
    }

    pMesh->m_Count = 6;

    // iterate through each edges
    for (i = 0; i < 6; ++i)
    {
        // initialize the vertex buffer
        csrVertexBufferInit(&pMesh->m_pVB[i]);

        // apply the user wished vertex format
        if (pVertFormat)
            pMesh->m_pVB[i].m_Format = *pVertFormat;

        // apply the user wished vertex culling
        if (pVertCulling)
            pMesh->m_pVB[i].m_Culling = *pVertCulling;

        // apply the user wished material
        if (pMaterial)
            pMesh->m_pVB[i].m_Material = *pMaterial;

        // set the vertex format type
        pMesh->m_pVB[i].m_Format.m_Type = CSR_VT_TriangleStrip;

        // calculate the stride
        csrVertexFormatCalculateStride(&pMesh->m_pVB[i].m_Format);
    }

    // iterate through vertices to create. Vertices are generated as follow:
    //     v2 *--------* v6
    //      / |      / |
    // v4 *--------* v8|
    //    |   |    |   |
    //    |v1 *----|---* v5
    //    | /      | /
    // v3 *--------* v7
    for (i = 0; i < 8; ++i)
    {
        // generate the 4 first vertices on the left, and 4 last on the right
        if (!(i / 4))
            vertices[i].m_X = -halfX;
        else
            vertices[i].m_X =  halfX;

        // generate 2 vertices on the front, then 2 vertices on the back
        if (!((i / 2) % 2))
            vertices[i].m_Z = -halfZ;
        else
            vertices[i].m_Z =  halfZ;

        // for each vertices, generates one on the top, and one on the bottom
        if (!(i % 2))
            vertices[i].m_Y = -halfY;
        else
            vertices[i].m_Y =  halfY;
    }

    // calculate normals
    normals[0].m_X = -1.0; normals[0].m_Y =  0.0; normals[0].m_Z =  0.0;
    normals[1].m_X =  1.0; normals[1].m_Y =  0.0; normals[1].m_Z =  0.0;
    normals[2].m_X =  0.0; normals[2].m_Y = -1.0; normals[2].m_Z =  0.0;
    normals[3].m_X =  0.0; normals[3].m_Y =  1.0; normals[3].m_Z =  0.0;
    normals[4].m_X =  0.0; normals[4].m_Y =  0.0; normals[4].m_Z = -1.0;
    normals[5].m_X =  0.0; normals[5].m_Y =  0.0; normals[5].m_Z =  1.0;

    // do repeat texture on each faces?
    if (repeatTexOnEachFace)
    {
        // calculate texture positions
        texCoords[0].m_X  = 0.0; texCoords[0].m_Y  = 0.0;
        texCoords[1].m_X  = 0.0; texCoords[1].m_Y  = 1.0;
        texCoords[2].m_X  = 1.0; texCoords[2].m_Y  = 0.0;
        texCoords[3].m_X  = 1.0; texCoords[3].m_Y  = 1.0;
        texCoords[4].m_X  = 0.0; texCoords[4].m_Y  = 0.0;
        texCoords[5].m_X  = 0.0; texCoords[5].m_Y  = 1.0;
        texCoords[6].m_X  = 1.0; texCoords[6].m_Y  = 0.0;
        texCoords[7].m_X  = 1.0; texCoords[7].m_Y  = 1.0;
        texCoords[8].m_X  = 0.0; texCoords[8].m_Y  = 0.0;
        texCoords[9].m_X  = 0.0; texCoords[9].m_Y  = 1.0;
        texCoords[10].m_X = 1.0; texCoords[10].m_Y = 0.0;
        texCoords[11].m_X = 1.0; texCoords[11].m_Y = 1.0;
        texCoords[12].m_X = 0.0; texCoords[12].m_Y = 0.0;
        texCoords[13].m_X = 0.0; texCoords[13].m_Y = 1.0;
        texCoords[14].m_X = 1.0; texCoords[14].m_Y = 0.0;
        texCoords[15].m_X = 1.0; texCoords[15].m_Y = 1.0;
        texCoords[16].m_X = 0.0; texCoords[16].m_Y = 0.0;
        texCoords[17].m_X = 0.0; texCoords[17].m_Y = 1.0;
        texCoords[18].m_X = 1.0; texCoords[18].m_Y = 0.0;
        texCoords[19].m_X = 1.0; texCoords[19].m_Y = 1.0;
        texCoords[20].m_X = 0.0; texCoords[20].m_Y = 0.0;
        texCoords[21].m_X = 0.0; texCoords[21].m_Y = 1.0;
        texCoords[22].m_X = 1.0; texCoords[22].m_Y = 0.0;
        texCoords[23].m_X = 1.0; texCoords[23].m_Y = 1.0;
    }
    else
    {
        // calculate texture offset
        const float texOffset = 1.0f / 3.0f;

        // calculate texture positions. They are distributed as follow:
        // -------------------
        // |     |     |     |
        // |  1  |  2  |  3  |
        // |     |     |     |
        // |-----------------|
        // |     |     |     |
        // |  4  |  5  |  6  |
        // |     |     |     |
        // -------------------
        // |  This texture   |
        // |  area is not    |
        // |  used           |
        // -------------------
        texCoords[0].m_X  = 0.0f;             texCoords[0].m_Y  = texOffset;
        texCoords[1].m_X  = 0.0f;             texCoords[1].m_Y  = 0.0f;
        texCoords[2].m_X  = texOffset;        texCoords[2].m_Y  = texOffset;
        texCoords[3].m_X  = texOffset;        texCoords[3].m_Y  = 0.0f;
        texCoords[4].m_X  = texOffset;        texCoords[4].m_Y  = texOffset;
        texCoords[5].m_X  = texOffset;        texCoords[5].m_Y  = 0.0f;
        texCoords[6].m_X  = texOffset * 2.0f; texCoords[6].m_Y  = texOffset;
        texCoords[7].m_X  = texOffset * 2.0f; texCoords[7].m_Y  = 0.0f;
        texCoords[8].m_X  = texOffset * 2.0f; texCoords[8].m_Y  = texOffset;
        texCoords[9].m_X  = texOffset * 2.0f; texCoords[9].m_Y  = 0.0f;
        texCoords[10].m_X = 1.0f;             texCoords[10].m_Y = texOffset;
        texCoords[11].m_X = 1.0f;             texCoords[11].m_Y = 0.0f;
        texCoords[12].m_X = 0.0f;             texCoords[12].m_Y = texOffset * 2.0f;
        texCoords[13].m_X = 0.0f;             texCoords[13].m_Y = texOffset;
        texCoords[14].m_X = texOffset;        texCoords[14].m_Y = texOffset * 2.0f;
        texCoords[15].m_X = texOffset;        texCoords[15].m_Y = texOffset;
        texCoords[16].m_X = texOffset;        texCoords[16].m_Y = texOffset * 2.0f;
        texCoords[17].m_X = texOffset;        texCoords[17].m_Y = texOffset;
        texCoords[18].m_X = texOffset * 2.0f; texCoords[18].m_Y = texOffset * 2.0f;
        texCoords[19].m_X = texOffset * 2.0f; texCoords[19].m_Y = texOffset;
        texCoords[20].m_X = texOffset * 2.0f; texCoords[20].m_Y = texOffset * 2.0f;
        texCoords[21].m_X = texOffset * 2.0f; texCoords[21].m_Y = texOffset;
        texCoords[22].m_X = 1.0f;             texCoords[22].m_Y = texOffset * 2.0f;
        texCoords[23].m_X = 1.0f;             texCoords[23].m_Y = texOffset;
    }

    // create box edge 1
    csrVertexBufferAdd(&vertices[1], &normals[0], &texCoords[4], 0, fOnGetVertexColor, &pMesh->m_pVB[0]);
    csrVertexBufferAdd(&vertices[0], &normals[0], &texCoords[5], 1, fOnGetVertexColor, &pMesh->m_pVB[0]);
    csrVertexBufferAdd(&vertices[3], &normals[0], &texCoords[6], 2, fOnGetVertexColor, &pMesh->m_pVB[0]);
    csrVertexBufferAdd(&vertices[2], &normals[0], &texCoords[7], 3, fOnGetVertexColor, &pMesh->m_pVB[0]);

    // create box edge 2
    csrVertexBufferAdd(&vertices[3], &normals[5], &texCoords[8],  0, fOnGetVertexColor, &pMesh->m_pVB[1]);
    csrVertexBufferAdd(&vertices[2], &normals[5], &texCoords[9],  1, fOnGetVertexColor, &pMesh->m_pVB[1]);
    csrVertexBufferAdd(&vertices[7], &normals[5], &texCoords[10], 2, fOnGetVertexColor, &pMesh->m_pVB[1]);
    csrVertexBufferAdd(&vertices[6], &normals[5], &texCoords[11], 3, fOnGetVertexColor, &pMesh->m_pVB[1]);

    // create box edge 3
    csrVertexBufferAdd(&vertices[7], &normals[1], &texCoords[12], 0, fOnGetVertexColor, &pMesh->m_pVB[2]);
    csrVertexBufferAdd(&vertices[6], &normals[1], &texCoords[13], 1, fOnGetVertexColor, &pMesh->m_pVB[2]);
    csrVertexBufferAdd(&vertices[5], &normals[1], &texCoords[14], 2, fOnGetVertexColor, &pMesh->m_pVB[2]);
    csrVertexBufferAdd(&vertices[4], &normals[1], &texCoords[15], 3, fOnGetVertexColor, &pMesh->m_pVB[2]);

    // create box edge 4
    csrVertexBufferAdd(&vertices[5], &normals[4], &texCoords[16], 0, fOnGetVertexColor, &pMesh->m_pVB[3]);
    csrVertexBufferAdd(&vertices[4], &normals[4], &texCoords[17], 1, fOnGetVertexColor, &pMesh->m_pVB[3]);
    csrVertexBufferAdd(&vertices[1], &normals[4], &texCoords[18], 2, fOnGetVertexColor, &pMesh->m_pVB[3]);
    csrVertexBufferAdd(&vertices[0], &normals[4], &texCoords[19], 3, fOnGetVertexColor, &pMesh->m_pVB[3]);

    // create box edge 5
    csrVertexBufferAdd(&vertices[1], &normals[3], &texCoords[0], 0, fOnGetVertexColor, &pMesh->m_pVB[4]);
    csrVertexBufferAdd(&vertices[3], &normals[3], &texCoords[1], 1, fOnGetVertexColor, &pMesh->m_pVB[4]);
    csrVertexBufferAdd(&vertices[5], &normals[3], &texCoords[2], 2, fOnGetVertexColor, &pMesh->m_pVB[4]);
    csrVertexBufferAdd(&vertices[7], &normals[3], &texCoords[3], 3, fOnGetVertexColor, &pMesh->m_pVB[4]);

    // create box edge 6
    csrVertexBufferAdd(&vertices[2], &normals[2], &texCoords[20], 0, fOnGetVertexColor, &pMesh->m_pVB[5]);
    csrVertexBufferAdd(&vertices[0], &normals[2], &texCoords[21], 1, fOnGetVertexColor, &pMesh->m_pVB[5]);
    csrVertexBufferAdd(&vertices[6], &normals[2], &texCoords[22], 2, fOnGetVertexColor, &pMesh->m_pVB[5]);
    csrVertexBufferAdd(&vertices[4], &normals[2], &texCoords[23], 3, fOnGetVertexColor, &pMesh->m_pVB[5]);

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateSphere(float                 radius,
                               int                   slices,
                               int                   stacks,
                         const CSR_VertexFormat*     pVertFormat,
                         const CSR_VertexCulling*    pVertCulling,
                         const CSR_Material*         pMaterial,
                         const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        int               i;
        int               j;
        float             majorStep;
        float             minorStep;
        float             a;
        float             b;
        float             r0;
        float             r1;
        float             z0;
        float             z1;
        float             c;
        float             x;
        float             y;
        size_t            index;
        CSR_Mesh*         pMesh;
        CSR_VertexBuffer* pVB;
        CSR_Vector3       vertex = {0};
        CSR_Vector3       normal = {0};
        CSR_Vector2       uv     = {0};
    #else
        int               i;
        int               j;
        float             majorStep;
        float             minorStep;
        float             a;
        float             b;
        float             r0;
        float             r1;
        float             z0;
        float             z1;
        float             c;
        float             x;
        float             y;
        size_t            index;
        CSR_Mesh*         pMesh;
        CSR_VertexBuffer* pVB;
        CSR_Vector3       vertex;
        CSR_Vector3       normal;
        CSR_Vector2       uv;
    #endif

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // initialize global values
    majorStep = ((float)M_PI         / slices);
    minorStep = ((float)(2.0 * M_PI) / stacks);

    // iterate through vertex slices
    for (i = 0; i < slices; ++i)
    {
        // create a new vertex buffer to contain the next slice
        pVB = (CSR_VertexBuffer*)csrMemoryAlloc(pMesh->m_pVB,
                                                sizeof(CSR_VertexBuffer),
                                                pMesh->m_Count + 1);

        // succeeded?
        if (!pVB)
        {
            csrMeshRelease(pMesh, 0);
            return 0;
        }

        // get the vertex buffer index and update the mesh
        index        = pMesh->m_Count;
        pMesh->m_pVB = pVB;
        ++pMesh->m_Count;

        // initialize the newly created vertex buffer
        csrVertexBufferInit(&pMesh->m_pVB[index]);

        // apply the user wished vertex format
        if (pVertFormat)
            pMesh->m_pVB[index].m_Format = *pVertFormat;

        // apply the user wished vertex culling
        if (pVertCulling)
            pMesh->m_pVB[index].m_Culling = *pVertCulling;

        // apply the user wished material
        if (pMaterial)
            pMesh->m_pVB[index].m_Material = *pMaterial;

        // set the vertex format type
        pMesh->m_pVB[index].m_Format.m_Type = CSR_VT_TriangleStrip;

        // calculate the stride
        csrVertexFormatCalculateStride(&pMesh->m_pVB[index].m_Format);

        // calculate next slice values
        a  = i      * majorStep;
        b  = a      + majorStep;
        r0 = radius * sinf(a);
        r1 = radius * sinf(b);
        z0 = radius * cosf(a);
        z1 = radius * cosf(b);

        // iterate through vertex stacks
        for (j = 0; j <= stacks; ++j)
        {
            c = j * minorStep;
            x = cosf(c);
            y = sinf(c);

            // calculate vertex
            vertex.m_X = x * r0;
            vertex.m_Y = y * r0;
            vertex.m_Z =     z0;

            // vertex has a normal?
            if (pMesh->m_pVB[0].m_Format.m_HasNormal)
            {
                normal.m_X = (x * r0) / radius;
                normal.m_Y = (y * r0) / radius;
                normal.m_Z =      z0  / radius;
            }

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasTexCoords)
            {
                uv.m_X = ((float)j / (float)stacks);
                uv.m_Y = ((float)i / (float)slices);
            }

            // add the vertex to the buffer
            csrVertexBufferAdd(&vertex,
                               &normal,
                               &uv,
                                j * 2,
                                fOnGetVertexColor,
                               &pMesh->m_pVB[index]);

            // calculate vertex
            vertex.m_X = x * r1;
            vertex.m_Y = y * r1;
            vertex.m_Z =     z1;

            // vertex has a normal?
            if (pMesh->m_pVB[0].m_Format.m_HasNormal)
            {
                normal.m_X = (x * r1) / radius;
                normal.m_Y = (y * r1) / radius;
                normal.m_Z =      z1  / radius;
            }

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasTexCoords)
            {
                uv.m_X = ( (float)j         / (float)stacks);
                uv.m_Y = (((float)i + 1.0f) / (float)slices);
            }

            // add the vertex to the buffer
            csrVertexBufferAdd(&vertex,
                               &normal,
                               &uv,
                               (j * 2) + 1,
                                fOnGetVertexColor,
                               &pMesh->m_pVB[index]);
        }
    }

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateCylinder(float                 minRadius,
                                 float                 maxRadius,
                                 float                 height,
                                 int                   faces,
                           const CSR_VertexFormat*     pVertFormat,
                           const CSR_VertexCulling*    pVertCulling,
                           const CSR_Material*         pMaterial,
                           const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        int         i;
        float       angle;
        float       step;
        CSR_Vector3 vertex = {0};
        CSR_Vector3 normal = {0};
        CSR_Vector2 uv     = {0};
        CSR_Mesh*   pMesh  = {0};
    #else
        int         i;
        float       angle;
        float       step;
        CSR_Vector3 vertex;
        CSR_Vector3 normal;
        CSR_Vector2 uv;
        CSR_Mesh*   pMesh;
    #endif

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a vertex buffer
    pMesh->m_pVB = csrVertexBufferCreate();

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh, 0);
        return 0;
    }

    pMesh->m_Count = 1;

    // apply the user wished vertex format
    if (pVertFormat)
        pMesh->m_pVB->m_Format = *pVertFormat;

    // apply the user wished vertex culling
    if (pVertCulling)
        pMesh->m_pVB->m_Culling = *pVertCulling;

    // apply the user wished material
    if (pMaterial)
        pMesh->m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_TriangleStrip;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // calculate step to apply between faces
    step = (float)(2.0 * M_PI) / (float)faces;

    // iterate through vertices to create
    for (i = 0; i < faces + 1; ++i)
    {
        // calculate angle
        angle = step * i;

        // set vertex data
        vertex.m_X =   minRadius * cosf(angle);
        vertex.m_Y = -(height    / 2.0f);
        vertex.m_Z =   minRadius * sinf(angle);

        // vertex has a normal?
        if (pMesh->m_pVB->m_Format.m_HasNormal)
        {
            // set normals
            normal.m_X = cosf(angle);
            normal.m_Y = 0.0f;
            normal.m_Z = sinf(angle);
        }

        // vertex has UV texture coordinates?
        if (pMesh->m_pVB->m_Format.m_HasTexCoords)
        {
            // is the first point to calculate?
            if (!i)
            {
                // add texture coordinates data to buffer
                uv.m_X = 0.0f;
                uv.m_Y = 0.0f;
            }
            else
            {
                // add texture coordinates data to buffer
                uv.m_X = 1.0f / (float)i;
                uv.m_Y = 0.0f;
            }
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex, &normal, &uv, i * 2, fOnGetVertexColor, pMesh->m_pVB);

        // set vertex data
        vertex.m_X =  maxRadius * cosf(angle);
        vertex.m_Y = (height    / 2.0f);
        vertex.m_Z =  maxRadius * sinf(angle);

        // vertex has a normal?
        if (pMesh->m_pVB->m_Format.m_HasNormal)
        {
            // set normals
            normal.m_X = cosf(angle);
            normal.m_Y = 0.0f;
            normal.m_Z = sinf(angle);
        }

        // vertex has UV texture coordinates?
        if (pMesh->m_pVB->m_Format.m_HasTexCoords)
        {
            // is the first point to calculate?
            if (!i)
            {
                // add texture coordinates data to buffer
                uv.m_X = 0.0f;
                uv.m_Y = 1.0f;
            }
            else
            {
                // add texture coordinates data to buffer
                uv.m_X = 1.0f / (float)i;
                uv.m_Y = 1.0f;
            }
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex, &normal, &uv, (i * 2) + 1, fOnGetVertexColor, pMesh->m_pVB);
    }

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateDisk(float                 centerX,
                             float                 centerY,
                             float                 radius,
                             unsigned              slices,
                       const CSR_VertexFormat*     pVertFormat,
                       const CSR_VertexCulling*    pVertCulling,
                       const CSR_Material*         pMaterial,
                       const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        unsigned    i;
        float       x;
        float       y;
        float       step;
        float       angle;
        CSR_Vector3 vertex = {0};
        CSR_Vector3 normal = {0};
        CSR_Vector2 uv     = {0};
        CSR_Mesh*   pMesh;
    #else
        unsigned    i;
        float       x;
        float       y;
        float       step;
        float       angle;
        CSR_Vector3 vertex;
        CSR_Vector3 normal;
        CSR_Vector2 uv;
        CSR_Mesh*   pMesh;
    #endif

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a vertex buffer
    pMesh->m_pVB = csrVertexBufferCreate();

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh, 0);
        return 0;
    }

    pMesh->m_Count = 1;

    // apply the user wished vertex format
    if (pVertFormat)
        pMesh->m_pVB->m_Format = *pVertFormat;

    // apply the user wished vertex culling
    if (pVertCulling)
        pMesh->m_pVB->m_Culling = *pVertCulling;
    else
    {
        // otherwise configure the default culling
        pMesh->m_pVB->m_Culling.m_Type = CSR_CT_None;
        pMesh->m_pVB->m_Culling.m_Face = CSR_CF_CW;
    }

    // apply the user wished material
    if (pMaterial)
        pMesh->m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_TriangleFan;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // calculate the slice step
    step = (float)(2.0 * M_PI) / (float)slices;

    // iterate through disk slices to create
    for (i = 0; i <= slices + 1; ++i)
    {
        angle = 0.0f;

        // is the first point to calculate?
        if (!i)
        {
            // get the center
            x = centerX;
            y = centerY;
        }
        else
        {
            // calculate the current slice angle
            angle = step * (float)(i - 1);

            // calculate the slice point
            x = centerX + radius * cosf(angle);
            y = centerY + radius * sinf(angle);
        }

        // add min point in buffer
        vertex.m_X = x;
        vertex.m_Y = y;
        vertex.m_Z = 0.0f;

        // vertex has a normal?
        if (pMesh->m_pVB->m_Format.m_HasNormal)
        {
            // set normal data
            normal.m_X = 0.0f;
            normal.m_Y = 0.0f;
            normal.m_Z = 1.0f;
        }

        // vertex has UV texture coordinates?
        if (pMesh->m_pVB->m_Format.m_HasTexCoords)
            // set texture data
            if (!i)
            {
                uv.m_X = 0.5f;
                uv.m_Y = 0.5f;
            }
            else
            {
                uv.m_X = 0.5f + (cosf(angle) * 0.5f);
                uv.m_Y = 0.5f + (sinf(angle) * 0.5f);
            }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex, &normal, &uv, i, fOnGetVertexColor, pMesh->m_pVB);
    }

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateRing(float                 centerX,
                             float                 centerY,
                             float                 minRadius,
                             float                 maxRadius,
                             unsigned              slices,
                       const CSR_VertexFormat*     pVertFormat,
                       const CSR_VertexCulling*    pVertCulling,
                       const CSR_Material*         pMaterial,
                       const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        unsigned    i;
        float       xA;
        float       yA;
        float       xB;
        float       yB;
        float       step;
        float       angle;
        float       texU;
        CSR_Vector3 vertex = {0};
        CSR_Vector3 normal = {0};
        CSR_Vector2 uv     = {0};
        CSR_Mesh*   pMesh;
    #else
        unsigned    i;
        float       xA;
        float       yA;
        float       xB;
        float       yB;
        float       step;
        float       angle;
        float       texU;
        CSR_Vector3 vertex;
        CSR_Vector3 normal;
        CSR_Vector2 uv;
        CSR_Mesh*   pMesh;
    #endif

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a vertex buffer
    pMesh->m_pVB = csrVertexBufferCreate();

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh, 0);
        return 0;
    }

    pMesh->m_Count = 1;

    // apply the user wished vertex format
    if (pVertFormat)
        pMesh->m_pVB->m_Format = *pVertFormat;

    // apply the user wished vertex culling
    if (pVertCulling)
        pMesh->m_pVB->m_Culling = *pVertCulling;
    else
    {
        // otherwise configure the default culling
        pMesh->m_pVB->m_Culling.m_Type = CSR_CT_None;
        pMesh->m_pVB->m_Culling.m_Face = CSR_CF_CW;
    }

    // apply the user wished material
    if (pMaterial)
        pMesh->m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_TriangleStrip;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // calculate the slice step
    step = (float)(2.0 * M_PI) / (float)slices;

    // iterate through ring slices to create
    for (i = 0; i <= slices; ++i)
    {
        // calculate the current slice angle
        angle = step * (float)i;

        // calculate the slice min point
        xA = centerX + minRadius * cosf(angle);
        yA = centerY - minRadius * sinf(angle);

        // calculate the slice max point
        xB = centerX + maxRadius * cosf(angle);
        yB = centerY - maxRadius * sinf(angle);

        // calculate texture u coordinate
        if (!i)
            texU = 0.0f;
        else
        if (i == slices)
            texU = 1.0f;
        else
            texU = (float)i / (float)slices;

        // add min point in buffer
        vertex.m_X = xA;
        vertex.m_Y = yA;
        vertex.m_Z = 0.0f;

        // vertex has a normal?
        if (pMesh->m_pVB->m_Format.m_HasNormal)
        {
            // set normal data
            normal.m_X = 0.0f;
            normal.m_Y = 0.0f;
            normal.m_Z = 1.0f;
        }

        // vertex has UV texture coordinates?
        if (pMesh->m_pVB->m_Format.m_HasTexCoords)
        {
            // set texture data
            uv.m_X = texU;
            uv.m_Y = 0.0f;
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex, &normal, &uv, i * 2, fOnGetVertexColor, pMesh->m_pVB);

        // add max point in the buffer
        vertex.m_X = xB;
        vertex.m_Y = yB;
        vertex.m_Z = 0.0f;

        // vertex has a normal?
        if (pMesh->m_pVB->m_Format.m_HasNormal)
        {
            // set normal data
            normal.m_X = 0.0f;
            normal.m_Y = 0.0f;
            normal.m_Z = 1.0f;
        }

        // vertex has UV texture coordinates?
        if (pMesh->m_pVB->m_Format.m_HasTexCoords)
        {
            // set texture data
            uv.m_X = texU;
            uv.m_Y = 1.0f;
        }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex, &normal, &uv, (i * 2) + 1, fOnGetVertexColor, pMesh->m_pVB);
    }

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateSpiral(float                 centerX,
                               float                 centerY,
                               float                 minRadius,
                               float                 maxRadius,
                               float                 deltaMin,
                               float                 deltaMax,
                               float                 deltaZ,
                               unsigned              slices,
                               unsigned              stacks,
                         const CSR_VertexFormat*     pVertFormat,
                         const CSR_VertexCulling*    pVertCulling,
                         const CSR_Material*         pMaterial,
                         const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        unsigned          i;
        unsigned          j;
        float             xA;
        float             yA;
        float             xB;
        float             yB;
        float             step;
        float             angle;
        float             z;
        float             texU;
        size_t            index;
        CSR_Vector3       vertex = {0};
        CSR_Vector3       normal = {0};
        CSR_Vector2       uv     = {0};
        CSR_Mesh*         pMesh;
        CSR_VertexBuffer* pVB;
    #else
        unsigned          i;
        unsigned          j;
        float             xA;
        float             yA;
        float             xB;
        float             yB;
        float             step;
        float             angle;
        float             z;
        float             texU;
        size_t            index;
        CSR_Vector3       vertex;
        CSR_Vector3       normal;
        CSR_Vector2       uv;
        CSR_Mesh*         pMesh;
        CSR_VertexBuffer* pVB;
    #endif

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // calculate the slice step
    step = (float)(2.0 * M_PI) / (float)slices;
    z    =  0.0f;

    // iterate through spiral stacks to create
    for (i = 0; i < stacks; ++i)
    {
        // create a new vertex buffer to contain the next slice
        pVB = (CSR_VertexBuffer*)csrMemoryAlloc(pMesh->m_pVB,
                                                sizeof(CSR_VertexBuffer),
                                                pMesh->m_Count + 1);

        // succeeded?
        if (!pVB)
        {
            csrMeshRelease(pMesh, 0);
            return 0;
        }

        // update the mesh
        pMesh->m_pVB = pVB;
        ++pMesh->m_Count;

        // get the vertex buffer index
        index = pMesh->m_Count - 1;

        // initialize the newly created vertex buffer
        csrVertexBufferInit(&pMesh->m_pVB[index]);

        // apply the user wished vertex format
        if (pVertFormat)
            pMesh->m_pVB[index].m_Format = *pVertFormat;

        // apply the user wished vertex culling
        if (pVertCulling)
            pMesh->m_pVB[index].m_Culling = *pVertCulling;
        else
        {
            // otherwise configure the default culling
            pMesh->m_pVB[index].m_Culling.m_Type = CSR_CT_None;
            pMesh->m_pVB[index].m_Culling.m_Face = CSR_CF_CW;
        }

        // apply the user wished material
        if (pMaterial)
            pMesh->m_pVB[index].m_Material = *pMaterial;

        // set the vertex format type
        pMesh->m_pVB[index].m_Format.m_Type = CSR_VT_TriangleStrip;

        // calculate the stride
        csrVertexFormatCalculateStride(&pMesh->m_pVB[index].m_Format);

        // iterate through spiral slices to create
        for (j = 0; j <= slices; ++j)
        {
            // calculate the current slice angle
            angle = step * (float)j;

            // calculate the slice min point
            xA = centerX + minRadius * cosf(angle);
            yA = centerY + minRadius * sinf(angle);

            // calculate the slice max point
            xB = centerX + maxRadius * cosf(angle);
            yB = centerY + maxRadius * sinf(angle);

            // calculate the spiral curve
            minRadius += deltaMin;
            maxRadius += deltaMax;

            // calculate the z position
            z -= deltaZ;

            // calculate texture u coordinate
            if (!j)
                texU = 0.0f;
            else
            if (j == slices)
                texU = 1.0f;
            else
                texU = (float)j / (float)slices;

            // add min point in buffer
            vertex.m_X = xA;
            vertex.m_Y = yA;
            vertex.m_Z = z;

            // vertex has a normal?
            if (pMesh->m_pVB[index].m_Format.m_HasNormal)
            {
                // set normal data
                normal.m_X = 0.0f;
                normal.m_Y = 0.0f;
                normal.m_Z = 1.0f;
            }

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[index].m_Format.m_HasTexCoords)
            {
                // set texture data
                uv.m_X = texU;
                uv.m_Y = 0.0f;
            }

            // add the vertex to the buffer
            csrVertexBufferAdd(&vertex,
                               &normal,
                               &uv,
                                j * 2,
                                fOnGetVertexColor,
                               &pMesh->m_pVB[index]);

            // add max point in the buffer
            vertex.m_X = xB;
            vertex.m_Y = yB;
            vertex.m_Z = z;

            // vertex has a normal?
            if (pMesh->m_pVB[index].m_Format.m_HasNormal)
            {
                // set normal data
                normal.m_X = 0.0f;
                normal.m_Y = 0.0f;
                normal.m_Z = 1.0f;
            }

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[index].m_Format.m_HasTexCoords)
            {
                // set texture data
                uv.m_X = texU;
                uv.m_Y = 1.0f;
            }

            // add the vertex to the buffer
            csrVertexBufferAdd(&vertex,
                               &normal,
                               &uv,
                               (j * 2) + 1,
                                fOnGetVertexColor,
                               &pMesh->m_pVB[index]);
        }

        // correct the last values otherwise the spiral will appears broken
        minRadius -= deltaMin;
        maxRadius -= deltaMax;
        z         += deltaZ;
    }

    return pMesh;
}
//---------------------------------------------------------------------------
// Skybox functions
//---------------------------------------------------------------------------
CSR_Mesh* csrSkyboxCreate(float width, float height, float depth)
{
    #ifdef _MSC_VER
        CSR_VertexCulling vc = {0};
    #else
        CSR_VertexCulling vc;
    #endif
    #ifdef CSR_USE_METAL
        CSR_VertexFormat vf;
    #endif

    // configure the vertex culling to use with the skybox
    vc.m_Type = CSR_CT_Back;
    vc.m_Face = CSR_CF_CW;

    #ifdef CSR_USE_METAL
        // configure the vertex format
        csrVertexFormatInit(&vf);
        vf.m_HasNormal         = 0;
        vf.m_HasTexCoords      = 1;
        vf.m_HasPerVertexColor = 1;

        // create the skybox geometry
        return csrShapeCreateBox(width, height, depth, 0, &vf, &vc, 0, 0);
    #else
        // create the skybox geometry
        return csrShapeCreateBox(width, height, depth, 0, 0, &vc, 0, 0);
    #endif
}
//---------------------------------------------------------------------------
// Bone functions
//---------------------------------------------------------------------------
CSR_Bone* csrBoneCreate(void)
{
    // create a new bone
    CSR_Bone* pBone = (CSR_Bone*)malloc(sizeof(CSR_Bone));

    // succeeded?
    if (!pBone)
        return 0;

    // initialize the bone content
    csrBoneInit(pBone);

    return pBone;
}
//---------------------------------------------------------------------------
void csrBoneRelease(CSR_Bone* pBone, int contentOnly, int releaseCustomData)
{
    size_t i;

    // no bone to release?
    if (!pBone)
        return;

    // free the bone children content
    for (i = 0; i < pBone->m_ChildrenCount; ++i)
        csrBoneRelease(&pBone->m_pChildren[i], 1, releaseCustomData);

    // free the bone children
    if (pBone->m_pChildren)
        free(pBone->m_pChildren);

    // release the name
    if (pBone->m_pName)
        free(pBone->m_pName);

    // release the custom data
    if (releaseCustomData)
        free(pBone->m_pCustomData);

    // release the bone itself
    if (!contentOnly)
        free(pBone);
}
//---------------------------------------------------------------------------
void csrBoneInit(CSR_Bone* pBone)
{
    // no bone to initialize?
    if (!pBone)
        return;

    // initialize the bone content
    pBone->m_pName         = 0;
    pBone->m_pParent       = 0;
    pBone->m_pChildren     = 0;
    pBone->m_ChildrenCount = 0;
    pBone->m_pCustomData   = 0;

    // initialize the bone matrix
    csrMat4Identity(&pBone->m_Matrix);
}
//---------------------------------------------------------------------------
CSR_Bone* csrBoneFind(const CSR_Bone* pBone, const char* pName)
{
    size_t i;

    // no bone?
    if (!pBone)
        return 0;

    // found the bone?
    if (pBone->m_pName && strcmp(pBone->m_pName, pName) == 0)
        return (CSR_Bone*)pBone;

    // iterate through the bone children
    for (i = 0; i < pBone->m_ChildrenCount; ++i)
    {
        // search in next children bone
        CSR_Bone* pChildBone = csrBoneFind(&pBone->m_pChildren[i], pName);

        // found the bone?
        if (pChildBone)
            return pChildBone;
    }

    return 0;
}
//---------------------------------------------------------------------------
void csrBoneGetMatrix(const CSR_Bone* pBone, CSR_Matrix4* pInitialMatrix, CSR_Matrix4* pMatrix)
{
    CSR_Matrix4 localMatrix;

    // no bone?
    if (!pBone)
        return;

    // no output matrix to write to?
    if (!pMatrix)
        return;

    // set the output matrix as identity
    csrMat4Identity(pMatrix);

    // iterate through bones
    while (pBone)
    {
        // get the previously stacked matrix as base to calculate the new one
        localMatrix = *pMatrix;

        // stack the previously calculated matrix with the current bone one
        csrMat4Multiply(&localMatrix, &pBone->m_Matrix, pMatrix);

        // go to parent bone
        pBone = pBone->m_pParent;
    }

    // initial matrix provided?
    if (pInitialMatrix)
    {
        // get the previously stacked matrix as base to calculate the new one
        localMatrix = *pMatrix;

        // stack the previously calculated matrix with the initial one
        csrMat4Multiply(&localMatrix, pInitialMatrix, pMatrix);
    }
}
//---------------------------------------------------------------------------
void csrBoneGetAnimMatrix(const CSR_Bone*         pBone,
                          const CSR_AnimationSet* pAnimSet,
                                size_t            frameIndex,
                                CSR_Matrix4*      pInitialMatrix,
                                CSR_Matrix4*      pMatrix)
{
    CSR_Matrix4 localMatrix;
    CSR_Matrix4 animMatrix;

    // no bone?
    if (!pBone)
        return;

    // no output matrix to write to?
    if (!pMatrix)
        return;

    // set the output matrix as identity
    csrMat4Identity(pMatrix);

    // iterate through bones
    while (pBone)
    {
        // get the previously stacked matrix as base to calculate the new one
        localMatrix = *pMatrix;

        // get the animated bone matrix matching with frame. If not found use the identity one
        if (!csrAnimationGetMatrix(pAnimSet, pBone, frameIndex, &animMatrix))
            csrMat4Identity(&animMatrix);

        // stack the previously calculated matrix with the current bone one
        csrMat4Multiply(&localMatrix, &animMatrix, pMatrix);

        // go to parent bone
        pBone = pBone->m_pParent;
    }

    // initial matrix provided?
    if (pInitialMatrix)
    {
        // get the previously stacked matrix as base to calculate the new one
        localMatrix = *pMatrix;

        // stack the previously calculated matrix with the initial one
        csrMat4Multiply(&localMatrix, pInitialMatrix, pMatrix);
    }
}
//---------------------------------------------------------------------------
// Skin weights functions
//---------------------------------------------------------------------------
CSR_Skin_Weights* csrSkinWeightsCreate(void)
{
    // create a new skin weights structure
    CSR_Skin_Weights* pSkinWeights = (CSR_Skin_Weights*)malloc(sizeof(CSR_Skin_Weights));

    // succeeded?
    if (!pSkinWeights)
        return 0;

    // initialize the skin weights content
    csrSkinWeightsInit(pSkinWeights);

    return pSkinWeights;
}
//---------------------------------------------------------------------------
void csrSkinWeightsRelease(CSR_Skin_Weights* pSkinWeights, int contentOnly)
{
    size_t i;

    // no skin weights to release?
    if (!pSkinWeights)
        return;

    // release the bone name
    if (pSkinWeights->m_pBoneName)
        free(pSkinWeights->m_pBoneName);

    // release the weight index table
    if (pSkinWeights->m_pIndexTable)
    {
        // release the weight index table content
        for (i = 0; i < pSkinWeights->m_IndexTableCount; ++i)
            // release the weight table data
            free(pSkinWeights->m_pIndexTable[i].m_pData);

        // free the weight index table
        free(pSkinWeights->m_pIndexTable);
    }

    // release the weight table
    if (pSkinWeights->m_pWeights)
        free(pSkinWeights->m_pWeights);

    // free the skin weights
    if (!contentOnly)
        free(pSkinWeights);
}
//---------------------------------------------------------------------------
void csrSkinWeightsInit(CSR_Skin_Weights* pSkinWeights)
{
    // no skin weights to initialize?
    if (!pSkinWeights)
        return;

    // initialize the skin weights content
    pSkinWeights->m_pBoneName       = 0;
    pSkinWeights->m_pBone           = 0;
    pSkinWeights->m_MeshIndex       = 0;
    pSkinWeights->m_pIndexTable     = 0;
    pSkinWeights->m_IndexTableCount = 0;
    pSkinWeights->m_pWeights        = 0;
    pSkinWeights->m_WeightCount     = 0;

    // initialize the matrix to convert the vertices to bone space
    csrMat4Identity(&pSkinWeights->m_Matrix);
}
//---------------------------------------------------------------------------
// Animation key functions
//---------------------------------------------------------------------------
CSR_AnimationKey* csrAnimationKeyCreate(void)
{
    // create a new animation key structure
    CSR_AnimationKey* pAnimationKey = (CSR_AnimationKey*)malloc(sizeof(CSR_AnimationKey));

    // succeeded?
    if (!pAnimationKey)
        return 0;

    // initialize the animation key content
    csrAnimationKeyInit(pAnimationKey);

    return pAnimationKey;
}
//---------------------------------------------------------------------------
void csrAnimationKeyRelease(CSR_AnimationKey* pAnimationKey, int contentOnly)
{
    // no animation key to release?
    if (!pAnimationKey)
        return;

    // release the values
    if (pAnimationKey->m_pValues)
        free(pAnimationKey->m_pValues);

    // free the animation key
    if (!contentOnly)
        free(pAnimationKey);
}
//---------------------------------------------------------------------------
void csrAnimationKeyInit(CSR_AnimationKey* pAnimationKey)
{
    // no animation key to initialize?
    if (!pAnimationKey)
        return;

    // initialize the animation keys content
    pAnimationKey->m_Frame   = 0;
    pAnimationKey->m_pValues = 0;
    pAnimationKey->m_Count   = 0;
}
//---------------------------------------------------------------------------
// Animation keys functions
//---------------------------------------------------------------------------
CSR_AnimationKeys* csrAnimationKeysCreate(void)
{
    // create a new animation keys structure
    CSR_AnimationKeys* pAnimationKeys = (CSR_AnimationKeys*)malloc(sizeof(CSR_AnimationKeys));

    // succeeded?
    if (!pAnimationKeys)
        return 0;

    // initialize the animation keys content
    csrAnimationKeysInit(pAnimationKeys);

    return pAnimationKeys;
}
//---------------------------------------------------------------------------
void csrAnimationKeysRelease(CSR_AnimationKeys* pAnimationKeys, int contentOnly)
{
    size_t i;

    // no animation keys to release?
    if (!pAnimationKeys)
        return;

    // release the keys
    if (pAnimationKeys->m_pKey)
    {
        // release the keys content
        for (i = 0; i < pAnimationKeys->m_Count; ++i)
            csrAnimationKeyRelease(&pAnimationKeys->m_pKey[i], 1);

        // free the keys container
        free(pAnimationKeys->m_pKey);
    }

    // free the animation keys
    if (!contentOnly)
        free(pAnimationKeys);
}
//---------------------------------------------------------------------------
void csrAnimationKeysInit(CSR_AnimationKeys* pAnimationKeys)
{
    // no animation keys to initialize?
    if (!pAnimationKeys)
        return;

    // initialize the animation keys content
    pAnimationKeys->m_Type  = CSR_KT_Unknown;
    pAnimationKeys->m_pKey  = 0;
    pAnimationKeys->m_Count = 0;
}
//---------------------------------------------------------------------------
// Animation functions
//---------------------------------------------------------------------------
CSR_Animation* csrAnimationCreate(void)
{
    // create a new animation structure
    CSR_Animation* pAnimation = (CSR_Animation*)malloc(sizeof(CSR_Animation));

    // succeeded?
    if (!pAnimation)
        return 0;

    // initialize the animation content
    csrAnimationInit(pAnimation);

    return pAnimation;
}
//---------------------------------------------------------------------------
void csrAnimationRelease(CSR_Animation* pAnimation, int contentOnly)
{
    size_t i;

    // no animation to release?
    if (!pAnimation)
        return;

    // release the bone name
    if (pAnimation->m_pBoneName)
        free(pAnimation->m_pBoneName);

    // release the keys
    if (pAnimation->m_pKeys)
    {
        // release the keys content
        for (i = 0; i < pAnimation->m_Count; ++i)
            csrAnimationKeysRelease(&pAnimation->m_pKeys[i], 1);

        // free the keys container
        free(pAnimation->m_pKeys);
    }

    // free the animation
    if (!contentOnly)
        free(pAnimation);
}
//---------------------------------------------------------------------------
void csrAnimationInit(CSR_Animation* pAnimation)
{
    // no animation to initialize?
    if (!pAnimation)
        return;

    // initialize the animation content
    pAnimation->m_pBoneName = 0;
    pAnimation->m_pBone     = 0;
    pAnimation->m_pKeys     = 0;
    pAnimation->m_Count     = 0;
}
//---------------------------------------------------------------------------
int csrAnimationGetMatrix(const CSR_AnimationSet* pAnimSet,
                          const CSR_Bone*         pBone,
                                size_t            frame,
                                CSR_Matrix4*      pMatrix)
{
    size_t i;
    size_t j;
    size_t k;

    // no animation set?
    if (!pAnimSet)
        return 0;

    // no bone?
    if (!pBone)
        return 0;

    // no output matrix?
    if (!pMatrix)
        return 0;

    // iterate through animations
    for (i = 0; i < pAnimSet->m_Count; ++i)
    {
        #ifdef _MSC_VER
            size_t         rotFrame;
            size_t         nextRotFrame;
            size_t         posFrame;
            size_t         nextPosFrame;
            size_t         scaleFrame;
            size_t         nextScaleFrame;
            float          frameDelta;
            float          frameLength;
            float          interpolation;
            CSR_Quaternion rotation        = {0};
            CSR_Quaternion nextRotation    = {0};
            CSR_Quaternion finalRotation   = {0};
            CSR_Vector3    position        = {0};
            CSR_Vector3    nextPosition    = {0};
            CSR_Vector3    finalPosition   = {0};
            CSR_Vector3    scaling         = {0};
            CSR_Vector3    nextScaling     = {0};
            CSR_Vector3    finalScaling    = {0};
            CSR_Matrix4    scaleMatrix     = {0};
            CSR_Matrix4    rotateMatrix    = {0};
            CSR_Matrix4    translateMatrix = {0};
            CSR_Matrix4    buildMatrix     = {0};
        #else
            size_t         rotFrame;
            size_t         nextRotFrame;
            size_t         posFrame;
            size_t         nextPosFrame;
            size_t         scaleFrame;
            size_t         nextScaleFrame;
            float          frameDelta;
            float          frameLength;
            float          interpolation;
            CSR_Quaternion rotation;
            CSR_Quaternion nextRotation;
            CSR_Quaternion finalRotation;
            CSR_Vector3    position;
            CSR_Vector3    nextPosition;
            CSR_Vector3    finalPosition;
            CSR_Vector3    scaling;
            CSR_Vector3    nextScaling;
            CSR_Vector3    finalScaling;
            CSR_Matrix4    scaleMatrix;
            CSR_Matrix4    rotateMatrix;
            CSR_Matrix4    translateMatrix;
            CSR_Matrix4    buildMatrix;
        #endif

        // found the animation matching with the bone for which the matrix should be get?
        if (pAnimSet->m_pAnimation[i].m_pBone != pBone)
            continue;

        rotFrame       = 0;
        nextRotFrame   = 0;
        posFrame       = 0;
        nextPosFrame   = 0;
        scaleFrame     = 0;
        nextScaleFrame = 0;

        // iterate through animation keys
        for (j = 0; j < pAnimSet->m_pAnimation[i].m_Count; ++j)
        {
            size_t keyIndex = 0;

            // iterate through animation key items
            for (k = 0; k < pAnimSet->m_pAnimation[i].m_pKeys[j].m_Count; ++k)
                if (frame >= pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[k].m_Frame)
                    keyIndex = k;
                else
                    break;

            // search for keys type
            switch (pAnimSet->m_pAnimation[i].m_pKeys[j].m_Type)
            {
                case CSR_KT_Rotation:
                    if (pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_Count != 4)
                        return 0;

                    // get the rotation quaternion at index
                    rotation.m_W = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[0];
                    rotation.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[1];
                    rotation.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[2];
                    rotation.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[3];
                    rotFrame     = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_Frame;

                    // get the next rotation quaternion
                    if (keyIndex + 1 >= pAnimSet->m_pAnimation[i].m_pKeys[j].m_Count)
                    {
                        nextRotation.m_W = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[0];
                        nextRotation.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[1];
                        nextRotation.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[2];
                        nextRotation.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[3];
                        nextRotFrame     = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_Frame;
                    }
                    else
                    {
                        nextRotation.m_W = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[0];
                        nextRotation.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[1];
                        nextRotation.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[2];
                        nextRotation.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[3];
                        nextRotFrame     = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_Frame;
                    }

                    continue;

                case CSR_KT_Scale:
                    if (pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_Count != 3)
                        return 0;

                    // get the scale values at index
                    scaling.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[0];
                    scaling.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[1];
                    scaling.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[2];
                    scaleFrame  = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_Frame;

                    // get the next rotation quaternion
                    if (keyIndex + 1 >= pAnimSet->m_pAnimation[i].m_pKeys[j].m_Count)
                    {
                        nextScaling.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[0];
                        nextScaling.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[1];
                        nextScaling.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[2];
                        nextScaleFrame  = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_Frame;
                    }
                    else
                    {
                        nextScaling.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[0];
                        nextScaling.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[1];
                        nextScaling.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[2];
                        nextScaleFrame  = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_Frame;
                    }

                    continue;

                case CSR_KT_Position:
                    if (pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_Count != 3)
                        return 0;

                    // get the position values at index
                    position.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[0];
                    position.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[1];
                    position.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[2];
                    posFrame     = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_Frame;

                    // get the next rotation quaternion
                    if (keyIndex + 1 >= pAnimSet->m_pAnimation[i].m_pKeys[j].m_Count)
                    {
                        nextPosition.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[0];
                        nextPosition.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[1];
                        nextPosition.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_pValues[2];
                        nextPosFrame     = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[0].m_Frame;
                    }
                    else
                    {
                        nextPosition.m_X = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[0];
                        nextPosition.m_Y = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[1];
                        nextPosition.m_Z = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_pValues[2];
                        nextPosFrame     = pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex + 1].m_Frame;
                    }

                    continue;

                case CSR_KT_MatrixKeys:
                    if (pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_Count != 16)
                        return 0;

                    // get the key matrix
                    for (k = 0; k < 16; ++k)
                        pMatrix->m_Table[k / 4][k % 4] =
                                pAnimSet->m_pAnimation[i].m_pKeys[j].m_pKey[keyIndex].m_pValues[k];

                    return 1;

                default:
                    continue;
            }
        }

        // calculate the frame delta, the frame length and the interpolation for the rotation
        frameDelta    = (float)(frame        - rotFrame);
        frameLength   = (float)(nextRotFrame - rotFrame);
        interpolation = frameDelta / frameLength;

        // interpolate the rotation
        csrQuatSlerp(&rotation, &nextRotation, interpolation, &finalRotation);

        // calculate the frame delta, the frame length and the interpolation for the scaling
        frameDelta    = (float)(frame          - scaleFrame);
        frameLength   = (float)(nextScaleFrame - scaleFrame);
        interpolation = frameDelta / frameLength;

        // interpolate the scaling
        finalScaling.m_X = scaling.m_X + ((nextScaling.m_X - scaling.m_X) * interpolation);
        finalScaling.m_Y = scaling.m_Y + ((nextScaling.m_Y - scaling.m_Y) * interpolation);
        finalScaling.m_Z = scaling.m_Z + ((nextScaling.m_Z - scaling.m_Z) * interpolation);

        // calculate the frame delta, the frame length and the interpolation for the rotation
        frameDelta    = (float)(frame        - posFrame);
        frameLength   = (float)(nextPosFrame - posFrame);
        interpolation = frameDelta / frameLength;

        // interpolate the position
        finalPosition.m_X = position.m_X + ((nextPosition.m_X - position.m_X) * interpolation);
        finalPosition.m_Y = position.m_Y + ((nextPosition.m_Y - position.m_Y) * interpolation);
        finalPosition.m_Z = position.m_Z + ((nextPosition.m_Z - position.m_Z) * interpolation);

        // get the rotation quaternion and the scale and translate vectors
        csrMat4Scale(&finalScaling, &scaleMatrix);
        csrQuatToMatrix(&finalRotation, &rotateMatrix);
        csrMat4Translate(&finalPosition, &translateMatrix);

        // build the final matrix
        csrMat4Multiply(&scaleMatrix, &rotateMatrix,    &buildMatrix);
        csrMat4Multiply(&buildMatrix, &translateMatrix, pMatrix);

        return 1;
    }

    return 0;
}
//---------------------------------------------------------------------------
// Animation set functions
//---------------------------------------------------------------------------
CSR_AnimationSet* csrAnimationSetCreate(void)
{
    // create a new animation set structure
    CSR_AnimationSet* pAnimationSet = (CSR_AnimationSet*)malloc(sizeof(CSR_AnimationSet));

    // succeeded?
    if (!pAnimationSet)
        return 0;

    // initialize the animation set content
    csrAnimationSetInit(pAnimationSet);

    return pAnimationSet;
}
//---------------------------------------------------------------------------
void csrAnimationSetRelease(CSR_AnimationSet* pAnimationSet, int contentOnly)
{
    size_t i;

    // no animation set to release?
    if (!pAnimationSet)
        return;

    // release the animation
    if (pAnimationSet->m_pAnimation)
    {
        // release the animation content
        for (i = 0; i < pAnimationSet->m_Count; ++i)
            csrAnimationRelease(&pAnimationSet->m_pAnimation[i], 1);

        // free the animation container
        free(pAnimationSet->m_pAnimation);
    }

    // free the animation set
    if (!contentOnly)
        free(pAnimationSet);
}
//---------------------------------------------------------------------------
void csrAnimationSetInit(CSR_AnimationSet* pAnimationSet)
{
    // no animation set to initialize?
    if (!pAnimationSet)
        return;

    // initialize the animation set content
    pAnimationSet->m_pAnimation = 0;
    pAnimationSet->m_Count      = 0;
}
//---------------------------------------------------------------------------
// Model functions
//---------------------------------------------------------------------------
CSR_Model* csrModelCreate(void)
{
    // create a new model
    CSR_Model* pModel = (CSR_Model*)malloc(sizeof(CSR_Model));

    // succeeded?
    if (!pModel)
        return 0;

    // initialize the model content
    csrModelInit(pModel);

    return pModel;
}
//---------------------------------------------------------------------------
void csrModelRelease(CSR_Model* pModel, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t i;
    size_t j;

    // no model to release?
    if (!pModel)
        return;

    // do free the meshes content?
    if (pModel->m_pMesh)
    {
        // iterate through meshes to free
        for (i = 0; i < pModel->m_MeshCount; ++i)
        {
            // delete the skin
            csrSkinContentRelease(&pModel->m_pMesh[i].m_Skin, fOnDeleteTexture);

            // do free the mesh vertex buffer?
            if (pModel->m_pMesh[i].m_pVB)
            {
                // free the mesh vertex buffer content
                for (j = 0; j < pModel->m_pMesh[i].m_Count; ++j)
                    if (pModel->m_pMesh[i].m_pVB[j].m_pData)
                        free(pModel->m_pMesh[i].m_pVB[j].m_pData);

                // free the mesh vertex buffer
                free(pModel->m_pMesh[i].m_pVB);
            }
        }

        // free the meshes
        free(pModel->m_pMesh);
    }

    // free the model
    free(pModel);
}
//---------------------------------------------------------------------------
void csrModelInit(CSR_Model* pModel)
{
    // no model to initialize?
    if (!pModel)
        return;

    // initialize the model content
    pModel->m_pMesh     = 0;
    pModel->m_MeshCount = 0;
    pModel->m_Time      = 0.0;
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
    CSR_ModelAnimation*  pAnimation;
    char                 skinName[16];
    char                 prevSkinName[16];
    unsigned             animationStartIndex;
    unsigned             skinNameIndex;
    const unsigned       skinNameLength = sizeof(skinName);
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
    if (!pVertFormat || pVertFormat->m_HasTexCoords)
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
            if (i == pHeader->m_FrameCount - 1 || memcmp(skinName, prevSkinName, skinNameLength) != 0)
            {
                // increase the memory to contain the new animation
                pAnimation = (CSR_ModelAnimation*)csrMemoryAlloc(pMDL->m_pAnimation,
                                                                 sizeof(CSR_ModelAnimation),
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
void csrMDLInit(CSR_MDL* pMDL)
{
    // no MDL model to initialize?
    if (!pMDL)
        return;

    // initialize the MDL model content
    pMDL->m_pModel         = 0;
    pMDL->m_ModelCount     = 0;
    pMDL->m_pAnimation     = 0;
    pMDL->m_AnimationCount = 0;
    pMDL->m_pSkin          = 0;
    pMDL->m_SkinCount      = 0;
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
            // swap the readed values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
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
            // swap the readed value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
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
            // swap the readed value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
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
    pSkin->m_pData = (unsigned char*)malloc(pSkin->m_TexLen * pSkin->m_Count);

    // read texture from buffer. NOTE 8 bit array, same in all endianness
    return csrBufferRead(pBuffer, pOffset, pSkin->m_TexLen , pSkin->m_Count, pSkin->m_pData);
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
            // swap the readed values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
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
            // swap the readed values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
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
            // swap the readed value in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            csrMemorySwap(&pFrameGroup->m_Type, sizeof(unsigned));
    #endif

    // is a single frame or a group of frames?
    if (!pFrameGroup->m_Type)
    {
        pFrameGroup->m_Count = 1;

        // create frame and time buffers
        pFrameGroup->m_pFrame = (CSR_MDLFrame*)malloc(sizeof(CSR_MDLFrame) * pFrameGroup->m_Count);
        pFrameGroup->m_pTime  = (float*)       malloc(sizeof(float)        * pFrameGroup->m_Count);

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
    pFrameGroup->m_pTime  = (float*)       malloc(sizeof(float)        * pFrameGroup->m_Count);

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
        ((unsigned char*)pPB->m_pData)[(i * bpp)]     = pTexPal[pSkin->m_pData[offset + i] * bpp];
        ((unsigned char*)pPB->m_pData)[(i * bpp) + 1] = pTexPal[pSkin->m_pData[offset + i] * bpp + 1];
        ((unsigned char*)pPB->m_pData)[(i * bpp) + 2] = pTexPal[pSkin->m_pData[offset + i] * bpp + 2];
    }

    return pPB;
}
//---------------------------------------------------------------------------
void csrMDLUncompressVertex(const CSR_MDLHeader* pHeader,
                            const CSR_MDLVertex* pVertex,
                                  CSR_Vector3*   pResult)
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

    // no model to populate?
    if (!pModel)
        return;

    // initialize the model and create all the meshes required to contain the MDL group frames
    pModel->m_MeshCount =  pFrameGroup->m_Count;
    pModel->m_pMesh     = (CSR_Mesh*)malloc(pFrameGroup->m_Count * sizeof(CSR_Mesh));
    pModel->m_Time      =  0.0;

    // succeeded?
    if (!pModel->m_pMesh)
        return;

    // iterate through sub-frames contained in group
    for (i = 0; i < pFrameGroup->m_Count; ++i)
    {
        // create the vertex buffers required for the sub-frames
        pModel->m_pMesh[i].m_Count =  1;
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
            lastKnownTime             = pFrameGroup->m_pTime[i];
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
                normal.m_X  = g_NormalTable[pSrcVertex->m_NormalIndex];
                normal.m_Y  = g_NormalTable[pSrcVertex->m_NormalIndex + 1];
                normal.m_Z  = g_NormalTable[pSrcVertex->m_NormalIndex + 2];

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
// Landscape creation functions
//---------------------------------------------------------------------------
int csrLandscapeGenerateVertices(const CSR_PixelBuffer* pPixelBuffer,
                                       float            height,
                                       float            scale,
                                       CSR_Buffer*      pVertices)
{
    size_t x;
    size_t z;
    float  scaleX;
    float  scaleZ;

    // validate the inputs
    if (!pPixelBuffer || height <= 0.0f || scale == 0.0f || !pVertices)
        return 0;

    // calculate landscape data size and reserve memory for landscape mesh
    pVertices->m_Length = pPixelBuffer->m_Width * pPixelBuffer->m_Height;
    pVertices->m_pData  = malloc(pVertices->m_Length * sizeof(CSR_Vector3));

    // calculate scaling factor on x and z axis
    scaleX = -(((pPixelBuffer->m_Width  - 1) * scale) / 2.0f);
    scaleZ =  (((pPixelBuffer->m_Height - 1) * scale) / 2.0f);

    // loop through height field points and calculate coordinates for each point
    for (z = 0; z < pPixelBuffer->m_Height; ++z)
        for (x = 0; x < pPixelBuffer->m_Width; ++x)
        {
            // calculate vertex index
            size_t index = (z * pPixelBuffer->m_Width) + x;
            float  value = (float)(((unsigned char*)pPixelBuffer->m_pData)[index * 3]) / 255.0f;

            // calculate landscape vertex
            ((CSR_Vector3*)pVertices->m_pData)[index].m_X = scaleX + ((float)x * scale);
            ((CSR_Vector3*)pVertices->m_pData)[index].m_Y = value  * height;
            ((CSR_Vector3*)pVertices->m_pData)[index].m_Z = scaleZ - ((float)z * scale);
        }

    return 1;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrLandscapeCreate(const CSR_PixelBuffer*      pPixelBuffer,
                                   float                 height,
                                   float                 scale,
                             const CSR_VertexFormat*     pVertFormat,
                             const CSR_VertexCulling*    pVertCulling,
                             const CSR_Material*         pMaterial,
                             const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        CSR_Mesh*   pMesh;
        CSR_Buffer  vertices;
        unsigned    x;
        unsigned    z;
        CSR_Vector3 v1  = {0};
        CSR_Vector3 v2  = {0};
        CSR_Vector3 v3  = {0};
        CSR_Vector3 v4  = {0};
        CSR_Vector3 n1  = {0};
        CSR_Vector3 n2  = {0};
        CSR_Vector2 uv1 = {0};
        CSR_Vector2 uv2 = {0};
        CSR_Vector2 uv3 = {0};
        CSR_Vector2 uv4 = {0};
        CSR_Plane   p1  = {0};
        CSR_Plane   p2  = {0};
    #else
        CSR_Mesh*   pMesh;
        CSR_Buffer  vertices;
        unsigned    x;
        unsigned    z;
        CSR_Vector3 v1;
        CSR_Vector3 v2;
        CSR_Vector3 v3;
        CSR_Vector3 v4;
        CSR_Vector3 n1;
        CSR_Vector3 n2;
        CSR_Vector2 uv1;
        CSR_Vector2 uv2;
        CSR_Vector2 uv3;
        CSR_Vector2 uv4;
        CSR_Plane   p1;
        CSR_Plane   p2;
    #endif

    // validate the inputs
    if (!pPixelBuffer || height <= 0.0f || scale == 0.0f)
        return 0;

    // create a mesh to contain the landscape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a new vertex buffer to contain the landscape
    pMesh->m_Count = 1;
    pMesh->m_pVB   = (CSR_VertexBuffer*)malloc(sizeof(CSR_VertexBuffer));

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh, 0);
        return 0;
    }

    // initialize the newly created vertex buffer
    csrVertexBufferInit(pMesh->m_pVB);

    // apply the user wished vertex format
    if (pVertFormat)
        pMesh->m_pVB->m_Format = *pVertFormat;

    // apply the user wished vertex culling
    if (pVertCulling)
        pMesh->m_pVB->m_Culling = *pVertCulling;
    else
    {
        // otherwise configure the default culling
        pMesh->m_pVB->m_Culling.m_Type = CSR_CT_None;
        pMesh->m_pVB->m_Culling.m_Face = CSR_CF_CW;
    }

    // apply the user wished material
    if (pMaterial)
        pMesh->m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_Triangles;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // generate landscape XYZ vertex from grayscale image
    if (!csrLandscapeGenerateVertices(pPixelBuffer, height, scale, &vertices))
    {
        csrMeshRelease(pMesh, 0);
        return 0;
    }

    // loop through landscape XYZ vertices and generate mesh polygons
    for (z = 0; z < pPixelBuffer->m_Height - 1; ++z)
        for (x = 0; x < pPixelBuffer->m_Width - 1; ++x)
        {
            // polygons are generated in the following order:
            // v1 -- v2
            //     /
            //    /
            // v3 -- v4

            unsigned index;
            unsigned i1;
            unsigned i2;
            unsigned i3;
            unsigned i4;

            // calculate vertex index
            index = (z * pPixelBuffer->m_Width) + x;

            // calculate first vertex
            v1.m_X = ((CSR_Vector3*)vertices.m_pData)[index].m_X;
            v1.m_Y = ((CSR_Vector3*)vertices.m_pData)[index].m_Y;
            v1.m_Z = ((CSR_Vector3*)vertices.m_pData)[index].m_Z;

            // calculate second vertex
            v2.m_X = ((CSR_Vector3*)vertices.m_pData)[index + 1].m_X;
            v2.m_Y = ((CSR_Vector3*)vertices.m_pData)[index + 1].m_Y;
            v2.m_Z = ((CSR_Vector3*)vertices.m_pData)[index + 1].m_Z;

            i1 = index;
            i2 = index + 1;

            // calculate next vertex index
            index = ((z + 1) * pPixelBuffer->m_Width) + x;

            // calculate third vertex
            v3.m_X = ((CSR_Vector3*)vertices.m_pData)[index].m_X;
            v3.m_Y = ((CSR_Vector3*)vertices.m_pData)[index].m_Y;
            v3.m_Z = ((CSR_Vector3*)vertices.m_pData)[index].m_Z;

            // calculate fourth vertex
            v4.m_X = ((CSR_Vector3*)vertices.m_pData)[index + 1].m_X;
            v4.m_Y = ((CSR_Vector3*)vertices.m_pData)[index + 1].m_Y;
            v4.m_Z = ((CSR_Vector3*)vertices.m_pData)[index + 1].m_Z;

            i3 = index;
            i4 = index + 1;

            // do include normals?
            if (pMesh->m_pVB->m_Format.m_HasNormal)
            {
                // calculate polygons planes
                csrPlaneFromPoints(&v1, &v2, &v3, &p1);
                csrPlaneFromPoints(&v2, &v3, &v4, &p2);

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
            if (pMesh->m_pVB->m_Format.m_HasPerVertexColor)
            {
                // generate texture coordinates
                uv1.m_X = (float)(x)     / (float)(pPixelBuffer->m_Width);
                uv1.m_Y = (float)(z)     / (float)(pPixelBuffer->m_Height);
                uv2.m_X = (float)(x + 1) / (float)(pPixelBuffer->m_Width);
                uv2.m_Y = (float)(z)     / (float)(pPixelBuffer->m_Height);
                uv3.m_X = (float)(x)     / (float)(pPixelBuffer->m_Width);
                uv3.m_Y = (float)(z + 1) / (float)(pPixelBuffer->m_Height);
                uv4.m_X = (float)(x + 1) / (float)(pPixelBuffer->m_Width);
                uv4.m_Y = (float)(z + 1) / (float)(pPixelBuffer->m_Height);
            }

            // add first polygon first vertex to buffer
            csrVertexBufferAdd(&v1,
                               &n1,
                               &uv1,
                                i1,
                                fOnGetVertexColor,
                                pMesh->m_pVB);

            // add first polygon second vertex to buffer
            csrVertexBufferAdd(&v2,
                               &n1,
                               &uv2,
                                i2,
                                fOnGetVertexColor,
                                pMesh->m_pVB);

            // add first polygon third vertex to buffer
            csrVertexBufferAdd(&v3,
                               &n1,
                               &uv3,
                                i3,
                                fOnGetVertexColor,
                                pMesh->m_pVB);

            // add second polygon first vertex to buffer
            csrVertexBufferAdd(&v2,
                               &n2,
                               &uv2,
                                i2,
                                fOnGetVertexColor,
                                pMesh->m_pVB);

            // add second polygon second vertex to buffer
            csrVertexBufferAdd(&v3,
                               &n2,
                               &uv3,
                                i3,
                                fOnGetVertexColor,
                                pMesh->m_pVB);

            // add second polygon third vertex to buffer
            csrVertexBufferAdd(&v4,
                               &n2,
                               &uv4,
                                i4,
                                fOnGetVertexColor,
                                pMesh->m_pVB);
        }

    // delete landscape XYZ vertices (no longer used as copied in mesh)
    if (vertices.m_Length)
        free(vertices.m_pData);

    return pMesh;
}
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

    // get the text
    pText         = (char*)malloc(length + 1);
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

    // is word empty or comtains just 1 char?
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

    // return the material matching with the indice. NOTE assume that the material list object only
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
        CSR_Vector3* pNormal = 0;
        CSR_Vector2* pUV     = 0;
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
        // calculate the uv index from the indice table
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

        // get the material index to apply to this vertex
        if (pMatListDataset->m_MaterialCount == 1)
            materialIndex = pMatListDataset->m_pMaterialIndices[0];
        else
            materialIndex = pMatListDataset->m_pMaterialIndices[matListIndex];

        // get the material item
        pMaterialItem = csrXGetMaterial(pMatList, materialIndex);

        // succeeded?
        if (pMaterialItem && pMaterialItem->m_ID == CSR_XI_Material_ID)
        {
            // get the material dataset
            CSR_Dataset_Material_X* pMaterialDataset =
                    (CSR_Dataset_Material_X*)pMaterialItem->m_pData;

            // found it?
            if (pMaterialDataset)
            {
                // change the vertex color to match with the material one
                pX->m_pMesh[meshIndex].m_pVB->m_Material.m_Color =
                        csrColorToRGBA(&pMaterialDataset->m_Color);

                // do the same thing for the print
                if (!pX->m_MeshOnly)
                    pX->m_pPrint[meshIndex].m_Material.m_Color =
                            pX->m_pMesh[meshIndex].m_pVB->m_Material.m_Color;
            }
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

    // also add the new vertex to the print if mesh is animated
    if (!csrVertexBufferAdd(&vertex,
                             pNormal,
                             pUV,
                             0,
                             fOnGetVertexColor,
                            &pX->m_pPrint[meshIndex]))
        return 0;

    // reset the previous print vertex color
    pX->m_pPrint[meshIndex].m_Material.m_Color = prevColor;

    weightIndex = 0;

    // link the newly added vertice to the mesh skin weights
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
                        // allocate memory for the new indice to add
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
                                         length,
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

                // set the vertex tabe item count
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

    // is model supporting animations?
    if (!pX->m_MeshOnly)
    {
        // if the animation is used, also create the associated mesh print
        CSR_VertexBuffer* pPrint = (CSR_VertexBuffer*)csrMemoryAlloc(pX->m_pPrint,
                                                                     sizeof(CSR_VertexBuffer),
                                                                     pX->m_PrintCount + 1);

        // succeeded?
        if (!pPrint)
            return 0;

        // update the model mesh print data
        pX->m_pPrint = pPrint;
        ++pX->m_PrintCount;

        // initialize the vertex buffer for the mesh print
        csrVertexBufferInit(&pX->m_pPrint[index]);

        // copy the vertex format, culling and material from the source mesh
        pX->m_pPrint[index].m_Format   = pX->m_pMesh[index].m_pVB->m_Format;
        pX->m_pPrint[index].m_Culling  = pX->m_pMesh[index].m_pVB->m_Culling;
        pX->m_pPrint[index].m_Material = pX->m_pMesh[index].m_pVB->m_Material;
    }

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
                            strcpy_s(pX->m_pMesh[index].m_Skin.m_Texture.m_pFileName, length, pTextureDataset->m_pFileName);
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

    // iterate through indice table
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
    CSR_AnimationSet* pAnimationSet = (CSR_AnimationSet*)csrMemoryAlloc(pX->m_pAnimationSet,
                                                                        sizeof(CSR_AnimationSet),
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
    csrAnimationSetInit(&pX->m_pAnimationSet[index]);

    // iterate through source animations
    for (i = 0; i < pItem->m_ChildrenCount; ++i)
    {
        // allocate memory for a new animation
        CSR_Animation* pAnimation =
                (CSR_Animation*)csrMemoryAlloc(pX->m_pAnimationSet[index].m_pAnimation,
                                               sizeof(CSR_Animation),
                                               pX->m_pAnimationSet[index].m_Count + 1);

        // succeeded?
        if (!pAnimation)
            return 0;

        // update the model animations
        pX->m_pAnimationSet[index].m_pAnimation = pAnimation;
        ++pX->m_pAnimationSet[index].m_Count;

        // initialize the animation content
        csrAnimationInit(&pX->m_pAnimationSet[index].m_pAnimation[i]);

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
            csrAnimationKeysInit(&pX->m_pAnimationSet[index].m_pAnimation[i].m_pKeys[j]);

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

    // configure it
    pX->m_pMesh               = 0;
    pX->m_MeshCount           = 0;
    pX->m_pPrint              = 0;
    pX->m_PrintCount          = 0;
    pX->m_pMeshWeights        = 0;
    pX->m_MeshWeightsCount    = 0;
    pX->m_pMeshToBoneDict     = 0;
    pX->m_MeshToBoneDictCount = 0;
    pX->m_pSkeleton           = 0;
    pX->m_pAnimationSet       = 0;
    pX->m_AnimationSetCount   = 0;
    pX->m_MeshOnly            = meshOnly;
    pX->m_PoseOnly            = poseOnly;

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

    // release the print
    if (pX->m_pPrint)
    {
        // free the print content
        for (i = 0; i < pX->m_PrintCount; ++i)
            if (pX->m_pPrint[i].m_pData)
                free(pX->m_pPrint[i].m_pData);

        // free the print
        free(pX->m_pPrint);
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
            csrAnimationSetRelease(&pX->m_pAnimationSet[i], 1);

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

                        // do read the vertices or indice count, or a new indice?
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

                        // do read the skin weights item count, or a new indice?
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
                // then the skeletton should also be not initalized
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
                    strcpy_s(pCurrent->m_pName, length, pData->m_pName);
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
