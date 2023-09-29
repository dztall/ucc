/****************************************************************************
 * ==> CSR_Iqm -------------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a Inter-Quake Model (.iqm) reader     *
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

#include "CSR_Iqm.h"

// std
#include <stdlib.h>
#include <string.h>

// visual studio specific code
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <math.h>
#else
    #include <math.h>
#endif

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_IQM_ID            "INTERQUAKEMODEL\0"
#define M_IQM_File_Version  2

//---------------------------------------------------------------------------
// Private structures
//---------------------------------------------------------------------------

/**
* Vertex array type
*@note All vertex array entries must ordered as defined below, if present.
*      I.e. position comes before normal, comes before ..., comes before custom,
*      where a format and size is given, this means models intended for portable
*      use should use these an IQM implementation is not required to honor any
*      other format/size than those recommended. However, it may support other
*      format/size combinations for these types if it desires
*/
typedef enum
{
    CSR_IQM_Position     = 0,   // float, 3
    CSR_IQM_TexCoord     = 1,   // float, 2
    CSR_IQM_Normal       = 2,   // float, 3
    CSR_IQM_Tangent      = 3,   // float, 4
    CSR_IQM_BlendIndexes = 4,   // unsigned byte, 4
    CSR_IQM_BlendWeights = 5,   // unsigned byte, 4
    CSR_IQM_Color        = 6,   // unsigned byte, 4
    CSR_IQM_Custom       = 0x10 // all values up to CSR_IQM_CUSTOM are reserved for future use
                                // any value >= CSR_IQM_CUSTOM is interpreted as CUSTOM type
                                // the value then defines an offset into the string table, where
                                // offset = value - CSR_IQM_CUSTOM this must be a valid string
                                // naming the type
} CSR_EIQMVertexArrayType;

/**
* Vertex array format
*/
typedef enum
{
    CSR_IQM_Byte   = 0,
    CSR_IQM_UByte  = 1,
    CSR_IQM_Short  = 2,
    CSR_IQM_UShort = 3,
    CSR_IQM_Int    = 4,
    CSR_IQM_UInt   = 5,
    CSR_IQM_Half   = 6,
    CSR_IQM_Float  = 7,
    CSR_IQM_Double = 8,
} CSR_EIQMVertexArrayFormat;

/**
* Animation flags
*/
typedef enum
{
    CSR_IQM_LOOP = 1 << 0
} CSR_IEIQMAnimFlags;

/**
* Inter-Quake Model (.iqm) header
*@note Offset fields are relative to the beginning of the CSR_IQMHeader struct
*@note Offset fields must be set to 0 when the particular data is empty
*@note Offset fields must be aligned to at least 4 byte boundaries
*/
typedef struct
{
    char     m_ID[16];          // the "INTERQUAKEMODEL\0" identifier, 0 terminated
    unsigned m_Version;         // version, must be 2
    unsigned m_FileSize;
    unsigned m_Flags;
    unsigned m_TextCount;
    unsigned m_TextOffset;
    unsigned m_MeshCount;
    unsigned m_MeshOffset;
    unsigned m_VertexArrayCount;
    unsigned m_VertexCount;
    unsigned m_VertexArrayOffset;
    unsigned m_TriangleCount;
    unsigned m_TriangleOffset;
    unsigned m_AdjacencyOffset;
    unsigned m_JointCount;
    unsigned m_JointOffset;
    unsigned m_PoseCount;
    unsigned m_PoseOffset;
    unsigned m_AnimCount;
    unsigned m_AnimOffset;
    unsigned m_FrameCount;
    unsigned m_FrameChannelCount;
    unsigned m_FrameOffset;
    unsigned m_BoundsOffset;
    unsigned m_CommentCount;
    unsigned m_CommentOffset;
    unsigned m_ExtensionCount;
    unsigned m_ExtensionOffset; // these are stored as a linked list, not as a contiguous array
} CSR_IQMHeader;

/**
* Inter-Quake Model text list
*/
typedef struct
{
    char**  m_pTexts;
    size_t* m_pOffsets;
    size_t  m_Count;

} CSR_IQMTexts;

/**
* Inter-Quake Model mesh
*/
typedef struct
{
    unsigned m_Name;     // mesh unique name, if desired
    unsigned m_Material; // set to a non-unique material or texture name
    unsigned m_FirstVertex;
    unsigned m_VertexCount;
    unsigned m_FirstTriangle;
    unsigned m_TriangleCount;
} CSR_IQMMesh;

/**
* Inter-Quake Model meshes
*/
typedef struct
{
    CSR_IQMMesh* m_pMesh;
    size_t       m_Count;
} CSR_IQMMeshes;

/**
* Inter-Quake Model vertex array
*/
typedef struct
{
    unsigned m_Type;   // type or custom name
    unsigned m_Flags;
    unsigned m_Format; // component format
    unsigned m_Size;   // component count
    unsigned m_Offset; // offset to array of tightly packed components, with
                       // num_vertexes * size total entries, must be aligned
                       // to max(sizeof(format), 4)
} CSR_IQMVertexArray;

/**
* Inter-Quake Model vertex arrays
*/
typedef struct
{
    CSR_IQMVertexArray* m_pVertexArray;
    size_t              m_Count;
} CSR_IQMVertexArrays;

/**
* Inter-Quake Model triangle
*/
typedef struct
{
    unsigned m_Vertex[3];
} CSR_IQMTriangle;

/**
* Inter-Quake Model triangles
*/
typedef struct
{
    CSR_IQMTriangle* m_pTriangle;
    size_t           m_Count;
} CSR_IQMTriangles;

/**
* Inter-Quake Model adjacency
*@note Each value is the index of the adjacent triangle for edge 0, 1, and 2,
*      where ~0 (= -1) indicates no adjacent triangle. Indexes are relative
*      to the CSR_IQMHeader.m_TriangleOffset array and span all meshes, where
*      0 is the first triangle, 1 is the second, 2 is the third, etc...
*/
typedef struct
{
    unsigned m_Triangle[3];
} CSR_IQMAdjacency;

/**
* Inter-Quake Model Adjacencies
*/
typedef struct
{
    CSR_IQMAdjacency* m_pAdjacency;
    size_t            m_Count;
} CSR_IQMAdjacencies;

/**
* Inter-Quake Model joint
*@note Translate is translation <Tx, Ty, Tz>, and rotate is quaternion rotation
*      <Qx, Qy, Qz, Qw>. Rotation is in relative/parent local space. Scale is
*      pre-scaling <Sx, Sy, Sz>, so:
*      output = (input * scale) * rotation + translation
*/
typedef struct
{
    unsigned m_Name;
    int      m_Parent; // parent < 0 means this is a root bone
    float    m_Translate[3];
    float    m_Rotate[4];
    float    m_Scale[3];
} CSR_IQMJoint;

/**
* Inter-Quake Model joints
*/
typedef struct
{
    CSR_IQMJoint* m_pJoint;
    size_t        m_Count;
} CSR_IQMJoints;

/**
* Inter-Quake Model pose
*@note Channels 0..2 are translation <Tx, Ty, Tz> and channels 3..6 are quaternion
*      rotation <Qx, Qy, Qz, Qw>. Rotation is in relative/parent local space.
*      Channels 7..9 are scale <Sx, Sy, Sz>, so:
*      output = (input * scale) * rotation + translation
*/
typedef struct
{
    int      m_Parent;      // parent < 0 means this is a root bone
    unsigned m_ChannelMask; // mask of which 10 channels are present for this joint pose
    float    m_ChannelOffset[10];
    float    m_ChannelScale[10];
} CSR_IQMPose;

/**
* Inter-Quake Model poses
*/
typedef struct
{
    CSR_IQMPose* m_pPose;
    size_t       m_Count;
} CSR_IQMPoses;

/**
* Inter-Quake Model animation
*/
typedef struct
{
    unsigned m_Name;
    unsigned m_FirstFrame;
    unsigned m_FrameCount;
    float    m_FrameRate;
    unsigned m_Flags;
} CSR_IQMAnim;

/**
* Inter-Quake Model animations
*/
typedef struct
{
    CSR_IQMAnim* m_pAnim;
    size_t       m_Count;
} CSR_IQMAnims;

/**
* Inter-Quake Model bounds
*/
typedef struct
{
    float m_Bbmins[3]; // the minimum coordinates of the bounding box for this animation frame
    float m_Bbmaxs[3]; // the maximum coordinates of the bounding box for this animation frame
    float m_Xyradius;  // the circular radius in the X-Y plane
    float m_Radius;    // the spherical radius
} CSR_IQMBounds;

/**
* Inter-Quake Model bounds array
*/
typedef struct
{
    CSR_IQMBounds* m_pBounds;
    size_t         m_Count;
} CSR_IQMBoundsArray;

/**
* Inter-Quake Model comments
*/
typedef struct
{
    char**  m_pComments;
    size_t* m_pOffsets;
    size_t  m_Count;

} CSR_IQMComments;

/**
* Inter-Quake Model extension
*/
typedef struct
{
    unsigned m_Name;
    unsigned m_DataCount;
    unsigned m_DataOffset;
    unsigned m_NextExtOffset; // pointer to next extension
} CSR_IQMExtension;

/**
* Inter-Quake Model extensions
*/
typedef struct
{
    CSR_IQMExtension* m_pExtension;
    size_t            m_Count;
} CSR_IQMExtensions;

/**
* Inter-Quake Model vertex
*/
typedef struct
{
    CSR_Vector3   m_Position;
    CSR_Vector2   m_TexCoord;
    CSR_Vector3   m_Normal;
    float         m_Tangent[4];
    unsigned char m_BlendIndices[4];
    unsigned char m_BlendWeights[4];
    unsigned char m_Color[4];
} CSR_IQMVertex;

/**
* Inter-Quake Model vertices
*/
typedef struct
{
    CSR_IQMVertex* m_pVertex;
    size_t         m_Count;
} CSR_IQMVertices;

