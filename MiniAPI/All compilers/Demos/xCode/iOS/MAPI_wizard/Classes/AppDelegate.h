/*****************************************************************************
 * ==> AppDelegate ----------------------------------------------------------*
 *****************************************************************************
 * Description : A Quake (MDL) model showing a wizard. Tap on the left or    *
 *               right to change the animation                               *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#import <UIKit/UIKit.h>

/**
* Application delegate, contains the main window and the basic events called
* during the window lifetime
*@author Jean-Milost Reymond
*/
@interface AppDelegate : UIResponder<UIApplicationDelegate>
    @property (strong, nonatomic) UIWindow* window;
@end
