/*****************************************************************************
 * ==> Minimal geometry library ---------------------------------------------*
 *****************************************************************************
 * Description : Minimal geometry library                                    *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not                 *
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
* Euclidean 2D vector
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
    float m_Table[4][4]; // 4x4 matrix array
} MG_Matrix;

/**
* Quaternion
*/
typedef struct
{
    float m_X;
    float m_Y;
    float m_Z;
    float m_W;
} MG_Quaternion;

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
* Normalizes the vector
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
*@param[out] pR - resulting identity matrix
*/
void GetIdentity(MG_Matrix* pR)
{
    pR->m_Table[0][0] = 1.0f; pR->m_Table[1][0] = 0.0f; pR->m_Table[2][0] = 0.0f; pR->m_Table[3][0] = 0.0f;
    pR->m_Table[0][1] = 0.0f; pR->m_Table[1][1] = 1.0f; pR->m_Table[2][1] = 0.0f; pR->m_Table[3][1] = 0.0f;
    pR->m_Table[0][2] = 0.0f; pR->m_Table[1][2] = 0.0f; pR->m_Table[2][2] = 1.0f; pR->m_Table[3][2] = 0.0f;
    pR->m_Table[0][3] = 0.0f; pR->m_Table[1][3] = 0.0f; pR->m_Table[2][3] = 0.0f; pR->m_Table[3][3] = 1.0f;
}

/**
* Gets orthogonal matrix
*@param pLeft - left view rect edge
*@param pRight - right view rect edge
*@param pBottom - bottom view rect edge
*@param pTop - top view rect edge
*@param pZNear - z near clipping value
*@param pZFar - z far clipping value
*@param[out] pR - resulting orthogonal matrix
*/
void GetOrtho(float*     pLeft,
              float*     pRight,
              float*     pBottom,
              float*     pTop,
              float*     pZNear,
              float*     pZFar,
              MG_Matrix* pR)
{
    // OpenGL specifications                                 can be rewritten as
    // |  2/(r-l)  0         0        -(r+l)/(r-l)  |        |  2/(r-l)  0        0        (r+l)/(l-r)  |
    // |  0        2/(t-b)   0        -(t+b)/(t-b)  |   =>   |  0        2/(t-b)  0        (t+b)/(b-t)  |
    // |  0        0        -2/(f-n)  -(f+n)/(f-n)  |        |  0        0        2/(n-f)  (f+n)/(n-f)  |
    // |  0        0         0         1            |        |  0        0        0        1            |

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
    pR->m_Table[0][0] = 2.0f / mrl; pR->m_Table[1][0] = 0.0f;       pR->m_Table[2][0] = 0.0f;       pR->m_Table[3][0] = prl / mlr;
    pR->m_Table[0][1] = 0.0f;       pR->m_Table[1][1] = 2.0f / mtb; pR->m_Table[2][1] = 0.0f;       pR->m_Table[3][1] = ptb / mbt;
    pR->m_Table[0][2] = 0.0f;       pR->m_Table[1][2] = 0.0f;       pR->m_Table[2][2] = 2.0f / mnf; pR->m_Table[3][2] = pfn / mnf;
    pR->m_Table[0][3] = 0.0f;       pR->m_Table[1][3] = 0.0f;       pR->m_Table[2][3] = 0.0f;       pR->m_Table[3][3] = 1.0f;
}

