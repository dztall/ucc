/*****************************************************************************
 * ==> MiniObjectiveCHelper -------------------------------------------------*
 *****************************************************************************
 * Description : This module provides a helper class to facilitate the usage *
 *               of the MiniAPI with the Objective-C language                *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// OSX
#import <Foundation/NSString.h>

/**
* Helper class to facilitate the usage of the MiniAPI with the Objective-C language
*@author Jean-Milost Reymond
*/
@interface MiniObjectiveCHelper : NSObject 
{}

/**
* Get the path of a resource
*@param resourceName - resource name to get
*@param resourceType - resource type (generally the same as the extension)
*@param[out] pFileName - file name
*@note The file name must be freed by the caller when useless
*/
+ (void) ResourceToFileName :(NSString*)resourceName :(NSString*)resourceType :(char**)pFileName;

@end
