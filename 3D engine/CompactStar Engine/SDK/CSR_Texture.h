/****************************************************************************
 * ==> CSR_Texture ---------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the texture functions and types       *
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

#ifndef CSR_TextureH
#define CSR_TextureH

// compactStar engine
#include "CSR_Common.h"

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Image type
*/
typedef enum
{
    CSR_IT_Raw,
    CSR_IT_Bitmap
} CSR_EImageType;

/**
* Pixel type
*/
typedef enum
{
    CSR_PT_RGB,
    CSR_PT_BGR,
    CSR_PT_RGBA,
    CSR_PT_BGRA,
    CSR_PT_ARGB,
    CSR_PT_ABGR,
} CSR_EPixelType;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* Pixel buffer
*/
typedef struct
{
    CSR_EImageType m_ImageType;
    CSR_EPixelType m_PixelType;
    unsigned       m_Width;
    unsigned       m_Height;
    unsigned       m_Stride;
    unsigned       m_BytePerPixel;
    size_t         m_DataLength;
    void*          m_pData;
} CSR_PixelBuffer;

/**
* Texture item
*/
typedef struct
{
    CSR_PixelBuffer* m_pBuffer;
    char*            m_pFileName;
} CSR_Texture;

/**
* Texture array
*/
typedef struct
{
    CSR_Texture* m_pItem;
    size_t       m_Count;
} CSR_TextureArray;

/**
* Skin
*/
typedef struct
{
    CSR_Texture m_Texture;
    CSR_Texture m_BumpMap;
    CSR_Texture m_CubeMap;
    double      m_Time;
} CSR_Skin;

//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------

/**
* Called when a texture should be deleted
*@param pTexture - texture to delete
*/
typedef void (*CSR_fOnDeleteTexture)(const CSR_Texture* pTexture);

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Pixel buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates a pixel buffer
        *@return newly created pixel buffer, 0 on error
        *@note The pixel buffer must be released when no longer used, see csrPixelBufferRelease()
        */
        CSR_PixelBuffer* csrPixelBufferCreate(void);

        /**
        * Releases a pixel buffer
        *@param[in, out] pPB - pixel buffer to release
        */
        void csrPixelBufferRelease(CSR_PixelBuffer* pPB);

        /**
        * Initializes a pixel buffer structure
        *@param[in, out] pPB - pixel buffer to initialize
        */
        void csrPixelBufferInit(CSR_PixelBuffer* pPB);

        /**
        * Loads a pixel buffer from a bitmap file
        *@param pFileName - bitmap file name to load from
        *@return pixel buffer, 0 on error
        *@note For now only most common bitmaps are loaded, some bitmap types may be unsupported
        *@note The pixel buffer must be released when no longer used, see csrPixelBufferRelease()
        */
        CSR_PixelBuffer* csrPixelBufferFromBitmapFile(const char* pFileName);

        /**
        * Loads a pixel buffer from a bitmap buffer
        *@param pBuffer - buffer containing the bitmap to load from
        *@return pixel buffer, 0 on error
        *@note For now only most common bitmaps are loaded, some bitmap types may be unsupported
        *@note The pixel buffer must be released when no longer used, see csrPixelBufferRelease()
        */
        CSR_PixelBuffer* csrPixelBufferFromBitmapBuffer(const CSR_Buffer* pBuffer);

        //-------------------------------------------------------------------
        // Texture functions
        //-------------------------------------------------------------------

        /**
        * Creates a texture
        *@return newly created texture, 0 on error
        *@note The texture must be released when no longer used, see csrTextureRelease()
        */
        CSR_Texture* csrTextureCreate(void);

        /**
        * Releases a texture
        *@param[in, out] pTexture - texture to release
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrTextureRelease(CSR_Texture* pTexture, const CSR_fOnDeleteTexture fOnDeleteTexture);

        /**
        * Releases a texture content
        *@param[in, out] pTexture - texture for which the content should be released
        *@note Only the texture content is released, the texture itself is not released
        */
        void csrTextureContentRelease(CSR_Texture* pTexture);

        /**
        * Initializes a texture
        *@param[in, out] pTexture - texture to initialize
        */
        void csrTextureInit(CSR_Texture* pTexture);

        //-------------------------------------------------------------------
        // Texture array functions
        //-------------------------------------------------------------------

        /**
        * Creates a texture array
        *@return newly created texture array, 0 on error
        *@note The texture array must be released when no longer used, see csrTextureArrayRelease()
        */
        CSR_TextureArray* csrTextureArrayCreate(void);

        /**
        * Releases a texture array
        *@param[in, out] pTA - texture array to release
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrTextureArrayRelease(CSR_TextureArray* pTA, const CSR_fOnDeleteTexture fOnDeleteTexture);

        /**
        * Initializes a texture array structure
        *@param[in, out] pTA - texture array to initialize
        */
        void csrTextureArrayInit(CSR_TextureArray* pTA);

        //-------------------------------------------------------------------
        // Skin functions
        //-------------------------------------------------------------------

        /**
        * Creates a skin
        *@return newly created skin, 0 on error
        *@note The skin must be released when no longer used, see csrSkinRelease()
        */
        CSR_Skin* csrSkinCreate(void);

        /**
        * Releases a skin
        *@param[in, out] pSkin - skin to release
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        */
        void csrSkinRelease(CSR_Skin* pSkin, const CSR_fOnDeleteTexture fOnDeleteTexture);

        /**
        * Releases a skin content
        *@param[in, out] pSkin - skin for which the content should be released
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@note Only the skin content is released, the skin itself is not released
        */
        void csrSkinContentRelease(CSR_Skin* pSkin, const CSR_fOnDeleteTexture fOnDeleteTexture);

        /**
        * Initializes a skin
        *@param[in, out] pSkin - skin to initialize
        */
        void csrSkinInit(CSR_Skin* pSkin);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Texture.c"
#endif

#endif
