/****************************************************************************
 * ==> CSR_Renderer_Metal_Types --------------------------------------------*
 ****************************************************************************
 * Description : This module contains the common structures and data to     *
 *               share between the renderer and the shader                  *
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

#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <simd/simd.h>

/**
* Uniform structure containing the matrice slots
*/
typedef struct
{
    matrix_float4x4 m_ProjectionMatrix;
    matrix_float4x4 m_ViewMatrix;
    matrix_float4x4 m_ModelMatrix;
} CSR_Uniforms;

#endif
