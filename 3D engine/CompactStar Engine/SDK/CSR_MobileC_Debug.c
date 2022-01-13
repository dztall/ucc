/****************************************************************************
 * ==> CSR_MobileC_Debug ---------------------------------------------------*
 ****************************************************************************
 * Description : This module provides several debug functions for the       *
 *               Mobile C Compiler                                          *
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

#include "CSR_MobileC_Debug.h"

//---------------------------------------------------------------------------
// Vector2 debug functions
//---------------------------------------------------------------------------
void csrVec2Log(const CSR_Vector2* pV)
{
    printf("m_X = %f, m_Y = %f\n", pV->m_X, pV->m_Y);
}
//---------------------------------------------------------------------------
// Vector3 debug functions
//---------------------------------------------------------------------------
void csrVec3Log(const CSR_Vector3* pV)
{
    printf("m_X = %f, m_Y = %f, m_Z = %f\n", pV->m_X, pV->m_Y, pV->m_Z);
}
//---------------------------------------------------------------------------
// Matrix debug functions
//---------------------------------------------------------------------------
void csrMat4Log(const CSR_Matrix4* pM)
{
    printf("m_Table[0][0] = %f, m_Table[1][0] = %f, m_Table[2][0] = %f, m_Table[3][0] = %f\n", pM->m_Table[0][0], pM->m_Table[1][0], pM->m_Table[2][0], pM->m_Table[3][0]);
    printf("m_Table[0][1] = %f, m_Table[1][1] = %f, m_Table[2][1] = %f, m_Table[3][1] = %f\n", pM->m_Table[0][1], pM->m_Table[1][1], pM->m_Table[2][1], pM->m_Table[3][1]);
    printf("m_Table[0][2] = %f, m_Table[1][2] = %f, m_Table[2][2] = %f, m_Table[3][2] = %f\n", pM->m_Table[0][2], pM->m_Table[1][2], pM->m_Table[2][2], pM->m_Table[3][2]);
    printf("m_Table[0][3] = %f, m_Table[1][3] = %f, m_Table[2][3] = %f, m_Table[3][3] = %f\n", pM->m_Table[0][3], pM->m_Table[1][3], pM->m_Table[2][3], pM->m_Table[3][3]);
}
//---------------------------------------------------------------------------