/**
* Gets frustrum matrix
*@param pLeft - left view rect edge
*@param pRight - right view rect edge
*@param pBottom - bottom view rect edge
*@param pTop - top view rect edge
*@param pZNear - z near clipping value
*@param pZFar - z far clipping value
*@param[out] pR - resulting frustrum matrix
*/
void GetFrustum(float*     pLeft,
                float*     pRight,
                float*     pBottom,
                float*     pTop,
                float*     pZNear,
                float*     pZFar,
                MG_Matrix* pR)
{
    // OpenGL specifications                                          can be rewritten as
    // |  2n/(r-l)     0             0             0         |        |  2n/(r-l)     0             0            0         |
    // |  0            2n/(t-b)      0             0         |   =>   |  0            2n/(t-b)      0            0         |
    // |  (r+l)/(r-l)  (t+b)/(t-b)  -(f+n)/(f-n)  -2fn/(f-n) |        |  (r+l)/(r-l)  (t+b)/(t-b)   (f+n)/(n-f)  2fn/(n-f) |
    // |  0            0            -1             0         |        |  0            0            -1            0         |
    // invalid for n <= 0, f <= 0, l = r, b = t, or n = f

    // calculate matrix component values
    float x2n  =  2.0f   * *pZNear;
    float x2nf =  x2n    * *pZFar;
    float pfn  = *pZFar  + *pZNear;
    float mnf  = *pZNear - *pZFar;
    float prl  = *pRight + *pLeft;
    float mrl  = *pRight - *pLeft;
    float ptb  = *pTop   + *pBottom;
    float mtb  = *pTop   - *pBottom;

    // build matrix
    pR->m_Table[0][0] = x2n / mrl; pR->m_Table[1][0] = 0.0f;      pR->m_Table[2][0] =  0.0f;      pR->m_Table[3][0] = 0.0f;
    pR->m_Table[0][1] = 0.0f;      pR->m_Table[1][1] = x2n / mtb; pR->m_Table[2][1] =  0.0f;      pR->m_Table[3][1] = 0.0f;
    pR->m_Table[0][2] = prl / mrl; pR->m_Table[1][2] = ptb / mtb; pR->m_Table[2][2] =  pfn / mnf; pR->m_Table[3][2] = x2nf / mnf;
    pR->m_Table[0][3] = 0.0f;      pR->m_Table[1][3] = 0.0f;      pR->m_Table[2][3] = -1.0f;      pR->m_Table[3][3] = 0.0f;
}

/**
* Gets perspective matrix
*@param pFovyDeg - field of view angle in radian
*@param pAspect - aspect ratio
*@param pZNear - z near clipping value
*@param pZFar - z far clipping value
*@param[out] pR - resulting perspective matrix
*/
void GetPerspective(float*     pFovyDeg,
                    float*     pAspect,
                    float*     pZNear,
                    float*     pZFar,
                    MG_Matrix* pR)
{
    const float maxY    = *pZNear *  tanf(*pFovyDeg * M_PI / 360.0f);
    const float maxX    =  maxY   * *pAspect;
    const float negMaxY = -maxY;
    const float negMaxX = -maxX;

    GetFrustum(&negMaxX, &maxX, &negMaxY, &maxY, pZNear, pZFar, pR);
}

/**
* Gets translation matrix
*@param pT - translation vector
*@param[out] pR - resulting translation matrix
*/
void GetTranslateMatrix(MG_Vector3* pT, MG_Matrix* pR)
{
    GetIdentity(pR);
    pR->m_Table[3][0] += pR->m_Table[0][0] * pT->m_X + pR->m_Table[1][0] * pT->m_Y + pR->m_Table[2][0] * pT->m_Z;
    pR->m_Table[3][1] += pR->m_Table[0][1] * pT->m_X + pR->m_Table[1][1] * pT->m_Y + pR->m_Table[2][1] * pT->m_Z;
    pR->m_Table[3][2] += pR->m_Table[0][2] * pT->m_X + pR->m_Table[1][2] * pT->m_Y + pR->m_Table[2][2] * pT->m_Z;
    pR->m_Table[3][3] += pR->m_Table[0][3] * pT->m_X + pR->m_Table[1][3] * pT->m_Y + pR->m_Table[2][3] * pT->m_Z;
}

