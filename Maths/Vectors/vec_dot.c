/*****************************************************************************
 * ==> Calculate dot product of a vector ------------------------------------*
 *****************************************************************************
 * Description : Calculates the dot product of a vector                      *
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
void Dot(QR_Vector3* pV1, QR_Vector3* pV2, float* pR)
{
    *pR = ((pV1->m_X * pV2->m_X) + (pV1->m_Y * pV2->m_Y) + (pV1->m_Z * pV2->m_Z));
}
//------------------------------------------------------------------------------
void main()
{
    QR_Vector3 vec1;
    vec1.m_X = 4.23f;
    vec1.m_Y = 2.57f;
    vec1.m_Z = 3.99f;

    QR_Vector3 vec2;
    vec2.m_X = 8.27f;
    vec2.m_Y = 3.33f;
    vec2.m_Z = 5.29f;

    float dot;
    Dot(&vec1, &vec2, &dot);

    printf("vec1 - x = %f, y = %f, z = %f\n", vec1.m_X, vec1.m_Y, vec1.m_Z);
    printf("vec2 - x = %f, y = %f, z = %f\n", vec2.m_X, vec2.m_Y, vec2.m_Z);
    printf("The dot product of these vectors is %f\n", dot);
}
//------------------------------------------------------------------------------
