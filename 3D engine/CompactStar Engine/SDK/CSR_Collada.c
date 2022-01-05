/****************************************************************************
 * ==> CSR_Collada ---------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a Collada (.dae) reader               *
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

#include "CSR_Collada.h"

// std
#include <stdlib.h>
#include <math.h>
#include <string.h>

// visual studio specific code
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
    #include <math.h>
#endif

// xml
#if defined(_OS_IOS_) || defined(_OS_ANDROID_) || defined(_OS_WINDOWS_)
    #include "../ThirdParty/sxml/sxmlc.h"
    #include "../ThirdParty/sxml/sxmlsearch.h"
#else
    #include "sxmlc.h"
    #include "sxmlsearch.h"
#endif

//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_Collada_Root_Tag                 "COLLADA"
#define M_Collada_Geometries_Tag           "library_geometries"
#define M_Collada_Geometry_Tag             "geometry"
#define M_Collada_Controllers_Tag          "library_controllers"
#define M_Collada_Controller_Tag           "controller"
#define M_Collada_Animations_Tag           "library_animations"
#define M_Collada_Animation_Tag            "animation"
#define M_Collada_Mesh_Tag                 "mesh"
#define M_Collada_Skin_Tag                 "skin"
#define M_Collada_Source_Tag               "source"
#define M_Collada_Sampler_Tag              "sampler"
#define M_Collada_Channel_Tag              "channel"
#define M_Collada_Float_Array_Tag          "float_array"
#define M_Collada_Name_Array_Tag           "Name_array"
#define M_Collada_Vertices_Tag             "vertices"
#define M_Collada_Triangles_Tag            "triangles"
#define M_Collada_Joints_Tag               "joints"
#define M_Collada_Vertex_Weights_Tag       "vertex_weights"
#define M_Collada_Technique_Tag            "technique"
#define M_Collada_Technique_Common_Tag     "technique_common"
#define M_Collada_Accessor_Tag             "accessor"
#define M_Collada_Param_Tag                "param"
#define M_Collada_Input_Tag                "input"
#define M_Collada_P_Tag                    "p"
#define M_Collada_V_Tag                    "v"
#define M_Collada_V_Count_Tag              "vcount"
#define M_Collada_Visual_Scenes_Tag        "library_visual_scenes"
#define M_Collada_Visual_Scene_Tag         "visual_scene"
#define M_Collada_Node_Tag                 "node"
#define M_Collada_Matrix_Tag               "matrix"
#define M_Collada_Bind_Shape_Matrix_Tag    "bind_shape_matrix"
#define M_Collada_Instance_Ctrl_Tag        "instance_controller"
#define M_Collada_Skeleton_Tag             "skeleton"
#define M_Collada_ID_Attribute             "id"
#define M_Collada_SID_Attribute            "sid"
#define M_Collada_Name_Attribute           "name"
#define M_Collada_Type_Attribute           "type"
#define M_Collada_Source_Attribute         "source"
#define M_Collada_Target_Attribute         "target"
#define M_Collada_Count_Attribute          "count"
#define M_Collada_Stride_Attribute         "stride"
#define M_Collada_Semantic_Attribute       "semantic"
#define M_Collada_Offset_Attribute         "offset"
#define M_Collada_Set_Attribute            "set"
#define M_Collada_Url_Attribute            "url"
#define M_Collada_Semantic_Position        "POSITION"
#define M_Collada_Semantic_Vertex          "VERTEX"
#define M_Collada_Semantic_Normal          "NORMAL"
#define M_Collada_Semantic_TexCoord        "TEXCOORD"
#define M_Collada_Semantic_Joint           "JOINT"
#define M_Collada_Semantic_Weight          "WEIGHT"
#define M_Collada_Semantic_Transform       "TRANSFORM"
#define M_Collada_Semantic_Inv_Bind_Matrix "INV_BIND_MATRIX"
#define M_Collada_Semantic_Input           "INPUT"
#define M_Collada_Semantic_Output          "OUTPUT"
#define M_Collada_Semantic_Interpolation   "INTERPOLATION"
//---------------------------------------------------------------------------
// Collada private structures
//---------------------------------------------------------------------------

/**
* Collada (.dae) param tag
*/
typedef struct
{
    char* m_pName; // param name
    char* m_pType; // param type
} CSR_Collada_Param;

/**
* Collada (.dae) matrix
*/
typedef struct
{
    char*       m_pSid;   // animation target identifier
    CSR_Matrix4 m_Matrix; // matrix
} CSR_Collada_Matrix;

/**
* Collada (.dae) float array
*/
typedef struct
{
    char*  m_pId;   // array identifier
    float* m_pData; // float array data
    size_t m_Count; // array count
} CSR_Collada_Float_Array;

/**
* Collada (.dae) integer array
*/
typedef struct
{
    int*   m_pData; // integer array data
    size_t m_Count; // array count
} CSR_Collada_Int_Array;

/**
* Collada (.dae) unsigned array
*/
typedef struct
{
    size_t* m_pData; // unsigned array data
    size_t  m_Count; // array count
} CSR_Collada_Unsigned_Array;

/**
* Collada (.dae) name array
*/
typedef struct
{
    char*   m_pId;   // array identifier
    char**  m_pData; // name array data
    size_t  m_Count; // array count
} CSR_Collada_Name_Array;

/**
* Collada (.dae) accessor tag
*/
typedef struct
{
    char*              m_pSource;    // source identifier at which this accessor belongs
    size_t             m_Count;      // accessor array count
    size_t             m_Stride;     // accessor array stride
    CSR_Collada_Param* m_pParams;    // accessor parameters
    size_t             m_ParamCount; // accessor parameter count
} CSR_Collada_Accessor;

/**
* Collada (.dae) technique common tag
*/
typedef struct
{
    CSR_Collada_Accessor* m_pAccessor; // accessor
} CSR_Collada_Technique_Common;

/**
* Collada (.dae) source
*/
typedef struct
{
    char*                         m_pId;         // array identifier
    CSR_Collada_Float_Array*      m_pFloatArray; // float array
    CSR_Collada_Name_Array*       m_pNameArray;  // name array
    CSR_Collada_Technique_Common* m_pTechCommon; // technique common
} CSR_Collada_Source;

/**
* Collada (.dae) input
*/
typedef struct
{
    char*  m_pSemantic;
    char*  m_pSource;
    size_t m_Offset;
    size_t m_Set;
} CSR_Collada_Input;

/**
* Collada (.dae) vertices
*/
typedef struct
{
    char*              m_pId;        // vertices identifier
    CSR_Collada_Input* m_pInputs;    // input array
    size_t             m_InputCount; // input count
} CSR_Collada_Vertices;

/**
* Collada (.dae) triangles
*/
typedef struct
{
    size_t                      m_Count;           // triangle count
    CSR_Collada_Input*          m_pInputs;         // input array
    size_t                      m_InputCount;      // input count
    CSR_Collada_Unsigned_Array* m_pPrimitiveArray; // primitive array
} CSR_Collada_Triangles;

/**
* Collada (.dae) mesh
*/
typedef struct
{
    CSR_Collada_Source*    m_pSources;      // sources
    size_t                 m_SourceCount;   // source count
    CSR_Collada_Vertices*  m_pVertices;     // vertices
    size_t                 m_VertexCount;   // vertex count
    CSR_Collada_Triangles* m_pTriangles;    // triangles
    size_t                 m_TriangleCount; // triangle count
} CSR_Collada_Mesh;

/**
* Collada (.dae) geometry
*/
typedef struct
{
    char*             m_pId;   // geometry identifier
    char*             m_pName; // geometry name
    CSR_Collada_Mesh* m_pMesh; // mesh
} CSR_Collada_Geometry;

/**
* Collada (.dae) geometries
*/
typedef struct
{
    CSR_Collada_Geometry* m_pGeometries;   // geometries
    size_t                m_GeometryCount; // geometry count
} CSR_Collada_Geometries;

/**
* Collada (.dae) joints
*/
typedef struct
{
    CSR_Collada_Input* m_pInputs;    // inputs
    size_t             m_InputCount; // input count
} CSR_Collada_Joints;

/**
* Collada (.dae) vertex weights
*/
typedef struct
{
    size_t                      m_Count;                   // vertex weights count
    CSR_Collada_Input*          m_pInputs;                 // inputs
    size_t                      m_InputCount;              // input count
    CSR_Collada_Unsigned_Array* m_pVertexToBoneArray;      // vertex to bone array
    CSR_Collada_Unsigned_Array* m_pVertexToBoneCountArray; // vertex to bone count array
} CSR_Collada_Vertex_Weights;

/**
* Collada (.dae) skin
*/
typedef struct
{
    char*                       m_pSource;          // source name
    CSR_Collada_Matrix*         m_pBindShapeMatrix; // bind shape matrix
    CSR_Collada_Source*         m_pSources;         // sources
    size_t                      m_SourceCount;      // source count
    CSR_Collada_Joints*         m_pJoints;          // joints
    CSR_Collada_Vertex_Weights* m_pVertexWeights;   // vertex weights
} CSR_Collada_Skin;

/**
* Collada (.dae) controller
*/
typedef struct
{
    char*             m_pId;   // controller identifier
    char*             m_pName; // controller name
    CSR_Collada_Skin* m_pSkin; // skin
} CSR_Collada_Controller;

/**
* Collada (.dae) controllers
*/
typedef struct
{
    CSR_Collada_Controller* m_pControllers;    // controllers
    size_t                  m_ControllerCount; // controller count
} CSR_Collada_Controllers;

/**
* Collada (.dae) sampler
*/
typedef struct
{
    char*              m_pId;        // controller identifier
    CSR_Collada_Input* m_pInputs;    // inputs
    size_t             m_InputCount; // input count
} CSR_Collada_Sampler;

/**
* Collada (.dae) channel
*/
typedef struct
{
    char* m_pSource; // source
    char* m_pTarget; // target
} CSR_Collada_Channel;

/**
* Collada (.dae) animation
*/
typedef struct CSR_tagCollada_Animation
{
    char*                            m_pId;            // controller identifier
    char*                            m_pName;          // controller name
    struct CSR_tagCollada_Animation* m_pAnimations;    // animations
    size_t                           m_AnimationCount; // animation count
    CSR_Collada_Source*              m_pSources;       // sources
    size_t                           m_SourceCount;    // source count
    CSR_Collada_Sampler*             m_pSamplers;      // samplers
    size_t                           m_SamplerCount;   // sampler count
    CSR_Collada_Channel*             m_pChannels;      // channels
    size_t                           m_ChannelCount;   // channel count
} CSR_Collada_Animation;

/**
* Collada (.dae) animations
*/
typedef struct
{
    CSR_Collada_Animation* m_pAnimations;    // animations
    size_t                 m_AnimationCount; // animation count
} CSR_Collada_Animations;

/**
* Collada (.dae) skeleton
*/
typedef struct
{
    char* m_pId; // skeleton root node link
} CSR_Collada_Skeleton;

/**
* Collada (.dae) instance controller
*/
typedef struct
{
    char*                 m_pUrl;          // animation target identifier
    CSR_Collada_Skeleton* m_pSkeletons;    // skeleton array
    size_t                m_SkeletonCount; // skeleton count
} CSR_Collada_Instance_Controller;

/**
* Collada (.dae) node
*/
typedef struct CSR_tagCollada_Node
{
    char*                            m_pName;             // node name
    char*                            m_pId;               // node identifier
    char*                            m_pSid;              // animation target identifier
    char*                            m_pType;             // node type
    CSR_Collada_Matrix*              m_pMatrices;         // bone matrices array
    size_t                           m_MatrixCount;       // matrix count
    CSR_Collada_Instance_Controller* m_pInstanceCtrls;    // instance controller array
    size_t                           m_InstanceCtrlCount; // instance controller count
    struct CSR_tagCollada_Node*      m_pParent;           // parent node
    struct CSR_tagCollada_Node*      m_pNodes;            // children nodes
    size_t                           m_NodeCount;         // child node count
} CSR_Collada_Node;

/**
* Collada (.dae) visual scene
*/
typedef struct
{
    char*             m_pId;       // identifier
    char*             m_pName;     // name
    CSR_Collada_Node* m_pNodes;    // children nodes
    size_t            m_NodeCount; // child node count
} CSR_Collada_Visual_Scene;

/**
* Collada (.dae) visual scenes
*/
typedef struct
{
    CSR_Collada_Visual_Scene* m_pVisualScenes;    // visual scenes
    size_t                    m_VisualSceneCount; // visual scene count
} CSR_Collada_Visual_Scenes;

