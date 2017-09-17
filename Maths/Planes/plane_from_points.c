/*****************************************************************************
 * ==> Calculate a plane from 3 points --------------------------------------*
 *****************************************************************************
 * Description : Calculate a plane from 3 points                             *
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
struct QR_Plane
{
    float m_A;
    float m_B;
    float m_C;
    float m_D;
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
void Cross(QR_Vector3* pV1, QR_Vector3* pV2, QR_Vector3* pR)
{
    pR->m_X = (pV1->m_Y * pV2->m_Z) - (pV2->m_Y * pV1->m_Z);
    pR->m_Y = (pV1->m_Z * pV2->m_X) - (pV2->m_Z * pV1->m_X);
    pR->m_Z = (pV1->m_X * pV2->m_Y) - (pV2->m_X * pV1->m_Y);
}
//------------------------------------------------------------------------------
void PlaneFromPointNormal(QR_Vector3* pP, QR_Vector3* pN, QR_Plane* pR)
{
    // the a, b, and c components are only the normal of the plane, and the D
    // component can be calculated using the aX + bY + cZ + d = 0 algorithm
    pR->m_A = pN->m_X;
    pR->m_B = pN->m_Y;
    pR->m_C = pN->m_Z;
    pR->m_D = -((pN->m_X * pP->m_X) + (pN->m_Y * pP->m_Y) + (pN->m_Z * pP->m_Z));
}
//------------------------------------------------------------------------------
void PlaneFromPoints(QR_Vector3* pV1, QR_Vector3* pV2, QR_Vector3* pV3, QR_Plane* pR)
{
    // calculate edge vectors
    QR_Vector3 e1;
    e1.m_X = pV2->m_X - pV1->m_X;
    e1.m_Y = pV2->m_Y - pV1->m_Y;
    e1.m_Z = pV2->m_Z - pV1->m_Z;

    QR_Vector3 e2;
    e2.m_X = pV3->m_X - pV1->m_X;
    e2.m_Y = pV3->m_Y - pV1->m_Y;
    e2.m_Z = pV3->m_Z - pV1->m_Z;

    // calculate the normal of the plane
    QR_Vector3 normal;
    Cross(&e1, &e2, &normal);
    Normalize(&normal, &normal);

    // calculate and return the plane
    PlaneFromPointNormal(pV1, &normal, pR);
}
//------------------------------------------------------------------------------
void main()
{
    QR_Vector3 p1;
    p1.m_X = 4.23f;
    p1.m_Y = 2.57f;
    p1.m_Z = 3.99f;

    QR_Vector3 p2;
    p2.m_X = 8.27f;
    p2.m_Y = 3.33f;
    p2.m_Z = 5.29f;

    QR_Vector3 p3;
    p3.m_X = 4.56f;
    p3.m_Y = 1.23f;
    p3.m_Z = 7.89f;

    QR_Plane plane;
    PlaneFromPoints(&p1, &p2, &p3, &plane);

    printf("p1 - x = %f, y = %f, z = %f\n", p1.m_X, p1.m_Y, p1.m_Z);
    printf("p2 - x = %f, y = %f, z = %f\n", p2.m_X, p2.m_Y, p2.m_Z);
    printf("p3 - x = %f, y = %f, z = %f\n", p3.m_X, p3.m_Y, p3.m_Z);
    printf("The plane of these 3 points is - a = %f, b = %f, c = %f, d = %f\n",
           plane.m_A,
           plane.m_B,
           plane.m_C,
           plane.m_D);
}
//------------------------------------------------------------------------------
