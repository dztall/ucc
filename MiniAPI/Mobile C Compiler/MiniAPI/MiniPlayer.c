/*****************************************************************************
 * ==> MiniPlayer -----------------------------------------------------------*
 *****************************************************************************
 * Description : This module provides the functions to play sound and music  *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#include "MiniPlayer.h"

// std
#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------------------
// Player functions
//----------------------------------------------------------------------------
int miniInitializeOpenAL(ALCdevice** pOpenALDevice, ALCcontext** pOpenALContext)
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
//----------------------------------------------------------------------------
int miniLoadSoundBuffer(const char*           pFileName,
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
    pFile = M_MINI_FILE_OPEN((const char*)pFileName, "rb");

    // succeeded?
    if (!pFile)
        return 0;

    // get bytes from file and put them into the data buffer
    M_MINI_FILE_READ(*pBuffer, sizeof(unsigned char), fileSize, pFile);

     // close file
    M_MINI_FILE_CLOSE(pFile);

    return 1;
}
//----------------------------------------------------------------------------
int miniCreateSound(const ALCdevice*     pOpenALDevice,
                    const ALCcontext*    pOpenALContext,
                          unsigned char* pBuffer,
                          unsigned int   fileSize,
                          unsigned int   sampling,
                          ALuint*        pBufferID,
                          ALuint*        pID)
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
//----------------------------------------------------------------------------
int miniPlaySound(ALuint id)
{
    if (id == M_OPENAL_ERROR_ID)
        return 0;

    alSourcePlay(id);
    return 1;
}
//----------------------------------------------------------------------------
int miniPauseSound(ALuint id)
{
    if (id == M_OPENAL_ERROR_ID)
        return 0;

    alSourcePause(id);
    return 1;
}
//----------------------------------------------------------------------------
int miniStopSound(ALuint id)
{
    if (id == M_OPENAL_ERROR_ID)
        return 0;

    alSourceStop(id);
    return 1;
}
//----------------------------------------------------------------------------
int miniIsSoundPlaying(ALuint id)
{
    ALenum state;

    if (id == M_OPENAL_ERROR_ID)
        return 0;

    alGetSourcei(id, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING)
        return 1;

    return 0;
}
//----------------------------------------------------------------------------
int miniChangeSoundPitch(ALuint id, float* pValue)
{
    if (id == M_OPENAL_ERROR_ID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_PITCH, *pValue);
        return 1;
    }

    return 0;
}
//----------------------------------------------------------------------------
int miniChangeSoundVolume(ALuint id, float* pValue)
{
    if (id == M_OPENAL_ERROR_ID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_GAIN, *pValue);
        return 1;
    }

    return 0;
}
//----------------------------------------------------------------------------
int miniChangeSoundVolumeMin(ALuint id, float* pValue)
{
    if (id == M_OPENAL_ERROR_ID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_MIN_GAIN, *pValue);
        return 1;
    }

    return 0;
}
//----------------------------------------------------------------------------
int miniChangeSoundVolumeMax(ALuint id, float* pValue)
{
    if (id == M_OPENAL_ERROR_ID)
        return 0;

    if (*pValue >= 0.0f && *pValue <= 1.0f)
    {
        alSourcef(id, AL_MAX_GAIN, *pValue);
        return 1;
    }

    return 0;
}
//----------------------------------------------------------------------------
int miniChangeSoundPosition(ALuint id, float* pX, float* pY, float* pZ)
{
    ALfloat position[3];

    if (id == M_OPENAL_ERROR_ID)
        return 0;

    position[0] = *pX;
    position[1] = *pY;
    position[2] = *pZ;

    alSourcefv(id, AL_POSITION, position);
    return 1;
}
//----------------------------------------------------------------------------
void miniLoopSound(ALuint id, int value)
{
    if (id != M_OPENAL_ERROR_ID)
        if (value)
            alSourcei(id, AL_LOOPING, AL_TRUE);
        else
            alSourcei(id, AL_LOOPING, AL_FALSE);
}
//----------------------------------------------------------------------------
void miniReleaseSound(ALuint bufferID, ALuint id)
{
    // delete source
    if (id != M_OPENAL_ERROR_ID)
        alDeleteSources(1, &id);

    // delete buffer
    if (bufferID != M_OPENAL_ERROR_ID)
        alDeleteBuffers(1, &bufferID);
}
//----------------------------------------------------------------------------
void miniReleaseOpenAL(ALCdevice* pOpenALDevice, ALCcontext* pOpenALContext)
{
    // release context
    if (pOpenALContext)
        alcDestroyContext(pOpenALContext);

    // close device
    if (pOpenALDevice)
        alcCloseDevice(pOpenALDevice);
}
//----------------------------------------------------------------------------
