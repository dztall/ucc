/*****************************************************************************
 * ==> Minimal geometry library ---------------------------------------------*
 *****************************************************************************
 * Description : Minimal geometry library                                    *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015, this file is part of the Minimal API. You are free    *
 *               to copy or redistribute this file, modify it, or use it for *
 *               your own projects, commercial or not                        *
 *****************************************************************************/

#ifndef MiniGeometryH
#define MiniGeometryH

// std
#include <stdlib.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

/**
* Euclidean 3D vector
*/
typedef struct
{
    float m_X;
    float m_Y;
} MG_Vector2;

/**
* Euclidean 3D vector
*/
typedef struct
{
    float m_X; // x coordinate for the 3D vector
    float m_Y; // y coordinate for the 3D vector
    float m_Z; // z coordinate for the 3D vector
} MG_Vector3;

/**
* Euclidean plane
*/
typedef struct
{
    float m_A;
    float m_B;
    float m_C;
    float m_D;
} MG_Plane;

/**
* Euclidean ray
*/
typedef struct
{
    MG_Vector3 m_Pos;
    MG_Vector3 m_Dir;
    MG_Vector3 m_InvDir;
} MG_Ray;

/**
* 4x4 matrix
*/
typedef struct
{
    float m_Table[4][4];
} MG_Matrix;

/**
* 2D point
*/
typedef struct
{
    float m_X;
    float m_Y;
} MG_Point;

/**
* 2D size
*/
typedef struct
{
    float m_Width;
    float m_Height;
} MG_Size;

/**
* 2D circle
*/
typedef struct
{
    MG_Point m_Pos;
    float    m_Radius;
} MG_Circle;

/**
* 2D rectangle
*/
typedef struct
{
    MG_Point m_Pos;  // rect left and top corner position
    MG_Size  m_Size;
} MG_Rect;

/**
* 3D aligned-axis box
*/
typedef struct
{
    MG_Vector3 m_Min;
    MG_Vector3 m_Max;
} MG_Box;

//-----------------------------------------------------------------------------
// Basic vector manipulation
//-----------------------------------------------------------------------------

/**
* Copies vector content to another vector
*@param pS - source vector to copy from
*@param pD - destination vector to copy to
*/
void Copy(const MG_Vector3* pS, MG_Vector3* pD)
{
    pD->m_X = pS->m_X;
    pD->m_Y = pS->m_Y;
    pD->m_Z = pS->m_Z;
}

/**
* Subtracts vector contents
*@param pV1 - vector that contains value to be subtracted
*@param pV2 - vector to subtract to
*@param[out] pR - resulting vector
*/
void Sub(const MG_Vector3* pV1, const MG_Vector3* pV2, MG_Vector3* pR)
{
    pR->m_X = pV1->m_X - pV2->m_X;
    pR->m_Y = pV1->m_Y - pV2->m_Y;
    pR->m_Z = pV1->m_Z - pV2->m_Z;
}

/**
* Calculates vector length
*@param pV - vector to calculate length
*@param[out] pR - vector length
*/
void Length(const MG_Vector3* pV, float* pR)
{
    *pR = sqrt((pV->m_X * pV->m_X) + (pV->m_Y * pV->m_Y) + (pV->m_Z * pV->m_Z));
}

/**
* Normalizes vector
*@param pV - vector to normalize
*@param[out] pR - normailzed vector
*/
void Normalize(const MG_Vector3* pV, MG_Vector3* pR)
{
    float len;

    // calculate vector length
    Length(pV, &len);

    // no vector length?
    if (len == 0.0f)
    {
        pR->m_X = 0.0f;
        pR->m_Y = 0.0f;
        pR->m_Z = 0.0f;
        return;
    }

    // normalize vector (thus values will always be between 0.0f, and 1.0f)
    pR->m_X = (pV->m_X / len);
    pR->m_Y = (pV->m_Y / len);
    pR->m_Z = (pV->m_Z / len);
}

