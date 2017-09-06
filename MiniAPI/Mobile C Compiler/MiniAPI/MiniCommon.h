/*****************************************************************************
 * ==> Minimal library common definitions -----------------------------------*
 *****************************************************************************
 * Description : Minimal library common definitions                          *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not                 *
 *****************************************************************************/

#ifndef MiniCommonH
#define MiniCommonH

//-----------------------------------------------------------------------------
// Global defines
//-----------------------------------------------------------------------------

#define MAPI_VERSION 1.7

// __CCR__ and __CCR_MINOR__ macros may be undefined on older compiler versions,
// so define them if needed
#ifndef __CCR__
    #define __CCR__ 0
#endif

#ifndef __CCR_MINOR__
    #define __CCR_MINOR__ 0
#endif

//-----------------------------------------------------------------------------
// Common aliases
//-----------------------------------------------------------------------------
#define MINI_FILE_OPEN  fopen
#define MINI_FILE_CLOSE fclose
#define MINI_FILE_READ  fread
#define MINI_FILE_SEEK  fseek

#endif
