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

#ifndef CSR_ParticlesH
#define CSR_ParticlesH

// std
#include <stddef.h>

// compactStar engine
#include "CSR_Geometry.h"
#include "CSR_Physics.h"

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

// particle system prototype
typedef struct CSR_Particles CSR_Particles;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Particle
*/
typedef struct
{
    void*        m_pKey;    // model key (in the scene)
    CSR_Matrix4* m_pMatrix; // model matrix
    CSR_Body*    m_pBody;   // particle physical body
} CSR_Particle;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when a motion should be calculated for a particle
*@param pParticles - particle system at which the particle belongs
*@param pParticle - particle for which the motion should be calculated
*@param elapsedTime - elapsed time since last animation, in milliseconds
*/
typedef void (*CSR_fOnCalculateMotion)(const CSR_Particles* pParticles,
                                             CSR_Particle*  pParticle,
                                             float          elapsedTime);

//---------------------------------------------------------------------------
// Implementation
//---------------------------------------------------------------------------

/**
* Particle system
*/
struct CSR_Particles
{
    CSR_Particle*          m_pParticle;
    size_t                 m_Count;
    CSR_fOnCalculateMotion m_fOnCalculateMotion;
};

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Particle functions
        //-------------------------------------------------------------------

        /**
        * Creates a particle
        *@return newly created particle, 0 on error
        *@note The particle must be released when no longer used, see csrParticleRelease()
        */
        CSR_Particle* csrParticleCreate(void);

        /**
        * Releases a particle
        *@param[in, out] pParticle - particle to release
        */
        void csrParticleRelease(CSR_Particle* pParticle);

        /**
        * Initializes a particle
        *@param[in, out] pParticle - particle to initialize
        */
        void csrParticleInit(CSR_Particle* pParticle);

        //-------------------------------------------------------------------
        // Particles functions
        //-------------------------------------------------------------------

        /**
        * Creates a particle system
        *@return newly created particle system, 0 on error
        *@note The particle must be released when no longer used, see csrParticleRelease()
        */
        CSR_Particles* csrParticlesCreate(void);

        /**
        * Releases a particle system
        *@param[in, out] pParticles - particle system to release
        */
        void csrParticlesRelease(CSR_Particles* pParticles);

        /**
        * Initializes a particle system
        *@param[in, out] pParticles - particle system to initialize
        */
        void csrParticlesInit(CSR_Particles* pParticles);

        /**
        * Adds a new particle to a particle system
        *@param pParticles - particle system in which the particle will be added
        *@return the newly added particle on success, otherwise 0
        */
        CSR_Particle* csrParticlesAdd(CSR_Particles* pParticles);

        /**
        * Gets a particle matching with a model
        *@param pParticles - particle system from which the particle should be get
        *@param pKey - search key, may be any model kind
        *@return particle, 0 if not found or on error
        */
        CSR_Particle* csrParticlesGet(const CSR_Particles* pParticles, const void* pKey);

        /**
        * Deletes a particle matching with a model from a particle system
        *@param pKey - key to delete, may be any model kind
        *@note The particle and all his associated resources will be freed internally. For that
        *      reason the caller should not take care of deleting them
        */
        void csrParticlesDeleteFrom(CSR_Particles* pParticles, const void* pKey);

        /**
        * Animates the particles
        *@param pParticles - particle system for which the particles should be animated
        *@param elapsedTime - elapsed time since last animation, in milliseconds
        */
        void csrParticlesAnimate(CSR_Particles* pParticles, float elapsedTime);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Particles.c"
#endif

#endif
