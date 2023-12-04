/****************************************************************************
 * ==> CSR_GJK -------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides an implementation of the              *
 *               Gilbert-Johnson-Keerthi (GJK) algorithm                    *
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

#ifndef CSR_GJKH
#define CSR_GJKH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

// collider prototype
typedef struct CSR_Collider CSR_Collider;

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Collider state
*/
typedef enum
{
    CSR_CS_Dynamic,
    CSR_CS_Static
} CSR_EColliderState;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Support function for the Minkowski difference
*@param pCollider - collider for which the support function should be called
*@param pDir - search direction
*@param[out] pR - the furthest vertex belonging to the collider along the direction
*/
typedef void (*CSR_fOnSupport)(const CSR_Collider* pCollider,
                               const CSR_Vector3*  pDir,
                                     CSR_Vector3*  pR);

//---------------------------------------------------------------------------
// Implementation
//---------------------------------------------------------------------------

/**
* Collider
*/
struct CSR_Collider
{
    CSR_EColliderState m_State;      // collider state, i.e. dynamic or static
	CSR_Vector3        m_Pos;        // collider position
	CSR_Matrix4        m_MatRS;      // rotation/scale matrix
	CSR_Matrix4        m_InvMatRS;   // inverted rotation/scale matrix
	CSR_Vector3        m_Min;        // box collider, min vertex
	CSR_Vector3        m_Max;        // box collider, max vertex
	float              m_TopY;       // capsule and cylinder collider, y top position
	float              m_BottomY;    // capsule and cylinder collider, y bottom position
	float              m_Radius;     // sphere, capsule and cylinder collider, radius
    CSR_fOnSupport     m_fOnSupport; // support function to use for Minkowski difference
};

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Support functions
        //-------------------------------------------------------------------
        /**
        * Box support function for the Minkowski difference
        *@param pCollider - collider for which the support function should be called
        *@param pDir - search direction
        *@param[out] pR - the furthest vertex belonging to the collider along the direction
        */
        void csrGJKSupportBox(const CSR_Collider* pCollider,
                              const CSR_Vector3*  pDir,
                                    CSR_Vector3*  pR);

        /**
        * Sphere support function for the Minkowski difference
        *@param pCollider - collider for which the support function should be called
        *@param pDir - search direction
        *@param[out] pR - the furthest vertex belonging to the collider along the direction
        */
        void csrGJKSupportSphere(const CSR_Collider* pCollider,
                                 const CSR_Vector3*  pDir,
                                       CSR_Vector3*  pR);

        /**
        * Cylinder support function for the Minkowski difference
        *@param pCollider - collider for which the support function should be called
        *@param pDir - search direction
        *@param[out] pR - the furthest vertex belonging to the collider along the direction
        */
        void csrGJKSupportCylinder(const CSR_Collider* pCollider,
                                   const CSR_Vector3*  pDir,
                                         CSR_Vector3*  pR);

        /**
        * Capsule support function for the Minkowski difference
        *@param pCollider - collider for which the support function should be called
        *@param pDir - search direction
        *@param[out] pR - the furthest vertex belonging to the collider along the direction
        */
        void csrGJKSupportCapsule(const CSR_Collider* pCollider,
                                  const CSR_Vector3*  pDir,
                                        CSR_Vector3*  pR);

        //-------------------------------------------------------------------
        // Collider functions
        //-------------------------------------------------------------------
        /**
        * Creates a collider structure
        *@return newly created collider structure, 0 on error
        *@note The collider structure must be released when no longer used, see csrColliderRelease()
        */
        CSR_Collider* csrColliderCreate(void);

        /**
        * Releases a collider structure
        *@param[in, out] pCollider - collider structure to release
        */
        void csrColliderRelease(CSR_Collider* pCollider);

        /**
        * Initializes a collider structure
        *@param[in, out] pCollider - collider structure to initialize
        */
        void csrColliderInit(CSR_Collider* pCollider);

        /**
        * Sets collider position
        *@param pPos - new position to set
        *@param[in, out] pCollider - collider structure for which the position should be set
        */
        void csrColliderSetPos(const CSR_Vector3* pPos, CSR_Collider* pCollider);

        /**
        * Sets collider rotation and scale
        *@param pMat - new rotation and scale matrix
        *@param[in, out] pCollider - collider structure for which the rotation and scale matrix should be set
        *@note If the matrix contains a position, it will be ignored. Please use csrColliderSetPos() to set the position
        */
        void csrColliderSetRS(const CSR_Matrix4* pMat, CSR_Collider* pCollider);

        //-------------------------------------------------------------------
        // GJK functions
        //-------------------------------------------------------------------
        /**
        * Resolves the GJK algorithm
        *@param pC1 - first collider to compare
        *@param pC2 - first collider to compare with
        *@return 1 if a collision happened, otherwise 0
        */
        int csrGJKResolve(const CSR_Collider* pC1, const CSR_Collider* pC2);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_GJK.c"
#endif

#endif
