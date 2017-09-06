/*****************************************************************************
 * ==> Minimal OpenAL player library ----------------------------------------*
 *****************************************************************************
 * Description : Minimal OpenAL player library                               *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not                 *
 *****************************************************************************/

#ifndef MiniPlayerH
#define MiniPlayerH

// std
#include <stdio.h>
#include <stdlib.h>

// OpenAL library
#ifdef __CODEGEARC__
    #include <al.h>
    #include <alc.h>
#else
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#endif

// mini API
#include "MiniCommon.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

const ALuint g_OpenALErrorID = 0xFFFFFFFF;

//-----------------------------------------------------------------------------
// OpenAL functions
//-----------------------------------------------------------------------------

/**
* Initializes OpenAL library
*@param[out] pOpenALDevice - newly created OpenAL device
*@param[out] pOpenALContext - newly created OpenAL context
*@return 1 on success, otherwise 0
*/
int InitializeOpenAL(ALCdevice** pOpenALDevice, ALCcontext** pOpenALContext)
{
    // select the "preferred device"
    *pOpenALDevice = alcOpenDevice(0);

    // found it?
    if (!pOpenALDevice)
        return 0;

    // use the device to make a context
    *pOpenALContext = alcCreateContext(*pOpenALDevice, 0);

    // found it?
    if (!(*pOpenALContext))
        return 0;

    // set context to the currently active one
    alcMakeContextCurrent(*pOpenALContext);

    return 1;
}

/**
* Loads sound from a .wav file and put it into a buffer
*@param pFileName - wav file name
*@param fileSize - wav file size
*@param[out] pBuffer - buffer containing the wav data
*@return 1 on success, otherwise 0
*/
int LoadSoundBuffer(unsigned char*  pFileName,
                    unsigned int    fileSize,
                    unsigned char** pBuffer)
{
    FILE* pFile;

    // no file name or file size?
    if (!pFileName || !fileSize)
        return 0;

    // no buffer?
    if (!pBuffer)
        return 0;

    // try to open file
    pFile = MINI_FILE_OPEN(pFileName, "rb");

    // succeeded?
    if (!pFile)
        return 0;

    // get bytes from file and put them into the data buffer
    MINI_FILE_READ(*pBuffer, sizeof(unsigned char), fileSize, pFile);

     // close file
    MINI_FILE_CLOSE(pFile);

    return 1;
}

/**
* Creates a sound form a wav buffer
*@param pOpenALDevice - OpenAL device to use
*@param pOpenALContext - OpenAL context to use
*@param pBuffer - buffer containing wav sound
*@param bufferSize - wav buffer size
*@param sampling - sampling to use (standard values are e.g. 48000, 44100, ...)
*@param[out] pBufferID - newly created OpenAL sound buffer identifier (needed to delete the sound)
*@param[out] pID - newly created OpenAL sound identifier
*@return 1 on success, otherwise 0
*/
int CreateSound(const ALCdevice*  pOpenALDevice,
                const ALCcontext* pOpenALContext,
                unsigned char*    pBuffer,
                unsigned int      fileSize,
                unsigned int      sampling,
                ALuint*           pBufferID,
                ALuint*           pID)
{
    ALuint bufferID;
    ALuint id;

    // no sound file to load?
    if (!pBuffer || !fileSize)
        return 0;

    // no OpenAL device?
    if (!pOpenALDevice)
        return 0;

    // no OpenAL context?
    if (!pOpenALContext)
        return 0;

    // grab a buffer ID from openAL
    alGenBuffers(1, &bufferID);

    // jam the audio data into the new buffer
    alBufferData(bufferID,
                 AL_FORMAT_STEREO16,
                 pBuffer,
                 fileSize,
                 sampling);

    // grab a source ID from openAL
    alGenSources(1, &id);

    // attach the buffer to the source
    alSourcei(id, AL_BUFFER, bufferID);

    // set some basic source preferences
    alSourcef(id, AL_GAIN,  1.0f);
    alSourcef(id, AL_PITCH, 1.0f);

    *pBufferID = bufferID;
    *pID       = id;

    return 1;
}

/**
* Plays sound
*@param id - sound identifier to play
*@return 1 on success, otherwise 0
*/
int PlaySound(ALuint id)
{
    if (id == g_OpenALErrorID)
        return 0;

    alSourcePlay(id);
    return 1;
}