/**
* Calculates cross product between 2 vectors
*@param pV1 - first vector
*@param pV2 - second vector
*@param[out] pR - resulting vector
*/
void Cross(const MG_Vector3* pV1, const MG_Vector3* pV2, MG_Vector3* pR)
{
    pR->m_X = (pV1->m_Y * pV2->m_Z) - (pV2->m_Y * pV1->m_Z);
    pR->m_Y = (pV1->m_Z * pV2->m_X) - (pV2->m_Z * pV1->m_X);
    pR->m_Z = (pV1->m_X * pV2->m_Y) - (pV2->m_X * pV1->m_Y);
}

/**
* Calculates dot product between 2 vectors
*@param pV1 - first vector
*@param pV2 - second vector
*@param[out] pR - resulting angle
*/
void Dot(const MG_Vector3* pV1, const MG_Vector3* pV2, float* pR)
{
    *pR = ((pV1->m_X * pV2->m_X) + (pV1->m_Y * pV2->m_Y) + (pV1->m_Z * pV2->m_Z));
}

//-----------------------------------------------------------------------------
// Basic plane manipulation
//-----------------------------------------------------------------------------

/**
* Creates a plane from a point and a normal
*@param pP - point
*@param pN - normal
*@param[out] pR - resulting plane
*/
void PlaneFromPointNormal(const MG_Vector3* pP, const MG_Vector3* pN, MG_Plane* pR)
{
    float d;

    // the a, b, and c components are only the normal of the plane
    pR->m_A = pN->m_X;
    pR->m_B = pN->m_Y;
    pR->m_C = pN->m_Z;

    // calculate plane d component using the aX + bY + cZ + d = 0 formula
    Dot(pN, pP, &d);
    pR->m_D = -d;
}

/**
* Creates a plane from 3 vertex
*@param pV1 - first vector
*@param pV2 - second vector
*@param pV3 - third vector
*@param[out] pR - resulting plane
*/
void PlaneFromPoints(const MG_Vector3* pV1,
                     const MG_Vector3* pV2,
                     const MG_Vector3* pV3,
                           MG_Plane*   pR)
{
    MG_Vector3 e1;
    MG_Vector3 e2;
    MG_Vector3 normal;

    // calculate edge vectors
    Sub(pV2, pV1, &e1);
    Sub(pV3, pV1, &e2);

    // calculate the normal of the plane
    Cross(&e1, &e2, &normal);
    Normalize(&normal, &normal);

    // calculate and return the plane
    PlaneFromPointNormal(pV1, &normal, pR);
}

//-----------------------------------------------------------------------------
// Matrix functions
//-----------------------------------------------------------------------------

/**
* Gets matrix identity
*@param[out] pM - resulting identity matrix
*/
void GetIdentity(MG_Matrix* pM)
{
    pM->m_Table[0][0] = 1.0f; pM->m_Table[1][0] = 0.0f; pM->m_Table[2][0] = 0.0f; pM->m_Table[3][0] = 0.0f;
    pM->m_Table[0][1] = 0.0f; pM->m_Table[1][1] = 1.0f; pM->m_Table[2][1] = 0.0f; pM->m_Table[3][1] = 0.0f;
    pM->m_Table[0][2] = 0.0f; pM->m_Table[1][2] = 0.0f; pM->m_Table[2][2] = 1.0f; pM->m_Table[3][2] = 0.0f;
    pM->m_Table[0][3] = 0.0f; pM->m_Table[1][3] = 0.0f; pM->m_Table[2][3] = 0.0f; pM->m_Table[3][3] = 1.0f;
}