/**
* Gets rotation matrix
*@param pAngle - rotation angle, in radian
*@param pAxis - rotation axis
*@param[out] pR - resulting orthogonal matrix
*/
void GetRotateMatrix(float* pAngle, MG_Vector3* pAxis, MG_Matrix* pR)
{
    // calculate sinus, cosinus and inverted cosinus values
    float c  = cosf(*pAngle);
    float s  = sinf(*pAngle);
    float ic = (1.0f - c);

    // create rotation matrix
    GetIdentity(pR);
    pR->m_Table[0][0] = (ic * pAxis->m_X * pAxis->m_X) +  c;
    pR->m_Table[1][0] = (ic * pAxis->m_X * pAxis->m_Y) - (s * pAxis->m_Z);
    pR->m_Table[2][0] = (ic * pAxis->m_X * pAxis->m_Z) + (s * pAxis->m_Y);
    pR->m_Table[0][1] = (ic * pAxis->m_Y * pAxis->m_X) + (s * pAxis->m_Z);
    pR->m_Table[1][1] = (ic * pAxis->m_Y * pAxis->m_Y) +  c;
    pR->m_Table[2][1] = (ic * pAxis->m_Y * pAxis->m_Z) - (s * pAxis->m_X);
    pR->m_Table[0][2] = (ic * pAxis->m_Z * pAxis->m_X) - (s * pAxis->m_Y);
    pR->m_Table[1][2] = (ic * pAxis->m_Z * pAxis->m_Y) + (s * pAxis->m_X);
    pR->m_Table[2][2] = (ic * pAxis->m_Z * pAxis->m_Z) +  c;
}

/**
* Gets scale matrix
*@param pFactor - scale factor
*@param[out] pR - resulting scale matrix
*/
void GetScaleMatrix(MG_Vector3* pFactor, MG_Matrix* pR)
{
    GetIdentity(pR);
    pR->m_Table[0][0] *= pFactor->m_X; pR->m_Table[1][0] *= pFactor->m_Y; pR->m_Table[2][0] *= pFactor->m_Z;
    pR->m_Table[0][1] *= pFactor->m_X; pR->m_Table[1][1] *= pFactor->m_Y; pR->m_Table[2][1] *= pFactor->m_Z;
    pR->m_Table[0][2] *= pFactor->m_X; pR->m_Table[1][2] *= pFactor->m_Y; pR->m_Table[2][2] *= pFactor->m_Z;
    pR->m_Table[0][3] *= pFactor->m_X; pR->m_Table[1][3] *= pFactor->m_Y; pR->m_Table[2][3] *= pFactor->m_Z;
}

