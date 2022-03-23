/****************************************************************************
 * ==> CSR_DebugHelper -----------------------------------------------------*
 ****************************************************************************
 * Description : This module provides several debug helper functions        *
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

#ifndef CSR_DebugHelperH
#define CSR_DebugHelperH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"
#ifdef USE_X
    #include "CSR_X.h"
#endif
#ifdef USE_COLLADA
    #include "CSR_Collada.h"
#endif
#ifdef USE_IQM
    #include "CSR_Iqm.h"
#endif
#include "CSR_Renderer.h"
#include "CSR_Renderer_OpenGL.h"

//---------------------------------------------------------------------------
// Global variables
//---------------------------------------------------------------------------

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // X model debug functions
        //-------------------------------------------------------------------

        /**
        * Draws the skeleton of a x model
        *@param pX - x model for which the skeleton should be drawn
        *@param pShader - line shader to use to draw skeleton
        *@param animSetIndex - animation set index
        *#param frameIndex - frame index
        */
        #ifdef USE_X
            void csrDebugDrawSkeletonX(const CSR_X*            pX,
                                       const CSR_OpenGLShader* pShader,
                                             size_t            animSetIndex,
                                             size_t            frameIndex);
        #endif

        //-------------------------------------------------------------------
        // Collada model debug functions
        //-------------------------------------------------------------------

        /**
        * Draws the skeleton of a Collada model
        *@param pCollada - Collada model for which the skeleton should be drawn
        *@param pShader - line shader to use to draw skeleton
        *@param animSetIndex - animation set index
        *#param frameIndex - frame index
        */
        #ifdef USE_COLLADA
            void csrDebugDrawSkeletonCollada(const CSR_Collada*      pCollada,
                                             const CSR_OpenGLShader* pShader,
                                                   size_t            animSetIndex,
                                                   size_t            frameIndex);
        #endif

        //-------------------------------------------------------------------
        // Inter-Quake model debug functions
        //-------------------------------------------------------------------

        /**
        * Draws the skeleton of an IQM model
        *@param pIQM - IQM model for which the skeleton should be drawn
        *@param pShader - line shader to use to draw skeleton
        *@param animSetIndex - animation set index
        *#param frameIndex - frame index
        */
        #ifdef USE_IQM
            void csrDebugDrawSkeletonIQM(const CSR_IQM*          pIQM,
                                         const CSR_OpenGLShader* pShader,
                                               size_t            animSetIndex,
                                               size_t            frameIndex);
        #endif

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_DebugHelper.c"
#endif

#endif
