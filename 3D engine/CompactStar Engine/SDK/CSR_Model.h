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

#ifndef CSR_ModelH
#define CSR_ModelH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"
#include "CSR_Texture.h"

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_MDL_Mesh_File_Version 6
#define M_MDL_ID                (('O' << 24) + ('P' << 16) + ('D' << 8) + 'I')

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Model, it's a collection of meshes, each of them represent a frame. The model may be animated, by
* showing each frame, one after the other
*/
typedef struct
{
    CSR_Mesh* m_pMesh;
    size_t    m_MeshCount;
    double    m_Time;
} CSR_Model;

/**
* Model animation (based on frames)
*/
typedef struct
{
    char   m_Name[16];
    size_t m_Start;
    size_t m_End;
} CSR_ModelAnimation;

/**
* Model texture
*/
typedef struct
{
    #ifdef CSR_USE_OPENGL
        GLuint m_TextureID;
        GLuint m_BumpMapID;
    #endif
    double     m_Time;
} CSR_ModelTexture;

/**
* Quake I (.mdl) model
*/
typedef struct
{
    CSR_Model*          m_pModel;
    size_t              m_ModelCount;
    CSR_ModelAnimation* m_pAnimation;
    size_t              m_AnimationCount;
    CSR_ModelTexture*   m_pTexture;
    size_t              m_TextureCount;
} CSR_MDL;

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

/**
* WaveFront vertex
*/
typedef struct
{
    float* m_pData;
    size_t m_Count;
} CSR_WavefrontVertex;

/**
* WaveFront normal
*/
typedef struct
{
    float* m_pData;
    size_t m_Count;
} CSR_WavefrontNormal;

/**
* WaveFront texture coordinate
*/
typedef struct
{
    float* m_pData;
    size_t m_Count;
} CSR_WavefrontTexCoord;

/**
* WaveFront face
*/
typedef struct
{
    int*   m_pData;
    size_t m_Count;
} CSR_WavefrontFace;

/**
* WaveFront group
*/
typedef struct
{
    CSR_WavefrontVertex*   m_pVertex;
    CSR_WavefrontNormal*   m_pNormal;
    CSR_WavefrontTexCoord* m_pUV;
    CSR_WavefrontFace*     m_pFace;
} CSR_WavefrontGroup;