/**
* Multiplies matrix by another matrix
*@param pM1 - first matrix to multiply
*@param pM2 - second matrix to multiply with
*@param[out] pR - resulting orthogonal matrix
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
* Inverses a matrix
*@param pM - matrix to inverse
*@param[out] pR - inversed matrix
*@param[out] pDeterminant - matrix determinant
*/
void Inverse(const MG_Matrix* pM, MG_Matrix* pR, float* pDeterminant)
{
    float invDet;
    float t[3];
    float v[16];
    int   i;
    int   j;

    t[0] = pM->m_Table[2][2] * pM->m_Table[3][3] - pM->m_Table[2][3] * pM->m_Table[3][2];
    t[1] = pM->m_Table[1][2] * pM->m_Table[3][3] - pM->m_Table[1][3] * pM->m_Table[3][2];
    t[2] = pM->m_Table[1][2] * pM->m_Table[2][3] - pM->m_Table[1][3] * pM->m_Table[2][2];

    v[0] =  pM->m_Table[1][1] * t[0] - pM->m_Table[2][1] * t[1] + pM->m_Table[3][1] * t[2];
    v[4] = -pM->m_Table[1][0] * t[0] + pM->m_Table[2][0] * t[1] - pM->m_Table[3][0] * t[2];

    t[0] =  pM->m_Table[1][0] * pM->m_Table[2][1] - pM->m_Table[2][0] * pM->m_Table[1][1];
    t[1] =  pM->m_Table[1][0] * pM->m_Table[3][1] - pM->m_Table[3][0] * pM->m_Table[1][1];
    t[2] =  pM->m_Table[2][0] * pM->m_Table[3][1] - pM->m_Table[3][0] * pM->m_Table[2][1];

    v[8]  =  pM->m_Table[3][3] * t[0] - pM->m_Table[2][3] * t[1] + pM->m_Table[1][3] * t[2];
    v[12] = -pM->m_Table[3][2] * t[0] + pM->m_Table[2][2] * t[1] - pM->m_Table[1][2] * t[2];

    *pDeterminant = pM->m_Table[0][0] * v[0] +
                    pM->m_Table[0][1] * v[4] +
                    pM->m_Table[0][2] * v[8] +
                    pM->m_Table[0][3] * v[12];

    if (*pDeterminant == 0.0)
        return;

    t[0] = pM->m_Table[2][2] * pM->m_Table[3][3] - pM->m_Table[2][3] * pM->m_Table[3][2];
    t[1] = pM->m_Table[0][2] * pM->m_Table[3][3] - pM->m_Table[0][3] * pM->m_Table[3][2];
    t[2] = pM->m_Table[0][2] * pM->m_Table[2][3] - pM->m_Table[0][3] * pM->m_Table[2][2];

    v[1] = -pM->m_Table[0][1] * t[0] + pM->m_Table[2][1] * t[1] - pM->m_Table[3][1] * t[2];
    v[5] =  pM->m_Table[0][0] * t[0] - pM->m_Table[2][0] * t[1] + pM->m_Table[3][0] * t[2];

    t[0] = pM->m_Table[0][0] * pM->m_Table[2][1] - pM->m_Table[2][0] * pM->m_Table[0][1];
    t[1] = pM->m_Table[3][0] * pM->m_Table[0][1] - pM->m_Table[0][0] * pM->m_Table[3][1];
    t[2] = pM->m_Table[2][0] * pM->m_Table[3][1] - pM->m_Table[3][0] * pM->m_Table[2][1];

    v[9]  = -pM->m_Table[3][3] * t[0] - pM->m_Table[2][3] * t[1] - pM->m_Table[0][3] * t[2];
    v[13] =  pM->m_Table[3][2] * t[0] + pM->m_Table[2][2] * t[1] + pM->m_Table[0][2] * t[2];

    t[0] = pM->m_Table[1][2] * pM->m_Table[3][3] - pM->m_Table[1][3] * pM->m_Table[3][2];
    t[1] = pM->m_Table[0][2] * pM->m_Table[3][3] - pM->m_Table[0][3] * pM->m_Table[3][2];
    t[2] = pM->m_Table[0][2] * pM->m_Table[1][3] - pM->m_Table[0][3] * pM->m_Table[1][2];

    v[2] =  pM->m_Table[0][1] * t[0] - pM->m_Table[1][1] * t[1] + pM->m_Table[3][1] * t[2];
    v[6] = -pM->m_Table[0][0] * t[0] + pM->m_Table[1][0] * t[1] - pM->m_Table[3][0] * t[2];

    t[0] = pM->m_Table[0][0] * pM->m_Table[1][1] - pM->m_Table[1][0] * pM->m_Table[0][1];
    t[1] = pM->m_Table[3][0] * pM->m_Table[0][1] - pM->m_Table[0][0] * pM->m_Table[3][1];
    t[2] = pM->m_Table[1][0] * pM->m_Table[3][1] - pM->m_Table[3][0] * pM->m_Table[1][1];

    v[10] =  pM->m_Table[3][3] * t[0] + pM->m_Table[1][3] * t[1] + pM->m_Table[0][3] * t[2];
    v[14] = -pM->m_Table[3][2] * t[0] - pM->m_Table[1][2] * t[1] - pM->m_Table[0][2] * t[2];

    t[0] = pM->m_Table[1][2] * pM->m_Table[2][3] - pM->m_Table[1][3] * pM->m_Table[2][2];
    t[1] = pM->m_Table[0][2] * pM->m_Table[2][3] - pM->m_Table[0][3] * pM->m_Table[2][2];
    t[2] = pM->m_Table[0][2] * pM->m_Table[1][3] - pM->m_Table[0][3] * pM->m_Table[1][2];

    v[3] = -pM->m_Table[0][1] * t[0] + pM->m_Table[1][1] * t[1] - pM->m_Table[2][1] * t[2];
    v[7] =  pM->m_Table[0][0] * t[0] - pM->m_Table[1][0] * t[1] + pM->m_Table[2][0] * t[2];

    v[11] = -pM->m_Table[0][0] * (pM->m_Table[1][1] * pM->m_Table[2][3] - pM->m_Table[1][3] * pM->m_Table[2][1]) +
             pM->m_Table[1][0] * (pM->m_Table[0][1] * pM->m_Table[2][3] - pM->m_Table[0][3] * pM->m_Table[2][1]) -
             pM->m_Table[2][0] * (pM->m_Table[0][1] * pM->m_Table[1][3] - pM->m_Table[0][3] * pM->m_Table[1][1]);

    v[15] = pM->m_Table[0][0] * (pM->m_Table[1][1] * pM->m_Table[2][2] - pM->m_Table[1][2] * pM->m_Table[2][1]) -
            pM->m_Table[1][0] * (pM->m_Table[0][1] * pM->m_Table[2][2] - pM->m_Table[0][2] * pM->m_Table[2][1]) +
            pM->m_Table[2][0] * (pM->m_Table[0][1] * pM->m_Table[1][2] - pM->m_Table[0][2] * pM->m_Table[1][1]);

    invDet = 1.0 / *pDeterminant;

    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            pR->m_Table[i][j] = v[4 * i + j] * invDet;
}

