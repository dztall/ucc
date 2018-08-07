/****************************************************************************
 * ==> CSR_SoftwareRaster --------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a small software rasterizer, that may *
 *               be used to replace OpenGL for simple rendering             *
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

#ifndef CSR_SoftwareRasterH
#define CSR_SoftwareRasterH

// std
#include <stddef.h>

// compactStart engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Raster type enumeration
*/
typedef enum
{
    CSR_RT_Fill = 0,
    CSR_RT_Overscan
} CSR_ERasterType;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* A simple RGBA pixel to use in the frame buffer
*/
typedef struct
{
    unsigned char m_R;
    unsigned char m_G;
    unsigned char m_B;
    unsigned char m_A;
} CSR_Pixel;

/**
* Frame buffer, that will contain the rendered scene
*/
typedef struct
{
    CSR_Pixel* m_pPixel;
    size_t     m_Width;
    size_t     m_Height;
    size_t     m_Size;
} CSR_FrameBuffer;

/**
* Depth buffer
*/
typedef struct
{
    float* m_pData;
    size_t m_Width;
    size_t m_Height;
    size_t m_Size;
} CSR_DepthBuffer;

/**
* Rasterizer options
*/
typedef struct
{
    float           m_ApertureWidth;  // in inches
    float           m_ApertureHeight; // in inches
    float           m_FocalLength;    // in mm
    CSR_ERasterType m_Type;
} CSR_Raster;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when the vertex shader should be applied
*@param pMatrix - matrix
*@param[in, out] pVertex - vertex
*@param[in, out] pNormal - vertex normal
*@param[in, out] pST - vertex texture coordinate
*@param[in, out] pColor - vertex color
*/
typedef void (*CSR_fOnApplyVertexShader)(const CSR_Matrix4* pMatrix,

                                               CSR_Vector3* pVertex,

                                               CSR_Vector3* pNormal,
                                               CSR_Vector2* pST,
                                               CSR_Color*   pColor);

