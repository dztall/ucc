/****************************************************************************
 * ==> CSR_Physics ---------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides physics algorithms and structures     *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2019, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

#include "CSR_Physics.h"

// std
#include <stdlib.h>
#include <math.h>

//---------------------------------------------------------------------------
// Body functions
//---------------------------------------------------------------------------
CSR_Body* csrBodyCreate(void)
{
    // create a new body
    CSR_Body* pBody = (CSR_Body*)malloc(sizeof(CSR_Body));

    // succeeded?
    if (!pBody)
        return 0;

    // initialize the body content
    csrBodyInit(pBody);

    return pBody;
}
//---------------------------------------------------------------------------
void csrBodyRelease(CSR_Body* pBody)
{
    // no body to release?
    if (!pBody)
        return;

    // free the body
    free(pBody);
}
//---------------------------------------------------------------------------
void csrBodyInit(CSR_Body* pBody)
{
    // no body to initialize?
    if (!pBody)
        return;

    // initialize the body
    pBody->m_Velocity.m_X = 0.0f;
    pBody->m_Velocity.m_Y = 0.0f;
    pBody->m_Velocity.m_Z = 0.0f;
    pBody->m_Mass         = 0.0f;
}
//---------------------------------------------------------------------------
// Physics functions
//---------------------------------------------------------------------------
void csrPhysicsWeightToMass(float weight, float* pMass)
{
    *pMass = weight / M_CSR_Gravitation;
}
//---------------------------------------------------------------------------
void csrPhysicsMassToWeight(float mass, float* pWeight)
{
    *pWeight = mass * M_CSR_Gravitation;
}
//---------------------------------------------------------------------------
void csrPhysicsGravity(float mass, float* pF)
{
    // the formula for the gravitation is F = m * g
    *pF = mass * M_CSR_Gravitation;
}
//---------------------------------------------------------------------------
void csrPhysicsRoll(const CSR_Vector3* pSlopeDir,
                          float        mass,
                          float        friction,
                          float        elapsedTime,
                          CSR_Vector3* pVelocity)
{
    #ifdef _MSC_VER
        float       gravity;
        float       thetaX;
        float       thetaZ;
        float       ffx;
        float       ffz;
        CSR_Vector3 xDir         = {0};
        CSR_Vector3 zDir         = {0};
        CSR_Vector3 acceleration = {0};
    #else
        float       gravity;
        float       thetaX;
        float       thetaZ;
        float       ffx;
        float       ffz;
        CSR_Vector3 xDir;
        CSR_Vector3 zDir;
        CSR_Vector3 acceleration;
    #endif

    // calculate the gravity force to apply to the body
    csrPhysicsGravity(mass, &gravity);

    xDir.m_X = 1.0f;
    xDir.m_Y = 0.0f;
    xDir.m_Z = 0.0f;

    // calculate the slope angle on the x axis
    csrVec3Dot(&xDir, pSlopeDir, &thetaX);

    zDir.m_X = 0.0f;
    zDir.m_Y = 0.0f;
    zDir.m_Z = 1.0f;

    // calculate the slope angle on the z axis
    csrVec3Dot(&zDir, pSlopeDir, &thetaZ);

    // the angles should always be positive
    #ifdef __CODEGEARC__
        thetaX = fabs(thetaX);
        thetaZ = fabs(thetaZ);
    #else
        thetaX = fabsf(thetaX);
        thetaZ = fabsf(thetaZ);
    #endif

    // calculate the friction force to apply to the body (using the formula a = dv / dt)
    if (elapsedTime)
    {
        ffx = (pVelocity->m_X / (elapsedTime * 1.0f)) * friction;
        ffz = (pVelocity->m_Z / (elapsedTime * 1.0f)) * friction;
    }
    else
    {
        ffx = 0.0f;
        ffz = 0.0f;
    }

    // calculate the body acceleration (using the formula a = ((m * g * sin(theta)) - Ff) / m)
    acceleration.m_X = ((gravity * thetaX) - ffx) / mass;
    acceleration.m_Z = ((gravity * thetaZ) - ffz) / mass;

    // calculate the final body velocity (using the formula v = v + (a * dt))
    pVelocity->m_X += (acceleration.m_X * elapsedTime);
    pVelocity->m_Z += (acceleration.m_Z * elapsedTime);
}
//---------------------------------------------------------------------------
