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

#ifndef CSR_ModelH
#define CSR_ModelH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"
#include "CSR_Texture.h"

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Animation key type
*/
typedef enum
{
    CSR_KT_Unknown  = -1,
    CSR_KT_Rotation =  0,
    CSR_KT_Scale    =  1,
    CSR_KT_Position =  2,
    CSR_KT_Matrix   =  4
} CSR_EAnimKeyType;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Bone, it's a local transformation to apply to a mesh and belonging to a skeleton
*/
typedef struct CSR_tagBone
{
           char*        m_pName;         // bone name
           CSR_Matrix4  m_Matrix;        // matrix containing the bone transformation to apply
    struct CSR_tagBone* m_pParent;       // bone parent, root bone if 0
    struct CSR_tagBone* m_pChildren;     // bone children
           size_t       m_ChildrenCount; // bone children count
           void*        m_pCustomData;   // additional custom data. Be careful, this data may not be released internally
} CSR_Bone;

/**
* Skeleton, it's a set of local transformations named bones
*/
typedef struct
{
    char*       m_pId;           // skeleton identifier
    char*       m_pTarget;       // target weights identifier
    CSR_Bone*   m_pRoot;         // root bone
    CSR_Matrix4 m_InitialMatrix; // initial matrix
} CSR_Skeleton;

/**
* Binding between a bone and a mesh
*/
typedef struct
{
    CSR_Bone* m_pBone;     // bone binded with mesh
    size_t    m_MeshIndex; // mesh index binded with bone
} CSR_Bone_Mesh_Binding;

/**
* Skin weights index table
*/
typedef struct
{
    size_t* m_pData; // indices of the vertices to modify in the source mesh
    size_t  m_Count; // indices count
} CSR_Skin_Weight_Index_Table;

/**
* Skin weights, it's a group of vertices influenced by a bone
*/
typedef struct
{
    char*                        m_pBoneName;       // linked bone name
    CSR_Bone*                    m_pBone;           // linked bone
    CSR_Matrix4                  m_Matrix;          // matrix to transform the mesh vertices to the bone space
    size_t                       m_MeshIndex;       // source mesh index
    CSR_Skin_Weight_Index_Table* m_pIndexTable;     // table containing the indices of the vertices to modify in the source mesh
    size_t                       m_IndexTableCount; // mesh indices count
    float*                       m_pWeights;        // weights indicating the bone influence on vertices, between 0.0f and 1.0f
    size_t                       m_WeightCount;     // weight count
} CSR_Skin_Weights;

/**
* Skin weights group
*@note Generally used to contain all skin weights belonging to a mesh
*/
typedef struct
{
    CSR_Skin_Weights* m_pSkinWeights; // skin weights list
    size_t            m_Count;        // skin weights count
} CSR_Skin_Weights_Group;

/**
* Animation key, may be a rotation, a translation, a scale, a matrix, ...
*/
typedef struct
{
    size_t m_Frame;
    float* m_pValues;
    size_t m_Count;
} CSR_AnimationKey;

/**
* Animation key list
*/
typedef struct
{
    CSR_EAnimKeyType  m_Type;
    CSR_AnimationKey* m_pKey;
    size_t            m_Count;
    int               m_ColOverRow;
} CSR_AnimationKeys;

/**
* Model animation (based on frames)
*/
typedef struct
{
    char   m_Name[16];
    size_t m_Start;
    size_t m_End;
} CSR_Animation_Frame;

/**
* Animation (based on bones)
*/
typedef struct
{
    char*              m_pBoneName;
    CSR_Bone*          m_pBone;
    CSR_AnimationKeys* m_pKeys;
    size_t             m_Count;
} CSR_Animation_Bone;

/**
* Animation set (based on bones)
*/
typedef struct
{
    CSR_Animation_Bone* m_pAnimation;
    size_t              m_Count;
} CSR_AnimationSet_Bone;

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

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when a texture should be loaded
*@param pTextureName - texture name to load
*@return pixel buffer containing the loaded texture
*/
typedef CSR_PixelBuffer* (*CSR_fOnLoadTexture)(const char* pTextureName);

