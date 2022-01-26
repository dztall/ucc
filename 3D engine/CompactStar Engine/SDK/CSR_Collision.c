/****************************************************************************
 * ==> CSR_Collision -------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the collision detection functions     *
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
        // using its index, extract the polygon from its vertex buffer
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

            // copy the polygon content inside its buffer
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
    #ifdef _MSC_VER
        unsigned      i;
        int           leftResolved   = 0;
        int           rightResolved  = 0;
        CSR_Polygon3* pPolygonBuffer = 0;
        CSR_Figure3   ray            = {0};
        CSR_Figure3   box            = {0};
    #else
        unsigned      i;
        int           leftResolved  = 0;
        int           rightResolved = 0;
        CSR_Polygon3* pPolygonBuffer;
        CSR_Figure3   ray;
        CSR_Figure3   box;
    #endif

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
// Sliding functions
//---------------------------------------------------------------------------
void csrSlidingPoint(const CSR_Plane*   pSlidingPlane,
                     const CSR_Vector3* pPosition,
                           float        radius,
                           CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        float        distanceToPlane;
        CSR_Plane    plane;
        CSR_Vector3  planeRatio       = {0};
        CSR_Vector3  pointBeyondPlane = {0};
        CSR_Vector3  pointOnPlane     = {0};
        CSR_Segment3 segment          = {0};
        CSR_Figure3  segmentFigure    = {0};
        CSR_Figure3  planeFigure      = {0};
    #else
        float        distanceToPlane;
        CSR_Plane    plane;
        CSR_Vector3  planeRatio;
        CSR_Vector3  pointBeyondPlane;
        CSR_Vector3  pointOnPlane;
        CSR_Segment3 segment;
        CSR_Figure3  segmentFigure;
        CSR_Figure3  planeFigure;
    #endif

    plane = *pSlidingPlane;

    // calculate the distance between the center of the sphere and the plane
    csrPlaneDistanceTo(pPosition, &plane, &distanceToPlane);

    // check if value is negative
    if (distanceToPlane < 0.0f)
    {
        // invert the plane
        plane.m_A = -plane.m_A;
        plane.m_B = -plane.m_B;
        plane.m_C = -plane.m_C;
        plane.m_D = -plane.m_D;
    }

    // calculate the direction of the line segment position - plane
    planeRatio.m_X = radius * plane.m_A;
    planeRatio.m_Y = radius * plane.m_B;
    planeRatio.m_Z = radius * plane.m_C;

    // calculate who the line segment perpendicular to the plane, from the center
    // of the sphere, cross the collision sphere. Normally this point is beyond
    // the plane
    pointBeyondPlane.m_X = pPosition->m_X - planeRatio.m_X;
    pointBeyondPlane.m_Y = pPosition->m_Y - planeRatio.m_Y;
    pointBeyondPlane.m_Z = pPosition->m_Z - planeRatio.m_Z;

    // configure the line segment to test
    segment.m_Start = *pPosition;
    segment.m_End   =  pointBeyondPlane;

    // build a figure containing the line segment
    segmentFigure.m_Type    =  CSR_F3_Segment;
    segmentFigure.m_pFigure = &segment;

    // build a figure containing the plane
    planeFigure.m_Type    = CSR_F3_Plane;
    planeFigure.m_pFigure = pSlidingPlane;

    // calculate the point where the segment "center of the sphere - point beyond
    // the plane" cross the collision plane
    csrIntersect3(&segmentFigure, &planeFigure, &pointOnPlane, 0, 0);

    // from point calculated above, add the sphere radius and return the value
    pR->m_X = pointOnPlane.m_X + planeRatio.m_X;
    pR->m_Y = pointOnPlane.m_Y + planeRatio.m_Y;
    pR->m_Z = pointOnPlane.m_Z + planeRatio.m_Z;
}
//---------------------------------------------------------------------------
// Ground collision functions
//---------------------------------------------------------------------------
int csrGroundCollision(const CSR_Sphere*   pSphere,
                       const CSR_Polygon3* pPolygon,
                       const CSR_Vector3*  pGroundDir,
                             CSR_Vector3*  pR)
{
    #ifdef _MSC_VER
        CSR_Ray3    ray;
        CSR_Vector3 groundDir;
        CSR_Figure3 rayToCheck     = {0};
        CSR_Figure3 polygonToCheck = {0};
    #else
        CSR_Ray3    ray;
        CSR_Vector3 groundDir;
        CSR_Figure3 rayToCheck;
        CSR_Figure3 polygonToCheck;
    #endif

    // validate the inputs
    if (!pSphere || !pPolygon)
        return 0;

    // get the ground direction
    if (pGroundDir)
        groundDir = *pGroundDir;
    else
    {
        groundDir.m_X =  0.0f;
        groundDir.m_Y = -1.0f;
        groundDir.m_Z =  0.0f;
    }

    // create the ground ray
    csrRay3FromPointDir(&pSphere->m_Center, &groundDir, &ray);

    // build the ray figure to check
    rayToCheck.m_Type    =  CSR_F3_Ray;
    rayToCheck.m_pFigure = &ray;

    // build the polygon figure to check against
    polygonToCheck.m_Type    = CSR_F3_Polygon;
    polygonToCheck.m_pFigure = pPolygon;

    // calculate the point where the ground ray hit the polygon
    if (!csrIntersect3(&rayToCheck, &polygonToCheck, pR, 0, 0))
        return 0;

    // consider the sphere radius in the result
    if (pR)
    {
        pR->m_X += (pSphere->m_Radius * -groundDir.m_X);
        pR->m_Y += (pSphere->m_Radius * -groundDir.m_Y);
        pR->m_Z += (pSphere->m_Radius * -groundDir.m_Z);
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrGroundPosY(const CSR_Sphere*   pBoundingSphere,
                  const CSR_AABBNode* pTree,
                  const CSR_Vector3*  pGroundDir,
                        CSR_Polygon3* pGroundPolygon,
                        float*        pR)
{
    size_t             i;
    CSR_Ray3           groundRay;
    CSR_Vector3        groundPos;
    CSR_Polygon3Buffer polygonBuffer;
    int                result;

    // validate the inputs
    if (!pBoundingSphere || !pTree || !pGroundDir)
        return 0;

    result = 0;

    // create the ground ray
    csrRay3FromPointDir(&pBoundingSphere->m_Center, pGroundDir, &groundRay);

    // using the ground ray, resolve aligned-axis bounding box tree
    csrAABBTreeResolve(&groundRay, pTree, 0, &polygonBuffer);

    // initialize the ground position from the bounding sphere center
    groundPos = pBoundingSphere->m_Center;

    // iterate through polygons to check
    for (i = 0; i < polygonBuffer.m_Count; ++i)
        // check if a ground polygon was found, calculate the ground position if yes
        if (csrGroundCollision(pBoundingSphere, &polygonBuffer.m_pPolygon[i], pGroundDir, &groundPos))
        {
            // copy the ground polygon, if required
            if (pGroundPolygon)
                *pGroundPolygon = polygonBuffer.m_pPolygon[i];

            result = 1;
            break;
        }

    // delete found polygons (no longer needed from now)
    if (polygonBuffer.m_Count)
        free(polygonBuffer.m_pPolygon);

    // copy the resulting y value
    if (pR)
        *pR = groundPos.m_Y;

    return result;
}
//---------------------------------------------------------------------------
