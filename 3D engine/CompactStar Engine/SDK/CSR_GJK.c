/****************************************************************************
 * ==> CSR_GJK -------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides an implementation of the              *
 *               Gilbert-Johnson-Keerthi (GJK) algorithm                    *
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

#include "CSR_GJK.h"

// std
#include <stdlib.h>

#define GJK_MAX_NUM_ITERATIONS 64

//---------------------------------------------------------------------------
// Support functions
//---------------------------------------------------------------------------
void csrGJKSupportBox(const CSR_Collider* pCollider, const CSR_Vector3* pDir, CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 invDir = {0};
        CSR_Vector3 result = {0};
    #else
        CSR_Vector3 invDir;
        CSR_Vector3 result;
    #endif

    // find support in model space
    csrMat4Transform(&pCollider->m_InvMatRS, pDir, &invDir);

    if (invDir.m_X > 0.0f)
        result.m_X = pCollider->m_Max.m_X;
    else
        result.m_X = pCollider->m_Min.m_X;

    if (invDir.m_Y > 0.0f)
        result.m_Y = pCollider->m_Max.m_Y;
    else
        result.m_Y = pCollider->m_Min.m_Y;

    if (invDir.m_Z > 0.0f)
        result.m_Z = pCollider->m_Max.m_Z;
    else
        result.m_Z = pCollider->m_Min.m_Z;

    //convert support to world space
    csrMat4Transform(&pCollider->m_MatRS, &result,           pR);
    csrVec3Add      ( pR,                 &pCollider->m_Pos, pR);
}
//---------------------------------------------------------------------------
void csrGJKSupportSphere(const CSR_Collider* pCollider, const CSR_Vector3* pDir, CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 dirN   = {0};
        CSR_Vector3 result = {0};
    #else
        CSR_Vector3 dirN;
        CSR_Vector3 result;
    #endif

    csrVec3Normalize( pDir,   &dirN);
    csrVec3MulVal   (&dirN,    pCollider->m_Radius, &result);
    csrVec3Add      (&result, &pCollider->m_Pos,     pR);
}
//---------------------------------------------------------------------------
void csrGJKSupportCylinder(const CSR_Collider* pCollider, const CSR_Vector3* pDir, CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 invDir = {0};
        CSR_Vector3 dirXZ  = {0};
        CSR_Vector3 dirXZN = {0};
        CSR_Vector3 result = {0};
    #else
        CSR_Vector3 invDir;
        CSR_Vector3 dirXZ;
        CSR_Vector3 dirXZN;
        CSR_Vector3 result;
    #endif

    // find support in model space
    csrMat4Transform(&pCollider->m_InvMatRS, pDir, &invDir);

    // calculate the furthest point on the XZ direction
    dirXZ.m_X = invDir.m_X;
    dirXZ.m_Y = 0.0f;
    dirXZ.m_Z = invDir.m_Z;

    // now calculate the furthest point on the Y direction
    csrVec3Normalize(&dirXZ,  &dirXZN);
    csrVec3MulVal   (&dirXZN,  pCollider->m_Radius, &result);

    if (invDir.m_Y > 0.0f)
        result.m_Y = pCollider->m_TopY;
    else
        result.m_Y = pCollider->m_BottomY;

    // convert support to world space
    csrMat4Transform(&pCollider->m_MatRS, &result,           pR);
    csrVec3Add      ( pR,                 &pCollider->m_Pos, pR);
}
//---------------------------------------------------------------------------
void csrGJKSupportCapsule(const CSR_Collider* pCollider, const CSR_Vector3* pDir, CSR_Vector3* pR)
{
    #ifdef _MSC_VER
        CSR_Vector3 invDir  = {0};
        CSR_Vector3 invDirN = {0};
        CSR_Vector3 result  = {0};
    #else
        CSR_Vector3 invDir;
        CSR_Vector3 invDirN;
        CSR_Vector3 result;
    #endif

    // find support in model space
    csrMat4Transform(&pCollider->m_InvMatRS, pDir, &invDir);

    // now calculate the furthest point on the Y direction
    csrVec3Normalize(&invDir,  &invDirN);
    csrVec3MulVal   (&invDirN,  pCollider->m_Radius, &result);

    if (result.m_Y > 0.0f)
        result.m_Y += pCollider->m_TopY;
    else
        result.m_Y += pCollider->m_BottomY;

    // convert support to world space
    csrMat4Transform(&pCollider->m_MatRS, &result,           pR);
    csrVec3Add      ( pR,                 &pCollider->m_Pos, pR);
}
//---------------------------------------------------------------------------
// Collider functions
//---------------------------------------------------------------------------
CSR_Collider* csrColliderCreate(void)
{
    // create a new collider
    CSR_Collider* pCollider = (CSR_Collider*)malloc(sizeof(CSR_Collider));

    // succeeded?
    if (!pCollider)
        return 0;

    // initialize the collider content
    csrColliderInit(pCollider);

    return pCollider;
}
//---------------------------------------------------------------------------
void csrColliderRelease(CSR_Collider* pCollider)
{
    // no collider to release?
    if (!pCollider)
        return;

    // free the collider
    free(pCollider);
}
//---------------------------------------------------------------------------
void csrColliderInit(CSR_Collider* pCollider)
{
    // no collider to initialize?
    if (!pCollider)
        return;

    // initialize the collider
    pCollider->m_State      = CSR_CS_Dynamic;
    pCollider->m_Pos.m_X    = 0.0f;
    pCollider->m_Pos.m_Y    = 0.0f;
    pCollider->m_Pos.m_Z    = 0.0f;
    pCollider->m_Min.m_X    = 0.0f;
    pCollider->m_Min.m_Y    = 0.0f;
    pCollider->m_Min.m_Z    = 0.0f;
    pCollider->m_Max.m_X    = 0.0f;
    pCollider->m_Max.m_Y    = 0.0f;
    pCollider->m_Max.m_Z    = 0.0f;
    pCollider->m_TopY       = 0.0f;
    pCollider->m_BottomY    = 0.0f;
    pCollider->m_Radius     = 0.0f;
    pCollider->m_fOnSupport = 0;

    // initialize the matrices
    csrMat4Identity(&pCollider->m_MatRS);
    csrMat4Identity(&pCollider->m_InvMatRS);
}
//---------------------------------------------------------------------------
void csrColliderSetPos(const CSR_Vector3* pPos, CSR_Collider* pCollider)
{
    // no collider to update?
    if (!pCollider)
        return;

    // no position?
    if (!pPos)
        return;

    pCollider->m_Pos = *pPos;
}
//---------------------------------------------------------------------------
void csrColliderSetRS(const CSR_Matrix4* pMat, CSR_Collider* pCollider)
{
    float determinant = 0.0f;

    // no collider to update?
    if (!pCollider)
        return;

    // no matrix?
    if (!pMat)
        return;

    pCollider->m_MatRS = *pMat;

    // remove any position data from matrix
    pCollider->m_MatRS.m_Table[3][0] = 0.0f;
    pCollider->m_MatRS.m_Table[3][1] = 0.0f;
    pCollider->m_MatRS.m_Table[3][2] = 0.0f;

    // calculate the inverted matrix (ready to use for collisions)
    csrMat4Inverse(&pCollider->m_MatRS, &pCollider->m_InvMatRS, &determinant);
}
//---------------------------------------------------------------------------
// GJK private functions
//---------------------------------------------------------------------------
void csrUpdateSimplex3(CSR_Vector3* pA,
                       CSR_Vector3* pB,
                       CSR_Vector3* pC,
                       CSR_Vector3* pD,
                       size_t*      pSimpDim,
                       CSR_Vector3* pSearchDir)
{
    #ifdef _MSC_VER
        CSR_Vector3 n      = {0};
        CSR_Vector3 ba     = {0};
        CSR_Vector3 ca     = {0};
        CSR_Vector3 aO     = {0};
        CSR_Vector3 baN    = {0};
        CSR_Vector3 nCA    = {0};
    #else
        CSR_Vector3 n;
        CSR_Vector3 ba;
        CSR_Vector3 ca;
        CSR_Vector3 aO;
        CSR_Vector3 baN;
        CSR_Vector3 nCA;
    #endif
    float           dotRes = 0.0f;

    // below is the required winding order
    //
    //  b
    //  |\
    //  | \
    //  |  \ a
    //  |  /
    //  | /
    //  |/
    //  c

    // triangle normal
    csrVec3Sub  ( pB,  pA, &ba);
    csrVec3Sub  ( pC,  pA, &ca);
    csrVec3Cross(&ba, &ca, &n);

    // direction to origin
    csrVec3Inverse(pA, &aO);

    // determine which feature is closest to origin, make that the new simplex
    *pSimpDim = 2;

    csrVec3Cross(&ba,  &n,  &baN);
    csrVec3Dot  (&baN, &aO, &dotRes);

    // closest to edge AB
    if (dotRes > 0.0f)
    {
        #ifdef _MSC_VER
            CSR_Vector3 baO = {0};
        #else
            CSR_Vector3 baO;
        #endif

        *pC = *pA;

        csrVec3Cross(&ba,  &aO, &baO);
        csrVec3Cross(&baO, &ba, pSearchDir);
        return;
    }

    csrVec3Cross(&n,   &ca, &nCA);
    csrVec3Dot  (&nCA, &aO, &dotRes);

    // closest to edge AC
    if (dotRes > 0.0f)
    {
        #ifdef _MSC_VER
            CSR_Vector3 caO = {0};
        #else
            CSR_Vector3 caO;
        #endif

        *pB = *pA;

        csrVec3Cross(&ca,  &aO, &caO);
        csrVec3Cross(&caO, &ca, pSearchDir);
        return;
    }

    *pSimpDim = 3;

    csrVec3Dot(&n, &aO, &dotRes);

    // above triangle
    if (dotRes > 0.0f)
    {
        *pD         = *pC;
        *pC         = *pB;
        *pB         = *pA;
        *pSearchDir = n;
        return;
    }

    // below triangle
    *pD = *pB;
    *pB = *pA;
    csrVec3Inverse(&n, pSearchDir);
}
//---------------------------------------------------------------------------
int csrUpdateSimplex4(CSR_Vector3* pA,
                      CSR_Vector3* pB,
                      CSR_Vector3* pC,
                      CSR_Vector3* pD,
                      size_t*      pSimpDim,
                      CSR_Vector3* pSearchDir)
{
    #ifdef _MSC_VER
        CSR_Vector3 ba     = {0};
        CSR_Vector3 ca     = {0};
        CSR_Vector3 da     = {0};
        CSR_Vector3 abc    = {0};
        CSR_Vector3 acd    = {0};
        CSR_Vector3 adb    = {0};
        CSR_Vector3 aO     = {0};
    #else
        CSR_Vector3 ba;
        CSR_Vector3 ca;
        CSR_Vector3 da;
        CSR_Vector3 abc;
        CSR_Vector3 acd;
        CSR_Vector3 adb;
        CSR_Vector3 aO;
    #endif
    float           dotRes = 0.0f;

    // a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
    // we know a priori that origin is above BCD and below a
    csrVec3Sub(pB, pA, &ba);
    csrVec3Sub(pC, pA, &ca);
    csrVec3Sub(pD, pA, &da);

    // get new faces normals
    csrVec3Cross(&ba, &ca, &abc);
    csrVec3Cross(&ca, &da, &acd);
    csrVec3Cross(&da, &ba, &adb);

    // dir to origin
    csrVec3Inverse(pA, &aO);

    // hoisting this just cause
    *pSimpDim = 3;

    // plane-test origin with 3 faces. NOTE kind of primitive approach used here. If origin is
    // in front of a face, just use it as the new simplex. Otherwise iterate through the faces
    // sequentially and exit at the first one which satisfies dot product. Perhaps this isn't
    // optimal or edges should be considered as possible simplexes,, but it seems that this
    // method is good enough. Makes no difference for AABBS, should test with more complex
    // colliders
    csrVec3Dot(&abc, &aO, &dotRes);

    // in front of ABC
    if (dotRes > 0.0f)
    {
        *pD         = *pC;
        *pC         = *pB;
        *pB         = *pA;
        *pSearchDir = abc;

        return 0;
    }

    csrVec3Dot(&acd, &aO, &dotRes);

    // in front of ACD
    if (dotRes > 0.0f)
    {
        *pB         = *pA;
        *pSearchDir = acd;

        return 0;
    }

    csrVec3Dot(&adb, &aO, &dotRes);

    // in front of ADB
    if (dotRes > 0.0f)
    {
        *pC         = *pD;
        *pD         = *pB;
        *pB         = *pA;
        *pSearchDir = adb;

        return 0;
    }

    // inside tetrahedron, enclosed! NOTE in case where two faces have similar normals,
    // the origin could eventually be close to an edge of the tetrahedron. It would
    // probably not make a difference to limit new simplexes to just one of the faces,
    // maybe test it later
    return 1;
}
//---------------------------------------------------------------------------
// GJK functions
//---------------------------------------------------------------------------
int csrGJKResolve(const CSR_Collider* pC1, const CSR_Collider* pC2)
{
    #ifdef _MSC_VER
        CSR_Vector3 searchDir    = {0};
        CSR_Vector3 invSearchDir = {0};
        CSR_Vector3 invSimplex1  = {0};
        CSR_Vector3 simplex21    = {0};
        CSR_Vector3 simplex211   = {0};
        CSR_Vector3 c1Support    = {0};
        CSR_Vector3 c2Support    = {0};
    #else
        CSR_Vector3 searchDir;
        CSR_Vector3 invSearchDir;
        CSR_Vector3 invSimplex1;
        CSR_Vector3 simplex21;
        CSR_Vector3 simplex211;
        CSR_Vector3 c1Support;
        CSR_Vector3 c2Support;
    #endif
    size_t          simpDim      = 0;
    size_t          iterations   = 0;
    float           dotRes       = 0.0f;

    // simplex, it's just a set of points (a is always most recently added)
    CSR_Vector3 simplex[4] = {0};

    // initial search direction between colliders
    csrVec3Sub(&pC1->m_Pos, &pC2->m_Pos, &searchDir);
    csrVec3Inverse(&searchDir, &invSearchDir);

    // get initial point for simplex
    pC1->m_fOnSupport(pC1, &invSearchDir, &c1Support);
    pC2->m_fOnSupport(pC2, &searchDir,    &c2Support);
    csrVec3Sub(&c2Support, &c1Support, &simplex[2]);

    // search in direction of origin
    csrVec3Inverse(&simplex[2], &searchDir);
    csrVec3Inverse(&searchDir,  &invSearchDir);

    // get second point for a line segment simplex
    pC1->m_fOnSupport(pC1, &invSearchDir, &c1Support);
    pC2->m_fOnSupport(pC2, &searchDir,    &c2Support);
    csrVec3Sub(&c2Support, &c1Support, &simplex[1]);

    csrVec3Dot(&simplex[1], &searchDir, &dotRes);

    // didn't reach the origin, won't enclose it
    if (dotRes < 0.0f)
        return 0;

    // search perpendicular to line segment towards origin
    csrVec3Inverse(&simplex[1], &invSimplex1);
    csrVec3Sub    (&simplex[2], &simplex[1],  &simplex21);
    csrVec3Cross  (&simplex21,  &invSimplex1, &simplex211);
    csrVec3Cross  (&simplex211, &simplex21,   &searchDir);

    // origin is on this line segment
    if (searchDir.m_X == 0.0f && searchDir.m_Y == 0.0f && searchDir.m_Z == 0.0f)
    {
        #ifdef _MSC_VER
            CSR_Vector3 upDir = {0};
        #else
            CSR_Vector3 upDir;
        #endif

        upDir.m_X = 1.0f;
        upDir.m_Y = 0.0f;
        upDir.m_Z = 0.0f;

        // apparently any normal search vector will do? Perform the normal with x-axis
        csrVec3Cross(&simplex21, &upDir, &searchDir);

        // normal with z-axis
        if (searchDir.m_X == 0.0f && searchDir.m_Y == 0.0f && searchDir.m_Z == 0.0f)
        {
            #ifdef _MSC_VER
                CSR_Vector3 downDir = {0};
            #else
                CSR_Vector3 downDir;
            #endif

            downDir.m_X =  0.0f;
            downDir.m_Y =  0.0f;
            downDir.m_Z = -1.0f;

            csrVec3Cross(&simplex21, &downDir, &searchDir);
        }
    }

    // simplex dimension
    simpDim = 2;

    for (iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; ++iterations)
    {
        csrVec3Inverse(&searchDir, &invSearchDir);
        pC1->m_fOnSupport(pC1, &invSearchDir, &c1Support);
        pC2->m_fOnSupport(pC2, &searchDir,    &c2Support);
        csrVec3Sub(&c2Support, &c1Support, &simplex[0]);

        csrVec3Dot(&simplex[0], &searchDir, &dotRes);

        // didn't reach the origin, won't enclose it
        if (dotRes < 0.0f)
            return 0;

        ++simpDim;

        if (simpDim == 3)
            csrUpdateSimplex3(&simplex[0], &simplex[1], &simplex[2], &simplex[3], &simpDim, &searchDir);
        else
        if (csrUpdateSimplex4(&simplex[0], &simplex[1], &simplex[2], &simplex[3], &simpDim, &searchDir))
            return 1;
    }

    return 0;
}
//---------------------------------------------------------------------------
