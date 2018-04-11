/****************************************************************************
 * ==> CSR_Collision -------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the colision detection functions      *
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

#include "CSR_Collision.h"

// std
#include <stdlib.h>

//---------------------------------------------------------------------------
// Aligned-Axis Bounding Box tree functions
//---------------------------------------------------------------------------
int csrAABBTreeFromIndexedPolygonBuffer(const CSR_IndexedPolygonBuffer* pIPB,
                                              CSR_AABBNode*             pNode)
{
    size_t                    i;
    size_t                    j;
    CSR_Box                   leftBox;
    CSR_Box                   rightBox;
    CSR_Polygon3              polygon;
    CSR_IndexedPolygon*       pNewPolygons    = 0;
    CSR_IndexedPolygonBuffer* pLeftPolygons   = 0;
    CSR_IndexedPolygonBuffer* pRightPolygons  = 0;
    int                       boxEmpty        = 1;
    int                       insideLeft      = 0;
    int                       insideRight     = 0;
    int                       canResolveLeft  = 0;
    int                       canResolveRight = 0;
    int                       result          = 0;

    // no indexed polygon buffer?
    if (!pIPB)
        return 0;

    // no node?
    if (!pNode)
        return 0;

    // initialize node content
    pNode->m_pParent        = 0;
    pNode->m_pLeft          = 0;
    pNode->m_pRight         = 0;
    pNode->m_pBox           = (CSR_Box*)malloc(sizeof(CSR_Box));
    pNode->m_pPolygonBuffer = csrIndexedPolygonBufferCreate();

    // succeeded?
    if (!pNode->m_pBox || !pNode->m_pPolygonBuffer)
    {
        csrAABBTreeNodeContentRelease(pNode);
        return 0;
    }

    // create the polygon buffers that will contain the divided polygons
    pLeftPolygons  = csrIndexedPolygonBufferCreate();
    pRightPolygons = csrIndexedPolygonBufferCreate();

    // succeeded?
    if (!pLeftPolygons || !pRightPolygons)
    {
        csrIndexedPolygonBufferRelease(pLeftPolygons);
        csrIndexedPolygonBufferRelease(pRightPolygons);
        csrAABBTreeNodeContentRelease(pNode);
        return 0;
    }

    // iterate through polygons to divide
    for (i = 0; i < pIPB->m_Count; ++i)
    {
        // using his index, extract the polygon from his vertex buffer
        csrIndexedPolygonToPolygon(&pIPB->m_pIndexedPolygon[i], &polygon);

        // extend the bounding box to include the polygon
        csrBoxExtendToPolygon(&polygon, pNode->m_pBox, &boxEmpty);
    }

    // divide the bounding box in 2 sub-boxes
    csrBoxCut(pNode->m_pBox, &leftBox, &rightBox);

    // iterate again through polygons to divide
    for (i = 0; i < pIPB->m_Count; ++i)
    {
        // get the concrete polygon (i.e. with physical coordinates, not indexes)
        csrIndexedPolygonToPolygon(&pIPB->m_pIndexedPolygon[i], &polygon);

        insideLeft  = 0;
        insideRight = 0;

        // check which box contains the most vertices
        for (j = 0; j < 3; ++j)
            // is vertex inside left or right sub-box?
            if (csrInsideBox(&polygon.m_Vertex[j], &leftBox))
                ++insideLeft;
            else
                ++insideRight;

        // check at which sub-box the polygon belongs (and thus to which buffer it should be added)
        if (insideLeft >= insideRight)
        {
            // allocate the memory to add a new polygon in the left polygon buffer
            pNewPolygons = (CSR_IndexedPolygon*)csrMemoryAlloc(pLeftPolygons->m_pIndexedPolygon,
                                                               sizeof(CSR_IndexedPolygon),
                                                               pLeftPolygons->m_Count + 1);

            // succeeded?
            if (!pNewPolygons)
            {
                csrIndexedPolygonBufferRelease(pLeftPolygons);
                csrIndexedPolygonBufferRelease(pRightPolygons);
                csrAABBTreeNodeContentRelease(pNode);
                return 0;
            }

            // update the buffer content
            pLeftPolygons->m_pIndexedPolygon = pNewPolygons;
            ++pLeftPolygons->m_Count;

            // copy the polygon index content in the left buffer
            pLeftPolygons->m_pIndexedPolygon[pLeftPolygons->m_Count - 1] = pIPB->m_pIndexedPolygon[i];
        }
        else
        {
            // allocate the memory to add a new polygon in the left polygon buffer
            pNewPolygons = (CSR_IndexedPolygon*)csrMemoryAlloc(pRightPolygons->m_pIndexedPolygon,
                                                               sizeof(CSR_IndexedPolygon),
                                                               pRightPolygons->m_Count + 1);

            // succeeded?
            if (!pNewPolygons)
            {
                csrIndexedPolygonBufferRelease(pLeftPolygons);
                csrIndexedPolygonBufferRelease(pRightPolygons);
                csrAABBTreeNodeContentRelease(pNode);
                return 0;
            }

            // update the buffer content
            pRightPolygons->m_pIndexedPolygon = pNewPolygons;
            ++pRightPolygons->m_Count;

            // copy the polygon content inside his buffer
            pRightPolygons->m_pIndexedPolygon[pRightPolygons->m_Count - 1] = pIPB->m_pIndexedPolygon[i];
        }
    }

    canResolveLeft  = (pLeftPolygons->m_Count  && pLeftPolygons->m_Count  < pIPB->m_Count);
    canResolveRight = (pRightPolygons->m_Count && pRightPolygons->m_Count < pIPB->m_Count);

    // leaf reached?
    if (!canResolveLeft && !canResolveRight)
    {
        // iterate through left polygons to copy to the leaf polygon buffer
        for (i = 0; i < pLeftPolygons->m_Count; ++i)
        {
            // allocate the memory to add a new polygon in the leaf node
            pNewPolygons = (CSR_IndexedPolygon*)csrMemoryAlloc(pNode->m_pPolygonBuffer->m_pIndexedPolygon,
                                                               sizeof(CSR_IndexedPolygon),
                                                               pNode->m_pPolygonBuffer->m_Count + 1);

            // succeeded?
            if (!pNewPolygons)
            {
                csrIndexedPolygonBufferRelease(pLeftPolygons);
                csrIndexedPolygonBufferRelease(pRightPolygons);
                csrAABBTreeNodeContentRelease(pNode);
                return 0;
            }

            // update the buffer content
            pNode->m_pPolygonBuffer->m_pIndexedPolygon = pNewPolygons;
            ++pNode->m_pPolygonBuffer->m_Count;

            // copy the polygon content inside the polygon buffer
            pNode->m_pPolygonBuffer->m_pIndexedPolygon[pNode->m_pPolygonBuffer->m_Count - 1] =
                    pLeftPolygons->m_pIndexedPolygon[i];
        }

        // iterate through right polygons to copy to the leaf polygon buffer
        for (i = 0; i < pRightPolygons->m_Count; ++i)
        {
            // allocate the memory to add a new polygon in the leaf node
            pNewPolygons = (CSR_IndexedPolygon*)csrMemoryAlloc(pNode->m_pPolygonBuffer->m_pIndexedPolygon,
                                                               sizeof(CSR_IndexedPolygon),
                                                               pNode->m_pPolygonBuffer->m_Count + 1);

            // succeeded?
            if (!pNewPolygons)
            {
                csrIndexedPolygonBufferRelease(pLeftPolygons);
                csrIndexedPolygonBufferRelease(pRightPolygons);
                csrAABBTreeNodeContentRelease(pNode);
                return 0;
            }

            // update the buffer content
            pNode->m_pPolygonBuffer->m_pIndexedPolygon = pNewPolygons;
            ++pNode->m_pPolygonBuffer->m_Count;

            // copy the polygon content inside the polygon buffer
            pNode->m_pPolygonBuffer->m_pIndexedPolygon[pNode->m_pPolygonBuffer->m_Count - 1] =
                    pRightPolygons->m_pIndexedPolygon[i];
        }

        // release the left and right polygon buffers, as they will no longer be used
        csrIndexedPolygonBufferRelease(pLeftPolygons);
        csrIndexedPolygonBufferRelease(pRightPolygons);

        return 1;
    }

    // do create left node?
    if (canResolveLeft)
    {
        // create the left node
        pNode->m_pLeft = (CSR_AABBNode*)malloc(sizeof(CSR_AABBNode));

        // populate it
        result |= csrAABBTreeFromIndexedPolygonBuffer(pLeftPolygons, pNode->m_pLeft);

        // set node parent. IMPORTANT must be done after the node is populated (because this value
        // will be reseted while the node is filled by csrAABBTreeFromIndexedPolygonBuffer())
        pNode->m_pLeft->m_pParent = pNode;

        // delete left polygon buffer, as it will no longer be used
        csrIndexedPolygonBufferRelease(pLeftPolygons);
    }

    // do create right node?
    if (canResolveRight)
    {
        // create the right node
        pNode->m_pRight = (CSR_AABBNode*)malloc(sizeof(CSR_AABBNode));

        // populate it
        result |= csrAABBTreeFromIndexedPolygonBuffer(pRightPolygons, pNode->m_pRight);

        // set node parent. IMPORTANT must be done after the node is populated (because this value
        // will be reseted while the node is filled by csrAABBTreeFromIndexedPolygonBuffer())
        pNode->m_pRight->m_pParent = pNode;

        // delete right polygon buffer, as it will no longer be used
        csrIndexedPolygonBufferRelease(pRightPolygons);
    }

    return result;
}
//---------------------------------------------------------------------------
CSR_AABBNode* csrAABBTreeFromMesh(const CSR_Mesh* pMesh)
{
    CSR_AABBNode* pRoot;
    int           success;

    // get indexed polygon buffer from mesh
    CSR_IndexedPolygonBuffer* pIPB = csrIndexedPolygonBufferFromMesh(pMesh);

    // succeeded?
    if (!pIPB)
        return 0;

    // create the root node
    pRoot = (CSR_AABBNode*)malloc(sizeof(CSR_AABBNode));

    // succeeded?
    if (!pRoot)
    {
        csrIndexedPolygonBufferRelease(pIPB);
        return 0;
    }

    // populate the AABB tree
    success = csrAABBTreeFromIndexedPolygonBuffer(pIPB, pRoot);

    // release the polygon buffer
    csrIndexedPolygonBufferRelease(pIPB);

    // tree was populated successfully?
    if (!success)
    {
        csrAABBTreeNodeRelease(pRoot);
        return 0;
    }

    return pRoot;
}
//---------------------------------------------------------------------------
int csrAABBTreeResolve(const CSR_Ray3*           pRay,
                       const CSR_AABBNode*       pNode,
                             size_t              deep,
                             CSR_Polygon3Buffer* pPolygons)
{
    unsigned      i;
    unsigned      j;
    int           leftResolved  = 0;
    int           rightResolved = 0;
    CSR_Polygon3* pPolygonBuffer;
    CSR_Figure3   ray;
    CSR_Figure3   box;

    // no ray?
    if (!pRay)
        return 0;

    // no node to resolve?
    if (!pNode)
        return 0;

    // no polygon buffer to contain the result?
    if (!pPolygons)
        return 0;

    // is the first iteration?
    if (!deep)
    {
        // ensure the polygon buffer is initialized, otherwise this may cause hard-to-debug bugs
        pPolygons->m_pPolygon = 0;
        pPolygons->m_Count    = 0;
    }

    // is leaf?
    if (!pNode->m_pLeft && !pNode->m_pRight)
    {
        // iterate through polygons contained in leaf
        for (i = 0; i < pNode->m_pPolygonBuffer->m_Count; ++i)
        {
            // allocate memory for a new polygon in the buffer
            pPolygonBuffer = (CSR_Polygon3*)csrMemoryAlloc(pPolygons->m_pPolygon,
                                                           sizeof(CSR_Polygon3),
                                                           pPolygons->m_Count + 1);

            // succeeded?
            if (!pPolygonBuffer)
                return 0;

            // update the polygon buffer
            pPolygons->m_pPolygon = pPolygonBuffer;
            ++pPolygons->m_Count;

            // copy the polygon content
            if (!csrIndexedPolygonToPolygon(&pNode->m_pPolygonBuffer->m_pIndexedPolygon[i],
                                            &pPolygons->m_pPolygon[pPolygons->m_Count - 1]))
                return 0;
        }

        return 1;
    }

    // convert ray to geometric figure
    ray.m_Type    = CSR_F3_Ray;
    ray.m_pFigure = pRay;

    // node contains a left child?
    if (pNode->m_pLeft)
    {
        // convert left box to geometric figure
        box.m_Type    = CSR_F3_Box;
        box.m_pFigure = pNode->m_pLeft->m_pBox;

        // check if ray intersects the left box
        if (csrIntersect3(&ray, &box, 0, 0, 0))
            // resolve left node
            leftResolved = csrAABBTreeResolve(pRay, pNode->m_pLeft, deep + 1, pPolygons);
    }

    // node contains a right child?
    if (pNode->m_pRight)
    {
        // convert right box to geometric figure
        box.m_Type    = CSR_F3_Box;
        box.m_pFigure = pNode->m_pRight->m_pBox;

        // check if ray intersects the right box
        if (csrIntersect3(&ray, &box, 0, 0, 0))
            // resolve right node
            rightResolved = csrAABBTreeResolve(pRay, pNode->m_pRight, deep + 1, pPolygons);
    }

    return (leftResolved || rightResolved);
}
//---------------------------------------------------------------------------
void csrAABBTreeNodeContentRelease(CSR_AABBNode* pNode)
{
    // release the bounding box
    if (pNode->m_pBox)
    {
        free(pNode->m_pBox);
        pNode->m_pBox = 0;
    }

    // release the polygon buffer
    if (pNode->m_pPolygonBuffer)
    {
        // release the polygon buffer content
        if (pNode->m_pPolygonBuffer->m_pIndexedPolygon)
            free(pNode->m_pPolygonBuffer->m_pIndexedPolygon);

        free(pNode->m_pPolygonBuffer);
        pNode->m_pPolygonBuffer = 0;
    }
}
//---------------------------------------------------------------------------
void csrAABBTreeNodeRelease(CSR_AABBNode* pNode)
{
    if (!pNode)
        return;

    // release all children on left side
    if (pNode->m_pLeft)
        csrAABBTreeNodeRelease(pNode->m_pLeft);

    // release all children on right side
    if (pNode->m_pRight)
        csrAABBTreeNodeRelease(pNode->m_pRight);

    // delete node content
    csrAABBTreeNodeContentRelease(pNode);

    // delete node
    free(pNode);
}
//---------------------------------------------------------------------------
// Collision info functions
//---------------------------------------------------------------------------
CSR_CollisionInfo* csrCollisionInfoCreate(void)
{
    // create a new collision info
    CSR_CollisionInfo* pCI = (CSR_CollisionInfo*)malloc(sizeof(CSR_CollisionInfo));

    // succeeded?
    if (!pCI)
        return 0;

    // initialize the collision info content
    csrCollisionInfoInit(pCI);

    return pCI;
}
//---------------------------------------------------------------------------
void csrCollisionInfoRelease(CSR_CollisionInfo* pCI)
{
    size_t i;

    // no collision info to release?
    if (!pCI)
        return;

    // free the polygon buffer
    if (pCI->m_Polygons.m_pPolygon)
        free(pCI->m_Polygons.m_pPolygon);

    // free the model array
    if (pCI->m_pModels)
        csrArrayRelease(pCI->m_pModels);

    // free the scene
    free(pCI);
}
//---------------------------------------------------------------------------
void csrCollisionInfoInit(CSR_CollisionInfo* pCI)
{
    // no collision info to initialize?
    if (!pCI)
        return;

    // initialize the collision info
    pCI->m_Collision           = 0;
    pCI->m_SlidingPlane.m_A    = 0.0f;
    pCI->m_SlidingPlane.m_B    = 0.0f;
    pCI->m_SlidingPlane.m_C    = 0.0f;
    pCI->m_SlidingPlane.m_D    = 0.0f;
    pCI->m_Polygons.m_pPolygon = 0;
    pCI->m_Polygons.m_Count    = 0;
    pCI->m_pModels             = 0;
}
//---------------------------------------------------------------------------
void csrCollisionInfoCalculateSlidingPlane(CSR_CollisionInfo* pCollisionInfo)
{
    // validate the input
    if (!pCollisionInfo)
        return;
}
//---------------------------------------------------------------------------
