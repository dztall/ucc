/*****************************************************************************
 * ==> Minimal shapes library -----------------------------------------------*
 *****************************************************************************
 * Description : Minimal shapes library                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not                 *
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

    CalculateStride(pVertexFormat);

    // initialize basic values
    majorStep     = (M_PI          / slices);
    minorStep     = ((2.0f * M_PI) / stacks);
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

/**
* Creates a cylinder
*@param pRadius - cylinder radius
*@param pHeight - cylinder height
*@param faces - number of faces composing the cylinder
*@param color - color in RGBA format
*@param[in, out] pVertexFormat - vertex format to use
*@param[out] pVertices - resulting vertex buffer
*@param[out] pVertexCount - vertex count in buffer
*/
void CreateCylinder(const float*           pRadius,
                    const float*           pHeight,
                          int              faces,
                          unsigned         color,
                          MV_VertexFormat* pVertexFormat,
                          float**          pVertices,
                          unsigned*        pVertexCount)
{
    int   i;
    int   index;
    int   iDiv;
    float angle;
    float step;

    CalculateStride(pVertexFormat);

    // initialize vertex buffer
    *pVertices    = 0;
    *pVertexCount = (faces + 1) * 2 * pVertexFormat->m_Stride;

    // generate vertices
    if (!(*pVertices))
        *pVertices = (float*)malloc(*pVertexCount * sizeof(float));
    else
        *pVertices = (float*)realloc(*pVertices,
                                     *pVertexCount * sizeof(float));

    // calculate step to apply between faces
    step  = (2.0f * M_PI) / (float)faces;
    index = 0;

    // iterate through vertices to create
    for (i = 0; i < faces + 1; ++i)
    {
        // calculate angle
        angle = step * i;

        // set vertex data
        (*pVertices)[index]     =   *pRadius * cosf(angle);
        (*pVertices)[index + 1] = -(*pHeight / 2.0f);
        (*pVertices)[index + 2] =   *pRadius * sinf(angle);

        index += 3;

        // do generate normals?
        if (pVertexFormat->m_UseNormals)
        {
            // set normals
            (*pVertices)[index]     = cosf(angle);
            (*pVertices)[index + 1] = 0.0f;
            (*pVertices)[index + 2] = sinf(angle);

            index += 3;
        }

        // do generate texture coordinates?
        if (pVertexFormat->m_UseTextures)
        {
            // add texture coordinates data to buffer
            (*pVertices)[index]     = 1.0f / i;
            (*pVertices)[index + 1] = 0.0f;

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

        // set vertex data
        (*pVertices)[index]     =  *pRadius * cosf(angle);
        (*pVertices)[index + 1] = (*pHeight / 2.0f);
        (*pVertices)[index + 2] =  *pRadius * sinf(angle);

        index += 3;

        // do generate normals?
        if (pVertexFormat->m_UseNormals)
        {
            // set normals
            (*pVertices)[index]     = cosf(angle);
            (*pVertices)[index + 1] = 0.0f;
            (*pVertices)[index + 2] = sinf(angle);

            index += 3;
        }

        // do generate texture coordinates?
        if (pVertexFormat->m_UseTextures)
        {
            // add texture coordinates data to buffer
            (*pVertices)[index]     = 1.0f / i;
            (*pVertices)[index + 1] = 1.0f;

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

#endif // MiniShapesH