/**
* Applies matrix to a vector
*@param pM - matrix to apply
*@param pV - vector on which matrix should be applied
*@param[out] pR - resulting vector
*/
void ApplyMatrixToVector(MG_Matrix* pM, MG_Vector3* pV, MG_Vector3* pR)
{
    pR->m_X = (pV->m_X * pM->m_Table[0][0] + pV->m_Y * pM->m_Table[1][0] + pV->m_Z * pM->m_Table[2][0] + pM->m_Table[3][0]);
    pR->m_Y = (pV->m_X * pM->m_Table[0][1] + pV->m_Y * pM->m_Table[1][1] + pV->m_Z * pM->m_Table[2][1] + pM->m_Table[3][1]);
    pR->m_Z = (pV->m_X * pM->m_Table[0][2] + pV->m_Y * pM->m_Table[1][2] + pV->m_Z * pM->m_Table[2][2] + pM->m_Table[3][2]);
}

/**
* Applies matrix to a normal
*@param pM - matrix to apply
*@param pN - normal on which matrix should be applied
*@param[out] pR - resulting normal
*/
void ApplyMatrixToNormal(MG_Matrix* pM, MG_Vector3* pN, MG_Vector3* pR)
{
    pR->m_X = (pN->m_X * pM->m_Table[0][0] + pN->m_Y * pM->m_Table[1][0] + pN->m_Z * pM->m_Table[2][0]);
    pR->m_Y = (pN->m_X * pM->m_Table[0][1] + pN->m_Y * pM->m_Table[1][1] + pN->m_Z * pM->m_Table[2][1]);
    pR->m_Z = (pN->m_X * pM->m_Table[0][2] + pN->m_Y * pM->m_Table[1][2] + pN->m_Z * pM->m_Table[2][2]);
}

