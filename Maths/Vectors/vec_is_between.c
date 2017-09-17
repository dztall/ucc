/*****************************************************************************
 * ==> Calculate if a vector is between a line segment ----------------------*
 *****************************************************************************
 * Description : Calculate if a vector is between a line segment             *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

// std
#include <stdlib.h>
#include <string.h>
#include <math.h>

//------------------------------------------------------------------------------
struct QR_Vector3
{
    float m_X; // x coordinate for the 3D vector
    float m_Y; // y coordinate for the 3D vector
    float m_Z; // z coordinate for the 3D vector
};
//------------------------------------------------------------------------------
void Min(float* pA, float* pB, float* pR)
{
    if (*pA < *pB)
    {
        *pR = *pA;
        return;
    }

    *pR = *pB;
}
//------------------------------------------------------------------------------
void Max(float* pA, float* pB, float* pR)
{
    if (*pA > *pB)
    {
        *pR = *pA;
        return;
    }

    *pR = *pB;
}
//------------------------------------------------------------------------------
int ValueIsBetween(float* pV, float* pS, float* pE, float* pEpsylon)
{
    float minVal;
    Min(pS, pE, &minVal);

    float maxVal;
    Max(pS, pE, &maxVal);

    // check if each value is between start and end limits considering tolerance
    if (*pV >= (minVal - *pEpsylon) && *pV <= (maxVal + *pEpsylon))
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
int VectorIsBetween(QR_Vector3* pP, QR_Vector3* pS, QR_Vector3* pE, float* pEpsylon)
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
//------------------------------------------------------------------------------
void main()
{
    QR_Vector3 vec1;
    vec1.m_X = 1.0f;
    vec1.m_Y = 0.0f;
    vec1.m_Z = 0.0f;

    QR_Vector3 vec2;
    vec2.m_X = 10.0f;
    vec2.m_Y = 0.0f;
    vec2.m_Z = 0.0f;

    QR_Vector3 p;
    p.m_X = 5.0f;
    p.m_Y = 0.0f;
    p.m_Z = 0.0f;

    float epsylon = 1.0E-3;
    int   between = VectorIsBetween(&p, &vec1, &vec2, &epsylon);

    printf("vec1 - x = %f, y = %f, z = %f\n", vec1.m_X, vec1.m_Y, vec1.m_Z);
    printf("vec2 - x = %f, y = %f, z = %f\n", vec2.m_X, vec2.m_Y, vec2.m_Z);
    printf("p - x = %f, y = %f, z = %f\n", p.m_X, p.m_Y, p.m_Z);

    if (between)
        printf("p is between vec 1 and vec2");
    else
        printf("p isn't between vec 1 and vec2");
}
//------------------------------------------------------------------------------
