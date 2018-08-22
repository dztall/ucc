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

#ifndef CSR_CollisionH
#define CSR_CollisionH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

// Aligned-axis bounding box tree node prototype
typedef struct CSR_AABBNode CSR_AABBNode;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Aligned-axis bounding box tree node
*/
struct CSR_AABBNode
{
    CSR_AABBNode*             m_pParent;
    CSR_AABBNode*             m_pLeft;
    CSR_AABBNode*             m_pRight;
    CSR_Box*                  m_pBox;
    CSR_IndexedPolygonBuffer* m_pPolygonBuffer;
};

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Aligned-Axis Bounding Box tree functions
        //-------------------------------------------------------------------

        /**
        * Populates an AABB tree from an indexed polygon buffer
        *@param pIPB - indexed polygon buffer to use to populate the tree
        *@param[in, out] pNode - root or parent node to create from, populated node on function ends
        *@return 1 on success, otherwise 0
        */
        int csrAABBTreeFromIndexedPolygonBuffer(const CSR_IndexedPolygonBuffer* pIPB,
                                                      CSR_AABBNode*             pNode);

        /**
        * Gets an AABB tree from a mesh
        *@param pMesh - mesh
        *@return aligned-axis bounding box tree root node, 0 on error
        *@note The AABB tree must be released when no longer used, see csrAABBTreeNodeRelease()
        */
        CSR_AABBNode* csrAABBTreeFromMesh(const CSR_Mesh* pMesh);

        /**
        * Resolves AABB tree
        *@param pRay - ray against which tree items will be tested
        *@param pNode - root or parent node to resolve
        *@param deep - tree deep level, used internally, should be set to 0
        *@param[out] pPolygons - polygons belonging to boxes hit by ray
        *@return 1 on success, otherwise 0
        */
        int csrAABBTreeResolve(const CSR_Ray3*           pRay,
                               const CSR_AABBNode*       pNode,
                                     size_t              deep,
                                     CSR_Polygon3Buffer* pPolygons);

        /**
        * Releases an AABB tree node content
        *@param[in, out] pNode - node for which content should be released
        *@note Only the node content is released, the node itself is not released
        */
        void csrAABBTreeNodeContentRelease(CSR_AABBNode* pNode);

        /**
        * Releases an AABB tree node and all his children
        *@param[in, out] pNode - AABB tree root node to release from
        */
        void csrAABBTreeNodeRelease(CSR_AABBNode* pNode);

        //-------------------------------------------------------------------
        // Sliding functions
        //-------------------------------------------------------------------

        /**
        * Gets the next position while sliding on a plane
        *@param pSlidingPlane - sliding plane
        *@param pPosition - current position
        *@param radius - radius around the current position
        *@param pR - resulting position
        */
        void csrSlidingPoint(const CSR_Plane*   pSlidingPlane,
                             const CSR_Vector3* pPosition,
                                   float        radius,
                                   CSR_Vector3* pR);

        //-------------------------------------------------------------------
        // Ground collision functions
        //-------------------------------------------------------------------

        /**
        * Calculates the position where a model or a point of view is placed on the ground
        *@param pPolygon - polygon belonging to the model showing the ground of a scene
        *@param pSphere - bounding sphere surrounding the point of view or model
        *@param pGroundDir - ground direction. If 0, a default direction of [0, -1, 0] will be used
        *@param[in, out] pR - resulting position where the bounding sphere surrounding the point of
        *                     view or model will be placed on the ground. Ignored if 0
        *@return 1 if the bounding sphere is above the ground polygon, otherwise 0
        *@note The bounding sphere should be in the same coordinate system as the polygon. This
        *      means that any transformation should be applied to the sphere before calling this
        *      function
        */
        int csrGroundCollision(const CSR_Sphere*   pSphere,
                               const CSR_Polygon3* pPolygon,
                               const CSR_Vector3*  pGroundDir,
                                     CSR_Vector3*  pR);

        /**
        * Calculates the y axis position where to place the point of view to stay above the ground
        *@param pBoundingSphere - sphere surrounding the point of view or model
        *@param pTree - ground model aligned-axis bounding box tree
        *@param pGroundDir - ground direction
        *@param[out] pGroundPolygon - polygon on which the ground was hit, ignored if 0
        *@param[out] pR - resulting position on the y axis where to place the point of view or model
        *@return 1 if a ground polygon was found, otherwise 0
        *@note The bounding sphere should be in the same coordinate system as the model. This means
        *      that any transformation should be applied to the sphere before calling this function
        */
        int csrGroundPosY(const CSR_Sphere*   pBoundingSphere,
                          const CSR_AABBNode* pTree,
                          const CSR_Vector3*  pGroundDir,
                                CSR_Polygon3* pGroundPolygon,
                                float*        pR);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Collision.c"
#endif

#endif
