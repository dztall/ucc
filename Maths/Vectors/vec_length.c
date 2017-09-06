/*****************************************************************************
 * ==> Calculate length of a vector -----------------------------------------*
 *****************************************************************************
 * Description : Calculates the length of a vector                           *
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
void main()
{
    QR_Vector3 vec;
    vec.m_X = 4.23f;
    vec.m_Y = 2.57f;
    vec.m_Z = 3.99f;

    float length;
    Length(&vec, &length);

    printf("The length of this vector is %f\n", length);
}
//------------------------------------------------------------------------------
