/****************************************************************************
 * ==> CSR_X ---------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a DirectX (.x) reader                 *
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

#ifndef CSR_XH
#define CSR_XH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Texture.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_X_FORMAT_MAGIC         ((' ' << 24) + ('f' << 16) + ('o' << 8) + 'x')
#define M_X_FORMAT_VERSION03     (('3' << 8)  +  '0')
#define M_X_FORMAT_VERSION02     (('2' << 8)  +  '0')
#define M_X_FORMAT_BINARY        ((' ' << 24) + ('n' << 16) + ('i' << 8) + 'b')
#define M_X_FORMAT_TEXT          ((' ' << 24) + ('t' << 16) + ('x' << 8) + 't')
#define M_X_FORMAT_COMPRESSED    ((' ' << 24) + ('p' << 16) + ('m' << 8) + 'c')
#define M_X_FORMAT_FLOAT_BITS_32 (('2' << 24) + ('3' << 16) + ('0' << 8) + '0')
#define M_X_FORMAT_FLOAT_BITS_64 (('4' << 24) + ('6' << 16) + ('0' << 8) + '0')

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* X file tokens
*@note Tokens MSDN reference:
*      https://docs.microsoft.com/en-us/windows/desktop/direct3d9/tokens
*/
typedef enum
{
    // custom tokens
    CSR_XT_Unknown       = 0,
    // record-bearing tokens
    CSR_XT_Name          = 1,
    CSR_XT_String        = 2,
    CSR_XT_Integer       = 3,
    CSR_XT_GUID          = 5,
    CSR_XT_Integer_List  = 6,
    CSR_XT_Float_List    = 7,
    // standalone tokens
    CSR_XT_Open_Brace    = 10,
    CSR_XT_Close_Brace   = 11,
    CSR_XT_Open_Parent   = 12,
    CSR_XT_Close_Parent  = 13,
    CSR_XT_Open_Bracket  = 14,
    CSR_XT_Close_Bracket = 15,
    CSR_XT_Open_Angle    = 16,
    CSR_XT_Close_Angle   = 17,
    CSR_XT_Dot           = 18,
    CSR_XT_Comma         = 19,
    CSR_XT_Semicolon     = 20,
    CSR_XT_Template      = 31,
    CSR_XT_Word          = 40,
    CSR_XT_DWord         = 41,
    CSR_XT_Float         = 42,
    CSR_XT_Double        = 43,
    CSR_XT_Char          = 44,
    CSR_XT_UChar         = 45,
    CSR_XT_SWord         = 46,
    CSR_XT_SDWord        = 47,
    CSR_XT_Void          = 48,
    CSR_XT_LPSTR         = 49,
    CSR_XT_Unicode       = 50,
    CSR_XT_CString       = 51,
    CSR_XT_Array         = 52
} CSR_ETokens_X;

/**
* X file data structure identifiers
*/
typedef enum
{
    CSR_XI_Link_ID                   = -1, // set to -1 because not part of the official IDs
    CSR_XI_Unknown                   =  0,
    CSR_XI_Template_ID               =  60,
    CSR_XI_Header_ID                 =  61,
    CSR_XI_Frame_ID                  =  62,
    CSR_XI_Frame_Transform_Matrix_ID =  63,
    CSR_XI_Mesh_ID                   =  64,
    CSR_XI_Mesh_Texture_Coords_ID    =  65,
    CSR_XI_Mesh_Material_List_ID     =  66,
    CSR_XI_Material_ID               =  67,
    CSR_XI_Skin_Mesh_Header_ID       =  68,
    CSR_XI_Skin_Weights_ID           =  69,
    CSR_XI_Texture_Filename_ID       =  70,
    CSR_XI_Mesh_Normals_ID           =  71,
    CSR_XI_Animation_Set_ID          =  72,
    CSR_XI_Animation_ID              =  73,
    CSR_XI_Animation_Key_ID          =  74
} CSR_EDataStructID_X;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* DirectX (.x) model mesh to bone dictionary item
*/
typedef struct
{
    CSR_Bone* m_pBone;
    size_t    m_MeshIndex;
} CSR_MeshBoneItem_X;

