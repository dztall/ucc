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

#include "CSR_Texture.h"

// std
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
// Private structures
//---------------------------------------------------------------------------

/**
* TARGA (.tga) header
*/
typedef struct
{
    unsigned char m_IdentSize;
    unsigned char m_CMapType;
    unsigned char m_ImageType;
    unsigned char m_CMapOrigin[2];
    unsigned char m_CMapSize[2];
    unsigned char m_CMapEntrySize;
    unsigned char m_XOrigin[2];
    unsigned char m_YOrigin[2];
    unsigned char m_Width[2];
    unsigned char m_Height[2];
    unsigned char m_PixelSize;
    unsigned char m_DescByte;
} CSR_TGAHeader;

//---------------------------------------------------------------------------
// Private functions
//---------------------------------------------------------------------------
static inline void csrBGRToRGB(unsigned char* pData, int len, int bpp)
{
    unsigned char* pEnd;

    for (pEnd = &pData[len]; pData < pEnd; pData += bpp)
    {
        pData[0] = pData[0] ^ pData[2];
        pData[2] = pData[0] ^ pData[2];
        pData[0] = pData[2] ^ pData[0];
    }
}
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
    #ifdef _MSC_VER
        CSR_PixelBuffer* pPixelBuffer;
        size_t           offset;
        unsigned         dataOffset;
        unsigned         headerSize;
        unsigned short   bpp;
        unsigned short   compressed;
        unsigned char    signature[2] = {0};
    #else
        CSR_PixelBuffer* pPixelBuffer;
        size_t           offset;
        unsigned         dataOffset;
        unsigned         headerSize;
        unsigned short   bpp;
        unsigned short   compressed;
        unsigned char    signature[2];
    #endif

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
    pPixelBuffer->m_DataLength   = (size_t)pPixelBuffer->m_Stride * (size_t)pPixelBuffer->m_Height;
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
CSR_PixelBuffer* csrPixelBufferFromTgaFile(const char* pFileName)
{
    CSR_Buffer*      pBuffer;
    CSR_PixelBuffer* pPixelBuffer;

    // open bitmap file
    pBuffer = csrFileOpen(pFileName);

    // convert to pixel buffer
    pPixelBuffer = csrPixelBufferFromTgaBuffer(pBuffer);

    // free the bitmap file
    csrBufferRelease(pBuffer);

    return pPixelBuffer;
}
//---------------------------------------------------------------------------
CSR_PixelBuffer* csrPixelBufferFromTgaBuffer(const CSR_Buffer* pBuffer)
{
    CSR_TGAHeader    header;
    CSR_PixelBuffer* pPixelBuffer;
    size_t           offset = 0;

    // validate the input
    if (!pBuffer)
        return 0;

    // read the header
    if (!csrBufferRead(pBuffer, &offset, sizeof(CSR_TGAHeader), 1, &header))
        return 0;

    // is image file valid?
    if (header.m_PixelSize != 8 && header.m_PixelSize != 24 && header.m_PixelSize != 32)
        return 0;

    offset += header.m_IdentSize;

    // is offset out of bounds?
    if (offset >= pBuffer->m_Length)
        return 0;

    // create a new pixel buffer
    pPixelBuffer = (CSR_PixelBuffer*)malloc(sizeof(CSR_PixelBuffer));

    // succeeded?
    if (!pPixelBuffer)
        return 0;

    // initialize it
    csrPixelBufferInit(pPixelBuffer);

    // get the image width and height
    pPixelBuffer->m_PixelType = CSR_PT_BGR;
    pPixelBuffer->m_ImageType = CSR_IT_Raw;
    pPixelBuffer->m_Width     = header.m_Width [0] + (header.m_Width [1] << 8);
    pPixelBuffer->m_Height    = header.m_Height[0] + (header.m_Height[1] << 8);
    pPixelBuffer->m_Stride    = (((pPixelBuffer->m_Width) * 3 + 3) / 4) * 4 - ((pPixelBuffer->m_Width) * 3 % 4);

    switch (header.m_ImageType)
    {
        case 1:
        {
            size_t         i;
            size_t         j;
            unsigned       cMapSize;
            unsigned char* pCMap;
            unsigned char* pIndexes;
            unsigned char* pSrc;
            unsigned char* pDst;

            // validate the color map entry size
            if (header.m_CMapEntrySize != 8 && header.m_CMapEntrySize != 24 && header.m_CMapEntrySize != 32)
            {
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            // get the color map size
            cMapSize = header.m_CMapSize[0] + (header.m_CMapSize[1] << 8);

            // calculate the byte per pixels, and allocate memory for color map
            pPixelBuffer->m_BytePerPixel = header.m_CMapEntrySize / 8;
            pCMap                        = (unsigned char*)malloc((size_t)pPixelBuffer->m_BytePerPixel *
                                                                  (size_t)cMapSize);

            // read the color map
            if (!csrBufferRead(pBuffer, &offset, (size_t)pPixelBuffer->m_BytePerPixel * (size_t)cMapSize, 1, pCMap))
            {
                free(pCMap);
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            // do convert the BGR pixel order to RGB?
            if (pPixelBuffer->m_BytePerPixel >= 3)
                csrBGRToRGB(pCMap, pPixelBuffer->m_BytePerPixel * cMapSize, pPixelBuffer->m_BytePerPixel);

            // allocate memory for pixels and get indexes pointer
            pPixelBuffer->m_DataLength =   (size_t)pPixelBuffer->m_BytePerPixel *
                                           (size_t)pPixelBuffer->m_Width        *
                                           (size_t)pPixelBuffer->m_Height;
            pPixelBuffer->m_pData      =   (unsigned char*)malloc(pPixelBuffer->m_DataLength);
            pIndexes                   = &((unsigned char*)pPixelBuffer->m_pData)[(pPixelBuffer->m_BytePerPixel - 1) *
                                                                                   pPixelBuffer->m_Width             *
                                                                                   pPixelBuffer->m_Height];

            // read the index table
            if (!csrBufferRead(pBuffer, &offset, (size_t)pPixelBuffer->m_Width * (size_t)pPixelBuffer->m_Height, 1, pIndexes))
            {
                free(pCMap);
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            pSrc = pIndexes;
            pDst = &((unsigned char*)pPixelBuffer->m_pData)[pPixelBuffer->m_DataLength];

            // copy the pixels
            for (i = 0; i < pPixelBuffer->m_Height; ++i)
            {
                unsigned char* pRow;

                pDst -= (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                pRow  = pDst;

                for (j = 0; j < pPixelBuffer->m_Width; ++j)
                {
                    memcpy(pRow, &pCMap[*pSrc++ * pPixelBuffer->m_BytePerPixel], pPixelBuffer->m_BytePerPixel);
                    pRow += pPixelBuffer->m_BytePerPixel;
                }
            }

            free(pCMap);

            return pPixelBuffer;
        }

        case 2:
        {
            size_t         i;
            unsigned char* pDst;

            // calculate the byte per pixels
            pPixelBuffer->m_BytePerPixel = header.m_PixelSize / 8;

            // allocate memory for pixels and get destination pointer
            pPixelBuffer->m_DataLength =   (size_t)pPixelBuffer->m_BytePerPixel *
                                           (size_t)pPixelBuffer->m_Width        *
                                           (size_t)pPixelBuffer->m_Height;
            pPixelBuffer->m_pData      =   (unsigned char*)malloc(pPixelBuffer->m_DataLength);
            pDst                       = &((unsigned char*)pPixelBuffer->m_pData)[pPixelBuffer->m_DataLength];

            // read the pixels
            for (i = 0; i < pPixelBuffer->m_Height; ++i)
            {
                pDst -= (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;

                // read the row
                if (!csrBufferRead(pBuffer, &offset, (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width, 1, pDst))
                {
                    csrPixelBufferRelease(pPixelBuffer);
                    return 0;
                }
            }

            // do convert the BGR pixel order to RGB?
            if (pPixelBuffer->m_BytePerPixel >= 3)
                csrBGRToRGB(pPixelBuffer->m_pData,
                            (int)pPixelBuffer->m_DataLength,
                            pPixelBuffer->m_BytePerPixel);

            return pPixelBuffer;
        }

        case 9:
        {
            unsigned       cMapSize;
            unsigned char* pCMap;
            unsigned char* pEnd;
            unsigned char* pDst;
            unsigned char  buffer[128];

            // validate the color map entry size
            if (header.m_CMapEntrySize != 8 && header.m_CMapEntrySize != 24 && header.m_CMapEntrySize != 32)
            {
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            // get the color map size
            cMapSize = header.m_CMapSize[0] + (header.m_CMapSize[1] << 8);

            // calculate the byte per pixels, and allocate memory for color map
            pPixelBuffer->m_BytePerPixel = header.m_CMapEntrySize / 8;
            pCMap                        = (unsigned char*) malloc((size_t)pPixelBuffer->m_BytePerPixel *
                                                                   (size_t)cMapSize);

            // read the color map
            if (!csrBufferRead(pBuffer, &offset, (size_t)pPixelBuffer->m_BytePerPixel * (size_t)cMapSize, 1, pCMap))
            {
                free(pCMap);
                csrPixelBufferRelease(pPixelBuffer);
                return 0;
            }

            // do convert the BGR pixel order to RGB?
            if (pPixelBuffer->m_BytePerPixel >= 3)
                csrBGRToRGB(pCMap, pPixelBuffer->m_BytePerPixel * cMapSize, pPixelBuffer->m_BytePerPixel);

            // allocate memory for pixels
            pPixelBuffer->m_DataLength = (size_t)pPixelBuffer->m_BytePerPixel *
                                         (size_t)pPixelBuffer->m_Width        *
                                         (size_t)pPixelBuffer->m_Height;
            pPixelBuffer->m_pData      = (unsigned char*)malloc(pPixelBuffer->m_DataLength);

            // iterate though pixels to read
            for (pEnd = &((unsigned char*)pPixelBuffer->m_pData)[pPixelBuffer->m_DataLength],
                    pDst = pEnd - (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                    pDst >= (unsigned char*)pPixelBuffer->m_pData;)
            {
                int c;

                // read next char
                if (!csrBufferRead(pBuffer, &offset, 1, 1, &c))
                {
                    free(pCMap);
                    csrPixelBufferRelease(pPixelBuffer);
                    return 0;
                }

                // compressed pixel?
                if (c & 0x80)
                {
                    int            index;
                    unsigned char* pCol;
                    unsigned char* pRun;

                    // read next char
                    if (!csrBufferRead(pBuffer, &offset, 1, 1, &index))
                    {
                        free(pCMap);
                        csrPixelBufferRelease(pPixelBuffer);
                        return 0;
                    }

                    pCol = &pCMap[index * pPixelBuffer->m_BytePerPixel];

                    c -= 0x7F;
                    c *= pPixelBuffer->m_BytePerPixel;

                    while (c > 0 && pDst >= (unsigned char*)pPixelBuffer->m_pData)
                    {
                        int   n  = 0;
                        float nF = 0.0f;

                        csrMathMin((float)c, (float)(pEnd - pDst), &nF);

                        n = (int)nF;

                        for (pRun = pDst + n; pDst < pRun; pDst += pPixelBuffer->m_BytePerPixel)
                            memcpy(pDst, pCol, pPixelBuffer->m_BytePerPixel);

                        c -= n;

                        if (pDst >= pEnd)
                        {
                            pEnd -=        (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                            pDst  = pEnd - (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                        }
                    }
                }
                else
                {
                    c += 1;

                    while (c > 0 && pDst >= (unsigned char*)pPixelBuffer->m_pData)
                    {
                        unsigned char* pSrc;
                        int            n  = 0;
                        float          nF = 0.0f;

                        csrMathMin((float)c, (float)(pEnd - pDst) / pPixelBuffer->m_BytePerPixel, &nF);

                        n = (int)nF;

                        // read the row
                        if (!csrBufferRead(pBuffer, &offset, n, 1, &buffer))
                        {
                            free(pCMap);
                            csrPixelBufferRelease(pPixelBuffer);
                            return 0;
                        }

                        for (pSrc = buffer; pSrc < &buffer[n]; pDst += pPixelBuffer->m_BytePerPixel)
                            memcpy(pDst, &pCMap[*pSrc++ * pPixelBuffer->m_BytePerPixel], pPixelBuffer->m_BytePerPixel);

                        c -= n;

                        if (pDst >= pEnd)
                        {
                            pEnd -=        (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                            pDst  = pEnd - (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                        }
                    }
                }
            }

            free(pCMap);

            return pPixelBuffer;
        }

        case 10:
        {
            unsigned char* pEnd;
            unsigned char* pDst;
            unsigned char  buffer[4];

            // calculate the byte per pixels
            pPixelBuffer->m_BytePerPixel = header.m_PixelSize / 8;

            // allocate memory for pixels
            pPixelBuffer->m_DataLength = (size_t)pPixelBuffer->m_BytePerPixel *
                                         (size_t)pPixelBuffer->m_Width        *
                                         (size_t)pPixelBuffer->m_Height;
            pPixelBuffer->m_pData      = (unsigned char*)malloc(pPixelBuffer->m_DataLength);

            // iterate though pixels to read
            for (pEnd = &((unsigned char*)pPixelBuffer->m_pData)[pPixelBuffer->m_DataLength],
                    pDst = pEnd - (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                    pDst >= (unsigned char*)pPixelBuffer->m_pData;)
            {
                int c;

                // read next char
                if (!csrBufferRead(pBuffer, &offset, 1, 1, &c))
                {
                    csrPixelBufferRelease(pPixelBuffer);
                    return 0;
                }

                // compressed pixel?
                if (c & 0x80)
                {
                    // read pixel
                    if (!csrBufferRead(pBuffer, &offset, pPixelBuffer->m_BytePerPixel, 1, buffer))
                    {
                        csrPixelBufferRelease(pPixelBuffer);
                        return 0;
                    }

                    c -= 0x7F;

                    if (pPixelBuffer->m_BytePerPixel >= 3)
                    {
                        buffer[0] = buffer[0] ^ buffer[2];
                        buffer[2] = buffer[0] ^ buffer[2];
                        buffer[0] = buffer[2] ^ buffer[0];
                    }

                    c *= pPixelBuffer->m_BytePerPixel;

                    while (c > 0)
                    {
                        unsigned char* pRun;
                        int            n  = 0;
                        float          nF = 0.0f;

                        csrMathMin((float)c, (float)(pEnd - pDst), &nF);

                        n = (int)nF;

                        for (pRun = pDst + n; pDst < pRun; pDst += pPixelBuffer->m_BytePerPixel)
                            memcpy(pDst, buffer, pPixelBuffer->m_BytePerPixel);

                        c -= n;

                        if (pDst >= pEnd)
                        {
                            pEnd -=        (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                            pDst  = pEnd - (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;

                            if (pDst < (unsigned char*)pPixelBuffer->m_pData)
                                break;
                        }
                    }
                }
                else
                {
                    c += 1;
                    c *= pPixelBuffer->m_BytePerPixel;

                    while (c > 0)
                    {
                        int   n  = 0;
                        float nF = 0.0f;

                        csrMathMin((float)c, (float)(pEnd - pDst), &nF);

                        n = (int)nF;

                        // read pixel
                        if (!csrBufferRead(pBuffer, &offset, n, 1, pDst))
                        {
                            csrPixelBufferRelease(pPixelBuffer);
                            return 0;
                        }

                        if (pPixelBuffer->m_BytePerPixel >= 3)
                            csrBGRToRGB(pDst, n, pPixelBuffer->m_BytePerPixel);

                        pDst += n;
                        c    -= n;

                        if (pDst >= pEnd)
                        {
                            pEnd -=        (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;
                            pDst  = pEnd - (size_t)pPixelBuffer->m_BytePerPixel * (size_t)pPixelBuffer->m_Width;

                            if (pDst < (unsigned char*)pPixelBuffer->m_pData)
                                break;
                        }
                    }
                }
            }

            return pPixelBuffer;
        }

        default:
            csrPixelBufferRelease(pPixelBuffer);
            return 0;
    }
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

    // release the bump map content
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
