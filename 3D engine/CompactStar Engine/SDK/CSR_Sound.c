/****************************************************************************
 * ==> CSR_Sound -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the functions to play sound and music *
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

#include "CSR_Sound.h"

// std
#include <stdlib.h>
#include <stdio.h>

//---------------------------------------------------------------------------
// Sound functions
//---------------------------------------------------------------------------
int csrSoundInitializeOpenAL(ALCdevice** pOpenALDevice, ALCcontext** pOpenALContext)
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
//---------------------------------------------------------------------------
void csrSoundReleaseOpenAL(ALCdevice* pOpenALDevice, ALCcontext* pOpenALContext)
{
    // destroy OpenAL context
    if (pOpenALContext)
        alcDestroyContext(pOpenALContext);

    // close the device
    if (pOpenALDevice)
        alcCloseDevice(pOpenALDevice);
}
//---------------------------------------------------------------------------
CSR_Sound* csrSoundCreate(const ALCdevice*  pOpenALDevice,
                          const ALCcontext* pOpenALContext,
                                CSR_Buffer* pBuffer,
                                unsigned    sampling)
{
    CSR_Sound* pSound;

    // no sound file to load?
    if (!pBuffer || !pBuffer->m_pData || !pBuffer->m_Length)
        return 0;

    // no OpenAL device?
    if (!pOpenALDevice)
        return 0;

    // no OpenAL context?
    if (!pOpenALContext)
        return 0;

    // create a new sound container
    pSound = (CSR_Sound*)malloc(sizeof(CSR_Sound));

    // succeeded?
    if (!pSound)
        return 0;

    // initialize the sound content
    csrSoundInit(pSound);

    // grab a buffer ID from openAL
    alGenBuffers(1, &pSound->m_BufferID);

    // succeeded?
    if (alGetError() != AL_NO_ERROR)
    {
        csrSoundRelease(pSound);
        return 0;
    }

    // jam the audio data into the new buffer
    alBufferData(pSound->m_BufferID,
                 AL_FORMAT_STEREO16,
                 pBuffer->m_pData,
                 (ALsizei)pBuffer->m_Length,
                 sampling);

    // succeeded?
    if (alGetError() != AL_NO_ERROR)
    {
        csrSoundRelease(pSound);
        return 0;
    }

    // grab a source ID from openAL
    alGenSources(1, &pSound->m_ID);

    // succeeded?
    if (alGetError() != AL_NO_ERROR)
    {
        csrSoundRelease(pSound);
        return 0;
    }

    // attach the buffer to the source
    alSourcei(pSound->m_ID, AL_BUFFER, pSound->m_BufferID);

    // succeeded?
    if (alGetError() != AL_NO_ERROR)
    {
        csrSoundRelease(pSound);
        return 0;
    }

    // set some basic source preferences
    alSourcef(pSound->m_ID, AL_GAIN,  1.0f);
    alSourcef(pSound->m_ID, AL_PITCH, 1.0f);

    return pSound;
}
//---------------------------------------------------------------------------
void csrSoundRelease(CSR_Sound* pSound)
{
    // no sound to release?
    if (!pSound)
        return;

    // delete source
    if (pSound->m_ID != M_OPENAL_ERROR_ID)
        alDeleteSources(1, &pSound->m_ID);

    // delete buffer
    if (pSound->m_BufferID != M_OPENAL_ERROR_ID)
        alDeleteBuffers(1, &pSound->m_BufferID);

    // free the sound
    free(pSound);
}
//---------------------------------------------------------------------------
CSR_Sound* csrSoundOpenWavFile(const ALCdevice*  pOpenALDevice,
                               const ALCcontext* pOpenALContext,
                               const char*       pFileName)
{
    CSR_Buffer* pBuffer;
    CSR_Sound*  pSound;

    // open the sound file
    pBuffer = csrFileOpen(pFileName);

    // create the sound from the Wav file content
    pSound = csrSoundOpenWavBuffer(pOpenALDevice, pOpenALContext, pBuffer);

    // release the file buffer (no longer required)
    csrBufferRelease(pBuffer);

    return pSound;
}
//---------------------------------------------------------------------------
CSR_Sound* csrSoundOpenWavBuffer(const ALCdevice*  pOpenALDevice,
                                 const ALCcontext* pOpenALContext,
                                 const CSR_Buffer* pBuffer)
{
    #ifdef _MSC_VER
        CSR_Buffer    dataBuffer   = {0};
        CSR_Sound*    pSound;
        size_t        offset;
        unsigned char signature[4] = {0};
        unsigned      sampleRate;
    #else
        CSR_Buffer    dataBuffer;
        CSR_Sound*    pSound;
        size_t        offset;
        unsigned char signature[4];
        unsigned      sampleRate;
    #endif

    // no buffer?
    if (!pBuffer)
        return 0;

    // is buffer empty?
    if (!pBuffer->m_Length)
        return 0;

    offset = 0;

    // read RIFF signature
    csrBufferRead(pBuffer, &offset, sizeof(unsigned char), 4, &signature[0]);

    // check the RIFF signature
    if (signature[0] != 'R' || signature[1] != 'I' || signature[2] != 'F' || signature[3] != 'F')
        return 0;

    // skip 4 next bytes (this is the entire WAV data length)
    offset += 4;

    // read WAVE signature
    csrBufferRead(pBuffer, &offset, sizeof(unsigned char), 4, &signature[0]);

    // check the WAVE signature
    if (signature[0] != 'W' || signature[1] != 'A' || signature[2] != 'V' || signature[3] != 'E')
        return 0;

    // skip next bytes until the sample rate
    offset += 12;

    // read the sample rate
    csrBufferRead(pBuffer, &offset, sizeof(unsigned), 1, &sampleRate);

    // skip all the following header data
    offset += 16;

    // check for data corruption
    if (offset >= pBuffer->m_Length)
        return 0;

    // populate a pseudo-buffer to read the sound data
    dataBuffer.m_pData  = ((unsigned char*)pBuffer->m_pData) + offset;
    dataBuffer.m_Length = pBuffer->m_Length - offset;

    // create the sound from the file content
    pSound = csrSoundCreate(pOpenALDevice, pOpenALContext, &dataBuffer, sampleRate);

    return pSound;
}
//---------------------------------------------------------------------------
void csrSoundInit(CSR_Sound* pSound)
{
    // no sound to initialize?
    if (!pSound)
        return;

    // initialize the sound content
    pSound->m_BufferID = M_OPENAL_ERROR_ID;
    pSound->m_ID       = M_OPENAL_ERROR_ID;
}
//---------------------------------------------------------------------------
int csrSoundPlay(CSR_Sound* pSound)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    alSourcePlay(pSound->m_ID);
    return 1;
}
//---------------------------------------------------------------------------
int csrSoundPause(CSR_Sound* pSound)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    alSourcePause(pSound->m_ID);
    return 1;
}
//---------------------------------------------------------------------------
int csrSoundStop(CSR_Sound* pSound)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    alSourceStop(pSound->m_ID);
    return 1;
}
//---------------------------------------------------------------------------
int csrSoundIsPlaying(CSR_Sound* pSound)
{
    ALenum state;

    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    alGetSourcei(pSound->m_ID, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING)
        return 1;

    return 0;
}
//---------------------------------------------------------------------------
int csrSoundChangePitch(CSR_Sound* pSound, float value)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    if (value >= 0.0f && value <= 1.0f)
    {
        alSourcef(pSound->m_ID, AL_PITCH, value);
        return 1;
    }

    return 0;
}
//---------------------------------------------------------------------------
int csrSoundChangeVolume(CSR_Sound* pSound, float value)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    if (value >= 0.0f && value <= 1.0f)
    {
        alSourcef(pSound->m_ID, AL_GAIN, value);
        return 1;
    }

    return 0;
}
//---------------------------------------------------------------------------
int csrSoundChangeVolumeMin(CSR_Sound* pSound, float value)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    if (value >= 0.0f && value <= 1.0f)
    {
        alSourcef(pSound->m_ID, AL_MIN_GAIN, value);
        return 1;
    }

    return 0;
}
//---------------------------------------------------------------------------
int csrSoundChangeVolumeMax(CSR_Sound* pSound, float value)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    if (value >= 0.0f && value <= 1.0f)
    {
        alSourcef(pSound->m_ID, AL_MAX_GAIN, value);
        return 1;
    }

    return 0;
}
//---------------------------------------------------------------------------
int csrSoundChangePosition(CSR_Sound* pSound, const CSR_Vector3* pPos)
{
    #ifdef _MSC_VER
        ALfloat position[3] = {0};
    #else
        ALfloat position[3];
    #endif

    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return 0;

    position[0] = pPos->m_X;
    position[1] = pPos->m_Y;
    position[2] = pPos->m_Z;

    alSourcefv(pSound->m_ID, AL_POSITION, position);
    return 1;
}
//---------------------------------------------------------------------------
void csrSoundLoop(CSR_Sound* pSound, int value)
{
    if (!pSound || pSound->m_ID == M_OPENAL_ERROR_ID)
        return;

    if (value)
        alSourcei(pSound->m_ID, AL_LOOPING, AL_TRUE);
    else
        alSourcei(pSound->m_ID, AL_LOOPING, AL_FALSE);
}
//---------------------------------------------------------------------------
