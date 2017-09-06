/*****************************************************************************
 * ==> Calculate normal of a vector -----------------------------------------*
 *****************************************************************************
 * Description : Calculates the normal of a vector                           *
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
void Length(QR_Vector3* pV, float* pR)
{
    *pR = sqrt((pV->m_X * pV->m_X) + (pV->m_Y * pV->m_Y) + (pV->m_Z * pV->m_Z));
}
//------------------------------------------------------------------------------
void Normalize(QR_Vector3* pV, QR_Vector3* pR)
{
    float len;
    Length(pV, &len);

    if (len == 0.0f)
    {
        pR->m_X = 0.0f;
        pR->m_Y = 0.0f;
        pR->m_Z = 0.0f;
        return;
    }

    pR->m_X = (pV->m_X / len);
    pR->m_Y = (pV->m_Y / len);
    pR->m_Z = (pV->m_Z / len);
}
//------------------------------------------------------------------------------
void main()
{
    QR_Vector3 vec;
    vec.m_X = 4.23f;
    vec.m_Y = 2.57f;
    vec.m_Z = 3.99f;

    printf("vec before normalize - x = %f, y = %f, z = %f\n", vec.m_X, vec.m_Y, vec.m_Z);

    QR_Vector3 nVec;
    Normalize(&vec, &nVec);

    printf("vec after normalize - x = %f, y = %f, z = %f\n", nVec.m_X, nVec.m_Y, nVec.m_Z);
}
//------------------------------------------------------------------------------