/**
* Called when the fragment shader should be applied
*@param pMatrix - matrix
*@param pPolygon - polygon currently drawing
*@param pST - texture coordinate matching with the pixel
*@param pSampler - sampler items (x = w0, y = w1, z = w2)
*@param z - pixel z order
*@param[in, out] pColor - pixel color
*/
typedef void (*CSR_fOnApplyFragmentShader)(const CSR_Matrix4*  pMatrix,
                                           const CSR_Polygon3* pPolygon,
                                           const CSR_Vector2*  pST,
                                           const CSR_Vector3*  pSampler,
                                                 float         z,
                                                 CSR_Color*    pColor);

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Frame buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates a frame buffer
        *@param width - scene width in pixels
        *@param height - scene height in pixels
        *@return Newly created frame buffer, 0 on error
        *@note The frame buffer must be released when no longer used, see csrFrameBufferRelease()
        */
        CSR_FrameBuffer* csrFrameBufferCreate(size_t width, size_t height);

        /**
        * Initializes a frame buffer
        *@param width - scene width in pixels
        *@param height - scene height in pixels
        *@param[in, out] pFB - the frame buffer to initialize
        *@return 1 on sucess, otherwise 0
        */
        int csrFrameBufferInit(size_t width, size_t height, CSR_FrameBuffer* pFB);

        /**
        * Releases a frame buffer
        *@param[in, out] pFB - the frame buffer to release
        */
        void csrFrameBufferRelease(CSR_FrameBuffer* pFB);

        /**
        * Clears a frame buffer
        *@param[in, out] pFB - the frame buffer to clear
        *@param pPixel - pixel sample with which the frame buffer will be filled
        */
        void csrFrameBufferClear(CSR_FrameBuffer* pFB, const CSR_Pixel* pPixel);

        //-------------------------------------------------------------------
        // Depth buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates a depth buffer
        *@param width - scene width in pixels
        *@param height - scene height in pixels
        *@return Newly created depth buffer, 0 on error
        *@note The depth buffer must be released when no longer used, see csrDepthBufferRelease()
        */
        CSR_DepthBuffer* csrDepthBufferCreate(size_t width, size_t height);

        /**
        * Initializes a depth buffer
        *@param width - scene width in pixels
        *@param height - scene height in pixels
        *@param[in, out] pDB - the depth buffer to initialize
        *@return 1 on sucess, otherwise 0
        */
        int csrDepthBufferInit(size_t width, size_t height, CSR_DepthBuffer* pDB);

        /**
        * Releases a depth buffer
        *@param[in, out] pDB - the depth buffer to release
        */
        void csrDepthBufferRelease(CSR_DepthBuffer* pDB);

        /**
        * Clears a depth buffer
        *@param[in, out] pDB - the depth buffer to clear
        *@param zFar - the far clipping plane value with which the depth buffer will be filled
        */
        void csrDepthBufferClear(CSR_DepthBuffer* pDB, float zFar);

        //-------------------------------------------------------------------
        // Raster functions
        //-------------------------------------------------------------------

        /**
        * Initializes the rasterizer options
        *@param[in, out] pRaster - the raster options to initialize
        */
        void csrRasterInit(CSR_Raster* pRaster);

        /**
        * Finds the minimum value between 3 values
        *@param a - first value to check
        *@param b - second value to check
        *@param c - third value to check
        *@param[out] pR - the minimum found value
        */
        void csrRasterFindMin(float a, float b, float c, float* pR);

        /**
        * Finds the maximum value between 3 values
        *@param a - first value to check
        *@param b - second value to check
        *@param c - third value to check
        *@param[out] pR - the maximum found value
        */
        void csrRasterFindMax(float a, float b, float c, float* pR);

        /**
        * Finds the edge value of a polygon
        *@param pV1 - first polygon vertex
        *@param pV2 - second polygon vertex
        *@param pV3 - third polygon vertex
        *@param[out] pR - the polygon edge value
        */
        void csrRasterFindEdge(const CSR_Vector3* pV1,
                               const CSR_Vector3* pV2,
                               const CSR_Vector3* pV3,
                                     float*       pR);

        /**
        * Gets the screen coordinates based on a physically-based camera model
        *@param pRaster - raster options
        *@param imageWidth - image width in pixels
        *@param imageHeight - image height in pixels
        *@param zNear - near clipping plane value
        *@param[out] pScreenRect - rect containing the screen coordinates
        */
        void csrRasterGetScreenCoordinates(const CSR_Raster* pRaster,
                                                 float       imageWidth,
                                                 float       imageHeight,
                                                 float       zNear,
                                                 CSR_Rect*   pScreenRect);

        /**
        * Rasterizes a vertex
        *@param pInVertex - input vertex to rasterize
        *@param pMatrix - matrix
        *@param pScreenRect - rect containing the screen coordinates
        *@param zNear - near clipping plane value
        *@param imageWidth - image width in pixels
        *@param imageHeight - image height in pixels
        *@param[out] pOutVertex - rasterized output vertex
        */
        void csrRasterRasterizeVertex(const CSR_Vector3* pInVertex,
                                      const CSR_Matrix4* pMatrix,
                                      const CSR_Rect*    pScreenRect,
                                            float        zNear,
                                            float        imageWidth,
                                            float        imageHeight,
                                            CSR_Vector3* pOutVertex);

        /**
        * Gets a polygon to rasterize
        *@param pMatrix - matrix
        *@param v1Index - index of the first polygon vertex in the vertex buffer
        *@param v2Index - index of the second polygon vertex in the vertex buffer
        *@param v3Index - index of the third  polygon vertex in the vertex buffer
        *@param pVB - vertex buffer from which the vertex should be extracted
        *@param[out] pPolygon - polygon
        *@param[out] pNormal - polygon normal (array of 3 items)
        *@param[out] pST - polygon texture coordinates (array of 3 items)
        *@param[out] pColor - polygon per-vertex colors (array of 3 items)
        *@param fOnApplyVertexShader - vertex shader callback
        *@return 1 on success, otherwise 0
        */
        int csrRasterGetPolygon(const CSR_Matrix4*             pMatrix,
                                      size_t                   v1Index,
                                      size_t                   v2Index,
                                      size_t                   v3Index,
                                const CSR_VertexBuffer*        pVB,
                                      CSR_Polygon3*            pPolygon,
                                      CSR_Vector3*             pNormal,
                                      CSR_Vector2*             pST,
                                      CSR_Color*               pColor,
                                const CSR_fOnApplyVertexShader fOnApplyVertexShader);

        /**
        * Draws a polygon
        *@param pPolygon - polygon
        *@param pNormal - polygon normal (array of 3 items)
        *@param pST - polygon texture coordinates (array of 3 items)
        *@param pColor - polygon per-vertex colors (array of 3 items)
        *@param pMatrix - matrix
        *@param zNear - near clipping plane value
        *@param cullingType - culling type to apply
        *@param cullingFace - culling face to apply
        *@param pScreenRect - rect containing the screen coordinates
        *@param[in, out] pFB - frame buffer in which the scene will be drawn
        *@param[in, out] pDB - depth buffer to use for depth checking
        *@param fOnApplyFragmentShader - fragment shader callback
        *@return 1 on success, otherwise 0
        */
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
                                 const CSR_fOnApplyFragmentShader fOnApplyFragmentShader);

        /**
        * Draws a vertex buffer
        *@param pMatrix - matrix
        *@param zNear - near clipping plane value
        *@param zFar - far clipping plane value
        *@param pVB - vertex buffer to draw
        *@aram pRaster - raster options
        *@param[in, out] pFB - frame buffer in which the scene will be drawn
        *@param[in, out] pDB - depth buffer to use for depth checking
        *@param fOnApplyVertexShader - vertex shader callback
        *@param fOnApplyFragmentShader - fragment shader callback
        *@return 1 on success, otherwise 0
        */
        int csrRasterDraw(const CSR_Matrix4*               pMatrix,
                                float                      zNear,
                                float                      zFar,
                          const CSR_VertexBuffer*          pVB,
                          const CSR_Raster*                pRaster,
                                CSR_FrameBuffer*           pFB,
                                CSR_DepthBuffer*           pDB,
                          const CSR_fOnApplyVertexShader   fOnApplyVertexShader,
                          const CSR_fOnApplyFragmentShader fOnApplyFragmentShader);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_SoftwareRaster.c"
#endif

#endif