/**
* Pauses sound
*@param id - sound identifier to pause
*@return 1 on success, otherwise 0
*/
int PauseSound(ALuint id)
{
    if (id == g_OpenALErrorID)
        return 0;

    alSourcePause(id);
    return 1;
}

/**
* Stops sound
*@param id - sound identifier to stop
*@return 1 on success, otherwise 0
*/
int StopSound(ALuint id)
{
    if (id == g_OpenALErrorID)
        return 0;

    alSourceStop(id);
    return 1;
}

/**
* Checks if sound is currently playing
*@param id - sound identifier to check
*@return 1 if sound is currently playing, otherwise 0
*/
int IsSoundPlaying(ALuint id)
{
    ALenum state;

    if (id == g_OpenALErrorID)
        return 0;

    alGetSourcei(id, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING)
        return 1;

    return 0;
}

/**
* Changes sound pitch
*@param id - sound identifier for which pitch should be modified
*@param pValue - new pitch value (see OpenAL documentation for correct value range)
*@return 1 on success, otherwise 0
*/
int ChangeSoundPitch(ALuint id, float* pValue)
{
    if (id == g_OpenALErrorID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_PITCH, *pValue);
        return 1;
    }

    return 0;
}

/**
* Changes sound volume
*@param id - sound identifier for which volume should be modified
*@param pValue - new volume value (see OpenAL documentation for correct value range)
*@return 1 on success, otherwise 0
*/
int ChangeSoundVolume(ALuint id, float* pValue)
{
    if (id == g_OpenALErrorID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_GAIN, *pValue);
        return 1;
    }

    return 0;
}

/**
* Changes sound minimum volume limit
*@param id - sound identifier for which limit should be modified
*@param pValue - new minimum volume limit value (see OpenAL documentation for correct value range)
*@return 1 on success, otherwise 0
*/
int ChangeSoundVolumeMin(ALuint id, float* pValue)
{
    if (id == g_OpenALErrorID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_MIN_GAIN, *pValue);
        return 1;
    }

    return 0;
}

/**
* Changes sound maximum volume limit
*@param id - sound identifier for which limit should be modified
*@param pValue - new maximum volume limit value (see OpenAL documentation for correct value range)
*@return 1 on success, otherwise 0
*/
int ChangeSoundVolumeMax(ALuint id, float* pValue)
{
    if (id == g_OpenALErrorID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_MAX_GAIN, *pValue);
        return 1;
    }

    return 0;
}

/**
* Set sound source position in 3D environment (e.g. sound can be played on the left)
*@param id - sound identifier to set
*@param pX - sound source x position
*@param pY - sound source y position
*@param pZ - sound source z position
*@return 1 on success, otherwise 0
*/
int ChangeSoundPosition(ALuint id, float* pX, float* pY, float* pZ)
{
    ALfloat position[3];

    if (id == g_OpenALErrorID)
        return 0;

    position[0] = *pX;
    position[1] = *pY;
    position[2] = *pZ;

    alSourcefv(id, AL_POSITION, position);
    return 1;
}

/**
* Loops sound when end is reached
*@param id - sound identifier to loop
*@param value - if 1, sound will be looped on end reached, otherwise sound will be stopped
*@return 1 on success, otherwise 0
*/
void LoopSound(ALuint id, int value)
{
    if (id != g_OpenALErrorID)
        if (value)
            alSourcei(id, AL_LOOPING, AL_TRUE);
        else
            alSourcei(id, AL_LOOPING, AL_FALSE);
}

/**
* Releases sound
*@param bufferID - sound buffer identifier to delete
*@param id - sound identifier to delete
*/
void ReleaseSound(ALuint bufferID, ALuint id)
{
    // delete source
    if (id != g_OpenALErrorID)
        alDeleteSources(1, &id);

    // delete buffer
    if (bufferID != g_OpenALErrorID)
        alDeleteBuffers(1, &bufferID);
}

/**
* Releases OpenAL
*@param pOpenALDevice - OpenAL device to release
*@param pOpenALContext - OpenAL context to release
*/
void ReleaseOpenAL(ALCdevice* pOpenALDevice, ALCcontext* pOpenALContext)
{
    // release context
    if (pOpenALContext)
        alcDestroyContext(pOpenALContext);

    // close device
    if (pOpenALDevice)
        alcCloseDevice(pOpenALDevice);
}

#endif