//---------------------------------------------------------------------------
// IQM model private functions
//---------------------------------------------------------------------------
int csrIQMReadHeader(const CSR_Buffer* pBuffer, size_t* pOffset, CSR_IQMHeader* pHeader)
{
    int success = 1;

    // read header from buffer
    success &= csrBufferRead(pBuffer, pOffset, sizeof(pHeader->m_ID), 1, &pHeader->m_ID);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_Version);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_FileSize);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_Flags);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_TextCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_TextOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_MeshCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_MeshOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_VertexArrayCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_VertexCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_VertexArrayOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_TriangleCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_TriangleOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_AdjacencyOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_JointCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_JointOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_PoseCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_PoseOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_AnimCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_AnimOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_FrameCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_FrameChannelCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_FrameOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_BoundsOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_CommentCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_CommentOffset);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_ExtensionCount);
    success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned),      1, &pHeader->m_ExtensionOffset);

    #ifdef CONVERT_ENDIANNESS
        // the read bytes are inverted and should be swapped if the target system is big endian
        if (success && csrMemoryEndianness() == CSR_E_BigEndian)
        {
            // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
            //csrMemorySwap(&pHeader->m_ID,                sizeof(pHeader->m_ID)); // todo -cCheck -oJean: check if required
            csrMemorySwap(&pHeader->m_Version,           sizeof(unsigned));
            csrMemorySwap(&pHeader->m_FileSize,          sizeof(unsigned));
            csrMemorySwap(&pHeader->m_Flags,             sizeof(unsigned));
            csrMemorySwap(&pHeader->m_TextCount,         sizeof(unsigned));
            csrMemorySwap(&pHeader->m_TextOffset,        sizeof(unsigned));
            csrMemorySwap(&pHeader->m_MeshCount,         sizeof(unsigned));
            csrMemorySwap(&pHeader->m_MeshOffset,        sizeof(unsigned));
            csrMemorySwap(&pHeader->m_VertexArrayCount,  sizeof(unsigned));
            csrMemorySwap(&pHeader->m_VertexCount,       sizeof(unsigned));
            csrMemorySwap(&pHeader->m_VertexArrayOffset, sizeof(unsigned));
            csrMemorySwap(&pHeader->m_TriangleCount,     sizeof(unsigned));
            csrMemorySwap(&pHeader->m_TriangleOffset,    sizeof(unsigned));
            csrMemorySwap(&pHeader->m_AdjacencyOffset,   sizeof(unsigned));
            csrMemorySwap(&pHeader->m_JointCount,        sizeof(unsigned));
            csrMemorySwap(&pHeader->m_JointOffset,       sizeof(unsigned));
            csrMemorySwap(&pHeader->m_PoseCount,         sizeof(unsigned));
            csrMemorySwap(&pHeader->m_PoseOffset,        sizeof(unsigned));
            csrMemorySwap(&pHeader->m_AnimCount,         sizeof(unsigned));
            csrMemorySwap(&pHeader->m_AnimOffset,        sizeof(unsigned));
            csrMemorySwap(&pHeader->m_FrameCount,        sizeof(unsigned));
            csrMemorySwap(&pHeader->m_FrameChannelCount, sizeof(unsigned));
            csrMemorySwap(&pHeader->m_FrameOffset,       sizeof(unsigned));
            csrMemorySwap(&pHeader->m_BoundsOffset,      sizeof(unsigned));
            csrMemorySwap(&pHeader->m_CommentCount,      sizeof(unsigned));
            csrMemorySwap(&pHeader->m_CommentOffset,     sizeof(unsigned));
            csrMemorySwap(&pHeader->m_ExtensionCount,    sizeof(unsigned));
            csrMemorySwap(&pHeader->m_ExtensionOffset,   sizeof(unsigned));
        }
    #endif

    return success;
}
//---------------------------------------------------------------------------
int csrIQMCopyStr(char* pTextBuffer, size_t start, size_t pos, char*** pTexts, size_t** pOffsets, size_t* pCount)
{
    char**  pTextArray;
    size_t* pOffsetArray;
    size_t  index;
    size_t  len;

    if (!pTexts)
        return 0;

    if (!pCount)
        return 0;

    // get text index
    index = *pCount;

    // allocate memory for a new string in the array
    pTextArray = (char**)csrMemoryAlloc(*pTexts, sizeof(char*), *pCount + 1);

    // succeeded?
    if (!pTextArray)
        return 0;

    // allocate memory for a new offset in the array
    pOffsetArray = (size_t*)csrMemoryAlloc(*pOffsets, sizeof(size_t), *pCount + 1);

    // succeeded?
    if (!pOffsetArray)
        return 0;

    // allocate new text array and offset array
    *pTexts   = pTextArray;
    *pOffsets = pOffsetArray;
    ++(*pCount);

    // calculate string length
    len = pos - start;

    // is string empty?
    if (!len)
    {
        // set an empty string
        (*pTexts)[index]    = (char*)malloc(1);
        (*pTexts)[index][0] = 0x0;
        return 1;
    }

    // allocate memory for new text string
    (*pTexts)[index] = (char*)malloc(len + 1);

    // succeeded?
    if (!(*pTexts)[index])
    {
        free(pTextBuffer);
        return 0;
    }

    // copy the text string
    memcpy((*pTexts)[index], &pTextBuffer[start], len);
    (*pTexts)[index][len] = 0x0;

    // copy the offset
    (*pOffsets)[index] = start;

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMGetTextIndex(const CSR_IQMTexts* pTexts, size_t offset)
{
    size_t i;

    for (i = 0; i < pTexts->m_Count; ++i)
        if (pTexts->m_pOffsets[i] == offset)
            return (int)i;

    return -1;
}
//---------------------------------------------------------------------------
int csrIQMReadTexts(const CSR_Buffer*    pBuffer,
                          size_t*        pOffset,
                    const CSR_IQMHeader* pHeader,
                          CSR_IQMTexts*  pTexts)
{
    size_t i;
    size_t start;
    char*  pTextBuffer;

    pTexts->m_pTexts   = 0;
    pTexts->m_pOffsets = 0;
    pTexts->m_Count    = 0;

    // no text to read?
    if (!pHeader->m_TextCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_TextOffset;

    // create a temporary buffer to read the whole texts
    pTextBuffer = (char*)malloc(pHeader->m_TextCount);

    // read the texts (contained as one whole string separated by \0)
    if (!csrBufferRead(pBuffer, pOffset, pHeader->m_TextCount, 1, pTextBuffer))
        return 0;

    start = 0;

    // iterate through strings to extract
    for (i = 0; i < pHeader->m_TextCount; ++i)
        // dispatch char
        switch (pTextBuffer[i])
        {
            case '\0':
                // copy the string text
                if (!csrIQMCopyStr(pTextBuffer,
                                   start,
                                   i,
                                  &pTexts->m_pTexts,
                                  &pTexts->m_pOffsets,
                                  &pTexts->m_Count))
                {
                    free(pTextBuffer);
                    return 0;
                }

                start = i + 1;
                break;
        }

    // is there a remaining string to copy?
    if (start < pHeader->m_TextCount)
        // copy the string text
        if (!csrIQMCopyStr(pTextBuffer,
                           start,
                           pHeader->m_TextCount,
                          &pTexts->m_pTexts,
                          &pTexts->m_pOffsets,
                          &pTexts->m_Count))
        {
            free(pTextBuffer);
            return 0;
        }

    free(pTextBuffer);
    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadMeshes(const CSR_Buffer*    pBuffer,
                           size_t*        pOffset,
                     const CSR_IQMHeader* pHeader,
                           CSR_IQMMeshes* pMeshes)
{
    size_t i;
    int    success;

    pMeshes->m_pMesh = 0;
    pMeshes->m_Count = 0;

    // no mesh to read?
    if (!pHeader->m_MeshCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_MeshOffset;

    // allocate memory for the meshes
    pMeshes->m_pMesh = (CSR_IQMMesh*)malloc(pHeader->m_MeshCount * sizeof(CSR_IQMMesh));
    pMeshes->m_Count = pHeader->m_MeshCount;
    success          = 1;

    // iterate through meshes to read
    for (i = 0; i < pHeader->m_MeshCount; ++i)
    {
        // read the mesh content
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pMeshes->m_pMesh[i].m_Name);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pMeshes->m_pMesh[i].m_Material);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pMeshes->m_pMesh[i].m_FirstVertex);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pMeshes->m_pMesh[i].m_VertexCount);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pMeshes->m_pMesh[i].m_FirstTriangle);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pMeshes->m_pMesh[i].m_TriangleCount);

        #ifdef CONVERT_ENDIANNESS
            // the read bytes are inverted and should be swapped if the target system is big endian
            if (success && csrMemoryEndianness() == CSR_E_BigEndian)
            {
                // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                csrMemorySwap(&pMeshes->m_pMesh[i].m_Name,          sizeof(unsigned));
                csrMemorySwap(&pMeshes->m_pMesh[i].m_Material,      sizeof(unsigned));
                csrMemorySwap(&pMeshes->m_pMesh[i].m_FirstVertex,   sizeof(unsigned));
                csrMemorySwap(&pMeshes->m_pMesh[i].m_VertexCount,   sizeof(unsigned));
                csrMemorySwap(&pMeshes->m_pMesh[i].m_FirstTriangle, sizeof(unsigned));
                csrMemorySwap(&pMeshes->m_pMesh[i].m_TriangleCount, sizeof(unsigned));
            }
        #endif

        if (!success)
            return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadVertexArrays(const CSR_Buffer*          pBuffer,
                                 size_t*              pOffset,
                           const CSR_IQMHeader*       pHeader,
                                 CSR_IQMVertexArrays* pVertexArrays)
{
    size_t i;
    int    success;

    pVertexArrays->m_pVertexArray = 0;
    pVertexArrays->m_Count        = 0;

    // no vertex array to read?
    if (!pHeader->m_VertexArrayCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_VertexArrayOffset;

    // allocate memory for the vertex arrays
    pVertexArrays->m_pVertexArray = (CSR_IQMVertexArray*)malloc(pHeader->m_VertexArrayCount * sizeof(CSR_IQMVertexArray));
    pVertexArrays->m_Count        = pHeader->m_VertexArrayCount;
    success                       = 1;

    // iterate through vertex arrays to read
    for (i = 0; i < pHeader->m_VertexArrayCount; ++i)
    {
        // read the vertex array content
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pVertexArrays->m_pVertexArray[i].m_Type);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pVertexArrays->m_pVertexArray[i].m_Flags);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pVertexArrays->m_pVertexArray[i].m_Format);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pVertexArrays->m_pVertexArray[i].m_Size);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pVertexArrays->m_pVertexArray[i].m_Offset);

        #ifdef CONVERT_ENDIANNESS
            // the read bytes are inverted and should be swapped if the target system is big endian
            if (success && csrMemoryEndianness() == CSR_E_BigEndian)
            {
                // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                csrMemorySwap(&pVertexArrays->m_pVertexArray[i].m_Type,   sizeof(unsigned));
                csrMemorySwap(&pVertexArrays->m_pVertexArray[i].m_Flags,  sizeof(unsigned));
                csrMemorySwap(&pVertexArrays->m_pVertexArray[i].m_Format, sizeof(unsigned));
                csrMemorySwap(&pVertexArrays->m_pVertexArray[i].m_Size,   sizeof(unsigned));
                csrMemorySwap(&pVertexArrays->m_pVertexArray[i].m_Offset, sizeof(unsigned));
            }
        #endif

        if (!success)
            return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadTriangles(const CSR_Buffer*         pBuffer,
                              size_t*             pOffset,
                        const CSR_IQMHeader*      pHeader,
                              CSR_IQMTriangles*   pTriangles,
                              CSR_IQMAdjacencies* pAdjacencies)
{
    size_t i;
    size_t j;
    int    success;

    pTriangles->m_pTriangle = 0;
    pTriangles->m_Count     = 0;

    pAdjacencies->m_pAdjacency = 0;
    pAdjacencies->m_Count      = 0;

    // no triangle to read?
    if (!pHeader->m_TriangleCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_TriangleOffset;

    // allocate memory for the triangles
    pTriangles->m_pTriangle = (CSR_IQMTriangle*)malloc(pHeader->m_TriangleCount * sizeof(CSR_IQMTriangle));
    pTriangles->m_Count     = pHeader->m_TriangleCount;
    success                 = 1;

    // iterate through triangles to read
    for (i = 0; i < pHeader->m_TriangleCount; ++i)
        // read the triangle content
        for (j = 0; j < 3; ++j)
        {
            success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pTriangles->m_pTriangle[i].m_Vertex[j]);

            #ifdef CONVERT_ENDIANNESS
                // the read bytes are inverted and should be swapped if the target system is big endian
                if (success && csrMemoryEndianness() == CSR_E_BigEndian)
                    // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                    csrMemorySwap(&pTriangles->m_pTriangle[i].m_Vertex[j], sizeof(unsigned));
            #endif

            if (!success)
                return 0;
        }

    // set offset to buffer data start position
    *pOffset = pHeader->m_AdjacencyOffset;

    // allocate memory for the adjacencies
    pAdjacencies->m_pAdjacency = (CSR_IQMAdjacency*)malloc(pHeader->m_TriangleCount * sizeof(CSR_IQMAdjacency));
    pAdjacencies->m_Count      = pHeader->m_TriangleCount;

    // iterate through adjacencies to read
    for (i = 0; i < pHeader->m_TriangleCount; ++i)
        // read the adjacency content
        for (j = 0; j < 3; ++j)
        {
            success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pAdjacencies->m_pAdjacency[i].m_Triangle[j]);

            #ifdef CONVERT_ENDIANNESS
                // the read bytes are inverted and should be swapped if the target system is big endian
                if (success && csrMemoryEndianness() == CSR_E_BigEndian)
                    // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                    csrMemorySwap(&pAdjacencies->m_pAdjacency[i].m_Triangle[j], sizeof(unsigned));
            #endif

            if (!success)
                return 0;
        }

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadJoints(const CSR_Buffer*    pBuffer,
                           size_t*        pOffset,
                     const CSR_IQMHeader* pHeader,
                           CSR_IQMJoints* pJoints)
{
    size_t i;
    size_t j;
    int    success;

    pJoints->m_pJoint = 0;
    pJoints->m_Count  = 0;

    // no joint to read?
    if (!pHeader->m_JointCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_JointOffset;

    // allocate memory for the joints
    pJoints->m_pJoint = (CSR_IQMJoint*)malloc(pHeader->m_JointCount * sizeof(CSR_IQMJoint));
    pJoints->m_Count  = pHeader->m_JointCount;
    success           = 1;

    // iterate through joints to read
    for (i = 0; i < pHeader->m_JointCount; ++i)
    {
        // read the joint content
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pJoints->m_pJoint[i].m_Name);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(int),      1, &pJoints->m_pJoint[i].m_Parent);

        for (j = 0; j < 3; ++j)
            success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pJoints->m_pJoint[i].m_Translate[j]);

        for (j = 0; j < 4; ++j)
            success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pJoints->m_pJoint[i].m_Rotate[j]);

        for (j = 0; j < 3; ++j)
            success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pJoints->m_pJoint[i].m_Scale[j]);

        #ifdef CONVERT_ENDIANNESS
            // the read bytes are inverted and should be swapped if the target system is big endian
            if (success && csrMemoryEndianness() == CSR_E_BigEndian)
            {
                // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                csrMemorySwap(&pJoints->m_pJoint[i].m_Name,   sizeof(unsigned));
                csrMemorySwap(&pJoints->m_pJoint[i].m_Parent, sizeof(int));

                for (j = 0; j < 3; ++j)
                    csrMemorySwap(&pJoints->m_pJoint[i].m_Translate[j], sizeof(float));

                for (j = 0; j < 4; ++j)
                    csrMemorySwap(&pJoints->m_pJoint[i].m_Rotate[j], sizeof(float));

                for (j = 0; j < 3; ++j)
                    csrMemorySwap(&pJoints->m_pJoint[i].m_Scale[j], sizeof(float));
            }
        #endif

        if (!success)
            return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadPoses(const CSR_Buffer*    pBuffer,
                          size_t*        pOffset,
                    const CSR_IQMHeader* pHeader,
                          CSR_IQMPoses*  pPoses)
{
    size_t i;
    size_t j;
    int    success;

    pPoses->m_pPose = 0;
    pPoses->m_Count = 0;

    // no pose to read?
    if (!pHeader->m_PoseCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_PoseOffset;

    // allocate memory for the poses
    pPoses->m_pPose = (CSR_IQMPose*)malloc(pHeader->m_PoseCount * sizeof(CSR_IQMPose));
    pPoses->m_Count = pHeader->m_PoseCount;
    success         = 1;

    // iterate through poses to read
    for (i = 0; i < pHeader->m_PoseCount; ++i)
    {
        // read the pose content
        success &= csrBufferRead(pBuffer, pOffset, sizeof(int),      1, &pPoses->m_pPose[i].m_Parent);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pPoses->m_pPose[i].m_ChannelMask);

        for (j = 0; j < 10; ++j)
            success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pPoses->m_pPose[i].m_ChannelOffset[j]);

        for (j = 0; j < 10; ++j)
            success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pPoses->m_pPose[i].m_ChannelScale[j]);

        #ifdef CONVERT_ENDIANNESS
            // the read bytes are inverted and should be swapped if the target system is big endian
            if (success && csrMemoryEndianness() == CSR_E_BigEndian)
            {
                // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                csrMemorySwap(&pPoses->m_pPose[i].m_Parent,      sizeof(int));
                csrMemorySwap(&pPoses->m_pPose[i].m_ChannelMask, sizeof(unsigned));

                for (j = 0; j < 10; ++j)
                    csrMemorySwap(&pPoses->m_pPose[i].m_ChannelOffset[j], sizeof(float));

                for (j = 0; j < 10; ++j)
                    csrMemorySwap(&pPoses->m_pPose[i].m_ChannelScale[j], sizeof(float));
            }
        #endif

        if (!success)
            return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadAnims(const CSR_Buffer*    pBuffer,
                          size_t*        pOffset,
                    const CSR_IQMHeader* pHeader,
                          CSR_IQMAnims*  pAnims)
{
    size_t i;
    int    success;

    pAnims->m_pAnim = 0;
    pAnims->m_Count = 0;

    // no animation to read?
    if (!pHeader->m_AnimCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_AnimOffset;

    // allocate memory for the animations
    pAnims->m_pAnim = (CSR_IQMAnim*)malloc(pHeader->m_AnimCount * sizeof(CSR_IQMAnim));
    pAnims->m_Count = pHeader->m_AnimCount;
    success         = 1;

    // iterate through animations to read
    for (i = 0; i < pHeader->m_AnimCount; ++i)
    {
        // read the animation content
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pAnims->m_pAnim[i].m_Name);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pAnims->m_pAnim[i].m_FirstFrame);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pAnims->m_pAnim[i].m_FrameCount);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(float),    1, &pAnims->m_pAnim[i].m_FrameRate);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pAnims->m_pAnim[i].m_Flags);

        #ifdef CONVERT_ENDIANNESS
            // the read bytes are inverted and should be swapped if the target system is big endian
            if (success && csrMemoryEndianness() == CSR_E_BigEndian)
            {
                // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                csrMemorySwap(&pAnims->m_pAnim[i].m_Name,       sizeof(unsigned));
                csrMemorySwap(&pAnims->m_pAnim[i].m_FirstFrame, sizeof(unsigned));
                csrMemorySwap(&pAnims->m_pAnim[i].m_FrameCount, sizeof(unsigned));
                csrMemorySwap(&pAnims->m_pAnim[i].m_FrameRate,  sizeof(float));
                csrMemorySwap(&pAnims->m_pAnim[i].m_Flags,      sizeof(unsigned));
            }
        #endif

        if (!success)
            return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadBounds(const CSR_Buffer*         pBuffer,
                           size_t*             pOffset,
                     const CSR_IQMHeader*      pHeader,
                           CSR_IQMBoundsArray* pBoundsArray)
{
    size_t i;
    size_t j;
    int    success;

    pBoundsArray->m_pBounds = 0;
    pBoundsArray->m_Count   = 0;

    // no bounds to read?
    if (!pHeader->m_FrameCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_BoundsOffset;

    // allocate memory for the bounds
    pBoundsArray->m_pBounds = (CSR_IQMBounds*)malloc(pHeader->m_FrameCount * sizeof(CSR_IQMBounds));
    pBoundsArray->m_Count   = pHeader->m_FrameCount;
    success         = 1;

    // iterate through bounds to read
    for (i = 0; i < pHeader->m_FrameCount; ++i)
    {
        // read the bounds content
        for (j = 0; j < 3; ++j)
            success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pBoundsArray->m_pBounds[i].m_Bbmins[j]);

        for (j = 0; j < 3; ++j)
            success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pBoundsArray->m_pBounds[i].m_Bbmaxs[j]);

        success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pBoundsArray->m_pBounds[i].m_Xyradius);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(float), 1, &pBoundsArray->m_pBounds[i].m_Radius);

        #ifdef CONVERT_ENDIANNESS
            // the read bytes are inverted and should be swapped if the target system is big endian
            if (success && csrMemoryEndianness() == CSR_E_BigEndian)
            {
                // swap the read values in the memory (thus 0xAABBCCDD will become 0xDDCCBBAA)
                for (j = 0; j < 3; ++j)
                    csrMemorySwap(&pBoundsArray->m_pBounds[i].m_Bbmins[j], sizeof(float));

                for (j = 0; j < 3; ++j)
                    csrMemorySwap(&pBoundsArray->m_pBounds[i].m_Bbmaxs[j], sizeof(float));

                csrMemorySwap(&pBoundsArray->m_pBounds[i].m_Xyradius, sizeof(float));
                csrMemorySwap(&pBoundsArray->m_pBounds[i].m_Radius,   sizeof(float));
            }
        #endif

        if (!success)
            return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadComments(const CSR_Buffer*      pBuffer,
                             size_t*          pOffset,
                       const CSR_IQMHeader*   pHeader,
                             CSR_IQMComments* pComments)
{
    size_t i;
    size_t start;
    char*  pTextBuffer;

    pComments->m_pComments = 0;
    pComments->m_pOffsets  = 0;
    pComments->m_Count     = 0;

    // no comments to read?
    if (!pHeader->m_CommentCount)
        return 1;

    // set offset to buffer data start position
    *pOffset = pHeader->m_CommentOffset;

    // create a temporary buffer to read the whole comments
    pTextBuffer = (char*)malloc(pHeader->m_CommentCount);

    // read the comments (contained as one whole string separated by \0)
    if (!csrBufferRead(pBuffer, pOffset, pHeader->m_CommentCount, 1, pTextBuffer))
        return 0;

    start = 0;

    // iterate through strings to extract
    for (i = 0; i < pHeader->m_CommentCount; ++i)
        // dispatch char
        switch (pTextBuffer[i])
        {
            case '\0':
                // copy the string text
                if (!csrIQMCopyStr(pTextBuffer,
                                   start,
                                   i,
                                  &pComments->m_pComments,
                                  &pComments->m_pOffsets,
                                  &pComments->m_Count))
                {
                    free(pTextBuffer);
                    return 0;
                }

                start = i + 1;
                break;
        }

    // is there a remaining string to copy?
    if (start < pHeader->m_CommentCount)
        // copy the string text
        if (!csrIQMCopyStr(pTextBuffer,
                           start,
                           pHeader->m_CommentCount,
                          &pComments->m_pComments,
                          &pComments->m_pOffsets,
                          &pComments->m_Count))
        {
            free(pTextBuffer);
            return 0;
        }

    free(pTextBuffer);
    return 1;
}
//---------------------------------------------------------------------------
int csrIQMReadExtensions(const CSR_Buffer*        pBuffer,
                               size_t*            pOffset,
                         const CSR_IQMHeader*     pHeader,
                               CSR_IQMExtensions* pExtensions)
{
    /*
    size_t i;
    int    success;
    */

    pExtensions->m_pExtension = 0;
    pExtensions->m_Count      = 0;

    // no extension to read?
    if (!pHeader->m_ExtensionCount)
        return 1;

    // todo -cCheck -oJean: Don't sure about how to read extensions, I need an example
    return 0;

    /*
    // set offset to buffer data start position
    *pOffset = pHeader->m_ExtensionOffset;

    // allocate memory for the extensions
    pExtensions->m_pExtension = (CSR_IQMExtension*)malloc(pHeader->m_ExtensionCount * sizeof(CSR_IQMExtension));
    pExtensions->m_Count      = pHeader->m_ExtensionCount;
    success                   = 1;

    // iterate through extensions to read
    for (i = 0; i < pHeader->m_ExtensionCount; ++i)
    {
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pExtensions->m_pExtension[i].m_Name);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pExtensions->m_pExtension[i].m_DataCount);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pExtensions->m_pExtension[i].m_DataOffset);
        success &= csrBufferRead(pBuffer, pOffset, sizeof(unsigned), 1, &pExtensions->m_pExtension[i].m_NextExtOffset);

        #ifdef CONVERT_ENDIANNESS
            // the read bytes are inverted and should be swapped if the target system is big endian
            if (success && csrMemoryEndianness() == CSR_E_BigEndian)
            {
                csrMemorySwap(&pExtensions->m_pExtension[i].m_Name,          sizeof(unsigned));
                csrMemorySwap(&pExtensions->m_pExtension[i].m_DataCount,     sizeof(unsigned));
                csrMemorySwap(&pExtensions->m_pExtension[i].m_DataOffset,    sizeof(unsigned));
                csrMemorySwap(&pExtensions->m_pExtension[i].m_NextExtOffset, sizeof(unsigned));
            }
        #endif

        if (!success)
            return 0;
    }

    return 1;
    */
}
//---------------------------------------------------------------------------
int csrIQMBuildSrcVertices(const CSR_IQMHeader*       pHeader,
                           const CSR_IQMVertexArrays* pVertexArrays,
                           const CSR_Buffer*          pBuffer,
                                 CSR_IQMVertices*     pSrcVertices)
{
    size_t i;
    size_t j;
    size_t k;
    size_t offset;

    if (!pHeader)
        return 0;

    if (!pVertexArrays)
        return 0;

    if (!pBuffer)
        return 0;

    if (!pSrcVertices)
        return 0;

    // allocate memory for the vertex array
    pSrcVertices->m_pVertex = (CSR_IQMVertex*)calloc(pHeader->m_VertexCount, sizeof(CSR_IQMVertex));
    pSrcVertices->m_Count   = pHeader->m_VertexCount;

    // iterate through vertex arrays
    for (i = 0; i < pVertexArrays->m_Count; ++i)
    {
        // get vertex array and start offset to read from
        CSR_IQMVertexArray* pVertexArray = &pVertexArrays->m_pVertexArray[i];
                            offset       =  pVertexArray->m_Offset;

        // search for vertex data type to read
        switch (pVertexArray->m_Type)
        {
            case CSR_IQM_Position:
                // is vertex format valid?
                if (pVertexArray->m_Format != CSR_IQM_Float || pVertexArray->m_Size != 3)
                    return 0;

                // iterate through vertices to read
                for (j = 0; j < (size_t)pHeader->m_VertexCount; ++j)
                    // iterate through vertex data size
                    for (k = 0; k < (size_t)pVertexArray->m_Size; ++k)
                    {
                        // read vertex data from buffer
                        float value;

                        if (!csrBufferRead(pBuffer,
                                           &offset,
                                           sizeof(float),
                                           1,
                                           &value))
                            return 0;

                            #ifdef CONVERT_ENDIANNESS
                                // the read bytes are inverted and should be swapped if the target system is big endian
                                if (csrMemoryEndianness() == CSR_E_BigEndian)
                                    csrMemorySwap(&value, sizeof(float));
                            #endif

                        switch (k)
                        {
                            case 0: pSrcVertices->m_pVertex[j].m_Position.m_X = value; break;
                            case 1: pSrcVertices->m_pVertex[j].m_Position.m_Y = value; break;
                            case 2: pSrcVertices->m_pVertex[j].m_Position.m_Z = value; break;
                        }
                    }

                break;

            case CSR_IQM_TexCoord:
                // is vertex format valid?
                if (pVertexArray->m_Format != CSR_IQM_Float || pVertexArray->m_Size != 2)
                    return 0;

                // iterate through vertices to read
                for (j = 0; j < (size_t)pHeader->m_VertexCount; ++j)
                    // iterate through vertex data size
                    for (k = 0; k < (size_t)pVertexArray->m_Size; ++k)
                    {
                        // read vertex data from buffer
                        float value;

                        if (!csrBufferRead(pBuffer,
                                           &offset,
                                           sizeof(float),
                                           1,
                                           &value))
                            return 0;

                        #ifdef CONVERT_ENDIANNESS
                            // the read bytes are inverted and should be swapped if the target system is big endian
                            if (csrMemoryEndianness() == CSR_E_BigEndian)
                                csrMemorySwap(&value, sizeof(float));
                        #endif

                        switch (k)
                        {
                            case 0: pSrcVertices->m_pVertex[j].m_TexCoord.m_X = value; break;
                            case 1: pSrcVertices->m_pVertex[j].m_TexCoord.m_Y = value; break;
                        }
                    }

                break;

            case CSR_IQM_Normal:
                // is vertex format valid?
                if (pVertexArray->m_Format != CSR_IQM_Float || pVertexArray->m_Size != 3)
                    return 0;

                // iterate through vertices to read
                for (j = 0; j < (size_t)pHeader->m_VertexCount; ++j)
                    // iterate through vertex data size
                    for (k = 0; k < (size_t)pVertexArray->m_Size; ++k)
                    {
                        // read vertex data from buffer
                        float value;

                        if (!csrBufferRead(pBuffer,
                                           &offset,
                                           sizeof(float),
                                           1,
                                           &value))
                            return 0;

                        #ifdef CONVERT_ENDIANNESS
                            // the read bytes are inverted and should be swapped if the target system is big endian
                            if (csrMemoryEndianness() == CSR_E_BigEndian)
                                csrMemorySwap(&value, sizeof(float));
                        #endif

                        switch (k)
                        {
                            case 0: pSrcVertices->m_pVertex[j].m_Normal.m_X = value; break;
                            case 1: pSrcVertices->m_pVertex[j].m_Normal.m_Y = value; break;
                            case 2: pSrcVertices->m_pVertex[j].m_Normal.m_Z = value; break;
                        }
                    }

                break;

            case CSR_IQM_Color:
                // is vertex format valid?
                if (pVertexArray->m_Format != CSR_IQM_UByte || pVertexArray->m_Size != 4)
                    return 0;

                // iterate through vertices to read
                for (j = 0; j < (size_t)pHeader->m_VertexCount; ++j)
                    // iterate through vertex data size
                    for (k = 0; k < (size_t)pVertexArray->m_Size; ++k)
                        // read vertex data from buffer
                        if (!csrBufferRead(pBuffer,
                                           &offset,
                                           sizeof(unsigned char),
                                           1,
                                           &pSrcVertices->m_pVertex[j].m_Color[k]))
                            return 0;

                break;

            case CSR_IQM_Tangent:
                // is vertex format valid?
                if (pVertexArray->m_Format != CSR_IQM_Float || pVertexArray->m_Size != 4)
                    return 0;

                // iterate through vertices to read
                for (j = 0; j < (size_t)pHeader->m_VertexCount; ++j)
                    // iterate through vertex data size
                    for (k = 0; k < (size_t)pVertexArray->m_Size; ++k)
                    {
                        // read vertex data from buffer
                        if (!csrBufferRead(pBuffer,
                                           &offset,
                                           sizeof(float),
                                           1,
                                           &pSrcVertices->m_pVertex[j].m_Tangent[k]))
                            return 0;

                        #ifdef CONVERT_ENDIANNESS
                            // the read bytes are inverted and should be swapped if the target system is big endian
                            if (csrMemoryEndianness() == CSR_E_BigEndian)
                                csrMemorySwap(&pSrcVertices->m_pVertex[j].m_Tangent[k], sizeof(float));
                        #endif
                    }

                break;

            case CSR_IQM_BlendIndexes:
                // is vertex format valid?
                if (pVertexArray->m_Format != CSR_IQM_UByte || pVertexArray->m_Size != 4)
                    return 0;

                // iterate through vertices to read
                for (j = 0; j < (size_t)pHeader->m_VertexCount; ++j)
                    // iterate through vertex data size
                    for (k = 0; k < (size_t)pVertexArray->m_Size; ++k)
                        // read vertex data from buffer
                        if (!csrBufferRead(pBuffer,
                                           &offset,
                                           sizeof(unsigned char),
                                           1,
                                           &pSrcVertices->m_pVertex[j].m_BlendIndices[k]))
                            return 0;

                break;

            case CSR_IQM_BlendWeights:
                // is vertex format valid?
                if (pVertexArray->m_Format != CSR_IQM_UByte || pVertexArray->m_Size != 4)
                    return 0;

                // iterate through vertices to read
                for (j = 0; j < (size_t)pHeader->m_VertexCount; ++j)
                    // iterate through vertex data size
                    for (k = 0; k < (size_t)pVertexArray->m_Size; ++k)
                        // read vertex data from buffer
                        if (!csrBufferRead(pBuffer,
                                           &offset,
                                           sizeof(unsigned char),
                                           1,
                                           &pSrcVertices->m_pVertex[j].m_BlendWeights[k]))
                            return 0;

                break;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrIQMQuatToRotMat(const CSR_Quaternion* pQ, CSR_Matrix4* pR)
{
    // initialize the constants to use for conversion
    const float tx  = 2.0f    * pQ->m_X;
    const float ty  = 2.0f    * pQ->m_Y;
    const float tz  = 2.0f    * pQ->m_Z;
    const float txx = tx      * pQ->m_X;
    const float tyy = ty      * pQ->m_Y;
    const float tzz = tz      * pQ->m_Z;
    const float txy = tx      * pQ->m_Y;
    const float txz = tx      * pQ->m_Z;
    const float tyz = ty      * pQ->m_Z;
    const float twx = pQ->m_W * tx;
    const float twy = pQ->m_W * ty;
    const float twz = pQ->m_W * tz;

    // initialize the rotation matrix
    csrMat4Identity(pR);

    // build it
    pR->m_Table[0][0] = 1.0f - (tyy + tzz);
    pR->m_Table[1][0] =         txy - twz;
    pR->m_Table[2][0] =         txz + twy;

    pR->m_Table[0][1] =         txy + twz;
    pR->m_Table[1][1] = 1.0f - (txx + tzz);
    pR->m_Table[2][1] =         tyz - twx;

    pR->m_Table[0][2] =         txz - twy;
    pR->m_Table[1][2] =         tyz + twx;
    pR->m_Table[2][2] = 1.0f - (txx + tyy);
}
//---------------------------------------------------------------------------
int csrIQMPopulateBone(const CSR_IQMTexts* pTexts, const CSR_IQMJoint* pJoint, size_t jointIndex, CSR_Bone* pBone)
{
    char*              pBoneName;
    size_t             boneNameIndex;
    size_t             nameLen;
    #ifdef _MSC_VER
        CSR_Vector3    position = {0};
        CSR_Vector3    scaling  = {0};
        CSR_Quaternion rotation = {0};
    #else
        CSR_Vector3    position;
        CSR_Vector3    scaling;
        CSR_Quaternion rotation;
    #endif
    CSR_Matrix4        scaleMatrix;
    CSR_Matrix4        rotateMatrix;
    CSR_Matrix4        translateMatrix;
    CSR_Matrix4        buildMatrix;

    if (!pTexts || !pJoint || !pBone)
        return 0;

    // get the bone name index in the text array
    boneNameIndex = csrIQMGetTextIndex(pTexts, pJoint->m_Name);

    // found it?
    if (boneNameIndex >= pTexts->m_Count)
        return 0;

    // get the bone name
    pBoneName = pTexts->m_pTexts[boneNameIndex];

    // measure it
    nameLen = strlen(pBoneName);

    // allocate memory for destination bone name
    pBone->m_pName = (char*)malloc((nameLen + 1) * sizeof(char));

    if (!pBone->m_pName)
        return 0;

    // copy the bone name
    memcpy(pBone->m_pName, pBoneName, nameLen);
    pBone->m_pName[nameLen] = 0x0;

    // get the position
    position.m_X = pJoint->m_Translate[0];
    position.m_Y = pJoint->m_Translate[1];
    position.m_Z = pJoint->m_Translate[2];

    // get the rotation
    rotation.m_X = pJoint->m_Rotate[0];
    rotation.m_Y = pJoint->m_Rotate[1];
    rotation.m_Z = pJoint->m_Rotate[2];
    rotation.m_W = pJoint->m_Rotate[3];

    // get the scaling
    scaling.m_X = pJoint->m_Scale[0];
    scaling.m_Y = pJoint->m_Scale[1];
    scaling.m_Z = pJoint->m_Scale[2];

    // get the rotation quaternion and the scale and translate vectors
    csrMat4Scale      (&scaling,  &scaleMatrix);
    csrIQMQuatToRotMat(&rotation, &rotateMatrix);
    csrMat4Translate  (&position, &translateMatrix);

    // build the final matrix
    csrMat4Multiply(&scaleMatrix, &rotateMatrix,    &buildMatrix);
    csrMat4Multiply(&buildMatrix, &translateMatrix, &pBone->m_Matrix);

    // allocate memory for the joint index
    pBone->m_pCustomData = malloc(sizeof(size_t));

    // succeeded?
    if (!pBone->m_pCustomData)
        return 0;

    // link the mesh to the bone owning it
    *((size_t*)pBone->m_pCustomData) = jointIndex;

    return 1;
}
//---------------------------------------------------------------------------
CSR_Bone* csrIQMFindBone(CSR_Bone* pBone, size_t index)
{
    size_t    i;
    CSR_Bone* pChild;

    if (pBone->m_pCustomData && *((size_t*)pBone->m_pCustomData) == index)
        return pBone;

    for (i = 0; i < pBone->m_ChildrenCount; ++i)
    {
        pChild = csrIQMFindBone(&pBone->m_pChildren[i], index);

        if (pChild)
            return pChild;
    }

    return 0;
}
//---------------------------------------------------------------------------
void csrIQMBoneSetParent(CSR_Bone* pBone, CSR_Bone* pParent)
{
    size_t i;

    if (!pBone)
        return;

    pBone->m_pParent = pParent;

    for (i = 0; i < pBone->m_ChildrenCount; ++i)
        csrIQMBoneSetParent(&pBone->m_pChildren[i], pBone);
}
//---------------------------------------------------------------------------
int csrIQMPopulateSkeleton(const CSR_IQMTexts* pTexts, const CSR_IQMJoints* pJoints, int parentIndex, CSR_Bone* pRoot, size_t* pIndex)
{
    size_t    i;
    size_t    index;
    CSR_Bone* pParent;
    CSR_Bone* pChildren;

    // iterate through source joints
    for (i = 0; i < pJoints->m_Count; ++i)
        // is root?
        if (pJoints->m_pJoint[i].m_Parent >= 0)
        {
            // find the parent bone to link with
            pParent = csrIQMFindBone(pRoot, pJoints->m_pJoint[i].m_Parent);

            // found it?
            if (!pParent)
                return 0;

            index = pParent->m_ChildrenCount;

            // allocate memory for new child bone
            pChildren = (CSR_Bone*)csrMemoryAlloc(pParent->m_pChildren, sizeof(CSR_Bone), index + 1);

            if (!pChildren)
                return 0;

            // initialize the bone
            csrBoneInit(&pChildren[index]);

            // populate the newly added bone
            if (!csrIQMPopulateBone(pTexts, &pJoints->m_pJoint[i], i, &pChildren[index]))
                return 0;

            // set it in parent bone
            pParent->m_pChildren = pChildren;
            ++pParent->m_ChildrenCount;
        }
        else
        {
            index = pRoot->m_ChildrenCount;

            // allocate memory for new child bone
            pChildren = (CSR_Bone*)csrMemoryAlloc(pRoot->m_pChildren, sizeof(CSR_Bone), index + 1);

            if (!pChildren)
                return 0;

            // initialize the bone
            csrBoneInit(&pChildren[index]);

            // populate the newly added bone
            if (!csrIQMPopulateBone(pTexts, &pJoints->m_pJoint[i], i, &pChildren[index]))
                return 0;

            // set it in parent bone
            pRoot->m_pChildren = pChildren;
            ++pRoot->m_ChildrenCount;
        }

    // allocate parents after all the bones were created to avoid pointer corruptions
    csrIQMBoneSetParent(pRoot, 0);

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMPopulateAnims(const CSR_Buffer*            pBuffer,
                        const CSR_IQMHeader*         pHeader,
                        const CSR_IQMTexts*          pTexts,
                        const CSR_IQMAnims*          pAnims,
                        const CSR_IQMPoses*          pPoses,
                              CSR_Bone*              pRootBone,
                              CSR_AnimationSet_Bone* pAnimSet)
{
    size_t i;
    size_t j;
    size_t k;
    size_t offset;

    // set buffer offset to frame data start position
    offset = pHeader->m_FrameOffset;

    // iterate through animation sets to create
    for (i = 0; i < pAnims->m_Count; ++i)
    {
        const CSR_IQMAnim* pSrcAnim = &pAnims->m_pAnim[i];

        // iterate through bone animations to create
        for (j = 0; j < pSrcAnim->m_FrameCount; ++j)
            // iterate through frames to create
            for (k = 0; k < pPoses->m_Count; ++k)
            {
                unsigned short      value;
                size_t              len;
                #ifdef _MSC_VER
                    CSR_Quaternion  rotation = {0};
                    CSR_Vector3     position = {0};
                    CSR_Vector3     scaling  = {0};
                #else
                    CSR_Quaternion  rotation;
                    CSR_Vector3     position;
                    CSR_Vector3     scaling;
                #endif
                CSR_Matrix4         scaleMatrix;
                CSR_Matrix4         rotateMatrix;
                CSR_Matrix4         translateMatrix;
                CSR_Matrix4         buildMatrix;
                CSR_Matrix4         matrix;
                CSR_Animation_Bone* pDstAnim;

                // get next pose
                const CSR_IQMPose* pPose = &pPoses->m_pPose[k];

                // get destination animation to populate
                pDstAnim = &pAnimSet[i].m_pAnimation[k];

                // do initialize the bone animations?
                if (!j)
                {
                    size_t l;

                    // find the bone to link with animation
                    pDstAnim->m_pBone = csrIQMFindBone(pRootBone, k);

                    // found it?
                    if (!pDstAnim->m_pBone)
                        return 0;

                    // measure the animation bone name
                    len = strlen(pDstAnim->m_pBone->m_pName);

                    if (len)
                    {
                        // allocate memory for new bone name string
                        pDstAnim->m_pBoneName = (char*)malloc(len + 1);

                        // succeeded?
                        if (!pDstAnim->m_pBoneName)
                            return 0;

                        // copy the bone name
                        memcpy(pDstAnim->m_pBoneName, pDstAnim->m_pBone->m_pName, len);
                        pDstAnim->m_pBoneName[len] = 0x0;
                    }

                    // create the animation keys
                    pDstAnim->m_pKeys = (CSR_AnimationKeys*)malloc(sizeof(CSR_AnimationKeys));

                    // succeeded?
                    if (!pDstAnim->m_pKeys)
                        return 0;

                    // set the key count
                    pDstAnim->m_Count = 1;

                    // initialize the keys container
                    pDstAnim->m_pKeys->m_Type       = CSR_KT_Matrix;
                    pDstAnim->m_pKeys->m_Count      = 0;
                    pDstAnim->m_pKeys->m_ColOverRow = 0;
                    pDstAnim->m_pKeys->m_pKey       = (CSR_AnimationKey*)malloc(pSrcAnim->m_FrameCount * sizeof(CSR_AnimationKey));

                    // succeeded?
                    if (!pDstAnim->m_pKeys->m_pKey)
                        return 0;

                    // set the keys container count
                    pDstAnim->m_pKeys->m_Count = pSrcAnim->m_FrameCount;

                    // initialize the keys
                    for (l = 0; l < pDstAnim->m_pKeys->m_Count; ++l)
                    {
                        // initialize the key
                        pDstAnim->m_pKeys->m_pKey[l].m_Frame   = l;
                        pDstAnim->m_pKeys->m_pKey[l].m_Count   = 0;
                        pDstAnim->m_pKeys->m_pKey[l].m_pValues = (float*)malloc(16 * sizeof(float));

                        // succeeded?
                        if (!pDstAnim->m_pKeys->m_pKey[l].m_pValues)
                            return 0;

                        // set the matrix data count
                        pDstAnim->m_pKeys->m_pKey[l].m_Count = 16;
                    }
                }

                position.m_X = pPose->m_ChannelOffset[0];

                if (pPose->m_ChannelMask & 0x01)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    position.m_X += value * pPose->m_ChannelScale[0];
                }

                position.m_Y = pPose->m_ChannelOffset[1];

                if (pPose->m_ChannelMask & 0x02)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    position.m_Y += value * pPose->m_ChannelScale[1];
                }

                position.m_Z = pPose->m_ChannelOffset[2];

                if (pPose->m_ChannelMask & 0x04)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    position.m_Z += value * pPose->m_ChannelScale[2];
                }

                rotation.m_X = pPose->m_ChannelOffset[3];

                if (pPose->m_ChannelMask & 0x08)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    rotation.m_X += value * pPose->m_ChannelScale[3];
                }

                rotation.m_Y = pPose->m_ChannelOffset[4];

                if (pPose->m_ChannelMask & 0x10)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    rotation.m_Y += value * pPose->m_ChannelScale[4];
                }

                rotation.m_Z = pPose->m_ChannelOffset[5];

                if (pPose->m_ChannelMask & 0x20)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    rotation.m_Z += value * pPose->m_ChannelScale[5];
                }

                rotation.m_W = pPose->m_ChannelOffset[6];

                if (pPose->m_ChannelMask & 0x40)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    rotation.m_W += value * pPose->m_ChannelScale[6];
                }

                scaling.m_X = pPose->m_ChannelOffset[7];

                if (pPose->m_ChannelMask & 0x80)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    scaling.m_X += value * pPose->m_ChannelScale[7];
                }

                scaling.m_Y = pPose->m_ChannelOffset[8];

                if (pPose->m_ChannelMask & 0x100)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    scaling.m_Y += value * pPose->m_ChannelScale[8];
                }

                scaling.m_Z = pPose->m_ChannelOffset[9];

                if (pPose->m_ChannelMask & 0x200)
                {
                    // read the frame data
                    if (!csrBufferRead(pBuffer, &offset, 1, sizeof(unsigned short), &value))
                        return 0;

                    scaling.m_Z += value * pPose->m_ChannelScale[9];
                }

                // get the rotation quaternion and the scale and translate vectors
                csrMat4Scale      (&scaling,  &scaleMatrix);
                csrIQMQuatToRotMat(&rotation, &rotateMatrix);
                csrMat4Translate  (&position, &translateMatrix);

                // build the final matrix
                csrMat4Multiply(&scaleMatrix, &rotateMatrix,    &buildMatrix);
                csrMat4Multiply(&buildMatrix, &translateMatrix, &matrix);

                // set the matrix content in the animation key
                memcpy(pDstAnim->m_pKeys->m_pKey[j].m_pValues, &matrix.m_Table, 16 * sizeof(float));
            }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrIQMGetInverseBindMatrix(const CSR_Bone* pBone, CSR_Matrix4* pMatrix)
{
    CSR_Matrix4 localMatrix;
    CSR_Matrix4 matrix;
    float       determinant;

    // no bone?
    if (!pBone)
        return;

    // no output matrix to write to?
    if (!pMatrix)
        return;

    // set the output matrix as identity
    csrMat4Identity(&matrix);

    // iterate through bones
    while (pBone)
    {
        // get the previously stacked matrix as base to calculate the new one
        localMatrix = matrix;

        // stack the previously calculated matrix with the current bone one
        csrMat4Multiply(&localMatrix, &pBone->m_Matrix, &matrix);

        // go to parent bone
        pBone = pBone->m_pParent;
    }

    // get the inverse bind matrix
    csrMat4Inverse(&matrix, pMatrix, &determinant);
}
//---------------------------------------------------------------------------
int csrIQMBuildWeightsFromSkeleton(CSR_Bone*               pBone,
                                   size_t                  meshIndex,
                                   CSR_Skin_Weights_Group* pSkinWeightsGroup)
{
    size_t            i;
    size_t            index;
    size_t            len;
    CSR_Skin_Weights* pSkinWeights = 0;

    if (!pBone)
        return 0;

    if (!pSkinWeightsGroup)
        return 0;

    index = pSkinWeightsGroup->m_Count;

    // add a new skin weights in the array
    pSkinWeights = (CSR_Skin_Weights*)csrMemoryAlloc(pSkinWeightsGroup->m_pSkinWeights,
                                                     sizeof(CSR_Skin_Weights),
                                                     pSkinWeightsGroup->m_Count + 1);

    // succeeded?
    if (!pSkinWeights)
        return 0;

    // set new visual scene in the visual scenes
    pSkinWeightsGroup->m_pSkinWeights = pSkinWeights;
    ++pSkinWeightsGroup->m_Count;

    // initialize the skin weights
    csrSkinWeightsInit(&pSkinWeightsGroup->m_pSkinWeights[index]);

    // populate the bone infos
    pSkinWeightsGroup->m_pSkinWeights[index].m_pBone = pBone;

    if (!pBone->m_pName)
        return 0;

    // measure the bone name length and reserve memory to copy it
    len                                                  = strlen(pBone->m_pName);
    pSkinWeightsGroup->m_pSkinWeights[index].m_pBoneName = (char*)malloc((len + 1) * sizeof(char));

    if (!pSkinWeightsGroup->m_pSkinWeights[index].m_pBoneName)
        return 0;

    // copy the bone name
    memcpy(pSkinWeightsGroup->m_pSkinWeights[index].m_pBoneName, pBone->m_pName, len);
    pSkinWeightsGroup->m_pSkinWeights[index].m_pBoneName[len] = 0x0;

    // set the mesh index
    pSkinWeightsGroup->m_pSkinWeights[index].m_MeshIndex = meshIndex;

    // get the inverse bind matrix
    csrIQMGetInverseBindMatrix(pBone, &pSkinWeightsGroup->m_pSkinWeights[index].m_Matrix);

    // iterate through children bones to create
    for (i = 0; i < pBone->m_ChildrenCount; ++i)
        // build from next child bone
        if (!csrIQMBuildWeightsFromSkeleton(&pBone->m_pChildren[i],
                                             meshIndex,
                                             pSkinWeightsGroup))
            return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrIQMPopulateModel(const CSR_Buffer*           pBuffer,
                        const CSR_IQMHeader*        pHeader,
                        const CSR_IQMTexts*         pTexts,
                        const CSR_IQMMeshes*        pMeshes,
                        const CSR_IQMVertexArrays*  pVertexArrays,
                        const CSR_IQMTriangles*     pTriangles,
                        const CSR_IQMAdjacencies*   pAdjacencies,
                        const CSR_IQMJoints*        pJoints,
                        const CSR_IQMPoses*         pPoses,
                        const CSR_IQMAnims*         pAnims,
                        const CSR_IQMBoundsArray*   pBoundsArray,
                        const CSR_IQMComments*      pComments,
                        const CSR_IQMExtensions*    pExtensions,
                        const CSR_VertexFormat*     pVertFormat,
                        const CSR_VertexCulling*    pVertCulling,
                        const CSR_Material*         pMaterial,
                        const CSR_fOnGetVertexColor fOnGetVertexColor,
                        const CSR_fOnLoadTexture    fOnLoadTexture,
                        const CSR_fOnApplySkin      fOnApplySkin,
                        const CSR_fOnDeleteTexture  fOnDeleteTexture,
                              CSR_IQM*              pModel)
{
    size_t           i;
    size_t           j;
    size_t           k;
    size_t           l;
    size_t           m;
    CSR_IQMVertices* pSrcVertices;

    // do create mesh only?
    if (!pModel->m_MeshOnly)
    {
        size_t                 boneIndex;
        CSR_Bone*              pRootBone;
        CSR_AnimationSet_Bone* pAnimSet;

        // create the root bone
        pRootBone          = csrBoneCreate();
        pRootBone->m_pName = (char*)calloc(13, sizeof(char));
        memcpy(pRootBone->m_pName, "CSR_RootBone", 12);
        boneIndex          = 0;

        // populate the skeleton
        if (!csrIQMPopulateSkeleton(pTexts, pJoints, -1, pRootBone, &boneIndex))
        {
            csrBoneRelease(pRootBone, 0, 1);
            return 0;
        }

        // set the skeleton in the model
        pModel->m_pSkeleton = pRootBone;

        // model contains animations and should create them?
        if (pAnims->m_Count && pRootBone && !pModel->m_PoseOnly)
        {
            // create the animation sets
            pAnimSet = (CSR_AnimationSet_Bone*)malloc(pAnims->m_Count * sizeof(CSR_AnimationSet_Bone));

            // iterate through animation sets to initialize
            for (i = 0; i < pAnims->m_Count; ++i)
            {
                pAnimSet[i].m_Count = 0;

                // create bone animation
                pAnimSet[i].m_pAnimation = (CSR_Animation_Bone*)malloc(pJoints->m_Count * sizeof(CSR_Animation_Bone));

                // succeeded?
                if (!pAnimSet[i].m_pAnimation)
                {
                    // release the animation set content
                    for (i = 0; i < pAnims->m_Count; ++i)
                        csrBoneAnimSetRelease(&pAnimSet[i], 1);

                    // free the animation sets
                    free(pAnimSet);

                    return 0;
                }

                // set bone animation count
                pAnimSet[i].m_Count = pJoints->m_Count;

                // iterate through bone animations to initialize
                for (j = 0; j < pAnimSet[i].m_Count; ++j)
                {
                    pAnimSet[i].m_pAnimation[j].m_pBone     = 0;
                    pAnimSet[i].m_pAnimation[j].m_pBoneName = 0;
                    pAnimSet[i].m_pAnimation[j].m_pKeys     = 0;
                    pAnimSet[i].m_pAnimation[j].m_Count     = 0;
                }
            }

            // populate the animations
            if (!csrIQMPopulateAnims(pBuffer, pHeader, pTexts, pAnims, pPoses, pRootBone, pAnimSet))
            {
                // release the animation set content
                for (i = 0; i < pAnims->m_Count; ++i)
                    csrBoneAnimSetRelease(&pAnimSet[i], 1);

                // free the animation sets
                free(pAnimSet);

                return 0;
            }

            // set the animations in the model
            pModel->m_pAnimationSet     = pAnimSet;
            pModel->m_AnimationSetCount = pAnims->m_Count;
        }
    }

    // create a source vertices container
    pSrcVertices = (CSR_IQMVertices*)malloc(sizeof(CSR_IQMVertices));

    // succeeded?
    if (!pSrcVertices)
        return 0;

    // initialize it
    pSrcVertices->m_pVertex = 0;
    pSrcVertices->m_Count   = 0;

    // read the source vertices
    if (!csrIQMBuildSrcVertices(pHeader,
                                pVertexArrays,
                                pBuffer,
                                pSrcVertices))
    {
        free(pSrcVertices);
        return 0;
    }

    // create the meshes
    pModel->m_pMesh = (CSR_Mesh*)malloc(pMeshes->m_Count * sizeof(CSR_Mesh));

    // succeeded?
    if (!pModel->m_pMesh)
    {
        free(pSrcVertices->m_pVertex);
        free(pSrcVertices);
        return 0;
    }

    // iterate through the created meshes and initialize them
    for (i = 0; i < pMeshes->m_Count; ++i)
        csrMeshInit(&pModel->m_pMesh[i]);

    // set the mesh count
    pModel->m_MeshCount = pMeshes->m_Count;

    // do build the weights?
    if (!pModel->m_MeshOnly)
    {
        // create the mesh weights groups
        pModel->m_pMeshWeights = (CSR_Skin_Weights_Group*)malloc(pMeshes->m_Count * sizeof(CSR_Skin_Weights_Group));

        // succeeded?
        if (!pModel->m_pMeshWeights)
        {
            free(pSrcVertices->m_pVertex);
            free(pSrcVertices);
            return 0;
        }

        // iterate through the created weights groups and initialize them
        for (i = 0; i < pMeshes->m_Count; ++i)
        {
            pModel->m_pMeshWeights[i].m_pSkinWeights = 0;
            pModel->m_pMeshWeights[i].m_Count        = 0;
        }

        // set the weights groups count
        pModel->m_MeshWeightsCount = pMeshes->m_Count;
    }

    // iterate through the source meshes
    for (i = 0; i < pMeshes->m_Count; ++i)
    {
        int                     canRelease;
        CSR_Mesh*               pMesh;
        CSR_Skin_Weights_Group* pWeightsGroup = 0;

        // get the next mesh
        pMesh = &pModel->m_pMesh[i];

        // create the mesh vertex buffer
        pMesh->m_Count = 1;
        pMesh->m_pVB   = (CSR_VertexBuffer*)malloc(sizeof(CSR_VertexBuffer));

        // prepare the vertex buffer format
        csrVertexBufferInit(pMesh->m_pVB);

        // apply the user wished vertex format
        if (pVertFormat)
            pMesh->m_pVB->m_Format = *pVertFormat;
        else
        {
            // otherwise configure the default vertex format
            pMesh->m_pVB->m_Format.m_HasNormal    = 1;
            pMesh->m_pVB->m_Format.m_HasTexCoords = 1;
        }

        // apply the user wished vertex culling
        if (pVertCulling)
            pMesh->m_pVB->m_Culling = *pVertCulling;
        else
            // otherwise configure the default culling
            pMesh->m_pVB->m_Culling.m_Face = CSR_CF_CW;

        // apply the user wished material
        if (pMaterial)
            pMesh->m_pVB->m_Material = *pMaterial;

        // set the vertex format type
        pMesh->m_pVB->m_Format.m_Type = CSR_VT_Triangles;

        // calculate the vertex stride
        csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

        // create the vertex buffer
        pMesh->m_pVB->m_pData = (float*)malloc(pMesh->m_pVB->m_Format.m_Stride * sizeof(float));

        // do build the weights?
        if (!pModel->m_MeshOnly)
        {
            // get the next weights group
            pWeightsGroup = &pModel->m_pMeshWeights[i];

            // populate the skin weights group
            if (!csrIQMBuildWeightsFromSkeleton(pModel->m_pSkeleton, i, pWeightsGroup))
            {
                free(pSrcVertices->m_pVertex);
                free(pSrcVertices);
                return 0;
            }
        }

        // iterate through source mesh triangles
        for (j = 0; j < pMeshes->m_pMesh[i].m_TriangleCount; ++j)
        {
            // get triangle index
            const size_t index = pMeshes->m_pMesh[i].m_FirstTriangle + j;

            // iterate through vertices to create
            for (k = 0; k < 3; ++k)
            {
                // get vertex index
                const size_t vertIndex = pTriangles->m_pTriangle[index].m_Vertex[k];

                // add next vertex to vertex buffer
                csrVertexBufferAdd(&pSrcVertices->m_pVertex[vertIndex].m_Position,
                                   &pSrcVertices->m_pVertex[vertIndex].m_Normal,
                                   &pSrcVertices->m_pVertex[vertIndex].m_TexCoord,
                                    0,
                                    fOnGetVertexColor,
                                    pMesh->m_pVB);

                // do build the weights?
                if (!pModel->m_MeshOnly)
                {
                    // iterate through vertex weights
                    for (l = 0; l < 4 && pSrcVertices->m_pVertex[vertIndex].m_BlendWeights[l]; ++l)
                    {
                        size_t                       weightIndex;
                        CSR_Skin_Weights*            pWeights = 0;
                        CSR_Bone*                    pBone = 0;
                        float*                       pWeightsArray = 0;
                        CSR_Skin_Weight_Index_Table* pIndexTable = 0;

                        // get the bone matching with weight to animate
                        pBone = csrIQMFindBone(pModel->m_pSkeleton,
                                               pSrcVertices->m_pVertex[vertIndex].m_BlendIndices[l]);

                        // found it?
                        if (!pBone)
                        {
                            free(pSrcVertices->m_pVertex);
                            free(pSrcVertices);
                            return 0;
                        }

                        // get the skin weights to populate
                        for (m = 0; m < pWeightsGroup->m_Count; ++m)
                            if (strcmp(pWeightsGroup->m_pSkinWeights[m].m_pBoneName, pBone->m_pName) == 0)
                            {
                                pWeights = &pWeightsGroup->m_pSkinWeights[m];
                                break;
                            }

                        // found it?
                        if (!pWeights)
                        {
                            free(pSrcVertices->m_pVertex);
                            free(pSrcVertices);
                            return 0;
                        }

                        // number of weights should always equals the index table one
                        if (pWeights->m_WeightCount != pWeights->m_IndexTableCount)
                        {
                            free(pSrcVertices->m_pVertex);
                            free(pSrcVertices);
                            return 0;
                        }

                        weightIndex = pWeights->m_WeightCount;

                        // add a new weights in the array
                        pWeightsArray = (float*)csrMemoryAlloc(pWeights->m_pWeights,
                                                               sizeof(float),
                                                               pWeights->m_WeightCount + 1);

                        // succeeded?
                        if (!pWeightsArray)
                        {
                            free(pSrcVertices->m_pVertex);
                            free(pSrcVertices);
                            return 0;
                        }

                        // set new weights array in the skin weights
                        pWeights->m_pWeights = pWeightsArray;
                        ++pWeights->m_WeightCount;

                        // write the weight value
                        pWeights->m_pWeights[weightIndex] =
                                (float)pSrcVertices->m_pVertex[vertIndex].m_BlendWeights[l] / 255.0f;

                        // add a new index table in the array
                        pIndexTable =
                                (CSR_Skin_Weight_Index_Table*)csrMemoryAlloc(pWeights->m_pIndexTable,
                                                                             sizeof(CSR_Skin_Weight_Index_Table),
                                                                             pWeights->m_IndexTableCount + 1);

                        // succeeded?
                        if (!pIndexTable)
                        {
                            free(pSrcVertices->m_pVertex);
                            free(pSrcVertices);
                            return 0;
                        }

                        // set new index table in the skin weights
                        pWeights->m_pIndexTable = pIndexTable;
                        ++pWeights->m_IndexTableCount;

                        // initialize the skin weights table
                        pWeights->m_pIndexTable[weightIndex].m_Count = 0;
                        pWeights->m_pIndexTable[weightIndex].m_pData = (size_t*)malloc(sizeof(size_t));

                        // succeeded?
                        if (!pWeights->m_pIndexTable[weightIndex].m_pData)
                        {
                            free(pSrcVertices->m_pVertex);
                            free(pSrcVertices);
                            return 0;
                        }

                        // populate the index
                        *(pWeights->m_pIndexTable[weightIndex].m_pData) = (size_t)((j * 3) + k) * pModel->m_pMesh[i].m_pVB[0].m_Format.m_Stride;
                          pWeights->m_pIndexTable[weightIndex].m_Count  = 1;
                    }
                }
            }
        }

        // do load the texture?
        if (pMesh->m_pVB->m_Format.m_HasTexCoords && pMeshes->m_pMesh[i].m_Material)
        {
            const size_t materialIndex = csrIQMGetTextIndex(pTexts, pMeshes->m_pMesh[i].m_Material);

            // measure the file name length and allocate memory for file name in local mesh
            const size_t fileNameLen            = strlen(pTexts->m_pTexts[materialIndex]);
            pMesh->m_Skin.m_Texture.m_pFileName = (char*)calloc(fileNameLen + 1, sizeof(char));

            // copy the file name
            if (pMesh->m_Skin.m_Texture.m_pFileName)
                memcpy(pMesh->m_Skin.m_Texture.m_pFileName, pTexts->m_pTexts[materialIndex], fileNameLen);

            // load the texture
            if (fOnLoadTexture)
                pMesh->m_Skin.m_Texture.m_pBuffer = fOnLoadTexture(pTexts->m_pTexts[materialIndex]);

            canRelease = 0;

            // apply the skin
            if (fOnApplySkin)
                fOnApplySkin(0, &pMesh->m_Skin, &canRelease);

            // can release the texture buffer?
            if (canRelease)
            {
                csrPixelBufferRelease(pMesh->m_Skin.m_Texture.m_pBuffer);
                pMesh->m_Skin.m_Texture.m_pBuffer = 0;
            }
        }
    }

    // release the memory
    free(pSrcVertices->m_pVertex);
    free(pSrcVertices);

    return 1;
}
//---------------------------------------------------------------------------
void csrIQMReleaseObjects(CSR_IQMHeader*       pHeader,
                          CSR_IQMTexts*        pTexts,
                          CSR_IQMMeshes*       pMeshes,
                          CSR_IQMVertexArrays* pVertexArrays,
                          CSR_IQMTriangles*    pTriangles,
                          CSR_IQMAdjacencies*  pAdjacencies,
                          CSR_IQMJoints*       pJoints,
                          CSR_IQMPoses*        pPoses,
                          CSR_IQMAnims*        pAnims,
                          CSR_IQMBoundsArray*  pBoundsArray,
                          CSR_IQMComments*     pComments,
                          CSR_IQMExtensions*   pExtensions)
{
    // free the header
    if (pHeader)
        free(pHeader);

    // free the texts
    if (pTexts)
    {
        size_t i;

        if (pTexts->m_pTexts)
        {
            // free texts
            for (i = 0; i < pTexts->m_Count; ++i)
                free(pTexts->m_pTexts[i]);

            // free text arrays
            free(pTexts->m_pOffsets);
            free(pTexts->m_pTexts);
        }

        // free text container
        free(pTexts);
    }

    // free the meshes
    if (pMeshes)
    {
        // free the meshes
        if (pMeshes->m_pMesh)
            free(pMeshes->m_pMesh);

        // free meshes container
        free(pMeshes);
    }

    // free the vertex arrays
    if (pVertexArrays)
    {
        // free the meshes
        if (pVertexArrays->m_pVertexArray)
            free(pVertexArrays->m_pVertexArray);

        // free vertex arrays container
        free(pVertexArrays);
    }

    // free the triangles
    if (pTriangles)
    {
        // free the triangles
        if (pTriangles->m_pTriangle)
            free(pTriangles->m_pTriangle);

        // free triangles container
        free(pTriangles);
    }

    // free the adjacencies
    if (pAdjacencies)
    {
        // free the adjacencies
        if (pAdjacencies->m_pAdjacency)
            free(pAdjacencies->m_pAdjacency);

        // free adjacencies container
        free(pAdjacencies);
    }

    // free the joints
    if (pJoints)
    {
        // free the joints
        if (pJoints->m_pJoint)
            free(pJoints->m_pJoint);

        // free joints container
        free(pJoints);
    }

    // free the poses
    if (pPoses)
    {
        // free the poses
        if (pPoses->m_pPose)
            free(pPoses->m_pPose);

        // free poses container
        free(pPoses);
    }

    // free the animations
    if (pAnims)
    {
        // free the animations
        if (pAnims->m_pAnim)
            free(pAnims->m_pAnim);

        // free animations container
        free(pAnims);
    }

    // free the bounds array
    if (pBoundsArray)
    {
        // free the bounds
        if (pBoundsArray->m_pBounds)
            free(pBoundsArray->m_pBounds);

        // free bounds array
        free(pBoundsArray);
    }

    // free the comments
    if (pComments)
    {
        size_t i;

        if (pComments->m_pComments)
        {
            // free comments
            for (i = 0; i < pComments->m_Count; ++i)
                free(pComments->m_pComments[i]);

            // free comments arrays
            free(pComments->m_pOffsets);
            free(pComments->m_pComments);
        }

        // free comments container
        free(pComments);
    }

    // free the extensions
    if (pExtensions)
    {
        // free the extension
        if (pExtensions->m_pExtension)
            free(pExtensions->m_pExtension);

        // free extension container
        free(pExtensions);
    }
}
//---------------------------------------------------------------------------
// IQM model functions
//---------------------------------------------------------------------------
CSR_IQM* csrIQMCreate(const CSR_Buffer*           pBuffer,
                      const CSR_VertexFormat*     pVertFormat,
                      const CSR_VertexCulling*    pVertCulling,
                      const CSR_Material*         pMaterial,
                            int                   meshOnly,
                            int                   poseOnly,
                      const CSR_fOnGetVertexColor fOnGetVertexColor,
                      const CSR_fOnLoadTexture    fOnLoadTexture,
                      const CSR_fOnApplySkin      fOnApplySkin,
                      const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    CSR_IQMHeader*       pHeader;
    CSR_IQMTexts*        pTexts;
    CSR_IQMMeshes*       pMeshes;
    CSR_IQMVertexArrays* pVertexArrays;
    CSR_IQMTriangles*    pTriangles;
    CSR_IQMAdjacencies*  pAdjacencies;
    CSR_IQMJoints*       pJoints;
    CSR_IQMPoses*        pPoses;
    CSR_IQMAnims*        pAnims;
    CSR_IQMBoundsArray*  pBoundsArray;
    CSR_IQMComments*     pComments;
    CSR_IQMExtensions*   pExtensions;
    CSR_IQM*             pIQM;
    size_t               offset = 0;

    // no buffer to read from?
    if (!pBuffer)
        return 0;

    // create a IQM model
    pIQM = (CSR_IQM*)malloc(sizeof(CSR_IQM));

    // succeeded?
    if (!pIQM)
        return 0;

    // initialize it
    csrIQMInit(pIQM);

    // set the model flags
    pIQM->m_MeshOnly = meshOnly;
    pIQM->m_PoseOnly = poseOnly;

    // create iqm header
    pHeader = (CSR_IQMHeader*)malloc(sizeof(CSR_IQMHeader));

    // succeeded?
    if (!pHeader)
    {
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read file header
    csrIQMReadHeader(pBuffer, &offset, pHeader);

    // is iqm file and version correct?
    if (strcmp(pHeader->m_ID, M_IQM_ID) != 0 || (pHeader->m_Version != M_IQM_File_Version))
    {
        free(pHeader);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create iqm texts
    pTexts = (CSR_IQMTexts*)malloc(sizeof(CSR_IQMTexts));

    // succeeded?
    if (!pTexts)
    {
        csrIQMReleaseObjects(pHeader, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read texts
    if (!csrIQMReadTexts(pBuffer, &offset, pHeader, pTexts))
    {
        csrIQMReleaseObjects(pHeader, pTexts, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create mesh container
    pMeshes = (CSR_IQMMeshes*)malloc(sizeof(CSR_IQMMeshes));

    // succeeded?
    if (!pMeshes)
    {
        csrIQMReleaseObjects(pHeader, pTexts, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read meshes
    if (!csrIQMReadMeshes(pBuffer, &offset, pHeader, pMeshes))
    {
        csrIQMReleaseObjects(pHeader, pTexts, pMeshes, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create vertex array container
    pVertexArrays = (CSR_IQMVertexArrays*)malloc(sizeof(CSR_IQMVertexArrays));

    // succeeded?
    if (!pVertexArrays)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read vertex arrays
    if (!csrIQMReadVertexArrays(pBuffer, &offset, pHeader, pVertexArrays))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create triangles and adjacencies container
    pTriangles   = (CSR_IQMTriangles*)  malloc(sizeof(CSR_IQMTriangles));
    pAdjacencies = (CSR_IQMAdjacencies*)malloc(sizeof(CSR_IQMAdjacencies));

    // succeeded?
    if (!pTriangles || !pAdjacencies)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read triangles
    if (!csrIQMReadTriangles(pBuffer, &offset, pHeader, pTriangles, pAdjacencies))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create joints container
    pJoints = (CSR_IQMJoints*)malloc(sizeof(CSR_IQMJoints));

    // succeeded?
    if (!pJoints)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read joints
    if (!csrIQMReadJoints(pBuffer, &offset, pHeader, pJoints))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             0,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create poses container
    pPoses = (CSR_IQMPoses*)malloc(sizeof(CSR_IQMPoses));

    // succeeded?
    if (!pPoses)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             0,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read poses
    if (!csrIQMReadPoses(pBuffer, &offset, pHeader, pPoses))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create animations container
    pAnims = (CSR_IQMAnims*)malloc(sizeof(CSR_IQMAnims));

    // succeeded?
    if (!pAnims)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             0,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read animations
    if (!csrIQMReadAnims(pBuffer, &offset, pHeader, pAnims))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create bounds array container
    pBoundsArray = (CSR_IQMBoundsArray*)malloc(sizeof(CSR_IQMBoundsArray));

    // succeeded?
    if (!pBoundsArray)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             0,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read bounds array
    if (!csrIQMReadBounds(pBuffer, &offset, pHeader, pBoundsArray))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             pBoundsArray,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create iqm comments
    pComments = (CSR_IQMComments*)malloc(sizeof(CSR_IQMComments));

    // succeeded?
    if (!pComments)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             pBoundsArray,
                             0,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read comments
    if (!csrIQMReadComments(pBuffer, &offset, pHeader, pComments))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             pBoundsArray,
                             pComments,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // create extensions container
    pExtensions = (CSR_IQMExtensions*)malloc(sizeof(CSR_IQMExtensions));

    // succeeded?
    if (!pExtensions)
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             pBoundsArray,
                             pComments,
                             0);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // read extensions
    if (!csrIQMReadExtensions(pBuffer, &offset, pHeader, pExtensions))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             pBoundsArray,
                             pComments,
                             pExtensions);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // populate the model
    if (!csrIQMPopulateModel(pBuffer,
                             pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             pBoundsArray,
                             pComments,
                             pExtensions,
                             pVertFormat,
                             pVertCulling,
                             pMaterial,
                             fOnGetVertexColor,
                             fOnLoadTexture,
                             fOnApplySkin,
                             fOnDeleteTexture,
                             pIQM))
    {
        csrIQMReleaseObjects(pHeader,
                             pTexts,
                             pMeshes,
                             pVertexArrays,
                             pTriangles,
                             pAdjacencies,
                             pJoints,
                             pPoses,
                             pAnims,
                             pBoundsArray,
                             pComments,
                             pExtensions);
        csrIQMRelease(pIQM, fOnDeleteTexture);
        return 0;
    }

    // release objects
    csrIQMReleaseObjects(pHeader,
                         pTexts,
                         pMeshes,
                         pVertexArrays,
                         pTriangles,
                         pAdjacencies,
                         pJoints,
                         pPoses,
                         pAnims,
                         pBoundsArray,
                         pComments,
                         pExtensions);

    return pIQM;
}
//---------------------------------------------------------------------------
void csrIQMInit(CSR_IQM* pIQM)
{
    // no IQM model to initialize?
    if (!pIQM)
        return;

    // initialize the IQM model
    pIQM->m_pMesh               = 0;
    pIQM->m_MeshCount           = 0;
    pIQM->m_pMeshWeights        = 0;
    pIQM->m_MeshWeightsCount    = 0;
    pIQM->m_pMeshToBoneDict     = 0;
    pIQM->m_MeshToBoneDictCount = 0;
    pIQM->m_pSkeleton           = 0;
    pIQM->m_pAnimationSet       = 0;
    pIQM->m_AnimationSetCount   = 0;
    pIQM->m_MeshOnly            = 0;
    pIQM->m_PoseOnly            = 0;
}
//---------------------------------------------------------------------------
CSR_IQM* csrIQMOpen(const char*                 pFileName,
                    const CSR_VertexFormat*     pVertFormat,
                    const CSR_VertexCulling*    pVertCulling,
                    const CSR_Material*         pMaterial,
                          int                   meshOnly,
                          int                   poseOnly,
                    const CSR_fOnGetVertexColor fOnGetVertexColor,
                    const CSR_fOnLoadTexture    fOnLoadTexture,
                    const CSR_fOnApplySkin      fOnApplySkin,
                    const CSR_fOnDeleteTexture  fOnDeleteTexture)
{
    CSR_Buffer* pBuffer;
    CSR_IQM*    pIQM;

    // open the model file
    pBuffer = csrFileOpen(pFileName);

    // succeeded?
    if (!pBuffer || !pBuffer->m_Length)
    {
        csrBufferRelease(pBuffer);
        return 0;
    }

    // create the IQM model from the file content
    pIQM = csrIQMCreate(pBuffer,
                        pVertFormat,
                        pVertCulling,
                        pMaterial,
                        meshOnly,
                        poseOnly,
                        fOnGetVertexColor,
                        fOnLoadTexture,
                        fOnApplySkin,
                        fOnDeleteTexture);

    // release the file buffer (no longer required)
    csrBufferRelease(pBuffer);

    return pIQM;
}
//---------------------------------------------------------------------------
void csrIQMRelease(CSR_IQM* pIQM, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t i;
    size_t j;

    // no IQM model to release?
    if (!pIQM)
        return;

    // do free the meshes content?
    if (pIQM->m_pMesh)
    {
        // iterate through meshes to free
        for (i = 0; i < pIQM->m_MeshCount; ++i)
        {
            // delete the skin
            csrSkinContentRelease(&pIQM->m_pMesh[i].m_Skin, fOnDeleteTexture);

            // do free the mesh vertex buffer?
            if (pIQM->m_pMesh[i].m_pVB)
            {
                // free the mesh vertex buffer content
                for (j = 0; j < pIQM->m_pMesh[i].m_Count; ++j)
                    if (pIQM->m_pMesh[i].m_pVB[j].m_pData)
                        free(pIQM->m_pMesh[i].m_pVB[j].m_pData);

                // free the mesh vertex buffer
                free(pIQM->m_pMesh[i].m_pVB);
            }
        }

        // free the meshes
        free(pIQM->m_pMesh);
    }

    // release the weights
    if (pIQM->m_pMeshWeights)
    {
        // release the mesh weights content
        for (i = 0; i < pIQM->m_MeshWeightsCount; ++i)
        {
            // release the mesh skin weights content
            for (j = 0; j < pIQM->m_pMeshWeights[i].m_Count; ++j)
                csrSkinWeightsRelease(&pIQM->m_pMeshWeights[i].m_pSkinWeights[j], 1);

            // free the mesh skin weights
            free(pIQM->m_pMeshWeights[i].m_pSkinWeights);
        }

        // free the mesh weights
        free(pIQM->m_pMeshWeights);
    }

    // release the mesh-to-bone dictionary
    if (pIQM->m_pMeshToBoneDict)
        free(pIQM->m_pMeshToBoneDict);

    // release the bones
    csrBoneRelease(pIQM->m_pSkeleton, 0, 1);

    // release the animation sets
    if (pIQM->m_pAnimationSet)
    {
        // release the animation set content
        for (i = 0; i < pIQM->m_AnimationSetCount; ++i)
            csrBoneAnimSetRelease(&pIQM->m_pAnimationSet[i], 1);

        // free the animation sets
        free(pIQM->m_pAnimationSet);
    }

    // release the model
    free(pIQM);
}
//---------------------------------------------------------------------------
