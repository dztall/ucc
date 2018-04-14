/****************************************************************************
 * ==> CSR_Lighting --------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the lighting functions and types      *
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

#ifndef CSR_LightingH
#define CSR_LightingH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Material, describes the way a mesh reacts to the light
*/
typedef struct
{
    unsigned m_Color;       // vertex color, applied to all vertices if per-vertex color is disabled
    int      m_Transparent; // whether or not the alpha blending should be activated
    int      m_Wireframe;   // whether or not the vertex buffer should be drawn in wireframe
} CSR_Material;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Material functions
        //-------------------------------------------------------------------

        /**
        * Initializes a material structure
        *@param[in, out] pMaterial - material to initialize
        */
        void csrMaterialInit(CSR_Material* pMaterial);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Lighting.c"
#endif

#endif