/****************************************************************************
 * ==> CSR_Scene -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the functions to draw a scene         *
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

#include "CSR_Scene.h"

// std
#include <stdlib.h>
#include <math.h>
#include <string.h>

// visual studio specific code
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif

//---------------------------------------------------------------------------
// Hit model functions
//---------------------------------------------------------------------------
CSR_HitModel* csrHitModelCreate(void)
{
    // create a new hit model
    CSR_HitModel* pHitModel = (CSR_HitModel*)malloc(sizeof(CSR_HitModel));

    // succeeded?
    if (!pHitModel)
        return 0;

    // initialize the hit model content
    csrHitModelInit(pHitModel);

    return pHitModel;
}
//---------------------------------------------------------------------------
void csrHitModelRelease(CSR_HitModel* pHitModel)
{
    // no hit model to release?
    if (!pHitModel)
        return;

    // free the found polygons
    if (pHitModel->m_Polygons.m_Count)
        free(pHitModel->m_Polygons.m_pPolygon);

    // free the hit model
    free(pHitModel);
}
//---------------------------------------------------------------------------
void csrHitModelInit(CSR_HitModel* pHitModel)
{
    // no hit model to initialize?
    if (!pHitModel)
        return;

    // initialize the hit model
    pHitModel->m_pModel              = 0;
    pHitModel->m_Type                = CSR_MT_Mesh;
    pHitModel->m_pAABBTree           = 0;
    pHitModel->m_Polygons.m_pPolygon = 0;
    pHitModel->m_Polygons.m_Count    = 0;

    // initialize the model matrix
    csrMat4Identity(&pHitModel->m_Matrix);
}
//---------------------------------------------------------------------------
// Collision input functions
//---------------------------------------------------------------------------
CSR_CollisionInput* csrCollisionInputCreate(void)
{
    // create a new collision input
    CSR_CollisionInput* pCI = (CSR_CollisionInput*)malloc(sizeof(CSR_CollisionInput));

    // succeeded?
    if (!pCI)
        return 0;

    // initialize the collision input content
    csrCollisionInputInit(pCI);

    return pCI;
}
//---------------------------------------------------------------------------
void csrCollisionInputRelease(CSR_CollisionInput* pCI)
{
    // no collision input to release?
    if (!pCI)
        return;

    // free the collision input
    free(pCI);
}
//---------------------------------------------------------------------------
void csrCollisionInputInit(CSR_CollisionInput* pCI)
{
    // no collision input to initialize?
    if (!pCI)
        return;

    // initialize the collision input
    pCI->m_MouseRay.m_Pos.m_X          = 0.0f;
    pCI->m_MouseRay.m_Pos.m_Y          = 0.0f;
    pCI->m_MouseRay.m_Pos.m_Z          = 0.0f;
    pCI->m_MouseRay.m_Dir.m_X          = 0.0f;
    pCI->m_MouseRay.m_Dir.m_Y          = 0.0f;
    pCI->m_MouseRay.m_Dir.m_Z          = 0.0f;
    pCI->m_MouseRay.m_InvDir.m_X       = 0.0f;
    pCI->m_MouseRay.m_InvDir.m_Y       = 0.0f;
    pCI->m_MouseRay.m_InvDir.m_Z       = 0.0f;
    pCI->m_BoundingSphere.m_Center.m_X = 0.0f;
    pCI->m_BoundingSphere.m_Center.m_Y = 0.0f;
    pCI->m_BoundingSphere.m_Center.m_Z = 0.0f;
    pCI->m_BoundingSphere.m_Radius     = 0.0f;
    pCI->m_CheckPos.m_X                = 0.0f;
    pCI->m_CheckPos.m_Y                = 0.0f;
    pCI->m_CheckPos.m_Z                = 0.0f;
}
//---------------------------------------------------------------------------
// Collision output functions
//---------------------------------------------------------------------------
CSR_CollisionOutput* csrCollisionOutputCreate(void)
{
    // create a new collision output
    CSR_CollisionOutput* pCO = (CSR_CollisionOutput*)malloc(sizeof(CSR_CollisionOutput));

    // succeeded?
    if (!pCO)
        return 0;

    // initialize the collision output content
    csrCollisionOutputInit(pCO);

    return pCO;
}
//---------------------------------------------------------------------------
void csrCollisionOutputRelease(CSR_CollisionOutput* pCO)
{
    size_t i;

    // no collision output to release?
    if (!pCO)
        return;

    // release the hit polygons
    if (pCO->m_pHitModel)
    {
        // iterate through hit models and free each of them
        for (i = 0; i < pCO->m_pHitModel->m_Count; ++i)
            csrHitModelRelease((CSR_HitModel*)pCO->m_pHitModel->m_pItem[i].m_pData);

        // free the hit model container
        csrArrayRelease(pCO->m_pHitModel);
    }

    // free the collision output
    free(pCO);
}
//---------------------------------------------------------------------------
void csrCollisionOutputInit(CSR_CollisionOutput* pCO)
{
    // no collision output to initialize?
    if (!pCO)
        return;

    // initialize the collision output
    pCO->m_Collision          = CSR_CO_None;
    pCO->m_GroundPos          = M_CSR_NoGround;
    pCO->m_CollisionPlane.m_A = 0.0f;
    pCO->m_CollisionPlane.m_B = 0.0f;
    pCO->m_CollisionPlane.m_C = 0.0f;
    pCO->m_CollisionPlane.m_D = 0.0f;
    pCO->m_GroundPlane.m_A    = 0.0f;
    pCO->m_GroundPlane.m_B    = 0.0f;
    pCO->m_GroundPlane.m_C    = 0.0f;
    pCO->m_GroundPlane.m_D    = 0.0f;
    pCO->m_pHitModel          = 0;
}
//---------------------------------------------------------------------------
// Scene context functions
//---------------------------------------------------------------------------
void csrSceneContextInit(CSR_SceneContext* pContext)
{
    // no context to initialize?
    if (!pContext)
        return;

    // initialize the context
    pContext->m_Handle                    = 0;
    pContext->m_fOnSceneBegin             = 0;
    pContext->m_fOnSceneEnd               = 0;
    pContext->m_fOnPrepareDraw            = 0;
    pContext->m_fOnPrepareTransparentDraw = 0;
    pContext->m_fOnGetModelIndex          = 0;
    pContext->m_fOnGetMDLIndex            = 0;
    pContext->m_fOnGetXIndex              = 0;
    pContext->m_fOnGetShader              = 0;
    pContext->m_fOnGetID                  = 0;
    pContext->m_fOnDeleteTexture          = 0;
}
//---------------------------------------------------------------------------
// Scene item private functions
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneItemDeleteModelFrom(CSR_SceneItem*       pItem,
                                           size_t               index,
                                           size_t               count,
                                     const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    CSR_SceneItem* pNewItem;

    // no item list to delete from?
    if (!pItem)
        return 0;

    // was the last model in the scene?
    if (count == 1)
        // don't create a new scene item list
        return 0;

    // create a scene item list one item smaller than the existing one
    pNewItem = (CSR_SceneItem*)csrMemoryAlloc(0, sizeof(CSR_SceneItem), count - 1);

    // succeeded?
    if (!pNewItem)
        return 0;

    // copy all the remaining items in the new list
    if (!index)
        memcpy(pNewItem, pItem + 1, (count - 1) * sizeof(CSR_SceneItem));
    else
    if (index == (count - 1))
        memcpy(pNewItem, pItem, (count - 1) * sizeof(CSR_SceneItem));
    else
    {
        memcpy(pNewItem,         pItem,             sizeof(CSR_SceneItem) *          index);
        memcpy(pNewItem + index, pItem + index + 1, sizeof(CSR_SceneItem) * (count - index - 1));
    }

    // release the item content
    csrSceneItemContentRelease(&pItem[index], fOnDeleteTexture);

    return pNewItem;
}
//---------------------------------------------------------------------------
// Scene item functions
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneItemCreate(void)
{
    // create a new scene item
    CSR_SceneItem* pSceneItem = (CSR_SceneItem*)malloc(sizeof(CSR_SceneItem));

    // succeeded?
    if (!pSceneItem)
        return 0;

    // initialize the scene item content
    csrSceneItemInit(pSceneItem);

    return pSceneItem;
}
//---------------------------------------------------------------------------
void csrSceneItemContentRelease(CSR_SceneItem*       pSceneItem,
                          const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    // no scene item to release?
    if (!pSceneItem)
        return;

    // release the model
    if (pSceneItem->m_pModel)
        switch (pSceneItem->m_Type)
        {
            case CSR_MT_Line:  free((CSR_Line*)pSceneItem->m_pModel);                   break;
            case CSR_MT_Mesh:  csrMeshRelease (pSceneItem->m_pModel, fOnDeleteTexture); break;
            case CSR_MT_Model: csrModelRelease(pSceneItem->m_pModel, fOnDeleteTexture); break;
            case CSR_MT_MDL:   csrMDLRelease  (pSceneItem->m_pModel, fOnDeleteTexture); break;
            case CSR_MT_X:     csrXRelease    (pSceneItem->m_pModel, fOnDeleteTexture); break;
        }

    // release the aligned-axis bounding box tree
    if (pSceneItem->m_pAABBTree)
    {
        size_t i;

        // release all the tree content
        for (i = 0; i < pSceneItem->m_AABBTreeCount; ++i)
        {
            // get the AABB tree root node
            CSR_AABBNode* pNode = &pSceneItem->m_pAABBTree[i];

            // release all children on left side
            if (pNode->m_pLeft)
                csrAABBTreeNodeRelease(pNode->m_pLeft);

            // release all children on right side
            if (pNode->m_pRight)
                csrAABBTreeNodeRelease(pNode->m_pRight);

            // delete node content
            csrAABBTreeNodeContentRelease(pNode);
        }

        // free the tree container
        free(pSceneItem->m_pAABBTree);
    }

    // release the matrix array
    csrArrayRelease(pSceneItem->m_pMatrixArray);

    // NOTE don't release the shader, as it's just linked with the item, not owned
}
//---------------------------------------------------------------------------
void csrSceneItemInit(CSR_SceneItem* pSceneItem)
{
    // no scene item to initialize?
    if (!pSceneItem)
        return;

    // initialize the scene item
    pSceneItem->m_pModel        = 0;
    pSceneItem->m_Type          = CSR_MT_Model;
    pSceneItem->m_CollisionType = CSR_CO_None;
    pSceneItem->m_pMatrixArray  = 0;
    pSceneItem->m_pAABBTree     = 0;
    pSceneItem->m_AABBTreeCount = 0;
    pSceneItem->m_AABBTreeIndex = 0;
}
//---------------------------------------------------------------------------
void csrSceneItemDraw(const CSR_Scene*        pScene,
                      const CSR_SceneContext* pContext,
                      const CSR_SceneItem*    pItem)
{
    void* pShader;

    // validate the inputs
    if (!pScene || !pContext || !pItem)
        return;

    pShader = 0;

    // get the shader to use with the model
    if (pContext->m_fOnGetShader)
        pShader = pContext->m_fOnGetShader(pItem->m_pModel, pItem->m_Type);

    // found one?
    if (!pShader)
        return;

    // enable the item shader
    csrShaderEnable(pShader);

    // connect the projection matrix to shader
    csrShaderConnectProjectionMatrix(pShader, &pScene->m_ProjectionMatrix);

    // connect the view matrix to shader
    csrShaderConnectViewMatrix(pShader, &pScene->m_ViewMatrix);

    // draw the model
    switch (pItem->m_Type)
    {
        case CSR_MT_Line:
            // draw the line
            csrDrawLine((const CSR_Line*)pItem->m_pModel, pShader);
            break;

        case CSR_MT_Mesh:
            // draw the mesh
            csrDrawMesh((const CSR_Mesh*)pItem->m_pModel,
                                         pShader,
                                         pItem->m_pMatrixArray,
                                         pContext->m_fOnGetID);

            break;

        case CSR_MT_Model:
        {
            size_t index = 0;

            // notify the caller that the model is about to be drawn
            if (pContext->m_fOnGetModelIndex)
                pContext->m_fOnGetModelIndex((const CSR_Model*)pItem->m_pModel, &index);

            // draw the model
            csrDrawModel((const CSR_Model*)pItem->m_pModel,
                                           index,
                                           pShader,
                                           pItem->m_pMatrixArray,
                                           pContext->m_fOnGetID);

            break;
        }

        case CSR_MT_MDL:
        {
            size_t skinIndex  = 0;
            size_t modelIndex = 0;
            size_t meshIndex  = 0;

            // notify the caller that the MDL model is about to be drawn
            if (pContext->m_fOnGetMDLIndex)
                pContext->m_fOnGetMDLIndex((const CSR_MDL*)pItem->m_pModel,
                                                          &skinIndex,
                                                          &modelIndex,
                                                          &meshIndex);

            // draw the MDL model
            csrDrawMDL((const CSR_MDL*)pItem->m_pModel,
                                       pShader,
                                       pItem->m_pMatrixArray,
                                       skinIndex,
                                       modelIndex,
                                       meshIndex,
                                       pContext->m_fOnGetID);

            break;
        }

        case CSR_MT_X:
        {
            size_t animSetIndex = 0;
            size_t frameIndex   = 0;

            // notify the caller that the MDL model is about to be drawn
            if (pContext->m_fOnGetXIndex)
                pContext->m_fOnGetXIndex((const CSR_X*)pItem->m_pModel, &animSetIndex, &frameIndex);

            // draw the X model
            csrDrawX((const CSR_X*)pItem->m_pModel,
                                   pShader,
                                   pItem->m_pMatrixArray,
                                   animSetIndex,
                                   frameIndex,
                                   pContext->m_fOnGetID);

            break;
        }
    }

    // disable the item shader
    csrShaderEnable(0);
}
//---------------------------------------------------------------------------
void csrSceneItemDetectCollision(const CSR_Scene*                   pScene,
                                 const CSR_SceneItem*               pSceneItem,
                                 const CSR_CollisionInput*          pCollisionInput,
                                       CSR_CollisionOutput*         pCollisionOutput,
                                       CSR_fOnCustomDetectCollision fOnCustomDetectCollision)
{
    #ifdef _MSC_VER
        size_t      i;
        CSR_Vector3 rayPos  = {0};
        CSR_Vector3 rayDir  = {0};
        CSR_Vector3 rayDirN = {0};
        CSR_Sphere  sphere  = {0};
    #else
        size_t      i;
        CSR_Vector3 rayPos;
        CSR_Vector3 rayDir;
        CSR_Vector3 rayDirN;
        CSR_Sphere  sphere;
    #endif

    // validate the inputs
    if (!pScene || !pSceneItem || !pCollisionInput || !pCollisionOutput)
        return;

    // can detect collision on this model?
    if (!(pSceneItem->m_CollisionType & CSR_CO_Custom) &&
         (pSceneItem->m_CollisionType == CSR_CO_None   ||
         !pSceneItem->m_pMatrixArray                   ||
         !pSceneItem->m_pMatrixArray->m_Count          ||
         !pSceneItem->m_AABBTreeCount                  ||
          pSceneItem->m_AABBTreeIndex >= pSceneItem->m_AABBTreeCount))
        return;

    // copy the sphere radius
    sphere.m_Radius = pCollisionInput->m_BoundingSphere.m_Radius;

    // iterate through each model position
    for (i = 0; i < pSceneItem->m_pMatrixArray->m_Count; ++i)
    {
        CSR_Matrix4 invertMatrix;
        float       determinant;

        // inverse the model matrix
        csrMat4Inverse(&((CSR_Matrix4*)pSceneItem->m_pMatrixArray->m_pItem->m_pData)[i],
                       &invertMatrix,
                       &determinant);

        // let the caller process custom collisions if required
        if (fOnCustomDetectCollision && pSceneItem->m_CollisionType & CSR_CO_Custom)
        {
            if (fOnCustomDetectCollision(pScene,
                                         pSceneItem,
                                         i,
                                        &invertMatrix,
                                         pCollisionInput,
                                         pCollisionOutput))
                continue;

            // because not checked above, to prevent that stupid things happen...
            if (!pSceneItem->m_AABBTreeCount || pSceneItem->m_AABBTreeIndex >= pSceneItem->m_AABBTreeCount)
                continue;
        }

        // put the bounding sphere into the model coordinate system (at the location where the
        // collision should be checked)
        csrMat4Transform(&invertMatrix, &pCollisionInput->m_CheckPos, &sphere.m_Center);

        // do detect the ground collision on this model?
        if (pSceneItem->m_CollisionType & CSR_CO_Ground)
        {
            CSR_Polygon3 groundPolygon;
            float        posY;

            // calculate the y position where to place the point of view
            if (csrGroundPosY(&sphere,
                              &pSceneItem->m_pAABBTree[pSceneItem->m_AABBTreeIndex],
                              &pScene->m_GroundDir,
                              &groundPolygon,
                              &posY))
            {
                CSR_Plane   polygonPlane;
                CSR_Matrix4 transposedMatrix;

                // notify that a ground collision happened
                pCollisionOutput->m_Collision |= CSR_CO_Ground;

                // set the new ground position
                pCollisionOutput->m_GroundPos = posY;

                // calculate and set the new ground plane
                csrPlaneFromPoints(&groundPolygon.m_Vertex[0],
                                   &groundPolygon.m_Vertex[1],
                                   &groundPolygon.m_Vertex[2],
                                   &polygonPlane);
                csrMat4Transpose(&invertMatrix, &transposedMatrix);
                csrPlaneTransform(&polygonPlane, &transposedMatrix, &pCollisionOutput->m_GroundPlane);
            }
        }

        // do detect the edge collision on this model?
        if (pSceneItem->m_CollisionType & CSR_CO_Edge)
        {
            CSR_Vector3 motionDir;
            CSR_Vector3 motionDirN;
            CSR_Ray3    motionRay;

            // calculate the motion ray and put it into the model coordinate system
            csrVec3Sub(&pCollisionInput->m_CheckPos, &pCollisionInput->m_BoundingSphere.m_Center, &motionDir);
            csrVec3Normalize(&motionDir, &motionDirN);
            csrMat4ApplyToVector(&invertMatrix, &pCollisionInput->m_BoundingSphere.m_Center, &rayPos);
            csrMat4ApplyToNormal(&invertMatrix, &motionDir, &rayDir);
            csrVec3Normalize(&rayDir, &rayDirN);
            csrRay3FromPointDir(&rayPos, &rayDirN, &motionRay);

            // 1. detect if the motion ray intersects one of the polygon. If yes the detection is terminated
            // 2. detect if the sphere intersects one of the polygon

            /*
            CSR_Polygon3Buffer polygonBuffer;

            // check for collision
            if (csrAABBTreeResolve(&transformedRay,
                                   &pSceneItem->m_pAABBTree[pSceneItem->m_AABBTreeIndex],
                                    0,
                                   &polygonBuffer))
            {
                // found at least 1 collision
                pCollisionInfo->m_Collision = 1;

                // FIXME calculate the resulting sliding plane
            }

            // delete found polygons (no longer needed from now)
            if (polygonBuffer.m_Count)
                free(polygonBuffer.m_pPolygon);
            */
        }

        // do detect the mouse collision on this model?
        if (pSceneItem->m_CollisionType & CSR_CO_Mouse)
        {
            CSR_Ray3      mouseRay;
            CSR_HitModel* pHitModel;

            // put the mouse ray into the model coordinate system
            csrMat4ApplyToVector(&invertMatrix, &pCollisionInput->m_MouseRay.m_Pos, &rayPos);
            csrMat4ApplyToNormal(&invertMatrix, &pCollisionInput->m_MouseRay.m_Dir, &rayDir);
            csrVec3Normalize(&rayDir, &rayDirN);
            csrRay3FromPointDir(&rayPos, &rayDirN, &mouseRay);

            // create a new hit model container, if required
            if (!pCollisionOutput->m_pHitModel)
                pCollisionOutput->m_pHitModel = csrArrayCreate();

            // create a new hit model
            pHitModel = csrHitModelCreate();

            // succeeded?
            if (!pHitModel)
                continue;

            // using the mouse ray, resolve aligned-axis bounding box tree
            csrAABBTreeResolve(&mouseRay,
                               &pSceneItem->m_pAABBTree[pSceneItem->m_AABBTreeIndex],
                                0,
                               &pHitModel->m_Polygons);

            // found a collision with the mouse ray?
            if (pHitModel->m_Polygons.m_Count)
            {
                // notify that a mouse collision happened
                pCollisionOutput->m_Collision |= CSR_CO_Mouse;

                // populate the hit model structure
                pHitModel->m_pModel    = pSceneItem->m_pModel;
                pHitModel->m_Type      = pSceneItem->m_Type;
                pHitModel->m_Matrix    = ((CSR_Matrix4*)pSceneItem->m_pMatrixArray->m_pItem->m_pData)[i];
                pHitModel->m_pAABBTree = &pSceneItem->m_pAABBTree[pSceneItem->m_AABBTreeIndex];

                // add the hit model structure in the array
                csrArrayAdd(pHitModel, pCollisionOutput->m_pHitModel, 0);
            }
            else
            {
                // no found collision, release the hit model
                csrHitModelRelease(pHitModel);
            }
        }
    }
}
//---------------------------------------------------------------------------
// Scene functions
//---------------------------------------------------------------------------
CSR_Scene* csrSceneCreate(void)
{
    // create a new scene
    CSR_Scene* pScene = (CSR_Scene*)malloc(sizeof(CSR_Scene));

    // succeeded?
    if (!pScene)
        return 0;

    // initialize the scene content
    csrSceneInit(pScene);

    return pScene;
}
//---------------------------------------------------------------------------
void csrSceneRelease(CSR_Scene* pScene, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t i;

    // no scene to release?
    if (!pScene)
        return;

    // do free the skybox?
    if (pScene->m_pSkybox)
        csrMeshRelease(pScene->m_pSkybox, fOnDeleteTexture);

    // do free the normal items content?
    if (pScene->m_pItem)
    {
        // iterate through each scene item to release, and release each of them
        for (i = 0; i < pScene->m_ItemCount; ++i)
            csrSceneItemContentRelease(&pScene->m_pItem[i], fOnDeleteTexture);

        // free the scene items
        free(pScene->m_pItem);
    }

    // do free the transparent items content?
    if (pScene->m_pTransparentItem)
    {
        // iterate through each scene transparent item to release, and release each of them
        for (i = 0; i < pScene->m_TransparentItemCount; ++i)
            csrSceneItemContentRelease(&pScene->m_pTransparentItem[i], fOnDeleteTexture);

        // free the scene transparent items
        free(pScene->m_pTransparentItem);
    }

    // free the scene
    free(pScene);
}
//---------------------------------------------------------------------------
void csrSceneInit(CSR_Scene* pScene)
{
    // no scene to initialize?
    if (!pScene)
        return;

    // initialize the scene
    pScene->m_Color.m_R            =  0.0f;
    pScene->m_Color.m_G            =  0.0f;
    pScene->m_Color.m_B            =  0.0f;
    pScene->m_Color.m_A            =  1.0f;
    pScene->m_GroundDir.m_X        =  0.0f;
    pScene->m_GroundDir.m_Y        = -1.0f;
    pScene->m_GroundDir.m_Z        =  0.0f;
    pScene->m_pSkybox              =  0;
    pScene->m_pItem                =  0;
    pScene->m_ItemCount            =  0;
    pScene->m_pTransparentItem     =  0;
    pScene->m_TransparentItemCount =  0;

    // set the default item matrix to identity
    csrMat4Identity(&pScene->m_ViewMatrix);
}
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneAddLine(CSR_Scene* pScene, CSR_Line* pLine, int transparent)
{
    CSR_SceneItem* pItem;
    int            index;

    // validate the inputs
    if (!pScene || !pLine)
        return 0;

    // search for a scene item which already contains the same line
    pItem = csrSceneGetItem(pScene, pLine);

    // found one?
    if (pItem)
        return pItem;

    // do add a transparent line?
    if (transparent)
    {
        // add a new item to the transparent items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pTransparentItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_TransparentItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the item index to update
        index = (int)pScene->m_TransparentItemCount;
    }
    else
    {
        // add a new item to the scene items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_ItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the scene item index to update
        index = (int)pScene->m_ItemCount;
    }

    // initialize the newly created item with the default values
    csrSceneItemInit(&pItem[index]);

    // configure the item
    pItem[index].m_pModel = pLine;
    pItem[index].m_Type   = CSR_MT_Line;

    // do add a transparent item?
    if (transparent)
    {
        // add item to the transparent item list
        pScene->m_pTransparentItem = pItem;
        ++pScene->m_TransparentItemCount;
    }
    else
    {
        // add item to the normal item list
        pScene->m_pItem = pItem;
        ++pScene->m_ItemCount;
    }

    return &pItem[index];
}
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneAddMesh(CSR_Scene* pScene, CSR_Mesh* pMesh, int transparent, int aabb)
{
    CSR_SceneItem* pItem;
    int            index;

    // validate the inputs
    if (!pScene || !pMesh)
        return 0;

    // search for a scene item which already contains the same mesh
    pItem = csrSceneGetItem(pScene, pMesh);

    // found one?
    if (pItem)
        return pItem;

    // do add a transparent item?
    if (transparent)
    {
        // add a new item to the transparent items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pTransparentItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_TransparentItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the item index to update
        index = (int)pScene->m_TransparentItemCount;
    }
    else
    {
        // add a new item to the scene items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_ItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the scene item index to update
        index = (int)pScene->m_ItemCount;
    }

    // initialize the newly created item with the default values
    csrSceneItemInit(&pItem[index]);

    // configure the item
    pItem[index].m_pModel = pMesh;
    pItem[index].m_Type   = CSR_MT_Mesh;

    // generate the aligned-axis bounding box tree for this mesh
    if (aabb)
    {
        pItem[index].m_AABBTreeCount = 1;
        pItem[index].m_pAABBTree     = csrAABBTreeFromMesh(pMesh);

        // succeeded?
        if (!pItem[index].m_pAABBTree)
        {
            // realloc to the previous size, thus the latest added item will be freed
            if (transparent)
                pScene->m_pTransparentItem =
                        (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                       sizeof(CSR_SceneItem),
                                                       pScene->m_TransparentItemCount);
            else
                pScene->m_pItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                 sizeof(CSR_SceneItem),
                                                                 pScene->m_ItemCount);

            return 0;
        }
    }

    // do add a transparent item?
    if (transparent)
    {
        // add item to the transparent item list
        pScene->m_pTransparentItem = pItem;
        ++pScene->m_TransparentItemCount;
    }
    else
    {
        // add item to the normal item list
        pScene->m_pItem = pItem;
        ++pScene->m_ItemCount;
    }

    return &pItem[index];
}
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneAddModel(CSR_Scene* pScene, CSR_Model* pModel, int transparent, int aabb)
{
    CSR_SceneItem* pItem;
    int            index;

    // validate the inputs
    if (!pScene || !pModel)
        return 0;

    // search for a scene item which already contains the same model
    pItem = csrSceneGetItem(pScene, pModel);

    // found one?
    if (pItem)
        return pItem;

    // do add a transparent item?
    if (transparent)
    {
        // add a new item to the transparent items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pTransparentItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_TransparentItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the item index to update
        index = (int)pScene->m_TransparentItemCount;
    }
    else
    {
        // add a new item to the scene items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_ItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the scene item index to update
        index = (int)pScene->m_ItemCount;
    }

    // initialize the newly created item with the default values
    csrSceneItemInit(&pItem[index]);

    // configure the item
    pItem[index].m_pModel = pModel;
    pItem[index].m_Type   = CSR_MT_Model;

    // generate the aligned-axis bounding box tree for this model
    if (aabb)
    {
        size_t i;

        // reserve memory for all the AABB trees to create
        pItem[index].m_AABBTreeCount = pModel->m_MeshCount;
        pItem[index].m_pAABBTree     = (CSR_AABBNode*)csrMemoryAlloc(0,
                                                                     sizeof(CSR_AABBNode),
                                                                     pItem[index].m_AABBTreeCount);

        // succeeded?
        if (!pItem[index].m_pAABBTree)
        {
            // realloc to the previous size, thus the latest added item will be freed
            if (transparent)
                pScene->m_pTransparentItem =
                        (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                       sizeof(CSR_SceneItem),
                                                       pScene->m_TransparentItemCount);
            else
                pScene->m_pItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                 sizeof(CSR_SceneItem),
                                                                 pScene->m_ItemCount);

            return 0;
        }

        // iterate through the model meshes
        for (i = 0; i < pModel->m_MeshCount; ++i)
        {
            // create a new tree for the mesh
            CSR_AABBNode* pAABBTree = csrAABBTreeFromMesh(&pModel->m_pMesh[i]);

            // succeeded?
            if (!pAABBTree)
            {
                // realloc to the previous size, thus the latest added item will be freed
                if (transparent)
                    pScene->m_pTransparentItem =
                            (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                           sizeof(CSR_SceneItem),
                                                           pScene->m_TransparentItemCount);
                else
                    pScene->m_pItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                     sizeof(CSR_SceneItem),
                                                                     pScene->m_ItemCount);

                return 0;
            }

            // copy the tree content
            memcpy(&pItem[index].m_pAABBTree[i], pAABBTree, sizeof(CSR_AABBNode));

            // release the source tree (NOTE reset its value before, otherwise the copied tree
            // content will also be released, which will corrupt the tree)
            pAABBTree->m_pParent        = 0;
            pAABBTree->m_pLeft          = 0;
            pAABBTree->m_pRight         = 0;
            pAABBTree->m_pBox           = 0;
            pAABBTree->m_pPolygonBuffer = 0;
            csrAABBTreeNodeRelease(pAABBTree);
        }
    }

    // do add a transparent item?
    if (transparent)
    {
        // add item to the transparent item list
        pScene->m_pTransparentItem = pItem;
        ++pScene->m_TransparentItemCount;
    }
    else
    {
        // add item to the normal item list
        pScene->m_pItem = pItem;
        ++pScene->m_ItemCount;
    }

    return &pItem[index];
}
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneAddMDL(CSR_Scene* pScene, CSR_MDL* pMDL, int transparent, int aabb)
{
    CSR_SceneItem* pItem;
    int            index;

    // validate the inputs
    if (!pScene || !pMDL)
        return 0;

    // search for a scene item which already contains the same model
    pItem = csrSceneGetItem(pScene, pMDL);

    // found one?
    if (pItem)
        return pItem;

    // do add a transparent item?
    if (transparent)
    {
        // add a new item to the transparent items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pTransparentItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_TransparentItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the item index to update
        index = (int)pScene->m_TransparentItemCount;
    }
    else
    {
        // add a new item to the scene items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_ItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the scene item index to update
        index = (int)pScene->m_ItemCount;
    }

    // initialize the newly created item with the default values
    csrSceneItemInit(&pItem[index]);

    // configure the item
    pItem[index].m_pModel = pMDL;
    pItem[index].m_Type   = CSR_MT_MDL;

    // generate the aligned-axis bounding box tree for this model
    if (aabb)
    {
        size_t i;
        size_t j;

        // reserve memory for all the AABB trees to create
        pItem[index].m_AABBTreeCount = pMDL->m_ModelCount * pMDL->m_pModel->m_MeshCount;
        pItem[index].m_pAABBTree     = (CSR_AABBNode*)csrMemoryAlloc(0,
                                                                     sizeof(CSR_AABBNode),
                                                                     pItem[index].m_AABBTreeCount);

        // succeeded?
        if (!pItem[index].m_pAABBTree)
        {
            // realloc to the previous size, thus the latest added item will be freed
            if (transparent)
                pScene->m_pTransparentItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                            sizeof(CSR_SceneItem),
                                                                            pScene->m_TransparentItemCount);
            else
                pScene->m_pItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                 sizeof(CSR_SceneItem),
                                                                 pScene->m_ItemCount);

            return 0;
        }

        // iterate through the model meshes
        for (i = 0; i < pMDL->m_ModelCount; ++i)
            for (j = 0; j < pMDL->m_pModel->m_MeshCount; ++j)
            {
                // create a new tree for the mesh
                CSR_AABBNode* pAABBTree = csrAABBTreeFromMesh(&pMDL->m_pModel[i].m_pMesh[j]);

                // succeeded?
                if (!pAABBTree)
                {
                    // realloc to the previous size, thus the latest added item will be freed
                    if (transparent)
                        pScene->m_pTransparentItem =
                                (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                               sizeof(CSR_SceneItem),
                                                               pScene->m_TransparentItemCount);
                    else
                        pScene->m_pItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                         sizeof(CSR_SceneItem),
                                                                         pScene->m_ItemCount);

                    return 0;
                }

                // copy the tree content
                memcpy(&pItem[index].m_pAABBTree[i], pAABBTree, sizeof(CSR_AABBNode));

                // release the source tree (NOTE reset its value before, otherwise the copied tree
                // content will also be released, which will corrupt the tree)
                pAABBTree->m_pParent        = 0;
                pAABBTree->m_pLeft          = 0;
                pAABBTree->m_pRight         = 0;
                pAABBTree->m_pBox           = 0;
                pAABBTree->m_pPolygonBuffer = 0;
                csrAABBTreeNodeRelease(pAABBTree);
            }
    }

    // do add a transparent item?
    if (transparent)
    {
        // add item to the transparent item list
        pScene->m_pTransparentItem = pItem;
        ++pScene->m_TransparentItemCount;
    }
    else
    {
        // add item to the normal item list
        pScene->m_pItem = pItem;
        ++pScene->m_ItemCount;
    }

    return &pItem[index];
}
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneAddX(CSR_Scene* pScene, CSR_X* pX, int transparent, int aabb)
{
    CSR_SceneItem* pItem;
    int            index;

    // validate the inputs
    if (!pScene || !pX)
        return 0;

    // search for a scene item which already contains the same model
    pItem = csrSceneGetItem(pScene, pX);

    // found one?
    if (pItem)
        return pItem;

    // do add a transparent item?
    if (transparent)
    {
        // add a new item to the transparent items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pTransparentItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_TransparentItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the item index to update
        index = (int)pScene->m_TransparentItemCount;
    }
    else
    {
        // add a new item to the scene items
        pItem = (CSR_SceneItem*)csrMemoryAlloc(pScene->m_pItem,
                                               sizeof(CSR_SceneItem),
                                               pScene->m_ItemCount + 1);

        // succeeded?
        if (!pItem)
            return 0;

        // get the scene item index to update
        index = (int)pScene->m_ItemCount;
    }

    // initialize the newly created item with the default values
    csrSceneItemInit(&pItem[index]);

    // configure the item
    pItem[index].m_pModel = pX;
    pItem[index].m_Type   = CSR_MT_X;

    // generate the aligned-axis bounding box tree for this model
    if (aabb)
    {
        size_t i;

        // reserve memory for all the AABB trees to create
        pItem[index].m_AABBTreeCount = pX->m_MeshCount;
        pItem[index].m_pAABBTree     = (CSR_AABBNode*)csrMemoryAlloc(0,
                                                                     sizeof(CSR_AABBNode),
                                                                     pItem[index].m_AABBTreeCount);

        // succeeded?
        if (!pItem[index].m_pAABBTree)
        {
            // realloc to the previous size, thus the latest added item will be freed
            if (transparent)
                pScene->m_pTransparentItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                            sizeof(CSR_SceneItem),
                                                                            pScene->m_TransparentItemCount);
            else
                pScene->m_pItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                 sizeof(CSR_SceneItem),
                                                                 pScene->m_ItemCount);

            return 0;
        }

        // iterate through the meshes
        for (i = 0; i < pX->m_MeshCount; ++i)
        {
            // create a new tree for the mesh
            CSR_AABBNode* pAABBTree = csrAABBTreeFromMesh(&pX->m_pMesh[i]);

            // succeeded?
            if (!pAABBTree)
            {
                // realloc to the previous size, thus the latest added item will be freed
                if (transparent)
                    pScene->m_pTransparentItem =
                            (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                           sizeof(CSR_SceneItem),
                                                           pScene->m_TransparentItemCount);
                else
                    pScene->m_pItem = (CSR_SceneItem*)csrMemoryAlloc(pItem,
                                                                     sizeof(CSR_SceneItem),
                                                                     pScene->m_ItemCount);

                return 0;
            }

            // copy the tree content
            memcpy(&pItem[index].m_pAABBTree[i], pAABBTree, sizeof(CSR_AABBNode));

            // release the source tree (NOTE reset its value before, otherwise the copied tree
            // content will also be released, which will corrupt the tree)
            pAABBTree->m_pParent        = 0;
            pAABBTree->m_pLeft          = 0;
            pAABBTree->m_pRight         = 0;
            pAABBTree->m_pBox           = 0;
            pAABBTree->m_pPolygonBuffer = 0;
            csrAABBTreeNodeRelease(pAABBTree);
        }
    }

    // do add a transparent item?
    if (transparent)
    {
        // add item to the transparent item list
        pScene->m_pTransparentItem = pItem;
        ++pScene->m_TransparentItemCount;
    }
    else
    {
        // add item to the normal item list
        pScene->m_pItem = pItem;
        ++pScene->m_ItemCount;
    }

    return &pItem[index];
}
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneAddModelMatrix(CSR_Scene* pScene, const void* pModel, CSR_Matrix4* pMatrix)
{
    CSR_SceneItem* pSceneItem;

    // validate inputs
    if (!pScene || !pModel || !pMatrix)
        return 0;

    // get the scene item matching with the model for which the matrix should be added
    pSceneItem = csrSceneGetItem(pScene, pModel);

    // found it?
    if (!pSceneItem)
        return 0;

    // do create a matrix array for the item?
    if (!pSceneItem->m_pMatrixArray)
    {
        // create a new matrix array
        pSceneItem->m_pMatrixArray = (CSR_Array*)csrMemoryAlloc(0, sizeof(CSR_Array), 1);

        // succeeded?
        if (!pSceneItem->m_pMatrixArray)
            return 0;

        // initialize the array content
        csrArrayInit(pSceneItem->m_pMatrixArray);
    }

    // add the matrix to the array
    csrArrayAddUnique(pMatrix, pSceneItem->m_pMatrixArray, 0);

    return pSceneItem;
}
//---------------------------------------------------------------------------
CSR_SceneItem* csrSceneGetItem(const CSR_Scene* pScene, const void* pKey)
{
    size_t i;
    size_t j;

    // validate inputs
    if (!pScene || !pKey)
        return 0;

    // first search in the standard models
    for (i = 0; i < pScene->m_ItemCount; ++i)
    {
        // found a matching model?
        if (pScene->m_pItem[i].m_pModel == pKey)
            return &pScene->m_pItem[i];

        // check also if the key is a known matrix
        if (pScene->m_pItem[i].m_pMatrixArray)
            for (j = 0; j < pScene->m_pItem[i].m_pMatrixArray->m_Count; ++j)
                if (&pScene->m_pItem[i].m_pMatrixArray->m_pItem[j].m_pData == pKey)
                    return &pScene->m_pItem[i];
    }

    // then search in the transparent models
    for (i = 0; i < pScene->m_TransparentItemCount; ++i)
    {
        // found a matching model?
        if (pScene->m_pTransparentItem[i].m_pModel == pKey)
            return &pScene->m_pTransparentItem[i];

        // check also if the key is a known matrix
        if (pScene->m_pTransparentItem[i].m_pMatrixArray)
            for (j = 0; j < pScene->m_pTransparentItem[i].m_pMatrixArray->m_Count; ++j)
                if (&pScene->m_pTransparentItem[i].m_pMatrixArray->m_pItem[j].m_pData == pKey)
                    return &pScene->m_pTransparentItem[i];
    }

    // not found
    return 0;
}
//---------------------------------------------------------------------------
void csrSceneDeleteFrom(      CSR_Scene*           pScene,
                        const void*                pKey,
                        const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t         i;
    size_t         j;
    CSR_SceneItem* pSceneItem;

    // validate inputs
    if (!pScene || !pKey)
        return;

    // first search in the standard models
    for (i = 0; i < pScene->m_ItemCount; ++i)
    {
        // found a matching model?
        if (pScene->m_pItem[i].m_pModel == pKey)
        {
            // delete the item from the list
            pSceneItem = csrSceneItemDeleteModelFrom(pScene->m_pItem,
                                                     i,
                                                     pScene->m_ItemCount,
                                                     fOnDeleteTexture);

            // update the scene content
            free(pScene->m_pItem);
            pScene->m_pItem = pSceneItem;
            --pScene->m_ItemCount;

            return;
        }

        // check also if the key is a known matrix
        if (pScene->m_pItem[i].m_pMatrixArray)
            for (j = 0; j < pScene->m_pItem[i].m_pMatrixArray->m_Count; ++j)
                if (pScene->m_pItem[i].m_pMatrixArray->m_pItem[j].m_pData == pKey)
                {
                    // delete the matrix
                    csrArrayDeleteAt(j, pScene->m_pItem[i].m_pMatrixArray);
                    return;
                }
    }

    // then search in the transparent models
    for (i = 0; i < pScene->m_TransparentItemCount; ++i)
    {
        // found a matching model?
        if (pScene->m_pTransparentItem[i].m_pModel == pKey)
        {
            // delete the item from the list
            pSceneItem = csrSceneItemDeleteModelFrom(pScene->m_pTransparentItem,
                                                     i,
                                                     pScene->m_TransparentItemCount,
                                                     fOnDeleteTexture);

            // update the scene content
            free(pScene->m_pTransparentItem);
            pScene->m_pTransparentItem = pSceneItem;
            --pScene->m_TransparentItemCount;

            return;
        }

        // check also if the key is a known matrix
        if (pScene->m_pTransparentItem[i].m_pMatrixArray)
            for (j = 0; j < pScene->m_pTransparentItem[i].m_pMatrixArray->m_Count; ++j)
                if (pScene->m_pTransparentItem[i].m_pMatrixArray->m_pItem[j].m_pData == pKey)
                {
                    // delete the matrix
                    csrArrayDeleteAt(j, pScene->m_pItem[i].m_pMatrixArray);
                    return;
                }
    }
}
//---------------------------------------------------------------------------
void csrSceneDraw(const CSR_Scene* pScene, const CSR_SceneContext* pContext)
{
    size_t i;

    // no scene to draw?
    if (!pScene)
        return;

    // no scene context?
    if (!pContext)
        return;

    // begin the scene drawing
    if (pContext->m_fOnSceneBegin)
        pContext->m_fOnSceneBegin(pScene, pContext);
    else
        csrDrawBegin(&pScene->m_Color);

    // do draw the skybox?
    if (pScene->m_pSkybox)
    {
        void* pShader = 0;

        // get the shader to use with the skybox
        if (pContext->m_fOnGetShader)
            pShader = pContext->m_fOnGetShader(pScene->m_pSkybox, CSR_MT_Mesh);

        // found one?
        if (pShader)
        {
            CSR_Matrix4 skyboxViewMatrix;

            // disable the depth buffer writing
            csrStateEnableDepthMask(0);

            // enable the skybox shader
            csrShaderEnable(pShader);

            // connect projection matrix to shader
            csrShaderConnectProjectionMatrix(pShader, &pScene->m_ProjectionMatrix);

            // copy the view matrix
            memcpy(&skyboxViewMatrix, &pScene->m_ViewMatrix.m_Table, sizeof(CSR_Matrix4));

            // remove the translation values from the view matrix
            skyboxViewMatrix.m_Table[3][0] = 0.0f;
            skyboxViewMatrix.m_Table[3][1] = 0.0f;
            skyboxViewMatrix.m_Table[3][2] = 0.0f;

            // apply the untranslated scene matrix
            csrShaderConnectViewMatrix(pShader, &skyboxViewMatrix);

            // draw the skybox
            csrDrawMesh(pScene->m_pSkybox, pShader, 0, pContext->m_fOnGetID);

            // disable the item shader
            csrShaderEnable(pShader);

            // enable the depth buffer writing again
            csrStateEnableDepthMask(1);
        }
    }

    // prepare the scene to draw common models
    if (pContext->m_fOnPrepareDraw)
        pContext->m_fOnPrepareDraw(pScene, pContext);

    // first draw the standard models
    for (i = 0; i < pScene->m_ItemCount; ++i)
        csrSceneItemDraw(pScene,
                         pContext,
                        &pScene->m_pItem[i]);

    // prepare the scene to draw transparent models
    if (pContext->m_fOnPrepareTransparentDraw)
        pContext->m_fOnPrepareTransparentDraw(pScene, pContext);

    // then draw the transparent models
    for (i = 0; i < pScene->m_TransparentItemCount; ++i)
        csrSceneItemDraw(pScene,
                         pContext,
                        &pScene->m_pTransparentItem[i]);

    // end the scene drawing
    if (pContext->m_fOnSceneEnd)
        pContext->m_fOnSceneEnd(pScene, pContext);
    else
        csrDrawEnd();
}
//---------------------------------------------------------------------------
void csrSceneArcBallToMatrix(const CSR_ArcBall* pArcball, CSR_Matrix4* pR)
{
    #ifdef _MSC_VER
        float       angleX;
        float       angleY;
        CSR_Vector3 axis           = {0};
        CSR_Matrix4 cameraMatrixX  = {0};
        CSR_Matrix4 cameraMatrixY  = {0};
        CSR_Matrix4 cameraMatrixXY = {0};
        CSR_Matrix4 cameraMatrix   = {0};
        CSR_Camera  camera         = {0};
    #else
        float       angleX;
        float       angleY;
        CSR_Vector3 axis;
        CSR_Matrix4 cameraMatrixX;
        CSR_Matrix4 cameraMatrixY;
        CSR_Matrix4 cameraMatrixXY;
        CSR_Matrix4 cameraMatrix;
        CSR_Camera  camera;
    #endif

    // validate the inputs
    if (!pArcball || !pR)
        return;

    // are angles out of bounds?
    #ifdef __CODEGEARC__
        angleX = fmod(pArcball->m_AngleX, (float)(M_PI * 2.0));
        angleY = fmod(pArcball->m_AngleY, (float)(M_PI * 2.0));
    #else
        angleX = fmodf(pArcball->m_AngleX, (float)(M_PI * 2.0));
        angleY = fmodf(pArcball->m_AngleY, (float)(M_PI * 2.0));
    #endif

    // create a matrix for the rotation on the X axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    csrMat4Rotate(angleX, &axis, &cameraMatrixX);

    // create a matrix for the rotation on the Y axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;
    csrMat4Rotate(angleY, &axis, &cameraMatrixY);

    // combine the rotation matrices
    csrMat4Multiply(&cameraMatrixY, &cameraMatrixX, &cameraMatrixXY);

    // configure the camera
    camera.m_Position.m_X =  0.0f;
    camera.m_Position.m_Y =  0.0f;
    camera.m_Position.m_Z = -pArcball->m_Radius;
    camera.m_xAngle       =  0.0f;
    camera.m_yAngle       =  0.0f;
    camera.m_zAngle       =  0.0f;
    camera.m_Factor.m_X   =  1.0f;
    camera.m_Factor.m_Y   =  1.0f;
    camera.m_Factor.m_Z   =  1.0f;
    camera.m_MatCombType  =  IE_CT_Scale_Rotate_Translate;

    // build the camera
    csrSceneCameraToMatrix(&camera, &cameraMatrix);
    csrMat4Multiply(&cameraMatrixXY, &cameraMatrix, pR);
}
//---------------------------------------------------------------------------
void csrSceneCameraToMatrix(const CSR_Camera* pCamera, CSR_Matrix4* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 axis            = {0};
        CSR_Matrix4 scaleMatrix     = {0};
        CSR_Matrix4 rotateXMatrix   = {0};
        CSR_Matrix4 rotateYMatrix   = {0};
        CSR_Matrix4 rotateZMatrix   = {0};
        CSR_Matrix4 translateMatrix = {0};
        CSR_Matrix4 buildMatrix1    = {0};
        CSR_Matrix4 buildMatrix2    = {0};
        CSR_Matrix4 buildMatrix3    = {0};
    #else
        CSR_Vector3 axis;
        CSR_Matrix4 scaleMatrix;
        CSR_Matrix4 rotateXMatrix;
        CSR_Matrix4 rotateYMatrix;
        CSR_Matrix4 rotateZMatrix;
        CSR_Matrix4 translateMatrix;
        CSR_Matrix4 buildMatrix1;
        CSR_Matrix4 buildMatrix2;
        CSR_Matrix4 buildMatrix3;
    #endif

    // validate the inputs
    if (!pCamera || !pR)
        return;

    // build a scale matrix
    csrMat4Scale(&pCamera->m_Factor, &scaleMatrix);

    // build a rotation matrix on the x axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    csrMat4Rotate(pCamera->m_xAngle, &axis, &rotateXMatrix);

    // build a rotation matrix on the y axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;
    csrMat4Rotate(pCamera->m_yAngle, &axis, &rotateYMatrix);

    // build a rotation matrix on the z axis
    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;
    csrMat4Rotate(pCamera->m_zAngle, &axis, &rotateZMatrix);

    // build a translation matrix
    csrMat4Translate(&pCamera->m_Position, &translateMatrix);

    // build model matrix
    switch (pCamera->m_MatCombType)
    {
        case IE_CT_Scale_Rotate_Translate:
            csrMat4Multiply(&scaleMatrix,  &rotateXMatrix,   &buildMatrix1);
            csrMat4Multiply(&buildMatrix1, &rotateYMatrix,   &buildMatrix2);
            csrMat4Multiply(&buildMatrix2, &rotateZMatrix,   &buildMatrix3);
            csrMat4Multiply(&buildMatrix3, &translateMatrix, pR);
            return;

        case IE_CT_Scale_Translate_Rotate:
            csrMat4Multiply(&scaleMatrix,  &translateMatrix, &buildMatrix1);
            csrMat4Multiply(&buildMatrix1, &rotateXMatrix,   &buildMatrix2);
            csrMat4Multiply(&buildMatrix2, &rotateYMatrix,   &buildMatrix3);
            csrMat4Multiply(&buildMatrix3, &rotateZMatrix,   pR);
            return;

        case IE_CT_Rotate_Translate_Scale:
            csrMat4Multiply(&rotateXMatrix, &rotateYMatrix,   &buildMatrix1);
            csrMat4Multiply(&buildMatrix1,  &rotateZMatrix,   &buildMatrix2);
            csrMat4Multiply(&buildMatrix2,  &translateMatrix, &buildMatrix3);
            csrMat4Multiply(&buildMatrix3,  &scaleMatrix,     pR);
            return;

        case IE_CT_Rotate_Scale_Translate:
            csrMat4Multiply(&rotateXMatrix, &rotateYMatrix,   &buildMatrix1);
            csrMat4Multiply(&buildMatrix1,  &rotateZMatrix,   &buildMatrix2);
            csrMat4Multiply(&buildMatrix2,  &scaleMatrix,     &buildMatrix3);
            csrMat4Multiply(&buildMatrix3,  &translateMatrix, pR);
            return;

        case IE_CT_Translate_Rotate_Scale:
            csrMat4Multiply(&translateMatrix, &rotateXMatrix, &buildMatrix1);
            csrMat4Multiply(&buildMatrix1,    &rotateYMatrix, &buildMatrix2);
            csrMat4Multiply(&buildMatrix2,    &rotateZMatrix, &buildMatrix3);
            csrMat4Multiply(&buildMatrix3,    &scaleMatrix,   pR);
            return;

        case IE_CT_Translate_Scale_Rotate:
            csrMat4Multiply(&translateMatrix, &scaleMatrix,   &buildMatrix1);
            csrMat4Multiply(&buildMatrix1,    &rotateXMatrix, &buildMatrix2);
            csrMat4Multiply(&buildMatrix2,    &rotateYMatrix, &buildMatrix3);
            csrMat4Multiply(&buildMatrix3,    &rotateZMatrix, pR);
            return;
    }
}
//---------------------------------------------------------------------------
void csrSceneDetectCollision(const CSR_Scene*                   pScene,
                             const CSR_CollisionInput*          pCollisionInput,
                                   CSR_CollisionOutput*         pCollisionOutput,
                                   CSR_fOnCustomDetectCollision fOnCustomDetectCollision)
{
    size_t i;

    // validate the inputs
    if (!pScene || !pCollisionInput || !pCollisionOutput)
        return;

    // initialize the collision output
    csrCollisionOutputInit(pCollisionOutput);

    // iterate through the scene items
    for (i = 0; i < pScene->m_ItemCount; ++i)
        csrSceneItemDetectCollision(pScene,
                                   &pScene->m_pItem[i],
                                    pCollisionInput,
                                    pCollisionOutput,
                                    fOnCustomDetectCollision);

    // iterate through the scene transparent items
    for (i = 0; i < pScene->m_TransparentItemCount; ++i)
        csrSceneItemDetectCollision(pScene,
                                   &pScene->m_pTransparentItem[i],
                                    pCollisionInput,
                                    pCollisionOutput,
                                    fOnCustomDetectCollision);
}
//---------------------------------------------------------------------------
void csrSceneTouchPosToViewportPos(const CSR_Vector2* pTouchPos,
                                   const CSR_Rect*    pTouchRect,
                                   const CSR_Rect*    pViewportRect,
                                         CSR_Vector3* pViewportPos)
{
    float touchWidth;
    float touchHeight;
    float viewportWidth;
    float viewportHeight;

    // validate the inputs
    if (!pTouchPos || !pTouchRect || !pViewportRect || !pViewportPos)
        return;

    // calculate the touch area width and height
    touchWidth  = pTouchRect->m_Max.m_X - pTouchRect->m_Min.m_X;
    touchHeight = pTouchRect->m_Max.m_Y - pTouchRect->m_Min.m_Y;

    // invalid touch width or height?
    if (!touchWidth || !touchHeight)
    {
        pViewportPos->m_X = 0.0f;
        pViewportPos->m_Y = 0.0f;
        pViewportPos->m_Z = 0.0f;
        return;
    }

    viewportWidth  = pViewportRect->m_Max.m_X - pViewportRect->m_Min.m_X;
    viewportHeight = pViewportRect->m_Min.m_Y - pViewportRect->m_Max.m_Y;

    // convert touch position to viewport position
    pViewportPos->m_X = pViewportRect->m_Min.m_X + ((pTouchPos->m_X * viewportWidth)  / touchWidth);
    pViewportPos->m_Y = pViewportRect->m_Min.m_Y - ((pTouchPos->m_Y * viewportHeight) / touchHeight);
    pViewportPos->m_Z = 0.0f;
}
//---------------------------------------------------------------------------
void csrSceneGetTouchRay(const CSR_Vector2* pTouchPos,
                         const CSR_Rect*    pTouchRect,
                         const CSR_Matrix4* pProjectionMatrix,
                         const CSR_Matrix4* pViewMatrix,
                               CSR_Ray3*    pTouchRay)
{
    #ifdef _MSC_VER
        CSR_Rect viewportRect = {0};
    #else
        CSR_Rect viewportRect;
    #endif

    // validate the inputs
    if (!pTouchPos || !pTouchRect || !pProjectionMatrix || !pViewMatrix || !pTouchRay)
        return;

    // get the viewport rectangle
    viewportRect.m_Min.m_X = -1.0f;
    viewportRect.m_Min.m_Y =  1.0f;
    viewportRect.m_Max.m_X =  1.0f;
    viewportRect.m_Max.m_Y = -1.0f;

    // get the ray in the touch coordinate system
    csrSceneTouchPosToViewportPos(pTouchPos, pTouchRect, &viewportRect, &pTouchRay->m_Pos);
    pTouchRay->m_Dir.m_X =  pTouchRay->m_Pos.m_X;
    pTouchRay->m_Dir.m_Y =  pTouchRay->m_Pos.m_Y;
    pTouchRay->m_Dir.m_Z = -1.0f;

    // put the ray in the viewport coordinates
    csrMat4Unproject(pProjectionMatrix, pViewMatrix, pTouchRay);
}
//---------------------------------------------------------------------------
