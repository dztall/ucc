/*****************************************************************************
 * ==> Minimal particles library --------------------------------------------*
 *****************************************************************************
 * Description : Minimal particles library                                   *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not                 *
 *****************************************************************************/

#ifndef MiniParticlesH
#define MiniParticlesH

// std
#include <stdio.h>
#include <stdlib.h>

// mini API
#include "MiniGeometry.h"

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

/**
* Particle
*/
typedef struct
{
    MG_Vector3 m_Position;
    MG_Vector3 m_Direction;
    MG_Vector3 m_Velocity;
} MP_Particle;

/**
* Particles system
*/
typedef struct
{
    MP_Particle* m_pParticles;
    unsigned     m_Count;
} MP_Particles;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

/**
* Emits a new particle
*@param pParticles - particle system in which the new particle will be emitted
*@param pStartPos - particle start position
*@param pStartDir - particle start direction
*@param pStartVelocity - particle start velocity
*@param count - max particles the system contains
*/
MP_Particle* EmitParticle(MP_Particles* pParticles,
                          MG_Vector3*   pStartPos,
                          MG_Vector3*   pStartDir,
                          MG_Vector3*   pStartVelocity,
                          unsigned      count)
{
    unsigned index;

    // no particle system?
    if (!pParticles)
        return 0;

    // is particle system saturated?
    if (pParticles->m_Count >= count)
        return 0;

    // create new particle and add it to system
    if (!pParticles->m_Count)
        pParticles->m_pParticles = (MP_Particle*)malloc(sizeof(MP_Particle));
    else
        pParticles->m_pParticles =
                (MP_Particle*)realloc(pParticles->m_pParticles,
                                     (pParticles->m_Count + 1) * sizeof(MP_Particle));

    // increase system count
    index = pParticles->m_Count;
    ++pParticles->m_Count;

    // initialize newly created particle
    pParticles->m_pParticles[index].m_Position.m_X  = pStartPos->m_X;
    pParticles->m_pParticles[index].m_Position.m_Y  = pStartPos->m_Y;
    pParticles->m_pParticles[index].m_Position.m_Z  = pStartPos->m_Z;
    pParticles->m_pParticles[index].m_Direction.m_X = pStartDir->m_X;
    pParticles->m_pParticles[index].m_Direction.m_Y = pStartDir->m_Y;
    pParticles->m_pParticles[index].m_Direction.m_Z = pStartDir->m_Z;
    pParticles->m_pParticles[index].m_Velocity.m_X  = pStartVelocity->m_X;
    pParticles->m_pParticles[index].m_Velocity.m_Y  = pStartVelocity->m_Y;
    pParticles->m_pParticles[index].m_Velocity.m_Z  = pStartVelocity->m_Z;

    return &pParticles->m_pParticles[index];
}

/**
* Moves a particle
*@param pParticle - particle to move
*@param frameCount - number of frames to apply since the last rendering
*/
void MoveParticle(MP_Particle* pParticle, float frameCount)
{
    // no particle system?
    if (!pParticle)
        return;

    // calculate next particle position
    pParticle->m_Position.m_X += (pParticle->m_Direction.m_X * pParticle->m_Velocity.m_X * frameCount);
    pParticle->m_Position.m_Y += (pParticle->m_Direction.m_Y * pParticle->m_Velocity.m_Y * frameCount);
    pParticle->m_Position.m_Z += (pParticle->m_Direction.m_Z * pParticle->m_Velocity.m_Z * frameCount);
}

/**
* Deletes a particle
*@param pParticles - particle system in which the particle shou;d be deleted
*@param index - index of the particle to delete
*/
void DeleteParticle(MP_Particles* pParticles, unsigned index)
{
    unsigned i;

    // no particle system?
    if (!pParticles)
        return;

    // is particle system empty?
    if (!pParticles->m_Count)
        return;

    // is index out of bounds?
    if (index >= pParticles->m_Count)
        return;

    // move all following particles one position on the left
    if (index != pParticles->m_Count - 1)
        /* unfortunatly memcpy doesn't work for now
        memcpy(pParticles->m_pParticles[index],
               pParticles->m_pParticles[index + 1],
              (pParticles->m_Count - (index + 1)) * sizeof(MP_Particle));
        */
        for (i = index; i < pParticles->m_Count - 1; ++i)
        {
            pParticles->m_pParticles[i].m_Position.m_X  = pParticles->m_pParticles[i + 1].m_Position.m_X;
            pParticles->m_pParticles[i].m_Position.m_Y  = pParticles->m_pParticles[i + 1].m_Position.m_Y;
            pParticles->m_pParticles[i].m_Position.m_Z  = pParticles->m_pParticles[i + 1].m_Position.m_Z;
            pParticles->m_pParticles[i].m_Direction.m_X = pParticles->m_pParticles[i + 1].m_Direction.m_X;
            pParticles->m_pParticles[i].m_Direction.m_Y = pParticles->m_pParticles[i + 1].m_Direction.m_Y;
            pParticles->m_pParticles[i].m_Direction.m_Z = pParticles->m_pParticles[i + 1].m_Direction.m_Z;
            pParticles->m_pParticles[i].m_Velocity.m_X  = pParticles->m_pParticles[i + 1].m_Velocity.m_X;
            pParticles->m_pParticles[i].m_Velocity.m_Y  = pParticles->m_pParticles[i + 1].m_Velocity.m_Y;
            pParticles->m_pParticles[i].m_Velocity.m_Z  = pParticles->m_pParticles[i + 1].m_Velocity.m_Z;
        }

    // delete last particle
    pParticles->m_pParticles =
            (MP_Particle*)realloc(pParticles->m_pParticles,
                                 (pParticles->m_Count - 1) * sizeof(MP_Particle));

    // decrease count
    --pParticles->m_Count;
}

/**
* Clears all particles
*@param pParticles - particle system to clear
*/
void ClearParticles(MP_Particles* pParticles)
{
    // no particle system?
    if (!pParticles)
        return;

    // clear particle system
    free(pParticles->m_pParticles);
    pParticles->m_pParticles = 0;
};

#endif // MiniParticlesH