/**
* Skin weights belonging to a mesh
*/
typedef struct
{
    CSR_Skin_Weights* m_pSkinWeights;
    size_t            m_Count;
} CSR_MeshSkinWeights_X;

/**
* DirectX (.x) model
*@note Each mesh is connected to its own weights count and skeleton, sorted in the same order in each list
*/
typedef struct
{
    CSR_Mesh*              m_pMesh;               // meshes composing the model
    size_t                 m_MeshCount;           // mesh count
    CSR_MeshSkinWeights_X* m_pMeshWeights;        // skin weights belonging to a mesh
    size_t                 m_MeshWeightsCount;    // skin weights count
    CSR_MeshBoneItem_X*    m_pMeshToBoneDict;     // mesh to bone dictionary
    size_t                 m_MeshToBoneDictCount; // mesh to bone dictionary item count
    CSR_Bone*              m_pSkeleton;           // model skeleton
    CSR_AnimationSet_Bone* m_pAnimationSet;       // set of animations to apply to bones
    size_t                 m_AnimationSetCount;   // animation set count
    int                    m_MeshOnly;            // if activated, only the mesh will be drawn. All other data will be ignored
    int                    m_PoseOnly;            // if activated, the model will take the default pose but will not be animated
} CSR_X;

/**
* X file header
*/
typedef struct
{
    unsigned       m_Magic;
    unsigned short m_Major_Version;
    unsigned short m_Minor_Version;
    unsigned       m_Format;
    unsigned       m_Float_Size;
} CSR_Header_X;

/**
* Generic dataset containing only a name
*/
typedef struct
{
    char* m_pName;
} CSR_Dataset_Generic_X;

/**
* Dataset containing a header
*/
typedef struct
{
    char*  m_pName;
    size_t m_Major;
    size_t m_Minor;
    size_t m_Flags;
    size_t m_ReadValCount;
} CSR_Dataset_Header_X;

/**
* Dataset containing a matrix
*/
typedef struct
{
    char*       m_pName;
    CSR_Matrix4 m_Matrix;
    size_t      m_ReadValCount;
} CSR_Dataset_Matrix_X;

/**
* Dataset containing an indexed vertex buffer. Used to read the meshes and normals
*/
typedef struct
{
    char*   m_pName;
    float*  m_pVertices;
    size_t  m_VerticeCount;
    size_t  m_VerticeTotal;
    size_t* m_pIndices;
    size_t  m_IndiceCount;
    size_t  m_IndiceTotal;
} CSR_Dataset_VertexBuffer_X;

/**
* Dataset containing the texture coordinates
*/
typedef struct
{
    char*  m_pName;
    float* m_pUV;
    size_t m_UVCount;
    size_t m_UVTotal;
} CSR_Dataset_TexCoords_X;

/**
* Dataset containing the mesh material list
*/
typedef struct
{
    char*   m_pName;
    size_t  m_MaterialCount;
    size_t* m_pMaterialIndices;
    size_t  m_MaterialIndiceCount;
    size_t  m_MaterialIndiceTotal;
} CSR_Dataset_MaterialList_X;

/**
* Dataset containing the mesh material list
*/
typedef struct
{
    char*     m_pName;
    CSR_Color m_Color;
    float     m_SpecularExp;
    CSR_Color m_SpecularColor;
    CSR_Color m_EmisiveColor;
    size_t    m_ReadValCount;
} CSR_Dataset_Material_X;

/**
* Dataset containing the mesh texture
*/
typedef struct
{
    char* m_pName;
    char* m_pFileName;
} CSR_Dataset_Texture_X;

/**
* Dataset containing the skin weights
*/
typedef struct
{
    char*       m_pName;
    char*       m_pBoneName;
    size_t      m_ItemCount;
    size_t*     m_pIndices;
    size_t      m_IndiceCount;
    float*      m_pWeights;
    size_t      m_WeightCount;
    CSR_Matrix4 m_Matrix;
    size_t      m_ReadValCount;
    size_t      m_BoneIndex;
    size_t      m_MeshIndex;
} CSR_Dataset_SkinWeights_X;

