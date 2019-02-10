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

#ifndef CSR_PhysicsH
#define CSR_PhysicsH

// std
#include <stddef.h>

// compactStar engine
#include "CSR_Geometry.h"

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_CSR_Gravitation 9.81f

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Physical body
*/
typedef struct
{
    CSR_Vector3 m_Velocity; // velocity (in m/s)
    float       m_Mass;     // mass (in Kilograms)
} CSR_Body;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Body functions
        //-------------------------------------------------------------------

        /**
        * Creates a body
        *@return newly created body, 0 on error
        *@note The body must be released when no longer used, see csrBodyRelease()
        */
        CSR_Body* csrBodyCreate(void);

        /**
        * Releases a body
        *@param[in, out] pBody - body to release
        */
        void csrBodyRelease(CSR_Body* pBody);

        /**
        * Initializes a body
        *@param[in, out] pBody - body to initialize
        */
        void csrBodyInit(CSR_Body* pBody);

        //-------------------------------------------------------------------
        // Physics functions
        //-------------------------------------------------------------------

        /**
        * Converts the weight of a body to a mass
        *@param weight - body weight, in Newtons
        *@param[out] pMass - body mass, in kilograms
        */
        void csrPhysicsWeightToMass(float weight, float* pMass);

        /**
        * Converts the mass of a body to a weight
        *@param mass - body mass, in kilograms
        *@param[out] pWeight - body weight, in Newtons
        */
        void csrPhysicsMassToWeight(float mass, float* pWeight);

        /**
        * Calculates the gravity force applied on an object
        *@param mass - object mass for which the gravitation force should be calculated in kilograms
        *@param[out] pF - resulting force in Newtons
        */
        void csrPhysicsGravity(float mass, float* pF);

        /**
        * Calculates the new velocity of a body rolling against a slope
        *@param pSlopeDir - slope direction (or slope normal)
        *@param mass - body mass
        *@param friction - friction factor to apply to the body
        *@param elapsedTime - elapsed time since last calculation in milliseconds
        *@param[in, out] pVelocity - last known velocity, new velocity on function ends
        */
        void csrPhysicsRoll(const CSR_Vector3* pSlopeDir,
                                  float        mass,
                                  float        friction,
                                  float        elapsedTime,
                                  CSR_Vector3* pVelocity);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Physics.c"
#endif

#endif
