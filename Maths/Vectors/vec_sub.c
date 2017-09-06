/*****************************************************************************
 * ==> Subtract a value from a vector ---------------------------------------*
 *****************************************************************************
 * Description : Subtract a value from a vector                              *
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
void Sub(QR_Vector3* pV, float* pValue, QR_Vector3* pR)
{
    pR->m_X = pV->m_X - *pValue;
    pR->m_Y = pV->m_Y - *pValue;
    pR->m_Z = pV->m_Z - *pValue;
}
//------------------------------------------------------------------------------
void main()
{
    QR_Vector3 vec;
    vec.m_X = 4.23f;
    vec.m_Y = 2.57f;
    vec.m_Z = 3.99f;

    printf("vec before sub - x = %f, y = %f, z = %f\n", vec.m_X, vec.m_Y, vec.m_Z);

    float value = 1.0f;
    Sub(&vec, &value, &vec);

    printf("vec after sub - x = %f, y = %f, z = %f\n", vec.m_X, vec.m_Y, vec.m_Z);
}
//------------------------------------------------------------------------------
