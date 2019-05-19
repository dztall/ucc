/****************************************************************************
 * ==> CSR_Model -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the model functions and types         *
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
#define M_MDL_Mesh_File_Version  6
#define M_MDL_ID                 (('O' << 24) + ('P' << 16) + ('D' << 8) + 'I')
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

/**
* Animation key type
*/
typedef enum
{
    CSR_KT_Unknown    = -1,
    CSR_KT_Rotation   =  0,
    CSR_KT_Scale      =  1,
    CSR_KT_Position   =  2,
    CSR_KT_MatrixKeys =  4
} CSR_EAnimKeyType;

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

typedef struct CSR_Bone         CSR_Bone;
typedef struct CSR_Skin_Weights CSR_Skin_Weights;
typedef struct CSR_Item_X       CSR_Item_X;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Bone, it's a hierarchical local transformation to apply to a mesh
*/
struct CSR_Bone
{
    char*       m_pName;         // bone name
    CSR_Matrix4 m_Matrix;        // matrix containing the bone transformation to apply
    CSR_Bone*   m_pParent;       // bone parent, root bone if 0
    CSR_Bone*   m_pChildren;     // bone children
    size_t      m_ChildrenCount; // bone children count
    void*       m_pCustomData;   // additional custom data. Be careful, this data isn't released internally
};

/**
* Skin weights index table
*/
typedef struct
{
    size_t* m_pData;
    size_t  m_Count;
} CSR_Skin_Weight_Index_Table;

/**
* Skin weights, it's a group of vertices influenced by a bone
*/
struct CSR_Skin_Weights
{
    char*                        m_pBoneName;       // linked bone name
    CSR_Bone*                    m_pBone;           // linked bone
    CSR_Matrix4                  m_Matrix;          // matrix to transform the mesh vertices to the bone space
    size_t                       m_MeshIndex;       // source mesh index
    CSR_Skin_Weight_Index_Table* m_pIndexTable;     // table containing the indices of the vertices to modify in the source mesh
    size_t                       m_IndexTableCount; // mesh indices count
    float*                       m_pWeights;        // weights indicating the bone influence on vertices, between 0.0f and 1.0f
    size_t                       m_WeightCount;     // weight count
};

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
} CSR_AnimationKeys;

/**
* Animation
*/
typedef struct
{
    char*              m_pBoneName;
    CSR_Bone*          m_pBone;
    CSR_AnimationKeys* m_pKeys;
    size_t             m_Count;
} CSR_Animation;

