/****************************************************************************
 * ==> CSR_Particles -------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a base for a particles system         *
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

#include "CSR_Particles.h"

// std
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
// Particle private functions
//---------------------------------------------------------------------------
CSR_Particle* csrParticleDeleteFrom(CSR_Particle* pParticle, size_t index, size_t count)
{
    CSR_Particle* pNewParticle;

    // no particle list to delete from?
    if (!pParticle)
        return 0;

    // was the last particle in the system?
    if (count == 1)
        // don't create a new particle list
        return 0;

    // create a particle list one item smaller than the existing one
    pNewParticle = (CSR_Particle*)csrMemoryAlloc(0, sizeof(CSR_Particle), count - 1);

    // succeeded?
    if (!pNewParticle)
        return 0;

    // copy all the remaining particles in the new list
    if (!index)
        memcpy(pNewParticle, pParticle + 1, (count - 1) * sizeof(CSR_Particle));
    else
    if (index == (count - 1))
        memcpy(pNewParticle, pParticle, (count - 1) * sizeof(CSR_Particle));
    else
    {
        memcpy(pNewParticle,         pParticle,             sizeof(CSR_Particle) *          index);
        memcpy(pNewParticle + index, pParticle + index + 1, sizeof(CSR_Particle) * (count - index - 1));
    }

    // NOTE the particle content may be released here

    return pNewParticle;
}
//---------------------------------------------------------------------------
// Particle functions
//---------------------------------------------------------------------------
CSR_Particle* csrParticleCreate(void)
{
    // create a new particle
    CSR_Particle* pParticle = (CSR_Particle*)malloc(sizeof(CSR_Particle));

    // succeeded?
    if (!pParticle)
        return 0;

    // initialize the particle content
    csrParticleInit(pParticle);

    return pParticle;
}
//---------------------------------------------------------------------------
void csrParticleRelease(CSR_Particle* pParticle)
{
    // no particle to release?
    if (!pParticle)
        return;

    // free the particle matrix
    if (pParticle->m_pMatrix)
        free(pParticle->m_pMatrix);

    // free the particle physical body
    if (pParticle->m_pBody)
        free(pParticle->m_pBody);

    // free the particle
    free(pParticle);
}
//---------------------------------------------------------------------------
void csrParticleInit(CSR_Particle* pParticle)
{
    // no particle to initialize?
    if (!pParticle)
        return;

    // initialize the particle
    pParticle->m_pKey = 0;

    // initialize the particle content
    pParticle->m_pMatrix = (CSR_Matrix4*)malloc(sizeof(CSR_Matrix4));
    pParticle->m_pBody   = (CSR_Body*)malloc(sizeof(CSR_Body));

    // initialize the model matrix
    csrMat4Identity(pParticle->m_pMatrix);

    // initialize the physical body
    csrBodyInit(pParticle->m_pBody);
}
//---------------------------------------------------------------------------
// Particles functions
//---------------------------------------------------------------------------
CSR_Particles* csrParticlesCreate(void)
{
    // create a new particle system
    CSR_Particles* pParticles = (CSR_Particles*)malloc(sizeof(CSR_Particles));

    // succeeded?
    if (!pParticles)
        return 0;

    // initialize the particle system content
    csrParticlesInit(pParticles);

    return pParticles;
}
//---------------------------------------------------------------------------
void csrParticlesRelease(CSR_Particles* pParticles)
{
    // no particle system to release?
    if (!pParticles)
        return;

    // do free the contained particles?
    if (pParticles->m_pParticle)
    {
        size_t i;

        // free the particles content
        for (i = 0; i < pParticles->m_Count; ++i)
        {
            // free the particle matrix
            if (pParticles->m_pParticle[i].m_pMatrix)
                free(pParticles->m_pParticle[i].m_pMatrix);

            // free the particle physical body
            if (pParticles->m_pParticle[i].m_pBody)
                free(pParticles->m_pParticle[i].m_pBody);
        }

        // free all the contained particles
        free(pParticles->m_pParticle);
    }

    // free the particle system
    free(pParticles);
}
//---------------------------------------------------------------------------
void csrParticlesInit(CSR_Particles* pParticles)
{
    // no particle system to initialize?
    if (!pParticles)
        return;

    // initialize the particle system
    pParticles->m_pParticle          = 0;
    pParticles->m_Count              = 0.0f;
    pParticles->m_fOnCalculateMotion = 0;
}
//---------------------------------------------------------------------------
CSR_Particle* csrParticlesAdd(CSR_Particles* pParticles)
{
    CSR_Particle* pParticle;
    int           index;

    // validate the input
    if (!pParticles)
        return 0;

    // add a new particle to the particles system
    pParticle = (CSR_Particle*)csrMemoryAlloc(pParticles->m_pParticle,
                                              sizeof(CSR_Particle),
                                              pParticles->m_Count + 1);

    // succeeded?
    if (!pParticle)
        return 0;

    // get the particle index to update
    index = (int)pParticles->m_Count;

    // initialize the newly created particle with the default values
    csrParticleInit(&pParticle[index]);

    // add particle to the particle system
    pParticles->m_pParticle = pParticle;
    ++pParticles->m_Count;

    return &pParticle[index];
}
//---------------------------------------------------------------------------
CSR_Particle* csrParticlesGet(const CSR_Particles* pParticles, const void* pKey)
{
    size_t i;

    // validate inputs
    if (!pParticles || !pKey)
        return 0;

    // search in the system particles
    for (i = 0; i < pParticles->m_Count; ++i)
        // found a matching model?
        if (pParticles->m_pParticle[i].m_pKey == pKey)
            return &pParticles->m_pParticle[i];

    // not found
    return 0;
}
//---------------------------------------------------------------------------
void csrParticlesDeleteFrom(CSR_Particles* pParticles, const void* pKey)
{
    size_t i;

    // validate inputs
    if (!pParticles || !pKey)
        return;

    // search in the system particles
    for (i = 0; i < pParticles->m_Count; ++i)
        // found a matching model?
        if (pParticles->m_pParticle[i].m_pKey == pKey)
        {
            // delete the item from the list
            CSR_Particle* pParticle = csrParticleDeleteFrom(pParticles->m_pParticle,
                                                            i,
                                                            pParticles->m_Count);

            // update the particle system content
            free(pParticles->m_pParticle);
            pParticles->m_pParticle = pParticle;
            --pParticles->m_Count;

            return;
        }
}
//---------------------------------------------------------------------------
void csrParticlesAnimate(CSR_Particles* pParticles, float elapsedTime)
{
    size_t i;

    // validate inputs
    if (!pParticles || !pParticles->m_fOnCalculateMotion)
        return;

    // iterate through each particles and calculate his motion
    for (i = 0; i < pParticles->m_Count; ++i)
        pParticles->m_fOnCalculateMotion(pParticles, &pParticles->m_pParticle[i], elapsedTime);
}
//---------------------------------------------------------------------------
