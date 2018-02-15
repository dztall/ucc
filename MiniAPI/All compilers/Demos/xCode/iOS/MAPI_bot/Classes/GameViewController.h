/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : An intelligent bot demo                                     *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

// mini api
#include "MiniAI.h"

/**
* A callback manager class, used to map the callbacks between the Objective-C
* and the C languages
*@author Jean-Milost Reymond
*/
class MINI_CallbackManager
{
    public:
        /**
        * Constructor
        *@param pOwner - owner for this manager
        */
        MINI_CallbackManager(void* pOwner);

        virtual ~MINI_CallbackManager();

        /**
        * Constructor
        *@param elapsedTime - elapsed time since last task was processed
        *@param pItems - task items
        */
        virtual void ExecuteTasks(float elapsedTime, MINI_BotItem* pItems);

    private:
        void* m_pOwner;

        /**
        * Constructor
        *@note This constructor is prohibited, as the manager cannot exist without an owner
        */
        MINI_CallbackManager();

        /**
        * Called while bot is watching
        *@param elapsedTime - elapsed time since last task was processed
        *@param pItem - running task item
        *@return 1 if task is done, otherwise 0
        */
        static int OnWatching(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called while bot is searching
        *@param elapsedTime - elapsed time since last task was processed
        *@param pItem - running task item
        *@return 1 if task is done, otherwise 0
        */
        static int OnSearching(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called while bot is attacking
        *@param elapsedTime - elapsed time since last task was processed
        *@param pItem - running task item
        *@return 1 if task is done, otherwise 0
        */
        static int OnAttacking(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called while bot is being hurt
        *@param elapsedTime - elapsed time since last task was processed
        *@param pItem - running task item
        *@return 1 if task is done, otherwise 0
        */
        static int OnBeingHurt(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called while bot is dying
        *@param elapsedTime - elapsed time since last task was processed
        *@param pItem - running task item
        *@return 1 if task is done, otherwise 0
        */
        static int OnDying(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called when a new task sould be attributed
        *@param elapsedTime - elapsed time since last task was processed
        *@param pItem - running task item
        *@return 1 if task is done, otherwise 0
        */
        static void OnNewTask(float elapsedTime, MINI_BotItem* pItem);
};

/**
* Game view controller, contains the application main view and his associated events
*@author Jean-Milost Reymond
*/
@interface GameViewController : GLKViewController
{}

/**
* Called when bot watching task should be executed
*@param elapedTime - elapsed time since last process
*@param pItem - bot item for which the task should be executed
*@return 1 if the task ended and the next one should be executed, otherwise 0
*/
- (int) OnBotWatching :(float)elapsedTime :(MINI_BotItem*)pItem;

/**
* Called when bot searching task should be executed
*@param elapedTime - elapsed time since last process
*@param pItem - bot item for which the task should be executed
*@return 1 if the task ended and the next one should be executed, otherwise 0
*/
- (int) OnBotSearching :(float)elapsedTime :(MINI_BotItem*)pItem;

/**
* Called when bot attacking task should be executed
*@param elapedTime - elapsed time since last process
*@param pItem - bot item for which the task should be executed
*@return 1 if the task ended and the next one should be executed, otherwise 0
*/
- (int)OnBotAttacking :(float)elapsedTime :(MINI_BotItem*)pItem;

/**
* Called when bot being hurt task should be executed
*@param elapedTime - elapsed time since last process
*@param pItem - bot item for which the task should be executed
*@return 1 if the task ended and the next one should be executed, otherwise 0
*/
- (int) OnBotBeingHurt :(float)elapsedTime :(MINI_BotItem*)pItem;

/**
* Called when bot dying task should be executed
*@param elapedTime - elapsed time since last process
*@param pItem - bot item for which the task should be executed
*@return 1 if the task ended and the next one should be executed, otherwise 0
*/
- (int) OnBotDying :(float)elapsedTime :(MINI_BotItem*)pItem;

/**
* Called when a new task should be sheluded for the bot
*@param elapedTime - elapsed time since last process
*@param pItem - bot item for which the task should be executed
*/
- (void) OnNewTask :(float)elapsedTime :(MINI_BotItem*)pItem;

@end
