/*****************************************************************************
 * ==> Minimal collision detection library ----------------------------------*
 *****************************************************************************
 * Description : Minimal collision detection library                         *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015, this file is part of the Minimal API. You are free    *
 *               to copy or redistribute this file, modify it, or use it for *
 *               your own projects, commercial or not                        *
 *****************************************************************************/

#ifndef MiniCollisionH
#define MiniCollisionH

// std
#include <stdlib.h>
#include <math.h>

// mini API
#include "MiniGeometry.h"

#define M_MC_Epsilon 1.0E-3 // epsilon value used for tolerance

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

/**
* Polygon
*/
typedef struct
{
    MG_Vector3 m_v[3];
} MC_Polygon;

/// Aligned-axis bounding box tree node prototype
typedef struct MC_AABBNode MC_AABBNode;

/**
* Aligned-axis bounding box tree node
*/
struct MC_AABBNode
{
    MC_AABBNode* m_pParent;
    MC_AABBNode* m_pLeft;
    MC_AABBNode* m_pRight;
    MG_Box*      m_pBox;
    MC_Polygon*  m_pPolygons;
    unsigned     m_PolygonsCount;
};

//-----------------------------------------------------------------------------
// Math functions
//-----------------------------------------------------------------------------

/**
* Gets minimum value between 2 values
*@param pA - first value to compare
*@param pB - second value to compare
*@return minimum value
*/
void Min(const float* pA, const float* pB, float* pR)
{
    if (*pA < *pB)
    {
        *pR = *pA;
        return;
    }

    *pR = *pB;
}

/**
* Gets maximum value between 2 values
*@param pA - first value to compare
*@param pB - second value to compare
*@return maximum value
*/
void Max(const float* pA, const float* pB, float* pR)

{
    if (*pA > *pB)
    {
        *pR = *pA;
        return;
    }

    *pR = *pB;
}

//-----------------------------------------------------------------------------
// Distances calculation
//-----------------------------------------------------------------------------

/**
* Checks if a value is between 2 values
*@param pV - value to test
*@param pS - start value
*@param pE - end value
*@param pEpsylon - epsylon value for tolerance
*@return 1 if value is between values, otherwise 0
*/
int ValueIsBetween(const float* pV, const float* pS, const float* pE, const float* pEpsylon)
{
    float minVal;
    float maxVal;

    Min(pS, pE, &minVal);
    Max(pS, pE, &maxVal);

    // check if each value is between start and end limits considering tolerance
    if (*pV >= (minVal - *pEpsylon) && *pV <= (maxVal + *pEpsylon))
        return 1;

    return 0;
}

/**
* Checks if a point is between 2 vertices
*@param pP - point to test
*@param pS - start vertex
*@param pE - End vertex
*@param pEpsylon - epsylon value for tolerance
*@return 1 if value is between points, otherwise 0
*/
int VectorIsBetween(const MG_Vector3* pP,
                    const MG_Vector3* pS,
                    const MG_Vector3* pE,
                    const float*      pEpsylon)
{
    // check if each vector component is between start and end limits
    if (!ValueIsBetween(&pP->m_X, &pS->m_X, &pE->m_X, pEpsylon))
        return 0;

    if (!ValueIsBetween(&pP->m_Y, &pS->m_Y, &pE->m_Y, pEpsylon))
        return 0;

    if (!ValueIsBetween(&pP->m_Z, &pS->m_Z, &pE->m_Z, pEpsylon))
        return 0;

    return 1;
}

