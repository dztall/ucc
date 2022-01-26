/****************************************************************************
 * ==> CSR_Model -----------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the model functions and types         *
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

#ifndef CSR_WavefrontH
#define CSR_WavefrontH

// compactStar engine
#include "CSR_Common.h"
#include "CSR_Geometry.h"
#include "CSR_Texture.h"
#include "CSR_Vertex.h"
#include "CSR_Model.h"

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

/**
* WaveFront vertex
*/
typedef struct
{
    float* m_pData;
    size_t m_Count;
} CSR_WavefrontVertex;

/**
* WaveFront normal
*/
typedef struct
{
    float* m_pData;
    size_t m_Count;
} CSR_WavefrontNormal;

/**
* WaveFront texture coordinate
*/
typedef struct
{
    float* m_pData;
    size_t m_Count;
} CSR_WavefrontTexCoord;

/**
* WaveFront face
*/
typedef struct
{
    int*   m_pData;
    size_t m_Count;
} CSR_WavefrontFace;

/**
* WaveFront group
*/
typedef struct
{
    CSR_WavefrontVertex*   m_pVertex;
    CSR_WavefrontNormal*   m_pNormal;
    CSR_WavefrontTexCoord* m_pUV;
    CSR_WavefrontFace*     m_pFace;
} CSR_WavefrontGroup;

/**
* WaveFront object
*/
typedef struct
{
    CSR_WavefrontGroup* m_pGroup;
    size_t              m_Count;
} CSR_WavefrontObject;

