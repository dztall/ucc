/****************************************************************************
 * ==> CSR_Vertex ----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the vertex functions and types        *
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

#ifndef CSR_VertexH
#define CSR_VertexH

// compactStar engine
#include "CSR_Geometry.h"
#include "CSR_Texture.h"
#include "CSR_Lighting.h"

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Culling type
*/
typedef enum
{
    CSR_CT_None,
    CSR_CT_Front,
    CSR_CT_Back,
    CSR_CT_Both
} CSR_ECullingType;

/**
* Culling face
*/
typedef enum
{
    CSR_CF_CW,
    CSR_CF_CCW
} CSR_ECullingFace;

/**
* Vertex type
*/
typedef enum
{
    CSR_VT_Triangles,
    CSR_VT_TriangleStrip,
    CSR_VT_TriangleFan,
    CSR_VT_Quads,
    CSR_VT_QuadStrip
} CSR_EVertexType;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Vertex format
*/
typedef struct
{
    CSR_EVertexType m_Type;
    int             m_HasNormal;         // each vertex contains a normal
    int             m_HasTexCoords;      // each vertex contains an UV texture coordinate
    int             m_HasPerVertexColor; // each vertex contains his own color, see CSR_fOnGetVertexColor callback
    unsigned        m_Stride;
} CSR_VertexFormat;

/**
* Vertex culling
*/
typedef struct
{
    CSR_ECullingType m_Type;
    CSR_ECullingFace m_Face;
} CSR_VertexCulling;

/**
* Vertex buffer
*/
typedef struct
{
    CSR_VertexFormat  m_Format;
    CSR_VertexCulling m_Culling;
    CSR_Material      m_Material;
    float*            m_pData;
    size_t            m_Count;
    double            m_Time;
} CSR_VertexBuffer;

/**
* Mesh
*/
typedef struct
{
    CSR_TextureShader m_Shader;
    CSR_VertexBuffer* m_pVB;
    size_t            m_Count;
    double            m_Time;
} CSR_Mesh;

/**
* Indexed polygon
*/
typedef struct
{
    const CSR_VertexBuffer* m_pVB;
          size_t            m_pIndex[3];
} CSR_IndexedPolygon;