/**
* Gets the shortest distance between 2 lines
*@param pL1S - first line start point
*@param pL1E - first line end point
*@param pL2S - second line start point
*@param pL2E - second line end point
*@param[out] pR - resulting distance
*/
void GetShortestDistance(const MG_Vector3* pL1S,
                         const MG_Vector3* pL1E,
                         const MG_Vector3* pL2S,
                         const MG_Vector3* pL2E,
                         const float*      pEpsylon,
                               float*      pR)
{
    MG_Vector3 delta21;
    MG_Vector3 delta43;
    MG_Vector3 delta13;
    MG_Vector3 dP;
    float      a;
    float      b;
    float      c;
    float      d;
    float      e;
    float      D;
    float      sc;
    float      sN;
    float      sD;
    float      tc;
    float      tN;
    float      tD;
    float      dotdP;

    // calculate p2 - p1, p4 - p3, and p1 - p3 line segments
    Sub(pL1E, pL1S, &delta21);
    Sub(pL2E, pL2S, &delta43);
    Sub(pL1S, pL2S, &delta13);

    // calculate some needed values - a, c and D are always >= 0
    Dot(&delta21, &delta21, &a);
    Dot(&delta21, &delta43, &b);
    Dot(&delta43, &delta43, &c);
    Dot(&delta21, &delta13, &d);
    Dot(&delta43, &delta13, &e);
    D = ((a * c) - (b * b));

    // sc = sN / sD, default sD = D >= 0
    sD = D;

    // tc = tN / tD, default tD = D >= 0
    tD = D;

    // compute the line parameters of the two closest points
    if (D < *pEpsylon)
    {
        // the lines are almost parallel, force using point P0 on segment S1
        // to prevent possible division by 0 later
        sN = 0.0;
        sD = 1.0;
        tN = e;
        tD = c;
    }
    else
    {
        // get the closest points on the infinite lines
        sN = ((b * e) - (c * d));
        tN = ((a * e) - (b * d));

        // sc < 0 => the s=0 edge is visible
        if (sN < 0.0)
        {
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else
        // sc > 1 => the s=1 edge is visible
        if (sN > sD)
        {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    // tc < 0 => the t=0 edge is visible
    if (tN < 0.0)
    {
        tN = 0.0;

        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else
        if (-d > a)
            sN = sD;
        else
        {
            sN = -d;
            sD =  a;
        }
    }
    else
    // tc > 1 => the t=1 edge is visible
    if (tN > tD)
    {
        tN = tD;

        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else
        if ((-d + b) > a)
            sN = sD;
        else
        {
            sN = (-d + b);
            sD = a;
        }
    }

    // finally do the division to get sc and tc
    if (fabs(sN) < *pEpsylon)
        sc = 0.0;
    else
        sc = sN / sD;

    if (fabs(tN) < *pEpsylon)
        tc = 0.0;
    else
        tc = tN / tD;

    // get the difference of the two closest points
    dP.m_X = delta13.m_X + (sc * delta21.m_X) - (tc * delta43.m_X);
    dP.m_Y = delta13.m_Y + (sc * delta21.m_Y) - (tc * delta43.m_Y);
    dP.m_Z = delta13.m_Z + (sc * delta21.m_Z) - (tc * delta43.m_Z);

    // return the closest distance
    Dot(&dP, &dP, &dotdP);
    *pR = sqrt(dotdP);
}

/**
* Calculates a distance between a point and a plane
*@param pP - point
*@param pPl - plane
*@param[out] - resulting distance
*/
void DistanceToPlane(const MG_Vector3* pP, const MG_Plane* pPl, float* pR)
{
    MG_Vector3 n;
    float      dist;

    // get the normal of the plane
    n.m_X = pPl->m_A;
    n.m_Y = pPl->m_B;
    n.m_Z = pPl->m_C;

    // calculate the distance between the plane and the point
    Dot(&n, pP, &dist);
    *pR = dist + pPl->m_D;
}

//-----------------------------------------------------------------------------
// Box calculation
//-----------------------------------------------------------------------------

/**
* Adds a polygon inside an existing bounding box
*@param pPolygon - polygon to add
*@param pBox - bounding box in which polygon should be added
*@param empty - if true, box is empty an still no contains any polygon
*/
void AddPolygonToBoundingBox(const MC_Polygon* pPolygon,
                                   MG_Box*     pBox,
                                   int*        pEmpty)
{
    float    r;
    unsigned i;

    // iterate through polygon vertices
    for (i = 0; i < 3; ++i)
    {
        // is box empty?
        if (*pEmpty)
        {
            // initialize bounding box with first vertex
            Copy(&pPolygon->m_v[i], &pBox->m_Min);
            Copy(&pPolygon->m_v[i], &pBox->m_Max);
            *pEmpty = 0;
            continue;
        }

        // search for box min edge
        Min(&pBox->m_Min.m_X, &pPolygon->m_v[i].m_X, &r);
        pBox->m_Min.m_X = r;
        Min(&pBox->m_Min.m_Y, &pPolygon->m_v[i].m_Y, &r);
        pBox->m_Min.m_Y = r;
        Min(&pBox->m_Min.m_Z, &pPolygon->m_v[i].m_Z, &r);
        pBox->m_Min.m_Z = r;

        // search for box max edge
        Max(&pBox->m_Max.m_X, &pPolygon->m_v[i].m_X, &r);
        pBox->m_Max.m_X = r;
        Max(&pBox->m_Max.m_Y, &pPolygon->m_v[i].m_Y, &r);
        pBox->m_Max.m_Y = r;
        Max(&pBox->m_Max.m_Z, &pPolygon->m_v[i].m_Z, &r);
        pBox->m_Max.m_Z = r;
    }
}

//-----------------------------------------------------------------------------
// Collision detection
//-----------------------------------------------------------------------------

/**
* Checks if a point is inside a polygon
*@param pP - point to check
*@param pV1 - first polygon vector
*@param pV2 - second polygon vector
*@param pV3 - third polygon vector
*@return 1 if point is inside polygon, otherwise 0
*/
int Inside(const MG_Vector3* pP,
           const MG_Vector3* pV1,
           const MG_Vector3* pV2,
           const MG_Vector3* pV3)
{
    MG_Vector3 nPToV1;
    MG_Vector3 nPToV2;
    MG_Vector3 nPToV3;
    float      a1;
    float      a2;
    float      a3;
    float      angleResult;

    /*
    * check if the point p is inside the polygon in the following manner:
    *
    *                  V1                         V1
    *                  /\                         /\
    *                 /  \                       /  \
    *                / *p \                  *P /    \
    *               /      \                   /      \
    *            V2 -------- V3             V2 -------- V3
    *
    * calculate the vectors between the point p and each polygon vertex, then
    * calculate the angle formed by each of these vectors. If the sum of the
    * angles is equal to a complete circle, i.e. 2 * pi in radians, then the
    * point p is inside the polygon limits, otherwise the point is outside. It
    * is assumed that the point to check belongs to the polygon's plane
    */
    Sub(pV1, pP, &nPToV1);
    Sub(pV2, pP, &nPToV2);
    Sub(pV3, pP, &nPToV3);
    Normalize(&nPToV1, &nPToV1);
    Normalize(&nPToV2, &nPToV2);
    Normalize(&nPToV3, &nPToV3);

    // calculate the angles using the scalar product of each vectors. We use
    // the following algorithms:
    // A1 = NPToV1.x * NPToV2.x + NPToV1.y * NPToV2.y + NPToV1.z * NPToV2.z
    // A2 = NPToV2.x * NPToV3.x + NPToV2.y * NPToV3.y + NPToV2.z * NPToV3.z
    // A3 = NPToV3.x * NPToV1.x + NPToV3.y * NPToV1.y + NPToV3.z * NPToV1.z
    Dot(&nPToV1, &nPToV2, &a1);
    Dot(&nPToV2, &nPToV3, &a2);
    Dot(&nPToV3, &nPToV1, &a3);

    // limit a1 to avoid rounding errors
    if (a1 > 1.0f)
        a1 = 1.0f;
    else
    if (a1 < -1.0f)
        a1 = -1.0f;

    // limit a2 to avoid rounding errors
    if (a2 > 1.0f)
        a2 = 1.0f;
    else
    if (a2 < -1.0f)
        a2 = -1.0f;

    // limit a3 to avoid rounding errors
    if (a3 > 1.0f)
        a3 = 1.0f;
    else
    if (a3 < -1.0f)
        a3 = -1.0f;

    // calculate the sum of all angles
    angleResult = acos(a1) + acos(a2) + acos(a3);

    // if sum is equal or higher to 6.28 radians then point P is inside polygon
    if (angleResult >= 6.28f)
        return 1;

    return 0;
}

/**
* Checks if a circle intersects a rectangle
*@param pCircle - circle to check
*@param pRect - rectangle to check against
*@return 1 if shapes intersect, otherwise 0
*/
int CircleRectIntersects(MG_Circle* pCircle, MG_Rect* pRect)
{
    MG_Point circleDistance;
    float    cornerDistanceSq;

    // calculate distance between circle and rect
    circleDistance.m_X = fabs(pCircle->m_Pos.m_X - pRect->m_Pos.m_X);
    circleDistance.m_Y = fabs(pCircle->m_Pos.m_Y - pRect->m_Pos.m_Y);

    // is distance too high to be collided?
    if (circleDistance.m_X > (pRect->m_Size.m_Width / 2.0f + pCircle->m_Radius))
        return 0;
    else
    if (circleDistance.m_Y > (pRect->m_Size.m_Height / 2.0f + pCircle->m_Radius))
        return 0;

    // is distance too small to be not collided?
    if (circleDistance.m_X <= (pRect->m_Size.m_Width / 2.0f))
        return 1;
    else
    if (circleDistance.m_Y <= (pRect->m_Size.m_Height / 2.0f))
        return 1;

    // calculate distance between circle and nearest rect corner
    cornerDistanceSq = pow((circleDistance.m_X - pRect->m_Size.m_Width  / 2.0f), 2.0f) +
                       pow((circleDistance.m_Y - pRect->m_Size.m_Height / 2.0f), 2.0f);

    // is distance between circle and nearest rect corner too small?
    if (cornerDistanceSq <= pow(pCircle->m_Radius, 2.0f))
        return 1;

    return 0;
}

/**
* Checks if a plane intesects a ray
*@param pPl - plane
*@param pPos -ray position
*@param pDir - ray direction
*@param[out] pR - resulting point where plane intersects line
*@return 1 if plane intersects line, otherwise 0
*/
int RayPlaneIntersect(const MG_Plane*   pPl,
                      const MG_Vector3* pPos,
                      const MG_Vector3* pDir,
                            MG_Vector3* pP)
{
    MG_Vector3 n;
    float      dot;
    float      nDot;
    float      temp;

    // get the normal of the plane
    n.m_X = pPl->m_A;
    n.m_Y = pPl->m_B;
    n.m_Z = pPl->m_C;

    // calculate the angle between the line and the normal to the plane
    Dot(&n, pDir, &dot);

    // if normal to the plane is perpendicular to the line, then the line is
    // either parallel to the plane and there are no solutions or the line is
    // on the plane in which case there are an infinite number of solutions
    if (!dot)
        return 0;

    Dot(&n, pPos, &nDot);

    temp = ((pPl->m_D + nDot) / dot);

    // calculates the intersection point
    pP->m_X = (pPos->m_X - (temp * pDir->m_X));
    pP->m_Y = (pPos->m_Y - (temp * pDir->m_Y));
    pP->m_Z = (pPos->m_Z - (temp * pDir->m_Z));

    return 1;
}

/**
* Checks if a line intesects a plane
*@param pPl - plane
*@param pP1 - first point
*@param pP2 - second point
*@param[out] pR - resulting point where plane intersects line
*@return 1 if plane intersects line, otherwise 0
*/
int LinePlaneIntersect(const MG_Plane*   pPl,
                       const MG_Vector3* pP1,
                       const MG_Vector3* pP2,
                             MG_Vector3* pP)
{
    MG_Vector3 dir;
    MG_Vector3 nDir;

    // calculates the direction of the line
    Sub(pP2, pP1, &dir);
    Normalize(&dir, &nDir);

    return RayPlaneIntersect(pPl, pP1, &nDir, pP);
}

/**
* Checks if a polygons intersects another polygon
*@param pP1V1 - first polygon first vertex
*@param pP1V2 - first polygon second vertex
*@param pP1V3 - first polygon third vertex
*@param pP2V1 - second polygon first vertex
*@param pP2V2 - second polygon second vertex
*@param pP2V3 - second polygon third vertex
*@return 1 if polygons intersect, otherwise 0
*/
int PolygonsIntersect(MG_Vector3* pP1V1, MG_Vector3* pP1V2, MG_Vector3* pP1V3,
                      MG_Vector3* pP2V1, MG_Vector3* pP2V2, MG_Vector3* pP2V3)

{
    MG_Plane    plane1;
    MG_Plane    plane2;
    MG_Vector3  v1v2LineS;
    MG_Vector3  v1v2LineE;
    MG_Vector3  v2v3LineS;
    MG_Vector3  v2v3LineE;
    MG_Vector3  v3v1LineS;
    MG_Vector3  v3v1LineE;
    MG_Vector3  ov1ov2LineS;
    MG_Vector3  ov1ov2LineE;
    MG_Vector3  ov2ov3LineS;
    MG_Vector3  ov2ov3LineE;
    MG_Vector3  ov3ov1LineS;
    MG_Vector3  ov3ov1LineE;
    MG_Vector3 normal1;
    MG_Vector3 normal2;
    MG_Vector3 p1pt1;
    MG_Vector3 p1pt2;
    MG_Vector3 p1pt3;
    MG_Vector3 p2pt1;
    MG_Vector3 p2pt2;
    MG_Vector3 p2pt3;
    MG_Vector3 p;
    int        p1ptsCount;
    int        p2ptsCount;
    float      planesDot;
    float      dist1v1;
    float      dist1v2;
    float      dist1v3;
    float      dist2v1;
    float      dist2v2;
    float      dist2v3;
    float      result;
    float      epsylon = 1.0E-3;

    // get planes from polygons
    PlaneFromPoints(pP1V1, pP1V2, pP1V3, &plane1);
    PlaneFromPoints(pP2V1, pP2V2, pP2V3, &plane2);

    // are planes merged?
    if (((plane1.m_A ==  plane2.m_A)  &&
         (plane1.m_B ==  plane2.m_B)  &&
         (plane1.m_C ==  plane2.m_C)  &&
         (plane1.m_D ==  plane2.m_D)) ||
        ((plane1.m_A == -plane2.m_A)  &&
         (plane1.m_B == -plane2.m_B)  &&
         (plane1.m_C == -plane2.m_C)  &&
         (plane1.m_D == -plane2.m_D)))
    {
        // is any vertex inside other polygon?
        if (Inside(pP1V1, pP2V1, pP2V2, pP2V3) ||
            Inside(pP1V2, pP2V1, pP2V2, pP2V3) ||
            Inside(pP1V3, pP2V1, pP2V2, pP2V3) ||
            Inside(pP2V1, pP1V1, pP1V2, pP1V3) ||
            Inside(pP2V2, pP1V1, pP1V2, pP1V3) ||
            Inside(pP2V3, pP1V1, pP1V2, pP1V3))
            return 1;

        // create polygon lines
        Copy(pP1V1, &v1v2LineS);
        Copy(pP1V2, &v1v2LineE);
        Copy(pP1V2, &v2v3LineS);
        Copy(pP1V3, &v2v3LineE);
        Copy(pP1V3, &v3v1LineS);
        Copy(pP1V1, &v3v1LineE);
        Copy(pP2V1, &ov1ov2LineS);
        Copy(pP2V2, &ov1ov2LineE);
        Copy(pP2V2, &ov2ov3LineS);
        Copy(pP2V3, &ov2ov3LineE);
        Copy(pP2V3, &ov3ov1LineS);
        Copy(pP2V1, &ov3ov1LineE);

        GetShortestDistance(&v1v2LineS,   &v1v2LineE,
                            &ov1ov2LineS, &ov1ov2LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v2v3LineS,   &v2v3LineE,
                            &ov1ov2LineS, &ov1ov2LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v3v1LineS,   &v3v1LineE,
                            &ov1ov2LineS, &ov1ov2LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v1v2LineS,   &v1v2LineE,
                            &ov2ov3LineS, &ov2ov3LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v2v3LineS,   &v2v3LineE,
                            &ov2ov3LineS, &ov2ov3LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v3v1LineS,   &v3v1LineE,
                            &ov2ov3LineS, &ov2ov3LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v1v2LineS,   &v1v2LineE,
                            &ov3ov1LineS, &ov3ov1LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v2v3LineS,   &v2v3LineE,
                            &ov3ov1LineS, &ov3ov1LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v3v1LineS,   &v3v1LineE,
                            &ov3ov1LineS, &ov3ov1LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        return 0;
    }

    // get plane normals
    normal1.m_X = plane1.m_A;
    normal1.m_Y = plane1.m_B;
    normal1.m_Z = plane1.m_C;

    normal2.m_X = plane2.m_A;
    normal2.m_Y = plane2.m_B;
    normal2.m_Z = plane2.m_C;

    // calculate angle between planes
    Dot(&normal1, &normal2, &planesDot);

    // are plane parallels?
    if (planesDot == 1.0f || planesDot == -1.0f)
        // planes are parallels but not merged, no collision is possible
        return 0;

    // calculate distance from each first polygon vertex to second polygon plane
    DistanceToPlane(pP1V1, &plane2, &dist1v1);
    DistanceToPlane(pP1V2, &plane2, &dist1v2);
    DistanceToPlane(pP1V3, &plane2, &dist1v3);

    // prepare list containing first polygon intersection points
    p1ptsCount = 0;

    // is first polygon V1 to V2 line segment intersects second polygon plane?
    if ((dist1v1 >= 0.0f && dist1v2 < 0.0f) || (dist1v1 < 0.0f && dist1v2 >= 0.0f))
        // calculate intersection point and add to list on success
        if (LinePlaneIntersect(&plane2, pP1V1, pP1V2, &p))
        {
            Copy(&p, &p1pt1);
            ++p1ptsCount;
        }

    // is first polygon V2 to V3 line segment intersects second polygon plane?
    if ((dist1v2 >= 0.0f && dist1v3 < 0.0f) || (dist1v2 < 0.0f && dist1v3 >= 0.0f))
        // calculate intersection point and add to list on success
        if (LinePlaneIntersect(&plane2, pP1V2, pP1V3, &p))
        {
            Copy(&p, &p1pt2);
            ++p1ptsCount;
        }

    // is first polygon V3 to V1 line segment intersects second polygon plane?
    if ((dist1v3 >= 0.0f && dist1v1 < 0.0f) || (dist1v3 < 0.0f && dist1v1 >= 0.0f))
        // calculate intersection point and add to list on success
        if (LinePlaneIntersect(&plane2, pP1V3, pP1V1, &p))
        {
            Copy(&p, &p1pt3);
            ++p1ptsCount;
        }

    // were the first polygon 2 intersection point found?
    if (p1ptsCount != 2)
        return 0;

    // calculate distance from each second polygon vertex to first polygon plane
    DistanceToPlane(pP2V1, &plane1, &dist2v1);
    DistanceToPlane(pP2V2, &plane1, &dist2v2);
    DistanceToPlane(pP2V3, &plane1, &dist2v3);

    // prepare list containing second polygon intersection points
    p2ptsCount = 0;

    // is second polygon V1 to V2 line segment intersects first polygon plane?
    if ((dist2v1 >= 0.0f && dist2v2 < 0.0f) || (dist2v1 < 0.0f && dist2v2 >= 0.0f))
        // calculate intersection point and add to list on success
        if (LinePlaneIntersect(&plane1, pP2V1, pP2V2, &p))
        {
            Copy(&p, &p2pt1);
            ++p2ptsCount;
        }

    // is second polygon V2 to V3 line segment intersects first polygon plane?
    if ((dist2v2 >= 0.0f && dist2v3 < 0.0f) || (dist2v2 < 0.0f && dist2v3 >= 0.0f))
        // calculate intersection point and add to list on success
        if (LinePlaneIntersect(&plane1, pP2V2, pP2V3, &p))
        {
            Copy(&p, &p2pt2);
            ++p2ptsCount;
        }

    // is second polygon V3 to V1 line segment intersects first polygon plane?
    if ((dist2v3 >= 0.0f && dist2v1 < 0.0f) || (dist2v3 < 0.0f && dist2v1 >= 0.0f))
        // calculate intersection point and add to list on success
        if (LinePlaneIntersect(&plane1, pP2V3, pP2V1, &p))
        {
            Copy(&p, &p2pt3);
            ++p2ptsCount;
        }

    // were the second polygon 2 intersection point found?
    if (p2ptsCount != 2)
        return 0;

    // first and second polygon intersection points are on the same line, so
    // check if calculated first and second polygon segments intersect
    if (VectorIsBetween(&p1pt1, &p2pt1, &p2pt2, &epsylon))
        return 1;

    if (VectorIsBetween(&p1pt2, &p2pt1, &p2pt2, &epsylon))
        return 1;

    if (VectorIsBetween(&p2pt1, &p1pt1, &p1pt2, &epsylon))
        return 1;

    if (VectorIsBetween(&p2pt2, &p1pt1, &p1pt2, &epsylon))
        return 1;

    return 0;
}

/**
* Checks if a ray intersects a polygon
*@param pRay - ray
*@param pPolygon - polygon
*@return 1 if ray intersects polygon, otherwise 0
*/
int TriRayIntersect(const MG_Ray* pRay, const MC_Polygon* pPolygon)
{
    MG_Plane   polygonPlane;
    MG_Vector3 pointOnPlane;

    // create a plane using the 3 vertices of the polygon
    PlaneFromPoints(&pPolygon->m_v[0], &pPolygon->m_v[1], &pPolygon->m_v[2], &polygonPlane);

    // calculate the intersection point
    if (!RayPlaneIntersect(&polygonPlane, &pRay->m_Pos, &pRay->m_Dir, &pointOnPlane))
        return 0;

    // check if calculated point is inside the polygon
    return Inside(&pointOnPlane, &pPolygon->m_v[0], &pPolygon->m_v[1], &pPolygon->m_v[2]);
}

/**
* Checks if a ray intersects a box
*@param pRay - ray
*@param pBox - box
*@return 1 if ray intersects box, otherwise 0
*/
int RayBoxIntersect(MG_Ray* pRay, MG_Box* pBox)
{
    float tx1;
    float tx2;
    float ty1;
    float ty2;
    float tz1;
    float tz2;
    float txn;
    float txf;
    float tyn;
    float tyf;
    float tzn;
    float tzf;
    float tnear;
    float tfar;

    // get infinite value
    const float inf = 1.0f / 0.0f;

    // calculate nearest point where ray intersects box on x coordinate
    if (pRay->m_InvDir.m_X != inf)
        tx1 = ((pBox->m_Min.m_X - pRay->m_Pos.m_X) * pRay->m_InvDir.m_X);
    else
    if ((pBox->m_Min.m_X - pRay->m_Pos.m_X) < 0.0f)
        tx1 = -inf;
    else
        tx1 = inf;

    // calculate farthest point where ray intersects box on x coordinate
    if (pRay->m_InvDir.m_X != inf)
        tx2 = ((pBox->m_Max.m_X - pRay->m_Pos.m_X) * pRay->m_InvDir.m_X);
    else
    if ((pBox->m_Max.m_X - pRay->m_Pos.m_X) < 0.0f)
        tx2 = -inf;
    else
        tx2 = inf;

    // calculate nearest point where ray intersects box on y coordinate
    if (pRay->m_InvDir.m_Y != inf)
        ty1 = ((pBox->m_Min.m_Y - pRay->m_Pos.m_Y) * pRay->m_InvDir.m_Y);
    else
    if ((pBox->m_Min.m_Y - pRay->m_Pos.m_Y) < 0.0f)
        ty1 = -inf;
    else
        ty1 = inf;

    // calculate farthest point where ray intersects box on y coordinate
    if (pRay->m_InvDir.m_Y != inf)
        ty2 = ((pBox->m_Max.m_Y - pRay->m_Pos.m_Y) * pRay->m_InvDir.m_Y);
    else
    if ((pBox->m_Max.m_Y - pRay->m_Pos.m_Y) < 0.0f)
        ty2 = -inf;
    else
        ty2 = inf;

    // calculate nearest point where ray intersects box on z coordinate
    if (pRay->m_InvDir.m_Z != inf)
        tz1 = ((pBox->m_Min.m_Z - pRay->m_Pos.m_Z) * pRay->m_InvDir.m_Z);
    else
    if ((pBox->m_Min.m_Z - pRay->m_Pos.m_Z) < 0.0f)
        tz1 = -inf;
    else
        tz1 = inf;

    // calculate farthest point where ray intersects box on z coordinate
    if (pRay->m_InvDir.m_Z != inf)
        tz2 = ((pBox->m_Max.m_Z - pRay->m_Pos.m_Z) * pRay->m_InvDir.m_Z);
    else
    if ((pBox->m_Max.m_Z - pRay->m_Pos.m_Z) < 0.0f)
        tz2 = -inf;
    else
        tz2 = inf;

    // calculate near/far intersection on each axis
    Min(&tx1, &tx2, &txn);
    Max(&tx1, &tx2, &txf);
    Min(&ty1, &ty2, &tyn);
    Max(&ty1, &ty2, &tyf);
    Min(&tz1, &tz2, &tzn);
    Max(&tz1, &tz2, &tzf);

    // calculate final near/far intersection point
    Max(&tyn, &tzn,   &tnear);
    Max(&txn, &tnear, &tnear);
    Min(&tyf, &tzf,  &tfar);
    Min(&txf, &tfar, &tfar);

    // check if ray intersects box
    return (tfar >= tnear);
}

//-----------------------------------------------------------------------------
// Polygons extraction
//-----------------------------------------------------------------------------

/**
* Adds polygon to array
*@param pVB - source vertex buffer
*@param v1 - first polygon vertex index in vertex buffer
*@param v2 - second polygon vertex index in vertex buffer
*@param v3 - third polygon vertex index in vertex buffer
*@param[out] pPolygons - polygon array that contains generated polygons
*@param[out] pPolygonsCount - polygons count contained in array
*@note Generated polygons should be deleted by calling ReleasePolygons() when useless
*/
void AddPolygon(const float*       pVB,
                      unsigned     v1,
                      unsigned     v2,
                      unsigned     v3,
                      MC_Polygon** pPolygons,
                      unsigned*    pPolygonsCount)
{
    // polygon array already contains polygons?
    if (!(*pPolygonsCount))
    {
        // no, add new first polygon in array
        *pPolygons     = (MC_Polygon*)malloc(sizeof(MC_Polygon));
        *pPolygonsCount = 1;
    }
    else
    {
        // yes, increase the polygons count and add new polygon inside array
        ++(*pPolygonsCount);
        *pPolygons = (MC_Polygon*)realloc(*pPolygons,
                                          *pPolygonsCount * sizeof(MC_Polygon));
    }

    // copy polygon
    (*pPolygons)[*pPolygonsCount - 1].m_v[0].m_X = pVB[v1];
    (*pPolygons)[*pPolygonsCount - 1].m_v[0].m_Y = pVB[v1 + 1];
    (*pPolygons)[*pPolygonsCount - 1].m_v[0].m_Z = pVB[v1 + 2];
    (*pPolygons)[*pPolygonsCount - 1].m_v[1].m_X = pVB[v2];
    (*pPolygons)[*pPolygonsCount - 1].m_v[1].m_Y = pVB[v2 + 1];
    (*pPolygons)[*pPolygonsCount - 1].m_v[1].m_Z = pVB[v2 + 2];
    (*pPolygons)[*pPolygonsCount - 1].m_v[2].m_X = pVB[v3];
    (*pPolygons)[*pPolygonsCount - 1].m_v[2].m_Y = pVB[v3 + 1];
    (*pPolygons)[*pPolygonsCount - 1].m_v[2].m_Z = pVB[v3 + 2];
}

/**
* Gets polygons from vertex buffer
*@param pVB - source vertex buffer
*@param length - vertex buffer length
*@param type - polygons type as arranged in vertex buffer, where:
*              0 = triangles
*              1 = triangle strip
*              2 = triangle fan
*              3 = quads
*              4 = quad strip
*@param[out] pPolygons - polygon array that contains generated polygons
*@param[out] pPolygonsCount - polygons count contained in array
*@return 1 on success, otherwise 0
*@note Generated polygons should be deleted by calling ReleasePolygons() when useless
*/
int GetPolygonsFromVB(const float*       pVB,
                            unsigned     length,
                            unsigned     type,
                            unsigned     stride,
                            MC_Polygon** pPolygons,
                            unsigned*    pPolygonsCount)
{
    unsigned i;

    // no data to extract from?
    if (!length)
        return 1;

    // search for vertex type
    switch (type)
    {
        // triangles
        case 0:
        {
            // calculate iteration step
            const unsigned step = (stride * 3);

            // iterate through source vertices
            for (i = 0; i < length; i += step)
                // extract polygon from source buffer
                AddPolygon(pVB,
                           i,
                           i +  stride,
                           i + (stride * 2),
                           pPolygons,
                           pPolygonsCount);

            return 1;
        }

        // triangle strip
        case 1:
        {
            // calculate length to read in triangle strip buffer
            const unsigned stripLength = (length - (stride * 2));

            // iterate through source vertices
            for (i = 0; i < stripLength; i += stride)
            {
                // extract polygon from source buffer, revert odd polygons
                if (!i || i % 2)
                    AddPolygon(pVB,
                               i,
                               i +  stride,
                               i + (stride * 2),
                               pPolygons,
                               pPolygonsCount);
                else
                    AddPolygon(pVB,
                               i +  stride,
                               i,
                               i + (stride * 2),
                               pPolygons,
                               pPolygonsCount);
            }

            return 1;
        }

        // triangle fan
        case 2:
        {
            // calculate length to read in triangle fan buffer
            const unsigned fanLength = (length - stride);

            // iterate through source vertices
            for (i = stride; i < fanLength; i += stride)
                // extract polygon from source buffer
                AddPolygon(pVB,
                           0,
                           i,
                           i + stride,
                           pPolygons,
                           pPolygonsCount);

            return 1;
        }

        // quads
        case 3:
        {
            // calculate iteration step
            const unsigned step = (stride * 4);

            // iterate through source vertices
            for (i = 0; i < length; i += step)
            {
                // calculate vertices position
                const unsigned v1 = i;
                const unsigned v2 = i +  stride;
                const unsigned v3 = i + (stride * 2);
                const unsigned v4 = i + (stride * 3);

                // extract polygons from source buffer
                AddPolygon(pVB, v1, v2, v3, pPolygons, pPolygonsCount);
                AddPolygon(pVB, v3, v2, v4, pPolygons, pPolygonsCount);
            }

            return 1;
        }

        // quad strip
        case 4:
        {
            // calculate iteration step
            const unsigned step = (stride * 2);

            // calculate length to read in triangle strip buffer
            const unsigned stripLength = (length - (stride * 2));

            // iterate through source vertices
            for (i = 0; i < stripLength; i += step)
            {
                // calculate vertices position
                const unsigned v1 = i;
                const unsigned v2 = i +  stride;
                const unsigned v3 = i + (stride * 2);
                const unsigned v4 = i + (stride * 3);

                // extract polygons from source buffer
                AddPolygon(pVB, v1, v2, v3, pPolygons, pPolygonsCount);
                AddPolygon(pVB, v3, v2, v4, pPolygons, pPolygonsCount);
            }

            return 1;
        }

        default:
            return 0;
    }
}

/**
* Release polygons previously created by GetPolygonsFromVB() or AddPolygon()
*@param pPolygons - polygon array to release
*/
void ReleasePolygons(MC_Polygon* pPolygons)
{
    if (pPolygons)
        free(pPolygons);
}

//-----------------------------------------------------------------------------
// Aligned-Axis Bounding Box tree
//-----------------------------------------------------------------------------

/**
* Cuts box on the longest axis
*@param pBox - box to cut
*@param[out] pLeftBox - resulting left box
*@param[out] pRightBox - resulting right box
*/
void CutBox(const MG_Box* pBox, MG_Box* pLeftBox, MG_Box* pRightBox)
{
    float    x;
    float    y;
    float    z;
    unsigned longestAxis;

    // calculate each edge length
    x = fabs(pBox->m_Max.m_X - pBox->m_Min.m_X);
    y = fabs(pBox->m_Max.m_Y - pBox->m_Min.m_Y);
    z = fabs(pBox->m_Max.m_Z - pBox->m_Min.m_Z);

    // search for longest axis
    if (x >= y && x >= z)
        longestAxis = 0;
    else
    if (y >= x && y >= z)
        longestAxis = 1;
    else
        longestAxis = 2;

    // cut box
    switch (longestAxis)
    {
        // cut on x axis
        case 0:
            Copy(&pBox->m_Min, &pLeftBox->m_Min);
            Copy(&pBox->m_Max, &pLeftBox->m_Max);
            pLeftBox->m_Max.m_X = pBox->m_Min.m_X + (x / 2.0f);

            Copy(&pBox->m_Min, &pRightBox->m_Min);
            Copy(&pBox->m_Max, &pRightBox->m_Max);
            pRightBox->m_Min.m_X = pBox->m_Min.m_X + (x / 2.0f);
            break;

        // cut on y axis
        case 1:
            Copy(&pBox->m_Min, &pLeftBox->m_Min);
            Copy(&pBox->m_Max, &pLeftBox->m_Max);
            pLeftBox->m_Max.m_Y = pBox->m_Min.m_Y + (y / 2.0f);

            Copy(&pBox->m_Min, &pRightBox->m_Min);
            Copy(&pBox->m_Max, &pRightBox->m_Max);
            pRightBox->m_Min.m_Y = pBox->m_Min.m_Y + (y / 2.0f);
            break;

        // cut on z axis
        case 2:
            Copy(&pBox->m_Min, &pLeftBox->m_Min);
            Copy(&pBox->m_Max, &pLeftBox->m_Max);
            pLeftBox->m_Max.m_Z = pBox->m_Min.m_Z + (z / 2.0f);

            Copy(&pBox->m_Min, &pRightBox->m_Min);
            Copy(&pBox->m_Max, &pRightBox->m_Max);
            pRightBox->m_Min.m_Z = pBox->m_Min.m_Z + (z / 2.0f);
            break;
    }
}

/**
* Populates AABB tree
*@param pNode - root or parent node to create from
*@param pPolygons - source polygon array
*@param polygonsCount - polygon array count
*@return 1 on success, otherwise 0
*/
int PopulateTree(MC_AABBNode* pNode, const MC_Polygon* pPolygons, unsigned polygonsCount)
{
    unsigned    i;
    unsigned    j;
    MG_Box      leftBox;
    MG_Box      rightBox;
    MC_Polygon* pLeftPolygons      = 0;
    MC_Polygon* pRightPolygons     = 0;
    unsigned    leftPolygonsCount  = 0;
    unsigned    rightPolygonsCount = 0;
    int         boxEmpty           = 1;
    int         canResolveLeft     = 0;
    int         canResolveRight    = 0;
    int         result             = 0;
    float       epsylon            = M_MC_Epsilon;

    // initialize node content
    pNode->m_pParent       = 0;
    pNode->m_pLeft         = 0;
    pNode->m_pRight        = 0;
    pNode->m_pBox          = 0;
    pNode->m_pPolygons     = 0;
    pNode->m_PolygonsCount = 0;

    // create a collision box
    pNode->m_pBox = (MG_Box*)malloc(sizeof(MG_Box));

    // iterate through polygons to divide
    for (i = 0; i < polygonsCount; ++i)
        // calculate bounding box
        AddPolygonToBoundingBox(&pPolygons[i], pNode->m_pBox, &boxEmpty);

    // divide box in 2 sub-boxes
    CutBox(pNode->m_pBox, &leftBox, &rightBox);

    // iterate again through polygons to divide
    for (i = 0; i < polygonsCount; ++i)
        for (j = 0; j < 3; ++i)
            // check if first polygon vertice belongs to left or right sub-box
            if (VectorIsBetween(&pPolygons[i].m_v[j], &leftBox.m_Min, &leftBox.m_Max, &epsylon))
            {
                // left node polygon array already contains polygons?
                if (!leftPolygonsCount)
                {
                    // no, add new first polygon in array
                    pLeftPolygons     = (MC_Polygon*)malloc(sizeof(MC_Polygon));
                    leftPolygonsCount = 1;
                }
                else
                {
                    // yes, increase the polygons count and add new polygon inside array
                    ++leftPolygonsCount;
                    pLeftPolygons = (MC_Polygon*)realloc(pLeftPolygons,
                                                         leftPolygonsCount * sizeof(MC_Polygon));
                }

                // copy polygon
                Copy(&pPolygons[i].m_v[0], &pLeftPolygons[leftPolygonsCount - 1].m_v[0]);
                Copy(&pPolygons[i].m_v[1], &pLeftPolygons[leftPolygonsCount - 1].m_v[1]);
                Copy(&pPolygons[i].m_v[2], &pLeftPolygons[leftPolygonsCount - 1].m_v[2]);
                break;
            }
            else
            if (VectorIsBetween(&pPolygons[i].m_v[j], &rightBox.m_Min, &rightBox.m_Max, &epsylon))
            {
                // right node polygon array already contains polygons?
                if (!rightPolygonsCount)
                {
                    // no, add new first polygon in array
                    pRightPolygons    = (MC_Polygon*)malloc(sizeof(MC_Polygon));
                    rightPolygonsCount = 1;
                }
                else
                {
                    // yes, increase the polygons count and add new polygon inside array
                    ++rightPolygonsCount;
                    pRightPolygons = (MC_Polygon*)realloc(pRightPolygons,
                                                          rightPolygonsCount * sizeof(MC_Polygon));
                }

                // copy polygon
                Copy(&pPolygons[i].m_v[0], &pRightPolygons[rightPolygonsCount - 1].m_v[0]);
                Copy(&pPolygons[i].m_v[1], &pRightPolygons[rightPolygonsCount - 1].m_v[1]);
                Copy(&pPolygons[i].m_v[2], &pRightPolygons[rightPolygonsCount - 1].m_v[2]);
                break;
            }

    canResolveLeft  = (leftPolygonsCount  && leftPolygonsCount  < polygonsCount);
    canResolveRight = (rightPolygonsCount && rightPolygonsCount < polygonsCount);

    // leaf reached?
    if (!canResolveLeft && !canResolveRight)
    {
        // copy left polygons in polygon list
        for (i = 0; i < leftPolygonsCount; ++i)
        {
            // node polygon array already contains polygons?
            if (!pNode->m_PolygonsCount)
            {
                // no, add new first polygon in array
                pNode->m_pPolygons     = (MC_Polygon*)malloc(sizeof(MC_Polygon));
                pNode->m_PolygonsCount = 1;
            }
            else
            {
                // yes, increase the polygons count and add new polygon inside array
                ++pNode->m_PolygonsCount;
                pNode->m_pPolygons = (MC_Polygon*)realloc(pNode->m_pPolygons,
                                                          pNode->m_PolygonsCount * sizeof(MC_Polygon));
            }

            // copy polygon
            for (j = 0; j < 3 ; ++j)
                Copy(&pLeftPolygons[i].m_v[j], &pNode->m_pPolygons[pNode->m_PolygonsCount - 1].m_v[j]);
        }

        // copy right polygons in polygon list
        for (i = 0; i < rightPolygonsCount; ++i)
        {
            // node polygon array already contains polygons?
            if (!pNode->m_PolygonsCount)
            {
                // no, add new first polygon in array
                pNode->m_pPolygons     = (MC_Polygon*)malloc(sizeof(MC_Polygon));
                pNode->m_PolygonsCount = 1;
            }
            else
            {
                // yes, increase the polygons count and add new polygon inside array
                ++pNode->m_PolygonsCount;
                pNode->m_pPolygons = (MC_Polygon*)realloc(pNode->m_pPolygons,
                                                          pNode->m_PolygonsCount * sizeof(MC_Polygon));
            }

            // copy polygon
            for (j = 0; j < 3 ; ++j)
                Copy(&pRightPolygons[i].m_v[j], &pNode->m_pPolygons[pNode->m_PolygonsCount - 1].m_v[j]);
        }

        // delete left list, as it will no more be used
        if (pLeftPolygons)
            free(pLeftPolygons);

        // delete right list, as it will no more be used
        if (pRightPolygons)
            free(pRightPolygons);

        return 1;
    }

    // do create left node?
    if (canResolveLeft)
    {
        // create and populate left node
        pNode->m_pLeft            = (MC_AABBNode*)malloc(sizeof(MC_AABBNode));
        pNode->m_pLeft->m_pParent = pNode;

        result |= PopulateTree(pNode->m_pLeft, pLeftPolygons, leftPolygonsCount);

        // delete current list, as it will no more be used
        if (pLeftPolygons)
            free(pLeftPolygons);
    }

    // do create right node?
    if (canResolveRight)
    {
        // create and populate right node
        pNode->m_pRight            = (MC_AABBNode*)malloc(sizeof(MC_AABBNode));
        pNode->m_pRight->m_pParent = pNode;

        result |= PopulateTree(pNode->m_pRight, pRightPolygons, rightPolygonsCount);

        // delete current list, as it will no more be used
        if (pRightPolygons)
            free(pRightPolygons);
    }

    return result;
}

/**
* Resolves AABB tree
*@param pRay - ray against which tree boxes will be tested
*@param pNode - root or parent node to resolve
*@param[out] pPolygons - polygons belonging to boxes hit by ray
*@param[out] polygonsCount - polygon array count
*@return 1 on success, otherwise 0
*/
int ResolveTree(MG_Ray* pRay, MC_AABBNode* pNode, MC_Polygon** pPolygons, unsigned* pPolygonsCount)
{
    unsigned i;
    unsigned j;
    int      leftResolved  = 0;
    int      rightResolved = 0;

    // no node to resolve? (this should never happen, but...)
    if (!pNode)
        return 0;

    // is leaf?
    if (!pNode->m_pLeft && !pNode->m_pRight)
    {
        for (i = 0; i < pNode->m_PolygonsCount; ++i)
        {
            // polygon array already contains polygons?
            if (!(*pPolygonsCount))
            {
                // no, add new first polygon in array
                *pPolygons     = (MC_Polygon*)malloc(sizeof(MC_Polygon));
                *pPolygonsCount = 1;
            }
            else
            {
                // yes, increase the polygons count and add new polygon inside array
                ++(*pPolygonsCount);
                *pPolygons = (MC_Polygon*)realloc(*pPolygons,
                                                  *pPolygonsCount * sizeof(MC_Polygon));
            }

            // copy polygon
            for (j = 0; j < 3; ++j)
                Copy(&pNode->m_pPolygons[i].m_v[j], &(*pPolygons)[*pPolygonsCount - 1].m_v[j]);
        }

        return 1;
    }

    // check if ray intersects the left box
    if (pNode->m_pLeft && RayBoxIntersect(pRay, pNode->m_pLeft->m_pBox))
        // resolve left node
        leftResolved = ResolveTree(pRay, pNode->m_pLeft, pPolygons, pPolygonsCount);

    // check if ray intersects the right box
    if (pNode->m_pRight && RayBoxIntersect(pRay, pNode->m_pRight->m_pBox))
        // resolve right node
        rightResolved = ResolveTree(pRay, pNode->m_pRight, pPolygons, pPolygonsCount);

    return (leftResolved || rightResolved);
}

/**
* Releases node content
*@param pNode - node for which content should be released
*@note Only the node content is released, the node itself is not released
*/
void ReleaseNode(MC_AABBNode* pNode)
{
    // delete bounding box, if exists
    if (pNode->m_pBox)
        free(pNode->m_pBox);

    // delete polygon array, if exists
    if (pNode->m_pPolygons)
        free(pNode->m_pPolygons);

    pNode->m_PolygonsCount = 0;
}

/**
* Releases tree content
*@param pNode - root node from which content should be released
*/
void ReleaseTree(MC_AABBNode* pNode)
{
    if (!pNode)
        return;

    // release all children on left side
    if (pNode->m_pLeft)
        ReleaseTree(pNode->m_pLeft);

    // release all children on right side
    if (pNode->m_pRight)
        ReleaseTree(pNode->m_pRight);

    // delete node content
    ReleaseNode(pNode);

    // delete node
    free(pNode);
}

#endif // MiniCollisionH
