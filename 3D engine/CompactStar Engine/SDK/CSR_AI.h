/*****************************************************************************
 * ==> CSR_AI ---------------------------------------------------------------*
 *****************************************************************************
 * Description : This module provides the foundations to implement a task    *
 *               based Artificial Intelligence system                        *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#ifndef CSR_AIH
#define CSR_AIH

// compactStar engine
#include "CSR_Common.h"

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

// task context prototype
typedef struct CSR_TaskContext CSR_TaskContext;

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

/**
* Task, contains an action to execute
*/
typedef struct
{
    int   m_Action;
    int   m_AutoFree;
    void* m_pData;
} CSR_Task;

/**
* Task manager
*/
typedef struct
{
    CSR_Task* m_pTask;
    size_t    m_Count;
} CSR_TaskManager;

//----------------------------------------------------------------------------
// Callbacks
//----------------------------------------------------------------------------

/**
* Called while a task is running
*@param pTask - running task
*@param elapsedTime - elapsed time since last task was processed
*@return 1 if task is done, otherwise 0
*/
typedef int (*CSR_fOnTaskRun)(CSR_Task* pTask, double elapsedTime);

/**
* Called when a task was completed and a new task sould be attributed
*@param[in, out] pTask - completed task, new task to execute on function ends
*@param elapsedTime - elapsed time since last task was processed
*/
typedef void (*CSR_fOnTaskChange)(CSR_Task* pTask, double elapsedTime);

//---------------------------------------------------------------------------
// Implementation
//---------------------------------------------------------------------------

/**
* Task context
*/
struct CSR_TaskContext
{
    CSR_fOnTaskRun    m_fOnTaskRun;
    CSR_fOnTaskChange m_fOnTaskChange;
};

#ifdef __cplusplus
    extern "C"
    {
#endif

        //----------------------------------------------------------------------------
        // Task functions
        //----------------------------------------------------------------------------

        /**
        * Creates a task
        *@return newly created task, 0 on error
        *@note The task must be released when no longer used, see csrTaskContentRelease()
        */
        CSR_Task* csrTaskCreate(void);

        /**
        * Releases a task content
        *@param[in, out] pTask - task for which the content should be released
        *@note Only the task content is released, the task itself is not released
        */
        void csrTaskContentRelease(CSR_Task* pTask);

        /**
        * Initializes a task structure
        *@param[in, out] pTask - task to initialize
        */
        void csrTaskInit(CSR_Task* pTask);

        //----------------------------------------------------------------------------
        // Task manager functions
        //----------------------------------------------------------------------------

        /**
        * Creates a task manager
        *@return newly created task manager, 0 on error
        *@note The task manager must be released when no longer used, see csrTaskManagerRelease()
        */
        CSR_TaskManager* csrTaskManagerCreate(void);

        /**
        * Releases a task manager
        *@param[in, out] pTM - task manager to release
        */
        void csrTaskManagerRelease(CSR_TaskManager* pTM);

        /**
        * Initializes a task manager structure
        *@param[in, out] pTM - task manager to initialize
        */
        void csrTaskManagerInit(CSR_TaskManager* pTM);

        /**
        * Executes the tasks
        *@param pTaskManager - task manager
        *@param pContext - task manager context
        *@param elapsedTime - elapsed time since last task was processed
        */
        void csrTaskManagerExecute(const CSR_TaskManager* pTaskManager,
                                   const CSR_TaskContext* pContext,
                                         double           elapsedTime);

#ifdef __cplusplus
    }
#endif

//----------------------------------------------------------------------------
// Compiler
//----------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_AI.c"
#endif

#endif
