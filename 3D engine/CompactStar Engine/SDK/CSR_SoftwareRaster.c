/****************************************************************************
 * ==> CSR_SoftwareRaster --------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a small software rasterizer, that may *
 *               be used to replace OpenGL for simple rendering             *
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

#include "CSR_SoftwareRaster.h"

// std
#include <stdlib.h>
#include <math.h>
#include <string.h>

//---------------------------------------------------------------------------
CSR_FrameBuffer* csrFrameBufferCreate(size_t width, size_t height)
{
    // create a frame buffer
    CSR_FrameBuffer* pFB = (CSR_FrameBuffer*)malloc(sizeof(CSR_FrameBuffer));

    // succeeded?
    if (!pFB)
        return 0;

    // initialize the frame buffer content
    if (!csrFrameBufferInit(width, height, pFB))
    {
        csrFrameBufferRelease(pFB);
        return 0;
    }

    return pFB;
}
//---------------------------------------------------------------------------
int csrFrameBufferInit(size_t width, size_t height, CSR_FrameBuffer* pFB)
{
    // calculate the buffer size to create
    const size_t size = width * height;

    // validate the input
    if (!size || !pFB)
        return 0;

    // create the pixel array
    pFB->m_pPixel = (CSR_Pixel*)malloc(size * sizeof(CSR_Pixel));

    // succeeded?
    if (!pFB->m_pPixel)
    {
        pFB->m_Width  = 0;
        pFB->m_Height = 0;
        pFB->m_Size   = 0;

        return 0;
    }

    // populate the frame buffer
    pFB->m_Width  = width;
    pFB->m_Height = height;
    pFB->m_Size   = size;

    return 1;
}
//---------------------------------------------------------------------------
void csrFrameBufferRelease(CSR_FrameBuffer* pFB)
{
    // nothing to release?
    if (!pFB)
        return;

    // release the pixel buffer
    if (pFB->m_pPixel)
        free(pFB->m_pPixel);

    // release the frame buffer
    free(pFB);
}
//---------------------------------------------------------------------------
void csrFrameBufferClear(CSR_FrameBuffer* pFB, const CSR_Pixel* pPixel)
{
    size_t i;

    // validate the input
    if (!pFB || !pFB->m_pPixel)
        return;

    // fill the buffer with the pixel color value
    for (i = 0; i < pFB->m_Size; ++i)
        memcpy(&pFB->m_pPixel[i], pPixel, sizeof(CSR_Pixel));
}
//---------------------------------------------------------------------------
CSR_DepthBuffer* csrDepthBufferCreate(size_t width, size_t height)
{
    // create a depth buffer
    CSR_DepthBuffer* pDB = (CSR_DepthBuffer*)malloc(sizeof(CSR_DepthBuffer));

    // succeeded?
    if (!pDB)
        return 0;

    // initialize the depth buffer content
    if (!csrDepthBufferInit(width, height, pDB))
    {
        csrDepthBufferRelease(pDB);
        return 0;
    }

    return pDB;
}
//---------------------------------------------------------------------------
int csrDepthBufferInit(size_t width, size_t height, CSR_DepthBuffer* pDB)
{
    // calculate the buffer size to create
    const size_t size = width * height;

    // validate the input
    if (!size || !pDB)
        return 0;

    // create the depth data
    pDB->m_pData = (float*)malloc(size * sizeof(float));

    // succeeded?
    if (!pDB->m_pData)
    {
        pDB->m_Width  = 0;
        pDB->m_Height = 0;
        pDB->m_Size   = 0;

        return 0;
    }

    // populate the frame buffer
    pDB->m_Width  = width;
    pDB->m_Height = height;
    pDB->m_Size   = size;

    return 1;
}
//---------------------------------------------------------------------------
void csrDepthBufferRelease(CSR_DepthBuffer* pDB)
{
    // nothing to release?
    if (!pDB)
        return;

    // release the data
    if (pDB->m_pData)
        free(pDB->m_pData);

    // release the depth buffer
    free(pDB);
}
//---------------------------------------------------------------------------
void csrDepthBufferClear(CSR_DepthBuffer* pDB, float zFar)
{
    size_t i;

    // validate the input
    if (!pDB || !pDB->m_pData)
        return;

    // fill the buffer with the far clipping plane value
    for (i = 0; i < pDB->m_Size; ++i)
        memcpy(&pDB->m_pData[i], &zFar, sizeof(float));
}
//---------------------------------------------------------------------------
void csrRasterInit(CSR_Raster* pRaster)
{
    // no raster to initialize?
    if (!pRaster)
        return;

    pRaster->m_ApertureWidth  = 0.980f; // 35mm full aperture in inches
    pRaster->m_ApertureHeight = 0.735f; // 35mm full aperture in inches
    pRaster->m_FocalLength    = 20.0f;  // focal length in mm
    pRaster->m_Type           = CSR_RT_Overscan;
}
//---------------------------------------------------------------------------
void csrRasterFindMin(float a, float b, float c, float* pR)
{
    if (a < b && a < c)
    {
        *pR = a;
        return;
    }

    if (b < c)
    {
        *pR = b;
        return;
    }

    *pR = c;
}
//---------------------------------------------------------------------------
void csrRasterFindMax(float a, float b, float c, float* pR)
{
    if (a > b && a > c)
    {
        *pR = a;
        return;
    }

    if (b > c)
    {
        *pR = b;
        return;
    }

    *pR = c;
}
//---------------------------------------------------------------------------
void csrRasterFindEdge(const CSR_Vector3* pV1,
                       const CSR_Vector3* pV2,
                       const CSR_Vector3* pV3,
                             float*       pR)
{
    *pR = ((pV3->m_X - pV1->m_X) * (pV2->m_Y - pV1->m_Y)) -
          ((pV3->m_Y - pV1->m_Y) * (pV2->m_X - pV1->m_X));
}
//---------------------------------------------------------------------------
void csrRasterGetScreenCoordinates(const CSR_Raster* pRaster,
                                         float       imageWidth,
                                         float       imageHeight,
                                         float       zNear,
                                         CSR_Rect*   pScreenRect)
{
          float xScale;
          float yScale;
    const float filmAspectRatio   = pRaster->m_ApertureWidth / pRaster->m_ApertureHeight;
    const float deviceAspectRatio = imageWidth               / imageHeight;
    const float inchToMm          = 25.4f;

    // validate the input
    if (!pRaster || !pScreenRect)
        return;

    // calculate the right and top edges
    pScreenRect->m_Min.m_Y = ((pRaster->m_ApertureHeight * inchToMm / 2.0f) / pRaster->m_FocalLength) * zNear;
    pScreenRect->m_Max.m_X = ((pRaster->m_ApertureWidth  * inchToMm / 2.0f) / pRaster->m_FocalLength) * zNear;

    xScale = 1;
    yScale = 1;

    // calculate the aspect ratio to apply
    switch (pRaster->m_Type)
    {
        case CSR_RT_Overscan:
            if (filmAspectRatio > deviceAspectRatio)
                yScale = filmAspectRatio / deviceAspectRatio;
            else
                xScale = deviceAspectRatio / filmAspectRatio;

            break;

        case CSR_RT_Fill:
        default:
            if (filmAspectRatio > deviceAspectRatio)
                xScale = deviceAspectRatio / filmAspectRatio;
            else
                yScale = filmAspectRatio / deviceAspectRatio;

            break;
    }

    // apply the ratio to the right and top edges
    pScreenRect->m_Max.m_X *= xScale;
    pScreenRect->m_Min.m_Y *= yScale;

    // calculate the left and bottom edges
    pScreenRect->m_Max.m_Y = -pScreenRect->m_Min.m_Y;
    pScreenRect->m_Min.m_X = -pScreenRect->m_Max.m_X;
}
//---------------------------------------------------------------------------
void csrRasterRasterizeVertex(const CSR_Vector3* pInVertex,
                              const CSR_Matrix4* pMatrix,
                              const CSR_Rect*    pScreenRect,
                                    float        zNear,
                                    float        imageWidth,
                                    float        imageHeight,
                                    CSR_Vector3* pOutVertex)
{
    #ifdef _MSC_VER
        CSR_Vector3 vertexCamera = {0};
        CSR_Vector2 vertexScreen = {0};
        CSR_Vector2 vertexNDC    = {0};
        float       subRightLeft;
        float       addRightLeft;
        float       subTopBottom;
        float       addTopBottom;
    #else
        CSR_Vector3 vertexCamera;
        CSR_Vector2 vertexScreen;
        CSR_Vector2 vertexNDC;
        float       subRightLeft;
        float       addRightLeft;
        float       subTopBottom;
        float       addTopBottom;
    #endif

    // validate the input
    if (!pInVertex || !pMatrix || !pScreenRect || !pOutVertex)
        return;

    // transform the input vertex into the camera space
    csrMat4Transform(pMatrix, pInVertex, &vertexCamera);

    // transfrom the camera vertex to a point in the screen space
    vertexScreen.m_X = (zNear * vertexCamera.m_X) / -vertexCamera.m_Z;
    vertexScreen.m_Y = (zNear * vertexCamera.m_Y) / -vertexCamera.m_Z;

    subRightLeft = pScreenRect->m_Max.m_X - pScreenRect->m_Min.m_X;
    addRightLeft = pScreenRect->m_Max.m_X + pScreenRect->m_Min.m_X;
    subTopBottom = pScreenRect->m_Min.m_Y - pScreenRect->m_Max.m_Y;
    addTopBottom = pScreenRect->m_Min.m_Y + pScreenRect->m_Max.m_Y;

    // convert point from screen space to Normalized Device Coordinates (NDC) space (in range [-1, 1])
    vertexNDC.m_X = ((2.0f * vertexScreen.m_X) / subRightLeft) - (addRightLeft / subRightLeft);
    vertexNDC.m_Y = ((2.0f * vertexScreen.m_Y) / subTopBottom) - (addTopBottom / subTopBottom);

    // convert to raster space. NOTE in raster space y is down, so the direction is inverted
    pOutVertex->m_X = (vertexNDC.m_X + 1.0f) / 2.0f * imageWidth;
    pOutVertex->m_Y = (1.0f - vertexNDC.m_Y) / 2.0f * imageHeight;
    pOutVertex->m_Z = -vertexCamera.m_Z;
}
//---------------------------------------------------------------------------
int csrRasterGetPolygon(const CSR_Matrix4*             pMatrix,
                              size_t                   v1Index,
                              size_t                   v2Index,
                              size_t                   v3Index,
                        const CSR_VertexBuffer*        pVB,
                              CSR_Polygon3*            pPolygon,
                              CSR_Vector3*             pNormal,
                              CSR_Vector2*             pST,
                              CSR_Color*               pColor,
                        const CSR_fOnApplyVertexShader fOnApplyVertexShader)
{
    size_t offset;
    size_t i;

    // validate the input
    if (!pVB || !pPolygon || !pNormal || !pST || !pColor)
        return 0;

    offset = 0;

    // extract the polygon from source vertex buffer
    pPolygon->m_Vertex[0].m_X = pVB->m_pData[v1Index];
    pPolygon->m_Vertex[0].m_Y = pVB->m_pData[v1Index + 1];
    pPolygon->m_Vertex[0].m_Z = pVB->m_pData[v1Index + 2];
    pPolygon->m_Vertex[1].m_X = pVB->m_pData[v2Index];
    pPolygon->m_Vertex[1].m_Y = pVB->m_pData[v2Index + 1];
    pPolygon->m_Vertex[1].m_Z = pVB->m_pData[v2Index + 2];
    pPolygon->m_Vertex[2].m_X = pVB->m_pData[v3Index];
    pPolygon->m_Vertex[2].m_Y = pVB->m_pData[v3Index + 1];
    pPolygon->m_Vertex[2].m_Z = pVB->m_pData[v3Index + 2];

    offset += 3;

    // extract the normal from source vertex buffer
    if (pVB->m_Format.m_HasNormal)
    {
        pNormal[0].m_X = pVB->m_pData[v1Index + offset];
        pNormal[0].m_Y = pVB->m_pData[v1Index + offset + 1];
        pNormal[0].m_Z = pVB->m_pData[v1Index + offset + 2];
        pNormal[1].m_X = pVB->m_pData[v2Index + offset];
        pNormal[1].m_Y = pVB->m_pData[v2Index + offset + 1];
        pNormal[1].m_Z = pVB->m_pData[v2Index + offset + 2];
        pNormal[2].m_X = pVB->m_pData[v3Index + offset];
        pNormal[2].m_Y = pVB->m_pData[v3Index + offset + 1];
        pNormal[2].m_Z = pVB->m_pData[v3Index + offset + 2];

        offset += 3;
    }
    else
    {
        pNormal[0].m_X = 0.0f;
        pNormal[0].m_Y = 0.0f;
        pNormal[0].m_Z = 0.0f;
        pNormal[1].m_X = 0.0f;
        pNormal[1].m_Y = 0.0f;
        pNormal[1].m_Z = 0.0f;
        pNormal[2].m_X = 0.0f;
        pNormal[2].m_Y = 0.0f;
        pNormal[2].m_Z = 0.0f;
    }

    // extract the texture coordinates from source vertex buffer
    if (pVB->m_Format.m_HasTexCoords)
    {
        pST[0].m_X = pVB->m_pData[v1Index + offset];
        pST[0].m_Y = pVB->m_pData[v1Index + offset + 1];
        pST[1].m_X = pVB->m_pData[v2Index + offset];
        pST[1].m_Y = pVB->m_pData[v2Index + offset + 1];
        pST[2].m_X = pVB->m_pData[v3Index + offset];
        pST[2].m_Y = pVB->m_pData[v3Index + offset + 1];

        offset += 2;
    }
    else
    {
        pST[0].m_X = 0.0f;
        pST[0].m_Y = 0.0f;
        pST[1].m_X = 0.0f;
        pST[1].m_Y = 0.0f;
        pST[2].m_X = 0.0f;
        pST[2].m_Y = 0.0f;
    }

    // extract the color from source vertex buffer
    if (pVB->m_Format.m_HasPerVertexColor)
    {
        pColor[0].m_R = pVB->m_pData[v1Index + offset];
        pColor[0].m_G = pVB->m_pData[v1Index + offset + 1];
        pColor[0].m_B = pVB->m_pData[v1Index + offset + 2];
        pColor[0].m_A = pVB->m_pData[v1Index + offset + 3];
        pColor[1].m_R = pVB->m_pData[v2Index + offset];
        pColor[1].m_G = pVB->m_pData[v2Index + offset + 1];
        pColor[1].m_B = pVB->m_pData[v2Index + offset + 2];
        pColor[1].m_A = pVB->m_pData[v2Index + offset + 3];
        pColor[2].m_R = pVB->m_pData[v3Index + offset];
        pColor[2].m_G = pVB->m_pData[v3Index + offset + 1];
        pColor[2].m_B = pVB->m_pData[v3Index + offset + 2];
        pColor[2].m_A = pVB->m_pData[v3Index + offset + 3];
    }
    else
    {
        pColor[0].m_R = 0.0f;
        pColor[0].m_G = 0.0f;
        pColor[0].m_B = 0.0f;
        pColor[0].m_A = 0.0f;
        pColor[1].m_R = 0.0f;
        pColor[1].m_G = 0.0f;
        pColor[1].m_B = 0.0f;
        pColor[1].m_A = 0.0f;
        pColor[2].m_R = 0.0f;
        pColor[2].m_G = 0.0f;
        pColor[2].m_B = 0.0f;
        pColor[2].m_A = 0.0f;
    }

    // for each vertex, apply the vertex shader
    if (fOnApplyVertexShader)
        for (i = 0; i < 3; ++i)
            fOnApplyVertexShader(pMatrix, &pPolygon->m_Vertex[i], &pNormal[i], &pST[i], &pColor[i]);

    return 1;
}
//---------------------------------------------------------------------------
int csrRasterDrawPolygon(const CSR_Polygon3*              pPolygon,
                         const CSR_Vector3*               pNormal,
                         const CSR_Vector2*               pST,
                         const CSR_Color*                 pColor,
                         const CSR_Matrix4*               pMatrix,
                               float                      zNear,
                               CSR_ECullingType           cullingType,
                               CSR_ECullingFace           cullingFace,
                         const CSR_Rect*                  pScreenRect,
                               CSR_FrameBuffer*           pFB,
                               CSR_DepthBuffer*           pDB,
                         const CSR_fOnApplyFragmentShader fOnApplyFragmentShader)
{
    #ifdef _MSC_VER
        float         xMin;
        float         yMin;
        float         xMax;
        float         yMax;
        float         xStart;
        float         yStart;
        float         xEnd;
        float         yEnd;
        float         area;
        float         w0;
        float         w1;
        float         w2;
        float         invZ;
        float         z;
        size_t        x;
        size_t        y;
        size_t        x0;
        size_t        x1;
        size_t        y0;
        size_t        y1;
        int           cullingMode;
        int           pixelVisible;
        CSR_Polygon3  rasterPoly  = {0};
        CSR_Vector2   st[3]       = {0};
        CSR_Vector2   stCoord     = {0};
        CSR_Vector3   pixelSample = {0};
        CSR_Vector3   sampler     = {0};
        CSR_Color     color       = {0};
    #else
        float         xMin;
        float         yMin;
        float         xMax;
        float         yMax;
        float         xStart;
        float         yStart;
        float         xEnd;
        float         yEnd;
        float         area;
        float         w0;
        float         w1;
        float         w2;
        float         invZ;
        float         z;
        size_t        x;
        size_t        y;
        size_t        x0;
        size_t        x1;
        size_t        y0;
        size_t        y1;
        int           cullingMode;
        int           pixelVisible;
        CSR_Polygon3  rasterPoly;
        CSR_Vector2   st[3];
        CSR_Vector2   stCoord;
        CSR_Vector3   pixelSample;
        CSR_Vector3   sampler;
        CSR_Color     color;
    #endif

    // validate the input
    if (!pPolygon || !pNormal || !pST || !pColor || !pMatrix || !pScreenRect || !pFB || !pDB)
        return 0;

    // rasterize the polygon
    csrRasterRasterizeVertex(&pPolygon->m_Vertex[0], pMatrix, pScreenRect, zNear, (float)pFB->m_Width, (float)pFB->m_Height, &rasterPoly.m_Vertex[0]);
    csrRasterRasterizeVertex(&pPolygon->m_Vertex[1], pMatrix, pScreenRect, zNear, (float)pFB->m_Width, (float)pFB->m_Height, &rasterPoly.m_Vertex[1]);
    csrRasterRasterizeVertex(&pPolygon->m_Vertex[2], pMatrix, pScreenRect, zNear, (float)pFB->m_Width, (float)pFB->m_Height, &rasterPoly.m_Vertex[2]);

    // check if the polygon is culled and determine the culling mode to use (0 = CW, 1 = CCW, 2 = both)
    switch (cullingType)
    {
        case CSR_CT_None:
            // both faces are accepted
            cullingType = 2;
            cullingMode = 0;
            break;

        case CSR_CT_Front:
        case CSR_CT_Back:
        {
            #ifdef _MSC_VER
                CSR_Plane   polygonPlane  = {0};
                CSR_Vector3 polygonNormal = {0};
                CSR_Vector3 cullingNormal = {0};
                float       cullingDot;
            #else
                CSR_Plane   polygonPlane;
                CSR_Vector3 polygonNormal;
                CSR_Vector3 cullingNormal;
                float       cullingDot;
            #endif

            // calculate the rasterized polygon plane
            csrPlaneFromPoints(&rasterPoly.m_Vertex[0],
                               &rasterPoly.m_Vertex[1],
                               &rasterPoly.m_Vertex[2],
                               &polygonPlane);

            // calculate the rasterized polygon surface normal
            polygonNormal.m_X = polygonPlane.m_A;
            polygonNormal.m_Y = polygonPlane.m_B;
            polygonNormal.m_Z = polygonPlane.m_C;

            // get the culling normal
            cullingNormal.m_X =  0.0f;
            cullingNormal.m_Y =  0.0f;
            cullingNormal.m_Z = -1.0f;

            // calculate the dot product between the culling and the polygon normal
            csrVec3Dot(&polygonNormal, &cullingNormal, &cullingDot);

            switch (cullingFace)
            {
                case CSR_CF_CW:
                    // is polygon rejected?
                    if (cullingDot <= 0.0f)
                        return 1;

                    // apply a clockwise culling
                    cullingMode = 0;
                    break;

                case CSR_CF_CCW:
                    // is polygon rejected?
                    if (cullingDot >= 0.0f)
                        return 1;

                    // apply a counter-clockwise culling
                    cullingMode = 1;
                    break;

                // error
                default:
                    return 1;
            }

            break;
        }

        case CSR_CT_Both:
        default:
            // both faces are rejected
            return 1;
    }

    // invert the vertex z-coordinate (to allow multiplication later instead of division)
    rasterPoly.m_Vertex[0].m_Z = 1.0f / rasterPoly.m_Vertex[0].m_Z;
    rasterPoly.m_Vertex[1].m_Z = 1.0f / rasterPoly.m_Vertex[1].m_Z;
    rasterPoly.m_Vertex[2].m_Z = 1.0f / rasterPoly.m_Vertex[2].m_Z;

    // calculate the texture coordinates, divide them by their vertex z-coordinate
    st[0].m_X = pST[0].m_X * rasterPoly.m_Vertex[0].m_Z;
    st[0].m_Y = pST[0].m_Y * rasterPoly.m_Vertex[0].m_Z;
    st[1].m_X = pST[1].m_X * rasterPoly.m_Vertex[1].m_Z;
    st[1].m_Y = pST[1].m_Y * rasterPoly.m_Vertex[1].m_Z;
    st[2].m_X = pST[2].m_X * rasterPoly.m_Vertex[2].m_Z;
    st[2].m_Y = pST[2].m_Y * rasterPoly.m_Vertex[2].m_Z;

    // calculate the polygon bounding rect
    csrRasterFindMin(rasterPoly.m_Vertex[0].m_X, rasterPoly.m_Vertex[1].m_X, rasterPoly.m_Vertex[2].m_X, &xMin);
    csrRasterFindMin(rasterPoly.m_Vertex[0].m_Y, rasterPoly.m_Vertex[1].m_Y, rasterPoly.m_Vertex[2].m_Y, &yMin);
    csrRasterFindMax(rasterPoly.m_Vertex[0].m_X, rasterPoly.m_Vertex[1].m_X, rasterPoly.m_Vertex[2].m_X, &xMax);
    csrRasterFindMax(rasterPoly.m_Vertex[0].m_Y, rasterPoly.m_Vertex[1].m_Y, rasterPoly.m_Vertex[2].m_Y, &yMax);

    // is the polygon out of screen?
    if (xMin > (float)(pFB->m_Width  - 1) || xMax < 0.0f ||
        yMin > (float)(pFB->m_Height - 1) || yMax < 0.0f)
        return 1;

    // calculate the area to draw
    csrMathMax(0.0f,                       xMin, &xStart);
    csrMathMin((float)(pFB->m_Width  - 1), xMax, &xEnd);
    csrMathMax(0.0f,                       yMin, &yStart);
    csrMathMin((float)(pFB->m_Height - 1), yMax, &yEnd);

    #ifdef __CODEGEARC__
        x0 = (size_t)floor(xStart);
        x1 = (size_t)floor(xEnd);
        y0 = (size_t)floor(yStart);
        y1 = (size_t)floor(yEnd);
    #else
        x0 = (size_t)floorf(xStart);
        x1 = (size_t)floorf(xEnd);
        y0 = (size_t)floorf(yStart);
        y1 = (size_t)floorf(yEnd);
    #endif

    // calculate the triangle area (multiplied by 2)
    csrRasterFindEdge(&rasterPoly.m_Vertex[0], &rasterPoly.m_Vertex[1], &rasterPoly.m_Vertex[2], &area);

    // iterate through pixels to draw
    for (y = y0; y <= y1; ++y)
        for (x = x0; x <= x1; ++x)
        {
            pixelSample.m_X = x + 0.5f;
            pixelSample.m_Y = y + 0.5f;
            pixelSample.m_Z =     0.0f;

            // calculate the sub-triangle areas (multiplied by 2)
            csrRasterFindEdge(&rasterPoly.m_Vertex[1], &rasterPoly.m_Vertex[2], &pixelSample, &w0);
            csrRasterFindEdge(&rasterPoly.m_Vertex[2], &rasterPoly.m_Vertex[0], &pixelSample, &w1);
            csrRasterFindEdge(&rasterPoly.m_Vertex[0], &rasterPoly.m_Vertex[1], &pixelSample, &w2);

            pixelVisible = 0;

            // check if the pixel is visible. The culling mode is important to determine the sign
            switch (cullingMode)
            {
                // clockwise
                case 0:
                    if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                        pixelVisible = 1;

                    break;

                // counter-clockwise
                case 1:
                    if (w0 <= 0 && w1 <= 0 && w2 <= 0)
                    {
                        pixelVisible = 1;

                        // invert the sampler values
                        w0 = -w0;
                        w1 = -w1;
                        w2 = -w2;
                    }

                    break;

                // both
                case 2:
                    if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                        pixelVisible = 1;
                    else
                    if (w0 <= 0 && w1 <= 0 && w2 <= 0)
                    {
                        pixelVisible = 1;

                        // invert the sampler values
                        w0 = -w0;
                        w1 = -w1;
                        w2 = -w2;
                    }

                    break;

                // error
                default:
                    return 0;
            }

            // is pixel visible?
            if (pixelVisible)
            {
                // calculate the barycentric coordinates, which are the areas of the sub-triangles
                // divided by the area of the main triangle
                w0 /= area;
                w1 /= area;
                w2 /= area;

                // calculate the pixel depth
                invZ = (rasterPoly.m_Vertex[0].m_Z * w0) +
                       (rasterPoly.m_Vertex[1].m_Z * w1) +
                       (rasterPoly.m_Vertex[2].m_Z * w2);
                z    = 1.0f / invZ;

                // test the pixel against the depth buffer
                if (z < pDB->m_pData[y * pFB->m_Width + x])
                {
                    // test passed, update the depth buffer
                    pDB->m_pData[y * pFB->m_Width + x] = z;

                    // calculate the default pixel color, based on the per-vertex color
                    color.m_R = w0 * pColor[0].m_R + w1 * pColor[1].m_R + w2 * pColor[2].m_R;
                    color.m_G = w0 * pColor[0].m_G + w1 * pColor[1].m_G + w2 * pColor[2].m_G;
                    color.m_B = w0 * pColor[0].m_B + w1 * pColor[1].m_B + w2 * pColor[2].m_B;

                    // calculate the texture coordinate
                    stCoord.m_X = ((st[0].m_X * w0) + (st[1].m_X * w1) + (st[2].m_X * w2)) * z;
                    stCoord.m_Y = ((st[0].m_Y * w0) + (st[1].m_Y * w1) + (st[2].m_Y * w2)) * z;

                    // for each pixel, apply the fragment shader
                    if (fOnApplyFragmentShader)
                    {
                        // set the sampler items
                        sampler.m_X = w0;
                        sampler.m_Y = w1;
                        sampler.m_Z = w2;

                        fOnApplyFragmentShader(pMatrix,
                                               pPolygon,
                                               &stCoord,
                                               &sampler,
                                               z,
                                               &color);
                    }

                    // limit the color components between 0.0 and 1.0
                    csrMathClamp(color.m_R, 0.0, 1.0, &color.m_R);
                    csrMathClamp(color.m_G, 0.0, 1.0, &color.m_G);
                    csrMathClamp(color.m_B, 0.0, 1.0, &color.m_B);

                    // write the final pixel inside the frame buffer
                    pFB->m_pPixel[y * pFB->m_Width + x].m_R = (unsigned char)(color.m_R * 255.0f);
                    pFB->m_pPixel[y * pFB->m_Width + x].m_G = (unsigned char)(color.m_G * 255.0f);
                    pFB->m_pPixel[y * pFB->m_Width + x].m_B = (unsigned char)(color.m_B * 255.0f);
                    pFB->m_pPixel[y * pFB->m_Width + x].m_A = (unsigned char)(color.m_A * 255.0f);
                }
            }
        }

    return 1;
}
//---------------------------------------------------------------------------
int csrRasterDraw(const CSR_Matrix4*               pMatrix,
                        float                      zNear,
                        float                      zFar,
                  const CSR_VertexBuffer*          pVB,
                  const CSR_Raster*                pRaster,
                        CSR_FrameBuffer*           pFB,
                        CSR_DepthBuffer*           pDB,
                  const CSR_fOnApplyVertexShader   fOnApplyVertexShader,
                  const CSR_fOnApplyFragmentShader fOnApplyFragmentShader)
{
    size_t       i;
    size_t       index;
    CSR_Rect     screenRect;
    CSR_Polygon3 polygon;
    CSR_Vector3  normal[3];
    CSR_Vector2  st[3];
    CSR_Color    color[3];

    // validate the input
    if (!pMatrix || !pVB || !pVB->m_Format.m_Stride || !pRaster || !pFB || !pDB)
        return 0;

    // get the raster screen coordinates
    csrRasterGetScreenCoordinates(pRaster,
                                  (float)pFB->m_Width,
                                  (float)pFB->m_Height,
                                  zNear,
                                 &screenRect);

    // search for vertex type
    switch (pVB->m_Format.m_Type)
    {
        case CSR_VT_Triangles:
        {
            // calculate iteration step
            const unsigned step = (pVB->m_Format.m_Stride * 3);

            // iterate through source vertices
            for (i = 0; i < pVB->m_Count; i += step)
            {
                // get the next polygon to draw
                if (!csrRasterGetPolygon(pMatrix,
                                         i,
                                         i +  (size_t)pVB->m_Format.m_Stride,
                                         i + ((size_t)pVB->m_Format.m_Stride * 2),
                                         pVB,
                                        &polygon,
                                         normal,
                                         st,
                                         color,
                                         fOnApplyVertexShader))
                    return 0;

                // draw the polygon
                if (!csrRasterDrawPolygon(&polygon,
                                           normal,
                                           st,
                                           color,
                                           pMatrix,
                                           zNear,
                                           pVB->m_Culling.m_Type,
                                           pVB->m_Culling.m_Face,
                                          &screenRect,
                                           pFB,
                                           pDB,
                                           fOnApplyFragmentShader))
                    return 0;
            }

            return 1;
        }

        case CSR_VT_TriangleStrip:
        {
            // calculate length to read in triangle strip buffer
            const unsigned stripLength = (unsigned)(pVB->m_Count - ((size_t)pVB->m_Format.m_Stride * 2));

            index = 0;

            // iterate through source vertices
            for (i = 0; i < stripLength; i += pVB->m_Format.m_Stride)
            {
                // extract polygon from source buffer, revert odd polygons
                if (!index || !(index % 2))
                {
                    // get the next polygon to draw
                    if (!csrRasterGetPolygon(pMatrix,
                                             i,
                                             i +  (size_t)pVB->m_Format.m_Stride,
                                             i + ((size_t)pVB->m_Format.m_Stride * 2),
                                             pVB,
                                            &polygon,
                                             normal,
                                             st,
                                             color,
                                             fOnApplyVertexShader))
                        return 0;
                }
                else
                {
                    // get the next polygon to draw
                    if (!csrRasterGetPolygon(pMatrix,
                                             i +  (size_t)pVB->m_Format.m_Stride,
                                             i,
                                             i + ((size_t)pVB->m_Format.m_Stride * 2),
                                             pVB,
                                            &polygon,
                                             normal,
                                             st,
                                             color,
                                             fOnApplyVertexShader))
                        return 0;
                }

                // draw the polygon
                if (!csrRasterDrawPolygon(&polygon,
                                           normal,
                                           st,
                                           color,
                                           pMatrix,
                                           zNear,
                                           pVB->m_Culling.m_Type,
                                           pVB->m_Culling.m_Face,
                                          &screenRect,
                                           pFB,
                                           pDB,
                                           fOnApplyFragmentShader))
                    return 0;

                ++index;
            }

            return 1;
        }

        case CSR_VT_TriangleFan:
        {
            // calculate length to read in triangle fan buffer
            const unsigned fanLength = (unsigned)(pVB->m_Count - pVB->m_Format.m_Stride);

            // iterate through source vertices
            for (i = pVB->m_Format.m_Stride; i < fanLength; i += pVB->m_Format.m_Stride)
            {
                // get the next polygon to draw
                if (!csrRasterGetPolygon(pMatrix,
                                         0,
                                         i,
                                         i + pVB->m_Format.m_Stride,
                                         pVB,
                                        &polygon,
                                         normal,
                                         st,
                                         color,
                                         fOnApplyVertexShader))
                    return 0;

                // draw the polygon
                if (!csrRasterDrawPolygon(&polygon,
                                           normal,
                                           st,
                                           color,
                                           pMatrix,
                                           zNear,
                                           pVB->m_Culling.m_Type,
                                           pVB->m_Culling.m_Face,
                                          &screenRect,
                                           pFB,
                                           pDB,
                                           fOnApplyFragmentShader))
                    return 0;
            }

            return 1;
        }

        case CSR_VT_Quads:
        {
            // calculate iteration step
            const unsigned step = (pVB->m_Format.m_Stride * 4);

            // iterate through source vertices
            for (i = 0; i < pVB->m_Count; i += step)
            {
                // calculate vertices position
                const unsigned v1 = (unsigned) i;
                const unsigned v2 = (unsigned)(i +  (size_t)pVB->m_Format.m_Stride);
                const unsigned v3 = (unsigned)(i + ((size_t)pVB->m_Format.m_Stride * 2));
                const unsigned v4 = (unsigned)(i + ((size_t)pVB->m_Format.m_Stride * 3));

                // get the next polygon to draw
                if (!csrRasterGetPolygon(pMatrix,
                                         v1,
                                         v2,
                                         v3,
                                         pVB,
                                        &polygon,
                                         normal,
                                         st,
                                         color,
                                         fOnApplyVertexShader))
                    return 0;

                // draw the polygon
                if (!csrRasterDrawPolygon(&polygon,
                                           normal,
                                           st,
                                           color,
                                           pMatrix,
                                           zNear,
                                           pVB->m_Culling.m_Type,
                                           pVB->m_Culling.m_Face,
                                          &screenRect,
                                           pFB,
                                           pDB,
                                           fOnApplyFragmentShader))
                    return 0;

                // get the next polygon to draw
                if (!csrRasterGetPolygon(pMatrix,
                                         v3,
                                         v2,
                                         v4,
                                         pVB,
                                        &polygon,
                                         normal,
                                         st,
                                         color,
                                         fOnApplyVertexShader))
                    return 0;

                // draw the polygon
                if (!csrRasterDrawPolygon(&polygon,
                                           normal,
                                           st,
                                           color,
                                           pMatrix,
                                           zNear,
                                           pVB->m_Culling.m_Type,
                                           pVB->m_Culling.m_Face,
                                          &screenRect,
                                           pFB,
                                           pDB,
                                           fOnApplyFragmentShader))
                    return 0;
            }

            return 1;
        }

        case CSR_VT_QuadStrip:
        {
            // calculate iteration step
            const unsigned step = (pVB->m_Format.m_Stride * 2);

            // calculate length to read in triangle strip buffer
            const unsigned stripLength = (unsigned)(pVB->m_Count - ((size_t)pVB->m_Format.m_Stride * 2));

            // iterate through source vertices
            for (i = 0; i < stripLength; i += step)
            {
                // calculate vertices position
                const unsigned v1 = (unsigned) i;
                const unsigned v2 = (unsigned)(i +  (size_t)pVB->m_Format.m_Stride);
                const unsigned v3 = (unsigned)(i + ((size_t)pVB->m_Format.m_Stride * 2));
                const unsigned v4 = (unsigned)(i + ((size_t)pVB->m_Format.m_Stride * 3));

                // get the next polygon to draw
                if (!csrRasterGetPolygon(pMatrix,
                                         v1,
                                         v2,
                                         v3,
                                         pVB,
                                        &polygon,
                                         normal,
                                         st,
                                         color,
                                         fOnApplyVertexShader))
                    return 0;

                // draw the polygon
                if (!csrRasterDrawPolygon(&polygon,
                                           normal,
                                           st,
                                           color,
                                           pMatrix,
                                           zNear,
                                           pVB->m_Culling.m_Type,
                                           pVB->m_Culling.m_Face,
                                          &screenRect,
                                           pFB,
                                           pDB,
                                           fOnApplyFragmentShader))
                    return 0;

                // get the next polygon to draw
                if (!csrRasterGetPolygon(pMatrix,
                                         v3,
                                         v2,
                                         v4,
                                         pVB,
                                        &polygon,
                                         normal,
                                         st,
                                         color,
                                         fOnApplyVertexShader))
                    return 0;

                // draw the polygon
                if (!csrRasterDrawPolygon(&polygon,
                                           normal,
                                           st,
                                           color,
                                           pMatrix,
                                           zNear,
                                           pVB->m_Culling.m_Type,
                                           pVB->m_Culling.m_Face,
                                          &screenRect,
                                           pFB,
                                           pDB,
                                           fOnApplyFragmentShader))
                    return 0;
            }

            return 1;
        }

        default:
            return 0;
    }
}
//---------------------------------------------------------------------------
