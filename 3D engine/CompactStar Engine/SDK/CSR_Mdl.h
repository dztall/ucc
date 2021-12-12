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
        * Initializes a MDL model structure
        *@param[in, out] pMDL - MDL model to initialize
        */
        void csrMDLInit(CSR_MDL* pMDL);

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