/**
* Set of animations
*/
typedef struct
{
    CSR_Animation* m_pAnimation;
    size_t         m_Count;
} CSR_AnimationSet;

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
* Quake I (.mdl) model
*/
typedef struct
{
    CSR_Model*          m_pModel;
    size_t              m_ModelCount;
    CSR_ModelAnimation* m_pAnimation;
    size_t              m_AnimationCount;
    CSR_Skin*           m_pSkin;
    size_t              m_SkinCount;
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
*@note Each mesh is connected to his own weights count and skeleton, sorted in the same order in each list
*/
typedef struct
{
    CSR_Mesh*              m_pMesh;               // meshes composing the model
    size_t                 m_MeshCount;           // mesh count
    CSR_VertexBuffer*      m_pPrint;              // printed meshes (i.e ready to be painted)
    size_t                 m_PrintCount;          // printed mesh count
    CSR_MeshSkinWeights_X* m_pMeshWeights;        // skin weights belonging to a mesh
    size_t                 m_MeshWeightsCount;    // skin weights count
    CSR_MeshBoneItem_X*    m_pMeshToBoneDict;     // mesh to bone dictionary
    size_t                 m_MeshToBoneDictCount; // mesh to bone dictionary item count
    CSR_Bone*              m_pSkeleton;           // model skeleton
    CSR_AnimationSet*      m_pAnimationSet;       // set of animations to apply to bones
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
struct CSR_Item_X
{
    CSR_EDataStructID_X m_ID;
    CSR_Item_X*         m_pParent;
    CSR_Item_X*         m_pChildren;
    size_t              m_ChildrenCount;
    void*               m_pData;
    int                 m_Opened;
    int                 m_ContentRead;
};

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
        void csrBoneGetAnimMatrix(const CSR_Bone*         pBone,
                                  const CSR_AnimationSet* pAnimSet,
                                        size_t            frameIndex,
                                        CSR_Matrix4*      pInitialMatrix,
                                        CSR_Matrix4*      pMatrix);

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
        CSR_AnimationKey* csrAnimationKeyCreate(void);

        /**
        * Releases an animation key
        *@param[in, out] pAnimationKey - animation key to release
        *@param contentOnly - if 1, the key content will be released, but not the key itself
        */
        void csrAnimationKeyRelease(CSR_AnimationKey* pAnimationKey, int contentOnly);

        /**
        * Initializes an animation key structure
        *@param[in, out] pAnimationKey - animation key to initialize
        */
        void csrAnimationKeyInit(CSR_AnimationKey* pAnimationKey);

        //-------------------------------------------------------------------
        // Animation keys functions
        //-------------------------------------------------------------------

        /**
        * Creates an animation keys
        *@return newly created animation keys, 0 on error
        *@note The animation keys must be released when no longer used, see csrAnimationKeysRelease()
        */
        CSR_AnimationKeys* csrAnimationKeysCreate(void);

        /**
        * Releases an animation keys
        *@param[in, out] pAnimationKeys - animation keys to release
        *@param contentOnly - if 1, the keys content will be released, but not the key set itself
        */
        void csrAnimationKeysRelease(CSR_AnimationKeys* pAnimationKeys, int contentOnly);

        /**
        * Initializes an animation keys structure
        *@param[in, out] pAnimationKeys - animation keys to initialize
        */
        void csrAnimationKeysInit(CSR_AnimationKeys* pAnimationKeys);

        //-------------------------------------------------------------------
        // Animation functions
        //-------------------------------------------------------------------

        /**
        * Creates an animation
        *@return newly created animation, 0 on error
        *@note The animation must be released when no longer used, see csrAnimationRelease()
        */
        CSR_Animation* csrAnimationCreate(void);

        /**
        * Releases an animation
        *@param[in, out] pAnimation - animation to release
        *@param contentOnly - if 1, the animation content will be released, but not the animation itself
        */
        void csrAnimationRelease(CSR_Animation* pAnimation, int contentOnly);

        /**
        * Initializes an animation structure
        *@param[in, out] pAnimation - animation to initialize
        */
        void csrAnimationInit(CSR_Animation* pAnimation);

        /**
        * Gets the animation matrix in an animation set for a bone
        *@param pAnimSet - animation set to search in
        *@param pBone - bone for which the animation should be get
        *@param frame - animation frame
        *@param[out] pMatrix - animation matrix
        *@return 1 on success, otherwise 0
        */
        int csrAnimationGetMatrix(const CSR_AnimationSet* pAnimSet,
                                  const CSR_Bone*         pBone,
                                        size_t            frame,
                                        CSR_Matrix4*      pMatrix);

        //-------------------------------------------------------------------
        // Animation set functions
        //-------------------------------------------------------------------

        /**
        * Creates an animation set
        *@return newly created animation set, 0 on error
        *@note The animation set must be released when no longer used, see csrAnimationSetRelease()
        */
        CSR_AnimationSet* csrAnimationSetCreate(void);

        /**
        * Releases an animation set
        *@param[in, out] pAnimationSet - animation set to release
        *@param contentOnly - if 1, the anim set content will be released, but not the anim set itself
        */
        void csrAnimationSetRelease(CSR_AnimationSet* pAnimationSet, int contentOnly);

        /**
        * Initializes an animation set structure
        *@param[in, out] pAnimationSet - animation set to initialize
        */
        void csrAnimationSetInit(CSR_AnimationSet* pAnimationSet);

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
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return model containing the WaveFront file on success, otherwise 0
        *@note The model content should be released using the csrModelRelease function when useless
        */
        CSR_Model* csrWaveFrontCreate(const CSR_Buffer*           pBuffer,
                                      const CSR_VertexFormat*     pVertFormat,
                                      const CSR_VertexCulling*    pVertCulling,
                                      const CSR_Material*         pMaterial,
                                      const CSR_fOnGetVertexColor fOnGetVertexColor,
                                      const CSR_fOnApplySkin      fOnApplySkin,
                                      const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Opens a WaveFront file
        *@param pFileName - WaveFront file name to open
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return model containing the WaveFront file on success, otherwise 0
        *@note The model content should be released using the csrModelRelease when useless
        */
        CSR_Model* csrWaveFrontOpen(const char*                 pFileName,
                                    const CSR_VertexFormat*     pVertFormat,
                                    const CSR_VertexCulling*    pVertCulling,
                                    const CSR_Material*         pMaterial,
                                    const CSR_fOnGetVertexColor fOnGetVertexColor,
                                    const CSR_fOnApplySkin      fOnApplySkin,
                                    const CSR_fOnDeleteTexture  fOnDeleteTexture);

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
        *@param fOnApplySkin - called when a skin should be applied to the model
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
                                  const CSR_fOnApplySkin       fOnApplySkin);

        /**
        * Builds a vertex buffer from a WaveFront data
        *@param pVertex - vertex array read in WaveFront file
        *@param pNormal - normal array read in WaveFront file
        *@param pUV - texture coordinates array read in WaveFront file
        *@param pFace - face array read in WaveFront file
        *@param[in, out] pVB - vertex buffer in which the WaveFront data should be built
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnApplySkin - called when a skin should be applied to the model
        */
        void csrWaveFrontBuildVertexBuffer(const CSR_WavefrontVertex*   pVertex,
                                           const CSR_WavefrontNormal*   pNormal,
                                           const CSR_WavefrontTexCoord* pUV,
                                           const CSR_WavefrontFace*     pFace,
                                                 CSR_VertexBuffer*      pVB,
                                           const CSR_fOnGetVertexColor  fOnGetVertexColor,
                                           const CSR_fOnApplySkin       fOnApplySkin);

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
    #include "CSR_Model.c"
#endif

#endif
