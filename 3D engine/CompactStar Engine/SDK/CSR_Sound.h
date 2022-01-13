/****************************************************************************
 * ==> CSR_Sound -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the functions to play sound and music *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2022, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

#ifndef CSR_SoundH
#define CSR_SoundH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"

// OpenAL library
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#elif defined(__APPLE__)
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#elif defined (__CODEGEARC__) || defined (__GNUC__) || defined(_MSC_VER)
    #include <al.h>
    #include <alc.h>
#endif

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_OPENAL_ERROR_ID 0xFFFFFFFF

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Sound
*/
typedef struct
{
    ALuint m_BufferID;
    ALuint m_ID;
} CSR_Sound;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Sound functions
        //-------------------------------------------------------------------

        /**
        * Initializes OpenAL library
        *@param[out] pOpenALDevice - newly created OpenAL device
        *@param[out] pOpenALContext - newly created OpenAL context
        *@return 1 on success, otherwise 0
        */
        int csrSoundInitializeOpenAL(ALCdevice** pOpenALDevice, ALCcontext** pOpenALContext);

        /**
        * Releases OpenAL
        *@param[in, out] pOpenALDevice - OpenAL device to release
        *@param[in, out] pOpenALContext - OpenAL context to release
        */
        void csrSoundReleaseOpenAL(ALCdevice* pOpenALDevice, ALCcontext* pOpenALContext);

        /**
        * Creates a sound form a wav buffer
        *@param pOpenALDevice - OpenAL device to use
        *@param pOpenALContext - OpenAL context to use
        *@param pBuffer - buffer containing wav sound
        *@param sampling - sound sampling (standard values are e.g. 48000, 44100, ...)
        *@return newly created sound on success, 0 on error
        *@note The buffer must be released when no longer used, see csrReleaseSound()
        */
        CSR_Sound* csrSoundCreate(const ALCdevice*  pOpenALDevice,
                                  const ALCcontext* pOpenALContext,
                                        CSR_Buffer* pBuffer,
                                        unsigned    sampling);

        /**
        * Releases a sound and frees its memory
        *@param[in, out] pSound - sound to release
        */
        void csrSoundRelease(CSR_Sound* pSound);

        /**
        * Opens a sound from a wav file
        *@param pOpenALDevice - OpenAL device to use
        *@param pOpenALContext - OpenAL context to use
        *@param pFileName - wav file name
        *@return opened sound on success, 0 on error
        *@note The sound must be released when no longer used, see csrReleaseSound()
        */
        CSR_Sound* csrSoundOpenWavFile(const ALCdevice*  pOpenALDevice,
                                       const ALCcontext* pOpenALContext,
                                       const char*       pFileName);

        /**
        * Opens a sound from a wav buffer
        *@param pOpenALDevice - OpenAL device to use
        *@param pOpenALContext - OpenAL context to use
        *@param pBuffer - buffer containing the wav file
        *@return opened sound on success, 0 on error
        *@note The sound must be released when no longer used, see csrReleaseSound()
        */
        CSR_Sound* csrSoundOpenWavBuffer(const ALCdevice*  pOpenALDevice,
                                         const ALCcontext* pOpenALContext,
                                         const CSR_Buffer* pBuffer);

        /**
        * Initializes a sound structure
        *@param[in, out] pSound - sound to initialize
        */
        void csrSoundInit(CSR_Sound* pSound);

        /**
        * Plays sound
        *@param pSound - sound to play
        *@return 1 on success, otherwise 0
        */
        int csrSoundPlay(CSR_Sound* pSound);

        /**
        * Pauses sound
        *@param pSound - sound to pause
        *@return 1 on success, otherwise 0
        */
        int csrSoundPause(CSR_Sound* pSound);

        /**
        * Stops sound
        *@param pSound - sound to stop
        *@return 1 on success, otherwise 0
        */
        int csrSoundStop(CSR_Sound* pSound);

        /**
        * Checks if sound is currently playing
        *@param pSound - sound to check
        *@return 1 if sound is currently playing, otherwise 0
        */
        int csrSoundIsPlaying(CSR_Sound* pSound);

        /**
        * Changes sound pitch
        *@param pSound - sound for which pitch should be modified
        *@param pValue - new pitch value (see OpenAL documentation for correct value range)
        *@return 1 on success, otherwise 0
        */
        int csrSoundChangePitch(CSR_Sound* pSound, float pValue);

        /**
        * Changes sound volume
        *@param pSound - sound for which volume should be modified
        *@param value - new volume value (see OpenAL documentation for correct value range)
        *@return 1 on success, otherwise 0
        */
        int csrSoundChangeVolume(CSR_Sound* pSound, float value);

        /**
        * Changes sound minimum volume limit
        *@param pSound - sound for which limit should be modified
        *@param value - new minimum volume limit value (see OpenAL documentation for correct value range)
        *@return 1 on success, otherwise 0
        */
        int csrSoundChangeVolumeMin(CSR_Sound* pSound, float value);

        /**
        * Changes sound maximum volume limit
        *@param pSound - sound for which limit should be modified
        *@param value - new maximum volume limit value (see OpenAL documentation for correct value range)
        *@return 1 on success, otherwise 0
        */
        int csrSoundChangeVolumeMax(CSR_Sound* pSound, float value);

        /**
        * Set sound source position in 3D environment (e.g. sound can be played on the left)
        *@param pSound - sound to set
        *@param pPos - sound source position
        *@return 1 on success, otherwise 0
        */
        int csrSoundChangePosition(CSR_Sound* pSound, const CSR_Vector3* pPos);

        /**
        * Loops sound when end is reached
        *@param pSound - sound to loop
        *@param value - if 1, sound will be looped on end reached, otherwise sound will be stopped
        */
        void csrSoundLoop(CSR_Sound* pSound, int value);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Sound.c"
#endif

#endif
