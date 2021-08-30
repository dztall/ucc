/****************************************************************************
 * ==> CSR_Texture ---------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the texture functions and types       *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2019, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

#include "CSR_Texture.h"

// std
#include <stdlib.h>

//---------------------------------------------------------------------------
// Pixel buffer functions
//---------------------------------------------------------------------------
CSR_PixelBuffer* csrPixelBufferCreate(void)
{
    // create a new pixel buffer
    CSR_PixelBuffer* pPB = (CSR_PixelBuffer*)malloc(sizeof(CSR_PixelBuffer));

    // succeeded?
    if (!pPB)
        return 0;

    // initialize the pixel buffer content
    csrPixelBufferInit(pPB);

    return pPB;
}
//---------------------------------------------------------------------------
void csrPixelBufferRelease(CSR_PixelBuffer* pPB)
{
    // no pixel buffer to release?
    if (!pPB)
        return;

    // free the pixel buffer content
    if (pPB->m_pData)
        free(pPB->m_pData);

    // free the pixel buffer
    free(pPB);
}
//---------------------------------------------------------------------------
void csrPixelBufferInit(CSR_PixelBuffer* pPB)
{
    // no pixel buffer to initialize?
    if (!pPB)
        return;

    // initialize the pixel buffer content
    pPB->m_ImageType    = CSR_IT_Raw;
    pPB->m_PixelType    = CSR_PT_RGBA;
    pPB->m_Width        = 0;
    pPB->m_Height       = 0;
    pPB->m_Stride       = 0;
    pPB->m_BytePerPixel = 0;
    pPB->m_DataLength   = 0;
    pPB->m_pData        = 0;
}
//---------------------------------------------------------------------------
CSR_PixelBuffer* csrPixelBufferFromBitmapFile(const char* pFileName)
{
    CSR_Buffer*      pBuffer;
    CSR_PixelBuffer* pPixelBuffer;

    // open bitmap file
    pBuffer = csrFileOpen(pFileName);

    // convert to pixel buffer
    pPixelBuffer = csrPixelBufferFromBitmapBuffer(pBuffer);

    // free the bitmap file
    csrBufferRelease(pBuffer);

    return pPixelBuffer;
}
//---------------------------------------------------------------------------
CSR_PixelBuffer* csrPixelBufferFromBitmapBuffer(const CSR_Buffer* pBuffer)
{
    CSR_PixelBuffer* pPixelBuffer;
    size_t           offset;
    unsigned         dataOffset;
    unsigned         headerSize;
    unsigned short   bpp;
    unsigned short   compressed;
    unsigned char    signature[2];

    // validate the input
    if (!pBuffer)
        return 0;

    offset = 0;

    // read bitmap signature
    csrBufferRead(pBuffer, &offset, sizeof(unsigned char), 2, &signature[0]);

    // is bitmap signature correct?
    if (signature[0] != 'B' || signature[1] != 'M')
        return 0;

    // create a pixel buffer
    pPixelBuffer = csrPixelBufferCreate();

    // succeeded?
    if (!pPixelBuffer)
        return 0;

    // initialize the pixel buffer
    csrPixelBufferInit(pPixelBuffer);

    // skip 8 next bytes
    offset += 8;

    // read data offset
    csrBufferRead(pBuffer, &offset, sizeof(unsigned), 1, &dataOffset);

    // read header size
    csrBufferRead(pBuffer, &offset, sizeof(unsigned), 1, &headerSize);

    // search for bitmap type
    switch (headerSize)
    {
        // V3
        case 40:
        {
            // read bitmap width
            csrBufferRead(pBuffer, &offset, sizeof(unsigned), 1, &pPixelBuffer->m_Width);

            // read bitmap height
            csrBufferRead(pBuffer, &offset, sizeof(unsigned), 1, &pPixelBuffer->m_Height);

            // skip next 2 bytes
            offset += 2;

            // read bitmap bit per pixels
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &bpp);

            // is bpp supported?
            if (bpp != 24)
            {
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            // read bitmap compressed flag
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &compressed);

            // is compressed?
            if (compressed)
            {
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            break;
        }

        // OS/2 V1
        case 12:
        {
            unsigned short width;
            unsigned short height;

            // read bitmap width
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &width);

            // read bitmap height
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &height);

            pPixelBuffer->m_Width  = width;
            pPixelBuffer->m_Height = height;

            // skip next 2 bytes
            offset += 2;

            // read bitmap bit per pixels
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &bpp);

            // is bpp supported?
            if (bpp != 24)
            {
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            break;
        }

        // Windows V4
        case 108:
        {
            unsigned short width;
            unsigned short height;

            // read bitmap width
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &width);

            // skip next 2 bytes
            offset += 2;

            // read bitmap height
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &height);

            pPixelBuffer->m_Width  = width;
            pPixelBuffer->m_Height = height;

            // skip next 4 bytes
            offset += 4;

            // read bitmap bit per pixels
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &bpp);

            // is bpp supported?
            if (bpp != 24)
            {
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            break;
        }

        default:
            // unsupported bitmap format
            csrPixelBufferRelease(pPixelBuffer);
            return 0;
    }

    pPixelBuffer->m_PixelType    = CSR_PT_BGR;
    pPixelBuffer->m_ImageType    = CSR_IT_Bitmap;
    pPixelBuffer->m_BytePerPixel = bpp / 8;
    pPixelBuffer->m_Stride       = (((pPixelBuffer->m_Width) * 3 + 3) / 4) * 4 - ((pPixelBuffer->m_Width) * 3 % 4);
    pPixelBuffer->m_DataLength   = pPixelBuffer->m_Stride * pPixelBuffer->m_Height;
    pPixelBuffer->m_pData        = malloc(sizeof(unsigned char) * pPixelBuffer->m_DataLength);

    offset = dataOffset;

    // read bitmap data
    csrBufferRead(pBuffer,
                 &offset,
                  sizeof(unsigned char),
                  pPixelBuffer->m_DataLength,
                  pPixelBuffer->m_pData);

    return pPixelBuffer;
}
//---------------------------------------------------------------------------
// Texture functions
//---------------------------------------------------------------------------
CSR_Texture* csrTextureCreate(void)
{
    // create a new texture
    CSR_Texture* pTexture = (CSR_Texture*)malloc(sizeof(CSR_Texture));

    // succeeded?
    if (!pTexture)
        return 0;

    // initialize the texture content
    csrTextureInit(pTexture);

    return pTexture;
}
//---------------------------------------------------------------------------
void csrTextureRelease(CSR_Texture* pTexture, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    // no texture to release?
    if (!pTexture)
        return;

    // notify the GPU side about the texture deletion
    if (fOnDeleteTexture)
        fOnDeleteTexture(pTexture);

    // release the texture content
    csrTextureContentRelease(pTexture);

    // release the texture itself
    free(pTexture);
}
//---------------------------------------------------------------------------
void csrTextureContentRelease(CSR_Texture* pTexture)
{
    // no texture to release?
    if (!pTexture)
        return;

    // release the texture pixel buffer
    csrPixelBufferRelease(pTexture->m_pBuffer);

    // release the file name
    if (pTexture->m_pFileName)
        free(pTexture->m_pFileName);
}
//---------------------------------------------------------------------------
void csrTextureInit(CSR_Texture* pTexture)
{
    // no texture to initialize?
    if (!pTexture)
        return;

    // initialize the texture content
    pTexture->m_pBuffer   = 0;
    pTexture->m_pFileName = 0;
}
//---------------------------------------------------------------------------
// Texture array functions
//---------------------------------------------------------------------------
CSR_TextureArray* csrTextureArrayCreate(void)
{
    // create a new texture array
    CSR_TextureArray* pTA = (CSR_TextureArray*)malloc(sizeof(CSR_TextureArray));

    // succeeded?
    if (!pTA)
        return 0;

    // initialize the texture array content
    csrTextureArrayInit(pTA);

    return pTA;
}
//---------------------------------------------------------------------------
void csrTextureArrayRelease(CSR_TextureArray* pTA, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t i;

    // no texture array to release?
    if (!pTA)
        return;

    // do free the texture items?
    if (pTA->m_pItem)
    {
        // iterate through texture items and release their content
        for (i = 0; i < pTA->m_Count; ++i)
        {
            // notify the GPU side about the texture deletion
            if (fOnDeleteTexture)
                fOnDeleteTexture(&pTA->m_pItem[i]);

            csrTextureContentRelease(&pTA->m_pItem[i]);
        }

        // free the texture items
        free(pTA->m_pItem);
    }

    // free the texture array
    free(pTA);
}
//---------------------------------------------------------------------------
void csrTextureArrayInit(CSR_TextureArray* pTA)
{
    // no texture array to initialize?
    if (!pTA)
        return;

    // initialize the texture array content
    pTA->m_pItem = 0;
    pTA->m_Count = 0;
}
//---------------------------------------------------------------------------
// Skin functions
//---------------------------------------------------------------------------
CSR_Skin* csrSkinCreate(void)
{
    // create a new skin
    CSR_Skin* pSkin = (CSR_Skin*)malloc(sizeof(CSR_Skin));

    // succeeded?
    if (!pSkin)
        return 0;

    // initialize the skin content
    csrSkinInit(pSkin);

    return pSkin;
}
//---------------------------------------------------------------------------
void csrSkinRelease(CSR_Skin* pSkin, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    // no skin to release?
    if (!pSkin)
        return;

    // release the skin content
    csrSkinContentRelease(pSkin, fOnDeleteTexture);

    // release the skin itself
    free(pSkin);
}
//---------------------------------------------------------------------------
void csrSkinContentRelease(CSR_Skin* pSkin, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    // no skin to release?
    if (!pSkin)
        return;

    // notify the GPU side about the skin deletion
    if (fOnDeleteTexture)
    {
        fOnDeleteTexture(&pSkin->m_Texture);
        fOnDeleteTexture(&pSkin->m_BumpMap);
        fOnDeleteTexture(&pSkin->m_CubeMap);
    }

    // release the texture content
    csrTextureContentRelease(&pSkin->m_Texture);

    // release the bumpmap content
    csrTextureContentRelease(&pSkin->m_BumpMap);

    // release the cubemap content
    csrTextureContentRelease(&pSkin->m_CubeMap);
}
//---------------------------------------------------------------------------
void csrSkinInit(CSR_Skin* pSkin)
{
    // no skin to initialize?
    if (!pSkin)
        return;

    // initialize the skin content
    csrTextureInit(&pSkin->m_Texture);
    csrTextureInit(&pSkin->m_BumpMap);
    csrTextureInit(&pSkin->m_CubeMap);

    pSkin->m_Time = 0.0;
}
//---------------------------------------------------------------------------
