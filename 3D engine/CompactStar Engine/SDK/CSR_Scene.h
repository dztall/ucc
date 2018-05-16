/****************************************************************************
 * ==> CSR_Scene -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the functions to draw a scene         *
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

#ifndef CSR_SceneH
#define CSR_SceneH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Collision.h"
#include "CSR_Model.h"
#include "CSR_Shader.h"
#include "CSR_Renderer.h"

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Model type
*/
typedef enum
{
    CSR_MT_Mesh,
    CSR_MT_Model,
    CSR_MT_MDL
} CSR_EModelType;

/**
* Collision type
*/
typedef enum
{
    CSR_CT_Neither,
    CSR_CT_Ground,
    CSR_CT_Edge
} CSR_ECollisionType;

/**
* Matrix combination type
*/
typedef enum
{
    IE_CT_Scale_Rotate_Translate,
    IE_CT_Scale_Translate_Rotate,
    IE_CT_Rotate_Translate_Scale,
    IE_CT_Rotate_Scale_Translate,
    IE_CT_Translate_Rotate_Scale,
    IE_CT_Translate_Scale_Rotate
} CSR_EMatCombType;

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

// scene context prototype
typedef struct CSR_SceneContext CSR_SceneContext;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Scene item
*/
typedef struct
{
    void*              m_pModel;        // the model to draw
    CSR_EModelType     m_Type;          // model type (a simple mesh, a model or a complex MDL model)
    CSR_ECollisionType m_CollisionType; // collision type to apply to model
    CSR_Array*         m_pMatrixArray;  // matrices sharing the same model, e.g. all the walls of a room
    CSR_AABBNode*      m_pAABBTree;     // aligned-axis bounding box trees owned by the model
    size_t             m_AABBTreeCount; // aligned-axis bounding box tree count
} CSR_SceneItem;

/**
* Scene
*/
typedef struct
{
    CSR_Color        m_Color;
    CSR_Matrix4      m_Matrix;
    CSR_SceneItem*   m_pItem;
    size_t           m_ItemCount;
    CSR_SceneItem*   m_pTransparentItem;
    size_t           m_TransparentItemCount;
} CSR_Scene;

/**
* Camera
*/
typedef struct
{
    CSR_Vector3      m_Position;
    float            m_xAngle;
    float            m_yAngle;
    float            m_zAngle;
    CSR_Vector3      m_Factor;
    CSR_EMatCombType m_MatCombType;
} CSR_Camera;

/**
* Arcball
*/
typedef struct
{
    float m_AngleX;
    float m_AngleY;
    float m_Radius;
} CSR_ArcBall;

/**
* Collision info
*/
typedef struct
{
    int                m_Collision; // if 1 a collision happened, if 0 no collision happened
    CSR_Polygon3Buffer m_Polygons;  // all the found polygons in collision
    CSR_Array*         m_pModels;   // models owning one or several polygons in collision
} CSR_CollisionInfo;

/**
* Collision model info
*/
typedef struct
{
    void*  m_pItem;        // scene item against which a collision happened
    size_t m_MatrixIndex;  // index of the model matrix in the scene item
    size_t m_AABBTreeItem; // index of the AABB tree in the scene item
} CSR_CollisionModelInfo;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when scene begins
*@param pScene - scene to begin
*@param pContext - scene context
*/
typedef void (*CSR_fOnSceneBegin)(const CSR_Scene* pScene, const CSR_SceneContext* pContext);

/**
* Called when scene ends
*@param pScene - scene to end
*@param pContext - scene context
*/
typedef void (*CSR_fOnSceneEnd)(const CSR_Scene* pScene, const CSR_SceneContext* pContext);

/**
* Called when a shader should be get for a model
*@param pModel - model for which the shader shoudl be get
*@param type - model type
*@return shader to use to draw the model, 0 if no shader
*@note The model will not be drawn if no shader is returned
*/
typedef CSR_Shader* (*CSR_fOnGetShader)(const void* pModel, CSR_EModelType type);

/**
* Called when a model index should be get
*@param pModel - model for which the index should be get
*@param[in, out] pIndex - model index
*/
typedef void (*CSR_fOnGetModelIndex)(const CSR_Model* pModel, size_t* pIndex);

/**
* Called when the MDL model indexes should be get
*@param pMDL - MDL model for which the indexes should be get
*@param[in, out] textureIndex - texture index
*@param[in, out] modelIndex - model index
*@param[in, out] meshIndex - mesh index
*/
typedef void (*CSR_fOnGetMDLIndex)(const CSR_MDL* pMDL,
                                         size_t*  pTextureIndex,
                                         size_t*  pModelIndex,
                                         size_t*  pMeshIndex);

