/****************************************************************************
 * ==> CSR_MobileC_Debug ---------------------------------------------------*
 ****************************************************************************
 * Description : This module provides several debug functions for the       *
 *               Mobile C Compiler                                          *
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

#ifndef CSR_MobileC_DebugH
#define CSR_MobileC_DebugH

// std
#include <stddef.h>

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Vector2 debug functions
        //-------------------------------------------------------------------

        /**
        * Log a vector 2 content to the compiler output
        *@param pM - matrix to log
        */
        void csrVec2Log(const CSR_Vector2* pV);

        //-------------------------------------------------------------------
        // Vector3 debug functions
        //-------------------------------------------------------------------

        /**
        * Log a vector 3 content to the compiler output
        *@param pM - matrix to log
        */
        void csrVec3Log(const CSR_Vector3* pV);

        //-------------------------------------------------------------------
        // Matrix debug functions
        //-------------------------------------------------------------------

        /**
        * Log a matrix content to the compiler output
        *@param pM - matrix to log
        */
        void csrMat4Log(const CSR_Matrix4* pM);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_MobileC_Debug.c"
#endif

#endif