/**
* Called when a skin should be applied to a model
*@param index - skin index (in case the model contains several skins)
*@param pSkin - skin
*@param[in, out] pCanRelease - if 1, the skin content may be released after the skin is applied
*/
typedef void (*CSR_fOnApplySkin)(size_t index, const CSR_Skin* pSkin, int* pCanRelease);

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
        *@param minRadius - minimum cylinder radius
        *@param maxRadius - maximum cylinder radius
        *@param height - cylinder height
        *@param faces - number of faces composing the cylinder
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the cylinder, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateCylinder(float                 minRadius,
                                         float                 maxRadius,
                                         float                 height,
                                         int                   faces,
                                   const CSR_VertexFormat*     pVertFormat,
                                   const CSR_VertexCulling*    pVertCulling,
                                   const CSR_Material*         pMaterial,
                                   const CSR_fOnGetVertexColor fOnGetVertexColor);

        /**
        * Gets a capsule mesh
        *@param height - the capsule height
        *@param radius - the capsule radius
        *@param resolution - the capsule resolution (or slices)
        *@param pVertFormat - mesh vertex format, if 0 the default format will be used
        *@param pVertCulling - mesh vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@return mesh containing the sphere, 0 on error
        *@note The mesh must be released when no longer used, see csrMeshRelease()
        */
        CSR_Mesh* csrShapeCreateCapsule(float                 height,
                                        float                 radius,
                                        float                 resolution,
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
        // Bone functions
        //-------------------------------------------------------------------

        /**
        * Creates a bone
        *@return newly created bone, 0 on error
        *@note The bone must be released when no longer used, see csrBoneRelease()
        */
        CSR_Bone* csrBoneCreate(void);

        /**
        * Releases a bone
        *@param[in, out] pBone - bone to release
        *@param contentOnly - if 1, the bone content will be released, but not the bone itself
        *@param releaseCustomData - if 1, the custom data will also be released
        *@note All the bone children will also be released
        */
        void csrBoneRelease(CSR_Bone* pBone, int contentOnly, int releaseCustomData);

        /**
        * Initializes a bone structure
        *@param[in, out] pBone - bone to initialize
        */
        void csrBoneInit(CSR_Bone* pBone);

        /**
        * Finds the bone matching with the name
        *@param pBone - bone from which the search should begin
        *@param pName - bone name to find
        *@return bone, 0 if not found or on error
        */
        CSR_Bone* csrBoneFind(const CSR_Bone* pBone, const char* pName);

        /**
        * Gets the bone matrix
        *@param pBone - bone for which the matrix should be get
        *@param pInitialMatrix - initial matrix from which the bone matrix should be get
        *@param[out] pMatrix - bone matrix
        */
        void csrBoneGetMatrix(const CSR_Bone* pBone, CSR_Matrix4* pInitialMatrix, CSR_Matrix4* pMatrix);

        /**
        * Gets the bone matrix matching with a running animation frame
        *@param pBone - bone for which the matrix should be get
        *@param pAnimSet - animation set containing the animated bones
        *@param frameIndex - frame index to process
        *@param pInitialMatrix - initial matrix from which the bone matrix should be get
        *@param[out] pMatrix - bone matrix
        */
        void csrBoneGetAnimMatrix(const CSR_Bone*              pBone,
                                  const CSR_AnimationSet_Bone* pAnimSet,
                                        size_t                 frameIndex,
                                        CSR_Matrix4*           pInitialMatrix,
                                        CSR_Matrix4*           pMatrix);

        //-------------------------------------------------------------------
        // Skeleton functions
        //-------------------------------------------------------------------

        /**
        * Creates a skeleton
        *@return newly created skeleton, 0 on error
        *@note The skeleton must be released when no longer used, see csrSkeletonRelease()
        */
        CSR_Skeleton* csrSkeletonCreate(void);

        /**
        * Releases a skeleton
        *@param[in, out] pSkeleton - skeleton to release
        *@param contentOnly - if 1, the skeleton content will be released, but not the skeleton itself
        */
        void csrSkeletonRelease(CSR_Skeleton* pSkeleton, int contentOnly);

        /**
        * Initializes a skeleton structure
        *@param[in, out] pSkeleton - skeleton to initialize
        */
        void csrSkeletonInit(CSR_Skeleton* pSkeleton);

        //-------------------------------------------------------------------
        // Skin weights functions
        //-------------------------------------------------------------------

        /**
        * Creates a skin weights
        *@return newly created skin weights, 0 on error
        *@note The skin weights must be released when no longer used, see csrSkinWeightsRelease()
        */
        CSR_Skin_Weights* csrSkinWeightsCreate(void);

        /**
        * Releases a skin weights
        *@param[in, out] pSkinWeights - skin weights to release
        *@param contentOnly - if 1, the weights content will be released, but not the weights itself
        */
        void csrSkinWeightsRelease(CSR_Skin_Weights* pSkinWeights, int contentOnly);

        /**
        * Initializes a skin weights structure
        *@param[in, out] pSkinWeights - skin weights to initialize
        */
        void csrSkinWeightsInit(CSR_Skin_Weights* pSkinWeights);

        //-------------------------------------------------------------------
        // Animation key functions
        //-------------------------------------------------------------------

        /**
        * Creates an animation key
        *@return newly created animation key, 0 on error
        *@note The animation key must be released when no longer used, see csrAnimationKeyRelease()
        */
        CSR_AnimationKey* csrAnimKeyCreate(void);

        /**
        * Releases an animation key
        *@param[in, out] pAnimationKey - animation key to release
        *@param contentOnly - if 1, the key content will be released, but not the key itself
        */
        void csrAnimKeyRelease(CSR_AnimationKey* pAnimationKey, int contentOnly);

        /**
        * Initializes an animation key structure
        *@param[in, out] pAnimationKey - animation key to initialize
        */
        void csrAnimKeyInit(CSR_AnimationKey* pAnimationKey);

        //-------------------------------------------------------------------
        // Animation keys functions
        //-------------------------------------------------------------------

        /**
        * Creates an animation keys
        *@return newly created animation keys, 0 on error
        *@note The animation keys must be released when no longer used, see csrAnimationKeysRelease()
        */
        CSR_AnimationKeys* csrAnimKeysCreate(void);

        /**
        * Releases an animation keys
        *@param[in, out] pAnimationKeys - animation keys to release
        *@param contentOnly - if 1, the keys content will be released, but not the key set itself
        */
        void csrAnimKeysRelease(CSR_AnimationKeys* pAnimationKeys, int contentOnly);

        /**
        * Initializes an animation keys structure
        *@param[in, out] pAnimationKeys - animation keys to initialize
        */
        void csrAnimKeysInit(CSR_AnimationKeys* pAnimationKeys);

        //-------------------------------------------------------------------
        // Frame animation functions
        //-------------------------------------------------------------------

        /**
        * Creates a frame animation
        *@return newly created frame animation, 0 on error
        *@note The animation must be released when no longer used, see csrFrameAnimationRelease()
        */
        CSR_Animation_Frame* csrFrameAnimCreate(void);

        /**
        * Releases a frame animation
        *@param[in, out] pAnimation - animation to release
        */
        void csrFrameAnimRelease(CSR_Animation_Frame* pAnimation);

        /**
        * Initializes a frame animation structure
        *@param[in, out] pAnimation - animation to initialize
        */
        void csrFrameAnimInit(CSR_Animation_Frame* pAnimation);

        //-------------------------------------------------------------------
        // Bone animation functions
        //-------------------------------------------------------------------

        /**
        * Creates a bone animation
        *@return newly created bone animation, 0 on error
        *@note The animation must be released when no longer used, see csrBoneAnimationRelease()
        */
        CSR_Animation_Bone* csrBoneAnimCreate(void);

        /**
        * Releases a bone animation
        *@param[in, out] pAnimation - animation to release
        *@param contentOnly - if 1, the animation content will be released, but not the animation itself
        */
        void csrBoneAnimRelease(CSR_Animation_Bone* pAnimation, int contentOnly);

        /**
        * Initializes a bone animation structure
        *@param[in, out] pAnimation - animation to initialize
        */
        void csrBoneAnimInit(CSR_Animation_Bone* pAnimation);

        /**
        * Gets the animation matrix in an animation set for a bone
        *@param pAnimSet - animation set to search in
        *@param pBone - bone for which the animation should be get
        *@param frame - animation frame
        *@param[out] pMatrix - animation matrix
        *@return 1 on success, otherwise 0
        */
        int csrBoneAnimGetAnimMatrix(const CSR_AnimationSet_Bone* pAnimSet,
                                     const CSR_Bone*              pBone,
                                           size_t                 frame,
                                           CSR_Matrix4*           pMatrix);

        //-------------------------------------------------------------------
        // Bone animation set functions
        //-------------------------------------------------------------------

        /**
        * Creates a bone animation set
        *@return newly created animation set, 0 on error
        *@note The animation set must be released when no longer used, see csrAnimationSetRelease()
        */
        CSR_AnimationSet_Bone* csrBoneAnimSetCreate(void);

        /**
        * Releases a bone animation set
        *@param[in, out] pAnimationSet - animation set to release
        *@param contentOnly - if 1, the animation set content will be released, but not the animation set itself
        */
        void csrBoneAnimSetRelease(CSR_AnimationSet_Bone* pAnimationSet, int contentOnly);

        /**
        * Initializes a bone animation set structure
        *@param[in, out] pAnimationSet - animation set to initialize
        */
        void csrBoneAnimSetInit(CSR_AnimationSet_Bone* pAnimationSet);

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
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrModelRelease(CSR_Model* pModel, const CSR_fOnDeleteTexture fOnDeleteTexture);

        /**
        * Initializes a model structure
        *@param[in, out] pModel - model to initialize
        */
        void csrModelInit(CSR_Model* pModel);

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
