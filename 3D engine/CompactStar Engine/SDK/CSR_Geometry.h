/****************************************************************************
 * ==> CSR_Geometry --------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the geometric functions and types     *
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

#ifndef CSR_GeometryH
#define CSR_GeometryH

// std
#include <stddef.h>

// compactStar engine
#include "CSR_Common.h"

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* 2D geometric figure enumeration
*/
typedef enum
{
    CSR_F2_Line,
    CSR_F2_Ray,
    CSR_F2_Segment,
    CSR_F2_Polygon,
    CSR_F2_Rect,
    CSR_F2_Circle
} CSR_EFigure2;

/**
* 3D geometric figure enumeration
*/
typedef enum
{
    CSR_F3_Line,
    CSR_F3_Ray,
    CSR_F3_Segment,
    CSR_F3_Plane,
    CSR_F3_Polygon,
    CSR_F3_Box,
    CSR_F3_Sphere
} CSR_EFigure3;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* 2D vector
*/
typedef struct
{
    float m_X;
    float m_Y;
} CSR_Vector2;

/**
* 3D vector
*/
typedef struct
{
    float m_X;
    float m_Y;
    float m_Z;
} CSR_Vector3;

/**
* 4x4 matrix
*/
typedef struct
{
    float m_Table[4][4];
} CSR_Matrix4;

/**
* Quaternion
*/
typedef struct
{
    float m_X;
    float m_Y;
    float m_Z;
    float m_W;
} CSR_Quaternion;

/**
* Plane
*/
typedef struct
{
    float m_A;
    float m_B;
    float m_C;
    float m_D;
} CSR_Plane;

/**
* 2D Ray
*/
typedef struct
{
    CSR_Vector2 m_Pos;
    CSR_Vector2 m_Dir;
    CSR_Vector2 m_InvDir;
} CSR_Ray2;

/**
* 2D Line segment
*/
typedef struct
{
    CSR_Vector2 m_Start;
    CSR_Vector2 m_End;
} CSR_Segment2;

/**
* 3D Ray
*/
typedef struct
{
    CSR_Vector3 m_Pos;
    CSR_Vector3 m_Dir;
    CSR_Vector3 m_InvDir;
} CSR_Ray3;

/**
* 3D Line segment
*/
typedef struct
{
    CSR_Vector3 m_Start;
    CSR_Vector3 m_End;
} CSR_Segment3;

/**
* Circle
*/
typedef struct
{
    CSR_Vector2 m_Center;
    float       m_Radius;
} CSR_Circle;

/**
* Aligned-axis rect
*/
typedef struct
{
    CSR_Vector2 m_Min;
    CSR_Vector2 m_Max;
} CSR_Rect;

/**
* Sphere
*/
typedef struct
{
    CSR_Vector3 m_Center;
    float       m_Radius;
} CSR_Sphere;

/**
* Aligned-axis box
*/
typedef struct
{
    CSR_Vector3 m_Min;
    CSR_Vector3 m_Max;
} CSR_Box;

/**
* 2D Polygon
*/
typedef struct
{
    CSR_Vector2 m_Vertex[3];
} CSR_Polygon2;

/**
* 3D Polygon
*/
typedef struct
{
    CSR_Vector3 m_Vertex[3];
} CSR_Polygon3;

/**
* 3D polygon buffer
*/
typedef struct
{
    CSR_Polygon3* m_pPolygon;
    size_t        m_Count;
} CSR_Polygon3Buffer;

/**
* 2D Figure
*/
typedef struct
{
    CSR_EFigure2 m_Type;
    const void*  m_pFigure;
} CSR_Figure2;