/**
* Unprojects a ray (i.e. transforms it in viewport coordinates)
*@param pProj - projection matrix
*@param pView - view matrix
*@param[in, out] pRayPos - ray position, unprojected ray position on function ends
*@param[in, out] pRayDir - ray direction, unprojected ray direction on function ends
*/
void Unproject(MG_Matrix* pProj, MG_Matrix* pView, MG_Vector3* pRayPos, MG_Vector3* pRayDir)
{
    float      determinant;
    MG_Matrix  invertProj;
    MG_Matrix  invertView;
    MG_Matrix  unprojectMat;
    MG_Vector3 unprojRayPos;
    MG_Vector3 unprojRayDir;

    // unproject the ray to make it in the viewport coordinates
    Inverse(pProj, &invertProj, &determinant);
    Inverse(pView, &invertView, &determinant);
    MatrixMultiply(&invertProj, &invertView, &unprojectMat);
    ApplyMatrixToVector(&unprojectMat, pRayPos, &unprojRayPos);
    ApplyMatrixToVector(&unprojectMat, pRayDir, &unprojRayDir);

    // copy resulting ray position
    pRayPos->m_X = unprojRayPos.m_X;
    pRayPos->m_Y = unprojRayPos.m_Y;
    pRayPos->m_Z = unprojRayPos.m_Z;

    // normalize and copy resulting ray direction
    Normalize(&unprojRayDir, pRayDir);
}

//-----------------------------------------------------------------------------
// Quaternion functions
//-----------------------------------------------------------------------------

/**
* Gets a quaternion from an angle and a vector representing a rotation axis
*@param pAngle - rotation angle
*@param pAxis- rotation axis
*@param[out] pR - resulting quaternion
*/
void QuatFromAxis(float* pAngle, const MG_Vector3* pAxis, MG_Quaternion* pR)
{
    float s = sinf(*pAngle * 0.5f);

    pR->m_X = (pAxis->m_X * s);
    pR->m_Y = (pAxis->m_Y * s);
    pR->m_Z = (pAxis->m_Z * s);
    pR->m_W =  cosf(*pAngle * 0.5f);
}

/**
* Gets a quaternion from Euler angles
*@param pAngleX - rotation angle on x axis
*@param pAngleY - rotation angle on y axis
*@param pAngleZ - rotation angle on z axis
*@param[out] pR - resulting quaternion
*/
void QuatFromEuler(float* pAngleX, float* pAngleY, float* pAngleZ, MG_Quaternion* pR)
{
    float cos_x_2 = cosf(0.5f * *pAngleX);
    float cos_y_2 = cosf(0.5f * *pAngleY);
    float cos_z_2 = cosf(0.5f * *pAngleZ);

    float sin_x_2 = sinf(0.5f * *pAngleX);
    float sin_y_2 = sinf(0.5f * *pAngleY);
    float sin_z_2 = sinf(0.5f * *pAngleZ);

    // create the quaternion
    pR->m_X = cos_z_2 * cos_y_2 * sin_x_2 - sin_z_2 * sin_y_2 * cos_x_2;
    pR->m_Y = cos_z_2 * sin_y_2 * cos_x_2 + sin_z_2 * cos_y_2 * sin_x_2;
    pR->m_Z = sin_z_2 * cos_y_2 * cos_x_2 - cos_z_2 * sin_y_2 * sin_x_2;
    pR->m_W = cos_z_2 * cos_y_2 * cos_x_2 + sin_z_2 * sin_y_2 * sin_x_2;
}

/**
* Gets the squared length of a quaternion
*@param pQ - quaternion for which the norm should be get
*@param[out] pR - the resulting norm
*/
void QuatLengthSquared(const MG_Quaternion* pQ, float* pR)
{
    *pR = ((pQ->m_X * pQ->m_X) +
           (pQ->m_Y * pQ->m_Y) +
           (pQ->m_Z * pQ->m_Z) +
           (pQ->m_W * pQ->m_W));
}