/**
* Gets orthogonal matrix
*@param pLeft - left view rect edge
*@param pRight - right view rect edge
*@param pBottom - bottom view rect edge
*@param pTop - top view rect edge
*@param pZNear - z near clipping value
*@param pZFar - z far clipping value
*@param[out] pM - resulting orthogonal matrix
*/
void GetOrtho(float*     pLeft,
              float*     pRight,
              float*     pBottom,
              float*     pTop,
              float*     pZNear,
              float*     pZFar,
              MG_Matrix* pM)
{
    // calculate matrix component values
    float pfn = *pZFar  + *pZNear;
    float mnf = *pZNear - *pZFar;
    float prl = *pRight + *pLeft;
    float mrl = *pRight - *pLeft;
    float ptb = *pTop   + *pBottom;
    float mtb = *pTop   - *pBottom;
    float mlr = -mrl;
    float mbt = -mtb;

    // build matrix
    pM->m_Table[0][0] = 2.0f / mrl; pM->m_Table[1][0] = 0.0f;       pM->m_Table[2][0] = 0.0f;       pM->m_Table[3][0] = prl / mlr;
    pM->m_Table[0][1] = 0.0f;       pM->m_Table[1][1] = 2.0f / mtb; pM->m_Table[2][1] = 0.0f;       pM->m_Table[3][1] = ptb / mbt;
    pM->m_Table[0][2] = 0.0f;       pM->m_Table[1][2] = 0.0f;       pM->m_Table[2][2] = 2.0f / mnf; pM->m_Table[3][2] = pfn / mnf;
    pM->m_Table[0][3] = 0.0f;       pM->m_Table[1][3] = 0.0f;       pM->m_Table[2][3] = 0.0f;       pM->m_Table[3][3] = 1.0f;
}

/**
* Gets frustrum matrix
*@param pLeft - left view rect edge
*@param pRight - right view rect edge
*@param pBottom - bottom view rect edge
*@param pTop - top view rect edge
*@param pZNear - z near clipping value
*@param pZFar - z far clipping value
*@param[out] pM - resulting frustrum matrix
*/
void GetFrustum(float*     pLeft,
                float*     pRight,
                float*     pBottom,
                float*     pTop,
                float*     pZNear,
                float*     pZFar,
                MG_Matrix* pM)
{
    // OpenGL specifications                                     can be rewritten as
    // |  2n/(r-l)    0        (r+l)/(r-l)       0      |        |  2n/(r-l)    0        (r+l)/(r-l)       0      |
    // |     0     2n/(t-b)    (t+b)/(t-b)       0      |   =>   |     0     2n/(t-b)    (t+b)/(t-b)       0      |
    // |     0        0       -(f+n)/(f-n)  -2fn/(f-n)  |        |     0        0        (f+n)/(n-f)   2fn/(n-f)  |
    // |     0        0            -1            0      |        |     0        0            -1            0      |
    // invalid for n <= 0, f <= 0, l = r, b = t, or n = f

    // calculate matrix component values
    float x2n  = 2.0f    * *pZNear;
    float x2nf = 2.0f    * *pZNear * *pZFar;
    float pfn  = *pZFar  + *pZNear;
    float mnf  = *pZNear - *pZFar;
    float prl  = *pRight + *pLeft;
    float mrl  = *pRight - *pLeft;
    float ptb  = *pTop   + *pBottom;
    float mtb  = *pTop   - *pBottom;

    // build matrix
    pM->m_Table[0][0] = x2n / mrl; pM->m_Table[1][0] = 0.0f;      pM->m_Table[2][0] =  prl / mrl; pM->m_Table[3][0] = 0.0f;
    pM->m_Table[0][1] = 0.0f;      pM->m_Table[1][1] = x2n / mtb; pM->m_Table[2][1] =  ptb / mtb; pM->m_Table[3][1] = 0.0f;
    pM->m_Table[0][2] = 0.0f;      pM->m_Table[1][2] = 0.0f;      pM->m_Table[2][2] =  pfn / mnf; pM->m_Table[3][2] = x2nf / mnf;
    pM->m_Table[0][3] = 0.0f;      pM->m_Table[1][3] = 0.0f;      pM->m_Table[2][3] = -1.0f;      pM->m_Table[3][3] = 0.0f;
}

