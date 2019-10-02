/****************************************************************************
 * ==> CSR_Common ----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the common functions and types        *
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

#ifndef CSR_CommonH
#define CSR_CommonH

// std
#include <stddef.h>

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_CSR_Version        1.05
#define M_CSR_Error_Code     0xFFFFFFFF // yes this is a 32 bit error code, but enough for this engine
#define M_CSR_Unknown_Index -1
#define M_CSR_Epsilon        1.0E-3     // epsilon value used for tolerance

//---------------------------------------------------------------------------
// Enumerators
//---------------------------------------------------------------------------

/**
* Endianness type
*/
typedef enum
{
    CSR_E_LittleEndian,
    CSR_E_BigEndian,
} CSR_EEndianness;

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* RGBA color
*@note Values are in percent, between 0.0f (0%) and 1.0f (100%)
*/
typedef struct
{
    float m_R;
    float m_G;
    float m_B;
    float m_A;
} CSR_Color;

/**
* Array item
*/
typedef struct
{
    void* m_pData;
    int   m_AutoFree;
} CSR_ArrayItem;

/**
* Array
*/
typedef struct
{
    CSR_ArrayItem* m_pItem;
    size_t         m_Count;
} CSR_Array;

/**
* Memory buffer
*/
typedef struct
{
    void*  m_pData;
    size_t m_Length;
} CSR_Buffer;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // Memory functions
        //-------------------------------------------------------------------

        /**
        * Allocates or reallocates a new block of memory
        *@param pMemory - previous memory block to reallocate, if 0 a new block will be allocated
        *@param size - size of a single item in the memory block, in bytes
        *@param count - number of items the memory block will contain
        *@return newly allocated or reallocated memory block, 0 on error
        *@note If the newly allocated size is smaller than the existing one, the remaining extra
        *      memory will be freed internally
        *@note The memory pointed by pMemory is preserved in case the reallocation failed
        *@note The new memory block should be freed with the free() function when becoming useless
        */
        void* csrMemoryAlloc(void* pMemory, size_t size, size_t count);

        /**
        * Detects if the target system endianness is big or little
        *@return the target system endianness
        */
        CSR_EEndianness csrMemoryEndianness(void);

        /**
        * Swaps the content of a memory from big endian to little endian, or vice-versa
        *@param[in, out] pMemory - memory to swap, swapped memory on function ends
        *@param size - size of the memory to swap
        */
        void csrMemorySwap(void* pMemory, size_t size);

        //-------------------------------------------------------------------
        // Math functions
        //-------------------------------------------------------------------

        /**
        * Gets the minimum value between 2 values
        *@param a - first value to compare
        *@param b - second value to compare with
        *@param[out] pR - the found minimum value
        */
        void csrMathMin(float a, float b, float* pR);

        /**
        * Gets the maximum value between 2 values
        *@param a - first value to compare
        *@param b - second value to compare with
        *@param[out] pR - the found maximum value
        */
        void csrMathMax(float a, float b, float* pR);

        /**
        * Clamps a value between a range
        *@param value - value to clamp
        *@param minVal - min range to clamp
        *@param maxVal - max range to clamp
        *@param[out] pR - the found maximum value
        */
        void csrMathClamp(float value, float minVal, float maxVal, float* pR);

        /**
        * Checks if a value is between a range
        *@param value - value to check
        *@param rangeStart - range start value
        *@param rangeEnd - range end value
        *@param tolerance - tolerance for calculation
        *@return 1 if value is between the range, otherwise 0
        */
        int csrMathBetween(float value, float rangeStart, float rangeEnd, float tolerance);

        /**
        * Rounds a value to the closest integer
        *@param value - value to round
        *@param[out] pR - rounded value
        */
        void csrMathRound(float value, float* pR);

        /**
        * Rounds a value to his next exponent
        *@param value - value to round
        *@param exp - exponent to which the value will be rounded
        *@param[out] pR - rounded value
        */
        void csrMathRoundToExp(float value, unsigned exp, float* pR);

        //-------------------------------------------------------------------
        // Color functions
        //-------------------------------------------------------------------

        /**
        * Converts a color to a RGBA color
        *@param pColor - color to convert
        *@return converted color
        */
        unsigned csrColorToRGBA(const CSR_Color* pColor);

        /**
        * Converts a RGBA color to a color
        *@param color - RGBA color to convert
        *@param[out] pColor - converted color
        */
        void csrRGBAToColor(unsigned color, CSR_Color* pColor);

        /**
        * Converts a 32 bit BGR color (Windows style) to a RGBA color
        *@param color - BGR color to convert
        *@return RGBA color
        */
        unsigned csrColorBGRToRGBA(unsigned color);

        /**
        * Converts a 32 bit ABGR color (Windows style) to a RGBA color
        *@param color - ABGR color to convert
        *@return RGBA color
        */
        unsigned csrColorABGRToRGBA(unsigned color);

        //-------------------------------------------------------------------
        // Array functions
        //-------------------------------------------------------------------

        /**
        * Creates a new array
        *@return newly created array, 0 on error
        *@note The array must be released when no longer used, see csrArrayRelease()
        */
        CSR_Array* csrArrayCreate(void);

        /**
        * Releases an array and frees his memory
        *@param[in, out] pArray - array to release
        */
        void csrArrayRelease(CSR_Array* pArray);

        /**
        * Initializes an array structure
        *@param[in, out] pArray - array to initialize
        */
        void csrArrayInit(CSR_Array* pArray);

        /**
        * Adds a data to an array
        *@param pData - data to add
        *@param[in, out] pArray - array to add to
        *@param autoFree - if 1, the data will be freed while the array will be released
        */
        void csrArrayAdd(void* pData, CSR_Array* pArray, int autoFree);

        /**
        * Adds a data to an array, but only if not exists in the array
        *@param pData - data to add
        *@param[in, out] pArray - array to add to
        *@param autoFree - if 1, the data will be freed while the array will be released
        */
        void csrArrayAddUnique(void* pData, CSR_Array* pArray, int autoFree);

        /**
        * Gets the index of a data
        *@param pData - data for which the index should be found
        *@param pArray - array to search in
        *@return index, M_CSR_Unknown_Index if not found or on error
        */
        size_t csrArrayGetIndex(void* pData, const CSR_Array* pArray);

        /**
        * Gets the array item containing a data
        *@param pData - data for which the array item should be found
        *@param pArray - array to search in
        *@return array item, 0 if not found or on error
        */
        CSR_ArrayItem* csrArrayGetItem(void* pData, const CSR_Array* pArray);

        /**
        * Gets the index of a data from a staring inndex
        *@param pData - data for which the index should be found
        *@param startIndex - start index to search from
        *@param pArray - array to search in
        *@return index, M_CSR_Unknown_Index if not found or on error
        */
        size_t csrArrayGetIndexFrom(void* pData, size_t startIndex, const CSR_Array* pArray);

        /**
        * Gets the array item containing a data from a staring inndex
        *@param pData - data for which the array item should be found
        *@param startIndex - start index to search from
        *@param pArray - array to search in
        *@return array item, 0 if not found or on error
        */
        CSR_ArrayItem* csrArrayGetItemFrom(void* pData, size_t startIndex, const CSR_Array* pArray);

        /**
        * Deletes an item from an array
        *@param pData - item data to search and delete
        *@param[in, out] pArray - array to delete from
        */
        void csrArrayDelete(void* pData, CSR_Array* pArray);

        /**
        * Deletes an item from an array
        *@param index - item index to delete
        *@param[in, out] pArray - array to delete from
        */
        void csrArrayDeleteAt(size_t index, CSR_Array* pArray);

        //-------------------------------------------------------------------
        // Buffer functions
        //-------------------------------------------------------------------

        /**
        * Creates a new buffer
        *@return newly created buffer, 0 on error
        *@note The buffer must be released when no longer used, see csrBufferRelease()
        */
        CSR_Buffer* csrBufferCreate(void);

        /**
        * Releases a buffer and frees his memory
        *@param[in, out] pBuffer - buffer to release
        */
        void csrBufferRelease(CSR_Buffer* pBuffer);

        /**
        * Initializes a buffer structure
        *@param[in, out] pBuffer - buffer to initialize
        */
        void csrBufferInit(CSR_Buffer* pBuffer);

        /**
        * Reads a data from a buffer
        *@param pBuffer - buffer to read from
        *@param[in, out] pOffset - offset to read from, new offset position after function ends
        *@param length - length of one data to read, in bytes
        *@param count - number of data to read in the buffer, in bytes
        *@param[out] pData - read data
        *@return 1 on success, otherwise 0
        */
        int csrBufferRead(const CSR_Buffer* pBuffer,
                                size_t*     pOffset,
                                size_t      length,
                                size_t      count,
                                void*       pData);

        /**
        * Writes a data in a buffer
        *@param pBuffer - buffer to write to
        *@param pData - data to write
        *@param length - length of one data to read, in bytes
        *@param count - number of data to read in the buffer, in bytes
        *@return 1 on success, otherwise 0
        *@note The data will always be written on the buffer end
        */
        int csrBufferWrite(CSR_Buffer* pBuffer,
                     const void*       pData,
                           size_t      length,
                           size_t      count);

        //-------------------------------------------------------------------
        // File functions
        //-------------------------------------------------------------------

        /**
        * Gets the size of a file
        *@param pFileName - file name for which the size should be get
        *@return file size in bytes, 0 on error
        */
        size_t csrFileSize(const char* pFileName);

        /**
        * Opens a file
        *@param pFileName - file name
        *@return a buffer containing the file content, 0 on error
        *@note The buffer must be released when no longer used, see csrReleaseBuffer()
        */
        CSR_Buffer* csrFileOpen(const char* pFileName);

        /**
        * Saves a buffer content inside a file
        *@param pFileName - file name
        *@param pBuffer - buffer to save to file
        *@return 1 on success, otherwise 0
        */
        int csrFileSave(const char* pFileName, const CSR_Buffer* pBuffer);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Common.c"
#endif

#endif
