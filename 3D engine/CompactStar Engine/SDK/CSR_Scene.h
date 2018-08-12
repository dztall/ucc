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
#include "CSR_Renderer.h"

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------

#define M_CSR_NoGround 1.0f / 0.0f // i.e. infinite, this is the only case where a division by 0 is allowed

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Model type
*/
typedef enum
{
    CSR_MT_Line,
    CSR_MT_Mesh,
    CSR_MT_Model,
    CSR_MT_MDL
} CSR_EModelType;

/**
* Collision type (can be combinated)
*/
typedef enum
{
    CSR_CO_None   = 0x0,
    CSR_CO_Ground = 0x1,
    CSR_CO_Edge   = 0x2,
    CSR_CO_Mouse  = 0x4,
    CSR_CO_Custom = 0x8
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
    size_t             m_AABBTreeIndex; // aligned-axis bounding box tree index to use for the collision detection
} CSR_SceneItem;

/**
* Scene
*/
typedef struct
{
    CSR_Color        m_Color;                // the scene background color
    CSR_Matrix4      m_ProjectionMatrix;     // the scene projection matrix
    CSR_Matrix4      m_ViewMatrix;           // the scene view matrix
    CSR_Vector3      m_GroundDir;            // the ground direction in the whole scene
    CSR_Mesh*        m_pSkybox;              // skybox geometry (because there is only one skybox per scene)
    CSR_SceneItem*   m_pItem;                // the items in this list will be drawn in the scene
    size_t           m_ItemCount;            // number of items
    CSR_SceneItem*   m_pTransparentItem;     // the items in this list will be drawn on the scene end, allowing transparency
    size_t           m_TransparentItemCount; // number of transparent items
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
* Hit model (for collision detection)
*/
typedef struct
{
    void*              m_pModel;    // the hit model
    CSR_EModelType     m_Type;      // model type (a simple mesh, a model or a complex MDL model)
    CSR_Matrix4        m_Matrix;    // model matrix
    CSR_AABBNode*      m_pAABBTree; // aligned-axis bounding box tree in which the collision was found
    CSR_Polygon3Buffer m_Polygons;  // hit polygons in the model
} CSR_HitModel;

/**
* Collision input
*@note All the provided items should be in the same coordinates system as the scene models. This
*      means that any projection or view transformation should be applied, if required, before
*      adding them in this structure
*/
typedef struct
{
    CSR_Ray3    m_MouseRay;       // ray starting from the mouse position, transformed in the viewport coordinates system
    CSR_Sphere  m_BoundingSphere; // bounding sphere representing the model or point of view at his current position
    CSR_Vector3 m_CheckPos;       // the model or point of view position to check
} CSR_CollisionInput;

/**
* Collision output
*/
typedef struct
{
    CSR_ECollisionType m_Collision;      // found collision type in the scene
    float              m_GroundPos;      // the ground position on the y axis, M_CSR_NoGround if no ground was found
    CSR_Plane          m_CollisionPlane; // the collision plane, in case a collision was found
    CSR_Plane          m_GroundPlane;    // the ground plane, in case a ground was found
    CSR_Array*         m_pHitModel;      // models hit by the mouse ray
} CSR_CollisionOutput;

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
*@param pModel - model for which the shader should be get
*@param type - model type
*@return shader to use to draw the model, 0 if no shader
*@note The model will not be drawn if no shader is returned
*/
typedef void* (*CSR_fOnGetShader)(const void* pModel, CSR_EModelType type);

/**
* Called when a model index should be get
*@param pModel - model for which the index should be get
*@param[in, out] pIndex - model index
*/
typedef void (*CSR_fOnGetModelIndex)(const CSR_Model* pModel, size_t* pIndex);

/**
* Called when the MDL model indexes should be get
*@param pMDL - MDL model for which the indexes should be get
*@param[in, out] pSkinIndex - skin index
*@param[in, out] pModelIndex - model index
*@param[in, out] pMeshIndex - mesh index
*/
typedef void (*CSR_fOnGetMDLIndex)(const CSR_MDL* pMDL,
                                         size_t*  pSkinIndex,
                                         size_t*  pModelIndex,
                                         size_t*  pMeshIndex);

/**
* Called when a custom collision should be detected in a scene
*@param pScene - scene in which the models to check are contained
*@param pSceneItem - the scene item currently tested
*@param index - model matrix currently tested in the scene item
*@param pInvertedModelMatrix - invert of the currently tested model matrix
*@param pCollisionInput - collision input
*@param[in, out] pCollisionOutput - collision output
*@return 1 if collision detection is done, 0 if default collisions (ground, edge, mouse) should be processed
*@note This callback will be called only for the items containing the CSR_CO_Custom collision type
*/
typedef int (*CSR_fOnCustomDetectCollision)(const CSR_Scene*           pScene,
                                            const CSR_SceneItem*       pSceneItem,
                                                  size_t               index,
                                            const CSR_Matrix4*         pInvertedModelMatrix,
                                            const CSR_CollisionInput*  pCollisionInput,
                                                  CSR_CollisionOutput* pCollisionOutput);

//---------------------------------------------------------------------------
// Implementation
//---------------------------------------------------------------------------

/**
* Scene context
*/
struct CSR_SceneContext
{
    size_t               m_Handle;
    CSR_fOnSceneBegin    m_fOnSceneBegin;
    CSR_fOnSceneEnd      m_fOnSceneEnd;
    CSR_fOnGetModelIndex m_fOnGetModelIndex;
    CSR_fOnGetMDLIndex   m_fOnGetMDLIndex;
    CSR_fOnGetShader     m_fOnGetShader;
    CSR_fOnGetID         m_fOnGetID;
    CSR_fOnDeleteTexture m_fOnDeleteTexture;
};

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Hit model functions
        //-------------------------------------------------------------------

        /**
        * Creates a hit model structure
        *@return newly created hit model structure, 0 on error
        *@note The hit model structure must be released when no longer used, see csrHitModelRelease()
        */
        CSR_HitModel* csrHitModelCreate(void);

        /**
        * Releases a hit model structure
        *@param[in, out] pHitModel - hit model structure to release
        */
        void csrHitModelRelease(CSR_HitModel* pHitModel);

        /**
        * Initializes a hit model structure
        *@param[in, out] pHitModel - hit model structure to initialize
        */
        void csrHitModelInit(CSR_HitModel* pHitModel);

        //-------------------------------------------------------------------
        // Collision input functions
        //-------------------------------------------------------------------

        /**
        * Creates a collision input
        *@return newly created collision input, 0 on error
        *@note The collision input must be released when no longer used, see csrCollisionInputRelease()
        */
        CSR_CollisionInput* csrCollisionInputCreate(void);

        /**
        * Releases a collision input
        *@param[in, out] pCI - collision input to release
        */
        void csrCollisionInputRelease(CSR_CollisionInput* pCI);

        /**
        * Initializes a collision input
        *@param[in, out] pCI - collision input to initialize
        */
        void csrCollisionInputInit(CSR_CollisionInput* pCI);

        //-------------------------------------------------------------------
        // Collision output functions
        //-------------------------------------------------------------------

        /**
        * Creates a collision output
        *@return newly created collision output, 0 on error
        *@note The collision output must be released when no longer used, see csrCollisionOutputRelease()
        */
        CSR_CollisionOutput* csrCollisionOutputCreate(void);

        /**
        * Releases a collision output
        *@param[in, out] pCO - collision output to release
        */
        void csrCollisionOutputRelease(CSR_CollisionOutput* pCO);

        /**
        * Initializes a collision output
        *@param[in, out] pCO - collision output to initialize
        */
        void csrCollisionOutputInit(CSR_CollisionOutput* pCO);

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
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@note Only the item content is released, the item itself is not released
        */
        void csrSceneItemContentRelease(CSR_SceneItem*       pSI,
                                  const CSR_fOnDeleteTexture fOnDeleteTexture);

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
        *@param pScene - scene containing the item to check
        *@param pSceneItem - scene item against which the collision should be detected
        *@param pCollisionInput - collision input
        *@param[in, out] pCollisionOutput - collision output containing the result
        *@param fOnCustomDetectCollision - custom collision detection callback
        */
        void csrSceneItemDetectCollision(const CSR_Scene*                   pScene,
                                         const CSR_SceneItem*               pSceneItem,
                                         const CSR_CollisionInput*          pCollisionInput,
                                               CSR_CollisionOutput*         pCollisionOutput,
                                               CSR_fOnCustomDetectCollision fOnCustomDetectCollision);

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
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrSceneRelease(CSR_Scene* pScene, const CSR_fOnDeleteTexture fOnDeleteTexture);

        /**
        * Initializes a scene structure
        *@param[in, out] pScene - scene to initialize
        */
        void csrSceneInit(CSR_Scene* pScene);

        /**
        * Adds a line to a scene
        *@param pScene - scene in which the line will be added
        *@param pLine - line to add
        *@param transparent - if 1, the line is transparent, if 0 the line is opaque
        *@return the scene item containing the line on success, otherwise 0
        *@note Once successfully added, the line will be owned by the scene and should no longer be
        *      released from outside
        */
        CSR_SceneItem* csrSceneAddLine(CSR_Scene* pScene, CSR_Line* pLine, int transparent);

        /**
        * Adds a mesh to a scene
        *@param pScene - scene in which the mesh will be added
        *@param pMesh - mesh to add
        *@param transparent - if 1, the mesh is transparent, if 0 the mesh is opaque
        *@param aabb - if 1, the AABB tree will be generated for the mesh
        *@return the scene item containing the mesh on success, otherwise 0
        *@note Once successfully added, the mesh will be owned by the scene and should no longer be
        *      released from outside
        */
        CSR_SceneItem* csrSceneAddMesh(CSR_Scene* pScene, CSR_Mesh* pMesh, int transparent, int aabb);

        /**
        * Adds a model to a scene
        *@param pScene - scene in which the model will be added
        *@param pModel - model to add
        *@param transparent - if 1, the model is transparent, if 0 the model is opaque
        *@param aabb - if 1, the AABB tree will be generated for the mesh
        *@return the scene item containing the model on success, otherwise 0
        *@note Once successfully added, the model will be owned by the scene and should no longer be
        *      released from outside
        */
        CSR_SceneItem* csrSceneAddModel(CSR_Scene* pScene, CSR_Model* pModel, int transparent, int aabb);

        /**
        * Adds a MDL model to a scene
        *@param pScene - scene in which the model will be added
        *@param pMDL - model to add
        *@param transparent - if 1, the model is transparent, if 0 the model is opaque
        *@param aabb - if 1, the AABB tree will be generated for the mesh
        *@return the scene item containing the model on success, otherwise 0
        *@note Once successfully added, the MDL model will be owned by the scene and should no
        *      longer be released from outside
        */
        CSR_SceneItem* csrSceneAddMDL(CSR_Scene* pScene, CSR_MDL* pMDL, int transparent, int aabb);

        /**
        * Adds a model matrix to a scene item. Doing that the same model may be drawn several time
        * at different locations
        *@param pScene - scene in which the model will be added
        *@param pModel - model for which the matrix should be added
        *@param pMatrix - matrix to add
        *@return the scene item containing the matrix on success, otherwise 0
        *@note The added matrix is not owned by the scene. For that reason it cannot be deleted as
        *      long as the scene uses it. The caller is responsible to delete the matrix if required
        */
        CSR_SceneItem* csrSceneAddModelMatrix(CSR_Scene* pScene, const void* pModel, CSR_Matrix4* pMatrix);

        /**
        * Gets a scene item matching with a model or a matrix
        *@param pScene - scene from which the item should be get
        *@param pKey - search key, may be any model kind or a matrix
        *@return scene item, 0 if not found or on error
        */
        CSR_SceneItem* csrSceneGetItem(const CSR_Scene* pScene, const void* pKey);

        /**
        * Deletes a model or a matrix from the scene
        *@param pScene - scene from which the item should be deleted
        *@param pKey - key to delete, may be any model kind or a matrix
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@note The item and all his associated resources will be freed internally. For that reason
        *      the caller should not take care of deleting them. Be aware that the key will no longer
        *      be valid and should no longer be used after the function will be executed
        */
        void csrSceneDeleteFrom(      CSR_Scene*           pScene,
                                const void*                pKey,
                                const CSR_fOnDeleteTexture fOnDeleteTexture);

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
        *@param pCollisionInput - collision input
        *@param[in, out] pCollisionOutput - collision output containing the result
        *@param fOnCustomDetectCollision - custom detection collision callback
        */
        void csrSceneDetectCollision(const CSR_Scene*                   pScene,
                                     const CSR_CollisionInput*          pCollisionInput,
                                           CSR_CollisionOutput*         pCollisionOutput,
                                           CSR_fOnCustomDetectCollision fOnCustomDetectCollision);

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
