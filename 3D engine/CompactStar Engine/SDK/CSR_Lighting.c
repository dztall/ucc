/****************************************************************************
 * ==> CSR_Lighting --------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the lighting functions and types      *
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

#include "CSR_Lighting.h"

//---------------------------------------------------------------------------
// Material functions
//---------------------------------------------------------------------------
void csrMaterialInit(CSR_Material* pMaterial)
{
    // no material to initialize?
    if (!pMaterial)
        return;

    // initialize the material content
    pMaterial->m_Color       = 0xFFFFFFFF;
    pMaterial->m_Transparent = 0;
    pMaterial->m_Wireframe   = 0;
}
//---------------------------------------------------------------------------
// Directional light functions
//---------------------------------------------------------------------------
void csrDirectionalLightInit(CSR_DirectionalLight* pLight)
{
    // no light to initialize?
    if (!pLight)
        return;

    // initialize the light content
    pLight->m_Ambient       = 0xFFFFFFFF;
    pLight->m_Color         = 0xFFFFFFFF;
    pLight->m_Direction.m_X = 0.0f;
    pLight->m_Direction.m_Y = 0.0f;
    pLight->m_Direction.m_Z = 0.0f;
}
//---------------------------------------------------------------------------
