/*****************************************************************************
 * ==> Calculate if a point is inside a polygon -----------------------------*
 *****************************************************************************
 * Description : Calculate if a point is inside a polygon                    *
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
void Sub(QR_Vector3* pV1, QR_Vector3* pV2, QR_Vector3* pR)
{
    pR->m_X = pV1->m_X - pV2->m_X;
    pR->m_Y = pV1->m_Y - pV2->m_Y;
    pR->m_Z = pV1->m_Z - pV2->m_Z;
}
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
void Dot(QR_Vector3* pV1, QR_Vector3* pV2, float* pR)
{
    *pR = ((pV1->m_X * pV2->m_X) + (pV1->m_Y * pV2->m_Y) + (pV1->m_Z * pV2->m_Z));
}
//------------------------------------------------------------------------------
int Inside(QR_Vector3* pP, QR_Vector3* pV1, QR_Vector3* pV2, QR_Vector3* pV3)
{
    /*
    * here we check if the point P is inside the polygon
    *
    *              Collision                 No collision
    *                  V1                         V1
    *                  /\                         /\
    *                 /  \                       /  \
    *                / *P \                  *P /    \
    *               /      \                   /      \
    *            V2 -------- V3             V2 -------- V3
    *
    * we calculate the segments between the P point and each vertex of the
    * polygon and we normalize this segment. For that we uses the following
    * algorithms:
    * -> PToV1 = Polygon.Vertex1 - Point;
    * -> PToV2 = Polygon.Vertex2 - Point;
    * -> PToV3 = Polygon.Vertex3 - Point;
    */
    QR_Vector3 nPToV1;
    QR_Vector3 nPToV2;
    QR_Vector3 nPToV3;
    Sub(pV1, pP, &nPToV1);
    Sub(pV2, pP, &nPToV2);
    Sub(pV3, pP, &nPToV3);
    Normalize(&nPToV1, &nPToV1);
    Normalize(&nPToV2, &nPToV2);
    Normalize(&nPToV3, &nPToV3);

    // calculate the angles using the scalar product of each vectors. We use
    // the following algorithms:
    // A1 = NPToV1.x * NPToV2.x + NPToV1.y * NPToV2.y + NPToV1.z * NPToV2.z
    // A2 = NPToV2.x * NPToV3.x + NPToV2.y * NPToV3.y + NPToV2.z * NPToV3.z
    // A3 = NPToV3.x * NPToV1.x + NPToV3.y * NPToV1.y + NPToV3.z * NPToV1.z
    float a1;
    float a2;
    float a3;
    Dot(&nPToV1, &nPToV2, &a1);
    Dot(&nPToV2, &nPToV3, &a2);
    Dot(&nPToV3, &nPToV1, &a3);

    // calculate the sum of all angles
    float angleResult = acos(a1) + acos(a2) + acos(a3);

    // if sum is equal or higher to 6.28 radians then point P is inside polygon
    if (angleResult >= 6.28f)
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
void main()
{
    QR_Vector3 vec1;
    vec1.m_X = -5.0f;
    vec1.m_Y = -5.0f;
    vec1.m_Z =  0.0f;

    QR_Vector3 vec2;
    vec2.m_X = 0.0f;
    vec2.m_Y = 5.0f;
    vec2.m_Z = 0.0f;

    QR_Vector3 vec3;
    vec3.m_X =  5.0f;
    vec3.m_Y = -5.0f;
    vec3.m_Z =  0.0f;

    QR_Vector3 point;
    point.m_X = 1.0f;
    point.m_Y = 0.0f;
    point.m_Z = 0.0f;

    QR_Vector3 point2;
    point2.m_X = 0.0f;
    point2.m_Y = 0.0f;
    point2.m_Z = 0.0f;

    QR_Vector3 point3;
    point3.m_X = 5.0f;
    point3.m_Y = 5.0f;
    point3.m_Z = 0.0f;

    if (Inside(&point, &vec1, &vec2, &vec3))
        printf("Point is inside the polygon\n");
    else
        printf("Point is not inside the polygon\n");

    if (Inside(&point2, &vec1, &vec2, &vec3))
        printf("Point is inside the polygon\n");
    else
        printf("Point is not inside the polygon\n");

    if (Inside(&point3, &vec1, &vec2, &vec3))
        printf("Point is inside the polygon\n");
    else
        printf("Point is not inside the polygon\n");
}
//------------------------------------------------------------------------------
