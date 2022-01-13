/****************************************************************************
 * ==> CSR_AI --------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the foundations to implement a task   *
 *               based Artificial Intelligence system                       *
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

#include "CSR_AI.h"

// std
#include <stdlib.h>

//---------------------------------------------------------------------------
// Task functions
//---------------------------------------------------------------------------
CSR_Task* csrTaskCreate(void)
{
    // create a new task
    CSR_Task* pTask = (CSR_Task*)malloc(sizeof(CSR_Task));

    // succeeded?
    if (!pTask)
        return 0;

    // initialize the task content
    csrTaskInit(pTask);

    return pTask;
}
//---------------------------------------------------------------------------
void csrTaskContentRelease(CSR_Task* pTask)
{
    // no task to release?
    if (!pTask)
        return;

    // the task data should not be released?
    if (!pTask->m_AutoFree)
        return;

    // release the task data
    if (pTask->m_pData)
        free(pTask->m_pData);
}
//---------------------------------------------------------------------------
void csrTaskInit(CSR_Task* pTask)
{
    // no task to initialize?
    if (!pTask)
        return;

    // initialize the task
    pTask->m_Action   = 0;
    pTask->m_AutoFree = 0;
    pTask->m_pData    = 0;
}
//---------------------------------------------------------------------------
// Task manager functions
//---------------------------------------------------------------------------
CSR_TaskManager* csrTaskManagerCreate(void)
{
    // create a new task manager
    CSR_TaskManager* pTaskManager = (CSR_TaskManager*)malloc(sizeof(CSR_TaskManager));

    // succeeded?
    if (!pTaskManager)
        return 0;

    // initialize the task manager content
    csrTaskManagerInit(pTaskManager);

    return pTaskManager;
}
//---------------------------------------------------------------------------
void csrTaskManagerRelease(CSR_TaskManager* pTM)
{
    // no task manager to release?
    if (!pTM)
        return;

    // release the tasks
    if (pTM->m_pTask)
    {
        size_t i;

        // release the task content
        for (i = 0; i < pTM->m_Count; ++i)
            csrTaskContentRelease(&pTM->m_pTask[i]);

        // free the tasks
        free(pTM->m_pTask);
    }

    // free the task manager
    free(pTM);
}
//---------------------------------------------------------------------------
void csrTaskManagerInit(CSR_TaskManager* pTM)
{
    // no task manager to initialize?
    if (!pTM)
        return;

    // initialize the task manager
    pTM->m_pTask = 0;
    pTM->m_Count = 0;
}
//---------------------------------------------------------------------------
void csrTaskManagerExecute(const CSR_TaskManager* pTaskManager,
                           const CSR_TaskContext* pContext,
                                 double           elapsedTime)
{
    size_t i;

    // validate the inputs
    if (!pTaskManager || !pContext)
        return;

    // iterate through tasks to execute
    for (i = 0; i < pTaskManager->m_Count; ++i)
    {
        int taskCompleted = 0;

        // run the task until be notified it was completed
        if (pContext->m_fOnTaskRun)
            taskCompleted = pContext->m_fOnTaskRun(&pTaskManager->m_pTask[i], elapsedTime);

        // if task was completed, change it
        if (taskCompleted && pContext->m_fOnTaskChange)
            pContext->m_fOnTaskChange(&pTaskManager->m_pTask[i], elapsedTime);
    }
}
//---------------------------------------------------------------------------