#ifdef __cplusplus
    extern "C"
    {
#endif
        //-------------------------------------------------------------------
        // WaveFront model functions
        //-------------------------------------------------------------------

        /**
        * Create a mesh from a buffer containing a WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return model containing the WaveFront file on success, otherwise 0
        *@note The model content should be released using the csrModelRelease function when useless
        */
        CSR_Model* csrWaveFrontCreate(const CSR_Buffer*           pBuffer,
                                      const CSR_VertexFormat*     pVertFormat,
                                      const CSR_VertexCulling*    pVertCulling,
                                      const CSR_Material*         pMaterial,
                                      const CSR_fOnGetVertexColor fOnGetVertexColor,
                                      const CSR_fOnApplySkin      fOnApplySkin,
                                      const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Opens a WaveFront file
        *@param pFileName - WaveFront file name to open
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@param fOnDeleteTexture - callback function to notify the GPU that a texture should be deleted
        *@return model containing the WaveFront file on success, otherwise 0
        *@note The model content should be released using the csrModelRelease when useless
        */
        CSR_Model* csrWaveFrontOpen(const char*                 pFileName,
                                    const CSR_VertexFormat*     pVertFormat,
                                    const CSR_VertexCulling*    pVertCulling,
                                    const CSR_Material*         pMaterial,
                                    const CSR_fOnGetVertexColor fOnGetVertexColor,
                                    const CSR_fOnApplySkin      fOnApplySkin,
                                    const CSR_fOnDeleteTexture  fOnDeleteTexture);

        /**
        * Reads a commented line from a WaveFront buffer
        *@param pBuffer - source buffer to read from
        *@param[in, out] pChar - last read char
        *@param[in, out] pIndex - char index
        */
        void csrWaveFrontReadComment(const CSR_Buffer* pBuffer, char* pChar, size_t* pIndex);

        /**
        * Reads a vertex from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pVertex - vertex array in which the new read vertex should be added
        */
        void csrWaveFrontReadVertex(const CSR_Buffer*          pBuffer,
                                          char*                pChar,
                                          size_t*              pIndex,
                                          CSR_WavefrontVertex* pVertex);

        /**
        * Reads a normal from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pNormal - normal array in which the new read normal should be added
        */
        void csrWaveFrontReadNormal(const CSR_Buffer*          pBuffer,
                                          char*                pChar,
                                          size_t*              pIndex,
                                          CSR_WavefrontNormal* pNormal);

        /**
        * Reads a texture coordinate from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pTexCoord - texture coordinate array in which the new read texture coordinate should be added
        */
        void csrWaveFrontReadTextureCoordinate(const CSR_Buffer*            pBuffer,
                                                     char*                  pChar,
                                                     size_t*                pIndex,
                                                     CSR_WavefrontTexCoord* pTexCoord);

        /**
        * Reads a face from the WaveFront file
        *@param pBuffer - buffer containing the WaveFront file
        *@param[in, out] pChar - last read char, new read char on function ends
        *@param[in, out] pIndex - last read char index, new read char index on function ends
        *@param[in, out] pFace - face array in which the new read face should be added
        */
        void csrWaveFrontReadFace(const CSR_Buffer*        pBuffer,
                                        char*              pChar,
                                        size_t*            pIndex,
                                        CSR_WavefrontFace* pFace);

        /**
        * Reads an unknown line from a WaveFront buffer
        *@param pBuffer - source buffer to read from
        *@param[in, out] pChar - last read char
        *@param[in, out] pIndex - char index
        */
        void csrWaveFrontReadUnknown(const CSR_Buffer* pBuffer, char* pChar, size_t* pIndex);

        /**
        * Converts a read value to float and adds it in an array
        *@param pBuffer - buffer containing the value to convert
        *@param[in, out] pArray - float array in which the value should be added
        *@param[in, out] pCount - array count
        */
        void csrWaveFrontConvertFloat(const char* pBuffer, float** pArray, size_t* pCount);

        /**
        * Converts a read value to int and adds it in an array
        *@param pBuffer - buffer containing the value to convert
        *@param[in, out] pArray - int array in which the value should be added
        *@param[in, out] pCount - array count
        */
        void csrWaveFrontConvertInt(const char* pBuffer, int** pArray, size_t* pCount);

        /**
        * Builds a face from WaveFront data
        *@param pVertex - vertex array read in WaveFront file
        *@param pNormal - normal array read in WaveFront file
        *@param pUV - texture coordinates array read in WaveFront file
        *@param pFace - face array read in WaveFront file
        *@param pVertFormat - model vertex format, if 0 the default format will be used
        *@param pVertCulling - model vertex culling, if 0 the default culling will be used
        *@param pMaterial - mesh material, if 0 the default material will be used
        *@param objectChanging - if 1, a new object should be created
        *@param groupChanging - if 1, a new group should be created
        *@param[in, out] pModel - model in which the WaveFront data should be built
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnApplySkin - called when a skin should be applied to the model
        *@return 1 on success, otherwise 0
        */
        int csrWaveFrontBuildFace(const CSR_WavefrontVertex*   pVertex,
                                  const CSR_WavefrontNormal*   pNormal,
                                  const CSR_WavefrontTexCoord* pUV,
                                  const CSR_WavefrontFace*     pFace,
                                  const CSR_VertexFormat*      pVertFormat,
                                  const CSR_VertexCulling*     pVertCulling,
                                  const CSR_Material*          pMaterial,
                                        int                    objectChanging,
                                        int                    groupChanging,
                                        CSR_Model*             pModel,
                                  const CSR_fOnGetVertexColor  fOnGetVertexColor,
                                  const CSR_fOnApplySkin       fOnApplySkin);

        /**
        * Builds a vertex buffer from a WaveFront data
        *@param pVertex - vertex array read in WaveFront file
        *@param pNormal - normal array read in WaveFront file
        *@param pUV - texture coordinates array read in WaveFront file
        *@param pFace - face array read in WaveFront file
        *@param[in, out] pVB - vertex buffer in which the WaveFront data should be built
        *@param fOnGetVertexColor - get vertex color callback function to use, 0 if not used
        *@param fOnApplySkin - called when a skin should be applied to the model
        */
        void csrWaveFrontBuildVertexBuffer(const CSR_WavefrontVertex*   pVertex,
                                           const CSR_WavefrontNormal*   pNormal,
                                           const CSR_WavefrontTexCoord* pUV,
                                           const CSR_WavefrontFace*     pFace,
                                                 CSR_VertexBuffer*      pVB,
                                           const CSR_fOnGetVertexColor  fOnGetVertexColor,
                                           const CSR_fOnApplySkin       fOnApplySkin);

#ifdef __cplusplus
    }
#endif

//---------------------------------------------------------------------------
// Compiler
//---------------------------------------------------------------------------

// needed in mobile c compiler to link the .h file with the .c
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "CSR_Wavefront.c"
#endif

#endif