/**
* Gets the quaternion length
*@param pQ - quaternion for which the length should be get
*@param[out] pR - the resulting length
*/
void QuatLength(const MG_Quaternion* pQ, float* pR)
{
    float norm;
    QuatLengthSquared(pQ, &norm);

    *pR = sqrt(norm);
}

/**
* Normalizes the quaternion
*@param pV - vector to normalize
*@param[out] pR - normailzed vector
*/
void QuatNormalize(const MG_Quaternion* pQ, MG_Quaternion* pR)
{
    float len;
    QuatLength(pQ, &len);

    if (len == 0.0f)
    {
        pR->m_X = 0.0f;
        pR->m_Y = 0.0f;
        pR->m_Z = 0.0f;
        pR->m_W = 0.0f;
        return;
    }

    pR->m_X = (pR->m_X / len);
    pR->m_Y = (pR->m_Y / len);
    pR->m_Z = (pR->m_Z / len);
    pR->m_W = (pR->m_W / len);
}

/**
* Calculates dot product between 2 quaternions
*@param pQ1 - first quaternion
*@param pQ2 - second quaternion
*@param[out] pR - resulting angle
*/
void QuatDot(const MG_Quaternion* pQ1, const MG_Quaternion* pQ2, float* pR)
{
    *pR = ((pQ1->m_X * pQ2->m_X) +
           (pQ1->m_Y * pQ2->m_Y) +
           (pQ1->m_Z * pQ2->m_Z) +
           (pQ1->m_W * pQ2->m_W));
}

/**
* Conjugates the quaternion
*@param pQ - quaternion to conjugate
*@param[out] pR - conjugated quaternion
*/
void Conjugate(const MG_Quaternion* pQ, MG_Quaternion* pR)
{
    pR->m_X = -pQ->m_X;
    pR->m_Y = -pQ->m_Y;
    pR->m_Z = -pQ->m_Z;
    pR->m_W =  pQ->m_W;
}

/**
* Gets a rotation matrix from a quaternion
*@param pQ - quaternion from which the matrix should be get
*@param[out] pR - rotation matrix
*/
void GetMatrix(const MG_Quaternion* pQ, MG_Matrix* pR)
{
    pR->m_Table[0][0] = 1.0f - 2.0f * (pQ->m_Y * pQ->m_Y + pQ->m_Z * pQ->m_Z); pR->m_Table[0][1] =        2.0f * (pQ->m_X * pQ->m_Y - pQ->m_Z * pQ->m_W); pR->m_Table[0][2] =        2.0f * (pQ->m_X * pQ->m_Z + pQ->m_Y * pQ->m_W); pR->m_Table[0][3] = 0.0f;
    pR->m_Table[1][0] =        2.0f * (pQ->m_X * pQ->m_Y + pQ->m_Z * pQ->m_W); pR->m_Table[1][1] = 1.0f - 2.0f * (pQ->m_X * pQ->m_X + pQ->m_Z * pQ->m_Z); pR->m_Table[1][2] =        2.0f * (pQ->m_Y * pQ->m_Z - pQ->m_X * pQ->m_W); pR->m_Table[1][3] = 0.0f;
    pR->m_Table[2][0] =        2.0f * (pQ->m_X * pQ->m_Z - pQ->m_Y * pQ->m_W); pR->m_Table[2][1] =        2.0f * (pQ->m_Y * pQ->m_Z + pQ->m_X * pQ->m_W); pR->m_Table[2][2] = 1.0f - 2.0f * (pQ->m_X * pQ->m_X + pQ->m_Y * pQ->m_Y); pR->m_Table[2][3] = 0.0f;
    pR->m_Table[3][0] = 0.0f;                                                  pR->m_Table[3][1] = 0.0f;                                                  pR->m_Table[3][2] = 0.0f;                                                  pR->m_Table[3][3] = 1.0f;
}

#endif // MiniGeometryH