//---------------------------------------------------------------------------
// Implementation
//---------------------------------------------------------------------------

/**
* Scene context
*/
struct CSR_SceneContext
{
    size_t                 m_Handle;
    CSR_fOnSceneBegin      m_fOnSceneBegin;
    CSR_fOnSceneEnd        m_fOnSceneEnd;
    CSR_fOnGetShader       m_fOnGetShader;
    CSR_fOnGetModelIndex   m_fOnGetModelIndex;
    CSR_fOnGetMDLIndex     m_fOnGetMDLIndex;
};

#ifdef __cplusplus
    extern "C"
    {
#endif

        //-------------------------------------------------------------------
        // Collision info functions
        //-------------------------------------------------------------------

        /**
        * Creates a collision info
        *@return newly created collision info, 0 on error
        *@note The collision info must be released when no longer used, see csrCollisionInfoRelease()
        */
        CSR_CollisionInfo* csrCollisionInfoCreate(void);

        /**
        * Releases a collision info
        *@param[in, out] pCI - collision info to release
        */
        void csrCollisionInfoRelease(CSR_CollisionInfo* pCI);

        /**
        * Initializes a collision info structure
        *@param[in, out] pCI - collision info to initialize
        */
        void csrCollisionInfoInit(CSR_CollisionInfo* pCI);

        //-------------------------------------------------------------------
        // Scene context functions
        //-------------------------------------------------------------------

        /**
        * Initializes a scene context
        *@param[in, out] pContext - context to initialize
        */
        void csrSceneContextInit(CSR_SceneContext* pContext);

        //-------------------------------------------------------------------
        // Scene item functions
        //-------------------------------------------------------------------

        /**
        * Creates a scene item
        *@return newly created scene item, 0 on error
        *@note The scene item must be released when no longer used, see csrSceneItemContentRelease()
        */
        CSR_SceneItem* csrSceneItemCreate(void);

        /**
        * Releases a scene item content
        *@param[in, out] pSI - scene item for which the content should be released
        *@note Only the item content is released, the item itself is not released
        */
        void csrSceneItemContentRelease(CSR_SceneItem* pSI);

        /**
        * Initializes a scene item structure
        *@param[in, out] pSI - scene item to initialize
        */
        void csrSceneItemInit(CSR_SceneItem* pSI);

        /**
        * Draws a scene item
        *@param pScene - scene at which the item belongs
        *@param pContext - scene context
        *@param pItem - scene item to draw
        */
        void csrSceneItemDraw(const CSR_Scene*        pScene,
                              const CSR_SceneContext* pContext,
                              const CSR_SceneItem*    pItem);

        /**
        * Detects the collisions happening against a scene item
        *@param pScene - scene item against which the collision should be detected
        *@param pRay - ray describing the movement to check
        *@param[in, out] pCollisionInfo - collision info
        *@return 1 if a collision with the scene item was found, otherwise 0
        */
        int csrSceneItemDetectCollision(const CSR_SceneItem*     pSceneItem,
                                        const CSR_Ray3*          pRay,
                                              CSR_CollisionInfo* pCollisionInfo);

        //-------------------------------------------------------------------
        // Scene functions
        //-------------------------------------------------------------------

        /**
        * Creates a scene
        *@return newly created scene, 0 on error
        *@note The scene must be released when no longer used, see csrSceneRelease()
        */
        CSR_Scene* csrSceneCreate(void);

        /**
        * Releases a scene
        *@param[in, out] pScene - scene to release
        */
        void csrSceneRelease(CSR_Scene* pScene);

        /**
        * Initializes a scene structure
        *@param[in, out] pScene - scene to initialize
        */
        void csrSceneInit(CSR_Scene* pScene);

        /**
        * Adds a mesh to a scene
        *@param pScene - scene in which the mesh will be added
        *@param pMesh - mesh to add
        *@param transparent - if 1, the mesh is transparent, if 0 the mesh is opaque
        *@param aabb - if 1, the AABB tree will be generated for the mesh
        *@return 1 on success, otherwise 0
        *@note Once successfully added, the mesh will be owned by the scene and should no longer be
        *      released from outside
        */
        int csrSceneAddMesh(CSR_Scene* pScene, CSR_Mesh* pMesh, int transparent, int aabb);

        /**
        * Adds a model to a scene
        *@param pScene - scene in which the model will be added
        *@param pModel- model to add
        *@param transparent - if 1, the model is transparent, if 0 the model is opaque
        *@param aabb - if 1, the AABB tree will be generated for the mesh
        *@return 1 on success, otherwise 0
        *@note Once successfully added, the model will be owned by the scene and should no longer be
        *      released from outside
        */
        int csrSceneAddModel(CSR_Scene* pScene, CSR_Model* pModel, int transparent, int aabb);

        /**
        * Adds a MDL model to a scene
        *@param pScene - scene in which the model will be added
        *@param pMDL - model to add
        *@param transparent - if 1, the model is transparent, if 0 the model is opaque
        *@param aabb - if 1, the AABB tree will be generated for the mesh
        *@return 1 on success, otherwise 0
        *@note Once successfully added, the MDL model will be owned by the scene and should no
        *      longer be released from outside
        */
        int csrSceneAddMDL(CSR_Scene* pScene, CSR_MDL* pMDL, int transparent, int aabb);

        /**
        * Adds a model matrix to a scene item. Doing that the same model may be drawn several time
        * at different locations
        *@param pScene - scene in which the model will be added
        *@param pModel - model for which the matrix should be added
        *@param pMatrix - matrix to add
        *@return 1 on success, otherwise 0
        *@note The added matrix is not owned by the scene. For that reason it cannot be deleted as
        *      long as the scene uses it. The caller is responsible to delete the matrix if required
        */
        int csrSceneAddModelMatrix(CSR_Scene* pScene, const void* pModel, CSR_Matrix4* pMatrix);

        /**
        * Gets a scene item matching with a model or a matrix
        *@param pScene - scene from which the item should be get
        *@param pKey - search key, may be any model kind or a matrix
        *@return scene item, 0 if not found or on error
        */
        CSR_SceneItem* csrSceneGetItem(const CSR_Scene* pScene, const void* pKey);

        /**
        * Deletes a model or a matrix from the scene
        *@param pKey - key to delete, may be any model kind or a matrix
        */
        void csrSceneDeleteFrom(CSR_Scene* pScene, const void* pKey);

        /**
        * Draws a scene
        *@param pScene - scene to draw
        *@param pContext - scene context
        */
        void csrSceneDraw(const CSR_Scene* pScene, const CSR_SceneContext* pContext);

        /**
        * Gets a camera (or view) matrix from arcball values
        *@param pArcball - arcball values
        *@param[in, out] pR - camera (or view) matrix
        */
        void csrSceneArcBallToMatrix(const CSR_ArcBall* pArcball, CSR_Matrix4* pR);

        /**
        * Gets a camera (or view) matrix
        *@param pCamera - camera position and rotation in the 3d world
        *@param[in, out] pR - camera (or view) matrix
        */
        void csrSceneCameraToMatrix(const CSR_Camera* pCamera, CSR_Matrix4* pR);

        /**
        * Detects the collisions happening in a scene
        *@param pScene - scene in which the collisions should be detected
        *@param pRay - ray describing the movement to check
        *@param[in, out] pCollisionInfo - collision info
        *@return 1 if a collision was found in the scene, otherwise 0
        */
        int csrSceneDetectCollision(const CSR_Scene*         pScene,
                                    const CSR_Ray3*          pRay,
                                          CSR_CollisionInfo* pCollisionInfo);

        /**
        * Converts a touch position (e.g. the mouse pointer or the finger) to a viewport position
        *@param pTouchPos - touch position to convert
        *@param pTouchRect - rect surrounding the area where a touch can happen (e.g. the client
        *                    rect of a window or the screen of a device)
        *@param pViewportRect - viewport rectangle
        *@param[out] pViewportPos - viewport position
        */
        void csrSceneTouchPosToViewportPos(const CSR_Vector2* pTouchPos,
                                           const CSR_Rect*    pTouchRect,
                                           const CSR_Rect*    pViewportRect,
                                                 CSR_Vector3* pViewportPos);

        /**
        * Gets the touch ray, in viewport coordinate system, from a touched position
        *@param pTouchPos - touch position from which the ray should be get
        *@param pTouchRect - rect surrounding the area where a touch can happen (e.g. the client
        *                    rect of a window or the screen of a device)
        *@param pProjectionMatrix - projection matrix used in the viewport
        *@param pViewMatrix - view matrix matrix used in the viewport
        *@param[out] pTouchRay - the touch ray, in viewport coordinate system
        */
        void csrSceneGetTouchRay(const CSR_Vector2* pTouchPos,
                                 const CSR_Rect*    pTouchRect,
                                 const CSR_Matrix4* pProjectionMatrix,
                                 const CSR_Matrix4* pViewMatrix,
                                       CSR_Ray3*    pTouchRay);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Scene.c"
#endif

#endif