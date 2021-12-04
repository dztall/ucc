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

#ifndef CSR_MdlH
#define CSR_MdlH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Texture.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_MDL_Mesh_File_Version  6
#define M_MDL_ID                 (('O' << 24) + ('P' << 16) + ('D' << 8) + 'I')

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Quake I (.mdl) model
*/
typedef struct
{
    CSR_Model*           m_pModel;
    size_t               m_ModelCount;
    CSR_Animation_Frame* m_pAnimation;
    size_t               m_AnimationCount;
    CSR_Skin*            m_pSkin;
    size_t               m_SkinCount;
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

#ifdef __cplusplus
    extern "C"
    {
#endif
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
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return the newly created MDL model, 0 on error
        *@note The MDL model must be released when no longer used, see csrMDLModelRelease()
        */
        CSR_MDL* csrMDLCreate(const CSR_Buffer*           pBuffer,
                              const CSR_Buffer*           pPalette,
                              const CSR_VertexFormat*     pVertFormat,
                              const CSR_VertexCulling*    pVertCulling,
                              const CSR_Material*         pMaterial,
                              const CSR_fOnGetVertexColor fOnGetVertexColor,
                              const CSR_fOnApplySkin      fOnApplySkin,
                              const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Opens a MDL model from a file
        *@param pFileName - MDL model file name
        *@param pPalette - palette to use to generate the model texture, if 0 a default palette will be used
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return the newly created MDL model, 0 on error
        *@note The MDL model must be released when no longer used, see csrMDLModelRelease()
        */
        CSR_MDL* csrMDLOpen(const char*                 pFileName,
                            const CSR_Buffer*           pPalette,
                            const CSR_VertexFormat*     pVertFormat,
                            const CSR_VertexCulling*    pVertCulling,
                            const CSR_Material*         pMaterial,
                            const CSR_fOnGetVertexColor fOnGetVertexColor,
                            const CSR_fOnApplySkin      fOnApplySkin,
                            const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Releases a MDL model
        *@param[in, out] pMDL - MDL model to release
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrMDLRelease(CSR_MDL* pMDL, const CSR_fOnDeleteTexture fOnDeleteTexture);

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
        *@param[in, out] pSkinIndex - skin index, new skin index on function ends
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
                                     size_t*  pSkinIndex,
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
        *@note The returned mesh will be valid as long as its owner model is
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
        *@param[out] pModel - model to populate (a blank model should be provided)
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

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Mdl.c"
#endif

#endif
