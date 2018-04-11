/****************************************************************************
 * ==> CSR_Common ----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the common functions and types        *
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

#include "CSR_Common.h"

// std
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

//---------------------------------------------------------------------------
// Memory functions
//---------------------------------------------------------------------------
void* csrMemoryAlloc(void* pMemory, size_t size, size_t count)
{
    // do reallocate a previously existing memory?
    if (!pMemory)
        // no, just allocate the new memory
        return malloc(size * count);

    // yes, reallocate the existing memory to include the new size
    return realloc(pMemory, size * count);
}
//---------------------------------------------------------------------------
CSR_EEndianness csrMemoryEndianness(void)
{
    int i = 1;

    if (!*((char*)&i))
        return CSR_E_BigEndian;

    return CSR_E_LittleEndian;
}
//---------------------------------------------------------------------------
void csrMemorySwap(void* pMemory, size_t size)
{
    unsigned char* pBytes   = (unsigned char*)pMemory;
    size_t         halfSize = size >> 1;
    size_t         lastIndex;
    size_t         i;

    // iterate through bytes to swap
    for (i = 0; i < halfSize; ++i)
    {
        // swap the byte content (XOR method)
        lastIndex         = (size - 1) - i;
        pBytes[i]         = pBytes[i] ^ pBytes[lastIndex];
        pBytes[lastIndex] = pBytes[i] ^ pBytes[lastIndex];
        pBytes[i]         = pBytes[i] ^ pBytes[lastIndex];
    }
}
//---------------------------------------------------------------------------
// Math functions
//---------------------------------------------------------------------------
void csrMathMin(float a, float b, float* pR)
{
    if (a < b)
    {
        *pR = a;
        return;
    }

    *pR = b;
}
//---------------------------------------------------------------------------
void csrMathMax(float a, float b, float* pR)
{
    if (a > b)
    {
        *pR = a;
        return;
    }

    *pR = b;
}
//---------------------------------------------------------------------------
void csrMathClamp(float value, float minVal, float maxVal, float* pR)
{
    csrMathMax(value, minVal, pR);
    csrMathMin(*pR,   maxVal, pR);
}
//---------------------------------------------------------------------------
int csrMathBetween(float value, float rangeStart, float rangeEnd, float tolerance)
{
    float minVal;
    float maxVal;

    csrMathMin(rangeStart, rangeEnd, &minVal);
    csrMathMax(rangeStart, rangeEnd, &maxVal);

    // check if each value is between start and end limits considering tolerance
    if (value >= (minVal - tolerance) && value <= (maxVal + tolerance))
        return 1;

    return 0;
}
//----------------------------------------------------------------------------
void csrMathRound(float value, float* pR)
{
    if (value < 0.0f)
        *pR = ceil(value - 0.5f);
    else
        *pR = floor(value + 0.5f);
}
//----------------------------------------------------------------------------
void csrMathRoundToExp(float value, unsigned exp, float* pR)
{
    const float power = value * pow(10.0f, exp);

    csrMathRound(power, pR);

    *pR *= pow(0.1f, exp);
}
//---------------------------------------------------------------------------
// Color functions
//---------------------------------------------------------------------------
unsigned csrColorToRGBA(const CSR_Color* pColor)
{
    // no color to convert?
    if (!pColor)
        return 0;

    return (((unsigned)(pColor->m_R * 255.0f) << 24) |
            ((unsigned)(pColor->m_G * 255.0f) << 16) |
            ((unsigned)(pColor->m_B * 255.0f) << 8)  |
             (unsigned)(pColor->m_A * 255.0f));
}
//---------------------------------------------------------------------------
void csrRGBAToColor(unsigned color, CSR_Color* pColor)
{
    // no color to convert?
    if (!pColor)
        return;

    pColor->m_R = (float)((color & 0xFF) >> 24) / 255.0f;
    pColor->m_G = (float)((color & 0xFF) >> 16) / 255.0f;
    pColor->m_B = (float)((color & 0xFF) >> 8)  / 255.0f;
    pColor->m_A = (float) (color & 0xFF)        / 255.0f;
}
//---------------------------------------------------------------------------
unsigned csrColorBGRToRGBA(unsigned color)
{
    return (((color        & 0xFF) << 24) |
           (((color >> 8)  & 0xFF) << 16) |
           (((color >> 16) & 0xFF) << 8)  |
                             0xFF);
}
//---------------------------------------------------------------------------
unsigned csrColorABGRToRGBA(unsigned color)
{
    return (((color        & 0xFF) << 24) |
           (((color >> 8)  & 0xFF) << 16) |
           (((color >> 16) & 0xFF) << 8)  |
            ((color >> 24) & 0xFF));
}
//---------------------------------------------------------------------------
// Array functions
//---------------------------------------------------------------------------
CSR_Array* csrArrayCreate(void)
{
    // create a new array
    CSR_Array* pArray = (CSR_Array*)malloc(sizeof(CSR_Array));

    // succeeded?
    if (!pArray)
        return 0;

    // initialize the array content
    csrArrayInit(pArray);

    return pArray;
}
//---------------------------------------------------------------------------
void csrArrayRelease(CSR_Array* pArray)
{
    // no array to release?
    if (!pArray)
        return;

    // free the array items
    if (pArray->m_pItem)
    {
        size_t i;

        // iterate through each item and free his content
        for (i = 0; i < pArray->m_Count; ++i)
            // do free the item content?
            if (pArray->m_pItem[i].m_AutoFree)
                free(pArray->m_pItem[i].m_pData);

        // free the item array
        free(pArray->m_pItem);
    }

    // free the array
    free(pArray);
}
//---------------------------------------------------------------------------
void csrArrayInit(CSR_Array* pArray)
{
    // no array to initialize?
    if (!pArray)
        return;

    // initialize the array content
    pArray->m_pItem = 0;
    pArray->m_Count = 0;
}
//---------------------------------------------------------------------------
void csrArrayAdd(void* pData, CSR_Array* pArray, int autoFree)
{
    size_t         index;
    CSR_ArrayItem* pNewItem;

    // validate the inputs
    if (!pArray || !pData)
        return;

    // add an item to the array
    pNewItem = (CSR_ArrayItem*)csrMemoryAlloc(pArray->m_pItem,
                                              sizeof(CSR_ArrayItem),
                                              pArray->m_Count + 1);

    // succeeded?
    if (!pNewItem)
        return;

    // get the new item index
    index = pArray->m_Count;

    // update the array
    pArray->m_pItem = pNewItem;
    ++pArray->m_Count;

    // set the data in the newly created item
    pArray->m_pItem[index].m_pData    = pData;
    pArray->m_pItem[index].m_AutoFree = autoFree;
}
//---------------------------------------------------------------------------
void csrArrayAddUnique(void* pData, CSR_Array* pArray, int autoFree)
{
    // check if the data was already added to this array
    if (csrArrayGetIndex(pData, pArray) != (size_t)M_CSR_Unknown_Index)
        return;

    // still not added, add it
    csrArrayAdd(pData, pArray, autoFree);
}
//---------------------------------------------------------------------------
size_t csrArrayGetIndex(void* pData, const CSR_Array* pArray)
{
    return csrArrayGetIndexFrom(pData, 0, pArray);
}
//---------------------------------------------------------------------------
CSR_ArrayItem* csrArrayGetItem(void* pData, const CSR_Array* pArray)
{
    return csrArrayGetItemFrom(pData, 0, pArray);
}
//---------------------------------------------------------------------------
size_t csrArrayGetIndexFrom(void* pData, size_t startIndex, const CSR_Array* pArray)
{
    size_t i;

    // validate the inputs
    if (!pArray || !pData)
        return M_CSR_Unknown_Index;

    // search for data index
    for (i = startIndex; i < pArray->m_Count; ++i)
        if (pArray->m_pItem[i].m_pData == pData)
            return i;

    return M_CSR_Unknown_Index;
}
//---------------------------------------------------------------------------
CSR_ArrayItem* csrArrayGetItemFrom(void* pData, size_t startIndex, const CSR_Array* pArray)
{
    // get the data index
    const size_t index = csrArrayGetIndexFrom(pData, startIndex, pArray);

    // found it?
    if (index == (size_t)M_CSR_Unknown_Index)
        return 0;

    return &pArray->m_pItem[index];
}
//---------------------------------------------------------------------------
void csrArrayDelete(void* pData, CSR_Array* pArray)
{
    size_t i;

    // validate the inputs
    if (!pArray || !pData)
        return;

    // search for an item matching with the data in the array, delete it if found
    for (i = 0; i < pArray->m_Count; ++i)
        if (pArray->m_pItem[i].m_pData == pData)
        {
            csrArrayDeleteAt(i, pArray);
            return;
        }
}
//---------------------------------------------------------------------------
void csrArrayDeleteAt(size_t index, CSR_Array* pArray)
{
    CSR_ArrayItem* pNewItem;

    // empty array?
    if (!pArray || !pArray->m_pItem || !pArray->m_Count)
        return;

    // is index out of bounds?
    if (index >= pArray->m_Count)
        return;

    // was the last item in the array?
    if (pArray->m_Count == 1)
    {
        // free the array
        if (pArray->m_pItem)
            free(pArray->m_pItem);

        // don't recreate nothing
        pArray->m_pItem = 0;
        pArray->m_Count = 0;

        return;
    }

    // create an item list one item smaller than the existing one
    pNewItem = (CSR_ArrayItem*)csrMemoryAlloc(0, sizeof(CSR_ArrayItem), pArray->m_Count - 1);

    // succeeded?
    if (!pNewItem)
        return;

    // copy all the remaining items in the new array
    if (!index)
        memcpy(pNewItem, pArray->m_pItem + 1, (pArray->m_Count - 1) * sizeof(CSR_ArrayItem));
    else
    if (index == (pArray->m_Count - 1))
        memcpy(pNewItem, pArray->m_pItem, (pArray->m_Count - 1) * sizeof(CSR_ArrayItem));
    else
    {
        memcpy(pNewItem,         pArray->m_pItem,             sizeof(CSR_ArrayItem) *                    index);
        memcpy(pNewItem + index, pArray->m_pItem + index + 1, sizeof(CSR_ArrayItem) * (pArray->m_Count - index - 1));
    }

    // free the array item content, if required
    if (pArray->m_pItem[index].m_AutoFree && pArray->m_pItem[index].m_pData)
        free(pArray->m_pItem[index].m_pData);

    // free the current array items
    if (pArray->m_pItem)
        free(pArray->m_pItem);

    // update the array with the newly recreated items
    pArray->m_pItem = pNewItem;
    --pArray->m_Count;
}
//---------------------------------------------------------------------------
// Buffer functions
//---------------------------------------------------------------------------
CSR_Buffer* csrBufferCreate(void)
{
    // create a new buffer
    CSR_Buffer* pBuffer = (CSR_Buffer*)malloc(sizeof(CSR_Buffer));

    // succeeded?
    if (!pBuffer)
        return 0;

    // initialize the buffer content
    csrBufferInit(pBuffer);

    return pBuffer;
}
//---------------------------------------------------------------------------
void csrBufferRelease(CSR_Buffer* pBuffer)
{
    // no buffer to release?
    if (!pBuffer)
        return;

    // free the buffer content
    if (pBuffer->m_pData)
        free(pBuffer->m_pData);

    // free the buffer
    free(pBuffer);
}
//---------------------------------------------------------------------------
void csrBufferInit(CSR_Buffer* pBuffer)
{
    // no buffer to initialize?
    if (!pBuffer)
        return;

    // initialize the buffer content
    pBuffer->m_pData  = 0;
    pBuffer->m_Length = 0;
}
//---------------------------------------------------------------------------
int csrBufferRead(const CSR_Buffer* pBuffer,
                        size_t*     pOffset,
                        size_t      length,
                        size_t      count,
                        void*       pData)
{
    size_t lengthToRead;

    // no source buffer, offset or destination data?
    if (!pBuffer || !pOffset || !pData)
        return 0;

    // offset exceeds the buffer length?
    if (*pOffset >= pBuffer->m_Length)
        return 0;

    // calculate the length to read
    lengthToRead = length * count;

    // size to read exceeds the buffer length?
    if (*pOffset + lengthToRead > pBuffer->m_Length)
    {
        // correct it
        lengthToRead = pBuffer->m_Length - *pOffset;

        // something remains to be read?
        if (!lengthToRead)
            return 0;
    }

    // read the data
    memcpy(pData, ((unsigned char*)pBuffer->m_pData) + *pOffset, lengthToRead);

    // update the offset position
    *pOffset += lengthToRead;

    return 1;
}
//---------------------------------------------------------------------------
int csrBufferWrite(      CSR_Buffer* pBuffer,
                   const void*       pData,
                         size_t      length,
                         size_t      count)
{
    size_t offset;
    size_t lengthToWrite;
    void*  pNewData;

    // validate the inputs
    if (!pBuffer || !pData)
        return 0;

    // nothing to copy?
    if (!length || !count)
        return 1;

    // get the current offset to write from
    offset        = pBuffer->m_Length;
    lengthToWrite = (length * count);

    // extend the buffer memory to include the new data
    pNewData = csrMemoryAlloc(pBuffer->m_pData, pBuffer->m_Length + lengthToWrite, 1);

    // succeeded?
    if (!pNewData)
        return 0;

    // update the buffer
    pBuffer->m_pData   = pNewData;
    pBuffer->m_Length += lengthToWrite;

    // write the data
    memcpy(((unsigned char*)pBuffer->m_pData) + offset, pData, lengthToWrite);

    return 1;
}
//---------------------------------------------------------------------------
// File functions
//---------------------------------------------------------------------------
size_t csrFileSize(const char* pFileName)
{
    FILE*  pFile;
    size_t fileSize;

    // open the file
    pFile = fopen(pFileName, "rb");

    // succeeded?
    if (!pFile)
        return 0;

    // measure the file size
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fclose(pFile);

    return fileSize;
}
//---------------------------------------------------------------------------
CSR_Buffer* csrFileOpen(const char* pFileName)
{
    FILE*       pFile;
    CSR_Buffer* pBuffer;
    size_t      bytesRead;

    if (!pFileName)
        return 0;

    // create a new buffer
    pBuffer = csrBufferCreate();

    // succeeded?
    if (!pBuffer)
        return 0;

    // open the file
    pFile = fopen(pFileName, "rb");

    // succeeded?
    if (!pFile)
        return pBuffer;

    // measure the file size
    fseek(pFile, 0, SEEK_END);
    pBuffer->m_Length = ftell(pFile);

    // is file empty?
    if (!pBuffer->m_Length)
    {
        fclose(pFile);
        return pBuffer;
    }

    // allocate memory for the file content
    pBuffer->m_pData = (unsigned char*)malloc(pBuffer->m_Length + 1);

    // succeeded?
    if (!pBuffer->m_pData)
    {
        fclose(pFile);
        pBuffer->m_Length = 0;
        return pBuffer;
    }

    // certify that the buffer content is well terminated (useful in case the data is a text)
    ((unsigned char*)pBuffer->m_pData)[pBuffer->m_Length] = '\0';

    // reset cursor position on the file start
    fseek(pFile, 0, SEEK_SET);

    // read the whole file content
    bytesRead = fread(pBuffer->m_pData, 1, pBuffer->m_Length, pFile);

    // close the file
    fclose(pFile);

    // file was read successfully?
    if (bytesRead != pBuffer->m_Length)
    {
        // clear the buffer
        free(pBuffer->m_pData);
        pBuffer->m_pData  = 0;
        pBuffer->m_Length = 0;

        return pBuffer;
    }

    return pBuffer;
}
//---------------------------------------------------------------------------
int csrFileSave(const char* pFileName, const CSR_Buffer* pBuffer)
{
    FILE*  pFile;
    size_t bytesWritten;

    // validate the inputs
    if (!pFileName || !pBuffer)
        return 0;

    // open the file
    pFile = fopen(pFileName, "wb");

    // succeeded?
    if (!pFile)
        return 0;

    // write the buffer content
    bytesWritten = fwrite(pBuffer->m_pData, pBuffer->m_Length, 1, pFile);

    // close the file
    fclose(pFile);

    return (bytesWritten == pBuffer->m_Length);
}
//---------------------------------------------------------------------------
