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
                            CSR_Matrix4*           pInitialMatrix,
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
                                 pInitialMatrix,
                                 &topMatrix);

        // get the bone bottom matrix
        if (poseOnly)
            csrBoneGetMatrix(pChild, 0, &bottomMatrix);
        else
            csrBoneGetAnimMatrix(pChild,
                                 &pAnimationSet[animSetIndex],
                                 frameIndex,
                                 pInitialMatrix,
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

        csrDebugDrawBone(pChild, pShader, pAnimationSet, pInitialMatrix, animSetIndex, frameIndex, poseOnly);
    }
}
//---------------------------------------------------------------------------
// X model debug functions
//---------------------------------------------------------------------------
#ifdef USE_X
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
                         0,
                         animSetIndex,
                         frameIndex,
                         pX->m_PoseOnly);
    }
#endif
//---------------------------------------------------------------------------
// Collada model debug functions
//---------------------------------------------------------------------------
#ifdef USE_COLLADA
    void csrDebugDrawSkeletonCollada(const CSR_Collada*      pCollada,
                                     const CSR_OpenGLShader* pShader,
                                           size_t            animSetIndex,
                                           size_t            frameIndex)
    {
        size_t i;

        if (!pCollada)
            return;

        if (!pShader)
            return;

        csrShaderEnable(pShader);

        for (i = 0; i < pCollada->m_SkeletonCount; ++i)
            csrDebugDrawBone(pCollada->m_pSkeletons[i].m_pRoot,
                             pShader,
                             pCollada->m_pAnimationSet,
                            &pCollada->m_pSkeletons[i].m_InitialMatrix,
                             animSetIndex,
                             frameIndex,
                             pCollada->m_PoseOnly);
    }
#endif
//---------------------------------------------------------------------------
// Inter-Quake model debug functions
//---------------------------------------------------------------------------
#ifdef USE_IQM
    void csrDebugDrawSkeletonIQM(const CSR_IQM*          pIQM,
                                 const CSR_OpenGLShader* pShader,
                                       size_t            animSetIndex,
                                       size_t            frameIndex)
    {
        if (!pIQM)
            return;

        if (!pShader)
            return;

        csrShaderEnable(pShader);

        csrDebugDrawBone(pIQM->m_pSkeleton,
                         pShader,
                         pIQM->m_pAnimationSet,
                         0,
                         animSetIndex,
                         frameIndex,
                         pIQM->m_PoseOnly);
    }
#endif
//---------------------------------------------------------------------------
