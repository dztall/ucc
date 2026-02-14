/****************************************************************************
 * ==> CSR_Iqm -------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a Inter-Quake Model (.iqm) reader     *
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

#ifndef CSR_IqmH
#define CSR_IqmH

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
* Inter-Quake Model (.iqm)
*@note Each mesh is connected to its own weights count and skeleton, sorted in the same order in each list
*/
typedef struct
{
    CSR_Mesh*               m_pMesh;               // meshes composing the model
    size_t                  m_MeshCount;           // mesh item count
    CSR_Skin_Weights_Group* m_pMeshWeights;        // mesh skin weights, in the same order as meshes
    size_t                  m_MeshWeightsCount;    // mesh skin weights item count
    CSR_Bone_Mesh_Binding*  m_pMeshToBoneDict;     // mesh to bone dictionary
    size_t                  m_MeshToBoneDictCount; // mesh to bone dictionary item count
    CSR_Bone*               m_pSkeleton;           // model skeleton
    CSR_AnimationSet_Bone*  m_pAnimationSet;       // set of animations to apply to bones
    size_t                  m_AnimationSetCount;   // animation set count
    int                     m_MeshOnly;            // if activated, only the mesh will be drawn. All other data will be ignored
    int                     m_PoseOnly;            // if activated, the model will take the default pose but will not be animated
} CSR_IQM;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // IQM model functions
        //-------------------------------------------------------------------

        /**
        * Creates a IQM model from a buffer
        *@param pBuffer - buffer containing the IQM data to read
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param meshOnly - if 1, only the mesh will be drawn. All other data will be ignored
        *@param poseOnly - if 1, the model will take the default pose but will not be animated
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnLoadTexture - called when a texture should be loaded
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return the newly created IQM model, 0 on error
        *@note The IQM model must be released when no longer used, see csrIQMModelRelease()
        */
        CSR_IQM* csrIQMCreate(const CSR_Buffer*           pBuffer,
                              const CSR_VertexFormat*     pVertFormat,
                              const CSR_VertexCulling*    pVertCulling,
                              const CSR_Material*         pMaterial,
                                    int                   meshOnly,
                                    int                   poseOnly,
                              const CSR_fOnGetVertexColor fOnGetVertexColor,
                              const CSR_fOnLoadTexture    fOnLoadTexture,
                              const CSR_fOnApplySkin      fOnApplySkin,
                              const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Initializes a IQM model structure
        *@param[in, out] pIQM - IQM model to initialize
        */
        void csrIQMInit(CSR_IQM* pIQM);

        /**
        * Opens a IQM model from a file
        *@param pFileName - IQM model file name
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param meshOnly - if 1, only the mesh will be drawn. All other data will be ignored
        *@param poseOnly - if 1, the model will take the default pose but will not be animated
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnLoadTexture - called when a texture should be loaded
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return the newly created IQM model, 0 on error
        *@note The IQM model must be released when no longer used, see csrIQMModelRelease()
        */
        CSR_IQM* csrIQMOpen(const char*                 pFileName,
                            const CSR_VertexFormat*     pVertFormat,
                            const CSR_VertexCulling*    pVertCulling,
                            const CSR_Material*         pMaterial,
                                  int                   meshOnly,
                                  int                   poseOnly,
                            const CSR_fOnGetVertexColor fOnGetVertexColor,
                            const CSR_fOnLoadTexture    fOnLoadTexture,
                            const CSR_fOnApplySkin      fOnApplySkin,
                            const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Releases a IQM model
        *@param[in, out] pIQM - IQM model to release
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrIQMRelease(CSR_IQM* pIQM, const CSR_fOnDeleteTexture fOnDeleteTexture);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Iqm.c"
#endif

#endif
