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

// compactStart engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Vertex.h"

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

// Aligned-axis bounding box tree node prototype
typedef struct CSR_AABBNode CSR_AABBNode;

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
        *@note The AABB tree must be released when no longer used, see csrAABBTreeRelease()
        */
        CSR_AABBNode* csrAABBTreeFromMesh(const CSR_Mesh* pMesh);

        /**
        * Resolves AABB tree
        *@param pRay - ray against which tree items will be tested
        *@param pNode - root or parent node to resolve
        *@param[out] pPolygons - polygons belonging to boxes hit by ray
        *@param deep - tree deep level, used internally, should be set to 0
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
        void csrAABBTreeNodeRelease(CSR_AABBNode* pNode);

        /**
        * Releases an AABB tree
        *@param[in, out] pNode - AABB tree root node to release from
        */
        void csrAABBTreeRelease(CSR_AABBNode* pNode);

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