/**
* Gets perspective matrix
*@param pFovyDeg - field of view angle in radian
*@param pAspect - aspect ratio
*@param pZNear - z near clipping value
*@param pZFar - z far clipping value
*@param[out] pM - resulting perspective matrix
*/
void GetPerspective(float*     pFovyDeg,
                    float*     pAspect,
                    float*     pZNear,
                    float*     pZFar,
                    MG_Matrix* pM)
{
    // OpenGL specifications                             can be rewritten as
    // |    c/a    0        0            0       |       |    c/a    0         0           0      |
    // |     0     c        0            0       |  ==>  |     0     c         0           0      |
    // |     0     0   -(f+n)/(f-n)  -2nf/(f+n)  |       |     0     0    (f+n)/(n-f)  2nf/(n-f)  |
    // |     0     0       -1            0       |       |     0     0        -1           0      |
    // invalid for n <= 0, f <= 0, a = 0, or n = f

    // calculate field of view, and sinus and cosinus values
    const float halfFovyRad = 3.1415f * *pFovyDeg / 360.0f;
    const float si          = sinf(halfFovyRad);
    const float co          = cosf(halfFovyRad);

    // calculate matrix component values
    const float c    = co / si; // cotangent
    const float a    = *pAspect;
    const float x2nf = 2.0f  * *pZNear * *pZFar;
    const float pfn  = *pZFar  + *pZNear;
    const float mnf  = *pZNear - *pZFar;

    // build matrix
    pM->m_Table[0][0] = c / a; pM->m_Table[1][0] = 0.0f; pM->m_Table[2][0] =  0.0f;      pM->m_Table[3][0] = 0.0f;
    pM->m_Table[0][1] = 0.0f;  pM->m_Table[1][1] = c;    pM->m_Table[2][1] =  0.0f;      pM->m_Table[3][1] = 0.0f;
    pM->m_Table[0][2] = 0.0f;  pM->m_Table[1][2] = 0.0f; pM->m_Table[2][2] =  pfn / mnf; pM->m_Table[3][2] = x2nf / mnf;
    pM->m_Table[0][3] = 0.0f;  pM->m_Table[1][3] = 0.0f; pM->m_Table[2][3] = -1.0f;      pM->m_Table[3][3] = 0.0f;
}

/**
* Gets translation matrix
*@param pT - translation vector
*@param[out] pM - resulting translation matrix
*/
void GetTranslateMatrix(MG_Vector3* pT, MG_Matrix* pM)
{
    GetIdentity(pM);
    pM->m_Table[3][0] += pM->m_Table[0][0] * pT->m_X + pM->m_Table[1][0] * pT->m_Y + pM->m_Table[2][0] * pT->m_Z;
    pM->m_Table[3][1] += pM->m_Table[0][1] * pT->m_X + pM->m_Table[1][1] * pT->m_Y + pM->m_Table[2][1] * pT->m_Z;
    pM->m_Table[3][2] += pM->m_Table[0][2] * pT->m_X + pM->m_Table[1][2] * pT->m_Y + pM->m_Table[2][2] * pT->m_Z;
    pM->m_Table[3][3] += pM->m_Table[0][3] * pT->m_X + pM->m_Table[1][3] * pT->m_Y + pM->m_Table[2][3] * pT->m_Z;
}