//---------------------------------------------------------------------------
// Collada private functions
//---------------------------------------------------------------------------
int csrColladaReadAttribValue(XMLAttribute* pAttribute, char** pText)
{
    if (!pAttribute)
        return 0;

    if (!pText)
        return 0;

    // found name, read it
    const size_t valueLen = strlen(pAttribute->value);

    // allocate memory for new mesh id
    *pText = (char*)malloc((valueLen + 1) * sizeof(char));

    if (!*pText)
        return 0;

    // copy the identifier from mesh node attribute
    memcpy(*pText, pAttribute->value, valueLen);
    (*pText)[valueLen] = 0x0;

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaParamInit(CSR_Collada_Param* pColladaParam)
{
    if (!pColladaParam)
        return;

    pColladaParam->m_pName = 0;
    pColladaParam->m_pType = 0;
}
//---------------------------------------------------------------------------
void csrColladaParamRelease(CSR_Collada_Param* pColladaParam)
{
    if (!pColladaParam)
        return;

    // free the name
    if (pColladaParam->m_pName)
        free(pColladaParam->m_pName);

    // free the type
    if (pColladaParam->m_pType)
        free(pColladaParam->m_pType);
}
//---------------------------------------------------------------------------
int csrColladaParamRead(XMLNode* pNode, CSR_Collada_Param* pColladaParam)
{
    size_t        i;
    size_t        len;
    #ifdef _MSC_VER
        size_t    index = 0;
    #else
        size_t    index;
    #endif
    XMLAttribute* pAttributes;

    if (!pNode)
        return 0;

    if (!pColladaParam)
        return 0;

    // initialize parameters info
    csrColladaParamInit(pColladaParam);

    // node contains attributes?
    if (!pNode->n_attributes)
        return 1;

    // get attribute list
    pAttributes = pNode->attributes;

    // found it?
    if (!pAttributes)
        return 0;

    // iterate through attributes
    for (i = 0; i < (size_t)pNode->n_attributes; ++i)
    {
        // measure attribute name length
        len = strlen(pAttributes[i].name);

        // search for next attribute to read
        if (len == strlen(M_Collada_Name_Attribute) &&
            memcmp(pAttributes[i].name, M_Collada_Name_Attribute, len) == 0)
        {
            // found name, read it
            if (!csrColladaReadAttribValue(&pAttributes[i], &pColladaParam->m_pName))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Type_Attribute) &&
            memcmp(pAttributes[i].name, M_Collada_Type_Attribute, len) == 0)
            // found type, read it
            if (!csrColladaReadAttribValue(&pAttributes[i], &pColladaParam->m_pType))
                return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaMatrixInit(CSR_Collada_Matrix* pColladaMatrix)
{
    if (!pColladaMatrix)
        return;

    pColladaMatrix->m_pSid = 0;

    csrMat4Identity(&pColladaMatrix->m_Matrix);
}
//---------------------------------------------------------------------------
void csrColladaMatrixRelease(CSR_Collada_Matrix* pColladaMatrix)
{
    if (!pColladaMatrix)
        return;

    // free the animation target identifier
    if (pColladaMatrix->m_pSid)
        free(pColladaMatrix->m_pSid);
}
//---------------------------------------------------------------------------
int csrColladaMatrixRead(XMLNode* pNode, CSR_Collada_Matrix* pColladaMatrix)
{
    size_t i;
    size_t len;
    size_t lineLen;
    size_t offset;
    size_t index;
    char*  pNumber;

    if (!pNode)
        return 0;

    if (!pColladaMatrix)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_SID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_SID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaMatrix->m_pSid);
        }
    }

    offset = 0;
    index  = 0;

    // reserve memory to copy the numbers to convert. Assume 64, because
    // no number should be longer than 64 digits
    pNumber = (char*)malloc(64 * sizeof(char));

    // succeeded?
    if (!pNumber)
        return 0;

    // measure the source line length
    lineLen = strlen(pNode->text);

    // iterate through source array chars
    for (i = 0; i < lineLen; ++i)
    {
        // found a separator?
        if (pNode->text[i] != ' ')
            continue;

        // to prevent that bad things happens...
        if (index >= 16)
        {
            free(pNumber);
            return 0;
        }

        // calculate next number length
        len = (i - offset);

        // 64 digits max
        if (len >= 64)
        {
            free(pNumber);
            return 0;
        }

        // read the next number to convert
        memcpy(pNumber, &pNode->text[offset], len);
        pNumber[len] = 0x0;

        // convert it and write it in the array
        pColladaMatrix->m_Matrix.m_Table[index % 4][index / 4] = (float)atof(pNumber);

        // start to read the next number
        ++index;
        offset = i + 1;
    }

    // calculate last number length
    len = (strlen(pNode->text) - offset);

    // sometimes the last string part contains only spaces and no numbers
    if (!len)
    {
        free(pNumber);
        return 1;
    }

    // 64 digits max
    if (len >= 64)
    {
        free(pNumber);
        return 0;
    }

    // to prevent that bad things happens...
    if (index >= 16)
    {
        free(pNumber);
        return 0;
    }

    // read the last number to convert
    memcpy(pNumber, &pNode->text[offset], len);
    pNumber[len] = 0x0;

    // convert it and write it in the array
    pColladaMatrix->m_Matrix.m_Table[index % 4][index / 4] = (float)atof(pNumber);

    free(pNumber);

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaFloatArrayInit(CSR_Collada_Float_Array* pColladaFloatArray)
{
    if (!pColladaFloatArray)
        return;

    pColladaFloatArray->m_pId   = 0;
    pColladaFloatArray->m_pData = 0;
    pColladaFloatArray->m_Count = 0;
}
//---------------------------------------------------------------------------
void csrColladaFloatArrayRelease(CSR_Collada_Float_Array* pColladaFloatArray)
{
    if (!pColladaFloatArray)
        return;

    // free the identifier
    if (pColladaFloatArray->m_pId)
        free(pColladaFloatArray->m_pId);

    // free the array
    if (pColladaFloatArray->m_pData)
        free(pColladaFloatArray->m_pData);
}
//---------------------------------------------------------------------------
int csrColladaFloatArrayRead(XMLNode* pNode, CSR_Collada_Float_Array* pColladaFloatArray)
{
    size_t i;
    size_t len;
    size_t lineLen;
    size_t offset  = 0;
    size_t index   = 0;
    char*  pNumber = 0;

    if (!pNode)
        return 0;

    if (!pColladaFloatArray)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaFloatArray->m_pId);
            else
            if (len == strlen(M_Collada_Count_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Count_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read count value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert count to value
                pColladaFloatArray->m_Count = atoi(pCount);

                free(pCount);
            }
        }
    }

    // only one array is allowed, if already exists it's an error
    if (pColladaFloatArray->m_pData)
        return 0;

    if (!pColladaFloatArray->m_Count)
        return 0;

    // allocate memory for the float array
    pColladaFloatArray->m_pData = (float*)malloc(pColladaFloatArray->m_Count * sizeof(float));

    // succeeded?
    if (!pColladaFloatArray->m_pData)
        return 0;

    // reserve memory to copy the numbers to convert. Assume 64, because
    // no number should be longer than 64 digits
    pNumber = (char*)malloc(64 * sizeof(char));

    // succeeded?
    if (!pNumber)
        return 0;

    // measure the source line length
    lineLen = strlen(pNode->text);

    // iterate through source array chars
    for (i = 0; i < lineLen; ++i)
    {
        // found a separator?
        if (pNode->text[i] != ' ')
            continue;

        // to prevent that bad things happens...
        if (index >= pColladaFloatArray->m_Count)
        {
            free(pNumber);
            return 0;
        }

        // calculate next number length
        len = (i - offset);

        // 64 digits max
        if (len >= 64)
        {
            free(pNumber);
            return 0;
        }

        // read the next number to convert
        memcpy(pNumber, &pNode->text[offset], len);
        pNumber[len] = 0x0;

        // convert it and write it in the array
        pColladaFloatArray->m_pData[index] = (float)atof(pNumber);

        // start to read the next number
        ++index;
        offset = i + 1;
    }

    // calculate last number length, and allocate memory to read it
    len = (strlen(pNode->text) - offset);

    // sometimes the last string part contains only spaces and no numbers
    if (!len)
    {
        free(pNumber);
        return 1;
    }

    // 64 digits max
    if (len >= 64)
    {
        free(pNumber);
        return 0;
    }

    // to prevent that bad things happens...
    if (index >= pColladaFloatArray->m_Count)
    {
        free(pNumber);
        return 0;
    }

    // read the last number to convert
    memcpy(pNumber, &pNode->text[offset], len);
    pNumber[len] = 0x0;

    // convert it and write it in the array
    pColladaFloatArray->m_pData[index] = (float)atof(pNumber);

    free(pNumber);

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaIntArrayInit(CSR_Collada_Int_Array* pColladaIntArray)
{
    if (!pColladaIntArray)
        return;

    pColladaIntArray->m_pData = 0;
    pColladaIntArray->m_Count = 0;
}
//---------------------------------------------------------------------------
void csrColladaIntArrayRelease(CSR_Collada_Int_Array* pColladaIntArray)
{
    if (!pColladaIntArray)
        return;

    // free the array
    if (pColladaIntArray->m_pData)
        free(pColladaIntArray->m_pData);
}
//---------------------------------------------------------------------------
int csrColladaIntArrayRead(XMLNode* pNode, CSR_Collada_Int_Array* pColladaIntArray, size_t count)
{
    size_t i;
    size_t len;
    size_t lineLen;
    size_t offset  = 0;
    size_t index   = 0;
    char*  pNumber = 0;

    if (!pNode)
        return 0;

    if (!pColladaIntArray)
        return 0;

    if (!count)
        return 0;

    // only one array is allowed, if already exists it's an error
    if (pColladaIntArray->m_pData)
        return 0;

    // set (and trust) the array count
    pColladaIntArray->m_Count = count;

    // allocate memory for the int array
    pColladaIntArray->m_pData = (int*)malloc(pColladaIntArray->m_Count * sizeof(int));

    // succeeded?
    if (!pColladaIntArray->m_pData)
        return 0;

    // reserve memory to copy the numbers to convert. Assume 64, because
    // no number should be longer than 64 digits
    pNumber = (char*)malloc(64 * sizeof(char));

    // succeeded?
    if (!pNumber)
        return 0;

    // measure the source line length
    lineLen = strlen(pNode->text);

    // iterate through source array chars
    for (i = 0; i < lineLen; ++i)
    {
        // found a separator?
        if (pNode->text[i] != ' ')
            continue;

        // to prevent that bad things happens...
        if (index >= count)
        {
            free(pNumber);
            return 0;
        }

        // calculate next number length, and allocate memory to read it
        len = (i - offset);

        // 64 digits max
        if (len >= 64)
        {
            free(pNumber);
            return 0;
        }

        // read the next number to convert
        memcpy(pNumber, &pNode->text[offset], len);
        pNumber[len] = 0x0;

        // convert it and write it in the array
        pColladaIntArray->m_pData[index] = atoi(pNumber);

        // start to read the next number
        ++index;
        offset = i + 1;
    }

    // calculate last number length, and allocate memory to read it
    len = (strlen(pNode->text) - offset);

    // sometimes the last string part contains only spaces and no numbers
    if (!len)
    {
        free(pNumber);
        return 1;
    }

    // 64 digits max
    if (len >= 64)
    {
        free(pNumber);
        return 0;
    }

    // to prevent that bad things happens...
    if (index >= count)
    {
        free(pNumber);
        return 0;
    }

    // read the last number to convert
    memcpy(pNumber, &pNode->text[offset], len);
    pNumber[len] = 0x0;

    // convert it and write it in the array
    pColladaIntArray->m_pData[index] = atoi(pNumber);

    free(pNumber);

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaUnsignedArrayInit(CSR_Collada_Unsigned_Array* pColladaUnsignedArray)
{
    if (!pColladaUnsignedArray)
        return;

    pColladaUnsignedArray->m_pData = 0;
    pColladaUnsignedArray->m_Count = 0;
}
//---------------------------------------------------------------------------
void csrColladaUnsignedArrayRelease(CSR_Collada_Unsigned_Array* pColladaUnsignedArray)
{
    if (!pColladaUnsignedArray)
        return;

    // free the array
    if (pColladaUnsignedArray->m_pData)
        free(pColladaUnsignedArray->m_pData);
}
//---------------------------------------------------------------------------
int csrColladaUnsignedArrayRead(XMLNode* pNode, CSR_Collada_Unsigned_Array* pColladaUnsignedArray, size_t count)
{
    size_t i;
    size_t len;
    size_t lineLen;
    size_t offset  = 0;
    size_t index   = 0;
    char*  pNumber = 0;

    if (!pNode)
        return 0;

    if (!pColladaUnsignedArray)
        return 0;

    if (!count)
        return 0;

    // only one array is allowed, if already exists it's an error
    if (pColladaUnsignedArray->m_pData)
        return 0;

    // set (and trust) the array count
    pColladaUnsignedArray->m_Count = count;

    // allocate memory for the unsigned array
    pColladaUnsignedArray->m_pData = (size_t*)malloc(pColladaUnsignedArray->m_Count * sizeof(size_t));

    // succeeded?
    if (!pColladaUnsignedArray->m_pData)
        return 0;

    // reserve memory to copy the numbers to convert. Assume 64, because
    // no number should be longer than 64 digits
    pNumber = (char*)malloc(64 * sizeof(char));

    // succeeded?
    if (!pNumber)
        return 0;

    // measure the source line length
    lineLen = strlen(pNode->text);

    // iterate through source array chars
    for (i = 0; i < lineLen; ++i)
    {
        // found a separator?
        if (pNode->text[i] != ' ')
            continue;

        // to prevent that bad things happens...
        if (index >= count)
        {
            free(pNumber);
            return 0;
        }

        // calculate next number length, and allocate memory to read it
        len = (i - offset);

        // 64 digits max
        if (len >= 64)
        {
            free(pNumber);
            return 0;
        }

        // read the next number to convert
        memcpy(pNumber, &pNode->text[offset], len);
        pNumber[len] = 0x0;

        // todo -cFeature -oJean: Not a very good conversion, but it's the
        //                        best one I found which respect all target
        //                        platforms. If a better function exists,
        //                        use it instead
        // convert it and write it in the array
        pColladaUnsignedArray->m_pData[index] = (size_t)atoll(pNumber);

        // start to read the next number
        ++index;
        offset = i + 1;
    }

    // calculate last number length, and allocate memory to read it
    len = (strlen(pNode->text) - offset);

    // sometimes the last string part contains only spaces and no numbers
    if (!len)
    {
        free(pNumber);
        return 1;
    }

    // 64 digits max
    if (len >= 64)
    {
        free(pNumber);
        return 0;
    }

    // to prevent that bad things happens...
    if (index >= count)
    {
        free(pNumber);
        return 0;
    }

    // read the last number to convert
    memcpy(pNumber, &pNode->text[offset], len);
    pNumber[len] = 0x0;

    // todo -cFeature -oJean: Not a very good conversion, but it's the
    //                        best one I found which respect all target
    //                        platforms. If a better function exists,
    //                        use it instead
    // convert it and write it in the array
    pColladaUnsignedArray->m_pData[index] = (size_t)atoll(pNumber);

    free(pNumber);

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaNameArrayInit(CSR_Collada_Name_Array* pColladaNameArray)
{
    if (!pColladaNameArray)
        return;

    pColladaNameArray->m_pId   = 0;
    pColladaNameArray->m_pData = 0;
    pColladaNameArray->m_Count = 0;
}
//---------------------------------------------------------------------------
void csrColladaNameArrayRelease(CSR_Collada_Name_Array* pColladaNameArray)
{
    if (!pColladaNameArray)
        return;

    // free the identifier
    if (pColladaNameArray->m_pId)
        free(pColladaNameArray->m_pId);

    // free the array
    if (pColladaNameArray->m_pData)
    {
        size_t i;

        // free names
        for (i = 0; i < pColladaNameArray->m_Count; ++i)
            free(pColladaNameArray->m_pData[i]);

        // free name container
        free(pColladaNameArray->m_pData);
    }
}
//---------------------------------------------------------------------------
int csrColladaNameArrayRead(XMLNode* pNode, CSR_Collada_Name_Array* pColladaNameArray)
{
    size_t i;
    size_t len;
    size_t lineLen;
    size_t offset  = 0;
    size_t index   = 0;
    char*  pNumber = 0;

    if (!pNode)
        return 0;

    if (!pColladaNameArray)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaNameArray->m_pId);
            else
            if (len == strlen(M_Collada_Count_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Count_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read count value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert count to value
                pColladaNameArray->m_Count = atoi(pCount);

                free(pCount);
            }
        }
    }

    // only one array is allowed, if already exists it's an error
    if (pColladaNameArray->m_pData)
        return 0;

    if (!pColladaNameArray->m_Count)
        return 0;

    // allocate memory for the name array
    pColladaNameArray->m_pData = (char**)malloc(pColladaNameArray->m_Count * sizeof(char*));

    // succeeded?
    if (!pColladaNameArray->m_pData)
        return 0;

    // measure the source line length
    lineLen = strlen(pNode->text);

    // iterate through source array chars
    for (i = 0; i < lineLen; ++i)
    {
        // found a separator?
        if (pNode->text[i] != ' ')
            continue;

        // to prevent that bad things happens...
        if (index >= pColladaNameArray->m_Count)
            return 0;

        // calculate next number length, and allocate memory to read it
        len = (i - offset);

        if (!len)
            return 0;

        // allocate memory for name
        pColladaNameArray->m_pData[index] = (char*)malloc((len + 1) * sizeof(char));

        if (!pColladaNameArray->m_pData[index])
            return 0;

        // read the name
        memcpy(pColladaNameArray->m_pData[index], &pNode->text[offset], len);
        pColladaNameArray->m_pData[index][len] = 0x0;

        // start to read the next number
        ++index;
        offset = i + 1;
    }

    // calculate last number length, and allocate memory to read it
    len = (strlen(pNode->text) - offset);

    // sometimes the last string part contains only spaces and no numbers
    if (!len)
        return 1;

    // to prevent that bad things happens...
    if (index >= pColladaNameArray->m_Count)
        return 0;

    if (!len)
        return 0;

    // allocate memory for name
    pColladaNameArray->m_pData[index] = (char*)malloc((len + 1) * sizeof(char));

    if (!pColladaNameArray->m_pData[index])
        return 0;

    // read the name
    memcpy(pColladaNameArray->m_pData[index], &pNode->text[offset], len);
    pColladaNameArray->m_pData[index][len] = 0x0;

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaAccessorInit(CSR_Collada_Accessor* pColladaAccessor)
{
    if (!pColladaAccessor)
        return;

    pColladaAccessor->m_pSource    = 0;
    pColladaAccessor->m_Count      = 0;
    pColladaAccessor->m_Stride     = 0;
    pColladaAccessor->m_pParams    = 0;
    pColladaAccessor->m_ParamCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaAccessorRelease(CSR_Collada_Accessor* pColladaAccessor)
{
    if (!pColladaAccessor)
        return;

    // free the source
    if (pColladaAccessor->m_pSource)
        free(pColladaAccessor->m_pSource);

    // free the parameter array
    if (pColladaAccessor->m_pParams)
    {
        size_t i;

        // iterate through mesh infos to free
        for (i = 0; i < pColladaAccessor->m_ParamCount; ++i)
            // free the parameter content
            csrColladaParamRelease(&pColladaAccessor->m_pParams[i]);

        // free the name array container
        free(pColladaAccessor->m_pParams);
    }
}
//---------------------------------------------------------------------------
int csrColladaAccessorRead(XMLNode* pNode, CSR_Collada_Accessor* pColladaAccessor)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaAccessor)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Source_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Source_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaAccessor->m_pSource);
            else
            if (len == strlen(M_Collada_Count_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Count_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read count value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert count to value
                pColladaAccessor->m_Count = atoi(pCount);

                free(pCount);
            }
            else
            if (len == strlen(M_Collada_Stride_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Stride_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read count value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert count to value
                pColladaAccessor->m_Stride = atoi(pCount);

                free(pCount);
            }
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Param_Tag) &&
            memcmp(pChild->tag, M_Collada_Param_Tag, len) == 0)
        {
            const size_t index = pColladaAccessor->m_ParamCount;

            // add a new parameter in the array
            CSR_Collada_Param* pParams =
                    (CSR_Collada_Param*)csrMemoryAlloc(pColladaAccessor->m_pParams,
                                                       sizeof(CSR_Collada_Param),
                                                       pColladaAccessor->m_ParamCount + 1);

            // succeeded?
            if (!pParams)
                return 0;

            // set new parameter array in the accessor
            pColladaAccessor->m_pParams = pParams;
            ++pColladaAccessor->m_ParamCount;

            // initialize newly added parameter
            csrColladaParamInit(&pColladaAccessor->m_pParams[index]);

            // read it
            if (!csrColladaParamRead(pChild, &pColladaAccessor->m_pParams[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaTechCommonInit(CSR_Collada_Technique_Common* pColladaTechCommon)
{
    if (!pColladaTechCommon)
        return;

    pColladaTechCommon->m_pAccessor = 0;
}
//---------------------------------------------------------------------------
void csrColladaTechCommonRelease(CSR_Collada_Technique_Common* pColladaTechCommon)
{
    if (!pColladaTechCommon)
        return;

    // free the accessor
    if (pColladaTechCommon->m_pAccessor)
    {
        csrColladaAccessorRelease(pColladaTechCommon->m_pAccessor);
        free(pColladaTechCommon->m_pAccessor);
    }
}
//---------------------------------------------------------------------------
int csrColladaTechCommonRead(XMLNode* pNode, CSR_Collada_Technique_Common* pColladaTechCommon)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaTechCommon)
        return 0;

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Accessor_Tag) &&
            memcmp(pChild->tag, M_Collada_Accessor_Tag, len) == 0)
        {
            // only one accessor is allowed
            if (pColladaTechCommon->m_pAccessor)
                return 0;

            // add a new accessor in the array
            pColladaTechCommon->m_pAccessor = (CSR_Collada_Accessor*)malloc(sizeof(CSR_Collada_Accessor));

            // succeeded?
            if (!pColladaTechCommon->m_pAccessor)
                return 0;

            // initialize newly added accessor
            csrColladaAccessorInit(pColladaTechCommon->m_pAccessor);

            // read it
            if (!csrColladaAccessorRead(pChild, pColladaTechCommon->m_pAccessor))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaSourceInit(CSR_Collada_Source* pColladaSource)
{
    if (!pColladaSource)
        return;

    pColladaSource->m_pId         = 0;
    pColladaSource->m_pFloatArray = 0;
    pColladaSource->m_pNameArray  = 0;
    pColladaSource->m_pTechCommon = 0;
}
//---------------------------------------------------------------------------
void csrColladaSourceRelease(CSR_Collada_Source* pColladaSource)
{
    // no collada source to release?
    if (!pColladaSource)
        return;

    // free the identifier
    if (pColladaSource->m_pId)
        free(pColladaSource->m_pId);

    // free the float array
    if (pColladaSource->m_pFloatArray)
    {
        csrColladaFloatArrayRelease(pColladaSource->m_pFloatArray);
        free(pColladaSource->m_pFloatArray);
    }

    // free the name array
    if (pColladaSource->m_pNameArray)
    {
        csrColladaNameArrayRelease(pColladaSource->m_pNameArray);
        free(pColladaSource->m_pNameArray);
    }

    // free the tech commons
    if (pColladaSource->m_pTechCommon)
    {
        csrColladaTechCommonRelease(pColladaSource->m_pTechCommon);
        free(pColladaSource->m_pTechCommon);
    }
}
//---------------------------------------------------------------------------
int csrColladaSourceRead(XMLNode* pNode, CSR_Collada_Source* pColladaSource)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaSource)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaSource->m_pId);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Float_Array_Tag) &&
            memcmp(pChild->tag, M_Collada_Float_Array_Tag, len) == 0)
        {
            // only one float array is allowed
            if (pColladaSource->m_pFloatArray)
                return 0;

            // create new float array
            pColladaSource->m_pFloatArray = (CSR_Collada_Float_Array*)malloc(sizeof(CSR_Collada_Float_Array));

            // succeeded?
            if (!pColladaSource->m_pFloatArray)
                return 0;

            // initialize newly created float array
            csrColladaFloatArrayInit(pColladaSource->m_pFloatArray);

            // read it
            if (!csrColladaFloatArrayRead(pChild, pColladaSource->m_pFloatArray))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Name_Array_Tag) &&
            memcmp(pChild->tag, M_Collada_Name_Array_Tag, len) == 0)
        {
            // only one name array is allowed
            if (pColladaSource->m_pNameArray)
                return 0;

            // create new name array
            pColladaSource->m_pNameArray = (CSR_Collada_Name_Array*)malloc(sizeof(CSR_Collada_Name_Array));

            // succeeded?
            if (!pColladaSource->m_pNameArray)
                return 0;

            // initialize newly created name array
            csrColladaNameArrayInit(pColladaSource->m_pNameArray);

            // read it
            if (!csrColladaNameArrayRead(pChild, pColladaSource->m_pNameArray))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Technique_Common_Tag) &&
            memcmp(pChild->tag, M_Collada_Technique_Common_Tag, len) == 0)
        {
            // only one tech common is allowed
            if (pColladaSource->m_pTechCommon)
                return 0;

            // create new technique common
            pColladaSource->m_pTechCommon = (CSR_Collada_Technique_Common*)malloc(sizeof(CSR_Collada_Technique_Common));

            // succeeded?
            if (!pColladaSource->m_pTechCommon)
                return 0;

            // initialize newly created technique commons
            csrColladaTechCommonInit(pColladaSource->m_pTechCommon);

            // read it
            if (!csrColladaTechCommonRead(pChild, pColladaSource->m_pTechCommon))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaInputInit(CSR_Collada_Input* pColladaInput)
{
    if (!pColladaInput)
        return;

    pColladaInput->m_Offset    = 0;
    pColladaInput->m_pSemantic = 0;
    pColladaInput->m_pSource   = 0;
    pColladaInput->m_Set       = 0;
}
//---------------------------------------------------------------------------
void csrColladaInputRelease(CSR_Collada_Input* pColladaInput)
{
    if (!pColladaInput)
        return;

    // free the source
    if (pColladaInput->m_pSource)
        free(pColladaInput->m_pSource);

    // free the semantic
    if (pColladaInput->m_pSemantic)
        free(pColladaInput->m_pSemantic);
}
//---------------------------------------------------------------------------
int csrColladaInputRead(XMLNode* pNode, CSR_Collada_Input* pColladaInput)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaInput)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Source_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Source_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaInput->m_pSource);
            else
            if (len == strlen(M_Collada_Semantic_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Semantic_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaInput->m_pSemantic);
            else
            if (len == strlen(M_Collada_Offset_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Offset_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read offset value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert offset to value
                pColladaInput->m_Offset = (size_t)atoi(pCount);

                free(pCount);
            }
            else
            if (len == strlen(M_Collada_Set_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Set_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read set value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert set to value
                pColladaInput->m_Set = atoi(pCount);

                free(pCount);
            }
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaVerticesInit(CSR_Collada_Vertices* pColladaVertices)
{
    if (!pColladaVertices)
        return;

    pColladaVertices->m_pId        = 0;
    pColladaVertices->m_pInputs    = 0;
    pColladaVertices->m_InputCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaVerticesRelease(CSR_Collada_Vertices* pColladaVertices)
{
    if (!pColladaVertices)
        return;

    // free the identifier
    if (pColladaVertices->m_pId)
        free(pColladaVertices->m_pId);

    // free the source array
    if (pColladaVertices->m_pInputs)
    {
        size_t i;

        // iterate through sources to free
        for (i = 0; i < pColladaVertices->m_InputCount; ++i)
            csrColladaInputRelease(&pColladaVertices->m_pInputs[i]);

        // free the sources container
        free(pColladaVertices->m_pInputs);
    }
}
//---------------------------------------------------------------------------
int csrColladaVerticesRead(XMLNode* pNode, CSR_Collada_Vertices* pColladaVertices)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaVertices)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaVertices->m_pId);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Input_Tag) &&
            memcmp(pChild->tag, M_Collada_Input_Tag, len) == 0)
        {
            const size_t index = pColladaVertices->m_InputCount;

            // add a new input in the array
            CSR_Collada_Input* pInputs =
                    (CSR_Collada_Input*)csrMemoryAlloc(pColladaVertices->m_pInputs,
                                                       sizeof(CSR_Collada_Input),
                                                       pColladaVertices->m_InputCount + 1);

            // succeeded?
            if (!pInputs)
                return 0;

            // set new input in the vertices
            pColladaVertices->m_pInputs = pInputs;
            ++pColladaVertices->m_InputCount;

            // initialize newly added input
            csrColladaInputInit(&pColladaVertices->m_pInputs[index]);

            // read it
            if (!csrColladaInputRead(pChild, &pColladaVertices->m_pInputs[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaTrianglesInit(CSR_Collada_Triangles* pColladaTriangles)
{
    if (!pColladaTriangles)
        return;

    pColladaTriangles->m_Count           = 0;
    pColladaTriangles->m_pInputs         = 0;
    pColladaTriangles->m_InputCount      = 0;
    pColladaTriangles->m_pPrimitiveArray = 0;
}
//---------------------------------------------------------------------------
void csrColladaTrianglesRelease(CSR_Collada_Triangles* pColladaTriangles)
{
    size_t i;

    if (!pColladaTriangles)
        return;

    // free the source array
    if (pColladaTriangles->m_pInputs)
    {
        // iterate through sources to free
        for (i = 0; i < pColladaTriangles->m_InputCount; ++i)
            csrColladaInputRelease(&pColladaTriangles->m_pInputs[i]);

        // free the sources container
        free(pColladaTriangles->m_pInputs);
    }

    // free the primitive array
    if (pColladaTriangles->m_pPrimitiveArray)
    {
        csrColladaUnsignedArrayRelease(pColladaTriangles->m_pPrimitiveArray);
        free(pColladaTriangles->m_pPrimitiveArray);
    }
}
//---------------------------------------------------------------------------
int csrColladaTrianglesRead(XMLNode* pNode, CSR_Collada_Triangles* pColladaTriangles)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaTriangles)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Count_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Count_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read count value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert count to value
                pColladaTriangles->m_Count = atoi(pCount);

                free(pCount);
            }
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Input_Tag) &&
            memcmp(pChild->tag, M_Collada_Input_Tag, len) == 0)
        {
            const size_t index = pColladaTriangles->m_InputCount;

            // add a new input in the array
            CSR_Collada_Input* pInputs =
                    (CSR_Collada_Input*)csrMemoryAlloc(pColladaTriangles->m_pInputs,
                                                       sizeof(CSR_Collada_Input),
                                                       pColladaTriangles->m_InputCount + 1);

            // succeeded?
            if (!pInputs)
                return 0;

            // set new parameter array in the triangles
            pColladaTriangles->m_pInputs = pInputs;
            ++pColladaTriangles->m_InputCount;

            // initialize newly added input
            csrColladaInputInit(&pColladaTriangles->m_pInputs[index]);

            // read it
            if (!csrColladaInputRead(pChild, &pColladaTriangles->m_pInputs[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_P_Tag) &&
            memcmp(pChild->tag, M_Collada_P_Tag, len) == 0)
        {
            // only one primitive array is allowed
            if (pColladaTriangles->m_pPrimitiveArray)
                return 0;

            // create new primitive array
            pColladaTriangles->m_pPrimitiveArray =
                    (CSR_Collada_Unsigned_Array*)malloc(sizeof(CSR_Collada_Unsigned_Array));

            // succeeded?
            if (!pColladaTriangles->m_pPrimitiveArray)
                return 0;

            // initialize newly created primitive array
            csrColladaUnsignedArrayInit(pColladaTriangles->m_pPrimitiveArray);

            // read it
            if (!csrColladaUnsignedArrayRead(pChild,
                                             pColladaTriangles->m_pPrimitiveArray,
                                             pColladaTriangles->m_Count * pColladaTriangles->m_InputCount * 3))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaMeshInit(CSR_Collada_Mesh* pColladaMesh)
{
    if (!pColladaMesh)
        return;

    pColladaMesh->m_pSources      = 0;
    pColladaMesh->m_SourceCount   = 0;
    pColladaMesh->m_pVertices     = 0;
    pColladaMesh->m_VertexCount   = 0;
    pColladaMesh->m_pTriangles    = 0;
    pColladaMesh->m_TriangleCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaMeshRelease(CSR_Collada_Mesh* pColladaMesh)
{
    size_t i;

    if (!pColladaMesh)
        return;

    // free the source array
    if (pColladaMesh->m_pSources)
    {
        // iterate through sources to free
        for (i = 0; i < pColladaMesh->m_SourceCount; ++i)
            csrColladaSourceRelease(&pColladaMesh->m_pSources[i]);

        // free the sources container
        free(pColladaMesh->m_pSources);
    }

    // free the vertex array
    if (pColladaMesh->m_pVertices)
    {
        // iterate through vertices to free
        for (i = 0; i < pColladaMesh->m_VertexCount; ++i)
            csrColladaVerticesRelease(&pColladaMesh->m_pVertices[i]);

        // free the vertices container
        free(pColladaMesh->m_pVertices);
    }
    // free the triangle array
    if (pColladaMesh->m_pTriangles)
    {
        // iterate through triangles to free
        for (i = 0; i < pColladaMesh->m_TriangleCount; ++i)
            csrColladaTrianglesRelease(&pColladaMesh->m_pTriangles[i]);

        // free the triangles container
        free(pColladaMesh->m_pTriangles);
    }
}
//---------------------------------------------------------------------------
int csrColladaMeshRead(XMLNode* pNode, CSR_Collada_Mesh* pColladaMesh)
{
    size_t              i;
    size_t              len;
    size_t              index    = 0;
    CSR_Collada_Source* pSources = 0;

    if (!pNode)
        return 0;

    if (!pColladaMesh)
        return 0;

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Source_Tag) &&
            memcmp(pChild->tag, M_Collada_Source_Tag, len) == 0)
        {
            const size_t index = pColladaMesh->m_SourceCount;

            // add a new source in the array
            CSR_Collada_Source* pSources =
                    (CSR_Collada_Source*)csrMemoryAlloc(pColladaMesh->m_pSources,
                                                        sizeof(CSR_Collada_Source),
                                                        pColladaMesh->m_SourceCount + 1);

            // succeeded?
            if (!pSources)
                return 0;

            // set new source array in the mesh
            pColladaMesh->m_pSources = pSources;
            ++pColladaMesh->m_SourceCount;

            // initialize newly added source
            csrColladaSourceInit(&pColladaMesh->m_pSources[index]);

            // read it
            if (!csrColladaSourceRead(pChild, &pColladaMesh->m_pSources[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Vertices_Tag) &&
            memcmp(pChild->tag, M_Collada_Vertices_Tag, len) == 0)
        {
            const size_t index = pColladaMesh->m_VertexCount;

            // add a new vertices item in the array
            CSR_Collada_Vertices* pVertices =
                    (CSR_Collada_Vertices*)csrMemoryAlloc(pColladaMesh->m_pVertices,
                                                          sizeof(CSR_Collada_Vertices),
                                                          pColladaMesh->m_VertexCount + 1);

            // succeeded?
            if (!pVertices)
                return 0;

            // set new vertices item in the mesh
            pColladaMesh->m_pVertices = pVertices;
            ++pColladaMesh->m_VertexCount;

            // initialize newly added vertices item
            csrColladaVerticesInit(&pColladaMesh->m_pVertices[index]);

            // read it
            if (!csrColladaVerticesRead(pChild, &pColladaMesh->m_pVertices[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Triangles_Tag) &&
            memcmp(pChild->tag, M_Collada_Triangles_Tag, len) == 0)
        {
            const size_t index = pColladaMesh->m_TriangleCount;

            // add a new triangles item in the array
            CSR_Collada_Triangles* pTriangles =
                    (CSR_Collada_Triangles*)csrMemoryAlloc(pColladaMesh->m_pTriangles,
                                                           sizeof(CSR_Collada_Triangles),
                                                           pColladaMesh->m_TriangleCount + 1);

            // succeeded?
            if (!pTriangles)
                return 0;

            // set new triangles item in the mesh
            pColladaMesh->m_pTriangles = pTriangles;
            ++pColladaMesh->m_TriangleCount;

            // initialize newly added triangles item
            csrColladaTrianglesInit(&pColladaMesh->m_pTriangles[index]);

            // read it
            if (!csrColladaTrianglesRead(pChild, &pColladaMesh->m_pTriangles[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrColladaMeshBuild(const CSR_VertexFormat*     pVertFormat,
                        const CSR_VertexCulling*    pVertCulling,
                        const CSR_Material*         pMaterial,
                              CSR_Collada_Mesh*     pColladaMesh,
                              CSR_Mesh*             pMesh,
                        const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    size_t                      i;
    size_t                      j;
    size_t                      k;
    size_t                      stride       = 0;
    size_t                      indicesCount = 0;
    CSR_Collada_Vertices*       pVertices    = 0;
    CSR_Collada_Triangles*      pTriangles   = 0;
    CSR_Collada_Source*         pPositions   = 0;
    CSR_Collada_Source*         pNormals     = 0;
    CSR_Collada_Source*         pTexCoords   = 0;
    CSR_Collada_Unsigned_Array* pIndices     = 0;

    if (!pColladaMesh)
        return 0;

    if (!pMesh)
        return 0;

    // iterate through vertices
    for (i = 0; i < pColladaMesh->m_VertexCount; ++i)
    {
        // already found triangles and doesn't match with the found one?
        if (pVertices && pVertices != &pColladaMesh->m_pVertices[i])
            return 0;

        pVertices = &pColladaMesh->m_pVertices[i];

        // iterate through vertex inputs
        for (j = 0; j < pVertices->m_InputCount; ++j)
        {
            // get the semantic length
            const size_t len = strlen(pVertices->m_pInputs[j].m_pSemantic);

            // search for semantic
            if ((len == strlen(M_Collada_Semantic_Position) &&
                 memcmp(pVertices->m_pInputs[j].m_pSemantic,
                        M_Collada_Semantic_Position,
                        len) == 0) ||
                (len == strlen(M_Collada_Semantic_Vertex) &&
                 memcmp(pVertices->m_pInputs[j].m_pSemantic,
                        M_Collada_Semantic_Vertex,
                        len) == 0))
            {
                // need a vertices source, iterate through sources
                for (k = 0; k < pColladaMesh->m_SourceCount; ++k)
                {
                    // get the id lengths
                    const size_t srcLen = strlen(pVertices->m_pInputs[j].m_pSource);
                    const size_t idLen  = strlen(pColladaMesh->m_pSources[k].m_pId);

                    // source and vertices id matches?
                    if (srcLen > 1          &&
                        srcLen - 1 == idLen &&
                        memcmp(pVertices->m_pInputs[j].m_pSource + 1,
                               pColladaMesh->m_pSources[k].m_pId,
                               idLen) == 0)
                    {
                        // already found positions source and doesn't match with the found one?
                        if (pPositions && pPositions != &pColladaMesh->m_pSources[k])
                            return 0;

                        // get positions source
                        pPositions = &pColladaMesh->m_pSources[k];
                    }
                }
            }
            else
            if (len == strlen(M_Collada_Semantic_Normal) &&
                memcmp(pVertices->m_pInputs[j].m_pSemantic,
                        M_Collada_Semantic_Normal,
                        len) == 0)
            {
                // need a normals source, iterate through sources
                for (k = 0; k < pColladaMesh->m_SourceCount; ++k)
                {
                    // get the id lengths
                    const size_t srcLen = strlen(pVertices->m_pInputs[j].m_pSource);
                    const size_t idLen  = strlen(pColladaMesh->m_pSources[k].m_pId);

                    // source and normals id matches?
                    if (srcLen > 1          &&
                        srcLen - 1 == idLen &&
                        memcmp(pVertices->m_pInputs[j].m_pSource + 1,
                               pColladaMesh->m_pSources[k].m_pId,
                               idLen) == 0)
                    {
                        // already found normals source and doesn't match with the found one?
                        if (pNormals && pNormals != &pColladaMesh->m_pSources[k])
                            return 0;

                        // get normals source
                        pNormals = &pColladaMesh->m_pSources[k];
                    }
                }
            }
            else
            if (len == strlen(M_Collada_Semantic_TexCoord) &&
                memcmp(pVertices->m_pInputs[j].m_pSemantic,
                       M_Collada_Semantic_TexCoord,
                       len) == 0)
            {
                // need a texture coordinates source, iterate through sources
                for (k = 0; k < pColladaMesh->m_SourceCount; ++k)
                {
                    // get the id lengths
                    const size_t srcLen = strlen(pVertices->m_pInputs[j].m_pSource);
                    const size_t idLen  = strlen(pColladaMesh->m_pSources[k].m_pId);

                    // source and texture coordinates id matches?
                    if (srcLen > 1          &&
                        srcLen - 1 == idLen &&
                        memcmp(pVertices->m_pInputs[j].m_pSource + 1,
                               pColladaMesh->m_pSources[k].m_pId,
                               idLen) == 0)
                    {
                        // already found texture coordinates source and doesn't match with the found one?
                        if (pTexCoords && pTexCoords != &pColladaMesh->m_pSources[k])
                            return 0;

                        // get texture coordinates source
                        pTexCoords = &pColladaMesh->m_pSources[k];
                    }
                }
            }
        }
    }

    // iterate through triangles
    for (i = 0; i < pColladaMesh->m_TriangleCount; ++i)
    {
        // already found triangles and doesn't match with the found one?
        if (pTriangles && pTriangles != &pColladaMesh->m_pTriangles[i])
            return 0;

        pTriangles = &pColladaMesh->m_pTriangles[i];

        // iterate through triangle inputs
        for (j = 0; j < pTriangles->m_InputCount; ++j)
        {
            // get the semantic length
            const size_t len = strlen(pTriangles->m_pInputs[j].m_pSemantic);

            // search for semantic
            if ((len == strlen(M_Collada_Semantic_Position) &&
                 memcmp(pTriangles->m_pInputs[j].m_pSemantic,
                        M_Collada_Semantic_Position,
                        len) == 0) ||
                (len == strlen(M_Collada_Semantic_Vertex) &&
                 memcmp(pTriangles->m_pInputs[j].m_pSemantic,
                        M_Collada_Semantic_Vertex,
                        len) == 0))
            {
                // need a vertices source, iterate through sources
                for (k = 0; k < pColladaMesh->m_SourceCount; ++k)
                {
                    // get the id lengths
                    const size_t srcLen = strlen(pTriangles->m_pInputs[j].m_pSource);
                    const size_t idLen  = strlen(pColladaMesh->m_pSources[k].m_pId);

                    // source and vertices id matches?
                    if (srcLen > 1          &&
                        srcLen - 1 == idLen &&
                        memcmp(pTriangles->m_pInputs[j].m_pSource + 1,
                               pColladaMesh->m_pSources[k].m_pId,
                               idLen) == 0)
                    {
                        // already found positions source and doesn't match with the found one?
                        if (pPositions && pPositions != &pColladaMesh->m_pSources[k])
                            return 0;

                        // get positions source
                        pPositions = &pColladaMesh->m_pSources[k];
                    }
                }
            }
            else
            if (len == strlen(M_Collada_Semantic_Normal) &&
                memcmp(pTriangles->m_pInputs[j].m_pSemantic,
                        M_Collada_Semantic_Normal,
                        len) == 0)
            {
                // need a normals source, iterate through sources
                for (k = 0; k < pColladaMesh->m_SourceCount; ++k)
                {
                    // get the id lengths
                    const size_t srcLen = strlen(pTriangles->m_pInputs[j].m_pSource);
                    const size_t idLen  = strlen(pColladaMesh->m_pSources[k].m_pId);

                    // source and normals id matches?
                    if (srcLen > 1          &&
                        srcLen - 1 == idLen &&
                        memcmp(pTriangles->m_pInputs[j].m_pSource + 1,
                                pColladaMesh->m_pSources[k].m_pId,
                                idLen) == 0)
                    {
                        // already found normals source and doesn't match with the found one?
                        if (pNormals && pNormals != &pColladaMesh->m_pSources[k])
                            return 0;

                        // get normals source
                        pNormals = &pColladaMesh->m_pSources[k];
                    }
                }
            }
            else
            if (len == strlen(M_Collada_Semantic_TexCoord) &&
                memcmp(pTriangles->m_pInputs[j].m_pSemantic,
                        M_Collada_Semantic_TexCoord,
                        len) == 0)
            {
                // need a texture coordinates source, iterate through sources
                for (k = 0; k < pColladaMesh->m_SourceCount; ++k)
                {
                    // get the id lengths
                    const size_t srcLen = strlen(pTriangles->m_pInputs[j].m_pSource);
                    const size_t idLen  = strlen(pColladaMesh->m_pSources[k].m_pId);

                    // source and texture coordinates id matches?
                    if (srcLen > 1          &&
                        srcLen - 1 == idLen &&
                        memcmp(pTriangles->m_pInputs[j].m_pSource + 1,
                                pColladaMesh->m_pSources[k].m_pId,
                                idLen) == 0)
                    {
                        // already found texture coordinates source and doesn't match with the found one?
                        if (pTexCoords && pTexCoords != &pColladaMesh->m_pSources[k])
                            return 0;

                        // get texture coordinates source
                        pTexCoords = &pColladaMesh->m_pSources[k];
                    }
                }
            }
        }

        // already found indices source and doesn't match with the found one?
        if (pIndices && pIndices != pTriangles->m_pPrimitiveArray)
            return 0;

        // get indices
        pIndices = pTriangles->m_pPrimitiveArray;
    }

    if (!pPositions)
        return 0;

    if (!pIndices)
        return 0;

    // get the indices stride, it's the same as the number of inputs
    stride = pTriangles->m_InputCount;

    // calculate the indices count: triangle count * stride * 3 vertices
    indicesCount = pTriangles->m_Count * stride * 3;

    // create a new vertex buffer for the mesh
    pMesh->m_pVB = csrVertexBufferCreate();

    // succeeded?
    if (!pMesh->m_pVB)
        return 0;

    pMesh->m_Count = 1;

    // apply the user wished vertex format
    if (pVertFormat)
        pMesh->m_pVB->m_Format = *pVertFormat;

    // apply the user wished vertex culling
    if (pVertCulling)
        pMesh->m_pVB->m_Culling = *pVertCulling;
    else
    {
        // otherwise configure the default culling
        pMesh->m_pVB->m_Culling.m_Type = CSR_CT_Back;
        pMesh->m_pVB->m_Culling.m_Face = CSR_CF_CCW;
    }

    // apply the user wished material
    if (pMaterial)
        pMesh->m_pVB->m_Material = *pMaterial;

    // set the vertex format type
    pMesh->m_pVB->m_Format.m_Type = CSR_VT_Triangles;

    // calculate the stride
    csrVertexFormatCalculateStride(&pMesh->m_pVB->m_Format);

    // iterate through indices
    for (i = 0; i < indicesCount; i += stride)
    {
        #ifdef _MSC_VER
            size_t      index   =  0;
            CSR_Vector3 vertex  = {0};
            CSR_Vector3 normal  = {0};
            CSR_Vector2 uv      = {0};
        #else
            size_t      index   = 0;
            CSR_Vector3 vertex;
            CSR_Vector3 normal;
            CSR_Vector2 uv;
        #endif

        index = pIndices->m_pData[i] * 3;

        // build the vertex
        vertex.m_X = pPositions->m_pFloatArray->m_pData[index];
        vertex.m_Y = pPositions->m_pFloatArray->m_pData[index + 1];
        vertex.m_Z = pPositions->m_pFloatArray->m_pData[index + 2];

        // mesh contains normals?
        if (pMesh->m_pVB->m_Format.m_HasNormal && pNormals)
        {
            if (stride == 1)
                index = pIndices->m_pData[i] * 3;
            else
                index = pIndices->m_pData[i + 1] * 3;

            // build the normal
            normal.m_X = pNormals->m_pFloatArray->m_pData[index];
            normal.m_Y = pNormals->m_pFloatArray->m_pData[index + 1];
            normal.m_Z = pNormals->m_pFloatArray->m_pData[index + 2];
        }
        else
        {
            normal.m_X = 0.0f;
            normal.m_Y = 0.0f;
            normal.m_Z = 0.0f;
        }

        // mesh contains texture coordinates?
        if (pMesh->m_pVB->m_Format.m_HasTexCoords && pTexCoords)
        {
            if (stride == 1)
                index = pIndices->m_pData[i] * 2;
            else
                index = pIndices->m_pData[i + 2] * 2;

            // build the normal
            uv.m_X = pTexCoords->m_pFloatArray->m_pData[index];
            uv.m_Y = pTexCoords->m_pFloatArray->m_pData[index + 1];
        }
        else
        {
            uv.m_X = 0.0f;
            uv.m_Y = 0.0f;
        }

        // add the next vertex to the buffer
        if (!csrVertexBufferAdd(&vertex,
                                &normal,
                                &uv,
                                 0,
                                 fOnGetVertexColor,
                                 pMesh->m_pVB))
            return 0;
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaGeometryInit(CSR_Collada_Geometry* pColladaGeometry)
{
    if (!pColladaGeometry)
        return;

    pColladaGeometry->m_pId   = 0;
    pColladaGeometry->m_pName = 0;
    pColladaGeometry->m_pMesh = 0;
}
//---------------------------------------------------------------------------
void csrColladaGeometryRelease(CSR_Collada_Geometry* pColladaGeometry)
{
    if (!pColladaGeometry)
        return;

    // free the identifier
    if (pColladaGeometry->m_pId)
        free(pColladaGeometry->m_pId);

    // free the name
    if (pColladaGeometry->m_pName)
        free(pColladaGeometry->m_pName);

    // free the mesh array
    if (pColladaGeometry->m_pMesh)
    {
        // free the mesh container
        csrColladaMeshRelease(pColladaGeometry->m_pMesh);
        free(pColladaGeometry->m_pMesh);
    }
}
//---------------------------------------------------------------------------
int csrColladaGeometryRead(XMLNode* pNode, CSR_Collada_Geometry* pColladaGeometry)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaGeometry)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaGeometry->m_pId);
            else
            if (len == strlen(M_Collada_Name_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Name_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaGeometry->m_pName);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Mesh_Tag) &&
            memcmp(pChild->tag, M_Collada_Mesh_Tag, len) == 0)
        {
            // only one mesh is allowed
            if (pColladaGeometry->m_pMesh)
                return 0;

            // create new mesh
            pColladaGeometry->m_pMesh = (CSR_Collada_Mesh*)malloc(sizeof(CSR_Collada_Mesh));

            // succeeded?
            if (!pColladaGeometry->m_pMesh)
                return 0;

            // initialize newly created mesh
            csrColladaMeshInit(pColladaGeometry->m_pMesh);

            // read it
            if (!csrColladaMeshRead(pChild, pColladaGeometry->m_pMesh))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaGeometriesInit(CSR_Collada_Geometries* pColladaGeometries)
{
    if (!pColladaGeometries)
        return;

    pColladaGeometries->m_pGeometries   = 0;
    pColladaGeometries->m_GeometryCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaGeometriesRelease(CSR_Collada_Geometries* pColladaGeometries)
{
    if (!pColladaGeometries)
        return;

    // free the geometry array
    if (pColladaGeometries->m_pGeometries)
    {
        size_t i;

        // iterate through geometries to free
        for (i = 0; i < pColladaGeometries->m_GeometryCount; ++i)
            csrColladaGeometryRelease(&pColladaGeometries->m_pGeometries[i]);

        // free the geometries container
        free(pColladaGeometries->m_pGeometries);
    }
}
//---------------------------------------------------------------------------
int csrColladaGeometriesRead(XMLNode* pNode, CSR_Collada_Geometries* pColladaGeometries)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaGeometries)
        return 0;

    // iterate through children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Geometry_Tag) &&
            memcmp(pChild->tag, M_Collada_Geometry_Tag, len) == 0)
        {
            const size_t index = pColladaGeometries->m_GeometryCount;

            // add a new geometry in the array
            CSR_Collada_Geometry* pGeometries =
                    (CSR_Collada_Geometry*)csrMemoryAlloc(pColladaGeometries->m_pGeometries,
                                                          sizeof(CSR_Collada_Geometry),
                                                          pColladaGeometries->m_GeometryCount + 1);

            // succeeded?
            if (!pGeometries)
                return 0;

            // set new geometry array in the geometries
            pColladaGeometries->m_pGeometries = pGeometries;
            ++pColladaGeometries->m_GeometryCount;

            // initialize newly added geometry
            csrColladaGeometryInit(&pColladaGeometries->m_pGeometries[index]);

            // read it
            if (!csrColladaGeometryRead(pChild, &pColladaGeometries->m_pGeometries[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaJointsInit(CSR_Collada_Joints* pColladaJoints)
{
    if (!pColladaJoints)
        return;

    pColladaJoints->m_pInputs    = 0;
    pColladaJoints->m_InputCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaJointsRelease(CSR_Collada_Joints* pColladaJoints)
{
    // no collada joints to release?
    if (!pColladaJoints)
        return;

    // free the input array
    if (pColladaJoints->m_pInputs)
    {
        size_t i;

        for (i = 0; i < pColladaJoints->m_InputCount; ++i)
            csrColladaInputRelease(&pColladaJoints->m_pInputs[i]);

        free(pColladaJoints->m_pInputs);
    }
}
//---------------------------------------------------------------------------
int csrColladaJointsRead(XMLNode* pNode, CSR_Collada_Joints* pColladaJoints)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaJoints)
        return 0;

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Input_Tag) &&
            memcmp(pChild->tag, M_Collada_Input_Tag, len) == 0)
        {
            const size_t index = pColladaJoints->m_InputCount;

            // add a new input in the array
            CSR_Collada_Input* pInputs =
                    (CSR_Collada_Input*)csrMemoryAlloc(pColladaJoints->m_pInputs,
                                                       sizeof(CSR_Collada_Input),
                                                       pColladaJoints->m_InputCount + 1);

            // succeeded?
            if (!pInputs)
                return 0;

            // set new input array in the joints
            pColladaJoints->m_pInputs = pInputs;
            ++pColladaJoints->m_InputCount;

            // initialize newly added input
            csrColladaInputInit(&pColladaJoints->m_pInputs[index]);

            // read it
            if (!csrColladaInputRead(pChild, &pColladaJoints->m_pInputs[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaVertexWeightsInit(CSR_Collada_Vertex_Weights* pColladaVertexWeights)
{
    if (!pColladaVertexWeights)
        return;

    pColladaVertexWeights->m_Count                   = 0;
    pColladaVertexWeights->m_InputCount              = 0;
    pColladaVertexWeights->m_pInputs                 = 0;
    pColladaVertexWeights->m_pVertexToBoneArray      = 0;
    pColladaVertexWeights->m_pVertexToBoneCountArray = 0;
}
//---------------------------------------------------------------------------
void csrColladaVertexWeightsRelease(CSR_Collada_Vertex_Weights* pColladaVertexWeights)
{
    size_t i;

    // no collada vertex weights to release?
    if (!pColladaVertexWeights)
        return;

    // free the vertex to bone array
    if (pColladaVertexWeights->m_pVertexToBoneArray)
    {
        csrColladaUnsignedArrayRelease(pColladaVertexWeights->m_pVertexToBoneArray);
        free(pColladaVertexWeights->m_pVertexToBoneArray);
    }

    // free the vertex to bone count array
    if (pColladaVertexWeights->m_pVertexToBoneCountArray)
    {
        csrColladaUnsignedArrayRelease(pColladaVertexWeights->m_pVertexToBoneCountArray);
        free(pColladaVertexWeights->m_pVertexToBoneCountArray);
    }

    // free the input array
    if (pColladaVertexWeights->m_pInputs)
    {
        for (i = 0; i < pColladaVertexWeights->m_InputCount; ++i)
            csrColladaInputRelease(&pColladaVertexWeights->m_pInputs[i]);

        free(pColladaVertexWeights->m_pInputs);
    }
}
//---------------------------------------------------------------------------
int csrColladaVertexWeightsRead(XMLNode* pNode, CSR_Collada_Vertex_Weights* pColladaVertexWeights)
{
    size_t i;
    size_t len;
    size_t dataCount = 0;

    if (!pNode)
        return 0;

    if (!pColladaVertexWeights)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Count_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Count_Attribute, len) == 0)
            {
                char* pCount = 0;

                // read count value
                csrColladaReadAttribValue(&pAttributes[i], &pCount);

                // succeeded?
                if (!pCount)
                    return 0;

                // convert count to value
                pColladaVertexWeights->m_Count = atoi(pCount);

                free(pCount);
            }
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_V_Count_Tag) &&
            memcmp(pChild->tag, M_Collada_V_Count_Tag, len) == 0)
        {
            // create new vertex to bone count array
            pColladaVertexWeights->m_pVertexToBoneCountArray =
                    (CSR_Collada_Unsigned_Array*)malloc(sizeof(CSR_Collada_Unsigned_Array));

            // succeeded?
            if (!pColladaVertexWeights->m_pVertexToBoneCountArray)
                return 0;

            // initialize newly added array
            csrColladaUnsignedArrayInit(pColladaVertexWeights->m_pVertexToBoneCountArray);

            // read it
            if (!csrColladaUnsignedArrayRead(pChild,
                                             pColladaVertexWeights->m_pVertexToBoneCountArray,
                                             pColladaVertexWeights->m_Count))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Input_Tag) &&
            memcmp(pChild->tag, M_Collada_Input_Tag, len) == 0)
        {
            const size_t index = pColladaVertexWeights->m_InputCount;

            // add a new input in the array
            CSR_Collada_Input* pInputs =
                    (CSR_Collada_Input*)csrMemoryAlloc(pColladaVertexWeights->m_pInputs,
                                                       sizeof(CSR_Collada_Input),
                                                       pColladaVertexWeights->m_InputCount + 1);

            // succeeded?
            if (!pInputs)
                return 0;

            // set new input array in the vertex weights
            pColladaVertexWeights->m_pInputs = pInputs;
            ++pColladaVertexWeights->m_InputCount;

            // initialize newly added input
            csrColladaInputInit(&pColladaVertexWeights->m_pInputs[index]);

            // read it
            if (!csrColladaInputRead(pChild, &pColladaVertexWeights->m_pInputs[index]))
                return 0;
        }
    }

    // count the number of indices in the vertex to bone array
    for (i = 0; i < pColladaVertexWeights->m_pVertexToBoneCountArray->m_Count; ++i)
        dataCount += pColladaVertexWeights->m_pVertexToBoneCountArray->m_pData[i];

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_V_Tag) &&
            memcmp(pChild->tag, M_Collada_V_Tag, len) == 0)
        {
            // create new vertex to bone array
            pColladaVertexWeights->m_pVertexToBoneArray =
                    (CSR_Collada_Unsigned_Array*)malloc(sizeof(CSR_Collada_Unsigned_Array));

            // succeeded?
            if (!pColladaVertexWeights->m_pVertexToBoneArray)
                return 0;

            // initialize newly added array
            csrColladaUnsignedArrayInit(pColladaVertexWeights->m_pVertexToBoneArray);

            // read it
            if (!csrColladaUnsignedArrayRead(pChild,
                                             pColladaVertexWeights->m_pVertexToBoneArray,
                                             dataCount * pColladaVertexWeights->m_InputCount))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaSkinInit(CSR_Collada_Skin* pColladaSkin)
{
    if (!pColladaSkin)
        return;

    pColladaSkin->m_pBindShapeMatrix = 0;
    pColladaSkin->m_pSource          = 0;
    pColladaSkin->m_pSources         = 0;
    pColladaSkin->m_SourceCount      = 0;
    pColladaSkin->m_pJoints          = 0;
    pColladaSkin->m_pVertexWeights   = 0;
}
//---------------------------------------------------------------------------
void csrColladaSkinRelease(CSR_Collada_Skin* pColladaSkin)
{
    size_t i;

    // no collada skin to release?
    if (!pColladaSkin)
        return;

    // free the source id
    if (pColladaSkin->m_pSource)
        free(pColladaSkin->m_pSource);

    // free the source array
    if (pColladaSkin->m_pSources)
    {
        for (i = 0; i < pColladaSkin->m_SourceCount; ++i)
            csrColladaSourceRelease(&pColladaSkin->m_pSources[i]);

        free(pColladaSkin->m_pSources);
    }

    // free the bind shape matrix
    if (pColladaSkin->m_pBindShapeMatrix)
    {
        csrColladaMatrixRelease(pColladaSkin->m_pBindShapeMatrix);
        free(pColladaSkin->m_pBindShapeMatrix);
    }

    // free the joints
    if (pColladaSkin->m_pJoints)
    {
        csrColladaJointsRelease(pColladaSkin->m_pJoints);
        free(pColladaSkin->m_pJoints);
    }

    // free the vertex weights
    if (pColladaSkin->m_pVertexWeights)
    {
        csrColladaVertexWeightsRelease(pColladaSkin->m_pVertexWeights);
        free(pColladaSkin->m_pVertexWeights);
    }
}
//---------------------------------------------------------------------------
int csrColladaSkinRead(XMLNode* pNode, CSR_Collada_Skin* pColladaSkin)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaSkin)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Source_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Source_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaSkin->m_pSource);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Bind_Shape_Matrix_Tag) &&
            memcmp(pChild->tag, M_Collada_Bind_Shape_Matrix_Tag, len) == 0)
        {
            // create a new bind shape matrix
            pColladaSkin->m_pBindShapeMatrix = (CSR_Collada_Matrix*)malloc(sizeof(CSR_Collada_Matrix));

            // succeeded?
            if (!pColladaSkin->m_pBindShapeMatrix)
                return 0;

            // initialize newly added bind shape matrix
            csrColladaMatrixInit(pColladaSkin->m_pBindShapeMatrix);

            // read it
            if (!csrColladaMatrixRead(pChild, pColladaSkin->m_pBindShapeMatrix))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Joints_Tag) &&
            memcmp(pChild->tag, M_Collada_Joints_Tag, len) == 0)
        {
            // create a new joints
            pColladaSkin->m_pJoints = (CSR_Collada_Joints*)malloc(sizeof(CSR_Collada_Joints));

            // succeeded?
            if (!pColladaSkin->m_pJoints)
                return 0;

            // initialize newly added joints
            csrColladaJointsInit(pColladaSkin->m_pJoints);

            // read it
            if (!csrColladaJointsRead(pChild, pColladaSkin->m_pJoints))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Vertex_Weights_Tag) &&
            memcmp(pChild->tag, M_Collada_Vertex_Weights_Tag, len) == 0)
        {
            // create a new vertex weights
            pColladaSkin->m_pVertexWeights =
                    (CSR_Collada_Vertex_Weights*)malloc(sizeof(CSR_Collada_Vertex_Weights));

            // succeeded?
            if (!pColladaSkin->m_pVertexWeights)
                return 0;

            // initialize newly added vertex weights
            csrColladaVertexWeightsInit(pColladaSkin->m_pVertexWeights);

            // read it
            if (!csrColladaVertexWeightsRead(pChild, pColladaSkin->m_pVertexWeights))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Source_Tag) &&
            memcmp(pChild->tag, M_Collada_Source_Tag, len) == 0)
        {
            const size_t index = pColladaSkin->m_SourceCount;

            // add a new source in the array
            CSR_Collada_Source* pSources =
                    (CSR_Collada_Source*)csrMemoryAlloc(pColladaSkin->m_pSources,
                                                        sizeof(CSR_Collada_Source),
                                                        pColladaSkin->m_SourceCount + 1);

            // succeeded?
            if (!pSources)
                return 0;

            // set new source  array in the skin
            pColladaSkin->m_pSources = pSources;
            ++pColladaSkin->m_SourceCount;

            // initialize newly added skeleton
            csrColladaSourceInit(&pColladaSkin->m_pSources[index]);

            // read it
            if (!csrColladaSourceRead(pChild, &pColladaSkin->m_pSources[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaControllerInit(CSR_Collada_Controller* pColladaController)
{
    if (!pColladaController)
        return;

    pColladaController->m_pId   = 0;
    pColladaController->m_pName = 0;
    pColladaController->m_pSkin = 0;
}
//---------------------------------------------------------------------------
void csrColladaControllerRelease(CSR_Collada_Controller* pColladaController)
{
    // no collada controller to release?
    if (!pColladaController)
        return;

    // free the identifier
    if (pColladaController->m_pId)
        free(pColladaController->m_pId);

    // free the name
    if (pColladaController->m_pName)
        free(pColladaController->m_pName);

    // free the skin
    if (pColladaController->m_pSkin)
    {
        csrColladaSkinRelease(pColladaController->m_pSkin);
        free(pColladaController->m_pSkin);
    }
}
//---------------------------------------------------------------------------
int csrColladaControllerRead(XMLNode* pNode, CSR_Collada_Controller* pColladaController)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaController)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaController->m_pId);
            else
            if (len == strlen(M_Collada_Name_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Name_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaController->m_pName);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Skin_Tag) &&
            memcmp(pChild->tag, M_Collada_Skin_Tag, len) == 0)
        {
            // only one skin is allowed
            if (pColladaController->m_pSkin)
                return 0;

            // create a new skin
            pColladaController->m_pSkin = (CSR_Collada_Skin*)malloc(sizeof(CSR_Collada_Skin));

            // succeeded?
            if (!pColladaController->m_pSkin)
                return 0;

            // initialize newly added skin
            csrColladaSkinInit(pColladaController->m_pSkin);

            // read it
            if (!csrColladaSkinRead(pChild, pColladaController->m_pSkin))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaControllersInit(CSR_Collada_Controllers* pColladaControllers)
{
    if (!pColladaControllers)
        return;

    pColladaControllers->m_pControllers    = 0;
    pColladaControllers->m_ControllerCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaControllersRelease(CSR_Collada_Controllers* pColladaControllers)
{
    if (!pColladaControllers)
        return;

    // free the controller array
    if (pColladaControllers->m_pControllers)
    {
        size_t i;

        // iterate through controllers to free
        for (i = 0; i < pColladaControllers->m_ControllerCount; ++i)
            csrColladaControllerRelease(&pColladaControllers->m_pControllers[i]);

        // free the controllers container
        free(pColladaControllers->m_pControllers);
    }
}
//---------------------------------------------------------------------------
int csrColladaControllersRead(XMLNode* pNode, CSR_Collada_Controllers* pColladaControllers)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaControllers)
        return 0;

    // iterate through children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Controller_Tag) &&
            memcmp(pChild->tag, M_Collada_Controller_Tag, len) == 0)
        {
            const size_t index = pColladaControllers->m_ControllerCount;

            // add a new controller in the array
            CSR_Collada_Controller* pControllers =
                    (CSR_Collada_Controller*)csrMemoryAlloc(pColladaControllers->m_pControllers,
                                                            sizeof(CSR_Collada_Controller),
                                                            pColladaControllers->m_ControllerCount + 1);

            // succeeded?
            if (!pControllers)
                return 0;

            // set new controller array in the controllers
            pColladaControllers->m_pControllers = pControllers;
            ++pColladaControllers->m_ControllerCount;

            // initialize newly added controller
            csrColladaControllerInit(&pColladaControllers->m_pControllers[index]);

            // read it
            if (!csrColladaControllerRead(pChild, &pColladaControllers->m_pControllers[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaSamplerInit(CSR_Collada_Sampler* pColladaSampler)
{
    if (!pColladaSampler)
        return;

    pColladaSampler->m_pId        = 0;
    pColladaSampler->m_pInputs    = 0;
    pColladaSampler->m_InputCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaSamplerRelease(CSR_Collada_Sampler* pColladaSampler)
{
    // no collada sampler to release?
    if (!pColladaSampler)
        return;

    // free the identifier
    if (pColladaSampler->m_pId)
        free(pColladaSampler->m_pId);

    // free the input array
    if (pColladaSampler->m_pInputs)
    {
        size_t i;

        // iterate through inputs to free
        for (i = 0; i < pColladaSampler->m_InputCount; ++i)
            csrColladaInputRelease(&pColladaSampler->m_pInputs[i]);

        // free the inputs container
        free(pColladaSampler->m_pInputs);
    }
}
//---------------------------------------------------------------------------
int csrColladaSamplerRead(XMLNode* pNode, CSR_Collada_Sampler* pColladaSampler)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaSampler)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaSampler->m_pId);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Input_Tag) &&
            memcmp(pChild->tag, M_Collada_Input_Tag, len) == 0)
        {
            const size_t index = pColladaSampler->m_InputCount;

            // add a new input in the array
            CSR_Collada_Input* pInputs =
                    (CSR_Collada_Input*)csrMemoryAlloc(pColladaSampler->m_pInputs,
                                                       sizeof(CSR_Collada_Input),
                                                       pColladaSampler->m_InputCount + 1);

            // succeeded?
            if (!pInputs)
                return 0;

            // set new input array in the vertex weights
            pColladaSampler->m_pInputs = pInputs;
            ++pColladaSampler->m_InputCount;

            // initialize newly added input
            csrColladaInputInit(&pColladaSampler->m_pInputs[index]);

            // read it
            if (!csrColladaInputRead(pChild, &pColladaSampler->m_pInputs[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaChannelInit(CSR_Collada_Channel* pColladaChannel)
{
    if (!pColladaChannel)
        return;

    pColladaChannel->m_pSource = 0;
    pColladaChannel->m_pTarget = 0;
}
//---------------------------------------------------------------------------
void csrColladaChannelRelease(CSR_Collada_Channel* pColladaChannel)
{
    // no collada channel to release?
    if (!pColladaChannel)
        return;

    // free the source name
    if (pColladaChannel->m_pSource)
        free(pColladaChannel->m_pSource);

    // free the target name
    if (pColladaChannel->m_pTarget)
        free(pColladaChannel->m_pTarget);
}
//---------------------------------------------------------------------------
int csrColladaChannelRead(XMLNode* pNode, CSR_Collada_Channel* pColladaChannel)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaChannel)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Source_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Source_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaChannel->m_pSource);
            else
            if (len == strlen(M_Collada_Target_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Target_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaChannel->m_pTarget);
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaAnimationInit(CSR_Collada_Animation* pColladaAnimation)
{
    if (!pColladaAnimation)
        return;

    pColladaAnimation->m_pId            = 0;
    pColladaAnimation->m_pName          = 0;
    pColladaAnimation->m_pAnimations    = 0;
    pColladaAnimation->m_AnimationCount = 0;
    pColladaAnimation->m_pSources       = 0;
    pColladaAnimation->m_SourceCount    = 0;
    pColladaAnimation->m_pSamplers      = 0;
    pColladaAnimation->m_SamplerCount   = 0;
    pColladaAnimation->m_pChannels      = 0;
    pColladaAnimation->m_ChannelCount   = 0;
}
//---------------------------------------------------------------------------
void csrColladaAnimationRelease(CSR_Collada_Animation* pColladaAnimation)
{
    size_t i;

    // no collada animation to release?
    if (!pColladaAnimation)
        return;

    // free the identifier
    if (pColladaAnimation->m_pId)
        free(pColladaAnimation->m_pId);

    // free the name
    if (pColladaAnimation->m_pName)
        free(pColladaAnimation->m_pName);

    // free the source array
    if (pColladaAnimation->m_pSources)
    {
        // iterate through sources to free
        for (i = 0; i < pColladaAnimation->m_SourceCount; ++i)
            csrColladaSourceRelease(&pColladaAnimation->m_pSources[i]);

        // free the sources container
        free(pColladaAnimation->m_pSources);
    }

    // free the sampler array
    if (pColladaAnimation->m_pSamplers)
    {
        // iterate through samplers to free
        for (i = 0; i < pColladaAnimation->m_SamplerCount; ++i)
            csrColladaSamplerRelease(&pColladaAnimation->m_pSamplers[i]);

        // free the samplers container
        free(pColladaAnimation->m_pSamplers);
    }

    // free the channel array
    if (pColladaAnimation->m_pChannels)
    {
        // iterate through channels to free
        for (i = 0; i < pColladaAnimation->m_ChannelCount; ++i)
            csrColladaChannelRelease(&pColladaAnimation->m_pChannels[i]);

        // free the channels container
        free(pColladaAnimation->m_pChannels);
    }

    // free the animation array
    if (pColladaAnimation->m_pAnimations)
    {
        // iterate through animations to free
        for (i = 0; i < pColladaAnimation->m_AnimationCount; ++i)
            csrColladaAnimationRelease(&pColladaAnimation->m_pAnimations[i]);

        // free the animations container
        free(pColladaAnimation->m_pAnimations);
    }
}
//---------------------------------------------------------------------------
int csrColladaAnimationRead(XMLNode* pNode, CSR_Collada_Animation* pColladaAnimation)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaAnimation)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaAnimation->m_pId);
            else
            if (len == strlen(M_Collada_Name_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Name_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaAnimation->m_pName);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Source_Tag) &&
            memcmp(pChild->tag, M_Collada_Source_Tag, len) == 0)
        {
            const size_t index = pColladaAnimation->m_SourceCount;

            // add a new source in the array
            CSR_Collada_Source* pSources =
                    (CSR_Collada_Source*)csrMemoryAlloc(pColladaAnimation->m_pSources,
                                                        sizeof(CSR_Collada_Source),
                                                        pColladaAnimation->m_SourceCount + 1);

            // succeeded?
            if (!pSources)
                return 0;

            // set new source array in the animation
            pColladaAnimation->m_pSources = pSources;
            ++pColladaAnimation->m_SourceCount;

            // initialize newly added source
            csrColladaSourceInit(&pColladaAnimation->m_pSources[index]);

            // read it
            if (!csrColladaSourceRead(pChild, &pColladaAnimation->m_pSources[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Sampler_Tag) &&
            memcmp(pChild->tag, M_Collada_Sampler_Tag, len) == 0)
        {
            const size_t index = pColladaAnimation->m_SamplerCount;

            // add a new sampler in the array
            CSR_Collada_Sampler* pSamplers =
                    (CSR_Collada_Sampler*)csrMemoryAlloc(pColladaAnimation->m_pSamplers,
                                                         sizeof(CSR_Collada_Sampler),
                                                         pColladaAnimation->m_SamplerCount + 1);

            // succeeded?
            if (!pSamplers)
                return 0;

            // set new sampler array in the animation
            pColladaAnimation->m_pSamplers = pSamplers;
            ++pColladaAnimation->m_SamplerCount;

            // initialize newly added sampler
            csrColladaSamplerInit(&pColladaAnimation->m_pSamplers[index]);

            // read it
            if (!csrColladaSamplerRead(pChild, &pColladaAnimation->m_pSamplers[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Channel_Tag) &&
            memcmp(pChild->tag, M_Collada_Channel_Tag, len) == 0)
        {
            const size_t index = pColladaAnimation->m_ChannelCount;

            // add a new channel in the array
            CSR_Collada_Channel* pChannel =
                    (CSR_Collada_Channel*)csrMemoryAlloc(pColladaAnimation->m_pChannels,
                                                         sizeof(CSR_Collada_Channel),
                                                         pColladaAnimation->m_ChannelCount + 1);

            // succeeded?
            if (!pChannel)
                return 0;

            // set new channel array in the animation
            pColladaAnimation->m_pChannels = pChannel;
            ++pColladaAnimation->m_ChannelCount;

            // initialize newly added channel
            csrColladaChannelInit(&pColladaAnimation->m_pChannels[index]);

            // read it
            if (!csrColladaChannelRead(pChild, &pColladaAnimation->m_pChannels[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Animation_Tag) &&
            memcmp(pChild->tag, M_Collada_Animation_Tag, len) == 0)
        {
            const size_t index = pColladaAnimation->m_AnimationCount;

            // add a new child animation in the array
            CSR_Collada_Animation* pAnimations =
                    (CSR_Collada_Animation*)csrMemoryAlloc(pColladaAnimation->m_pAnimations,
                                                           sizeof(CSR_Collada_Animation),
                                                           pColladaAnimation->m_AnimationCount + 1);

            // succeeded?
            if (!pAnimations)
                return 0;

            // set new child animation array in the animation
            pColladaAnimation->m_pAnimations = pAnimations;
            ++pColladaAnimation->m_AnimationCount;

            // initialize newly added child animation
            csrColladaAnimationInit(&pColladaAnimation->m_pAnimations[index]);

            // read it
            if (!csrColladaAnimationRead(pChild, &pColladaAnimation->m_pAnimations[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaAnimationsInit(CSR_Collada_Animations* pColladaAnimations)
{
    if (!pColladaAnimations)
        return;

    pColladaAnimations->m_pAnimations    = 0;
    pColladaAnimations->m_AnimationCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaAnimationsRelease(CSR_Collada_Animations* pColladaAnimations)
{
    if (!pColladaAnimations)
        return;

    // free the animation array
    if (pColladaAnimations->m_pAnimations)
    {
        size_t i;

        // iterate through animations to free
        for (i = 0; i < pColladaAnimations->m_AnimationCount; ++i)
            csrColladaAnimationRelease(&pColladaAnimations->m_pAnimations[i]);

        // free the animations container
        free(pColladaAnimations->m_pAnimations);
    }
}
//---------------------------------------------------------------------------
int csrColladaAnimationsRead(XMLNode* pNode, CSR_Collada_Animations* pColladaAnimations)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaAnimations)
        return 0;

    // iterate through children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Animation_Tag) &&
            memcmp(pChild->tag, M_Collada_Animation_Tag, len) == 0)
        {
            const size_t index = pColladaAnimations->m_AnimationCount;

            // add a new animation in the array
            CSR_Collada_Animation* pAnimations =
                    (CSR_Collada_Animation*)csrMemoryAlloc(pColladaAnimations->m_pAnimations,
                                                           sizeof(CSR_Collada_Animation),
                                                           pColladaAnimations->m_AnimationCount + 1);

            // succeeded?
            if (!pAnimations)
                return 0;

            // set new animation array in the controllers
            pColladaAnimations->m_pAnimations = pAnimations;
            ++pColladaAnimations->m_AnimationCount;

            // initialize newly added animation
            csrColladaAnimationInit(&pColladaAnimations->m_pAnimations[index]);

            // read it
            if (!csrColladaAnimationRead(pChild, &pColladaAnimations->m_pAnimations[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaSkeletonInit(CSR_Collada_Skeleton* pColladaSkeleton)
{
    if (!pColladaSkeleton)
        return;

    pColladaSkeleton->m_pId = 0;
}
//---------------------------------------------------------------------------
void csrColladaSkeletonRelease(CSR_Collada_Skeleton* pColladaSkeleton)
{
    // no collada skeleton to release?
    if (!pColladaSkeleton)
        return;

    // free the identifier
    if (pColladaSkeleton->m_pId)
        free(pColladaSkeleton->m_pId);
}
//---------------------------------------------------------------------------
int csrColladaSkeletonRead(XMLNode* pNode, CSR_Collada_Skeleton* pColladaSkeleton)
{
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaSkeleton)
        return 0;

    len = strlen(pNode->text);

    if (!len)
        return 0;

    // create new identifier text
    pColladaSkeleton->m_pId = (char*)malloc((len + 1) * sizeof(char));

    // succeeded?
    if (!pColladaSkeleton->m_pId)
        return 0;

    // read the skeleton identifier
    memcpy(pColladaSkeleton->m_pId, pNode->text, len);
    pColladaSkeleton->m_pId[len] = 0x0;

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaInstanceCtrlInit(CSR_Collada_Instance_Controller* pColladaInstCtrl)
{
    if (!pColladaInstCtrl)
        return;

    pColladaInstCtrl->m_pUrl          = 0;
    pColladaInstCtrl->m_pSkeletons    = 0;
    pColladaInstCtrl->m_SkeletonCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaInstanceCtrlRelease(CSR_Collada_Instance_Controller* pColladaInstCtrl)
{
    // no collada instance controller to release?
    if (!pColladaInstCtrl)
        return;

    // free the identifier
    if (pColladaInstCtrl->m_pUrl)
        free(pColladaInstCtrl->m_pUrl);

    // free the skeleton array
    if (pColladaInstCtrl->m_pSkeletons)
    {
        size_t i;

        for (i = 0; i < pColladaInstCtrl->m_SkeletonCount; ++i)
            csrColladaSkeletonRelease(&pColladaInstCtrl->m_pSkeletons[i]);

        free(pColladaInstCtrl->m_pSkeletons);
    }
}
//---------------------------------------------------------------------------
int csrColladaInstanceCtrlRead(XMLNode* pNode, CSR_Collada_Instance_Controller* pColladaInstCtrl)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaInstCtrl)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Url_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Url_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaInstCtrl->m_pUrl);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Skeleton_Tag) &&
            memcmp(pChild->tag, M_Collada_Skeleton_Tag, len) == 0)
        {
            const size_t index = pColladaInstCtrl->m_SkeletonCount;

            // add a new skeleton in the array
            CSR_Collada_Skeleton* pSkeletons =
                    (CSR_Collada_Skeleton*)csrMemoryAlloc(pColladaInstCtrl->m_pSkeletons,
                                                          sizeof(CSR_Collada_Skeleton),
                                                          pColladaInstCtrl->m_SkeletonCount + 1);

            // succeeded?
            if (!pSkeletons)
                return 0;

            // set new skeleton array in the accessor
            pColladaInstCtrl->m_pSkeletons = pSkeletons;
            ++pColladaInstCtrl->m_SkeletonCount;

            // initialize newly added skeleton
            csrColladaSkeletonInit(&pColladaInstCtrl->m_pSkeletons[index]);

            // read it
            if (!csrColladaSkeletonRead(pChild, &pColladaInstCtrl->m_pSkeletons[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaNodeInit(CSR_Collada_Node* pColladaNode)
{
    if (!pColladaNode)
        return;

    pColladaNode->m_pName             = 0;
    pColladaNode->m_pId               = 0;
    pColladaNode->m_pSid              = 0;
    pColladaNode->m_pType             = 0;
    pColladaNode->m_pInstanceCtrls    = 0;
    pColladaNode->m_InstanceCtrlCount = 0;
    pColladaNode->m_pParent           = 0;
    pColladaNode->m_pNodes            = 0;
    pColladaNode->m_NodeCount         = 0;
    pColladaNode->m_pMatrices         = 0;
    pColladaNode->m_MatrixCount       = 0;
}
//---------------------------------------------------------------------------
void csrColladaNodeRelease(CSR_Collada_Node* pColladaNode)
{
    size_t i;

    if (!pColladaNode)
        return;

    // free the name
    if (pColladaNode->m_pName)
        free(pColladaNode->m_pName);

    // free the identifier
    if (pColladaNode->m_pId)
        free(pColladaNode->m_pId);

    // free the animation target identifier
    if (pColladaNode->m_pSid)
        free(pColladaNode->m_pSid);

    // free the type
    if (pColladaNode->m_pType)
        free(pColladaNode->m_pType);

    // free the matrix array
    if (pColladaNode->m_pMatrices)
    {
        for (i = 0; i < pColladaNode->m_MatrixCount; ++i)
            csrColladaMatrixRelease(&pColladaNode->m_pMatrices[i]);

        free(pColladaNode->m_pMatrices);
    }

    // free the instance controller array
    if (pColladaNode->m_pInstanceCtrls)
    {
        for (i = 0; i < pColladaNode->m_InstanceCtrlCount; ++i)
            csrColladaInstanceCtrlRelease(&pColladaNode->m_pInstanceCtrls[i]);

        free(pColladaNode->m_pInstanceCtrls);
    }

    // free the children nodes
    if (pColladaNode->m_pNodes)
    {
        // iterate through meshes to free
        for (i = 0; i < pColladaNode->m_NodeCount; ++i)
            csrColladaNodeRelease(&pColladaNode->m_pNodes[i]);

        // free the nodes container
        free(pColladaNode->m_pNodes);
    }
}
//---------------------------------------------------------------------------
int csrColladaNodeRead(XMLNode* pNode, CSR_Collada_Node* pParent, CSR_Collada_Node* pColladaNode)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaNode)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_Name_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Name_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaNode->m_pName);
            else
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaNode->m_pId);
            else
            if (len == strlen(M_Collada_SID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_SID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaNode->m_pSid);
            else
            if (len == strlen(M_Collada_Type_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Type_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaNode->m_pType);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Node_Tag) &&
            memcmp(pChild->tag, M_Collada_Node_Tag, len) == 0)
        {
            const size_t index = pColladaNode->m_NodeCount;

            // add a new node in the array
            CSR_Collada_Node* pNodes =
                    (CSR_Collada_Node*)csrMemoryAlloc(pColladaNode->m_pNodes,
                                                      sizeof(CSR_Collada_Node),
                                                      pColladaNode->m_NodeCount + 1);

            // succeeded?
            if (!pNodes)
                return 0;

            // set new mesh array in the geometry
            pColladaNode->m_pNodes = pNodes;
            ++pColladaNode->m_NodeCount;

            // initialize newly added mesh
            csrColladaNodeInit(&pColladaNode->m_pNodes[index]);

            // read it
            if (!csrColladaNodeRead(pChild, pColladaNode, &pColladaNode->m_pNodes[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Matrix_Tag) &&
            memcmp(pChild->tag, M_Collada_Matrix_Tag, len) == 0)
        {
            const size_t index = pColladaNode->m_MatrixCount;

            // add a new matrix in the array
            CSR_Collada_Matrix* pMatrices =
                    (CSR_Collada_Matrix*)csrMemoryAlloc(pColladaNode->m_pMatrices,
                                                        sizeof(CSR_Collada_Matrix),
                                                        pColladaNode->m_MatrixCount + 1);

            // succeeded?
            if (!pMatrices)
                return 0;

            // set new matrix array in the node
            pColladaNode->m_pMatrices = pMatrices;
            ++pColladaNode->m_MatrixCount;

            // initialize newly added matrix
            csrColladaMatrixInit(&pColladaNode->m_pMatrices[index]);

            // read it
            if (!csrColladaMatrixRead(pChild, &pColladaNode->m_pMatrices[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Instance_Ctrl_Tag) &&
            memcmp(pChild->tag, M_Collada_Instance_Ctrl_Tag, len) == 0)
        {
            const size_t index = pColladaNode->m_InstanceCtrlCount;

            // add a new instance controller in the array
            CSR_Collada_Instance_Controller* pInstanceCtrls =
                    (CSR_Collada_Instance_Controller*)csrMemoryAlloc(pColladaNode->m_pInstanceCtrls,
                                                                     sizeof(CSR_Collada_Instance_Controller),
                                                                     pColladaNode->m_InstanceCtrlCount + 1);

            // succeeded?
            if (!pInstanceCtrls)
                return 0;

            // set new instance controller array in the node
            pColladaNode->m_pInstanceCtrls = pInstanceCtrls;
            ++pColladaNode->m_InstanceCtrlCount;

            // initialize newly added instance controller
            csrColladaInstanceCtrlInit(&pColladaNode->m_pInstanceCtrls[index]);

            // read it
            if (!csrColladaInstanceCtrlRead(pChild, &pColladaNode->m_pInstanceCtrls[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaNodeFindSkeletons(CSR_Collada_Node*       pColladaNode,
                                 CSR_Collada_Skeleton*** pSkeletons,
                                 char***                 pUrls,
                                 size_t*                 pCount)
{
    size_t i;
    size_t j;

    if (!pColladaNode)
        return;

    if (!pSkeletons)
        return;

    if (!pUrls)
        return;

    if (!pCount)
        return;

    // iterate through instance controllers
    for (i = 0; i < pColladaNode->m_InstanceCtrlCount; ++i)
    {
        // instance controller contains an url?
        if (!pColladaNode->m_pInstanceCtrls[i].m_pUrl)
            continue;

        // iterate through skeletons
        for (j = 0; j < pColladaNode->m_pInstanceCtrls[i].m_SkeletonCount; ++j)
        {
            CSR_Collada_Skeleton** pSkeletonArray;
            char**                 pUrlArray;
            const size_t           index = *pCount;

            // add a new skeleton in the array
            pSkeletonArray =
                    (CSR_Collada_Skeleton**)csrMemoryAlloc(*pSkeletons,
                                                            sizeof(CSR_Collada_Skeleton*),
                                                            index + 1);

            // succeeded?
            if (!pSkeletonArray)
                return;

            *pSkeletons          = pSkeletonArray;
            (*pSkeletons)[index] = &pColladaNode->m_pInstanceCtrls[i].m_pSkeletons[j];

            // add a new url in the array
            pUrlArray = (char**)csrMemoryAlloc(*pUrls, sizeof(char*), index + 1);

            // succeeded?
            if (!pUrlArray)
                return;

            *pUrls          = pUrlArray;
            (*pUrls)[index] = pColladaNode->m_pInstanceCtrls[i].m_pUrl;

            ++(*pCount);
        }
    }

    // search in the node children
    for (i = 0; i < pColladaNode->m_NodeCount; ++i)
        csrColladaNodeFindSkeletons(&pColladaNode->m_pNodes[i],
                                     pSkeletons,
                                     pUrls,
                                     pCount);
}
//---------------------------------------------------------------------------
void csrColladaNodeFindRootBones(CSR_Collada_Node*   pColladaNode,
                                 const char*         pID,
                                 CSR_Collada_Node*** pRootBones,
                                 size_t*             pCount)
{
    size_t i;
    size_t len;

    if (!pID)
        return;

    if (!pRootBones)
        return;

    if (!pColladaNode)
        return;

    if (!pCount)
        return;

    len = strlen(pID);

    // found the matching bone node?
    if (len > 1 && pID[0] == '#' && memcmp(&pID[1], pColladaNode->m_pId, len - 1) == 0)
    {
        CSR_Collada_Node** pRootBoneArray;
        const size_t       index = *pCount;

        // add a new root bone in the array
        pRootBoneArray =
                (CSR_Collada_Node**)csrMemoryAlloc(*pRootBones,
                                                    sizeof(CSR_Collada_Node*),
                                                    index + 1);

        // succeeded?
        if (!pRootBoneArray)
            return;

        *pRootBones          = pRootBoneArray;
        (*pRootBones)[index] = pColladaNode;

        ++(*pCount);
        return;
    }

    // search in the node children
    for (i = 0; i < pColladaNode->m_NodeCount; ++i)
        csrColladaNodeFindRootBones(&pColladaNode->m_pNodes[i], pID, pRootBones, pCount);
}
//---------------------------------------------------------------------------
void csrColladaVisualSceneInit(CSR_Collada_Visual_Scene* pColladaVisualScene)
{
    if (!pColladaVisualScene)
        return;

    pColladaVisualScene->m_pId       = 0;
    pColladaVisualScene->m_pName     = 0;
    pColladaVisualScene->m_pNodes    = 0;
    pColladaVisualScene->m_NodeCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaVisualSceneRelease(CSR_Collada_Visual_Scene* pColladaVisualScene)
{
    if (!pColladaVisualScene)
        return;

    // free the identifier
    if (pColladaVisualScene->m_pId)
        free(pColladaVisualScene->m_pId);

    // free the name
    if (pColladaVisualScene->m_pName)
        free(pColladaVisualScene->m_pName);

    // free the mesh array
    if (pColladaVisualScene->m_pNodes)
    {
        size_t i;

        // iterate through meshes to free
        for (i = 0; i < pColladaVisualScene->m_NodeCount; ++i)
            csrColladaNodeRelease(&pColladaVisualScene->m_pNodes[i]);

        // free the meshes container
        free(pColladaVisualScene->m_pNodes);
    }
}
//---------------------------------------------------------------------------
int csrColladaVisualSceneRead(XMLNode* pNode, CSR_Collada_Visual_Scene* pColladaVisualScene)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaVisualScene)
        return 0;

    // node contains attributes?
    if ((size_t)pNode->n_attributes)
    {
        size_t        index = 0;
        XMLAttribute* pAttributes;

        // get attribute list
        pAttributes = pNode->attributes;

        // found it?
        if (!pAttributes)
            return 0;

        // iterate through attributes
        for (i = 0; i < (size_t)pNode->n_attributes; ++i)
        {
            // measure attribute name length
            len = strlen(pAttributes[i].name);

            // search for attribute to read
            if (len == strlen(M_Collada_ID_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_ID_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaVisualScene->m_pId);
            else
            if (len == strlen(M_Collada_Name_Attribute) &&
                memcmp(pAttributes[i].name, M_Collada_Name_Attribute, len) == 0)
                csrColladaReadAttribValue(&pAttributes[i], &pColladaVisualScene->m_pName);
        }
    }

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Node_Tag) &&
            memcmp(pChild->tag, M_Collada_Node_Tag, len) == 0)
        {
            const size_t index = pColladaVisualScene->m_NodeCount;

            // add a new node in the array
            CSR_Collada_Node* pNodes =
                    (CSR_Collada_Node*)csrMemoryAlloc(pColladaVisualScene->m_pNodes,
                                                      sizeof(CSR_Collada_Node),
                                                      pColladaVisualScene->m_NodeCount + 1);

            // succeeded?
            if (!pNodes)
                return 0;

            // set new mesh array in the geometry
            pColladaVisualScene->m_pNodes = pNodes;
            ++pColladaVisualScene->m_NodeCount;

            // initialize newly added mesh
            csrColladaNodeInit(&pColladaVisualScene->m_pNodes[index]);

            // read it
            if (!csrColladaNodeRead(pChild, 0, &pColladaVisualScene->m_pNodes[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaVisualScenesInit(CSR_Collada_Visual_Scenes* pColladaVisualScenes)
{
    if (!pColladaVisualScenes)
        return;

    pColladaVisualScenes->m_pVisualScenes    = 0;
    pColladaVisualScenes->m_VisualSceneCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaVisualScenesRelease(CSR_Collada_Visual_Scenes* pColladaVisualScenes)
{
    if (!pColladaVisualScenes)
        return;

    // free the mesh array
    if (pColladaVisualScenes->m_pVisualScenes)
    {
        size_t i;

        // iterate through meshes to free
        for (i = 0; i < pColladaVisualScenes->m_VisualSceneCount; ++i)
            csrColladaVisualSceneRelease(&pColladaVisualScenes->m_pVisualScenes[i]);

        // free the meshes container
        free(pColladaVisualScenes->m_pVisualScenes);
    }
}
//---------------------------------------------------------------------------
int csrColladaVisualScenesRead(XMLNode* pNode, CSR_Collada_Visual_Scenes* pColladaVisualScenes)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaVisualScenes)
        return 0;

    // iterate through node children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
            return 0;

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Visual_Scene_Tag) &&
            memcmp(pChild->tag, M_Collada_Visual_Scene_Tag, len) == 0)
        {
            const size_t index = pColladaVisualScenes->m_VisualSceneCount;

            // add a new visual scene in the array
            CSR_Collada_Visual_Scene* pVisualScenes =
                    (CSR_Collada_Visual_Scene*)csrMemoryAlloc(pColladaVisualScenes->m_pVisualScenes,
                                                              sizeof(CSR_Collada_Visual_Scene),
                                                              pColladaVisualScenes->m_VisualSceneCount + 1);

            // succeeded?
            if (!pVisualScenes)
                return 0;

            // set new visual scene in the visual scenes
            pColladaVisualScenes->m_pVisualScenes = pVisualScenes;
            ++pColladaVisualScenes->m_VisualSceneCount;

            // initialize newly added visual scene
            csrColladaVisualSceneInit(&pColladaVisualScenes->m_pVisualScenes[index]);

            // read it
            if (!csrColladaVisualSceneRead(pChild, &pColladaVisualScenes->m_pVisualScenes[index]))
                return 0;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrGetLinkedGeometry(const CSR_Collada_Geometries* pGeometries, const char* pId, CSR_Collada_Geometry** pR)
{
    size_t i;

    if (!pGeometries)
        return 0;

    if (!pId)
        return 0;

    if (!pR)
        return 0;

    for (i = 0; i < pGeometries->m_GeometryCount; ++i)
    {
        const size_t srcLen = strlen(pGeometries->m_pGeometries[i].m_pId);
        const size_t dstLen = strlen(pId);

        if (srcLen == (dstLen - 1) &&
            memcmp(pGeometries->m_pGeometries[i].m_pId, pId + 1, srcLen) == 0)
        {
            *pR = &pGeometries->m_pGeometries[i];
            return 1;
        }
    }

    return 0;
}
//---------------------------------------------------------------------------
int csrColladaGetSourceFromSkin(char* pId, CSR_Collada_Skin* pSkin, CSR_Collada_Source** pR)
{
    size_t i;
    size_t len;

    if (!pId)
        return 0;

    if (!pSkin)
        return 0;

    if (!pR)
        return 0;

    len = strlen(pId);

    // iterate through mesh sources
    for (i = 0; i < pSkin->m_SourceCount; ++i)
    {
        // measure source id length
        const size_t srcLen = strlen(pSkin->m_pSources[i].m_pId);

        // found the source?
        if ((len - 1) == srcLen && memcmp(pSkin->m_pSources[i].m_pId, pId + 1, srcLen) == 0)
        {
            *pR = &pSkin->m_pSources[i];
            return 1;
        }
    }

    return 0;
}
//---------------------------------------------------------------------------
int csrColladaBuildSkeleton(CSR_Collada_Node* pNode, CSR_Bone* pBone)
{
    size_t i;

    if (!pNode)
        return 0;

    if (!pBone)
        return 0;

    // copy bone matrix from collada node. For now assume that node contains
    // only one matrix (it should be the case for simple collada models exported
    // from Blender)
    if (pNode->m_MatrixCount)
        pBone->m_Matrix = pNode->m_pMatrices[0].m_Matrix;

    // copy bone name from collada node
    if (pNode->m_pSid)
    {
        const size_t idLength = strlen(pNode->m_pSid);

        pBone->m_pName = (char*)malloc(idLength + 1);

        if (!pBone->m_pName)
            return 0;

        memcpy(pBone->m_pName, pNode->m_pSid, idLength);
        pBone->m_pName[idLength] = 0x0;
    }

    // copy bone id from collada node (used to link with animations)
    if (pNode->m_pId)
    {
        const size_t idLength = strlen(pNode->m_pId);

        pBone->m_pCustomData = (char*)malloc(idLength + 1);

        if (!pBone->m_pCustomData)
            return 0;

        memcpy((char*)pBone->m_pCustomData, pNode->m_pId, idLength);
        ((char*)pBone->m_pCustomData)[idLength] = 0x0;
    }

    // is a leaf?
    if (pNode->m_NodeCount)
    {
        // allocate memory for children bones
        CSR_Bone* pChildren = (CSR_Bone*)csrMemoryAlloc(pBone->m_pChildren,
                                                        sizeof(CSR_Bone),
                                                        pNode->m_NodeCount);

        // succeeded?
        if (!pChildren)
            return 0;

        // update children bone array
        pBone->m_pChildren     = pChildren;
        pBone->m_ChildrenCount = pNode->m_NodeCount;

        // iterate through children bones to create
        for (i = 0; i < pNode->m_NodeCount; ++i)
        {
            // get next child bone
            CSR_Bone* pChild = &pBone->m_pChildren[i];

            // initialize it
            csrBoneInit(pChild);

            // set its parent
            pChild->m_pParent = pBone;

            // populate it
            csrColladaBuildSkeleton(&pNode->m_pNodes[i], pChild);
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrColladaGetInverseBindMatrix(CSR_Collada_Source* pBindPoses, size_t jointIndex, CSR_Matrix4* pMatrix)
{
    size_t i;
    size_t stride = 0;
    size_t count  = 0;
    size_t offset = 0;

    if (!pBindPoses)
        return 0;

    if (!pMatrix)
        return 0;

    if (!pBindPoses->m_pTechCommon)
        return 0;

    if (!pBindPoses->m_pTechCommon->m_pAccessor)
        return 0;

    // get the matrix stride and count
    stride = pBindPoses->m_pTechCommon->m_pAccessor->m_Stride;
    count  = pBindPoses->m_pTechCommon->m_pAccessor->m_Count;

    if (jointIndex >= count)
        return 0;

    offset = jointIndex * stride;

    // get the matrix
    for (i = 0; i < stride; ++i)
        pMatrix->m_Table[i % 4][i / 4] =
                pBindPoses->m_pFloatArray->m_pData[offset + i];

    return 1;
}
//---------------------------------------------------------------------------
int csrColladaBuildWeightsFromSkeleton(CSR_Bone*               pBone,
                                       size_t                  meshIndex,
                                       CSR_Skin_Weights_Group* pSkinWeightsGroup,
                                       CSR_Collada_Source*     pJoints,
                                       CSR_Collada_Source*     pBindMatrices)
{
    size_t            i;
    size_t            index;
    size_t            len;
    CSR_Skin_Weights* pSkinWeights = 0;

    if (!pBone)
        return 0;

    if (!pSkinWeightsGroup)
        return 0;

    if (!pJoints)
        return 0;

    if (!pBindMatrices)
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

    // iterate through bone names
    for (i = 0; i < pJoints->m_pNameArray->m_Count; ++i)
        // found the bone name in the joint name array?
        if (strcmp(pJoints->m_pNameArray->m_pData[i],
                   pSkinWeightsGroup->m_pSkinWeights[index].m_pBoneName) == 0)
        {
            // set the inverse bind matrix
            if (!csrColladaGetInverseBindMatrix(pBindMatrices,
                                                i,
                                               &pSkinWeightsGroup->m_pSkinWeights[index].m_Matrix))
                return 0;

            break;
        }

    // iterate through children bones to create
    for (i = 0; i < pBone->m_ChildrenCount; ++i)
        // build from next child bone
        if (!csrColladaBuildWeightsFromSkeleton(&pBone->m_pChildren[i],
                                                 meshIndex,
                                                 pSkinWeightsGroup,
                                                 pJoints,
                                                 pBindMatrices))
            return 0;

    return 1;
}
//---------------------------------------------------------------------------
int csrColladaMeshWeightsBuild(CSR_Collada_Geometry*   pGeometry,
                               CSR_Collada_Controller* pController,
                               size_t                  meshIndex,
                               CSR_Collada*            pCollada)
{
    size_t                       i;
    size_t                       j;
    size_t                       k;
    size_t                       index;
    size_t                       offset            = 0;
    size_t                       jointPos          = 0;
    size_t                       weightPos         = 0;
    size_t                       inputCount        = 0;
    size_t                       stride            = 0;
    CSR_Collada_Skin*            pSkin             = 0;
    CSR_Collada_Source*          pJoints           = 0;
    CSR_Collada_Source*          pBindMatrices     = 0;
    CSR_Collada_Source*          pWeights          = 0;
    CSR_Skin_Weights_Group*      pSkinWeightsGroup = 0;
    CSR_Skin_Weights*            pSkinWeights      = 0;
    float*                       pWeightsArray     = 0;
    CSR_Skin_Weight_Index_Table* pIndexTable       = 0;

    if (!pGeometry)
        return 0;

    if (!pController)
        return 0;

    if (!pCollada)
        return 0;

    pSkin = pController->m_pSkin;

    // search for joint sources
    for (i = 0; i < pSkin->m_pJoints->m_InputCount; ++i)
        for (j = 0; j < pSkin->m_SourceCount; ++j)
        {
            // measure joint source id and source id
            const size_t inputLen = strlen(pSkin->m_pJoints->m_pInputs[i].m_pSource);
            const size_t srcLen   = strlen(pSkin->m_pSources[j].m_pId);

            // joint source id is matching the source id?
            if (inputLen == (srcLen + 1) &&
                strcmp(pSkin->m_pJoints->m_pInputs[i].m_pSource + 1, pSkin->m_pSources[j].m_pId) == 0)
                // search for semantic
                if (strcmp(pSkin->m_pJoints->m_pInputs[i].m_pSemantic, M_Collada_Semantic_Joint) == 0)
                    pJoints = &pSkin->m_pSources[j];
                else
                if (strcmp(pSkin->m_pJoints->m_pInputs[i].m_pSemantic, M_Collada_Semantic_Inv_Bind_Matrix) == 0)
                    pBindMatrices = &pSkin->m_pSources[j];
        }

    // search for vertex weights sources
    for (i = 0; i < pSkin->m_pVertexWeights->m_InputCount; ++i)
    {
        const size_t len = strlen(pSkin->m_pVertexWeights->m_pInputs[i].m_pSemantic);

        // search for input containing the source identifier
        if (len == strlen(M_Collada_Semantic_Joint) &&
            memcmp(pSkin->m_pVertexWeights->m_pInputs[i].m_pSemantic, M_Collada_Semantic_Joint, len) == 0)
        {
            // get the joint index position in the vertex to bone array
            jointPos = i;

            // get source
            if (!pJoints)
                if (!csrColladaGetSourceFromSkin(pSkin->m_pVertexWeights->m_pInputs[i].m_pSource,
                                                 pSkin,
                                                &pJoints))
                    return 0;
        }
        else
        if (len == strlen(M_Collada_Semantic_Weight) &&
            memcmp(pSkin->m_pVertexWeights->m_pInputs[i].m_pSemantic, M_Collada_Semantic_Weight, len) == 0)
        {
            // get the weight index position in the vertex to bone array
            weightPos = i;

            // get source
            if (!csrColladaGetSourceFromSkin(pSkin->m_pVertexWeights->m_pInputs[i].m_pSource,
                                             pSkin,
                                            &pWeights))
                return 0;
        }
    }

    if (!pJoints)
        return 0;

    if (!pBindMatrices)
        return 0;

    if (!pWeights)
        return 0;

    pSkinWeightsGroup = &pCollada->m_pMeshWeights[meshIndex];

    // get the skin weights group to populate
    if (!pSkinWeightsGroup)
        return 0;

    // iterate through existing skeletons
    for (i = 0; i < pCollada->m_SkeletonCount; ++i)
        // populate the skin weights group
        if (!csrColladaBuildWeightsFromSkeleton(pCollada->m_pSkeletons[i].m_pRoot,
                                                meshIndex,
                                                pSkinWeightsGroup,
                                                pJoints,
                                                pBindMatrices))
            return 0;

    // iterate through weights count items
    for (i = 0; i < pSkin->m_pVertexWeights->m_pVertexToBoneCountArray->m_Count; ++i)
    {
        // get the weights count and calculate start offset in the vertex weights array
        const size_t count      = pSkin->m_pVertexWeights->m_pVertexToBoneCountArray->m_pData[i];
        const size_t baseOffset = offset * pSkin->m_pVertexWeights->m_InputCount;

        // iterate through vertex to weights pair
        for (j = 0; j < count; ++j)
        {
            // calculate the joint and weight index
            const size_t curOffset   = baseOffset + (j * pSkin->m_pVertexWeights->m_InputCount);
            const size_t jointIndex  = pSkin->m_pVertexWeights->m_pVertexToBoneArray->m_pData[curOffset + jointPos];
            const size_t weightIndex = pSkin->m_pVertexWeights->m_pVertexToBoneArray->m_pData[curOffset + weightPos];

            // todo -cFeature -oJean: Collada standard may use negative index,
            //                        but for now assume that this will not happen
            if (jointIndex < 0 || weightIndex < 0)
                return 0;

            // get the bone name for which the weights should be applied
            char* pBoneName = pJoints->m_pNameArray->m_pData[jointIndex];

            // found it?
            if (!pBoneName)
                return 0;

            // get the skin weights to populate
            for (k = 0; k < pCollada->m_pMeshWeights[meshIndex].m_Count; ++k)
                if (pCollada->m_pMeshWeights[meshIndex].m_pSkinWeights[k].m_pBoneName &&
                    strcmp(pCollada->m_pMeshWeights[meshIndex].m_pSkinWeights[k].m_pBoneName, pBoneName) == 0)
                {
                    pSkinWeights = &pCollada->m_pMeshWeights[meshIndex].m_pSkinWeights[k];
                    break;
                }

            // found it?
            if (!pSkinWeights)
                return 0;

            if (pSkinWeights->m_WeightCount != pSkinWeights->m_IndexTableCount)
                return 0;

            index = pSkinWeights->m_WeightCount;

            // add a new weights in the array
            pWeightsArray = (float*)csrMemoryAlloc(pSkinWeights->m_pWeights,
                                                   sizeof(float),
                                                   pSkinWeights->m_WeightCount + 1);

            // succeeded?
            if (!pWeightsArray)
                return 0;

            // set new weights array in the skin weights
            pSkinWeights->m_pWeights = pWeightsArray;
            ++pSkinWeights->m_WeightCount;

            // write the weight value
            pSkinWeights->m_pWeights[index] = pWeights->m_pFloatArray->m_pData[weightIndex];

            // add a new index table in the array
            pIndexTable =
                    (CSR_Skin_Weight_Index_Table*)csrMemoryAlloc(pSkinWeights->m_pIndexTable,
                                                                 sizeof(CSR_Skin_Weight_Index_Table),
                                                                 pSkinWeights->m_IndexTableCount + 1);

            // succeeded?
            if (!pIndexTable)
                return 0;

            // set new index table in the skin weights
            pSkinWeights->m_pIndexTable = pIndexTable;
            ++pSkinWeights->m_IndexTableCount;

            // initialize the skin weights table
            pSkinWeights->m_pIndexTable[index].m_Count = 0;
            pSkinWeights->m_pIndexTable[index].m_pData = 0;

            // todo -cFeature -oJean: Collada standard may use several triangles set, as well as
            //                        several triangles arrangement or many geometrical shapes.
            //                        For now assume that only one triangle set is used per mesh
            if (pGeometry->m_pMesh->m_TriangleCount != 1)
                return 0;

            inputCount = pGeometry->m_pMesh->m_pTriangles[0].m_InputCount;
            stride     = inputCount * 3;

            // iterate through source vertex indices
            for (k = 0; k < pGeometry->m_pMesh->m_pTriangles[0].m_Count * stride; k += inputCount)
            {
                      size_t  dataIndex;
                      size_t* pData;
                const size_t  vertexIndex = pGeometry->m_pMesh->m_pTriangles[0].m_pPrimitiveArray->m_pData[k];

                // found the vertex index influenced by weight?
                if (vertexIndex != i)
                    continue;

                // get the data index
                dataIndex = pSkinWeights->m_pIndexTable[index].m_Count;

                // add a new index table in the array
                pData = (size_t*)csrMemoryAlloc(pSkinWeights->m_pIndexTable[index].m_pData,
                                                sizeof(size_t),
                                                pSkinWeights->m_pIndexTable[index].m_Count + 1);

                // succeeded?
                if (!pData)
                    return 0;

                // set new index table in the skin weights
                pSkinWeights->m_pIndexTable[index].m_pData = pData;
                ++pSkinWeights->m_pIndexTable[index].m_Count;

                // set the mesh vertex index influenced by weight
                pSkinWeights->m_pIndexTable[index].m_pData[dataIndex] =
                        (k / inputCount) * pCollada->m_pMesh[meshIndex].m_pVB[0].m_Format.m_Stride;
            }
        }

        offset += count;
    }

    return 1;
}
//---------------------------------------------------------------------------
int csrColladaFindBone(char* pId, CSR_Bone* pBone, CSR_Bone** pFound)
{
    size_t i;

    if (!pId)
        return 0;

    if (!pBone)
        return 0;

    if (!pFound)
        return 0;

    if (strlen(pId) == strlen((char*)pBone->m_pCustomData) &&
        strcmp(pId, (char*)pBone->m_pCustomData) == 0)
    {
        *pFound = pBone;
        return 1;
    }

    // is a leaf?
    for (i = 0; i < pBone->m_ChildrenCount; ++i)
        if (csrColladaFindBone(pId, &pBone->m_pChildren[i], pFound))
            return 1;

    return 0;
}
//---------------------------------------------------------------------------
int csrColladaAnimationBuild(CSR_Collada_Animation* pAnimation,
                             CSR_AnimationSet_Bone* pAnimSetBone,
                             CSR_Collada*           pCollada)
{
    size_t              i;
    size_t              j;
    size_t              k;
    size_t              l;
    CSR_Animation_Bone* pAnimatedBones = 0;
    CSR_Animation_Bone* pAnimatedBone  = 0;

    if (!pAnimation)
        return 0;

    if (!pAnimSetBone)
        return 0;

    if (!pCollada)
        return 0;

    // reserve memory for boned animations
    pAnimatedBones =
            (CSR_Animation_Bone*)csrMemoryAlloc(pAnimSetBone->m_pAnimation,
                                                sizeof(CSR_Animation_Bone),
                                                pAnimation->m_AnimationCount);

    if (!pAnimatedBones)
        return 0;

    // set animations in the animation set
    pAnimSetBone->m_pAnimation = pAnimatedBones;
    pAnimSetBone->m_Count      = pAnimation->m_AnimationCount;

    // iterate through animations to populate
    for (i = 0; i < pAnimation->m_AnimationCount; ++i)
    {
        CSR_Collada_Source* pInput         = 0;
        CSR_Collada_Source* pOutput        = 0;
        CSR_Collada_Source* pInterpolation = 0;
        size_t              stride         = 0;
        size_t              keyCount       = 0;


        // iterate through animation children and find input, output and interpolation sources
        for (j = 0; j < pAnimation->m_pAnimations[i].m_SamplerCount; ++j)
            for (k = 0; k < pAnimation->m_pAnimations[i].m_pSamplers[j].m_InputCount; ++k)
                if (strcmp(pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSemantic,
                           M_Collada_Semantic_Input) == 0)
                    for (l = 0; l < pAnimation->m_pAnimations[i].m_SourceCount; ++l)
                    {
                        const size_t len = strlen(pAnimation->m_pAnimations[i].m_pSources[l].m_pId);

                        if ((len + 1) == strlen(pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSource) &&
                            strcmp(pAnimation->m_pAnimations[i].m_pSources[l].m_pId,
                                   pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSource + 1) == 0)
                            pInput = &pAnimation->m_pAnimations[i].m_pSources[l];
                    }
                else
                if (strcmp(pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSemantic,
                            M_Collada_Semantic_Output) == 0)
                    for (l = 0; l < pAnimation->m_pAnimations[i].m_SourceCount; ++l)
                    {
                        const size_t len = strlen(pAnimation->m_pAnimations[i].m_pSources[l].m_pId);

                        if ((len + 1) == strlen(pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSource) &&
                            strcmp(pAnimation->m_pAnimations[i].m_pSources[l].m_pId,
                                   pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSource + 1) == 0)
                            pOutput = &pAnimation->m_pAnimations[i].m_pSources[l];
                    }
                else
                if (strcmp(pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSemantic,
                            M_Collada_Semantic_Interpolation) == 0)
                    for (l = 0; l < pAnimation->m_pAnimations[i].m_SourceCount; ++l)
                    {
                        const size_t len = strlen(pAnimation->m_pAnimations[i].m_pSources[l].m_pId);

                        if ((len + 1) == strlen(pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSource) &&
                            strcmp(pAnimation->m_pAnimations[i].m_pSources[l].m_pId,
                                   pAnimation->m_pAnimations[i].m_pSamplers[j].m_pInputs[k].m_pSource + 1) == 0)
                            pInterpolation = &pAnimation->m_pAnimations[i].m_pSources[l];
                    }

        if (!pInput || !pOutput || !pInterpolation)
            return 0;

        // get animated bone to populate
        pAnimatedBone = &pAnimSetBone->m_pAnimation[i];

        // initialize it
        pAnimatedBone->m_pBone     = 0;
        pAnimatedBone->m_pBoneName = 0;
        pAnimatedBone->m_pKeys     = 0;
        pAnimatedBone->m_Count     = 0;

        // iterate through animation channels
        for (j = 0; j < pAnimation->m_pAnimations[i].m_ChannelCount; ++j)
        {
                  size_t targetEnd   = 0;
                  char*  pTargetName = 0;
                  int    found       = 0;
            const size_t targetLen   = strlen(pAnimation->m_pAnimations[i].m_pChannels[j].m_pTarget);

            // extract bone name from target link
            for (k = 0; k < targetLen; ++k)
                if (pAnimation->m_pAnimations[i].m_pChannels[j].m_pTarget[k] == '/')
                {
                    targetEnd = k;
                    break;
                }

            if (!targetEnd)
                return 0;

            // reserve memory for target bone name
            pTargetName = (char*)malloc((targetEnd + 1) * sizeof(char));

            if (!pTargetName)
                return 0;

            // get target bone name
            memcpy(pTargetName, pAnimation->m_pAnimations[i].m_pChannels[j].m_pTarget, targetEnd);
            pTargetName[targetEnd] = 0x0;

            // iterate through existing skeletons
            for (k = 0; k < pCollada->m_SkeletonCount; ++k)
            {
                // search for matching bone
                if (csrColladaFindBone(pTargetName,
                                       pCollada->m_pSkeletons[k].m_pRoot,
                                       &pAnimatedBone->m_pBone))
                {
                    // found it?
                    if (!pAnimatedBone->m_pBone)
                    {
                        free(pTargetName);
                        return 0;
                    }

                    // set target bone name
                    pAnimatedBone->m_pBoneName = pTargetName;

                    found = 1;
                    break;
                }
            }

            if (found)
                break;

            free(pTargetName);
        }

        // found matching bone?
        if (!pAnimatedBone->m_pBone)
            return 0;

        // get the stride and accessor from tech common info
        if (pOutput->m_pTechCommon && pOutput->m_pTechCommon->m_pAccessor)
        {
            stride   = pOutput->m_pTechCommon->m_pAccessor->m_Stride;
            keyCount = pOutput->m_pTechCommon->m_pAccessor->m_Count;
        }

        if (!stride)
            return 0;

        if (!keyCount)
            return 0;

        // for now, only animated matrix list is supported
        if (stride != 16)
            return 0;

        // allocate memory for animation keys array
        pAnimatedBone->m_pKeys = (CSR_AnimationKeys*)malloc(sizeof(CSR_AnimationKeys));
        pAnimatedBone->m_Count = 1;

        if (!pAnimatedBone->m_pKeys)
            return 0;

        // allocate memory for animation keys
        pAnimatedBone->m_pKeys->m_Type       = CSR_KT_Matrix;
        pAnimatedBone->m_pKeys->m_pKey       = (CSR_AnimationKey*)malloc(keyCount * sizeof(CSR_AnimationKey));
        pAnimatedBone->m_pKeys->m_Count      = keyCount;
        pAnimatedBone->m_pKeys->m_ColOverRow = 1;

        // iterate through keys to populate
        for (j = 0; j < keyCount; ++j)
        {
            // set key frame and count, and reserve memory for data
            pAnimatedBone->m_pKeys->m_pKey[j].m_Frame   = j;
            pAnimatedBone->m_pKeys->m_pKey[j].m_Count   = stride;
            pAnimatedBone->m_pKeys->m_pKey[j].m_pValues = (float*)malloc(stride * sizeof(float));

            // copy key matrix data
            memcpy(pAnimatedBone->m_pKeys->m_pKey[j].m_pValues,
                  &pOutput->m_pFloatArray->m_pData[j * stride],
                   stride * sizeof(float));
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaGeometryLibraryRelease(CSR_Collada_Geometries* pGeometryLibrary, size_t count)
{
    size_t i;

    if (!pGeometryLibrary)
        return;

    for (i = 0; i < count; ++i)
        csrColladaGeometriesRelease(&pGeometryLibrary[i]);

    free(pGeometryLibrary);
}
//---------------------------------------------------------------------------
void csrColladaControllerLibraryRelease(CSR_Collada_Controllers* pControllerLibrary, size_t count)
{
    size_t i;

    if (!pControllerLibrary)
        return;

    for (i = 0; i < count; ++i)
        csrColladaControllersRelease(&pControllerLibrary[i]);

    free(pControllerLibrary);
}
//---------------------------------------------------------------------------
void csrColladaAnimationLibraryRelease(CSR_Collada_Animations* pAnimationLibrary, size_t count)
{
    size_t i;

    if (!pAnimationLibrary)
        return;

    for (i = 0; i < count; ++i)
        csrColladaAnimationsRelease(&pAnimationLibrary[i]);

    free(pAnimationLibrary);
}
//---------------------------------------------------------------------------
void csrColladaVisualSceneLibraryRelease(CSR_Collada_Visual_Scenes* pVisualScenesLibrary, size_t count)
{
    size_t i;

    if (!pVisualScenesLibrary)
        return;

    for (i = 0; i < count; ++i)
        csrColladaVisualScenesRelease(&pVisualScenesLibrary[i]);

    free(pVisualScenesLibrary);
}
//---------------------------------------------------------------------------
void csrColladaNodeSetParent(CSR_Collada_Node* pNode, CSR_Collada_Node* pParent)
{
    size_t i;

    if (!pNode)
        return;

    pNode->m_pParent = pParent;

    for (i = 0; i < pNode->m_NodeCount; ++i)
        csrColladaNodeSetParent(&pNode->m_pNodes[i], pNode);
}
//---------------------------------------------------------------------------
int csrColladaParse(const CSR_Buffer*           pBuffer,
                    const CSR_VertexFormat*     pVertFormat,
                    const CSR_VertexCulling*    pVertCulling,
                    const CSR_Material*         pMaterial,
                          CSR_Collada*          pCollada,
                    const CSR_fOnGetVertexColor fOnGetVertexColor)
{
    XMLDoc                     doc;
    size_t                     i;
    size_t                     j;
    size_t                     k;
    size_t                     l;
    size_t                     index            = 0;
    size_t                     len              = 0;
    size_t                     count            = 0;
    size_t                     geometryCount    = 0;
    size_t                     controllerCount  = 0;
    size_t                     animationCount   = 0;
    size_t                     visualSceneCount = 0;
    CSR_Collada_Geometries*    pGeometries      = 0;
    CSR_Collada_Controllers*   pControllers     = 0;
    CSR_Collada_Animations*    pAnimations      = 0;
    CSR_Collada_Visual_Scenes* pVisualScenes    = 0;
    CSR_Mesh*                  pMesh            = 0;
    CSR_Skin_Weights_Group*    pMeshWeights     = 0;

    if (!pBuffer)
        return 0;

    if (!pCollada)
        return 0;

    // initialize xml document
    XMLDoc_init(&doc);

    // open xml document, name it as collada_file for logging and events
    XMLDoc_parse_buffer_DOM(pBuffer->m_pData, "collada_file", &doc);

    // get root node
    XMLNode* pNode = XMLDoc_root(&doc);

    // measure tag name length
    len = strlen(pNode->tag);

    // is a collada file?
    if (len != strlen(M_Collada_Root_Tag) ||
        memcmp(pNode->tag, M_Collada_Root_Tag, len) != 0)
    {
        // release xml document
        XMLDoc_free(&doc);
        return 0;
    }

    // iterate through root children
    for (i = 0; i < (size_t)pNode->n_children; ++i)
    {
        // get next child
        XMLNode* pChild = pNode->children[i];

        if (!pChild)
        {
            // release xml document
            XMLDoc_free(&doc);
            return 0;
        }

        // measure tag name length
        len = strlen(pChild->tag);

        // search for tag to read
        if (len == strlen(M_Collada_Geometries_Tag) &&
            memcmp(pChild->tag, M_Collada_Geometries_Tag, len) == 0)
        {
            CSR_Collada_Geometries* pNewGeometries;
            index = geometryCount;

            // add new geometries container
            pNewGeometries =
                    (CSR_Collada_Geometries*)csrMemoryAlloc(pGeometries,
                                                            sizeof(CSR_Collada_Geometries),
                                                            geometryCount + 1);

            // succeeded?
            if (!pNewGeometries)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);
                return 0;
            }

            pGeometries = pNewGeometries;
            ++geometryCount;

            // initialize geometries container
            csrColladaGeometriesInit(&pGeometries[index]);

            // read geometry library
            if (!csrColladaGeometriesRead(pChild, &pGeometries[index]))
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);

                return 0;
            }
        }
        else
        if (len == strlen(M_Collada_Controllers_Tag) &&
            memcmp(pChild->tag, M_Collada_Controllers_Tag, len) == 0)
        {
            CSR_Collada_Controllers* pNewControllers;
            index = controllerCount;

            // add new controllers container
            pNewControllers =
                    (CSR_Collada_Controllers*)csrMemoryAlloc(pControllers,
                                                             sizeof(CSR_Collada_Controllers),
                                                             controllerCount + 1);

            // succeeded?
            if (!pNewControllers)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);
                return 0;
            }

            pControllers = pNewControllers;
            ++controllerCount;

            // initialize controllers container
            csrColladaControllersInit(&pControllers[index]);

            // read controller library
            if (!csrColladaControllersRead(pChild, &pControllers[index]))
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);

                return 0;
            }
        }
        else
        if (len == strlen(M_Collada_Animations_Tag) &&
            memcmp(pChild->tag, M_Collada_Animations_Tag, len) == 0)
        {
            CSR_Collada_Animations* pNewAnimations;
            index = animationCount;

            // add new animations container
            pNewAnimations =
                    (CSR_Collada_Animations*)csrMemoryAlloc(pAnimations,
                                                            sizeof(CSR_Collada_Animations),
                                                            animationCount + 1);

            // succeeded?
            if (!pNewAnimations)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);
                return 0;
            }

            pAnimations = pNewAnimations;
            ++animationCount;

            // initialize animations container
            csrColladaAnimationsInit(&pAnimations[index]);

            // read animation library
            if (!csrColladaAnimationsRead(pChild, &pAnimations[index]))
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);

                return 0;
            }
        }
        else
        if (len == strlen(M_Collada_Visual_Scenes_Tag) &&
            memcmp(pChild->tag, M_Collada_Visual_Scenes_Tag, len) == 0)
        {
            CSR_Collada_Visual_Scenes* pNewVisualScenes;
            index = visualSceneCount;

            // add new visual scenes container
            pNewVisualScenes =
                    (CSR_Collada_Visual_Scenes*)csrMemoryAlloc(pVisualScenes,
                                                               sizeof(CSR_Collada_Visual_Scenes),
                                                               visualSceneCount + 1);

            // succeeded?
            if (!pNewVisualScenes)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);
                return 0;
            }

            pVisualScenes = pNewVisualScenes;
            ++visualSceneCount;

            // initialize visual scenes container
            csrColladaVisualScenesInit(&pVisualScenes[index]);

            // read visual scenes library
            if (!csrColladaVisualScenesRead(pChild, &pVisualScenes[index]))
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                // release xml document
                XMLDoc_free(&doc);

                return 0;
            }
        }
    }

    // allocate parent node in each visual scene nodes
    if (pVisualScenes)
        for (i = 0; i < visualSceneCount; ++i)
            for (j = 0; j < pVisualScenes[i].m_VisualSceneCount; ++j)
                for (k = 0; k < pVisualScenes[i].m_pVisualScenes[j].m_NodeCount; ++k)
                    csrColladaNodeSetParent(&pVisualScenes[i].m_pVisualScenes[j].m_pNodes[k], 0);

    // release xml document
    XMLDoc_free(&doc);

    // iterate through geometry libraries
    for (i = 0; i < geometryCount; ++i)
        // iterate through geometries
        for (j = 0; j < pGeometries[i].m_GeometryCount; ++j)
        {
            // geometry doesn't contain a mesh?
            if (!pGeometries[i].m_pGeometries[j].m_pMesh)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                return 0;
            }

            index = pCollada->m_MeshCount;

            // create a mesh
            pMesh = (CSR_Mesh*)csrMemoryAlloc(pCollada->m_pMesh, sizeof(CSR_Mesh), pCollada->m_MeshCount + 1);

            // succeeded?
            if (!pMesh)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                return 0;
            }

            pCollada->m_pMesh = pMesh;
            ++pCollada->m_MeshCount;

            csrMeshInit(&pCollada->m_pMesh[index]);

            // build the mesh
            csrColladaMeshBuild(pVertFormat,
                                pVertCulling,
                                pMaterial,
                                pGeometries[i].m_pGeometries[j].m_pMesh,
                               &pCollada->m_pMesh[index],
                                fOnGetVertexColor);
        }

    // iterate through visual scene libraries
    for (i = 0; i < visualSceneCount; ++i)
    {
        CSR_Collada_Skeleton** pSrcSkeletons = 0;
        CSR_Collada_Node**     pRootBones    = 0;
        CSR_Skeleton*          pSkeletons    = 0;
        char**                 pUrls         = 0;
        size_t                 count         = 0;
        size_t                 rootCount     = 0;

        // find the skeletons contained in the scene
        for (j = 0; j < pVisualScenes[i].m_VisualSceneCount; ++j)
            for (k = 0; k < pVisualScenes[i].m_pVisualScenes[j].m_NodeCount; ++k)
                csrColladaNodeFindSkeletons(&pVisualScenes[i].m_pVisualScenes[j].m_pNodes[k],
                                            &pSrcSkeletons,
                                            &pUrls,
                                            &count);

        // find the skeleton root nodes
        for (j = 0; j < pVisualScenes[i].m_VisualSceneCount; ++j)
            for (k = 0; k < pVisualScenes[i].m_pVisualScenes[j].m_NodeCount; ++k)
                for (l = 0; l < count; ++l)
                    csrColladaNodeFindRootBones(&pVisualScenes[i].m_pVisualScenes[j].m_pNodes[k],
                                                 pSrcSkeletons[l]->m_pId,
                                                &pRootBones,
                                                &rootCount);

        // not found?
        if (!pRootBones || rootCount != count)
        {
            if (pSrcSkeletons)
                free(pSrcSkeletons);

            if (pUrls)
                free(pUrls);

            if (pRootBones)
                free(pRootBones);

            // release collada objects
            csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
            csrColladaControllerLibraryRelease(pControllers, controllerCount);
            csrColladaAnimationLibraryRelease(pAnimations, animationCount);
            csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

            return 0;
        }

        // iterate through source skeletons
        for (j = 0; j < count; ++j)
        {
            int    skeletonExists = 0;
            size_t index          = 0;
            size_t idLen          = 0;
            size_t parentIdLen    = 0;
            size_t targetLen      = 0;

            // search if skeleton already exists
            for (k = 0; k < pCollada->m_SkeletonCount; ++k)
                if (strcmp(pSrcSkeletons[j]->m_pId, pCollada->m_pSkeletons[k].m_pId) == 0)
                {
                    skeletonExists = 1;
                    break;
                }

            // skeleton already exists?
            if (skeletonExists)
                continue;

            index = pCollada->m_SkeletonCount;

            // add a new skeleton in the array
            pSkeletons =
                    (CSR_Skeleton*)csrMemoryAlloc(pCollada->m_pSkeletons,
                                                  sizeof(CSR_Skeleton),
                                                  pCollada->m_SkeletonCount + 1);

            // succeeded?
            if (!pSkeletons)
            {
                if (pSrcSkeletons)
                    free(pSrcSkeletons);

                if (pUrls)
                    free(pUrls);

                if (pRootBones)
                    free(pRootBones);

                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                return 0;
            }

            // set new skeleton array in the model
            pCollada->m_pSkeletons = pSkeletons;
            ++pCollada->m_SkeletonCount;

            // initialize the newly added skeleton
            csrSkeletonInit(&pCollada->m_pSkeletons[index]);

            // measure the source url length
            targetLen = strlen(pUrls[j]);

            // set the skeleton identifier
            if (targetLen)
            {
                // reserve memory for the identifier
                pCollada->m_pSkeletons[index].m_pTarget = (char*)malloc((targetLen + 1) * sizeof(char));

                // copy the value
                if (pCollada->m_pSkeletons[index].m_pTarget)
                {
                    memcpy(pCollada->m_pSkeletons[index].m_pTarget, pUrls[j], targetLen);
                    pCollada->m_pSkeletons[index].m_pTarget[targetLen] = 0x0;
                }
            }

            // measure the source id length
            idLen = strlen(pSrcSkeletons[j]->m_pId);

            // set the skeleton identifier
            if (idLen)
            {
                // reserve memory for the identifier
                pCollada->m_pSkeletons[index].m_pId = (char*)malloc((idLen + 1) * sizeof(char));

                // copy the value
                if (pCollada->m_pSkeletons[index].m_pId)
                {
                    memcpy(pCollada->m_pSkeletons[index].m_pId, pSrcSkeletons[j]->m_pId, idLen);
                    pCollada->m_pSkeletons[index].m_pId[idLen] = 0x0;
                }
            }

            // create a new skeleton
            pCollada->m_pSkeletons[index].m_pRoot = (CSR_Bone*)malloc(sizeof(CSR_Bone));
            csrBoneInit(pCollada->m_pSkeletons[index].m_pRoot);

            // get the initial matrix, if exists. NOTE assume that only one initial matrix exists
            if (pRootBones[j]->m_pParent && pRootBones[j]->m_pParent->m_MatrixCount)
                pCollada->m_pSkeletons[index].m_InitialMatrix = pRootBones[j]->m_pParent->m_pMatrices[0].m_Matrix;

            // build it
            csrColladaBuildSkeleton(pRootBones[j], pCollada->m_pSkeletons[index].m_pRoot);
        }

        if (pSrcSkeletons)
            free(pSrcSkeletons);

        if (pUrls)
            free(pUrls);

        if (pRootBones)
            free(pRootBones);
    }

    // is model supporting skin weights?
    if (!pCollada->m_MeshOnly && pCollada->m_MeshCount < 2)
    {
        // create as many mesh weights as meshes
        pCollada->m_pMeshWeights =
                (CSR_Skin_Weights_Group*)csrMemoryAlloc(pCollada->m_pMeshWeights,
                                                        sizeof(CSR_Skin_Weights_Group),
                                                        pCollada->m_MeshCount);

        // succeeded?
        if (!pCollada->m_pMeshWeights)
        {
            // release collada objects
            csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
            csrColladaControllerLibraryRelease(pControllers, controllerCount);
            csrColladaAnimationLibraryRelease(pAnimations, animationCount);
            csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

            return 0;
        }

        // set mesh weights count
        pCollada->m_MeshWeightsCount = pCollada->m_MeshCount;

        // iterate through mesh skin weights items to populate
        for (i = 0; i < pCollada->m_MeshWeightsCount; ++i)
        {
            CSR_Collada_Geometry*   pGeometry   = 0;
            CSR_Collada_Controller* pController = 0;
            size_t                  geomIndex   = 0;

            // initialize the mesh skin weights item
            pCollada->m_pMeshWeights[i].m_pSkinWeights = 0;
            pCollada->m_pMeshWeights[i].m_Count        = 0;

            // iterate through geometry libraries
            for (j = 0; j < geometryCount; ++j)
            {
                int doBreak = 0;

                // iterate through geometries
                for (k = 0; k < pGeometries[j].m_GeometryCount; ++k)
                {
                    // found the matching source geometry?
                    if (geomIndex == i)
                    {
                        pGeometry = &pGeometries[j].m_pGeometries[k];
                        doBreak   = 1;
                        break;
                    }

                    ++geomIndex;
                }

                if (doBreak)
                    break;
            }

            if (!pGeometry)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease   (pGeometries,   geometryCount);
                csrColladaControllerLibraryRelease (pControllers,  controllerCount);
                csrColladaAnimationLibraryRelease  (pAnimations,   animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                return 0;
            }

            // iterate through controller libraries
            for (j = 0; j < controllerCount; ++j)
            {
                int doBreak = 0;

                // iterate through controllers
                for (k = 0; k < pControllers[j].m_ControllerCount; ++k)
                    // found the matching source skin?
                    if (pControllers[j].m_pControllers[k].m_pSkin->m_pSource &&
                        strcmp(pGeometry->m_pId, pControllers[j].m_pControllers[k].m_pSkin->m_pSource + 1) == 0)
                    {
                        pController = &pControllers[j].m_pControllers[k];
                        doBreak     =  1;
                        break;
                    }

                if (doBreak)
                    break;
            }

            if (!pController)
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                return 0;
            }

            // build the mesh weights
            if (!csrColladaMeshWeightsBuild(pGeometry, pController, i, pCollada))
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                return 0;
            }
        }
    }

    // is model supporting animations?
    if (!pCollada->m_MeshOnly && !pCollada->m_PoseOnly && pAnimations)
    {
        // create as many mesh weights as meshes
        pCollada->m_pAnimationSet =
                (CSR_AnimationSet_Bone*)csrMemoryAlloc(pCollada->m_pAnimationSet,
                                                       sizeof(CSR_AnimationSet_Bone),
                                                       pAnimations->m_AnimationCount);

        // succeeded?
        if (!pCollada->m_pAnimationSet)
        {
            // release collada objects
            csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
            csrColladaControllerLibraryRelease(pControllers, controllerCount);
            csrColladaAnimationLibraryRelease(pAnimations, animationCount);
            csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

            return 0;
        }

        // set animation set count
        pCollada->m_AnimationSetCount = pAnimations->m_AnimationCount;

        // iterate through animation items to populate
        for (i = 0; i < pAnimations->m_AnimationCount; ++i)
        {
            // initialize the animation item
            pCollada->m_pAnimationSet[i].m_pAnimation = 0;
            pCollada->m_pAnimationSet[i].m_Count      = 0;

            // build the animation
            if (!csrColladaAnimationBuild(&pAnimations->m_pAnimations[i],
                                          &pCollada->m_pAnimationSet[i],
                                           pCollada))
            {
                // release collada objects
                csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
                csrColladaControllerLibraryRelease(pControllers, controllerCount);
                csrColladaAnimationLibraryRelease(pAnimations, animationCount);
                csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

                return 0;
            }
        }
    }

    // release collada objects
    csrColladaGeometryLibraryRelease(pGeometries, geometryCount);
    csrColladaControllerLibraryRelease(pControllers, controllerCount);
    csrColladaAnimationLibraryRelease(pAnimations, animationCount);
    csrColladaVisualSceneLibraryRelease(pVisualScenes, visualSceneCount);

    return 1;
}
//---------------------------------------------------------------------------
// Collada functions
//---------------------------------------------------------------------------
CSR_Collada* csrColladaCreate(const CSR_Buffer*           pBuffer,
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
    CSR_Collada* pCollada;

    // is buffer valid?
    if (!pBuffer || !pBuffer->m_Length)
        return 0;

    // create the collada model
    pCollada = (CSR_Collada*)malloc(sizeof(CSR_Collada));

    // succeeded?
    if (!pCollada)
        return 0;

    csrColladaInit(pCollada);

    pCollada->m_MeshOnly = meshOnly;
    pCollada->m_PoseOnly = poseOnly;

    // parse the file content
    if (!csrColladaParse(pBuffer, pVertFormat, pVertCulling, pMaterial, pCollada, fOnGetVertexColor))
        return 0;

    return pCollada;
}
//---------------------------------------------------------------------------
CSR_Collada* csrColladaOpen(const char*                 pFileName,
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
    CSR_Buffer*  pBuffer;
    CSR_Collada* pCollada;

    // open the model file
    pBuffer = csrFileOpen(pFileName);

    // succeeded?
    if (!pBuffer || !pBuffer->m_Length)
    {
        csrBufferRelease(pBuffer);
        return 0;
    }

    // create the collada model from the file content
    pCollada = csrColladaCreate(pBuffer,
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

    return pCollada;
}
//---------------------------------------------------------------------------
void csrColladaInit(CSR_Collada* pCollada)
{
    // no collada model to initialize?
    if (!pCollada)
        return;

    // initialize the collada model
    pCollada->m_pMesh               = 0;
    pCollada->m_MeshCount           = 0;
    pCollada->m_pMeshWeights        = 0;
    pCollada->m_MeshWeightsCount    = 0;
    pCollada->m_pMeshToBoneDict     = 0;
    pCollada->m_MeshToBoneDictCount = 0;
    pCollada->m_pSkeletons          = 0;
    pCollada->m_SkeletonCount       = 0;
    pCollada->m_pAnimationSet       = 0;
    pCollada->m_AnimationSetCount   = 0;
    pCollada->m_MeshOnly            = 0;
    pCollada->m_PoseOnly            = 0;
}
//---------------------------------------------------------------------------
void csrColladaRelease(CSR_Collada* pCollada, const CSR_fOnDeleteTexture fOnDeleteTexture)
{
    size_t i;
    size_t j;

    // no collada model to release?
    if (!pCollada)
        return;

    // do free the meshes content?
    if (pCollada->m_pMesh)
    {
        // iterate through meshes to free
        for (i = 0; i < pCollada->m_MeshCount; ++i)
        {
            // delete the skin
            csrSkinContentRelease(&pCollada->m_pMesh[i].m_Skin, fOnDeleteTexture);

            // do free the mesh vertex buffer?
            if (pCollada->m_pMesh[i].m_pVB)
            {
                // free the mesh vertex buffer content
                for (j = 0; j < pCollada->m_pMesh[i].m_Count; ++j)
                    if (pCollada->m_pMesh[i].m_pVB[j].m_pData)
                        free(pCollada->m_pMesh[i].m_pVB[j].m_pData);

                // free the mesh vertex buffer
                free(pCollada->m_pMesh[i].m_pVB);
            }
        }

        // free the meshes
        free(pCollada->m_pMesh);
    }

    // release the weights
    if (pCollada->m_pMeshWeights)
    {
        // release the mesh weights content
        for (i = 0; i < pCollada->m_MeshWeightsCount; ++i)
        {
            // release the mesh skin weights content
            for (j = 0; j < pCollada->m_pMeshWeights[i].m_Count; ++j)
                csrSkinWeightsRelease(&pCollada->m_pMeshWeights[i].m_pSkinWeights[j], 1);

            // free the mesh skin weights
            free(pCollada->m_pMeshWeights[i].m_pSkinWeights);
        }

        // free the mesh weights
        free(pCollada->m_pMeshWeights);
    }

    // release the mesh-to-bone dictionary
    if (pCollada->m_pMeshToBoneDict)
        free(pCollada->m_pMeshToBoneDict);

    // release the skeletons
    if (pCollada->m_pSkeletons)
    {
        // release the skeleton content
        for (i = 0; i < pCollada->m_SkeletonCount; ++i)
            csrSkeletonRelease(&pCollada->m_pSkeletons[i], 1);

        // free the skeletons
        free(pCollada->m_pSkeletons);
    }

    // release the animation sets
    if (pCollada->m_pAnimationSet)
    {
        // release the animation set content
        for (i = 0; i < pCollada->m_AnimationSetCount; ++i)
            csrBoneAnimSetRelease(&pCollada->m_pAnimationSet[i], 1);

        // free the animation sets
        free(pCollada->m_pAnimationSet);
    }

    // release the model
    free(pCollada);
}
//---------------------------------------------------------------------------