/**
* Dataset containing the animation key
*/
typedef struct
{
    int    m_Frame;
    float* m_pValues;
    size_t m_Count;
    size_t m_Total;
} CSR_Dataset_AnimationKey_X;

/**
* Dataset containing the animation keys
*/
typedef struct
{
    char*                       m_pName;
    CSR_EAnimKeyType            m_Type;
    CSR_Dataset_AnimationKey_X* m_pKeys;
    size_t                      m_KeyCount;
    size_t                      m_KeyTotal;
    size_t                      m_KeyIndex;
    size_t                      m_ReadValCount;
} CSR_Dataset_AnimationKeys_X;

/**
* X item
*/
typedef struct CSR_tagItemX
{
           CSR_EDataStructID_X m_ID;
    struct CSR_tagItemX*       m_pParent;
    struct CSR_tagItemX*       m_pChildren;
           size_t              m_ChildrenCount;
           void*               m_pData;
           int                 m_Opened;
           int                 m_ContentRead;
} CSR_Item_X;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // X model functions
        //-------------------------------------------------------------------

        /**
        * Creates a X model from a buffer
        *@param pBuffer - buffer containing the X data to read
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param meshOnly - if 1, only the mesh will be drawn. All other data will be ignored
        *@param poseOnly - if 1, the model will take the default pose but will not be animated
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnLoadTexture - called when a texture should be loaded
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return the newly created X model, 0 on error
        *@note The X model must be released when no longer used, see csrXModelRelease()
        */
        CSR_X* csrXCreate(const CSR_Buffer*           pBuffer,
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
        * Opens a X model from a file
        *@param pFileName - X model file name
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param meshOnly - if 1, only the mesh will be drawn. All other data will be ignored
        *@param poseOnly - if 1, the model will take the default pose but will not be animated
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnLoadTexture - called when a texture should be loaded
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return the newly created X model, 0 on error
        *@note The X model must be released when no longer used, see csrXModelRelease()
        */
        CSR_X* csrXOpen(const char*                 pFileName,
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
        * Releases a X model
        *@param[in, out] pX - X model to release
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrXRelease(CSR_X* pX, const CSR_fOnDeleteTexture fOnDeleteTexture);

        /**
        * Initializes a X model structure
        *@param[in, out] pX - X model to initialize
        */
        void csrXInit(CSR_X* pX);

        /**
        * Parses the x file content
        *@param pBuffer - buffer containing the x file to parse
        *@param[in, out] pOffset - offset
        *@param[in, out] pItem - x item
        *@return 1 on success, otherwise 0
        */
        int csrXParse(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_Item_X** pItem);

        /**
        * Parses a X word
        *@param pBuffer - buffer containing the X data to read
        *@param startOffset - word start offset
        *@param endOffset - word end offset
        *@param[in, out] pItem - x item
        *@return 1 on success, otherwise 0
        */
        int csrXParseWord(const CSR_Buffer*  pBuffer,
                                size_t       startOffset,
                                size_t       endOffset,
                                CSR_Item_X** pItem);

        /**
        * Converts an item hierarchy to a x model
        *@param pItem - root item from which model should be converted
        *@param[in, out] pX - converted x model
        *@param[in, out] pBone - current bone, if 0 will be considered as the root bone
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnLoadTexture - called when a texture should be loaded
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return 1 on success, otherwise 0
        */
        int csrXItemToModel(const CSR_Item_X*           pItem,
                                  CSR_X*                pX,
                                  CSR_Bone*             pBone,
                            const CSR_VertexFormat*     pVertFormat,
                            const CSR_VertexCulling*    pVertCulling,
                            const CSR_Material*         pMaterial,
                            const CSR_fOnGetVertexColor fOnGetVertexColor,
                            const CSR_fOnLoadTexture    fOnLoadTexture,
                            const CSR_fOnApplySkin      fOnApplySkin,
                            const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Releases the X objects used during the opening
        *@param pItem - root item from which this and all children will be released
        *@param contentOnly - if 1, the item content will be released, but not the item itself
        */
        void csrXReleaseItems(CSR_Item_X* pItem, int contentOnly);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_X.c"
#endif

#endif