/**
* WaveFront object
*/
typedef struct
{
    CSR_WavefrontGroup* m_pGroup;
    size_t              m_Count;
} CSR_WavefrontObject;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when a texture was read in a model
*@param index - texture index in the model
*@param pPixelBuffer - pixel buffer containing the read texture
*@param[in, out] pNoGPU - if 1, the texture will not be loaded on the GPU while model is loading
*/
typedef void (*CSR_fOnTextureRead)(size_t index, const CSR_PixelBuffer* pPixelBuffer, int* pNoGPU);

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Shape functions
        //-------------------------------------------------------------------

        /**
        * Creates a surface
        *@param width - surface width (on the x axis)
        *@param height - surface height (on the y axis)
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the surface, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateSurface(float                 width,
                                        float                 height,
                                  const CSR_VertexFormat*     pVertFormat,
                                  const CSR_VertexCulling*    pVertCulling,
                                  const CSR_Material*         pMaterial,
                                  const CSR_fOnGetVertexColor fOnGetVertexColor);

        /**
        * Creates a box
        *@param width - box width (on the x axis)
        *@param height - box height (on the y axis)
        *@param depth - box depth (on the z axis)
        *@param repeatTexOnEachFace - if 1 the texture will be repeated on each face
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the box, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateBox(float                 width,
                                    float                 height,
                                    float                 depth,
                                    int                   repeatTexOnEachFace,
                              const CSR_VertexFormat*     pVertFormat,
                              const CSR_VertexCulling*    pVertCulling,
                              const CSR_Material*         pMaterial,
                              const CSR_fOnGetVertexColor fOnGetVertexColor);

        /**
        * Creates a sphere
        *@param radius - sphere radius
        *@param slices - slices (longitude) count
        *@param stacks - stacks (latitude) count
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the sphere, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateSphere(float                 radius,
                                       int                   slices,
                                       int                   stacks,
                                 const CSR_VertexFormat*     pVertFormat,
                                 const CSR_VertexCulling*    pVertCulling,
                                 const CSR_Material*         pMaterial,
                                 const CSR_fOnGetVertexColor fOnGetVertexColor);

        /**
        * Creates a cylinder
        *@param radius - cylinder radius
        *@param height - cylinder height
        *@param faces - number of faces composing the cylinder
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the cylinder, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateCylinder(float                 radius,
                                         float                 height,
                                         int                   faces,
                                   const CSR_VertexFormat*     pVertFormat,
                                   const CSR_VertexCulling*    pVertCulling,
                                   const CSR_Material*         pMaterial,
                                   const CSR_fOnGetVertexColor fOnGetVertexColor);

        /**
        * Creates a disk
        *@param centerX - the disk center on the x axis
        *@param centerY - the disk center on the y axis
        *@param radius - disk radius
        *@param slices - disk slice count
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the disk, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateDisk(float                 centerX,
                                     float                 centerY,
                                     float                 radius,
                                     unsigned              slices,
                               const CSR_VertexFormat*     pVertFormat,
                               const CSR_VertexCulling*    pVertCulling,
                               const CSR_Material*         pMaterial,
                               const CSR_fOnGetVertexColor fOnGetVertexColor);

        /**
        * Creates a ring
        *@param centerX - the ring center on the x axis
        *@param centerY - the ring center on the y axis
        *@param minRadius - internal radius
        *@param maxRadius - external radius
        *@param slices - slice count
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the ring, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateRing(float                 centerX,
                                     float                 centerY,
                                     float                 minRadius,
                                     float                 maxRadius,
                                     unsigned              slices,
                               const CSR_VertexFormat*     pVertFormat,
                               const CSR_VertexCulling*    pVertCulling,
                               const CSR_Material*         pMaterial,
                               const CSR_fOnGetVertexColor fOnGetVertexColor);

        /**
        * Creates a spiral
        *@param centerX - the spiral center on the x axis
        *@param centerY - the spiral center on the y axis
        *@param minRadius - internal radius
        *@param maxRadius - external radius
        *@param deltaMin - delta to apply to the min radius
        *@param deltaMax - delta to apply to the max radius
        *@param deltaZ - delta to apply to the z radius
        *@param slices - slice count
        *@param stacks - stack count
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the spiral, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
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
                                 const CSR_fOnGetVertexColor fOnGetVertexColor);

        //-------------------------------------------------------------------
        // Skybox functions
        //-------------------------------------------------------------------

        /**
        * Creates a skybox
        *@param width - skybox width (on the x axis)
        *@param height - skybox height (on the y axis)
        *@param depth - skybox depth (on the z axis)
        *@return mesh containing the skybox, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrSkyboxCreate(float width, float height, float depth);

        //-------------------------------------------------------------------
        // Model functions
        //-------------------------------------------------------------------

        /**
        * Creates a model
        *@return newly created model, 0 on error
        *@note The model must be released when no longer used, see csrModelRelease()
        */
        CSR_Model* csrModelCreate(void);

        /**
        * Releases a model
        *@param[in, out] pModel - model to release
        */
        void csrModelRelease(CSR_Model* pModel);

        /**
        * Initializes a model structure
        *@param[in, out] pModel - model to initialize
        */
        void csrModelInit(CSR_Model* pModel);

        //-------------------------------------------------------------------
        // MDL model functions
        //-------------------------------------------------------------------

        /**
        * Creates a MDL model from a buffer
        *@param pBuffer - buffer containing the MDL data to read
        *@param pPalette - palette to use to generate the model texture, if 0 a default palette will be used
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnTextureRead - called when a texture is read
        *@return the newly created MDL model, 0 on error
        *@note The MDL model must be released when no longer used, see csrMDLModelRelease()
        */
        CSR_MDL* csrMDLCreate(const CSR_Buffer*           pBuffer,
                              const CSR_Buffer*           pPalette,
                              const CSR_VertexFormat*     pVertFormat,
                              const CSR_VertexCulling*    pVertCulling,
                              const CSR_Material*         pMaterial,
                              const CSR_fOnGetVertexColor fOnGetVertexColor,
                              const CSR_fOnTextureRead    fOnTextureRead);

        /**
        * Opens a MDL model from a file
        *@param pFileName - MDL model file name
        *@param pPalette - palette to use to generate the model texture, if 0 a default palette will be used
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return the newly created MDL model, 0 on error
        *@note The MDL model must be released when no longer used, see csrMDLModelRelease()
        */
        CSR_MDL* csrMDLOpen(const char*                 pFileName,
                            const CSR_Buffer*           pPalette,
                            const CSR_VertexFormat*     pVertFormat,
                            const CSR_VertexCulling*    pVertCulling,
                            const CSR_Material*         pMaterial,
                            const CSR_fOnGetVertexColor fOnGetVertexColor,
                            const CSR_fOnTextureRead    fOnTextureRead);

        /**
        * Releases a MDL model
        *@param[in, out] pMDL - MDL model to release
        */
        void csrMDLRelease(CSR_MDL* pMDL);

        /**
        * Initializes a MDL model structure
        *@param[in, out] pMDL - MDL model to initialize
        */
        void csrMDLInit(CSR_MDL* pMDL);

        /**
        * Updates the MDL model indexes (e.g. before getting the next mesh to show)
        *@param pMDL - MDL model
        *@param fps - frame per seconds to apply
        *@param animationIndex - animation index
        *@param[in, out] pTextureIndex - texture index, new texture index on function ends
        *@param[in, out] pModelIndex - model index, new model index on function ends
        *@param[in, out] pMeshIndex - mesh index, new mesh index on function ends
        *@param[in, out] pTextureLastTime - texture last known time
        *@param[in, out] pModelLastTime - model last known time
        *@param[in, out] pMeshLastTime - mesh last known time
        *@param elapsedTime - elapsed time since last calculation
        */
        void csrMDLUpdateIndex(const CSR_MDL* pMDL,
                                     size_t   fps,
                                     size_t   animationIndex,
                                     size_t*  pTextureIndex,
                                     size_t*  pModelIndex,
                                     size_t*  pMeshIndex,
                                     double*  pTextureLastTime,
                                     double*  pModelLastTime,
                                     double*  pMeshLastTime,
                                     double   elapsedTime);

        /**
        * Get the current mesh from a MDL model (e.g. to draw it)
        *@param pMDL - MDL model to get from
        *@param modelIndex - model index, 0 if unknown (csrMDLUpdateIndex() may be called first)
        *@param meshIndex - mesh index, 0 if unknown (csrMDLUpdateIndex() may be called first)
        *@return current mesh from MDL model, 0 on error or if not found
        *@note The returned mesh will be valid as long as his owner model is
        */
        CSR_Mesh* csrMDLGetMesh(const CSR_MDL* pMDL, size_t modelIndex, size_t meshIndex);

        /**
        * Reads MDL header
        *@param pBuffer - buffer containing the MDL data
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param[out] pHeader - MDL header structure to populate
        *@return 1 on success, otherwise 0
        */
        int csrMDLReadHeader(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_MDLHeader* pHeader);

        /**
        * Reads MDL skin
        *@param pBuffer - buffer containing the MDL data
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param pHeader - MDL header structure
        *@param[out] pSkin - MDL skin structure to populate
        *@return 1 on success, otherwise 0
        */
        int csrMDLReadSkin(const CSR_Buffer*    pBuffer,
                                 size_t*        pOffset,
                           const CSR_MDLHeader* pHeader,
                                 CSR_MDLSkin*   pSkin);

        /**
        * Reads MDL texture coordinates
        *@param pBuffer - buffer containing the MDL data
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param[out] pTexCoord - MDL texture coordinates structure to populate
        *@return 1 on success, otherwise 0
        */
        int csrMDLReadTextureCoord(const CSR_Buffer*          pBuffer,
                                         size_t*              pOffset,
                                         CSR_MDLTextureCoord* pTexCoord);

        /**
        * Reads MDL polygon
        *@param pBuffer - buffer containing the MDL data
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param[out] pPolygon - MDL polygon structure to populate
        *@return 1 on success, otherwise 0
        */
        int csrMDLReadPolygon(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_MDLPolygon* pPolygon);

        /**
        * Reads MDL vertex
        *@param pBuffer - buffer containing the MDL data
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param[out] pVertex - MDL vertex structure to populate
        *@return 1 on success, otherwise 0
        */
        int csrMDLReadVertex(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_MDLVertex* pVertex);

        /**
        * Reads MDL frame
        *@param pBuffer - buffer containing the MDL data
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param pHeader - MDL header
        *@param[out] pFrame - MDL frame structure to populate
        *@return 1 on success, otherwise 0
        */
        int csrMDLReadFrame(const CSR_Buffer*    pBuffer,
                                  size_t*        pOffset,
                            const CSR_MDLHeader* pHeader,
                                  CSR_MDLFrame*  pFrame);

        /**
        * Reads MDL frame group
        *@param pBuffer - buffer containing the MDL data
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param pHeader - MDL header
        *@param[out] pFrameGroup - MDL frame group structure to populate
        *@return 1 on success, otherwise 0
        */
        int csrMDLReadFrameGroup(const CSR_Buffer*        pBuffer,
                                       size_t*            pOffset,
                                 const CSR_MDLHeader*     pHeader,
                                       CSR_MDLFrameGroup* pFrameGroup);

        /**
        * Uncompresses the MDL texture
        *@param pSkin - model skin
        *@param pPalette - palette to use to uncompress the texture, if 0 the default palette will be used
        *@param width - texture width
        *@param height - texture height
        *@param index - texture index
        *@return a pixel buffer containing the texture, 0 on error
        *@note The pixel buffer must be released when no longer used, see csrPixelBufferRelease()
        */
        CSR_PixelBuffer* csrMDLUncompressTexture(const CSR_MDLSkin* pSkin,
                                                 const CSR_Buffer*  pPalette,
                                                       size_t       width,
                                                       size_t       height,
                                                       size_t       index);

        /**
        * Uncompresses a MDL vertex
        *@param pHeader - MDL header
        *@param pVertex - MDL vertex
        *@param[out] pResult - resulting vector
        */
        void csrMDLUncompressVertex(const CSR_MDLHeader* pHeader,
                                    const CSR_MDLVertex* pVertex,
                                          CSR_Vector3*   pResult);

        /**
        * Populates a model from MDL data
        *@param pHeader - MDL header
        *@param pFrameGroup - MDL frame group
        *@param pPolygon - MDL polygon
        *@param pTexCoord - MDL texture coordinates
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param[out] pModel - model to populate (a blank model should be propvided)
        */
        void csrMDLPopulateModel(const CSR_MDLHeader*        pHeader,
                                 const CSR_MDLFrameGroup*    pFrameGroup,
                                 const CSR_MDLPolygon*       pPolygon,
                                 const CSR_MDLTextureCoord*  pTexCoord,
                                 const CSR_VertexFormat*     pVertFormat,
                                 const CSR_VertexCulling*    pVertCulling,
                                 const CSR_Material*         pMaterial,
                                 const CSR_fOnGetVertexColor fOnGetVertexColor,
                                       CSR_Model*            pModel);

        /**
        * Releases the MDL objects used during the opening
        *@param pHeader - MDL header
        *@param pFrameGroup - MDL frame group
        *@param pSkin - MDL skin
        *@param pTexCoord - MDL texture coordinates
        *@param pPolygon - MDL polygon
        */
        void csrMDLReleaseObjects(CSR_MDLHeader*       pHeader,
                                  CSR_MDLFrameGroup*   pFrameGroup,
                                  CSR_MDLSkin*         pSkin,
                                  CSR_MDLTextureCoord* pTexCoord,
                                  CSR_MDLPolygon*      pPolygon);

        //-------------------------------------------------------------------
        // WaveFront model functions
        //-------------------------------------------------------------------

        /**
        * Create a mesh from a buffer containing a WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnTextureRead - called when a texture is read
        *@return model containing the WaveFront file on success, otherwise 0
        *@note The model content should be released using the csrModelRelease function when useless
        */
        CSR_Model* csrWaveFrontCreate(const CSR_Buffer*           pBuffer,
                                      const CSR_VertexFormat*     pVertFormat,
                                      const CSR_VertexCulling*    pVertCulling,
                                      const CSR_Material*         pMaterial,
                                      const CSR_fOnGetVertexColor fOnGetVertexColor,
                                      const CSR_fOnTextureRead    fOnTextureRead);

        /**
        * Opens a WaveFront file
        *@param pFileName - WaveFront file name to open
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnTextureRead - called when a texture is read
        *@return model containing the WaveFront file on success, otherwise 0
        *@note The model content should be released using the csrModelRelease when useless
        */
        CSR_Model* csrWaveFrontOpen(const char*                 pFileName,
                                    const CSR_VertexFormat*     pVertFormat,
                                    const CSR_VertexCulling*    pVertCulling,
                                    const CSR_Material*         pMaterial,
                                    const CSR_fOnGetVertexColor fOnGetVertexColor,
                                    const CSR_fOnTextureRead    fOnTextureRead);

        /**
        * Reads a commented line from a WaveFront buffer
        *@param pBuffer - source huffer to read from
        *@param[in, out] pChar - last read char
        *@param[in, out] pIndex - char index
        */
        void csrWaveFrontReadComment(const CSR_Buffer* pBuffer, char* pChar, size_t* pIndex);

        /**
        * Reads a vertex from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pVertex - vertex array in which the new read vertex should be added
        */
        void csrWaveFrontReadVertex(const CSR_Buffer*          pBuffer,
                                          char*                pChar,
                                          size_t*              pIndex,
                                          CSR_WavefrontVertex* pVertex);

        /**
        * Reads a normal from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pNormal - normal array in which the new read normal should be added
        */
        void csrWaveFrontReadNormal(const CSR_Buffer*          pBuffer,
                                          char*                pChar,
                                          size_t*              pIndex,
                                          CSR_WavefrontNormal* pNormal);

        /**
        * Reads a texture coordinate from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pTexCoord - texture coordinate array in which the new read texture coordinate should be added
        */
        void csrWaveFrontReadTextureCoordinate(const CSR_Buffer*            pBuffer,
                                                     char*                  pChar,
                                                     size_t*                pIndex,
                                                     CSR_WavefrontTexCoord* pTexCoord);

        /**
        * Reads a face from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pFace - face array in which the new read face should be added
        */
        void csrWaveFrontReadFace(const CSR_Buffer*        pBuffer,
                                        char*              pChar,
                                        size_t*            pIndex,
                                        CSR_WavefrontFace* pFace);

        /**
        * Reads an unknown line from a WaveFront buffer
        *@param pBuffer - source huffer to read from
        *@param[in, out] pChar - last read char
        *@param[in, out] pIndex - char index
        */
        void csrWaveFrontReadUnknown(const CSR_Buffer* pBuffer, char* pChar, size_t* pIndex);

        /**
        * Converts a read value to float and adds it in an array
        *@param pBuffer - buffer containing the value to convert
        *@param[in, out] pArray - float array in which the value should be added
        *@param[in, out] pCount - array count
        */
        void csrWaveFrontConvertFloat(const char* pBuffer, float** pArray, size_t* pCount);

        /**
        * Converts a read value to int and adds it in an array
        *@param pBuffer - buffer containing the value to convert
        *@param[in, out] pArray - int array in which the value should be added
        *@param[in, out] pCount - array count
        */
        void csrWaveFrontConvertInt(const char* pBuffer, int** pArray, size_t* pCount);

        /**
        * Builds a face from WaveFront data
        *@param pVertex - vertex array read in WaveFront file
        *@param pNormal - normal array read in WaveFront file
        *@param pUV - texture coordinates array read in WaveFront file
        *@param pFace - face array read in WaveFront file
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param objectChanging - if 1, a new object should be created
        *@param groupChanging - if 1, a new group should be created
        *@param[in, out] pModel - model in which the WaveFront data should be built
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnTextureRead - called when a texture is read
        *@return 1 on success, otherwise 0
        */
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
                                  const CSR_fOnTextureRead     fOnTextureRead);

        /**
        * Builds a vertex buffer from a WaveFront data
        *@param pVertex - vertex array read in WaveFront file
        *@param pNormal - normal array read in WaveFront file
        *@param pUV - texture coordinates array read in WaveFront file
        *@param pFace - face array read in WaveFront file
        *@param[in, out] pVB - vertex buffer in which the WaveFront data should be built
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnTextureRead - called when a texture is read
        */
        void csrWaveFrontBuildVertexBuffer(const CSR_WavefrontVertex*   pVertex,
                                           const CSR_WavefrontNormal*   pNormal,
                                           const CSR_WavefrontTexCoord* pUV,
                                           const CSR_WavefrontFace*     pFace,
                                                 CSR_VertexBuffer*      pVB,
                                           const CSR_fOnGetVertexColor  fOnGetVertexColor,
                                           const CSR_fOnTextureRead     fOnTextureRead);

        //-------------------------------------------------------------------
        // Landscape creation functions
        //-------------------------------------------------------------------

        /**
        * Generates landscape vertices from a grayscale image
        *@param pPixelBuffer - pixel buffer containing the image
        *@param height - map height
        *@param scale - scale factor
        *@param[out] pVertices - buffer containing the landscape vertices
        *@return 1 on success, otherwise 0
        */
        int csrLandscapeGenerateVertices(const CSR_PixelBuffer* pPixelBuffer,
                                               float            height,
                                               float            scale,
                                               CSR_Buffer*      pVertices);

        /**
        * Creates a landscape mesh from a grayscale image
        *@param pPixelBuffer - pixel buffer containing the landscape map image
        *@param height - landscape height
        *@param scale - scale factor
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the landscape, 0 on error
        *@note The mesh content should be released using the csrMeshRelease function when useless
        */
        CSR_Mesh* csrLandscapeCreate(const CSR_PixelBuffer*      pPixelBuffer,
                                           float                 height,
                                           float                 scale,
                                     const CSR_VertexFormat*     pVertFormat,
                                     const CSR_VertexCulling*    pVertCulling,
                                     const CSR_Material*         pMaterial,
                                     const CSR_fOnGetVertexColor fOnGetVertexColor);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Model.c"
#endif

#endif