/**
* 3D Figure
*/
typedef struct
{
    CSR_EFigure3 m_Type;
    const void*  m_pFigure;
} CSR_Figure3;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // 2D vector functions
        //-------------------------------------------------------------------

        /**
        * Adds vector contents
        *@param pV1 - vector that contains values to be added
        *@param pV2 - vector to add to
        *@param[out] pR - resulting vector
        */
        void csrVec2Add(const CSR_Vector2* pV1, const CSR_Vector2* pV2, CSR_Vector2* pR);

        /**
        * Subtracts vector contents
        *@param pV1 - vector that contains values to be subtracted
        *@param pV2 - vector to subtract by
        *@param[out] pR - resulting vector
        */
        void csrVec2Sub(const CSR_Vector2* pV1, const CSR_Vector2* pV2, CSR_Vector2* pR);

        /**
        * Calculates vector length
        *@param pV - vector to calculate length
        *@param[out] pR - vector length
        */
        void csrVec2Length(const CSR_Vector2* pV, float* pR);

        /**
        * Normalizes the vector
        *@param pV - vector to normalize
        *@param[out] pR - normailzed vector
        */
        void csrVec2Normalize(const CSR_Vector2* pV, CSR_Vector2* pR);

        /**
        * Calculates cross product between 2 vectors
        *@param pV1 - first vector
        *@param pV2 - second vector
        *@param[out] pR - resulting vector
        */
        void csrVec2Cross(const CSR_Vector2* pV1, const CSR_Vector2* pV2, CSR_Vector2* pR);

        /**
        * Calculates dot product between 2 vectors
        *@param pV1 - first vector
        *@param pV2 - second vector
        *@param[out] pR - resulting angle
        */
        void csrVec2Dot(const CSR_Vector2* pV1, const CSR_Vector2* pV2, float* pR);

        /**
        * Checks if a vector is between a range of values
        *@param pV - vector to test
        *@param pRS - range start
        *@param pRE - range end
        *@param tolerance - tolerance for calculation
        *@return 1 if vector is between range, otherwise 0
        */
        int csrVec2BetweenRange(const CSR_Vector2* pV,
                                const CSR_Vector2* pRS,
                                const CSR_Vector2* pRE,
                                      float        tolerance);

        //-------------------------------------------------------------------
        // 3D vector functions
        //-------------------------------------------------------------------

        /**
        * Adds vector contents
        *@param pV1 - vector that contains values to be added
        *@param pV2 - vector to add to
        *@param[out] pR - resulting vector
        */
        void csrVec3Add(const CSR_Vector3* pV1, const CSR_Vector3* pV2, CSR_Vector3* pR);

        /**
        * Subtracts vector contents
        *@param pV1 - vector that contains values to be subtracted
        *@param pV2 - vector to subtract by
        *@param[out] pR - resulting vector
        */
        void csrVec3Sub(const CSR_Vector3* pV1, const CSR_Vector3* pV2, CSR_Vector3* pR);

        /**
        * Calculates vector length
        *@param pV - vector to calculate length
        *@param[out] pR - vector length
        */
        void csrVec3Length(const CSR_Vector3* pV, float* pR);

        /**
        * Normalizes the vector
        *@param pV - vector to normalize
        *@param[out] pR - normailzed vector
        */
        void csrVec3Normalize(const CSR_Vector3* pV, CSR_Vector3* pR);

        /**
        * Calculates cross product between 2 vectors
        *@param pV1 - first vector
        *@param pV2 - second vector
        *@param[out] pR - resulting vector
        */
        void csrVec3Cross(const CSR_Vector3* pV1, const CSR_Vector3* pV2, CSR_Vector3* pR);

        /**
        * Calculates dot product between 2 vectors
        *@param pV1 - first vector
        *@param pV2 - second vector
        *@param[out] pR - resulting angle
        */
        void csrVec3Dot(const CSR_Vector3* pV1, const CSR_Vector3* pV2, float* pR);

        /**
        * Checks if a vector is between a range of values
        *@param pV - vector to test
        *@param pRS - range start
        *@param pRE - range end
        *@param tolerance - tolerance for calculation
        *@return 1 if vector is between range, otherwise 0
        */
        int csrVec3BetweenRange(const CSR_Vector3* pV,
                                const CSR_Vector3* pRS,
                                const CSR_Vector3* pRE,
                                      float        tolerance);

        //-------------------------------------------------------------------
        // Matrix functions
        //-------------------------------------------------------------------

        /**
        * Gets matrix identity
        *@param[out] pR - resulting identity matrix
        */
        void csrMat4Identity(CSR_Matrix4* pR);

        /**
        * Gets orthogonal matrix
        *@param left - left view rect edge
        *@param right - right view rect edge
        *@param bottom - bottom view rect edge
        *@param top - top view rect edge
        *@param zNear - z near clipping value
        *@param zFar - z far clipping value
        *@param[out] pR - resulting orthogonal matrix
        */
        void csrMat4Ortho(float        left,
                          float        right,
                          float        bottom,
                          float        top,
                          float        zNear,
                          float        zFar,
                          CSR_Matrix4* pR);

        /**
        * Gets frustrum matrix
        *@param left - left view rect edge
        *@param right - right view rect edge
        *@param bottom - bottom view rect edge
        *@param top - top view rect edge
        *@param zNear - z near clipping value
        *@param zFar - z far clipping value
        *@param[out] pR - resulting frustrum matrix
        */
        void csrMat4Frustum(float        left,
                            float        right,
                            float        bottom,
                            float        top,
                            float        zNear,
                            float        zFar,
                            CSR_Matrix4* pR);

        /**
        * Gets perspective matrix
        *@param fovyDeg - field of view angle in radian
        *@param aspect - aspect ratio
        *@param zNear - z near clipping value
        *@param zFar - z far clipping value
        *@param[out] pR - resulting perspective matrix
        */
        void csrMat4Perspective(float        fovyDeg,
                                float        aspect,
                                float        zNear,
                                float        zFar,
                                CSR_Matrix4* pR);

        /**
        * Gets translation matrix
        *@param pT - translation vector
        *@param[out] pR - resulting translation matrix
        */
        void csrMat4Translate(const CSR_Vector3* pT, CSR_Matrix4* pR);

        /**
        * Gets rotation matrix
        *@param angle - rotation angle, in radians
        *@param pAxis - rotation axis
        *@param[out] pR - resulting orthogonal matrix
        */
        void csrMat4Rotate(float angle, const CSR_Vector3* pAxis, CSR_Matrix4* pR);

        /**
        * Gets scale matrix
        *@param pFactor - scale factor
        *@param[out] pR - resulting scale matrix
        */
        void csrMat4Scale(const CSR_Vector3* pFactor, CSR_Matrix4* pR);

        /**
        * Multiplies matrix by another matrix
        *@param pM1 - first matrix to multiply
        *@param pM2 - second matrix to multiply with
        *@param[out] pR - resulting orthogonal matrix
        */
        void csrMat4Multiply(const CSR_Matrix4* pM1, const CSR_Matrix4* pM2, CSR_Matrix4* pR);

        /**
        * Inverses a matrix
        *@param pM - matrix to inverse
        *@param[out] pR - inversed matrix
        *@param[out] pDeterminant - matrix determinant
        */
        void csrMat4Inverse(const CSR_Matrix4* pM, CSR_Matrix4* pR, float* pDeterminant);

        /**
        * Applies a matrix to a vector
        *@param pM - matrix to apply
        *@param pV - vector on which matrix should be applied
        *@param[out] pR - resulting vector
        */
        void csrMat4ApplyToVector(const CSR_Matrix4* pM, const CSR_Vector3* pV, CSR_Vector3* pR);

        /**
        * Applies a matrix to a normal
        *@param pM - matrix to apply
        *@param pN - normal on which matrix should be applied
        *@param[out] pR - resulting normal
        */
        void csrMat4ApplyToNormal(const CSR_Matrix4* pM, const CSR_Vector3* pN, CSR_Vector3* pR);

        /**
        * Transforms a vector by a matrix
        *@param pM - transform matrix
        *@param pV - vector to transform
        *@param[out] pR - transformed vector
        */
        void csrMat4Transform(const CSR_Matrix4* pM, const CSR_Vector3* pV, CSR_Vector3* pR);

        /**
        * Unprojects a ray (i.e. transforms it in viewport coordinates)
        *@param pP - projection matrix
        *@param pV - view matrix
        *@param[in, out] pR - ray to unproject, unprojected ray on function ends
        *@note The inverted direction is also calculated in the resulting ray
        */
        void csrMat4Unproject(const CSR_Matrix4* pP, const CSR_Matrix4* pV, CSR_Ray3* pR);

        //-------------------------------------------------------------------
        // Quaternion functions
        //-------------------------------------------------------------------

        /**
        * Gets a quaternion from an angle and a vector representing a rotation axis
        *@param pAngle - rotation angle
        *@param pAxis- rotation axis
        *@param[out] pR - resulting quaternion
        */
        void csrQuatFromAxis(float angle, const CSR_Vector3* pAxis, CSR_Quaternion* pR);

        /**
        * Gets a quaternion from Euler angles
        *@param pAngleX - rotation angle on x axis
        *@param pAngleY - rotation angle on y axis
        *@param pAngleZ - rotation angle on z axis
        *@param[out] pR - resulting quaternion
        */
        void csrQuatFromEuler(float angleX, float angleY, float angleZ, CSR_Quaternion* pR);

        /**
        * Gets a quaternion from a pitch, yaw and roll angles
        *@param pitch - the pitch angle in radians, pitch is comparable to a head up/down movement
        *@param yaw - the yaw angle in radians, yaw is comparable to a head left/right movement
        *@param roll - the roll angle in radians, roll is comparable to an aircraft rolling movement
        *@param[out] pR - resulting quaternion
        */
        void csrQuatFromPYR(float pitch, float yaw, float roll, CSR_Quaternion* pR);

        /**
        * Gets the squared length of a quaternion
        *@param pQ - quaternion for which the norm should be get
        *@param[out] pR - the resulting norm
        */
        void csrQuatLengthSquared(const CSR_Quaternion* pQ, float* pR);

        /**
        * Gets the quaternion length
        *@param pQ - quaternion for which the length should be get
        *@param[out] pR - the resulting length
        */
        void csrQuatLength(const CSR_Quaternion* pQ, float* pR);

        /**
        * Normalizes the quaternion
        *@param pV - vector to normalize
        *@param[out] pR - normailzed vector
        */
        void csrQuatNormalize(const CSR_Quaternion* pQ, CSR_Quaternion* pR);

        /**
        * Calculates dot product between 2 quaternions
        *@param pQ1 - first quaternion
        *@param pQ2 - second quaternion
        *@param[out] pR - resulting angle
        */
        void csrQuatDot(const CSR_Quaternion* pQ1, const CSR_Quaternion* pQ2, float* pR);

        /**
        * Scales a quaternion
        *@param pQ - quaternion to scale
        *@param s - scale factor
        *@param[out] pR - scaled quaternion
        */
        void csrQuatScale(const CSR_Quaternion* pQ, float s, CSR_Quaternion* pR);

        /**
        * Conjugates the quaternion
        *@param pQ - quaternion to conjugate
        *@param[out] pR - conjugated quaternion
        */
        void csrQuatConjugate(const CSR_Quaternion* pQ, CSR_Quaternion* pR);

        /**
        * Multiplies a quaternion by another
        *@param pQ1 - first quaternion to multiply
        *@param pQ2 - second quaternion to multiply with
        *@param[out] pR - multiplied quaternion
        */
        void csrQuatMultiply(const CSR_Quaternion* pQ1, const CSR_Quaternion* pQ2, CSR_Quaternion* pR);

        /**
        * Inverse the quaternion
        *@param pQ - quaternion to inverse
        *@param[out] pR - inverted quaternion
        */
        void csrQuatInverse(const CSR_Quaternion* pQ, CSR_Quaternion* pR);

        /**
        * Rotates a vector by a quaternion
        *@param pQ - quaternion by which the vector will be rotated
        *@param pV  vector to rotate
        *@param[out] pR - rotated vector
        */
        void csrQuatRotate(const CSR_Quaternion* pQ, const CSR_Vector3* pV, CSR_Vector3* pR);

        /**
        * Gets the spherical linear interpolated quaternion between 2 quaternions
        *@param pQ1 - quaternion to interpolate
        *@param pQ2 - quaternion to interpolate with
        *@param p - interpolation position, in percent (between 0.0f and 1.0f)
        *@param[out] pR - the resulting spherical linear interpolated quaternion
        *@return 1 on success, otherwise 0
        */
        int csrQuatSlerp(const CSR_Quaternion* pQ1,
                         const CSR_Quaternion* pQ2,
                               float           p,
                               CSR_Quaternion* pR);

        /**
        * Get a quaternion from a matrix
        *@param pM - rotation matrix
        *@param[out] pR - quaternion
        *@return 1 on success, otherwise 0
        */
        int csrQuatFromMatrix(const CSR_Matrix4* pM, CSR_Quaternion* pR);

        /**
        * Gets a rotation matrix from a quaternion
        *@param pQ - quaternion from which the matrix should be get
        *@param[out] pR - rotation matrix
        */
        void csrQuatToMatrix(const CSR_Quaternion* pQ, CSR_Matrix4* pR);

        //-------------------------------------------------------------------
        // Plane functions
        //-------------------------------------------------------------------

        /**
        * Creates a plane from a point and a normal
        *@param pP - point
        *@param pN - normal
        *@param[out] pR - resulting plane
        */
        void csrPlaneFromPointNormal(const CSR_Vector3* pP, const CSR_Vector3* pN, CSR_Plane* pR);

        /**
        * Creates a plane from 3 vertex
        *@param pV1 - first vector
        *@param pV2 - second vector
        *@param pV3 - third vector
        *@param[out] pR - resulting plane
        */
        void csrPlaneFromPoints(const CSR_Vector3* pV1,
                                const CSR_Vector3* pV2,
                                const CSR_Vector3* pV3,
                                      CSR_Plane*   pR);

        /**
        * Calculates a distance between a point and a plane
        *@param pP - point
        *@param pPl - plane
        *@param[out] - resulting distance
        */
        void csrPlaneDistanceTo(const CSR_Vector3* pP, const CSR_Plane* pPl, float* pR);

        //-------------------------------------------------------------------
        // 3D ray functions
        //-------------------------------------------------------------------

        /**
        * Calculates a ray from a point and a direction
        *@param pP - point
        *@param pD - direction
        *@param[out] pR - ray
        */
        void csrRay3FromPointDir(const CSR_Vector3* pP, const CSR_Vector3* pD, CSR_Ray3* pR);

        //-------------------------------------------------------------------
        // 3D segment functions
        //-------------------------------------------------------------------

        /**
        * Gets the shortest distance between 2 line segments
        *@param pS1 - first line segment
        *@param pS2 - second line segment from which distance should be calculated
        *@param tolerance - tolerance for calculation
        *@param[out] pR - resulting distance
        */
        void csrSeg3DistanceBetween(const CSR_Segment3* pS1,
                                    const CSR_Segment3* pS2,
                                          float         tolerance,
                                          float*        pR);

        /**
        * Calculates and gets the closest point on a line segment from a point
        *@param pS - line segment
        *@param pP - point
        *@param[out] pR - closest point on the line segment
        *@note The closest point is calculated as follow:
        *      pS->m_End
        *          |
        *          |
        *          |
        *       pR x------x pP
        *          |
        *          |
        *          |
        *      pS->m_Start
        */
        void csrSeg3ClosestPoint(const CSR_Segment3* pS, const CSR_Vector3* pP, CSR_Vector3* pR);

        //-------------------------------------------------------------------
        // 3D polygon functions
        //-------------------------------------------------------------------

        /**
        * Calculates and gets the closest point on a polygon edge from a point
        *@param pP - point
        *@param pPo - polygon
        *@param[out] pR - closest point on the polygon edge
        *@note The closest point is calculated as follow:
        *                      pPo->m_Vertex[0]
        *                            /\
        *                           /  \
        *                          /    \
        *                         /      \
        *                        /   pR   \
        *      pPo->m_Vertex[1] /____x_____\ pPo->m_Vertex[2]
        *                            |
        *                            |
        *                            x pP
        */
        void csrPolygon3ClosestPoint(const CSR_Vector3* pP, const CSR_Polygon3* pPo, CSR_Vector3* pR);

        //-------------------------------------------------------------------
        // Box functions
        //-------------------------------------------------------------------

        /**
        * Extends a box to encompass a polygon
        *@param pPolygon - polygon to encompass in the box
        *@param[in, out] pBox - bounding box that will encompass the polygon
        *@param[in, out] pEmpty - if 1, box is empty and still not contains any polygon
        */
        void csrBoxExtendToPolygon(const CSR_Polygon3* pPolygon,
                                         CSR_Box*      pBox,
                                         int*          pEmpty);

        /**
        * Cuts box on the longest axis
        *@param pBox - box to cut
        *@param[out] pLeftBox - resulting left box
        *@param[out] pRightBox - resulting right box
        */
        void csrBoxCut(const CSR_Box* pBox, CSR_Box* pLeftBox, CSR_Box* pRightBox);

        //-------------------------------------------------------------------
        // Inside checks
        //-------------------------------------------------------------------

        /**
        * Checks if a point is inside a 2D polygon
        *@param pP - point to check
        *@param pPo - polygon to check against
        *@return 1 if point is inside the polygon, otherwise 0
        */
        int csrInsidePolygon2(const CSR_Vector2* pP, const CSR_Polygon2* pPo);

        /**
        * Checks if a point is inside a polygon
        *@param pP - point to check
        *@param pPo - polygon to check against
        *@return 1 if point is inside the polygon, otherwise 0
        */
        int csrInsidePolygon3(const CSR_Vector3* pP, const CSR_Polygon3* pPo);

        /**
        * Checks if a point is inside a rectangle
        *@param pP - point to check
        *@param pR - rectangle to check against
        *@return 1 if point is inside the rectangle, otherwise 0
        */
        int csrInsideRect(const CSR_Vector2* pP, const CSR_Rect* pR);

        /**
        * Checks if a point is inside a circle
        *@param pP - point to check
        *@param pC - circle to check against
        *@return 1 if point is inside the circle, otherwise 0
        */
        int csrInsideCircle(const CSR_Vector2* pP, const CSR_Circle* pC);

        /**
        * Checks if a point is inside a box
        *@param pP - point to check
        *@param pB - box to check against
        *@return 1 if point is inside the box, otherwise 0
        */
        int csrInsideBox(const CSR_Vector3* pP, const CSR_Box* pB);

        /**
        * Checks if a point is inside a sphere
        *@param pP - point to check
        *@param pS - sphere to check against
        *@return 1 if point is inside the sphere, otherwise 0
        */
        int csrInsideSphere(const CSR_Vector3* pP, const CSR_Sphere* pS);

        //-------------------------------------------------------------------
        // Intersection checks
        //-------------------------------------------------------------------

        /**
        * Checks if a geometric 2D figure intersects another geometric 2D figure
        *@param pFigure1 - first geometric figure to check
        *@param pFigure2 - second geometric figure to check against
        *@param[out] pP1 - first resulting point where the figures intersect (if any), ignored if 0
        *@param[out] pP2 - second resulting point where the figures intersect (if any), ignored if 0
        *@return 1 if geometric figure intersect, otherwise 0
        *@note Some geometric figures may be unsupported. In this case the result is always 0
        */
        int csrIntersect2(const CSR_Figure2* pFigure1,
                          const CSR_Figure2* pFigure2,
                                CSR_Vector2* pR1,
                                CSR_Vector2* pR2);

        /**
        * Checks if a geometric 3D figure intersects another geometric 3D figure
        *@param pFigure1 - first geometric figure to check
        *@param pFigure2 - second geometric figure to check against
        *@param[out] pR1 - first resulting point where the figures intersect (if any), ignored if 0
        *@param[out] pR2 - second resulting point where the figures intersect (if any), ignored if 0
        *@param[out] pR3 - plane resulting from intersection (if any), ignored if 0
        *@return 1 if geometric figure intersect, otherwise 0
        *@note Some geometric figures may be unsupported. In this case the result is always 0
        */
        int csrIntersect3(const CSR_Figure3* pFigure1,
                          const CSR_Figure3* pFigure2,
                                CSR_Vector3* pR1,
                                CSR_Vector3* pR2,
                                CSR_Plane*   pR3);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Geometry.c"
#endif

#endif
