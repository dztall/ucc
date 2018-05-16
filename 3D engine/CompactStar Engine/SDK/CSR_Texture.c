/****************************************************************************
 * ==> CSR_Texture ---------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the texture functions and types       *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2018, this file is part of the CompactStar Engine.  *
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
CSR_PixelBuffer* csrPixelBufferFromBitmap(const char* pFileName)
{
    CSR_Buffer*      pBuffer;
    CSR_PixelBuffer* pPixelBuffer;
    size_t           offset;
    unsigned         dataOffset;
    unsigned         headerSize;
    unsigned short   bpp;
    unsigned short   compressed;
    unsigned char    signature[2];

    // open bitmap file
    pBuffer = csrFileOpen(pFileName);

    // succeeded?
    if (!pBuffer)
        return 0;

    offset = 0;

    // read bitmap signature
    csrBufferRead(pBuffer, &offset, sizeof(unsigned char), 2, &signature[0]);

    // is bitmap signature correct?
    if (signature[0] != 'B' || signature[1] != 'M')
    {
        csrBufferRelease(pBuffer);
        return 0;
    }

    // create a pixel buffer
    pPixelBuffer = csrPixelBufferCreate();

    // succeeded?
    if (!pPixelBuffer)
    {
        csrBufferRelease(pBuffer);
        return 0;
    }

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
                csrBufferRelease(pBuffer);
                return 0;
            }

            // read bitmap compressed flag
            csrBufferRead(pBuffer, &offset, sizeof(unsigned short), 1, &compressed);

            // is compressed?
            if (compressed)
            {
                csrPixelBufferRelease(pPixelBuffer);
                csrBufferRelease(pBuffer);
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
                csrBufferRelease(pBuffer);
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
                csrBufferRelease(pBuffer);
                return 0;
            }

            break;
        }

        default:
            // unsupported bitmap format
            csrPixelBufferRelease(pPixelBuffer);
            csrBufferRelease(pBuffer);
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

    csrBufferRelease(pBuffer);

    return pPixelBuffer;
}
//---------------------------------------------------------------------------
// Texture functions
//---------------------------------------------------------------------------
GLuint csrTextureFromPixelBuffer(const CSR_PixelBuffer* pPixelBuffer)
{
    unsigned char* pPixels;
    unsigned       x;
    unsigned       y;
    unsigned char  c;
    GLuint         index;

    // validate the input
    if (!pPixelBuffer           ||
        !pPixelBuffer->m_Width  ||
        !pPixelBuffer->m_Height ||
         pPixelBuffer->m_BytePerPixel != 3)
        return M_CSR_Error_Code;

    // reorder the pixels if image is a bitmap
    if (pPixelBuffer->m_ImageType == CSR_IT_Bitmap)
    {
        pPixels = (unsigned char*)malloc(sizeof(unsigned char)  *
                                         pPixelBuffer->m_Width  *
                                         pPixelBuffer->m_Height *
                                         3);

        // get bitmap data into right format
        for (y = 0; y < pPixelBuffer->m_Height; ++y)
            for (x = 0; x < pPixelBuffer->m_Width; ++x)
                for (c = 0; c < 3; ++c)
                    pPixels[3 * (pPixelBuffer->m_Width * y + x) + c] =
                            ((unsigned char*)pPixelBuffer->m_pData)
                                    [pPixelBuffer->m_Stride * y + 3 * (pPixelBuffer->m_Width - x - 1) + (2 - c)];
    }
    else
        pPixels = (unsigned char*)pPixelBuffer->m_pData;

    // create new OpenGL texture
    glGenTextures(1, &index);
    glBindTexture(GL_TEXTURE_2D, index);

    // set texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set texture wrapping mode
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // generate texture from bitmap data
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 pPixelBuffer->m_Width,
                 pPixelBuffer->m_Height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 pPixels);

    // delete local pixel buffer
    if (pPixelBuffer->m_ImageType == CSR_IT_Bitmap)
        free(pPixels);

    return index;
}
//---------------------------------------------------------------------------
// Texture item functions
//---------------------------------------------------------------------------
CSR_TextureItem* csrTextureItemCreate(void)
{
    // create a new texture item
    CSR_TextureItem* pTI = (CSR_TextureItem*)malloc(sizeof(CSR_TextureItem));

    // succeeded?
    if (!pTI)
        return 0;

    // initialize the texture item content
    csrTextureItemInit(pTI);

    return pTI;
}
//---------------------------------------------------------------------------
void csrTextureItemContentRelease(CSR_TextureItem* pTI)
{
    // no texture item to release?
    if (!pTI)
        return;

    // do release the texture buffer?
    if (pTI->m_pBuffer)
        csrPixelBufferRelease(pTI->m_pBuffer);

    // do release the file name?
    if (pTI->m_pFileName)
        free(pTI->m_pFileName);

    // do release the texture loaded on the GPU?
    if (pTI->m_ID != M_CSR_Error_Code)
        glDeleteTextures(1, &pTI->m_ID);
}
//---------------------------------------------------------------------------
void csrTextureItemInit(CSR_TextureItem* pTI)
{
    // no texture item to initialize?
    if (!pTI)
        return;

    // initialize the texture item content
    pTI->m_pBuffer   = 0;
    pTI->m_pFileName = 0;
    pTI->m_ID        = M_CSR_Error_Code;
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
void csrTextureArrayRelease(CSR_TextureArray* pTA)
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
            csrTextureItemContentRelease(&pTA->m_pItem[i]);

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
// Texture shader functions
//---------------------------------------------------------------------------
void csrTextureShaderInit(CSR_TextureShader* pTextureShader)
{
    // no texture shader to initialize?
    if (!pTextureShader)
        return;

    // initialize the texture shader content
    pTextureShader->m_TextureID = M_CSR_Error_Code;
    pTextureShader->m_BumpMapID = M_CSR_Error_Code;
}
//---------------------------------------------------------------------------
