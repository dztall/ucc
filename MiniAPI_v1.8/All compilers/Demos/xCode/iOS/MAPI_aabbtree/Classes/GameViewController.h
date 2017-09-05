/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A ray picking demo with aligned-axis bounding box. Tap      *
 *               anywhere on the sphere to select a polygon, swipe to the    *
 *               left or right to rotate the sphere                          *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

/**
* Game view controller, contains the application main view and his associated events
*@author Jean-Milost Reymond
*/
@interface GameViewController : GLKViewController
@end
