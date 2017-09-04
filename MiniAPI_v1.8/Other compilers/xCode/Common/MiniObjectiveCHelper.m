/*****************************************************************************
 * ==> MiniObjectiveCHelper -------------------------------------------------*
 *****************************************************************************
 * Description : This module provides a helper class to facilitate the usage *
 *               of the MiniAPI with the Objective-C language                *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#include "MiniObjectiveCHelper.h"

// OSX
#import <Foundation/Foundation.h>

//----------------------------------------------------------------------------
@implementation MiniObjectiveCHelper
//----------------------------------------------------------------------------
+ (void) ResourceToFileName :(NSString*)resourceName :(NSString*)resourceType :(char**)pFileName
{
    // get the file path from resources
    const NSString* pFilePath = [[NSBundle mainBundle]pathForResource:resourceName ofType:resourceType];

    // convert the path data to UTF8 string
    const NSData* pStrData =
            [pFilePath dataUsingEncoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8)];

    // get the file name
    *pFileName = (char*)calloc(pStrData.length + 1, sizeof(char));
    memcpy(*pFileName, pStrData.bytes, pStrData.length);
    (*pFileName)[pStrData.length] = '\0';
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
