/****************************************************************************
 * ==> CSR_Model -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the model functions and types         *
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

#include "CSR_Model.h"

// std
#include <stdlib.h>
#include <math.h>

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
CSR_Mesh* csrShapeCreateSurface(      float                 width,
                                      float                 height,
                                const CSR_VertexFormat*     pVertFormat,
                                const CSR_VertexCulling*    pVertCulling,
                                const CSR_Material*         pMaterial,
                                const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    int         i;
    int         index;
    CSR_Mesh*   pMesh;
    CSR_Vector3 vertex;
    CSR_Vector3 normal;
    CSR_Vector2 uv;

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
    pMesh->m_Count = 1;
    pMesh->m_pVB   = csrVertexBufferCreate();

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh);
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
CSR_Mesh* csrShapeCreateBox(      float                 width,
                                  float                 height,
                                  float                 depth,
                                  int                   repeatTexOnEachFace,
                            const CSR_VertexFormat*     pVertFormat,
                            const CSR_VertexCulling*    pVertCulling,
                            const CSR_Material*         pMaterial,
                            const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    unsigned    color;
    size_t      i;
    CSR_Vector3 vertices[8];
    CSR_Vector3 normals[6];
    CSR_Vector2 texCoords[24];
    CSR_Mesh*   pMesh;

    // calculate half values
    const float halfX = width  / 2.0;
    const float halfY = height / 2.0;
    const float halfZ = depth  / 2.0;

    // create a new mesh for the box
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // prepare the vertex buffer for each box edges
    pMesh->m_pVB   = (CSR_VertexBuffer*)csrMemoryAlloc(0, sizeof(CSR_VertexBuffer), 6);
    pMesh->m_Count = 6;

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh);
        return 0;
    }

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
        const float texOffset = 1.0 / 3.0;

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
        texCoords[0].m_X  = 0.0;             texCoords[0].m_Y  = texOffset;
        texCoords[1].m_X  = 0.0;             texCoords[1].m_Y  = 0.0;
        texCoords[2].m_X  = texOffset;       texCoords[2].m_Y  = texOffset;
        texCoords[3].m_X  = texOffset;       texCoords[3].m_Y  = 0.0;
        texCoords[4].m_X  = texOffset;       texCoords[4].m_Y  = texOffset;
        texCoords[5].m_X  = texOffset;       texCoords[5].m_Y  = 0.0;
        texCoords[6].m_X  = texOffset * 2.0; texCoords[6].m_Y  = texOffset;
        texCoords[7].m_X  = texOffset * 2.0; texCoords[7].m_Y  = 0.0;
        texCoords[8].m_X  = texOffset * 2.0; texCoords[8].m_Y  = texOffset;
        texCoords[9].m_X  = texOffset * 2.0; texCoords[9].m_Y  = 0.0;
        texCoords[10].m_X = 1.0;             texCoords[10].m_Y = texOffset;
        texCoords[11].m_X = 1.0;             texCoords[11].m_Y = 0.0;
        texCoords[12].m_X = 0.0;             texCoords[12].m_Y = texOffset * 2.0;
        texCoords[13].m_X = 0.0;             texCoords[13].m_Y = texOffset;
        texCoords[14].m_X = texOffset;       texCoords[14].m_Y = texOffset * 2.0;
        texCoords[15].m_X = texOffset;       texCoords[15].m_Y = texOffset;
        texCoords[16].m_X = texOffset;       texCoords[16].m_Y = texOffset * 2.0;
        texCoords[17].m_X = texOffset;       texCoords[17].m_Y = texOffset;
        texCoords[18].m_X = texOffset * 2.0; texCoords[18].m_Y = texOffset * 2.0;
        texCoords[19].m_X = texOffset * 2.0; texCoords[19].m_Y = texOffset;
        texCoords[20].m_X = texOffset * 2.0; texCoords[20].m_Y = texOffset * 2.0;
        texCoords[21].m_X = texOffset * 2.0; texCoords[21].m_Y = texOffset;
        texCoords[22].m_X = 1.0;             texCoords[22].m_Y = texOffset * 2.0;
        texCoords[23].m_X = 1.0;             texCoords[23].m_Y = texOffset;
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
CSR_Mesh* csrShapeCreateSphere(      float                 radius,
                                     int                   slices,
                                     int                   stacks,
                               const CSR_VertexFormat*     pVertFormat,
                               const CSR_VertexCulling*    pVertCulling,
                               const CSR_Material*         pMaterial,
                               const CSR_fOnGetVertexColor fOnGetVertexColor)
{
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
    size_t            vbLength;
    CSR_Mesh*         pMesh;
    CSR_VertexBuffer* pVB;
    CSR_Vector3       vertex;
    CSR_Vector3       normal;
    CSR_Vector2       uv;

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // initialize global values
    majorStep = (M_PI          / slices);
    minorStep = ((2.0f * M_PI) / stacks);

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
            csrMeshRelease(pMesh);
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
CSR_Mesh* csrShapeCreateCylinder(      float                 radius,
                                       float                 height,
                                       int                   faces,
                                 const CSR_VertexFormat*     pVertFormat,
                                 const CSR_VertexCulling*    pVertCulling,
                                 const CSR_Material*         pMaterial,
                                 const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    int         i;
    float       angle;
    float       step;
    CSR_Vector3 vertex;
    CSR_Vector3 normal;
    CSR_Vector2 uv;
    CSR_Mesh*   pMesh;

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a new vertex buffer to contain the cylinder
    pMesh->m_Count = 1;
    pMesh->m_pVB   = (CSR_VertexBuffer*)malloc(sizeof(CSR_VertexBuffer));

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh);
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

    // apply the user wished material
    if (pMaterial)
        pMesh->m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_TriangleStrip;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // calculate step to apply between faces
    step = (2.0f * M_PI) / (float)faces;

    // iterate through vertices to create
    for (i = 0; i < faces + 1; ++i)
    {
        // calculate angle
        angle = step * i;

        // set vertex data
        vertex.m_X =   radius * cosf(angle);
        vertex.m_Y = -(height / 2.0f);
        vertex.m_Z =   radius * sinf(angle);

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
        vertex.m_X =  radius * cosf(angle);
        vertex.m_Y = (height / 2.0f);
        vertex.m_Z =  radius * sinf(angle);

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
CSR_Mesh* csrShapeCreateDisk(      float                 centerX,
                                   float                 centerY,
                                   float                 radius,
                                   unsigned              slices,
                             const CSR_VertexFormat*     pVertFormat,
                             const CSR_VertexCulling*    pVertCulling,
                             const CSR_Material*         pMaterial,
                             const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    unsigned    i;
    float       x;
    float       y;
    float       step;
    float       angle;
    CSR_Vector3 vertex;
    CSR_Vector3 normal;
    CSR_Vector2 uv;
    CSR_Mesh*   pMesh;

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a new vertex buffer to contain the disk
    pMesh->m_Count = 1;
    pMesh->m_pVB   = (CSR_VertexBuffer*)malloc(sizeof(CSR_VertexBuffer));

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh);
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
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_TriangleFan;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // calculate the slice step
    step = (2.0f * M_PI) / (float)slices;

    // iterate through disk slices to create
    for (i = 0; i <= slices + 1; ++i)
    {
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
            x = centerX + radius * cos(angle);
            y = centerY + radius * sin(angle);
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
                uv.m_X = 0.5f + (cos(angle) * 0.5f);
                uv.m_Y = 0.5f + (sin(angle) * 0.5f);
            }

        // add the vertex to the buffer
        csrVertexBufferAdd(&vertex, &normal, &uv, i, fOnGetVertexColor, pMesh->m_pVB);
    }

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateRing(      float                 centerX,
                                   float                 centerY,
                                   float                 minRadius,
                                   float                 maxRadius,
                                   unsigned              slices,
                             const CSR_VertexFormat*     pVertFormat,
                             const CSR_VertexCulling*    pVertCulling,
                             const CSR_Material*         pMaterial,
                             const CSR_fOnGetVertexColor fOnGetVertexColor)
{
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

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // create a new vertex buffer to contain the ring
    pMesh->m_Count = 1;
    pMesh->m_pVB   = (CSR_VertexBuffer*)malloc(sizeof(CSR_VertexBuffer));

    // succeeded?
    if (!pMesh->m_pVB)
    {
        csrMeshRelease(pMesh);
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
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_TriangleStrip;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // calculate the slice step
    step = (2.0f * M_PI) / (float)slices;

    // iterate through ring slices to create
    for (i = 0; i <= slices; ++i)
    {
        // calculate the current slice angle
        angle = step * (float)i;

        // calculate the slice min point
        xA = centerX + minRadius * cos(angle);
        yA = centerY - minRadius * sin(angle);

        // calculate the slice max point
        xB = centerX + maxRadius * cos(angle);
        yB = centerY - maxRadius * sin(angle);

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
CSR_Mesh* csrShapeCreateSpiral(      float                 centerX,
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

    // create a mesh to contain the shape
    pMesh = csrMeshCreate();

    // succeeded?
    if (!pMesh)
        return 0;

    // calculate the slice step
    step = (2.0f * M_PI) / (float)slices;
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
            csrMeshRelease(pMesh);
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
            xA = centerX + minRadius * cos(angle);
            yA = centerY + minRadius * sin(angle);

            // calculate the slice max point
            xB = centerX + maxRadius * cos(angle);
            yB = centerY + maxRadius * sin(angle);

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
void csrModelRelease(CSR_Model* pModel)
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
            // delete the texture
            if (pModel->m_pMesh[i].m_Shader.m_TextureID != M_CSR_Error_Code)
            {
                // check if the same texture is assigned to several meshes
                for (j = i + 1; j < pModel->m_MeshCount; ++j)
                    if (pModel->m_pMesh[i].m_Shader.m_TextureID == pModel->m_pMesh[j].m_Shader.m_TextureID)
                        // reset the identifier to avoid to delete it twice
                        pModel->m_pMesh[j].m_Shader.m_TextureID = M_CSR_Error_Code;

                glDeleteTextures(1, &pModel->m_pMesh[i].m_Shader.m_TextureID);
            }

            // delete the bump map
            if (pModel->m_pMesh[i].m_Shader.m_BumpMapID != M_CSR_Error_Code)
            {
                // check if the same bump map is assigned to several meshes
                for (j = i + 1; j < pModel->m_MeshCount; ++j)
                    if (pModel->m_pMesh[i].m_Shader.m_BumpMapID == pModel->m_pMesh[j].m_Shader.m_BumpMapID)
                        // reset the identifier to avoid to delete it twice
                        pModel->m_pMesh[j].m_Shader.m_BumpMapID = M_CSR_Error_Code;

                glDeleteTextures(1, &pModel->m_pMesh[i].m_Shader.m_BumpMapID);
            }

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
                      const CSR_fOnTextureRead    fOnTextureRead)
{
    CSR_MDLHeader*       pHeader;
    CSR_MDLSkin*         pSkin;
    CSR_MDLTextureCoord* pTexCoord;
    CSR_MDLPolygon*      pPolygon;
    CSR_MDLFrameGroup*   pFrameGroup;
    CSR_MDL*             pMDL;
    CSR_PixelBuffer*     pPixelBuffer;
    CSR_ModelTexture*    pTexture;
    CSR_ModelAnimation*  pAnimation;
    unsigned char        skinName[16];
    unsigned char        prevSkinName[16];
    unsigned             animationStartIndex;
    unsigned             skinNameIndex;
    const unsigned       skinNameLength = sizeof(skinName);
    GLuint               textureID;
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
        csrMDLRelease(pMDL);
        return 0;
    }

    // read file header
    csrMDLReadHeader(pBuffer, &offset, pHeader);

    // is mdl file and version correct?
    if ((pHeader->m_ID != M_MDL_ID) || ((float)pHeader->m_Version != M_MDL_Mesh_File_Version))
    {
        free(pHeader);
        csrMDLRelease(pMDL);
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
                csrMDLRelease(pMDL);
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
                csrMDLRelease(pMDL);
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
                csrMDLRelease(pMDL);
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
                csrMDLRelease(pMDL);
                return 0;
            }
    }
    else
        pFrameGroup = 0;

    // do generate textures?
    if (!pVertFormat || pVertFormat->m_HasTexCoords)
    {
        // assign the memory to contain the textures
        pMDL->m_pTexture     = (CSR_ModelTexture*)malloc(sizeof(CSR_ModelTexture));
        pMDL->m_TextureCount = pSkin->m_Count;

        // succeeded?
        if (!pMDL->m_pTexture)
        {
            // release the MDL object used for the loading
            csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

            // release the model
            csrMDLRelease(pMDL);

            return 0;
        }

        // iterate through textures to extract
        for (i = 0; i < pSkin->m_Count; ++i)
        {
            int noGPU = 0;

            // extract texture from model
            pPixelBuffer = csrMDLUncompressTexture(pSkin,
                                                   pPalette,
                                                   pHeader->m_SkinWidth,
                                                   pHeader->m_SkinHeight,
                                                   i);

            // succeeded?
            if (!pPixelBuffer)
            {
                // release the MDL object used for the loading
                csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

                // release the model
                csrMDLRelease(pMDL);

                return 0;
            }

            // notify that a texture was read
            if (fOnTextureRead)
                fOnTextureRead(i, pPixelBuffer, &noGPU);

            // can load the texture on the GPU?
            if (!noGPU)
            {
                // is a default texture?
                if (pPixelBuffer->m_DataLength <= 48)
                {
                    unsigned color;

                    free(pPixelBuffer->m_pData);

                    // recreate a 4 * 4 * 3 pixel buffer
                    pPixelBuffer->m_DataLength = 48;
                    pPixelBuffer->m_pData      = (unsigned char*)malloc(pPixelBuffer->m_DataLength);

                    // succeeded?
                    if (!pPixelBuffer->m_pData)
                    {
                        // release the MDL object used for the loading
                        csrMDLReleaseObjects(pHeader, pFrameGroup, pSkin, pTexCoord, pPolygon);

                        // release the model
                        csrMDLRelease(pMDL);

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
                        ((unsigned char*)pPixelBuffer->m_pData)[ j * 3]      = ((color >> 24) & 0xFF);
                        ((unsigned char*)pPixelBuffer->m_pData)[(j * 3) + 1] = ((color >> 16) & 0xFF);
                        ((unsigned char*)pPixelBuffer->m_pData)[(j * 3) + 2] = ((color >> 8)  & 0xFF);
                    }
                }

                // create a GPU texture for the model
                pMDL->m_pTexture[i].m_TextureID = csrTextureFromPixelBuffer(pPixelBuffer);
            }
            else
                // texture isn't used
                pMDL->m_pTexture[i].m_TextureID = M_CSR_Error_Code;

            // the mdl file contains no bump map. This may be used later, but for now it's unused
            pMDL->m_pTexture[i].m_BumpMapID = M_CSR_Error_Code;

            // release the pixel buffer
            csrPixelBufferRelease(pPixelBuffer);

            // also get the animation time
            if (pSkin->m_pTime)
            {
                pMDL->m_pTexture[i].m_Time = pSkin->m_pTime[i] - lastKnownTime;
                lastKnownTime              = pSkin->m_pTime[i];
            }
            else
                pMDL->m_pTexture[i].m_Time = 0.0;
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
            // get the skin name
            memset(skinName, 0x0, skinNameLength);
            strcpy(skinName, pFrameGroup[i].m_pFrame[0].m_Name);

            // revert iterate through the skin name and remove all the trailing numbers
            for (j = 0; j < skinNameLength; ++j)
            {
                // calculate the skin name index
                skinNameIndex = (skinNameLength - 1) - j;

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
                    csrMDLRelease(pMDL);

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
                animationStartIndex = i;
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
                    const CSR_fOnTextureRead    fOnTextureRead)
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
                        fOnTextureRead);

    // release the file buffer (no longer required)
    csrBufferRelease(pBuffer);

    return pMDL;
}
//---------------------------------------------------------------------------
void csrMDLRelease(CSR_MDL* pMDL)
{
    size_t i;
    size_t j;
    size_t k;

    // no MDL model to release?
    if (!pMDL)
        return;

    // do free the textures?
    if (pMDL->m_pTexture)
    {
        // delete each texture
        for (i = 0; i < pMDL->m_TextureCount; ++i)
        {
            if (pMDL->m_pTexture[i].m_TextureID != M_CSR_Error_Code)
                glDeleteTextures(1, &pMDL->m_pTexture[i].m_TextureID);

            if (pMDL->m_pTexture[i].m_BumpMapID != M_CSR_Error_Code)
                glDeleteTextures(1, &pMDL->m_pTexture[i].m_BumpMapID);
        }

        // free the textures
        free(pMDL->m_pTexture);
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
    pMDL->m_pTexture       = 0;
    pMDL->m_TextureCount   = 0;
}
//---------------------------------------------------------------------------
void csrMDLUpdateIndex(const CSR_MDL* pMDL,
                             size_t   fps,
                             size_t   animationIndex,
                             size_t*  pTextureIndex,
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
        *pTextureIndex    = 0;
        *pModelIndex      = 0;
        *pMeshIndex       = 0;
        *pTextureLastTime = 0.0;
        *pModelLastTime   = 0.0;
        *pMeshLastTime    = 0.0;
        return;
    }

    // are textures animated?
    if (pMDL->m_TextureCount > 1)
    {
        // apply the elapsed time
        *pTextureLastTime += elapsedTime;

        // certify that the texture index is inside the limits
        *pTextureIndex = (*pTextureIndex % pMDL->m_TextureCount);

        // do get the next texture?
        while (*pTextureLastTime >= (pMDL->m_pTexture[*pTextureIndex].m_Time))
        {
            // decrease the counted time
            *pTextureLastTime -= pMDL->m_pTexture[*pTextureIndex].m_Time;

            // go to next index
            *pTextureIndex = ((*pTextureIndex + 1) % pMDL->m_TextureCount);
        }
    }

    // get the current model mesh for which the index should be updated
    pMesh = csrMDLGetMesh(pMDL, *pModelIndex, *pMeshIndex);

    // found it?
    if (!pMesh)
    {
        // reset all values
        *pTextureIndex    = 0;
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
        *pTextureIndex    = 0;
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
        *pTextureIndex    = 0;
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
        *pTextureIndex    = 0;
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
    pPB->m_Width        = width;
    pPB->m_Height       = height;
    pPB->m_BytePerPixel = bpp;
    pPB->m_Stride       = width * pPB->m_BytePerPixel;
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
    unsigned i;
    float    vertex[3];

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
    int            i;
    size_t         j;
    size_t         k;
    CSR_Vector3    vertex;
    CSR_Vector3    normal;
    CSR_Vector2    uv;
    CSR_MDLVertex* pSrcVertex;
    double         lastKnownTime = 0.0;

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
        pModel->m_pMesh[i].m_Shader.m_TextureID = M_CSR_Error_Code;
        pModel->m_pMesh[i].m_Shader.m_BumpMapID = M_CSR_Error_Code;

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
                uv.m_X = pTexCoord[pPolygon[j].m_VertexIndex[k]].m_U;
                uv.m_Y = pTexCoord[pPolygon[j].m_VertexIndex[k]].m_V;

                // is texture coordinate on the back face?
                if (!pPolygon[j].m_FacesFront && pTexCoord[pPolygon[j].m_VertexIndex[k]].m_OnSeam)
                    // correct the texture coordinate to put it on the back face
                    uv.m_X += pHeader->m_SkinWidth * 0.5f;

                // scale s and t to range from 0.0 to 1.0
                uv.m_X = (uv.m_X + 0.5) / pHeader->m_SkinWidth;
                uv.m_Y = (uv.m_Y + 0.5) / pHeader->m_SkinHeight;

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
                              const CSR_fOnTextureRead    fOnTextureRead)
{
    size_t                 i;
    int                    objectChanging;
    int                    groupChanging;
    char                   ch;
    char                   buffer[256];
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
                                           fOnTextureRead))
                {
                    // free the model
                    csrModelRelease(pModel);

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
                            const CSR_fOnTextureRead    fOnTextureRead)
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
                                fOnTextureRead);

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

        // skip all following separators
        while (*pChar == ' ' && *pChar == '\r' && *pChar == '\n' && *pIndex < pBuffer->m_Length)
        {
            // go to next char
            ++(*pIndex);
            *pChar = ((char*)pBuffer->m_pData)[*pIndex];
        }
    }
}
//---------------------------------------------------------------------------
void csrWaveFrontReadVertex(const CSR_Buffer*          pBuffer,
                                  char*                pChar,
                                  size_t*              pIndex,
                                  CSR_WavefrontVertex* pVertex)
{
    char   line[256];
    size_t lineIndex;
    int    doExit;

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
                // skip all following separators
                while (*pChar == ' ' && *pChar == '\r' && *pChar == '\n' && *pIndex < pBuffer->m_Length)
                {
                    // go to next char
                    ++(*pIndex);
                    *pChar = ((char*)pBuffer->m_pData)[*pIndex];
                }

                // somehing to parse?
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
    char   line[256];
    size_t lineIndex;
    int    doExit;

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
                // skip all following separators
                while (*pChar == ' ' && *pChar == '\r' && *pChar == '\n' && *pIndex < pBuffer->m_Length)
                {
                    // go to next char
                    ++(*pIndex);
                    *pChar = ((char*)pBuffer->m_pData)[*pIndex];
                }

                // somehing to parse?
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
    char   line[256];
    size_t lineIndex;
    int    doExit;

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
                // skip all following separators
                while (*pChar == ' ' && *pChar == '\r' && *pChar == '\n' && *pIndex < pBuffer->m_Length)
                {
                    // go to next char
                    ++(*pIndex);
                    *pChar = ((char*)pBuffer->m_pData)[*pIndex];
                }

                // somehing to parse?
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
    char   line[256];
    size_t lineIndex;
    int    doExit;

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
                // skip all following separators
                while (*pChar == ' ' && *pChar == '\r' && *pChar == '\n' && *pIndex < pBuffer->m_Length)
                {
                    // go to next char
                    ++(*pIndex);
                    *pChar = ((char*)pBuffer->m_pData)[*pIndex];
                }

                // somehing to parse?
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

        // skip all following separators
        while (*pChar == ' ' && *pChar == '\r' && *pChar == '\n' && *pIndex < pBuffer->m_Length)
        {
            // go to next char
            ++(*pIndex);
            *pChar = ((char*)pBuffer->m_pData)[*pIndex];
        }
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
    (*pArray)[index] = atof(pBuffer);
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
                          const CSR_fOnTextureRead     fOnTextureRead)
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
                                      fOnTextureRead);
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
                                   const CSR_fOnTextureRead     fOnTextureRead)
{
    size_t i;
    size_t faceStride;
    size_t normalOffset;
    size_t uvOffset;
    size_t dataIndex;
    int    baseVertexIndex;
    int    baseNormalIndex;
    int    baseUVIndex;

    // calculate the normal and uv offsets. Be careful, the face values follows one each other in
    // the file, without distinction, so the correct format (v, v/n, v/f or v/n/f) should be
    // determined and used
    normalOffset = (pNormal && pNormal->m_Count) ? 1                : 0;
    uvOffset     = (pUV     && pUV->m_Count)     ? normalOffset + 1 : 0;

    // wavefront faces are organized as triangle fan, so get the first vertex
    // and build all others from it
    baseVertexIndex = (pFace->m_pData[0] - 1) * 3;
    faceStride      = 1;

    // get the first normal
    if (pNormal->m_Count)
    {
        baseNormalIndex = (pFace->m_pData[normalOffset] - 1) * 3;
        ++faceStride;
    }

    // get the first texture coordinate
    if (pUV->m_Count)
    {
        baseUVIndex = (pFace->m_pData[uvOffset] - 1) * 2;
        ++faceStride;
    }

    // iterate through remaining indices
    for (i = 1; i <= (pFace->m_Count / faceStride) - 2; ++i)
    {
        CSR_Vector3 vertex;
        CSR_Vector3 normal;
        CSR_Vector2 uv;

        // build polygon vertex 1
        int vertexIndex =                                baseVertexIndex;
        int normalIndex = pVB->m_Format.m_HasNormal    ? baseNormalIndex : 0;
        int uvIndex     = pVB->m_Format.m_HasTexCoords ? baseUVIndex     : 0;

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
        normalIndex = pVB->m_Format.m_HasNormal    ? (pFace->m_pData[(i * faceStride) + normalOffset] - 1) * 3 : 0;
        uvIndex     = pVB->m_Format.m_HasTexCoords ? (pFace->m_pData[(i * faceStride) + uvOffset]     - 1) * 2 : 0;

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
        normalIndex = pVB->m_Format.m_HasNormal    ? (pFace->m_pData[((i + 1) * faceStride) + normalOffset] - 1) * 3 : 0;
        uvIndex     = pVB->m_Format.m_HasTexCoords ? (pFace->m_pData[((i + 1) * faceStride) + uvOffset]     - 1) * 2 : 0;

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

    // calculate lansdcape data size and reserve memory for landscape mesh
    pVertices->m_Length = pPixelBuffer->m_Width * pPixelBuffer->m_Height;
    pVertices->m_pData  = malloc(pVertices->m_Length * sizeof(CSR_Vector3));

    // calculate scaling factor on x and z axis
    scaleX = -(((pPixelBuffer->m_Width  - 1) * scale) / 2.0f);
    scaleZ =  (((pPixelBuffer->m_Height - 1) * scale) / 2.0f);

    // loop through heightfield points and calculate coordinates for each point
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
        csrMeshRelease(pMesh);
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
        csrMeshRelease(pMesh);
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
