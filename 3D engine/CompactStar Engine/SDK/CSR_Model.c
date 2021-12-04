/****************************************************************************
 * ==> CSR_Model -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the model functions and types         *
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

//---------------------------------------------------------------------------
// Shape private functions
//---------------------------------------------------------------------------
void csrCapsuleGetCylinderVertex(const CSR_Vector3* pTop,
                                 const CSR_Vector3* pLocalX,
                                 const CSR_Vector3* pLocalY,
                                 const CSR_Vector3* pLocalZ,
                                       float        radius,
                                       float        length,
                                       float        u,
                                       float        v,
                                       CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 vX      = {0};
        CSR_Vector3 vY      = {0};
        CSR_Vector3 vZ      = {0};
        CSR_Vector3 vBuild1 = {0};
        CSR_Vector3 vBuild2 = {0};
    #else
        CSR_Vector3 vX;
        CSR_Vector3 vY;
        CSR_Vector3 vZ;
        CSR_Vector3 vBuild1;
        CSR_Vector3 vBuild2;
    #endif

    const float x = cosf(2.0f * (float)M_PI * u) * radius;
    const float y = sinf(2.0f * (float)M_PI * u) * radius;
    const float z = v * length;

    csrVec3MulVal(pLocalX, x, &vX);
    csrVec3MulVal(pLocalY, y, &vY);
    csrVec3MulVal(pLocalZ, z, &vZ);

    csrVec3Add(pTop,     &vX, &vBuild1);
    csrVec3Add(&vBuild1, &vY, &vBuild2);
    csrVec3Add(&vBuild2, &vZ, pR);
}
//---------------------------------------------------------------------------
void csrCapsuleGetSphereStartVertex(const CSR_Vector3* pTop,
                                    const CSR_Vector3* pLocalX,
                                    const CSR_Vector3* pLocalY,
                                    const CSR_Vector3* pLocalZ,
                                          float        radius,
                                          float        u,
                                          float        v,
                                          CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 vX      = {0};
        CSR_Vector3 vY      = {0};
        CSR_Vector3 vZ      = {0};
        CSR_Vector3 vBuild1 = {0};
        CSR_Vector3 vBuild2 = {0};
    #else
        CSR_Vector3 vX;
        CSR_Vector3 vY;
        CSR_Vector3 vZ;
        CSR_Vector3 vBuild1;
        CSR_Vector3 vBuild2;
    #endif

    const float latitude = (float)(M_PI / 2.0) * (v - 1);
    const float x        = cosf(2.0f * (float)M_PI * u) * cosf(latitude) * radius;
    const float y        = sinf(2.0f * (float)M_PI * u) * cosf(latitude) * radius;
    const float z        = sinf(latitude) * radius;

    csrVec3MulVal(pLocalX, x, &vX);
    csrVec3MulVal(pLocalY, y, &vY);
    csrVec3MulVal(pLocalZ, z, &vZ);

    csrVec3Add(pTop,     &vX, &vBuild1);
    csrVec3Add(&vBuild1, &vY, &vBuild2);
    csrVec3Add(&vBuild2, &vZ, pR);
}
//---------------------------------------------------------------------------
void csrCapsuleGetSphereEndVertex(const CSR_Vector3* pBottom,
                                  const CSR_Vector3* pLocalX,
                                  const CSR_Vector3* pLocalY,
                                  const CSR_Vector3* pLocalZ,
                                        float        radius,
                                        float        u,
                                        float        v,
                                        CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 vX      = {0};
        CSR_Vector3 vY      = {0};
        CSR_Vector3 vZ      = {0};
        CSR_Vector3 vBuild1 = {0};
        CSR_Vector3 vBuild2 = {0};
    #else
        CSR_Vector3 vX;
        CSR_Vector3 vY;
        CSR_Vector3 vZ;
        CSR_Vector3 vBuild1;
        CSR_Vector3 vBuild2;
    #endif

    const float latitude = (float)(M_PI / 2.0) * v;
    const float x        = cosf(2.0f * (float)M_PI * u) * cosf(latitude) * radius;
    const float y        = sinf(2.0f * (float)M_PI * u) * cosf(latitude) * radius;
    const float z        = sinf(latitude) * radius;

    csrVec3MulVal(pLocalX, x, &vX);
    csrVec3MulVal(pLocalY, y, &vY);
    csrVec3MulVal(pLocalZ, z, &vZ);

    csrVec3Add(pBottom,  &vX, &vBuild1);
    csrVec3Add(&vBuild1, &vY, &vBuild2);
    csrVec3Add(&vBuild2, &vZ, pR);
}
//---------------------------------------------------------------------------
void csrCapsuleGetAnyPerpendicularUnitVector(const CSR_Vector3* pVec, CSR_Vector3* pR)
{
    if (pVec->m_Y != 0.0f || pVec->m_Z != 0.0f)
    {
        pR->m_X = 1.0f;
        pR->m_Y = 0.0f;
        pR->m_Z = 0.0f;
        return;
    }

    pR->m_X = 0.0f;
    pR->m_Y = 1.0f;
    pR->m_Z = 0.0f;
}
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
                                (size_t)j * 2,
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
                               ((size_t)j * 2) + 1,
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
        CSR_Mesh*   pMesh;
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
        csrVertexBufferAdd(&vertex, &normal, &uv, (size_t)i * 2, fOnGetVertexColor, pMesh->m_pVB);

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
        csrVertexBufferAdd(&vertex, &normal, &uv, ((size_t)i * 2) + 1, fOnGetVertexColor, pMesh->m_pVB);
    }

    return pMesh;
}
//---------------------------------------------------------------------------
CSR_Mesh* csrShapeCreateCapsule(float                 height,
                                float                 radius,
                                float                 resolution,
                          const CSR_VertexFormat*     pVertFormat,
                          const CSR_VertexCulling*    pVertCulling,
                          const CSR_Material*         pMaterial,
                          const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    #ifdef _MSC_VER
        size_t      i;
        size_t      j;
        float       length;
        CSR_Vector3 capsuleTop    = {0};
        CSR_Vector3 capsuleBottom = {0};
        CSR_Vector3 lineDir       = {0};
        CSR_Vector3 lineEndOffset = {0};
        CSR_Vector3 top           = {0};
        CSR_Vector3 bottom        = {0};
        CSR_Vector3 axis          = {0};
        CSR_Vector3 localZ        = {0};
        CSR_Vector3 localX        = {0};
        CSR_Vector3 localY        = {0};
        CSR_Vector3 start         = {0};
        CSR_Vector3 end           = {0};
        CSR_Vector3 step          = {0};
        CSR_Vector3 p0            = {0};
        CSR_Vector3 p1            = {0};
        CSR_Vector3 p2            = {0};
        CSR_Vector3 p3            = {0};
        CSR_Vector3 normal0       = {0};
        CSR_Vector3 normal1       = {0};
        CSR_Vector3 normal2       = {0};
        CSR_Vector3 normal3       = {0};
        CSR_Vector2 uv0           = {0};
        CSR_Vector2 uv1           = {0};
        CSR_Vector2 uv2           = {0};
        CSR_Vector2 uv3           = {0};
        CSR_Mesh*   pMesh         =  0;
    #else
        size_t      i;
        size_t      j;
        float       length;
        CSR_Vector3 capsuleTop;
        CSR_Vector3 capsuleBottom;
        CSR_Vector3 lineDir;
        CSR_Vector3 lineEndOffset;
        CSR_Vector3 top;
        CSR_Vector3 bottom;
        CSR_Vector3 axis;
        CSR_Vector3 localZ;
        CSR_Vector3 localX;
        CSR_Vector3 localY;
        CSR_Vector3 start;
        CSR_Vector3 end;
        CSR_Vector3 step;
        CSR_Vector3 p0;
        CSR_Vector3 p1;
        CSR_Vector3 p2;
        CSR_Vector3 p3;
        CSR_Vector3 normal0;
        CSR_Vector3 normal1;
        CSR_Vector3 normal2;
        CSR_Vector3 normal3;
        CSR_Vector2 uv0;
        CSR_Vector2 uv1;
        CSR_Vector2 uv2;
        CSR_Vector2 uv3;
        CSR_Mesh*   pMesh;
    #endif

    const float third     = 1.0f / 3.0f;
    const float twoThirds = 2.0f / 3.0f;

    if (radius == 0.0f || resolution == 0.0f)
        return 0;

    capsuleTop.m_X    = 0.0f;
    capsuleTop.m_Y    = height;
    capsuleTop.m_Z    = 0.0f;
    capsuleBottom.m_X = 0.0f;
    capsuleBottom.m_Y = 0.0f;
    capsuleBottom.m_Z = 0.0f;

    // calculate capsule bounds
    csrVec3Sub(&capsuleTop, &capsuleBottom, &lineDir);
    csrVec3Normalize(&lineDir, &lineDir);
    csrVec3MulVal(&lineDir, radius,        &lineEndOffset);
    csrVec3Sub(&capsuleTop, &lineEndOffset, &top);
    csrVec3Add(&capsuleBottom, &lineEndOffset, &bottom);

    // calculate capsule axis and length
    csrVec3Sub(&bottom, &top, &axis);
    csrVec3Length(&axis, &length);

    if (length == 0.0f)
        localZ = axis;
    else
        csrVec3DivVal(&axis, length, &localZ);

    csrCapsuleGetAnyPerpendicularUnitVector(&localZ, &localX);
    csrVec3Cross(&localZ, &localX, &localY);

    start.m_X = 0.0f;
    start.m_Y = 0.0f;
    start.m_Z = 0.0f;
    end.m_X   = 1.0f;
    end.m_Y   = 1.0f;
    end.m_Z   = 1.0f;

    csrVec3Sub(&end, &start, &step);
    csrVec3DivVal(&step, resolution, &step);

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
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_Triangles;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // iterate through latitude and longitude
    for (i = 0; i < (size_t)resolution; ++i)
        for (j = 0; j < (size_t)resolution; ++j)
        {
            // calculate the current slice
            const float u = i * step.m_X + start.m_X;
            const float v = j * step.m_Y + start.m_Y;

            // calculate the next slice
            const float un = (i + 1 == (size_t)resolution) ? end.m_X : (i + 1) * step.m_X + start.m_X;
            const float vn = (j + 1 == (size_t)resolution) ? end.m_Y : (j + 1) * step.m_Y + start.m_Y;

            // create next cylinder face
            csrCapsuleGetCylinderVertex(&top, &localX, &localY, &localZ, radius, length, u,  v,  &p0);
            csrCapsuleGetCylinderVertex(&top, &localX, &localY, &localZ, radius, length, u,  vn, &p1);
            csrCapsuleGetCylinderVertex(&top, &localX, &localY, &localZ, radius, length, un, v,  &p2);
            csrCapsuleGetCylinderVertex(&top, &localX, &localY, &localZ, radius, length, un, vn, &p3);

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasNormal)
            {
                // calculate the vertex normals
                csrVec3DivVal(&p0, radius, &normal0);
                csrVec3DivVal(&p1, radius, &normal1);
                csrVec3DivVal(&p2, radius, &normal2);
                csrVec3DivVal(&p3, radius, &normal3);
            }

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasTexCoords)
            {
                uv0.m_X =           (i      / resolution);
                uv0.m_Y = third +  ((j      / resolution) * third);
                uv1.m_X =           (i      / resolution);
                uv1.m_Y = third + (((j + 1) / resolution) * third);
                uv2.m_X =          ((i + 1) / resolution);
                uv2.m_Y = third +  ((j      / resolution) * third);
                uv3.m_X =          ((i + 1) / resolution);
                uv3.m_Y = third + (((j + 1) / resolution) * third);
            }

            // add face to vertex buffer
            csrVertexBufferAdd(&p0, &normal0, &uv0, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p1, &normal1, &uv1, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p2, &normal2, &uv2, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p2, &normal2, &uv2, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p1, &normal1, &uv1, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p3, &normal3, &uv3, 0, fOnGetVertexColor, pMesh->m_pVB);

            // create next sphere start face
            csrCapsuleGetSphereStartVertex(&top, &localX, &localY, &localZ, radius, u,  v,  &p0);
            csrCapsuleGetSphereStartVertex(&top, &localX, &localY, &localZ, radius, u,  vn, &p1);
            csrCapsuleGetSphereStartVertex(&top, &localX, &localY, &localZ, radius, un, v,  &p2);
            csrCapsuleGetSphereStartVertex(&top, &localX, &localY, &localZ, radius, un, vn, &p3);

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasNormal)
            {
                // calculate the vertex normals
                csrVec3DivVal(&p0, radius, &normal0);
                csrVec3DivVal(&p1, radius, &normal1);
                csrVec3DivVal(&p2, radius, &normal2);
                csrVec3DivVal(&p3, radius, &normal3);
            }

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasTexCoords)
            {
                uv0.m_X =   (i      / resolution);
                uv0.m_Y =  ((j      / resolution) * third);
                uv1.m_X =   (i      / resolution);
                uv1.m_Y = (((j + 1) / resolution) * third);
                uv2.m_X =  ((i + 1) / resolution);
                uv2.m_Y =  ((j      / resolution) * third);
                uv3.m_X =  ((i + 1) / resolution);
                uv3.m_Y = (((j + 1) / resolution) * third);
            }

            // add face to vertex buffer
            csrVertexBufferAdd(&p0, &normal0, &uv0, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p1, &normal1, &uv1, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p2, &normal2, &uv2, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p2, &normal2, &uv2, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p1, &normal1, &uv1, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p3, &normal3, &uv3, 0, fOnGetVertexColor, pMesh->m_pVB);

            // create next sphere end face
            csrCapsuleGetSphereEndVertex(&bottom, &localX, &localY, &localZ, radius, u,  v,  &p0);
            csrCapsuleGetSphereEndVertex(&bottom, &localX, &localY, &localZ, radius, u,  vn, &p1);
            csrCapsuleGetSphereEndVertex(&bottom, &localX, &localY, &localZ, radius, un, v,  &p2);
            csrCapsuleGetSphereEndVertex(&bottom, &localX, &localY, &localZ, radius, un, vn, &p3);

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasNormal)
            {
                // calculate the vertex normals
                csrVec3DivVal(&p0, radius, &normal0);
                csrVec3DivVal(&p1, radius, &normal1);
                csrVec3DivVal(&p2, radius, &normal2);
                csrVec3DivVal(&p3, radius, &normal3);
            }

            // vertex has UV texture coordinates?
            if (pMesh->m_pVB[0].m_Format.m_HasTexCoords)
            {
                uv0.m_X =               (i      / resolution);
                uv0.m_Y = twoThirds +  ((j      / resolution) * third);
                uv1.m_X =               (i      / resolution);
                uv1.m_Y = twoThirds + (((j + 1) / resolution) * third);
                uv2.m_X =              ((i + 1) / resolution);
                uv2.m_Y = twoThirds +  ((j      / resolution) * third);
                uv3.m_X =              ((i + 1) / resolution);
                uv3.m_Y = twoThirds + (((j + 1) / resolution) * third);
            }

            // add face to vertex buffer
            csrVertexBufferAdd(&p0, &normal0, &uv0, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p1, &normal1, &uv1, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p2, &normal2, &uv2, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p2, &normal2, &uv2, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p1, &normal1, &uv1, 0, fOnGetVertexColor, pMesh->m_pVB);
            csrVertexBufferAdd(&p3, &normal3, &uv3, 0, fOnGetVertexColor, pMesh->m_pVB);
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
        csrVertexBufferAdd(&vertex, &normal, &uv, (size_t)i * 2, fOnGetVertexColor, pMesh->m_pVB);

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
        csrVertexBufferAdd(&vertex, &normal, &uv, ((size_t)i * 2) + 1, fOnGetVertexColor, pMesh->m_pVB);
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
                               (size_t)j * 2,
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
                               ((size_t)j * 2) + 1,
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
void csrBoneGetAnimMatrix(const CSR_Bone*              pBone,
                          const CSR_AnimationSet_Bone* pAnimSet,
                                size_t                 frameIndex,
                                CSR_Matrix4*           pInitialMatrix,
                                CSR_Matrix4*           pMatrix)
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
        if (!csrBoneAnimGetAnimMatrix(pAnimSet, pBone, frameIndex, &animMatrix))
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
CSR_AnimationKey* csrAnimKeyCreate(void)
{
    // create a new animation key structure
    CSR_AnimationKey* pAnimationKey = (CSR_AnimationKey*)malloc(sizeof(CSR_AnimationKey));

    // succeeded?
    if (!pAnimationKey)
        return 0;

    // initialize the animation key content
    csrAnimKeyInit(pAnimationKey);

    return pAnimationKey;
}
//---------------------------------------------------------------------------
void csrAnimKeyRelease(CSR_AnimationKey* pAnimationKey, int contentOnly)
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
void csrAnimKeyInit(CSR_AnimationKey* pAnimationKey)
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
CSR_AnimationKeys* csrAnimKeysCreate(void)
{
    // create a new animation keys structure
    CSR_AnimationKeys* pAnimationKeys = (CSR_AnimationKeys*)malloc(sizeof(CSR_AnimationKeys));

    // succeeded?
    if (!pAnimationKeys)
        return 0;

    // initialize the animation keys content
    csrAnimKeysInit(pAnimationKeys);

    return pAnimationKeys;
}
//---------------------------------------------------------------------------
void csrAnimKeysRelease(CSR_AnimationKeys* pAnimationKeys, int contentOnly)
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
            csrAnimKeyRelease(&pAnimationKeys->m_pKey[i], 1);

        // free the keys container
        free(pAnimationKeys->m_pKey);
    }

    // free the animation keys
    if (!contentOnly)
        free(pAnimationKeys);
}
//---------------------------------------------------------------------------
void csrAnimKeysInit(CSR_AnimationKeys* pAnimationKeys)
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
// Frame animation functions
//---------------------------------------------------------------------------
CSR_Animation_Frame* csrFrameAnimCreate(void)
{
    // create a new animation structure
    CSR_Animation_Frame* pAnimation = (CSR_Animation_Frame*)malloc(sizeof(CSR_Animation_Frame));

    // succeeded?
    if (!pAnimation)
        return 0;

    // initialize the animation content
    csrFrameAnimInit(pAnimation);

    return pAnimation;
}
//---------------------------------------------------------------------------
void csrFrameAnimRelease(CSR_Animation_Frame* pAnimation)
{
    // no animation to release?
    if (!pAnimation)
        return;

    // free the animation
    free(pAnimation);
}
//---------------------------------------------------------------------------
void csrFrameAnimInit(CSR_Animation_Frame* pAnimation)
{
    // no animation to initialize?
    if (!pAnimation)
        return;

    // initialize the animation content
    memset(pAnimation->m_Name, 0x0, sizeof(pAnimation->m_Name));
    pAnimation->m_Start = 0;
    pAnimation->m_End   = 0;
}
//---------------------------------------------------------------------------
// Bone animation functions
//---------------------------------------------------------------------------
CSR_Animation_Bone* csrBoneAnimCreate(void)
{
    // create a new animation structure
    CSR_Animation_Bone* pAnimation = (CSR_Animation_Bone*)malloc(sizeof(CSR_Animation_Bone));

    // succeeded?
    if (!pAnimation)
        return 0;

    // initialize the animation content
    csrBoneAnimInit(pAnimation);

    return pAnimation;
}
//---------------------------------------------------------------------------
void csrBoneAnimRelease(CSR_Animation_Bone* pAnimation, int contentOnly)
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
            csrAnimKeysRelease(&pAnimation->m_pKeys[i], 1);

        // free the keys container
        free(pAnimation->m_pKeys);
    }

    // free the animation
    if (!contentOnly)
        free(pAnimation);
}
//---------------------------------------------------------------------------
void csrBoneAnimInit(CSR_Animation_Bone* pAnimation)
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
int csrBoneAnimGetAnimMatrix(const CSR_AnimationSet_Bone* pAnimSet,
                             const CSR_Bone*              pBone,
                                   size_t                 frame,
                                   CSR_Matrix4*           pMatrix)
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
// Bone animation set functions
//---------------------------------------------------------------------------
CSR_AnimationSet_Bone* csrBoneAnimSetCreate(void)
{
    // create a new animation set structure
    CSR_AnimationSet_Bone* pAnimationSet = (CSR_AnimationSet_Bone*)malloc(sizeof(CSR_AnimationSet_Bone));

    // succeeded?
    if (!pAnimationSet)
        return 0;

    // initialize the animation set content
    csrBoneAnimSetInit(pAnimationSet);

    return pAnimationSet;
}
//---------------------------------------------------------------------------
void csrBoneAnimSetRelease(CSR_AnimationSet_Bone* pAnimationSet, int contentOnly)
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
            csrBoneAnimRelease(&pAnimationSet->m_pAnimation[i], 1);

        // free the animation container
        free(pAnimationSet->m_pAnimation);
    }

    // free the animation set
    if (!contentOnly)
        free(pAnimationSet);
}
//---------------------------------------------------------------------------
void csrBoneAnimSetInit(CSR_AnimationSet_Bone* pAnimationSet)
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

    // calculate landscape data size
    pVertices->m_Length = (size_t)(pPixelBuffer->m_Width * pPixelBuffer->m_Height);

    // incorrect length?
    if (!pVertices->m_Length)
        return 0;

    // allocate memory for landscape mesh
    pVertices->m_pData = (CSR_Vector3*)malloc(pVertices->m_Length * sizeof(CSR_Vector3));

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
