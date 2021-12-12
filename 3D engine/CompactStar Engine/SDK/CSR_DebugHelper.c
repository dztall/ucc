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

#include "CSR_DebugHelper.h"

//---------------------------------------------------------------------------
// Private functions
//---------------------------------------------------------------------------
void csrDebugDrawBone(const CSR_Bone*              pBone,
                      const CSR_OpenGLShader*      pShader,
                      const CSR_AnimationSet_Bone* pAnimationSet,
                            size_t                 animSetIndex,
                            size_t                 frameIndex,
                            int                    poseOnly)
{
    size_t i;

    if (!pBone)
        return;

    if (!pShader)
        return;

    if (!poseOnly && !pAnimationSet)
        return;

    for (i = 0; i < pBone->m_ChildrenCount; ++i)
    {
        #ifdef _MSC_VER
            CSR_Matrix4 topMatrix    = {0};
            CSR_Matrix4 bottomMatrix = {0};
            CSR_Line    boneLine     = {0};
            CSR_Bone*   pChild       = &pBone->m_pChildren[i];
        #else
            CSR_Matrix4 topMatrix;
            CSR_Matrix4 bottomMatrix;
            CSR_Line    boneLine;
            CSR_Bone*   pChild = &pBone->m_pChildren[i];
        #endif

        // get the bone top matrix
        if (poseOnly)
            csrBoneGetMatrix(pBone, 0, &topMatrix);
        else
            csrBoneGetAnimMatrix(pBone,
                                 &pAnimationSet[animSetIndex],
                                 frameIndex,
                                 0,
                                 &topMatrix);

        // get the bone bottom matrix
        if (poseOnly)
            csrBoneGetMatrix(pChild, 0, &bottomMatrix);
        else
            csrBoneGetAnimMatrix(pChild,
                                 &pAnimationSet[animSetIndex],
                                 frameIndex,
                                 0,
                                 &bottomMatrix);

        boneLine.m_Start.m_X = topMatrix.m_Table[3][0];
        boneLine.m_Start.m_Y = topMatrix.m_Table[3][1];
        boneLine.m_Start.m_Z = topMatrix.m_Table[3][2];

        boneLine.m_End.m_X = bottomMatrix.m_Table[3][0];
        boneLine.m_End.m_Y = bottomMatrix.m_Table[3][1];
        boneLine.m_End.m_Z = bottomMatrix.m_Table[3][2];

        boneLine.m_StartColor.m_R = 0.25f;
        boneLine.m_StartColor.m_G = 0.12f;
        boneLine.m_StartColor.m_B = 0.1f;
        boneLine.m_StartColor.m_A = 1.0f;

        boneLine.m_EndColor.m_R = 0.95f;
        boneLine.m_EndColor.m_G = 0.06f;
        boneLine.m_EndColor.m_B = 0.15f;
        boneLine.m_EndColor.m_A = 1.0f;

        boneLine.m_Width          = 1.0f;
        boneLine.m_Smooth         = 1;
        boneLine.m_CustomModelMat = 1;

        glDisable(GL_DEPTH_TEST);
        csrDrawLine(&boneLine, pShader);
        glEnable(GL_DEPTH_TEST);

        csrDebugDrawBone(pChild, pShader, pAnimationSet, animSetIndex, frameIndex, poseOnly);
    }
}
//---------------------------------------------------------------------------
// X model debug functions
//---------------------------------------------------------------------------
void csrDebugDrawSkeletonX(const CSR_X*            pX,
                           const CSR_OpenGLShader* pShader,
                                 size_t            animSetIndex,
                                 size_t            frameIndex)
{
    if (!pX)
        return;

    if (!pShader)
        return;

    csrShaderEnable(pShader);

    csrDebugDrawBone(pX->m_pSkeleton,
                     pShader,
                     pX->m_pAnimationSet,
                     animSetIndex,
                     frameIndex,
                     pX->m_PoseOnly);
}
//---------------------------------------------------------------------------
// Collada model debug functions
//---------------------------------------------------------------------------
void csrDebugDrawSkeletonCollada(const CSR_Collada*      pCollada,
                                 const CSR_OpenGLShader* pShader,
                                       size_t            animSetIndex,
                                       size_t            frameIndex)
{
    if (!pCollada)
        return;

    if (!pShader)
        return;

    csrShaderEnable(pShader);

    csrDebugDrawBone(pCollada->m_pSkeleton,
                     pShader,
                     pCollada->m_pAnimationSet,
                     animSetIndex,
                     frameIndex,
                     pCollada->m_PoseOnly);
}
//---------------------------------------------------------------------------
