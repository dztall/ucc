/*****************************************************************************
 * ==> CSR_Renderer_Metal_Types ---------------------------------------------*
 *****************************************************************************
 * Description : This module contains the common structures and data to      *
 *               share between the renderer and the shader                   *
  * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

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