/**
* Indexed polygon buffer
*/
typedef struct
{
    CSR_IndexedPolygon* m_pIndexedPolygon;
    size_t              m_Count;
} CSR_IndexedPolygonBuffer;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when a vertex color should be get
*@param pVB - vertex buffer that will contain the vertex for which the color should be get
*@param pNormal - vertex normal
*@param groupIndex - the vertex group index (e.g. the inner and outer vertices of a ring)
*@return RGBA color to apply to the vertex
*@note This callback will be called only if the per-vertex color option is activated in the vertex
*      buffer
*/
typedef unsigned (*CSR_fOnGetVertexColor)(const CSR_VertexBuffer* pVB,
                                          const CSR_Vector3*      pNormal,
                                                size_t            groupIndex);

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Vertex format functions
        //-------------------------------------------------------------------

        /**
        * Initializes a vertex format structure
        *@param[in, out] pVertexFormat - vertex format to initialize
        */
        void csrVertexFormatInit(CSR_VertexFormat* pVertexFormat);

        /**
        * Calculates the vertex stride
        *@param[in, out] pVertexFormat - vertex format for which the stride should be calculated
        */
        void csrVertexFormatCalculateStride(CSR_VertexFormat* pVertexFormat);

        //-------------------------------------------------------------------
        // Vertex culling functions
        //-------------------------------------------------------------------

        /**
        * Initializes a vertex culling structure
        *@param[in, out] pVertexCulling - vertex culling to initialize
        */
        void csrVertexCullingInit(CSR_VertexCulling* pVertexCulling);

        //-------------------------------------------------------------------
        // Vertex buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates a vertex buffer
        *@return newly created vertex buffer, 0 on error
        *@note The vertex buffer must be released when no longer used, see csrVertexBufferRelease()
        */
        CSR_VertexBuffer* csrVertexBufferCreate(void);

        /**
        * Releases a vertex buffer
        *@param[in, out] pVB - vertex buffer to release
        */
        void csrVertexBufferRelease(CSR_VertexBuffer* pVB);

        /**
        * Initializes a vertex buffer structure
        *@param[in, out] pVB - vertex buffer to initialize
        */
        void csrVertexBufferInit(CSR_VertexBuffer* pVB);

        /**
        * Adds a vertex to a vertex buffer
        *@param pVertex - vertex
        *@param pNormal - normal
        *@param pUV - texture coordinate
        *@param groupIndex - the vertex group index (e.g. the inner and outer vertices of a ring)
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param[in, out] pVB - vertex buffer to add to
        *@return 1 on success, otherwise 0
        */
        int csrVertexBufferAdd(const CSR_Vector3*          pVertex,
                               const CSR_Vector3*          pNormal,
                               const CSR_Vector2*          pUV,
                                     size_t                groupIndex,
                               const CSR_fOnGetVertexColor fOnGetVertexColor,
                                     CSR_VertexBuffer*     pVB);

        //-------------------------------------------------------------------
        // Mesh functions
        //-------------------------------------------------------------------

        /**
        * Creates a mesh
        *@return newly created mesh, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrMeshCreate(void);

        /**
        * Releases a mesh
        *@param[in, out] pMesh - mesh to release
        */
        void csrMeshRelease(CSR_Mesh* pMesh);

        /**
        * Initializes a mesh structure
        *@param[in, out] pMesh - mesh to initialize
        */
        void csrMeshInit(CSR_Mesh* pMesh);

        //-------------------------------------------------------------------
        // Indexed polygon functions
        //-------------------------------------------------------------------

        /**
        * Initializes an indexed polygon structure
        *@param[in, out] pIndexedPolygon - indexed polygon to initialize
        */
        void csrIndexedPolygonInit(CSR_IndexedPolygon* pIndexedPolygon);

        /**
        * Gets a polygon from an indexed polygon
        *@param pIndexedPolygon - indexed polygon to get from
        *@param[out] pPolygon - polygon
        *@return 1 on success, otherwise 0
        */
        int csrIndexedPolygonToPolygon(const CSR_IndexedPolygon* pIndexedPolygon,
                                             CSR_Polygon3*       pPolygon);

        //-------------------------------------------------------------------
        // Indexed polygon buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates an indexed polygon buffer
        *@return newly created indexed polygon buffer, 0 on error
        *@note The indexed polygon buffer must be released when no longer used, see
               csrIndexedPolygonBufferRelease()
        */
        CSR_IndexedPolygonBuffer* csrIndexedPolygonBufferCreate(void);

        /**
        * Releases an indexed polygon buffer
        *@param[in, out] pIPB - indexed polygon buffer to release
        */
        void csrIndexedPolygonBufferRelease(CSR_IndexedPolygonBuffer* pIPB);

        /**
        * Initializes an indexed polygon buffer structure
        *@param[in, out] pIPB - indexed polygon buffer to initialize
        */
        void csrIndexedPolygonBufferInit(CSR_IndexedPolygonBuffer* pIPB);

        /**
        * Adds an indexed polygon to an indexed polygon buffer
        *@param pIndexedPolygon - indexed polygon to add to the indexed polygon buffer
        *@param[in, out] pIPB - indexed polygon buffer to add to
        *@return 1 on success, otherwise 0
        */
        int csrIndexedPolygonBufferAdd(const CSR_IndexedPolygon*       pIndexedPolygon,
                                             CSR_IndexedPolygonBuffer* pIPB);

        /**
        * Gets an indexed polygon buffer from a mesh
        *@param pMesh - mesh
        *@return indexed polygon buffer, 0 on error
        *@note The indexed polygon buffer must be released when no longer used, see
        *      csrIndexedPolygonBufferRelease()
        *@note BE CAREFUL, the indexed polygon buffer is valid as long as his source mesh is valid.
        *      If the mesh is released, the indexed polygon buffer should be released together.
        *      However the indexed polygon buffer may be released after the mesh
        */
        CSR_IndexedPolygonBuffer* csrIndexedPolygonBufferFromMesh(const CSR_Mesh* pMesh);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Vertex.c"
#endif

#endif
