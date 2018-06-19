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

#include "CSR_Geometry.h"

// std
#include <math.h>

//---------------------------------------------------------------------------
// 2D vector functions
//---------------------------------------------------------------------------
void csrVec2Add(const CSR_Vector2* pV1, const CSR_Vector2* pV2, CSR_Vector2* pR)
{
    pR->m_X = pV1->m_X + pV2->m_X;
    pR->m_Y = pV1->m_Y + pV2->m_Y;
}
//---------------------------------------------------------------------------
void csrVec2Sub(const CSR_Vector2* pV1, const CSR_Vector2* pV2, CSR_Vector2* pR)
{
    pR->m_X = pV1->m_X - pV2->m_X;
    pR->m_Y = pV1->m_Y - pV2->m_Y;
}
//---------------------------------------------------------------------------
void csrVec2Length(const CSR_Vector2* pV, float* pR)
{
    *pR = sqrt((pV->m_X * pV->m_X) + (pV->m_Y * pV->m_Y));
}
//---------------------------------------------------------------------------
void csrVec2Normalize(const CSR_Vector2* pV, CSR_Vector2* pR)
{
    float len;

    // calculate vector length
    csrVec2Length(pV, &len);

    // no vector length?
    if (len == 0.0f)
    {
        pR->m_X = 0.0f;
        pR->m_Y = 0.0f;
        return;
    }

    // normalize vector (thus values will always be between 0.0f, and 1.0f)
    pR->m_X = (pV->m_X / len);
    pR->m_Y = (pV->m_Y / len);
}
//---------------------------------------------------------------------------
void csrVec2Cross(const CSR_Vector2* pV1, const CSR_Vector2* pV2, CSR_Vector2* pR)
{
    pR->m_X = (pV1->m_Y * pV2->m_X) - (pV2->m_Y * pV1->m_X);
    pR->m_Y = (pV1->m_X * pV2->m_Y) - (pV2->m_X * pV1->m_Y);
}
//---------------------------------------------------------------------------
void csrVec2Dot(const CSR_Vector2* pV1, const CSR_Vector2* pV2, float* pR)
{
    *pR = ((pV1->m_X * pV2->m_X) + (pV1->m_Y * pV2->m_Y));
}
//---------------------------------------------------------------------------
int csrVec2BetweenRange(const CSR_Vector2* pV,
                        const CSR_Vector2* pRS,
                        const CSR_Vector2* pRE,
                              float        tolerance)
{
    // check if each vector component is between start and end limits
    if (!csrMathBetween(pV->m_X, pRS->m_X, pRE->m_X, tolerance))
        return 0;

    if (!csrMathBetween(pV->m_Y, pRS->m_Y, pRE->m_Y, tolerance))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
// 3D vector functions
//---------------------------------------------------------------------------
void csrVec3Add(const CSR_Vector3* pV1, const CSR_Vector3* pV2, CSR_Vector3* pR)
{
    pR->m_X = pV1->m_X + pV2->m_X;
    pR->m_Y = pV1->m_Y + pV2->m_Y;
    pR->m_Z = pV1->m_Z + pV2->m_Z;
}
//---------------------------------------------------------------------------
void csrVec3Sub(const CSR_Vector3* pV1, const CSR_Vector3* pV2, CSR_Vector3* pR)
{
    pR->m_X = pV1->m_X - pV2->m_X;
    pR->m_Y = pV1->m_Y - pV2->m_Y;
    pR->m_Z = pV1->m_Z - pV2->m_Z;
}
//---------------------------------------------------------------------------
void csrVec3Length(const CSR_Vector3* pV, float* pR)
{
    *pR = sqrt((pV->m_X * pV->m_X) + (pV->m_Y * pV->m_Y) + (pV->m_Z * pV->m_Z));
}
//---------------------------------------------------------------------------
void csrVec3Normalize(const CSR_Vector3* pV, CSR_Vector3* pR)
{
    float len;

    // calculate vector length
    csrVec3Length(pV, &len);

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
//---------------------------------------------------------------------------
void csrVec3Cross(const CSR_Vector3* pV1, const CSR_Vector3* pV2, CSR_Vector3* pR)
{
    pR->m_X = (pV1->m_Y * pV2->m_Z) - (pV2->m_Y * pV1->m_Z);
    pR->m_Y = (pV1->m_Z * pV2->m_X) - (pV2->m_Z * pV1->m_X);
    pR->m_Z = (pV1->m_X * pV2->m_Y) - (pV2->m_X * pV1->m_Y);
}
//---------------------------------------------------------------------------
void csrVec3Dot(const CSR_Vector3* pV1, const CSR_Vector3* pV2, float* pR)
{
    *pR = ((pV1->m_X * pV2->m_X) + (pV1->m_Y * pV2->m_Y) + (pV1->m_Z * pV2->m_Z));
}
//---------------------------------------------------------------------------
int csrVec3BetweenRange(const CSR_Vector3* pV,
                        const CSR_Vector3* pRS,
                        const CSR_Vector3* pRE,
                              float        tolerance)
{
    // check if each vector component is between start and end limits
    if (!csrMathBetween(pV->m_X, pRS->m_X, pRE->m_X, tolerance))
        return 0;

    if (!csrMathBetween(pV->m_Y, pRS->m_Y, pRE->m_Y, tolerance))
        return 0;

    if (!csrMathBetween(pV->m_Z, pRS->m_Z, pRE->m_Z, tolerance))
        return 0;

    return 1;
}
//---------------------------------------------------------------------------
// Matrix functions
//---------------------------------------------------------------------------
void csrMat4Identity(CSR_Matrix4* pR)
{
    pR->m_Table[0][0] = 1.0f; pR->m_Table[1][0] = 0.0f; pR->m_Table[2][0] = 0.0f; pR->m_Table[3][0] = 0.0f;
    pR->m_Table[0][1] = 0.0f; pR->m_Table[1][1] = 1.0f; pR->m_Table[2][1] = 0.0f; pR->m_Table[3][1] = 0.0f;
    pR->m_Table[0][2] = 0.0f; pR->m_Table[1][2] = 0.0f; pR->m_Table[2][2] = 1.0f; pR->m_Table[3][2] = 0.0f;
    pR->m_Table[0][3] = 0.0f; pR->m_Table[1][3] = 0.0f; pR->m_Table[2][3] = 0.0f; pR->m_Table[3][3] = 1.0f;
}
//---------------------------------------------------------------------------
void csrMat4Ortho(float left, float right, float bottom, float top, float zNear, float zFar, CSR_Matrix4* pR)
{
    // OpenGL specifications                                 can be rewritten as
    // |  2/(r-l)  0         0        -(r+l)/(r-l)  |        |  2/(r-l)  0        0        (r+l)/(l-r)  |
    // |  0        2/(t-b)   0        -(t+b)/(t-b)  |   =>   |  0        2/(t-b)  0        (t+b)/(b-t)  |
    // |  0        0        -2/(f-n)  -(f+n)/(f-n)  |        |  0        0        2/(n-f)  (f+n)/(n-f)  |
    // |  0        0         0         1            |        |  0        0        0        1            |

    // calculate matrix component values
    const float pfn =  zFar  + zNear;
    const float mnf =  zNear - zFar;
    const float prl =  right + left;
    const float mrl =  right - left;
    const float ptb =  top   + bottom;
    const float mtb =  top   - bottom;
    const float mlr = -mrl;
    const float mbt = -mtb;

    // build matrix
    pR->m_Table[0][0] = 2.0f / mrl; pR->m_Table[1][0] = 0.0f;       pR->m_Table[2][0] = 0.0f;       pR->m_Table[3][0] = prl / mlr;
    pR->m_Table[0][1] = 0.0f;       pR->m_Table[1][1] = 2.0f / mtb; pR->m_Table[2][1] = 0.0f;       pR->m_Table[3][1] = ptb / mbt;
    pR->m_Table[0][2] = 0.0f;       pR->m_Table[1][2] = 0.0f;       pR->m_Table[2][2] = 2.0f / mnf; pR->m_Table[3][2] = pfn / mnf;
    pR->m_Table[0][3] = 0.0f;       pR->m_Table[1][3] = 0.0f;       pR->m_Table[2][3] = 0.0f;       pR->m_Table[3][3] = 1.0f;
}
//---------------------------------------------------------------------------
void csrMat4Frustum(float left, float right, float bottom, float top, float zNear, float zFar, CSR_Matrix4* pR)
{
    // OpenGL specifications                                          can be rewritten as
    // |  2n/(r-l)     0             0             0         |        |  2n/(r-l)     0             0            0         |
    // |  0            2n/(t-b)      0             0         |   =>   |  0            2n/(t-b)      0            0         |
    // |  (r+l)/(r-l)  (t+b)/(t-b)  -(f+n)/(f-n)  -2fn/(f-n) |        |  (r+l)/(r-l)  (t+b)/(t-b)   (f+n)/(n-f)  2fn/(n-f) |
    // |  0            0            -1             0         |        |  0            0            -1            0         |
    // invalid for n <= 0, f <= 0, l = r, b = t, or n = f

    // calculate matrix component values
    const float x2n  =  2.0f * zNear;
    const float x2nf =  x2n  * zFar;
    const float pfn  = zFar  + zNear;
    const float mnf  = zNear - zFar;
    const float prl  = right + left;
    const float mrl  = right - left;
    const float ptb  = top   + bottom;
    const float mtb  = top   - bottom;

    // build matrix
    pR->m_Table[0][0] = x2n / mrl; pR->m_Table[1][0] = 0.0f;      pR->m_Table[2][0] =  0.0f;      pR->m_Table[3][0] = 0.0f;
    pR->m_Table[0][1] = 0.0f;      pR->m_Table[1][1] = x2n / mtb; pR->m_Table[2][1] =  0.0f;      pR->m_Table[3][1] = 0.0f;
    pR->m_Table[0][2] = prl / mrl; pR->m_Table[1][2] = ptb / mtb; pR->m_Table[2][2] =  pfn / mnf; pR->m_Table[3][2] = x2nf / mnf;
    pR->m_Table[0][3] = 0.0f;      pR->m_Table[1][3] = 0.0f;      pR->m_Table[2][3] = -1.0f;      pR->m_Table[3][3] = 0.0f;
}
//---------------------------------------------------------------------------
void csrMat4Perspective(float fovyDeg, float aspect, float zNear, float zFar, CSR_Matrix4* pR)
{
    const float maxY    =  zNear *  tanf(fovyDeg * M_PI / 360.0f);
    const float maxX    =  maxY   * aspect;
    const float negMaxY = -maxY;
    const float negMaxX = -maxX;

    csrMat4Frustum(negMaxX, maxX, negMaxY, maxY, zNear, zFar, pR);
}
//---------------------------------------------------------------------------
void csrMat4LookAtLH(const CSR_Vector3* pPos,
                     const CSR_Vector3* pDir,
                     const CSR_Vector3* pUp,
                           CSR_Matrix4* pR)
{
    float       xAxisDotPos;
    float       yAxisDotPos;
    float       zAxisDotPos;
    CSR_Vector3 xAxis;
    CSR_Vector3 yAxis;
    CSR_Vector3 zAxis;
    CSR_Vector3 vec;

    // compute per axis transformations
    csrVec3Sub(pDir, pPos, &vec);
    csrVec3Normalize(&vec, &zAxis);
    csrVec3Cross(pUp, &zAxis, &vec);
    csrVec3Normalize(&vec, &xAxis);
    csrVec3Cross(&zAxis, &xAxis, &yAxis);
    csrVec3Dot(&xAxis, pPos, &xAxisDotPos);
    csrVec3Dot(&yAxis, pPos, &yAxisDotPos);
    csrVec3Dot(&zAxis, pPos, &zAxisDotPos);

    // create look at matrix, translate eye position
    pR->m_Table[0][0] =  xAxis.m_X;   pR->m_Table[1][0] =  yAxis.m_X;   pR->m_Table[2][0] =  zAxis.m_X;   pR->m_Table[3][0] = 0.0f;
    pR->m_Table[0][1] =  xAxis.m_Y;   pR->m_Table[1][1] =  yAxis.m_Y;   pR->m_Table[2][1] =  zAxis.m_Y;   pR->m_Table[3][1] = 0.0f;
    pR->m_Table[0][2] =  xAxis.m_Z;   pR->m_Table[1][2] =  yAxis.m_Z;   pR->m_Table[2][2] =  zAxis.m_Z;   pR->m_Table[3][2] = 0.0f;
    pR->m_Table[0][3] = -xAxisDotPos; pR->m_Table[1][3] = -yAxisDotPos; pR->m_Table[2][3] = -zAxisDotPos; pR->m_Table[3][3] = 1.0f;
}
//---------------------------------------------------------------------------
void csrMat4LookAtRH(const CSR_Vector3* pPos,
                     const CSR_Vector3* pDir,
                     const CSR_Vector3* pUp,
                           CSR_Matrix4* pR)
{
    float       xAxisDotPos;
    float       yAxisDotPos;
    float       zAxisDotPos;
    CSR_Vector3 xAxis;
    CSR_Vector3 yAxis;
    CSR_Vector3 zAxis;
    CSR_Vector3 vec;

    // compute per axis transformations
    csrVec3Sub(pDir, pPos, &vec);
    csrVec3Normalize(&vec, &zAxis);
    csrVec3Cross(pUp, &zAxis, &vec);
    csrVec3Normalize(&vec, &xAxis);
    csrVec3Cross(&zAxis, &xAxis, &yAxis);
    csrVec3Dot(&xAxis, pPos, &xAxisDotPos);
    csrVec3Dot(&yAxis, pPos, &yAxisDotPos);
    csrVec3Dot(&zAxis, pPos, &zAxisDotPos);

    // create look at matrix, translate eye position
    pR->m_Table[0][0] = xAxis.m_X;   pR->m_Table[1][0] = yAxis.m_X;   pR->m_Table[2][0] = zAxis.m_X;   pR->m_Table[3][0] = 0.0f;
    pR->m_Table[0][1] = xAxis.m_Y;   pR->m_Table[1][1] = yAxis.m_Y;   pR->m_Table[2][1] = zAxis.m_Y;   pR->m_Table[3][1] = 0.0f;
    pR->m_Table[0][2] = xAxis.m_Z;   pR->m_Table[1][2] = yAxis.m_Z;   pR->m_Table[2][2] = zAxis.m_Z;   pR->m_Table[3][2] = 0.0f;
    pR->m_Table[0][3] = xAxisDotPos; pR->m_Table[1][3] = yAxisDotPos; pR->m_Table[2][3] = zAxisDotPos; pR->m_Table[3][3] = 1.0f;
}
//---------------------------------------------------------------------------
void csrMat4Translate(const CSR_Vector3* pT, CSR_Matrix4* pR)
{
    csrMat4Identity(pR);
    pR->m_Table[3][0] += pR->m_Table[0][0] * pT->m_X + pR->m_Table[1][0] * pT->m_Y + pR->m_Table[2][0] * pT->m_Z;
    pR->m_Table[3][1] += pR->m_Table[0][1] * pT->m_X + pR->m_Table[1][1] * pT->m_Y + pR->m_Table[2][1] * pT->m_Z;
    pR->m_Table[3][2] += pR->m_Table[0][2] * pT->m_X + pR->m_Table[1][2] * pT->m_Y + pR->m_Table[2][2] * pT->m_Z;
    pR->m_Table[3][3] += pR->m_Table[0][3] * pT->m_X + pR->m_Table[1][3] * pT->m_Y + pR->m_Table[2][3] * pT->m_Z;
}
//---------------------------------------------------------------------------
void csrMat4Rotate(float angle, const CSR_Vector3* pAxis, CSR_Matrix4* pR)
{
    // calculate sinus, cosinus and inverted cosinus values
    const float c  = cosf(angle);
    const float s  = sinf(angle);
    const float ic = (1.0f - c);

    // create rotation matrix
    csrMat4Identity(pR);
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
//---------------------------------------------------------------------------
void csrMat4Scale(const CSR_Vector3* pFactor, CSR_Matrix4* pR)
{
    csrMat4Identity(pR);
    pR->m_Table[0][0] *= pFactor->m_X; pR->m_Table[1][0] *= pFactor->m_Y; pR->m_Table[2][0] *= pFactor->m_Z;
    pR->m_Table[0][1] *= pFactor->m_X; pR->m_Table[1][1] *= pFactor->m_Y; pR->m_Table[2][1] *= pFactor->m_Z;
    pR->m_Table[0][2] *= pFactor->m_X; pR->m_Table[1][2] *= pFactor->m_Y; pR->m_Table[2][2] *= pFactor->m_Z;
    pR->m_Table[0][3] *= pFactor->m_X; pR->m_Table[1][3] *= pFactor->m_Y; pR->m_Table[2][3] *= pFactor->m_Z;
}
//---------------------------------------------------------------------------
void csrMat4Multiply(const CSR_Matrix4* pM1, const CSR_Matrix4* pM2, CSR_Matrix4* pR)
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
//---------------------------------------------------------------------------
void csrMat4Transpose(const CSR_Matrix4* pM, CSR_Matrix4* pR)
{
    int i;
    int j;

    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            pR->m_Table[j][i] = pM->m_Table[i][j];
}
//---------------------------------------------------------------------------
void csrMat4Inverse(const CSR_Matrix4* pM, CSR_Matrix4* pR, float* pDeterminant)
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
//---------------------------------------------------------------------------
void csrMat4ApplyToVector(const CSR_Matrix4* pM, const CSR_Vector3* pV, CSR_Vector3* pR)
{
    pR->m_X = (pV->m_X * pM->m_Table[0][0] + pV->m_Y * pM->m_Table[1][0] + pV->m_Z * pM->m_Table[2][0] + pM->m_Table[3][0]);
    pR->m_Y = (pV->m_X * pM->m_Table[0][1] + pV->m_Y * pM->m_Table[1][1] + pV->m_Z * pM->m_Table[2][1] + pM->m_Table[3][1]);
    pR->m_Z = (pV->m_X * pM->m_Table[0][2] + pV->m_Y * pM->m_Table[1][2] + pV->m_Z * pM->m_Table[2][2] + pM->m_Table[3][2]);
}
//---------------------------------------------------------------------------
void csrMat4ApplyToNormal(const CSR_Matrix4* pM, const CSR_Vector3* pN, CSR_Vector3* pR)
{
    pR->m_X = (pN->m_X * pM->m_Table[0][0] + pN->m_Y * pM->m_Table[1][0] + pN->m_Z * pM->m_Table[2][0]);
    pR->m_Y = (pN->m_X * pM->m_Table[0][1] + pN->m_Y * pM->m_Table[1][1] + pN->m_Z * pM->m_Table[2][1]);
    pR->m_Z = (pN->m_X * pM->m_Table[0][2] + pN->m_Y * pM->m_Table[1][2] + pN->m_Z * pM->m_Table[2][2]);
}
//---------------------------------------------------------------------------
void csrMat4Transform(const CSR_Matrix4* pM, const CSR_Vector3* pV, CSR_Vector3* pR)
{
    float w;

    // apply the transformations (position, rotation, scaling, ...) to the vector
    csrMat4ApplyToVector(pM, pV, pR);

    // calculate the amplitude
    w = (pV->m_X * pM->m_Table[0][3] + pV->m_Y * pM->m_Table[1][3] + pV->m_Z * pM->m_Table[2][3] + pM->m_Table[3][3]);

    // should not happen, unless the matrix is wrong
    if (!w)
        return;

    // calculate the final vector
    pR->m_X /= w;
    pR->m_Y /= w;
    pR->m_Z /= w;
}
//---------------------------------------------------------------------------
void csrMat4Unproject(const CSR_Matrix4* pP, const CSR_Matrix4* pV, CSR_Ray3* pR)
{
    float       determinant;
    CSR_Matrix4 invertProj;
    CSR_Matrix4 invertView;
    CSR_Vector3 unprojRayPos;
    CSR_Vector3 unprojRayDir;
    CSR_Vector3 unprojRayDirN;

    // unproject the ray to make it in the viewport coordinates
    csrMat4Inverse(pP, &invertProj, &determinant);
    csrMat4ApplyToVector(&invertProj, &pR->m_Pos, &unprojRayPos);
    csrMat4ApplyToVector(&invertProj, &pR->m_Dir, &unprojRayDir);
    csrVec3Normalize(&unprojRayDir, &unprojRayDirN);
    csrRay3FromPointDir(&unprojRayPos, &unprojRayDirN, pR);

    // do unproject the viewport only?
    if (!pV)
        return;

    // unproject the ray to make it in the view coordinates
    csrMat4Inverse(pV, &invertView, &determinant);
    csrMat4ApplyToVector(&invertView, &pR->m_Pos, &unprojRayPos);
    csrMat4ApplyToNormal(&invertView, &pR->m_Dir, &unprojRayDir);
    csrVec3Normalize(&unprojRayDir, &unprojRayDirN);
    csrRay3FromPointDir(&unprojRayPos, &unprojRayDirN, pR);
}
//---------------------------------------------------------------------------
void csrMat4TranslationFrom(const CSR_Matrix4* pM, float* pX, float* pY, float* pZ)
{
    *pX = pM->m_Table[3][0];
    *pY = pM->m_Table[3][1];
    *pZ = pM->m_Table[3][2];
}
//---------------------------------------------------------------------------
void csrMat4RotationFrom(const CSR_Matrix4* pM, float* pX, float* pY, float* pZ)
{
    float rx;
    float ry;
    float C;

    // calculate the y angle
    *pY = asin(pM->m_Table[2][0]);
     C  = cos(*pY);

    // gimbal lock?
    if (fabs(C) > 0.0005f)
    {
        // calculate the x angle
         rx =  pM->m_Table[2][2] / C;
         ry = -pM->m_Table[2][1] / C;
        *pX =  atan2(ry, rx);

        // calculate the z angle
         rx =  pM->m_Table[0][0] / C;
         ry = -pM->m_Table[1][0] / C;
        *pZ =  atan2(ry, rx);
    }
    else
    {
        // in this case x is always 0
        *pX  = 0.0f;

        // calculate the z angle
         rx = pM->m_Table[1][1];
         ry = pM->m_Table[0][1];
        *pZ = atan2(ry, rx);
    }

    // limit the resulting angles between the max possible value
    *pX = fmod(*pX, M_PI);
    *pY = fmod(*pY, M_PI);
    *pZ = fmod(*pZ, M_PI);
}
//---------------------------------------------------------------------------
// Quaternion functions
//---------------------------------------------------------------------------
void csrQuatFromAxis(float angle, const CSR_Vector3* pAxis, CSR_Quaternion* pR)
{
    const float s = sinf(angle * 0.5f);

    pR->m_X = (pAxis->m_X * s);
    pR->m_Y = (pAxis->m_Y * s);
    pR->m_Z = (pAxis->m_Z * s);
    pR->m_W =  cosf(angle * 0.5f);
}
//---------------------------------------------------------------------------
void csrQuatFromEuler(float angleX, float angleY, float angleZ, CSR_Quaternion* pR)
{
    const float cos_x_2 = cosf(0.5f * angleX);
    const float cos_y_2 = cosf(0.5f * angleY);
    const float cos_z_2 = cosf(0.5f * angleZ);

    const float sin_x_2 = sinf(0.5f * angleX);
    const float sin_y_2 = sinf(0.5f * angleY);
    const float sin_z_2 = sinf(0.5f * angleZ);

    // create the quaternion
    pR->m_X = cos_z_2 * cos_y_2 * sin_x_2 - sin_z_2 * sin_y_2 * cos_x_2;
    pR->m_Y = cos_z_2 * sin_y_2 * cos_x_2 + sin_z_2 * cos_y_2 * sin_x_2;
    pR->m_Z = sin_z_2 * cos_y_2 * cos_x_2 - cos_z_2 * sin_y_2 * sin_x_2;
    pR->m_W = cos_z_2 * cos_y_2 * cos_x_2 + sin_z_2 * sin_y_2 * sin_x_2;
}
//---------------------------------------------------------------------------
void csrQuatFromPYR(float pitch, float yaw, float roll, CSR_Quaternion* pR)
{
    // calculate the sinus and cosinus of each angles
    const float c1 = cos(yaw   / 2.0f);
    const float c2 = cos(pitch / 2.0f);
    const float c3 = cos(roll  / 2.0f);
    const float s1 = sin(yaw   / 2.0f);
    const float s2 = sin(pitch / 2.0f);
    const float s3 = sin(roll  / 2.0f);

    // calculate the quaternion values
    pR->m_X = (s1 * s2 * c3) + (c1 * c2 * s3);
    pR->m_Y = (s1 * c2 * c3) + (c1 * s2 * s3);
    pR->m_Z = (c1 * s2 * c3) - (s1 * c2 * s3);
    pR->m_W = (c1 * c2 * c3) - (s1 * s2 * s3);
}
//---------------------------------------------------------------------------
void csrQuatLengthSquared(const CSR_Quaternion* pQ, float* pR)
{
    *pR = ((pQ->m_X * pQ->m_X) + (pQ->m_Y * pQ->m_Y) + (pQ->m_Z * pQ->m_Z) + (pQ->m_W * pQ->m_W));
}
//---------------------------------------------------------------------------
void csrQuatLength(const CSR_Quaternion* pQ, float* pR)
{
    float norm;
    csrQuatLengthSquared(pQ, &norm);

    *pR = sqrt(norm);
}
//---------------------------------------------------------------------------
void csrQuatNormalize(const CSR_Quaternion* pQ, CSR_Quaternion* pR)
{
    float len;
    csrQuatLength(pQ, &len);

    if (!len)
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
//---------------------------------------------------------------------------
void csrQuatDot(const CSR_Quaternion* pQ1, const CSR_Quaternion* pQ2, float* pR)
{
    *pR = ((pQ1->m_X * pQ2->m_X) + (pQ1->m_Y * pQ2->m_Y) + (pQ1->m_Z * pQ2->m_Z) + (pQ1->m_W * pQ2->m_W));
}
//---------------------------------------------------------------------------
void csrQuatScale(const CSR_Quaternion* pQ, float s, CSR_Quaternion* pR)
{
    pR->m_X = pQ->m_X * s;
    pR->m_Y = pQ->m_Y * s;
    pR->m_Z = pQ->m_Z * s;
    pR->m_W = pQ->m_W * s;
}
//---------------------------------------------------------------------------
void csrQuatConjugate(const CSR_Quaternion* pQ, CSR_Quaternion* pR)
{
    pR->m_X = -pQ->m_X;
    pR->m_Y = -pQ->m_Y;
    pR->m_Z = -pQ->m_Z;
    pR->m_W =  pQ->m_W;
}
//---------------------------------------------------------------------------
void csrQuatMultiply(const CSR_Quaternion* pQ1, const CSR_Quaternion* pQ2, CSR_Quaternion* pR)
{
    pR->m_X = pQ1->m_W * pQ2->m_X + pQ1->m_X * pQ2->m_W + pQ1->m_Y * pQ2->m_Z - pQ1->m_Z * pQ2->m_Y;
    pR->m_Y = pQ1->m_W * pQ2->m_Y - pQ1->m_X * pQ2->m_Z + pQ1->m_Y * pQ2->m_W + pQ1->m_Z * pQ2->m_X;
    pR->m_Z = pQ1->m_W * pQ2->m_Z + pQ1->m_X * pQ2->m_Y - pQ1->m_Y * pQ2->m_X + pQ1->m_Z * pQ2->m_W;
    pR->m_W = pQ1->m_W * pQ2->m_W - pQ1->m_X * pQ2->m_X - pQ1->m_Y * pQ2->m_Y - pQ1->m_Z * pQ2->m_Z;
}
//---------------------------------------------------------------------------
void csrQuatInverse(const CSR_Quaternion* pQ, CSR_Quaternion* pR)
{
    float          lengthSq;
    CSR_Quaternion conjugedQuat;

    // calculate the squared length of the quaternion
    csrQuatLengthSquared(pQ, &lengthSq);

    // empty quaternion?
    if (!lengthSq)
    {
        pR->m_X = 0.0f;
        pR->m_Y = 0.0f;
        pR->m_Z = 0.0f;
        pR->m_W = 0.0f;
        return;
    }

    csrQuatConjugate(pQ, &conjugedQuat);
    csrQuatScale(&conjugedQuat, 1.0f / lengthSq, pR);
}
//---------------------------------------------------------------------------
void csrQuatRotate(const CSR_Quaternion* pQ, const CSR_Vector3* pV, CSR_Vector3* pR)
{
    CSR_Quaternion qv;
    CSR_Quaternion qi;
    CSR_Quaternion qm;

    qv.m_X = pV->m_X;
    qv.m_Y = pV->m_Y;
    qv.m_Z = pV->m_Z;
    qv.m_W = 0.0f;

    csrQuatInverse(pQ, &qi);

    // rotate vector
    qm.m_X = pQ->m_X * qv.m_X * qi.m_X;
    qm.m_Y = pQ->m_Y * qv.m_Y * qi.m_Y;
    qm.m_Z = pQ->m_Z * qv.m_Z * qi.m_Z;
    qm.m_W = pQ->m_W * qv.m_W * qi.m_W;

    pR->m_X = qm.m_X;
    pR->m_Y = qm.m_Y;
    pR->m_Z = qm.m_Z;
}
//---------------------------------------------------------------------------
int csrQuatSlerp(const CSR_Quaternion* pQ1, const CSR_Quaternion* pQ2, float p, CSR_Quaternion* pR)
{
    CSR_Quaternion interpolateWith;
    float          scale0;
    float          scale1;
    float          theta;
    float          sinTheta;
    float          result;

    // are quaternions identical?
    if (pQ1->m_X == pQ2->m_X && pQ1->m_Y == pQ2->m_Y && pQ1->m_Z == pQ2->m_Z && pQ1->m_W == pQ2->m_W)
    {
        *pR = *pQ1;
        return 1;
    }

    // calculate dot product between q1 and q2
    csrQuatDot(pQ1, pQ2, &result);

    // check if angle is higher than 90° (this happen if dot product is less than 0)
    if (result < 0.0f)
    {
        // negate the second quaternion and the dot product result
        interpolateWith.m_X = -pQ2->m_X;
        interpolateWith.m_Y = -pQ2->m_Y;
        interpolateWith.m_Z = -pQ2->m_Z;
        interpolateWith.m_W = -pQ2->m_W;
        result              = -result;
    }
    else
        interpolateWith = *pQ2;

    // calculate the first and second scaling factor to apply to the interpolation
    scale0 = 1.0f - p;
    scale1 = p;

    // is angle large enough to apply the calculation?
    if ((1.0f - result) > 0.1f)
    {
        // calculate the angle between the 2 quaternions and get the sinus of that angle
        theta    = acos(result);
        sinTheta = sinf(theta);

        // is resulting sinus equal to 0? (just to verify, should not happen)
        if (!sinTheta)
        {
            pR->m_X = 0.0f;
            pR->m_Y = 0.0f;
            pR->m_Z = 0.0f;
            pR->m_W = 0.0f;

            return 0;
        }

        // calculate the scaling for q1 and q2, according to the angle and it's sine value
        scale0 = sinf((1.0f - p) * theta) / sinTheta;
        scale1 = sinf (p         * theta) / sinTheta;
    }

    // calculate the resulting quaternion by using a special form of linear interpolation
    pR->m_X = (scale0 * pQ1->m_X) + (scale1 * interpolateWith.m_X);
    pR->m_Y = (scale0 * pQ1->m_Y) + (scale1 * interpolateWith.m_Y);
    pR->m_Z = (scale0 * pQ1->m_Z) + (scale1 * interpolateWith.m_Z);
    pR->m_W = (scale0 * pQ1->m_W) + (scale1 * interpolateWith.m_W);

    return 1;
}
//---------------------------------------------------------------------------
int csrQuatFromMatrix(const CSR_Matrix4* pM, CSR_Quaternion* pR)
{
    float diagonal;
    float scale;

    // calculate the matrix diagonal by adding up it's diagonal indices (also known as "trace")
    diagonal = pM->m_Table[0][0] + pM->m_Table[1][1] + pM->m_Table[2][2] + pM->m_Table[3][3];

    // is diagonal greater than zero?
    if (diagonal > 0.00000001f)
    {
        // calculate the diagonal scale
        scale = sqrt(diagonal) * 2.0f;

        // calculate the quaternion values using the respective equation
        pR->m_X = (pM->m_Table[1][2] - pM->m_Table[2][1]) / scale;
        pR->m_Y = (pM->m_Table[2][0] - pM->m_Table[0][2]) / scale;
        pR->m_Z = (pM->m_Table[0][1] - pM->m_Table[1][0]) / scale;
        pR->m_W =  0.25f * scale;

        return 1;
    }

    // search for highest value in the matrix diagonal
    if (pM->m_Table[0][0] > pM->m_Table[1][1] && pM->m_Table[0][0] > pM->m_Table[2][2])
    {
        // calculate scale using the first diagonal element and double that value
        scale = sqrt(1.0f + pM->m_Table[0][0] - pM->m_Table[1][1] - pM->m_Table[2][2]) * 2.0f;

        // should not happen, but better to verify
        if (!scale)
        {
            pR->m_X = 0.0f;
            pR->m_Y = 0.0f;
            pR->m_Z = 0.0f;
            pR->m_W = 0.0f;

            return 0;
        }

        // calculate the quaternion values using the respective equation
        pR->m_X =  0.25f * scale;
        pR->m_Y = (pM->m_Table[0][1] + pM->m_Table[1][0]) / scale;
        pR->m_Z = (pM->m_Table[2][0] + pM->m_Table[0][2]) / scale;
        pR->m_W = (pM->m_Table[1][2] - pM->m_Table[2][1]) / scale;

        return 1;
    }

    if (pM->m_Table[1][1] > pM->m_Table[2][2])
    {
        // calculate scale using the second diagonal element and double that value
        scale = sqrt(1.0f + pM->m_Table[1][1] - pM->m_Table[0][0] - pM->m_Table[2][2]) * 2.0f;

        // should not happen, but better to verify
        if (!scale)
        {
            pR->m_X = 0.0f;
            pR->m_Y = 0.0f;
            pR->m_Z = 0.0f;
            pR->m_W = 0.0f;

            return 0;
        }

        // calculate the quaternion values using the respective equation
        pR->m_X = (pM->m_Table[0][1] + pM->m_Table[1][0]) / scale;
        pR->m_Y =  0.25f * scale;
        pR->m_Z = (pM->m_Table[1][2] + pM->m_Table[2][1]) / scale;
        pR->m_W = (pM->m_Table[2][0] - pM->m_Table[0][2]) / scale;

        return 1;
    }

    // calculate scale using the third diagonal element and double that value
    scale = sqrt(1.0f + pM->m_Table[2][2] - pM->m_Table[0][0] - pM->m_Table[1][1]) * 2.0f;

    // should not happen, but better to verify
    if (!scale)
    {
        pR->m_X = 0.0f;
        pR->m_Y = 0.0f;
        pR->m_Z = 0.0f;
        pR->m_W = 0.0f;

        return 0;
    }

    // calculate the quaternion values using the respective equation
    pR->m_X = (pM->m_Table[2][0] + pM->m_Table[0][2]) / scale;
    pR->m_Y = (pM->m_Table[1][2] + pM->m_Table[2][1]) / scale;
    pR->m_Z =  0.25f * scale;
    pR->m_W = (pM->m_Table[0][1] - pM->m_Table[1][0]) / scale;

    return 1;
}
//---------------------------------------------------------------------------
void csrQuatToMatrix(const CSR_Quaternion* pQ, CSR_Matrix4* pR)
{
    pR->m_Table[0][0] = 1.0f - 2.0f * (pQ->m_Y * pQ->m_Y + pQ->m_Z * pQ->m_Z); pR->m_Table[0][1] =        2.0f * (pQ->m_X * pQ->m_Y - pQ->m_Z * pQ->m_W); pR->m_Table[0][2] =        2.0f * (pQ->m_X * pQ->m_Z + pQ->m_Y * pQ->m_W); pR->m_Table[0][3] = 0.0f;
    pR->m_Table[1][0] =        2.0f * (pQ->m_X * pQ->m_Y + pQ->m_Z * pQ->m_W); pR->m_Table[1][1] = 1.0f - 2.0f * (pQ->m_X * pQ->m_X + pQ->m_Z * pQ->m_Z); pR->m_Table[1][2] =        2.0f * (pQ->m_Y * pQ->m_Z - pQ->m_X * pQ->m_W); pR->m_Table[1][3] = 0.0f;
    pR->m_Table[2][0] =        2.0f * (pQ->m_X * pQ->m_Z - pQ->m_Y * pQ->m_W); pR->m_Table[2][1] =        2.0f * (pQ->m_Y * pQ->m_Z + pQ->m_X * pQ->m_W); pR->m_Table[2][2] = 1.0f - 2.0f * (pQ->m_X * pQ->m_X + pQ->m_Y * pQ->m_Y); pR->m_Table[2][3] = 0.0f;
    pR->m_Table[3][0] = 0.0f;                                                  pR->m_Table[3][1] = 0.0f;                                                  pR->m_Table[3][2] = 0.0f;                                                  pR->m_Table[3][3] = 1.0f;
}
//---------------------------------------------------------------------------
// Plane functions
//---------------------------------------------------------------------------
void csrPlaneFromPointNormal(const CSR_Vector3* pP, const CSR_Vector3* pN, CSR_Plane* pR)
{
    float d;

    // the a, b, and c components are only the normal of the plane
    pR->m_A = pN->m_X;
    pR->m_B = pN->m_Y;
    pR->m_C = pN->m_Z;

    // calculate plane d component using the aX + bY + cZ + d = 0 formula
    csrVec3Dot(pN, pP, &d);
    pR->m_D = -d;
}
//---------------------------------------------------------------------------
void csrPlaneFromPoints(const CSR_Vector3* pV1,
                        const CSR_Vector3* pV2,
                        const CSR_Vector3* pV3,
                              CSR_Plane*   pR)
{
    CSR_Vector3 e1;
    CSR_Vector3 e2;
    CSR_Vector3 normal;

    // calculate edge vectors
    csrVec3Sub(pV2, pV1, &e1);
    csrVec3Sub(pV3, pV1, &e2);

    // calculate the normal of the plane
    csrVec3Cross(&e1, &e2, &normal);
    csrVec3Normalize(&normal, &normal);

    // calculate and return the plane
    csrPlaneFromPointNormal(pV1, &normal, pR);
}
//---------------------------------------------------------------------------
void csrPlaneTransform(const CSR_Plane* pPlane, const CSR_Matrix4* pM, CSR_Plane* pR)
{
    pR->m_A = pM->m_Table[0][0] * pPlane->m_A + pM->m_Table[1][0] * pPlane->m_B + pM->m_Table[2][0] * pPlane->m_C + pM->m_Table[3][0] * pPlane->m_D;
    pR->m_B = pM->m_Table[0][1] * pPlane->m_A + pM->m_Table[1][1] * pPlane->m_B + pM->m_Table[2][1] * pPlane->m_C + pM->m_Table[3][1] * pPlane->m_D;
    pR->m_C = pM->m_Table[0][2] * pPlane->m_A + pM->m_Table[1][2] * pPlane->m_B + pM->m_Table[2][2] * pPlane->m_C + pM->m_Table[3][2] * pPlane->m_D;
    pR->m_D = pM->m_Table[0][3] * pPlane->m_A + pM->m_Table[1][3] * pPlane->m_B + pM->m_Table[2][3] * pPlane->m_C + pM->m_Table[3][3] * pPlane->m_D;
}
//---------------------------------------------------------------------------
void csrPlaneDistanceTo(const CSR_Vector3* pP, const CSR_Plane* pPl, float* pR)
{
    CSR_Vector3 n;
    float       dist;

    // get the normal of the plane
    n.m_X = pPl->m_A;
    n.m_Y = pPl->m_B;
    n.m_Z = pPl->m_C;

    // calculate the distance between the plane and the point
    csrVec3Dot(&n, pP, &dist);
    *pR = dist + pPl->m_D;
}
//---------------------------------------------------------------------------
// 3D ray functions
//---------------------------------------------------------------------------
void csrRay3FromPointDir(const CSR_Vector3* pP, const CSR_Vector3* pD, CSR_Ray3* pR)
{
    // get infinite value (NOTE this is the only case where a division by 0 is allowed)
    const float inf = 1.0f / 0.0f;

    pR->m_Pos = *pP;
    pR->m_Dir = *pD;

    // recompute the ray inverted direction
    if (!pD->m_X)
        pR->m_InvDir.m_X = inf;
    else
        pR->m_InvDir.m_X = 1.0f / pD->m_X;

    if (!pD->m_Y)
        pR->m_InvDir.m_Y = inf;
    else
        pR->m_InvDir.m_Y = 1.0f / pD->m_Y;

    if (!pD->m_Z)
        pR->m_InvDir.m_Z = inf;
    else
        pR->m_InvDir.m_Z = 1.0f / pD->m_Z;
}
//---------------------------------------------------------------------------
// 3D segment functions
//---------------------------------------------------------------------------
void csrSeg3DistanceBetween(const CSR_Segment3* pS1,
                            const CSR_Segment3* pS2,
                                  float         tolerance,
                                  float*        pR)
{
    CSR_Vector3 delta21;
    CSR_Vector3 delta43;
    CSR_Vector3 delta13;
    CSR_Vector3 dP;
    float       a;
    float       b;
    float       c;
    float       d;
    float       e;
    float       D;
    float       sc;
    float       sN;
    float       sD;
    float       tc;
    float       tN;
    float       tD;
    float       dotdP;

    // the parametric formulas for the 2 segments are:
    // p = s1 + s(s2 - s1), where s1 = pS1->m_Start and s2 = pS1->m_End
    // p = s3 + t(s4 - s3), where s3 = pS2->m_Start and s4 = pS2->m_End
    // so, calculate the basic values to use for the calculation
    csrVec3Sub(&pS1->m_End,   &pS1->m_Start, &delta21);
    csrVec3Sub(&pS2->m_End,   &pS2->m_Start, &delta43);
    csrVec3Sub(&pS1->m_Start, &pS2->m_Start, &delta13);

    // calculate the distance (represented by D) between the 2 line segments
    csrVec3Dot(&delta21, &delta21, &a);
    csrVec3Dot(&delta21, &delta43, &b);
    csrVec3Dot(&delta43, &delta43, &c);
    csrVec3Dot(&delta21, &delta13, &d);
    csrVec3Dot(&delta43, &delta13, &e);
    D = ((a * c) - (b * b));

    // sc = sN / sD, default sD = D >= 0
    sD = D;

    // tc = tN / tD, default tD = D >= 0
    tD = D;

    // compute the line parameters of the two closest points
    if (fabs(D) < tolerance)
    {
        // the lines are almost parallel, force using point P0 on segment S1 to prevent possible
        // division by 0 later
        sN = 0.0f;
        sD = 1.0f;
        tN = e;
        tD = c;
    }
    else
    {
        // get the closest points on the infinite lines
        sN = ((b * e) - (c * d));
        tN = ((a * e) - (b * d));

        // sc < 0 => the s=0 edge is visible
        if (sN < 0.0f)
        {
            sN = 0.0f;
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
    if (tN < 0.0f)
    {
        tN = 0.0f;

        // recompute sc for this edge
        if (-d < 0.0f)
            sN = 0.0f;
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
        if ((-d + b) < 0.0f)
            sN = 0.0f;
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
    if (fabs(sN) < tolerance)
        sc = 0.0f;
    else
        sc = sN / sD;

    if (fabs(tN) < tolerance)
        tc = 0.0f;
    else
        tc = tN / tD;

    // get the difference of the two closest points
    dP.m_X = delta13.m_X + (sc * delta21.m_X) - (tc * delta43.m_X);
    dP.m_Y = delta13.m_Y + (sc * delta21.m_Y) - (tc * delta43.m_Y);
    dP.m_Z = delta13.m_Z + (sc * delta21.m_Z) - (tc * delta43.m_Z);

    // return the closest distance
    csrVec3Dot(&dP, &dP, &dotdP);
    *pR = sqrt(dotdP);
}
//---------------------------------------------------------------------------
void csrSeg3ClosestPoint(const CSR_Segment3* pS, const CSR_Vector3* pP, CSR_Vector3* pR)
{
    float       segLength;
    float       angle;
    CSR_Vector3 PToStart;
    CSR_Vector3 length;
    CSR_Vector3 normalizedLength;
    CSR_Vector3 p;

    // calculate the distance between the test point and the segment
    csrVec3Sub( pP,        &pS->m_Start, &PToStart);
    csrVec3Sub(&pS->m_End, &pS->m_Start, &length);
    csrVec3Length(&length, &segLength);

    // calculate the direction of the segment
    csrVec3Normalize(&length, &normalizedLength);

    // calculate the projection of the point on the segment
    csrVec3Dot(&normalizedLength, &PToStart, &angle);

    // check if projection is before the segment
    if (angle < 0.0)
        *pR = pS->m_Start;
    else
    // check if projection is after the segment
    if (angle > segLength)
        *pR = pS->m_End;
    else
    {
        // calculate the position of the projection on the segment
        p.m_X = normalizedLength.m_X * angle;
        p.m_Y = normalizedLength.m_Y * angle;
        p.m_Z = normalizedLength.m_Z * angle;

        // calculate and returns the point coordinate on the segment
        csrVec3Add(&pS->m_Start, &p, pR);
    }
}
//---------------------------------------------------------------------------
// 3D polygon functions
//---------------------------------------------------------------------------
void csrPolygon3ClosestPoint(const CSR_Vector3* pP, const CSR_Polygon3* pPo, CSR_Vector3* pR)
{
    float        dAB;
    float        dBC;
    float        dCA;
    float        min;
    CSR_Vector3  rab;
    CSR_Vector3  rbc;
    CSR_Vector3  rca;
    CSR_Vector3  vAB;
    CSR_Vector3  vBC;
    CSR_Vector3  vCA;
    CSR_Segment3 sab;
    CSR_Segment3 sbc;
    CSR_Segment3 sca;

    // get line segments from each polygon edge
    sab.m_Start = pPo->m_Vertex[0];
    sab.m_End   = pPo->m_Vertex[1];
    sbc.m_Start = pPo->m_Vertex[1];
    sbc.m_End   = pPo->m_Vertex[2];
    sca.m_Start = pPo->m_Vertex[2];
    sca.m_End   = pPo->m_Vertex[0];

    // calculate the projections points on each edge of the triangle
    csrSeg3ClosestPoint(&sab, pP, &rab);
    csrSeg3ClosestPoint(&sbc, pP, &rbc);
    csrSeg3ClosestPoint(&sca, pP, &rca);

    // calculate the distances between points below and test point
    csrVec3Sub(pP, &rab, &vAB);
    csrVec3Sub(pP, &rbc, &vBC);
    csrVec3Sub(pP, &rca, &vCA);

    // calculate the length of each segments
    csrVec3Length(&vAB, &dAB);
    csrVec3Length(&vBC, &dBC);
    csrVec3Length(&vCA, &dCA);

    // calculate the shortest distance
    min = dAB;
    *pR = rab;

    // check if dBC is shortest
    if (dBC < min)
    {
        min = dBC;
        *pR = rbc;
    }

    // check if dCA is shortest
    if (dCA < min)
        *pR = rca;
}
//---------------------------------------------------------------------------
// Box functions
//---------------------------------------------------------------------------
void csrBoxExtendToPolygon(const CSR_Polygon3* pPolygon,
                                 CSR_Box*      pBox,
                                 int*          pEmpty)
{
    unsigned i;

    // iterate through polygon vertices
    for (i = 0; i < 3; ++i)
    {
        // is box empty?
        if (*pEmpty)
        {
            // initialize bounding box with first vertex
             pBox->m_Min = pPolygon->m_Vertex[i];
             pBox->m_Max = pPolygon->m_Vertex[i];
            *pEmpty      = 0;
            continue;
        }

        // search for box min edge
        csrMathMin(pBox->m_Min.m_X, pPolygon->m_Vertex[i].m_X, &pBox->m_Min.m_X);
        csrMathMin(pBox->m_Min.m_Y, pPolygon->m_Vertex[i].m_Y, &pBox->m_Min.m_Y);
        csrMathMin(pBox->m_Min.m_Z, pPolygon->m_Vertex[i].m_Z, &pBox->m_Min.m_Z);

        // search for box max edge
        csrMathMax(pBox->m_Max.m_X, pPolygon->m_Vertex[i].m_X, &pBox->m_Max.m_X);
        csrMathMax(pBox->m_Max.m_Y, pPolygon->m_Vertex[i].m_Y, &pBox->m_Max.m_Y);
        csrMathMax(pBox->m_Max.m_Z, pPolygon->m_Vertex[i].m_Z, &pBox->m_Max.m_Z);
    }
}
//---------------------------------------------------------------------------
void csrBoxCut(const CSR_Box* pBox, CSR_Box* pLeftBox, CSR_Box* pRightBox)
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
            pLeftBox->m_Min     = pBox->m_Min;
            pLeftBox->m_Max     = pBox->m_Max;
            pLeftBox->m_Max.m_X = pBox->m_Min.m_X + (x / 2.0f);

            pRightBox->m_Min     = pBox->m_Min;
            pRightBox->m_Max     = pBox->m_Max;
            pRightBox->m_Min.m_X = pBox->m_Min.m_X + (x / 2.0f);
            break;

        // cut on y axis
        case 1:
            pLeftBox->m_Min     = pBox->m_Min;
            pLeftBox->m_Max     = pBox->m_Max;
            pLeftBox->m_Max.m_Y = pBox->m_Min.m_Y + (y / 2.0f);

            pRightBox->m_Min     = pBox->m_Min;
            pRightBox->m_Max     = pBox->m_Max;
            pRightBox->m_Min.m_Y = pBox->m_Min.m_Y + (y / 2.0f);
            break;

        // cut on z axis
        case 2:
            pLeftBox->m_Min     = pBox->m_Min;
            pLeftBox->m_Max     = pBox->m_Max;
            pLeftBox->m_Max.m_Z = pBox->m_Min.m_Z + (z / 2.0f);

            pRightBox->m_Min     = pBox->m_Min;
            pRightBox->m_Max     = pBox->m_Max;
            pRightBox->m_Min.m_Z = pBox->m_Min.m_Z + (z / 2.0f);
            break;
    }
}
//---------------------------------------------------------------------------
// Inside checks
//---------------------------------------------------------------------------
int csrInsidePolygon2(const CSR_Vector2* pP, const CSR_Polygon2* pPo)
{
    CSR_Vector2 nPToV1;
    CSR_Vector2 nPToV2;
    CSR_Vector2 nPToV3;
    float       a1;
    float       a2;
    float       a3;
    float       angleResult;

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
    * calculate the vectors between the point p and each polygon vertex, then calculate the angle
    * formed by each of these vectors. If the sum of the angles is equal to a complete circle, i.e.
    * 2 * PI in radians, then the point p is inside the polygon limits, otherwise the point is
    * outside. It is assumed that the point to check belongs to the polygon's plane
    */
    csrVec2Sub(&pPo->m_Vertex[0], pP, &nPToV1);
    csrVec2Sub(&pPo->m_Vertex[1], pP, &nPToV2);
    csrVec2Sub(&pPo->m_Vertex[2], pP, &nPToV3);
    csrVec2Normalize(&nPToV1, &nPToV1);
    csrVec2Normalize(&nPToV2, &nPToV2);
    csrVec2Normalize(&nPToV3, &nPToV3);

    // calculate the angles using the scalar product of each vectors with the following formulas:
    // A1 = NPToV1.x * NPToV2.x + NPToV1.y * NPToV2.y
    // A2 = NPToV2.x * NPToV3.x + NPToV2.y * NPToV3.y
    // A3 = NPToV3.x * NPToV1.x + NPToV3.y * NPToV1.y
    csrVec2Dot(&nPToV1, &nPToV2, &a1);
    csrVec2Dot(&nPToV2, &nPToV3, &a2);
    csrVec2Dot(&nPToV3, &nPToV1, &a3);

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
//---------------------------------------------------------------------------
int csrInsidePolygon3(const CSR_Vector3* pP, const CSR_Polygon3* pPo)
{
    CSR_Vector3 nPToV1;
    CSR_Vector3 nPToV2;
    CSR_Vector3 nPToV3;
    float       a1;
    float       a2;
    float       a3;
    float       angleResult;

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
    * calculate the vectors between the point p and each polygon vertex, then calculate the angle
    * formed by each of these vectors. If the sum of the angles is equal to a complete circle, i.e.
    * 2 * PI in radians, then the point p is inside the polygon limits, otherwise the point is
    * outside. It is assumed that the point to check belongs to the polygon's plane
    */
    csrVec3Sub(&pPo->m_Vertex[0], pP, &nPToV1);
    csrVec3Sub(&pPo->m_Vertex[1], pP, &nPToV2);
    csrVec3Sub(&pPo->m_Vertex[2], pP, &nPToV3);
    csrVec3Normalize(&nPToV1, &nPToV1);
    csrVec3Normalize(&nPToV2, &nPToV2);
    csrVec3Normalize(&nPToV3, &nPToV3);

    // calculate the angles using the scalar product of each vectors with the following formulas:
    // A1 = NPToV1.x * NPToV2.x + NPToV1.y * NPToV2.y + NPToV1.z * NPToV2.z
    // A2 = NPToV2.x * NPToV3.x + NPToV2.y * NPToV3.y + NPToV2.z * NPToV3.z
    // A3 = NPToV3.x * NPToV1.x + NPToV3.y * NPToV1.y + NPToV3.z * NPToV1.z
    csrVec3Dot(&nPToV1, &nPToV2, &a1);
    csrVec3Dot(&nPToV2, &nPToV3, &a2);
    csrVec3Dot(&nPToV3, &nPToV1, &a3);

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
//---------------------------------------------------------------------------
int csrInsideRect(const CSR_Vector2* pP, const CSR_Rect* pR)
{
    return (pP->m_X >= pR->m_Min.m_X && pP->m_Y >= pR->m_Min.m_Y &&
            pP->m_X <= pR->m_Max.m_X && pP->m_Y <= pR->m_Max.m_Y);
}
//---------------------------------------------------------------------------
int csrInsideCircle(const CSR_Vector2* pP, const CSR_Circle* pC)
{
    CSR_Vector2 length;
    float       distance;

    // calculate the distance between test point and the center of the circle
    csrVec2Sub(pP, &pC->m_Center, &length);
    csrVec2Length(&length, &distance);

    // check if distance is shorter than the circle radius and return result
    return (distance <= pC->m_Radius);
}
//---------------------------------------------------------------------------
int csrInsideBox(const CSR_Vector3* pP, const CSR_Box* pB)
{
    return (pP->m_X >= pB->m_Min.m_X && pP->m_Y >= pB->m_Min.m_Y && pP->m_Z >= pB->m_Min.m_Z &&
            pP->m_X <= pB->m_Max.m_X && pP->m_Y <= pB->m_Max.m_Y && pP->m_Z <= pB->m_Max.m_Z);
}
//---------------------------------------------------------------------------
int csrInsideSphere(const CSR_Vector3* pP, const CSR_Sphere* pS)
{
    float       distance;
    CSR_Vector3 length;

    // calculate the distance between test point and the center of the sphere
    csrVec3Sub(pP, &pS->m_Center, &length);
    csrVec3Length(&length, &distance);

    // check if distance is shorter than the radius of the sphere and return result
    return (distance <= pS->m_Radius);
}
//---------------------------------------------------------------------------
// Intersection checks
//---------------------------------------------------------------------------
int csrIntersect2(const CSR_Figure2* pFigure1,
                  const CSR_Figure2* pFigure2,
                        CSR_Vector2* pP1,
                        CSR_Vector2* pP2)
{
    int         intersectionType;
    const void* pFirst;
    const void* pSecond;

    // intersection calculation isn't possible if at least one of the figures is missing
    if (!pFigure1 || !pFigure2 || !pFigure1->m_pFigure || !pFigure2->m_pFigure)
        return 0;

    // determine which intersection type should be applied
    switch (pFigure1->m_Type)
    {
        case CSR_F2_Line:
            switch (pFigure2->m_Type)
            {
                case CSR_F2_Line:    intersectionType =  0; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Ray:     intersectionType =  1; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Segment: intersectionType =  2; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Polygon: intersectionType =  3; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Rect:    intersectionType =  4; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Circle:  intersectionType =  5; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F2_Ray:
            switch (pFigure2->m_Type)
            {
                case CSR_F2_Line:    intersectionType =  1;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Ray:     intersectionType =  6;  pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Segment: intersectionType =  7;  pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Polygon: intersectionType =  8;  pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Rect:    intersectionType =  9;  pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Circle:  intersectionType =  10; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F2_Segment:
            switch (pFigure2->m_Type)
            {
                case CSR_F2_Line:    intersectionType =  2;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Ray:     intersectionType =  7;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Segment: intersectionType =  11; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Polygon: intersectionType =  12; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Rect:    intersectionType =  13; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Circle:  intersectionType =  14; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F2_Polygon:
            switch (pFigure2->m_Type)
            {
                case CSR_F2_Line:    intersectionType =  3;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Ray:     intersectionType =  8;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Segment: intersectionType =  12; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Polygon: intersectionType =  15; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Rect:    intersectionType =  16; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Circle:  intersectionType =  17; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F2_Rect:
            switch (pFigure2->m_Type)
            {
                case CSR_F2_Line:    intersectionType =  4;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Ray:     intersectionType =  9;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Segment: intersectionType =  11; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Polygon: intersectionType =  16; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Rect:    intersectionType =  18; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F2_Circle:  intersectionType =  19; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F2_Circle:
            switch (pFigure2->m_Type)
            {
                case CSR_F2_Line:    intersectionType =  5;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Ray:     intersectionType =  10; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Segment: intersectionType =  14; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Polygon: intersectionType =  17; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Rect:    intersectionType =  19; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F2_Circle:  intersectionType =  20; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        default:
            intersectionType = -1;
            break;
    }

    // unknown intersection type?
    if (intersectionType == -1)
        return 0;

    // calculate the intersection
    switch (intersectionType)
    {
        // rect-rect intersection
        case 18:
        {
            // get the figures to check
            const CSR_Rect* pRect1 = (CSR_Rect*)pFirst;
            const CSR_Rect* pRect2 = (CSR_Rect*)pSecond;

            return !(pRect1->m_Min.m_X <= pRect2->m_Max.m_X && pRect1->m_Max.m_X >= pRect2->m_Min.m_X &&
                     pRect1->m_Min.m_Y <= pRect2->m_Max.m_Y && pRect1->m_Max.m_Y >= pRect2->m_Min.m_Y);
        }

        // circle-circle intersection
        case 20:
        {
            CSR_Vector2 dist;
            float       length;

            // get the figures to check
            const CSR_Circle* pCircle1 = (CSR_Circle*)pFirst;
            const CSR_Circle* pCircle2 = (CSR_Circle*)pSecond;

            // calculate the distance between the both circle centers
            dist.m_X = fabs(pCircle1->m_Center.m_X - pCircle2->m_Center.m_X);
            dist.m_Y = fabs(pCircle1->m_Center.m_Y - pCircle2->m_Center.m_Y);

            // calculate the length between the both circle centers
            csrVec2Length(&dist, &length);

            // the circles are in collision if the length between their centers is lower than or
            // equal to the sum of the both sphere radius
            return (length <= (pCircle1->m_Radius + pCircle2->m_Radius));
        }

        default:
            // unknown or unsupported
            return 0;
    }
}
//---------------------------------------------------------------------------
int csrIntersect3(const CSR_Figure3* pFigure1,
                  const CSR_Figure3* pFigure2,
                        CSR_Vector3* pR1,
                        CSR_Vector3* pR2,
                        CSR_Plane*   pR3)
{
    int         intersectionType;
    const void* pFirst;
    const void* pSecond;

    // intersection calculation isn't possible if at least one of the figures is missing
    if (!pFigure1 || !pFigure2 || !pFigure1->m_pFigure || !pFigure2->m_pFigure)
        return 0;

    // determine which intersection type should be applied
    switch (pFigure1->m_Type)
    {
        case CSR_F3_Line:
            switch (pFigure2->m_Type)
            {
                case CSR_F3_Line:    intersectionType =  0; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Ray:     intersectionType =  1; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Segment: intersectionType =  2; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Plane:   intersectionType =  3; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Polygon: intersectionType =  4; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Box:     intersectionType =  5; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Sphere:  intersectionType =  6; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F3_Ray:
            switch (pFigure2->m_Type)
            {
                case CSR_F3_Line:    intersectionType =  1;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Ray:     intersectionType =  7;  pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Segment: intersectionType =  8;  pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Plane:   intersectionType =  9;  pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Polygon: intersectionType =  10; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Box:     intersectionType =  11; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Sphere:  intersectionType =  12; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F3_Segment:
            switch (pFigure2->m_Type)
            {
                case CSR_F3_Line:    intersectionType =  2;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Ray:     intersectionType =  8;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Segment: intersectionType =  13; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Plane:   intersectionType =  14; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Polygon: intersectionType =  15; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Box:     intersectionType =  16; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Sphere:  intersectionType =  17; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F3_Plane:
            switch (pFigure2->m_Type)
            {
                case CSR_F3_Line:    intersectionType =  3;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Ray:     intersectionType =  9;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Segment: intersectionType =  14; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Plane:   intersectionType =  18; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Polygon: intersectionType =  19; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Box:     intersectionType =  20; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Sphere:  intersectionType =  21; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F3_Polygon:
            switch (pFigure2->m_Type)
            {
                case CSR_F3_Line:    intersectionType =  4;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Ray:     intersectionType =  10; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Segment: intersectionType =  15; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Plane:   intersectionType =  19; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Polygon: intersectionType =  22; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Box:     intersectionType =  23; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Sphere:  intersectionType =  24; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F3_Box:
            switch (pFigure2->m_Type)
            {
                case CSR_F3_Line:    intersectionType =  5;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Ray:     intersectionType =  11; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Segment: intersectionType =  16; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Plane:   intersectionType =  20; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Polygon: intersectionType =  23; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Box:     intersectionType =  25; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                case CSR_F3_Sphere:  intersectionType =  26; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        case CSR_F3_Sphere:
            switch (pFigure2->m_Type)
            {
                case CSR_F3_Line:    intersectionType =  6;  pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Ray:     intersectionType =  12; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Segment: intersectionType =  17; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Plane:   intersectionType =  21; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Polygon: intersectionType =  24; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Box:     intersectionType =  26; pFirst = pFigure2->m_pFigure; pSecond = pFigure1->m_pFigure; break;
                case CSR_F3_Sphere:  intersectionType =  27; pFirst = pFigure1->m_pFigure; pSecond = pFigure2->m_pFigure; break;
                default:             intersectionType = -1;
            }

            break;

        default:
            intersectionType = -1;
            break;
    }

    // unknown intersection type?
    if (intersectionType == -1)
        return 0;

    // calculate the intersection
    switch (intersectionType)
    {
        // ray-plane intersection
        case 9:
        {
            CSR_Vector3 n;
            float       dot;
            float       nDot;
            float       temp;

            // get the figures to check
            const CSR_Ray3*  pRay   = (CSR_Ray3*)pFirst;
            const CSR_Plane* pPlane = (CSR_Plane*)pSecond;

            // get the normal of the plane
            n.m_X = pPlane->m_A;
            n.m_Y = pPlane->m_B;
            n.m_Z = pPlane->m_C;

            // calculate the angle between the line and the normal to the plane
            csrVec3Dot(&n, &pRay->m_Dir, &dot);

            // if normal to the plane is perpendicular to the line, then the line is either parallel
            // to the plane and there are no solutions or the line is on the plane in which case
            // there are an infinite number of solutions
            if (!dot)
                return 0;

            csrVec3Dot(&n, &pRay->m_Pos, &nDot);

            temp = ((pPlane->m_D + nDot) / dot);

            // calculate the intersection point
            if (pR1)
            {
                pR1->m_X = (pRay->m_Pos.m_X - (temp * pRay->m_Dir.m_X));
                pR1->m_Y = (pRay->m_Pos.m_Y - (temp * pRay->m_Dir.m_Y));
                pR1->m_Z = (pRay->m_Pos.m_Z - (temp * pRay->m_Dir.m_Z));
            }

            return 1;
        }

        // ray-polygon intersection
        case 10:
        {
            CSR_Plane     polygonPlane;
            CSR_Vector3   pointOnPlane;
            CSR_Figure3   plane;

            // get the polygon to check
            const CSR_Polygon3* pPolygon = (CSR_Polygon3*)pSecond;

            // create a plane using the 3 vertices of the polygon
            csrPlaneFromPoints(&pPolygon->m_Vertex[0],
                               &pPolygon->m_Vertex[1],
                               &pPolygon->m_Vertex[2],
                               &polygonPlane);

            // build a figure for the plane
            plane.m_Type    = CSR_F3_Plane;
            plane.m_pFigure = &polygonPlane;

            // calculate the intersection point
            if (!csrIntersect3(pFigure1, &plane, &pointOnPlane, pR2, pR3))
                return 0;

            // check if calculated point is inside the polygon
            if (csrInsidePolygon3(&pointOnPlane, pPolygon))
            {
                if (pR1)
                    *pR1 = pointOnPlane;

                return 1;
            }

            return 0;
        }

        // ray-box intersection
        case 11:
        {
            float tX1;
            float tX2;
            float tY1;
            float tY2;
            float tZ1;
            float tZ2;
            float tXn;
            float tXf;
            float tYn;
            float tYf;
            float tZn;
            float tZf;
            float tNear;
            float tFar;

            // get the figures to check
            const CSR_Ray3* pRay = (CSR_Ray3*)pFirst;
            const CSR_Box*  pBox = (CSR_Box*)pSecond;

            // get infinite value
            const float inf = 1.0f / 0.0f;

            // calculate nearest point where ray intersects box on x coordinate
            if (pRay->m_InvDir.m_X != inf)
                tX1 = ((pBox->m_Min.m_X - pRay->m_Pos.m_X) * pRay->m_InvDir.m_X);
            else
            if ((pBox->m_Min.m_X - pRay->m_Pos.m_X) < 0.0f)
                tX1 = -inf;
            else
                tX1 =  inf;

            // calculate farthest point where ray intersects box on x coordinate
            if (pRay->m_InvDir.m_X != inf)
                tX2 = ((pBox->m_Max.m_X - pRay->m_Pos.m_X) * pRay->m_InvDir.m_X);
            else
            if ((pBox->m_Max.m_X - pRay->m_Pos.m_X) < 0.0f)
                tX2 = -inf;
            else
                tX2 =  inf;

            // calculate nearest point where ray intersects box on y coordinate
            if (pRay->m_InvDir.m_Y != inf)
                tY1 = ((pBox->m_Min.m_Y - pRay->m_Pos.m_Y) * pRay->m_InvDir.m_Y);
            else
            if ((pBox->m_Min.m_Y - pRay->m_Pos.m_Y) < 0.0f)
                tY1 = -inf;
            else
                tY1 =  inf;

            // calculate farthest point where ray intersects box on y coordinate
            if (pRay->m_InvDir.m_Y != inf)
                tY2 = ((pBox->m_Max.m_Y - pRay->m_Pos.m_Y) * pRay->m_InvDir.m_Y);
            else
            if ((pBox->m_Max.m_Y - pRay->m_Pos.m_Y) < 0.0f)
                tY2 = -inf;
            else
                tY2 =  inf;

            // calculate nearest point where ray intersects box on z coordinate
            if (pRay->m_InvDir.m_Z != inf)
                tZ1 = ((pBox->m_Min.m_Z - pRay->m_Pos.m_Z) * pRay->m_InvDir.m_Z);
            else
            if ((pBox->m_Min.m_Z - pRay->m_Pos.m_Z) < 0.0f)
                tZ1 = -inf;
            else
                tZ1 =  inf;

            // calculate farthest point where ray intersects box on z coordinate
            if (pRay->m_InvDir.m_Z != inf)
                tZ2 = ((pBox->m_Max.m_Z - pRay->m_Pos.m_Z) * pRay->m_InvDir.m_Z);
            else
            if ((pBox->m_Max.m_Z - pRay->m_Pos.m_Z) < 0.0f)
                tZ2 = -inf;
            else
                tZ2 =  inf;

            // calculate near/far intersection on each axis
            csrMathMin(tX1, tX2, &tXn);
            csrMathMax(tX1, tX2, &tXf);
            csrMathMin(tY1, tY2, &tYn);
            csrMathMax(tY1, tY2, &tYf);
            csrMathMin(tZ1, tZ2, &tZn);
            csrMathMax(tZ1, tZ2, &tZf);

            // calculate final near/far intersection point
            csrMathMax(tYn, tZn,   &tNear);
            csrMathMax(tXn, tNear, &tNear);
            csrMathMin(tYf, tZf,   &tFar);
            csrMathMin(tXf, tFar,  &tFar);

            // check if ray intersects box
            return (tFar >= tNear);
        }

        // segment-plane intersection
        case 14:
        {
            CSR_Vector3 point;
            CSR_Vector3 dir;
            CSR_Ray3    ray;
            CSR_Figure3 rayFigure;

            // get the segment to check
            const CSR_Segment3* pSegment = (CSR_Segment3*)pFirst;

            // get the ray start position
            ray.m_Pos = pSegment->m_Start;

            // calculate the ray direction. NOTE the inverted direction can be omitted here because
            // this value will not be used by the csrIntersectRayPlane() function
            csrVec3Sub(&pSegment->m_End, &pSegment->m_Start, &dir);
            csrVec3Normalize(&dir, &ray.m_Dir);

            // build a figure for the plane
            rayFigure.m_Type    = CSR_F3_Ray;
            rayFigure.m_pFigure = &ray;

            // check the intersection. NOTE the segment will only intersect the plane if the
            // intersection point is inside the segment limits
            if (csrIntersect3(&rayFigure, pSecond, &point, pR2, pR3))
                if (csrVec3BetweenRange(&point, &pSegment->m_Start, &pSegment->m_End, M_CSR_Epsilon))
                {
                    if (pR1)
                        *pR1 = point;

                    return 1;
                }

            return 0;
        }

        // segment-polygon intersection
        case 15:
        {
            CSR_Figure3 plane;
            CSR_Plane   polygonPlane;
            CSR_Vector3 pointOnPlane;
            CSR_Vector3 point;

            // get the figures to check
            const CSR_Polygon3* pPolygon = (CSR_Polygon3*)pSecond;

            // create a plane using the 3 vertices of the polygon
            csrPlaneFromPoints(&pPolygon->m_Vertex[0],
                               &pPolygon->m_Vertex[1],
                               &pPolygon->m_Vertex[2],
                               &polygonPlane);

            // build a figure for the plane
            plane.m_Type    = CSR_F3_Plane;
            plane.m_pFigure = &polygonPlane;

            // calculate the intersection point
            if (!csrIntersect3(pFigure1, &plane, &pointOnPlane, pR2, pR3))
                return 0;

            // check if calculated point is inside the polygon
            if (csrInsidePolygon3(&pointOnPlane, pPolygon))
            {
                if (pR1)
                    *pR1 = pointOnPlane;

                return 1;
            }

            return 0;
        }

        // polygon-sphere intersection
        case 24:
        {
            float        testPoint1;
            float        testPoint2;
            CSR_Vector3  sphereNormal;
            CSR_Vector3  pointOnSphere;
            CSR_Vector3  pointOnPlane;
            CSR_Vector3  pointOnTriangle;
            CSR_Plane    polygonPlane;
            CSR_Segment3 seg;
            CSR_Figure3  segment;
            CSR_Figure3  plane;

            // get the figures to check
            const CSR_Polygon3* pPolygon = (CSR_Polygon3*)pFirst;
            const CSR_Sphere*   pSphere  = (CSR_Sphere*)pSecond;

            // create a plane using the 3 vertices of the polygon
            csrPlaneFromPoints(&pPolygon->m_Vertex[0],
                               &pPolygon->m_Vertex[1],
                               &pPolygon->m_Vertex[2],
                               &polygonPlane);

            // calculate the distance between the center of the sphere and the plane
            csrPlaneDistanceTo(&pSphere->m_Center, &polygonPlane, &testPoint1);

            // calculate the normal of the distance sphere-plane using the positive or negative
            // value of the calculated distance between plane and position
            if (testPoint1 < 0.0f)
            {
                sphereNormal.m_X = polygonPlane.m_A;
                sphereNormal.m_Y = polygonPlane.m_B;
                sphereNormal.m_Z = polygonPlane.m_C;
            }
            else
            {
                sphereNormal.m_X = -polygonPlane.m_A;
                sphereNormal.m_Y = -polygonPlane.m_B;
                sphereNormal.m_Z = -polygonPlane.m_C;
            }

            // calculate the point who the segment from center of sphere in the direction of the
            // plane will cross the border of the sphere
            pointOnSphere.m_X = pSphere->m_Center.m_X + (pSphere->m_Radius * sphereNormal.m_X);
            pointOnSphere.m_Y = pSphere->m_Center.m_Y + (pSphere->m_Radius * sphereNormal.m_Y),
            pointOnSphere.m_Z = pSphere->m_Center.m_Z + (pSphere->m_Radius * sphereNormal.m_Z);

            // calculate the distance between the border of the sphere and the plane
            csrPlaneDistanceTo(&pointOnSphere, &polygonPlane, &testPoint2);

            // if the test points are on each side of the plane, then the sphere cross the plane. We
            // assume that the segment from the center of the sphere to the direction of the plane
            // can never be coplanar
            if ((testPoint1 <= 0.0f && testPoint2 >= 0.0f) || (testPoint2 <= 0.0f && testPoint1 >= 0.0f))
            {
                // calculate who the segment cross the plane
                if (!testPoint1)
                    // if testPoint1 is equal to 0, the center of the sphere cross the plane
                    pointOnPlane = pSphere->m_Center;
                else
                if (!testPoint2)
                    // if testPoint2 is equal to 0, the border of the sphere cross the plane
                    pointOnPlane = pointOnSphere;
                else
                {
                    seg.m_Start = pSphere->m_Center;
                    seg.m_End   = pointOnSphere;

                    // build a figure for the segment
                    segment.m_Type    = CSR_F3_Segment;
                    segment.m_pFigure = &polygonPlane;

                    // build a figure for the plane
                    plane.m_Type    = CSR_F3_Plane;
                    plane.m_pFigure = &polygonPlane;

                    // calculate the intersection point
                    csrIntersect3(&segment, &plane, &pointOnPlane, pR2, pR3);
                }

                // check if calculated point is inside the polygon
                if (csrInsidePolygon3(&pointOnPlane, pPolygon))
                {
                    // yes, the sphere collide the polygon. Copy the plane and return 1
                    if (pR3)
                        *pR3 = polygonPlane;

                    return 1;
                }
                else
                {
                    // otherwise check if the sphere collide a polygon edge. First calculate the
                    // point to check on the polygon edge
                    csrPolygon3ClosestPoint(&pointOnPlane, pPolygon, &pointOnTriangle);

                    // check if this point is inside the sphere
                    if (csrInsideSphere(&pointOnTriangle, pSphere))
                    {
                        // yes, the sphere collide the polygon. In this case the sliding plane is
                        // copied and the return value is 1
                        if (pR3)
                            *pR3 = polygonPlane;

                        return 1;
                    }
                }
            }

            // no collision was found
            return 0;
        }

        // box-box intersection
        case 25:
        {
            // get the figures to check
            const CSR_Box* pBox1 = (CSR_Box*)pFirst;
            const CSR_Box* pBox2 = (CSR_Box*)pSecond;

            return !(pBox1->m_Min.m_X <= pBox2->m_Max.m_X &&
                     pBox1->m_Max.m_X >= pBox2->m_Min.m_X &&
                     pBox1->m_Min.m_Y <= pBox2->m_Max.m_Y &&
                     pBox1->m_Max.m_Y >= pBox2->m_Min.m_Y &&
                     pBox1->m_Min.m_Z <= pBox2->m_Max.m_Z &&
                     pBox1->m_Max.m_Z >= pBox2->m_Min.m_Z);
        }

        // box-sphere intersection
        case 26:
        {
            float d = 0.0f;

            // get the figures to check
            const CSR_Box*    pBox    = (CSR_Box*)pFirst;
            const CSR_Sphere* pSphere = (CSR_Sphere*)pSecond;

            // find the square of the distance from the sphere to the box on the x axis
            if (pSphere->m_Center.m_X < pBox->m_Min.m_X)
                d += pow(pSphere->m_Center.m_X - pBox->m_Min.m_X, 2.0f);
            else
            if (pSphere->m_Center.m_X > pBox->m_Max.m_X)
                d += pow(pSphere->m_Center.m_X - pBox->m_Max.m_X, 2.0f);

            // find the square of the distance from the sphere to the box on the y axis
            if (pSphere->m_Center.m_Y < pBox->m_Min.m_Y)
                d += pow(pSphere->m_Center.m_Y - pBox->m_Min.m_Y, 2.0f);
            else
            if (pSphere->m_Center.m_Y > pBox->m_Max.m_Y)
                d += pow(pSphere->m_Center.m_Y - pBox->m_Max.m_Y, 2.0f);

            // find the square of the distance from the sphere to the box on the z axis
            if (pSphere->m_Center.m_Z < pBox->m_Min.m_Z)
                d += pow(pSphere->m_Center.m_Z - pBox->m_Min.m_Z, 2.0f);
            else
            if (pSphere->m_Center.m_Z > pBox->m_Max.m_Z)
                d += pow(pSphere->m_Center.m_Z - pBox->m_Max.m_Z, 2.0f);

            return (d <= pow(pSphere->m_Radius, 2.0f));
        }

        // sphere-sphere intersection
        case 27:
        {
            CSR_Vector3 dist;
            float       length;

            // get the figures to check
            const CSR_Sphere* pSphere1 = (CSR_Sphere*)pFirst;
            const CSR_Sphere* pSphere2 = (CSR_Sphere*)pSecond;

            // calculate the distance between the both sphere centers
            dist.m_X = fabs(pSphere1->m_Center.m_X - pSphere2->m_Center.m_X);
            dist.m_Y = fabs(pSphere1->m_Center.m_Y - pSphere2->m_Center.m_Y);
            dist.m_Z = fabs(pSphere1->m_Center.m_Z - pSphere2->m_Center.m_Z);

            // calculate the length between the both sphere centers
            csrVec3Length(&dist, &length);

            // the spheres are in collision if the length between their centers is lower than or equal to
            // the sum of the both sphere radius
            return (length <= (pSphere1->m_Radius + pSphere2->m_Radius));
        }

        default:
            // unknown or unsupported
            return 0;
    }
}
//---------------------------------------------------------------------------