/**
* Gets rotation matrix
*@param pAngle - rotation angle, in radian
*@param pAxis - rotation axis
*@param[out] pM - resulting orthogonal matrix
*/
void GetRotateMatrix(float* pAngle, MG_Vector3* pAxis, MG_Matrix* pM)
{
    // calculate sinus, cosinus and inverted cosinus values
    float c  = cosf(*pAngle);
    float s  = sinf(*pAngle);
    float ic = (1.0f - c);

    // create rotation matrix
    GetIdentity(pM);
    pM->m_Table[0][0] = (ic * pAxis->m_X * pAxis->m_X) + c;
    pM->m_Table[1][0] = (ic * pAxis->m_X * pAxis->m_Y) - (s * pAxis->m_Z);
    pM->m_Table[2][0] = (ic * pAxis->m_X * pAxis->m_Z) + (s * pAxis->m_Y);
    pM->m_Table[0][1] = (ic * pAxis->m_Y * pAxis->m_X) + (s * pAxis->m_Z);
    pM->m_Table[1][1] = (ic * pAxis->m_Y * pAxis->m_Y) + c;
    pM->m_Table[2][1] = (ic * pAxis->m_Y * pAxis->m_Z) - (s * pAxis->m_X);
    pM->m_Table[0][2] = (ic * pAxis->m_Z * pAxis->m_X) - (s * pAxis->m_Y);
    pM->m_Table[1][2] = (ic * pAxis->m_Z * pAxis->m_Y) + (s * pAxis->m_X);
    pM->m_Table[2][2] = (ic * pAxis->m_Z * pAxis->m_Z) + c;
}

/**
* Gets scale matrix
*@param pFactor - scale factor
*@param[out] pM - resulting scale matrix
*/
void GetScaleMatrix(MG_Vector3* pFactor, MG_Matrix* pM)
{
    GetIdentity(pM);
    pM->m_Table[0][0] *= pFactor->m_X; pM->m_Table[1][0] *= pFactor->m_Y; pM->m_Table[2][0] *= pFactor->m_Z;
    pM->m_Table[0][1] *= pFactor->m_X; pM->m_Table[1][1] *= pFactor->m_Y; pM->m_Table[2][1] *= pFactor->m_Z;
    pM->m_Table[0][2] *= pFactor->m_X; pM->m_Table[1][2] *= pFactor->m_Y; pM->m_Table[2][2] *= pFactor->m_Z;
    pM->m_Table[0][3] *= pFactor->m_X; pM->m_Table[1][3] *= pFactor->m_Y; pM->m_Table[2][3] *= pFactor->m_Z;
}

/**
* Multiplies matrix by another matrix
*@param pM1 - first matrix to multiply
*@param pM2 - second matrix to multiply with
*@param[out] pM - resulting orthogonal matrix
*/
void MatrixMultiply(MG_Matrix* pM1, MG_Matrix* pM2, MG_Matrix* pR)
{
    int i;
    int j;

    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            pR->m_Table[i][j] = pM1->m_Table[i][0] * pM2->m_Table[0][j] +
                                pM1->m_Table[i][1] * pM2->m_Table[1][j] +
                                pM1->m_Table[i][2] * pM2->m_Table[2][j] +
                                pM1->m_Table[i][3] * pM2->m_Table[3][j];
}

/**
* Applies matrix to a vector
*@param pM - matrix to apply
*@param pV - vector on which matrix shoudl be applied
*@param[out] pR - resulting vector
*/
void ApplyMatrixToVector(MG_Matrix* pM, MG_Vector3* pV, MG_Vector3* pR)
{
    pR->m_X = (pV->m_X * pM->m_Table[0][0] +
               pV->m_Y * pM->m_Table[1][0] +
               pV->m_Z * pM->m_Table[2][0] +
                         pM->m_Table[3][0]);
    pR->m_Y = (pV->m_X * pM->m_Table[0][1] +
               pV->m_Y * pM->m_Table[1][1] +
               pV->m_Z * pM->m_Table[2][1] +
                         pM->m_Table[3][1]);
    pR->m_Z = (pV->m_X * pM->m_Table[0][2] +
               pV->m_Y * pM->m_Table[1][2] +
               pV->m_Z * pM->m_Table[2][2] +
                         pM->m_Table[3][2]);
}

#endif // MiniGeometryH
