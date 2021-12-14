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

// Collada geometry sample
/*
  <library_geometries>
    <geometry id="Sphere-mesh" name="Sphere">
      <mesh>
        <source id="Sphere-mesh-positions">
          <float_array id="Sphere-mesh-positions-array" count="6927">0 0.1950903 0.9807853 0 ...</float_array>
          <technique_common>
            <accessor source="#Sphere-mesh-positions-array" count="2309" stride="3">
              <param name="X" type="float"/>
              <param name="Y" type="float"/>
              <param name="Z" type="float"/>
            </accessor>
          </technique_common>
        </source>
        <source id="Sphere-mesh-normals">
          <float_array id="Sphere-mesh-normals-array" count="13995">0.0463792 0.4708896 0.8809722 ...</float_array>
          <technique_common>
            <accessor source="#Sphere-mesh-normals-array" count="4665" stride="3">
              <param name="X" type="float"/>
              <param name="Y" type="float"/>
              <param name="Z" type="float"/>
            </accessor>
          </technique_common>
        </source>
        <source id="Sphere-mesh-map-0">
          <float_array id="Sphere-mesh-map-0-array" count="28368">0.75 0.875 0.71875 0.8125 0.75 ...</float_array>
          <technique_common>
            <accessor source="#Sphere-mesh-map-0-array" count="14184" stride="2">
              <param name="S" type="float"/>
              <param name="T" type="float"/>
            </accessor>
          </technique_common>
        </source>
        <vertices id="Sphere-mesh-vertices">
          <input semantic="POSITION" source="#Sphere-mesh-positions"/>
        </vertices>
        <triangles count="4728">
          <input semantic="VERTEX" source="#Sphere-mesh-vertices" offset="0"/>
          <input semantic="NORMAL" source="#Sphere-mesh-normals" offset="1"/>
          <input semantic="TEXCOORD" source="#Sphere-mesh-map-0" offset="2" set="0"/>
          <p>1 0 0 9 0 1 2 0 2 2 1 3 10 1 4 3 1 5 3 2 6 11 2 7 4 2 8 4 3 9 12 3 10 5 3 11 5 4 12 13 4 13 6 ...</p>
        </triangles>
      </mesh>
    </geometry>
  </library_geometries>
*/

// Collada skeleton sample
/*
  <library_visual_scenes>
    <visual_scene id="Scene" name="Scene">
      <node id="Armature" name="Armature" type="NODE">
        <matrix sid="transform">0.9979954 0.004671548 -0.09376783 0.3710257 -0.004102812 0.9999624 0.01126042 0.2228146 0.06957536 -0.008042011 1.345678 -2.414656 0 0 0 1</matrix>
        <node id="Armature_Bone" name="Bone" sid="Bone" type="JOINT">
          <matrix sid="transform">1 0 0 0 0 0 -1 0 0 1 0 0 0 0 0 1</matrix>
          <node id="Armature_Bone_001" name="Bone.001" sid="Bone_001" type="JOINT">
            <matrix sid="transform">0.4825036 0.03371993 -0.8752446 0 -0.03256724 0.9992582 0.0205441 0.9999999 0.8752882 0.0185917 0.4832439 0 0 0 0 1</matrix>
            <node id="Armature_Bone_002" name="Bone.002" sid="Bone_002" type="JOINT">
              <matrix sid="transform">0.8301729 -0.4698131 0.3001482 -4.09782e-8 0.376711 0.07584341 -0.9232209 0.8146697 0.410977 0.8795018 0.2399469 -2.23517e-8 0 0 0 1</matrix>
              <node id="Armature_Bone_005" name="Bone.005" sid="Bone_005" type="JOINT">
                <matrix sid="transform">0.9563425 -0.2848547 0.06532297 2.29105e-7 0.2848549 0.8586038 -0.4262129 0.3295822 0.06532219 0.426213 0.9022614 -1.3411e-7 0 0 0 1</matrix>
                <node id="Armature_Bone_006" name="Bone.006" sid="Bone_006" type="JOINT">
                  <matrix sid="transform">0.9290453 -0.3573819 0.09567147 -7.45058e-8 0.3573819 0.8000469 -0.4818745 0.5148735 0.09567154 0.4818745 0.8710017 2.68221e-7 0 0 0 1</matrix>
                  <extra>
                    <technique profile="blender">
                      <connect sid="connect" type="bool">1</connect>
                      <layer sid="layer" type="string">0</layer>
                      <roll sid="roll" type="float">2.58385</roll>
                      <tip_x sid="tip_x" type="float">-0.2519096</tip_x>
                      <tip_y sid="tip_y" type="float">0.1178481</tip_y>
                      <tip_z sid="tip_z" type="float">-0.4467086</tip_z>
                    </technique>
                  </extra>
                </node>
                <extra>
                  <technique profile="blender">
                    <connect sid="connect" type="bool">1</connect>
                    <layer sid="layer" type="string">0</layer>
                    <roll sid="roll" type="float">3.116575</roll>
                  </technique>
                </extra>
              </node>
              <extra>
                <technique profile="blender">
                  <connect sid="connect" type="bool">1</connect>
                  <layer sid="layer" type="string">0</layer>
                  <roll sid="roll" type="float">-2.66228</roll>
                </technique>
              </extra>
            </node>
            <node id="Armature_Bone_003" name="Bone.003" sid="Bone_003" type="JOINT">
              <matrix sid="transform">0.8209246 0.474395 0.3178559 -4.47035e-8 -0.3974511 0.07500358 0.9145529 0.8146697 0.410019 -0.8771111 0.2501212 -4.09782e-8 0 0 0 1</matrix>
              <node id="Armature_Bone_007" name="Bone.007" sid="Bone_007" type="JOINT">
                <matrix sid="transform">0.9950608 0.09524012 0.02799028 9.31323e-8 -0.09524009 0.8364414 0.5397177 0.2816591 0.02799049 -0.5397177 0.8413807 -7.40401e-8 0 0 0 1</matrix>
                <node id="Armature_Bone_008" name="Bone.008" sid="Bone_008" type="JOINT">
                  <matrix sid="transform">0.9919804 0.1165871 0.04880872 -6.70552e-8 -0.1165874 0.6949258 0.7095672 0.5861385 0.04880798 -0.7095674 0.7029452 2.08616e-7 0 0 0 1</matrix>
                  <extra>
                    <technique profile="blender">
                      <connect sid="connect" type="bool">1</connect>
                      <layer sid="layer" type="string">0</layer>
                      <roll sid="roll" type="float">1.548818</roll>
                      <tip_x sid="tip_x" type="float">0.1062508</tip_x>
                      <tip_y sid="tip_y" type="float">0.1215499</tip_y>
                      <tip_z sid="tip_z" type="float">-0.4448311</tip_z>
                    </technique>
                  </extra>
                </node>
                <extra>
                  <technique profile="blender">
                    <connect sid="connect" type="bool">1</connect>
                    <layer sid="layer" type="string">0</layer>
                    <roll sid="roll" type="float">0.9091836</roll>
                  </technique>
                </extra>
              </node>
              <extra>
                <technique profile="blender">
                  <connect sid="connect" type="bool">1</connect>
                  <layer sid="layer" type="string">0</layer>
                  <roll sid="roll" type="float">0.3876594</roll>
                </technique>
              </extra>
            </node>
            <node id="Armature_Bone_004" name="Bone.004" sid="Bone_004" type="JOINT">
              <matrix sid="transform">0.9988919 -0.04701851 -0.002125236 -5.58794e-9 0.04698164 0.9987841 -0.01494653 0.8146698 0.002825361 0.01483012 0.9998861 1.86265e-9 0 0 0 1</matrix>
              <extra>
                <technique profile="blender">
                  <connect sid="connect" type="bool">1</connect>
                  <layer sid="layer" type="string">0</layer>
                  <roll sid="roll" type="float">-1.070866</roll>
                  <tip_x sid="tip_x" type="float">-6.04041e-4</tip_x>
                  <tip_y sid="tip_y" type="float">0.004685759</tip_y>
                  <tip_z sid="tip_z" type="float">0.3038596</tip_z>
                </technique>
              </extra>
            </node>
            <extra>
              <technique profile="blender">
                <connect sid="connect" type="bool">1</connect>
                <layer sid="layer" type="string">0</layer>
                <roll sid="roll" type="float">-1.032612</roll>
              </technique>
            </extra>
          </node>
          <extra>
            <technique profile="blender">
              <layer sid="layer" type="string">0</layer>
            </technique>
          </extra>
        </node>
        <node id="DropChar" name="DropChar" type="NODE">
          <matrix sid="transform">1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1</matrix>
          <instance_controller url="#Armature_DropChar-skin">
            <skeleton>#Armature_Bone</skeleton>
          </instance_controller>
        </node>
      </node>
    </visual_scene>
  </library_visual_scenes>
*/
//---------------------------------------------------------------------------
// Global defines
//---------------------------------------------------------------------------
#define M_Collada_Root_Tag             "COLLADA"
#define M_Collada_Geometries_Tag       "library_geometries"
#define M_Collada_Geometry_Tag         "geometry"
#define M_Collada_Mesh_Tag             "mesh"
#define M_Collada_Source_Tag           "source"
#define M_Collada_Float_Array_Tag      "float_array"
#define M_Collada_Name_Array_Tag       "Name_array"
#define M_Collada_Vertices_Tag         "vertices"
#define M_Collada_Triangles_Tag        "triangles"
#define M_Collada_Technique_Tag        "technique"
#define M_Collada_Technique_Common_Tag "technique_common"
#define M_Collada_Accessor_Tag         "accessor"
#define M_Collada_Param_Tag            "param"
#define M_Collada_Input_Tag            "input"
#define M_Collada_P_Tag                "p"
#define M_Collada_Visual_Scenes_Tag    "library_visual_scenes"
#define M_Collada_Visual_Scene_Tag     "visual_scene"
#define M_Collada_Node_Tag             "node"
#define M_Collada_Matrix_Tag           "matrix"
#define M_Collada_ID_Attribute         "id"
#define M_Collada_SID_Attribute        "sid"
#define M_Collada_Name_Attribute       "name"
#define M_Collada_Type_Attribute       "type"
#define M_Collada_Source_Attribute     "source"
#define M_Collada_Count_Attribute      "count"
#define M_Collada_Stride_Attribute     "stride"
#define M_Collada_Semantic_Attribute   "semantic"
#define M_Collada_Offset_Attribute     "offset"
#define M_Collada_Set_Attribute        "set"
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
    CSR_Collada_Accessor* m_pAccessors;    // accessors
    size_t                m_AccessorCount; // accessors count
} CSR_Collada_Technique_Common;

/**
* Collada (.dae) float array tag
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
    int*    m_pData; // integer array data
    size_t  m_Count; // array count
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
* Collada (.dae) source
*/
typedef struct
{
    char*                         m_pId;          // array identifier
    CSR_Collada_Float_Array*      m_pFloatArray;  // float array
    CSR_Collada_Technique_Common* m_pTechCommons; // technique common
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
    char*              m_pId;
    CSR_Collada_Input* m_pInputs;
    size_t             m_InputCount;
} CSR_Collada_Vertices;

/**
* Collada (.dae) triangles
*/
typedef struct
{
    size_t                      m_Count;
    CSR_Collada_Input*          m_pInputs;
    size_t                      m_InputCount;
    CSR_Collada_Unsigned_Array* m_pPrimitiveIndexArray;
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
    char*             m_pId;       // geometry identifier
    char*             m_pName;     // geometry name
    CSR_Collada_Mesh* m_pMeshes;   // meshes
    size_t            m_MeshCount; // mesh count
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
* Collada (.dae) matrix
*/
typedef struct
{
    char*       m_pSid;   // animation target identifier
    CSR_Matrix4 m_Matrix; // matrix
} CSR_Collada_Matrix;

/**
* Collada (.dae) node
*/
typedef struct CSR_tagCollada_Node
{
    char*                       m_pName;     // node name
    char*                       m_pId;       // node identifier
    char*                       m_pSid;      // animation target identifier
    char*                       m_pType;     // node type
    CSR_Collada_Matrix*         m_pMatrix;   // bone matrix
    struct CSR_tagCollada_Node* m_pNodes;    // children nodes
    size_t                      m_NodeCount; // child node count
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

    // geometry node contains attributes?
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

    // iterate through accessor node children
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

    pColladaTechCommon->m_pAccessors    = 0;
    pColladaTechCommon->m_AccessorCount = 0;
}
//---------------------------------------------------------------------------
void csrColladaTechCommonRelease(CSR_Collada_Technique_Common* pColladaTechCommon)
{
    if (!pColladaTechCommon)
        return;

    // free the accessor array
    if (pColladaTechCommon->m_pAccessors)
    {
        size_t i;

        // iterate through accessors to free
        for (i = 0; i < pColladaTechCommon->m_AccessorCount; ++i)
            csrColladaAccessorRelease(&pColladaTechCommon->m_pAccessors[i]);

        // free the accessors container
        free(pColladaTechCommon->m_pAccessors);
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

    // iterate through technique common node children
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
            const size_t index = pColladaTechCommon->m_AccessorCount;

            // add a new accessor in the array
            CSR_Collada_Accessor* pAccessors =
                    (CSR_Collada_Accessor*)csrMemoryAlloc(pColladaTechCommon->m_pAccessors,
                                                          sizeof(CSR_Collada_Accessor),
                                                          pColladaTechCommon->m_AccessorCount + 1);

            // succeeded?
            if (!pAccessors)
                return 0;

            // set new accessor array in the technique common
            pColladaTechCommon->m_pAccessors = pAccessors;
            ++pColladaTechCommon->m_AccessorCount;

            // initialize newly added accessor
            csrColladaAccessorInit(&pColladaTechCommon->m_pAccessors[index]);

            // read it
            if (!csrColladaAccessorRead(pChild, &pColladaTechCommon->m_pAccessors[index]))
                return 0;
        }
    }

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

    // geometry node contains attributes?
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
        pColladaFloatArray->m_pData[index] = (float)atof(pNumber);

        // start to read the next number
        ++index;
        offset = i;
    }

    // something was read?
    if (offset)
        // skip the last read space
        ++offset;

    // calculate last number length, and allocate memory to read it
    len = (strlen(pNode->text) - offset);

    // 64 digits max
    if (len >= 64)
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
        offset = i;
    }

    // to prevent that bad things happens...
    if (index >= count)
        return 0;

    // something was read?
    if (offset)
        // skip the last read space
        ++offset;

    // calculate last number length, and allocate memory to read it
    len = (strlen(pNode->text) - offset);

    // 64 digits max
    if (len >= 64)
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

    // set (and trust) the array count
    pColladaUnsignedArray->m_Count = count;

    // allocate memory for the int array
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
            return 0;

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
        offset = i;
    }

    // to prevent that bad things happens...
    if (index >= count)
        return 0;

    // something was read?
    if (offset)
        // skip the last read space
        ++offset;

    // calculate last number length, and allocate memory to read it
    len = (strlen(pNode->text) - offset);

    // 64 digits max
    if (len >= 64)
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
void csrColladaSourceInit(CSR_Collada_Source* pColladaSource)
{
    if (!pColladaSource)
        return;

    pColladaSource->m_pId          = 0;
    pColladaSource->m_pFloatArray  = 0;
    pColladaSource->m_pTechCommons = 0;
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
    csrColladaFloatArrayRelease(pColladaSource->m_pFloatArray);
    free(pColladaSource->m_pFloatArray);

    // free the tech commons
    csrColladaTechCommonRelease(pColladaSource->m_pTechCommons);
    free(pColladaSource->m_pTechCommons);
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

    // geometry node contains attributes?
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

    // iterate through source node children
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
        if (len == strlen(M_Collada_Technique_Common_Tag) &&
            memcmp(pChild->tag, M_Collada_Technique_Common_Tag, len) == 0)
        {
            // create new technique commons
            pColladaSource->m_pTechCommons = (CSR_Collada_Technique_Common*)malloc(sizeof(CSR_Collada_Technique_Common));

            // succeeded?
            if (!pColladaSource->m_pTechCommons)
                return 0;

            // initialize newly created technique commons
            csrColladaTechCommonInit(pColladaSource->m_pTechCommons);

            // read it
            if (!csrColladaTechCommonRead(pChild, pColladaSource->m_pTechCommons))
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

    // geometry node contains attributes?
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
    size_t i;

    if (!pColladaVertices)
        return;

    // free the identifier
    if (pColladaVertices->m_pId)
        free(pColladaVertices->m_pId);

    // free the source array
    if (pColladaVertices->m_pInputs)
    {
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

    // geometry node contains attributes?
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

    // iterate through accessor node children
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

    pColladaTriangles->m_Count                = 0;
    pColladaTriangles->m_pInputs              = 0;
    pColladaTriangles->m_InputCount           = 0;
    pColladaTriangles->m_pPrimitiveIndexArray = 0;
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

    // free the primitive index array
    if (pColladaTriangles->m_pPrimitiveIndexArray)
    {
        csrColladaUnsignedArrayRelease(pColladaTriangles->m_pPrimitiveIndexArray);
        free(pColladaTriangles->m_pPrimitiveIndexArray);
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

    // geometry node contains attributes?
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

    // iterate through accessor node children
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

    // iterate through mesh node children
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
void csrColladaGeometryInit(CSR_Collada_Geometry* pColladaGeometry)
{
    if (!pColladaGeometry)
        return;

    pColladaGeometry->m_pId       = 0;
    pColladaGeometry->m_pName     = 0;
    pColladaGeometry->m_pMeshes   = 0;
    pColladaGeometry->m_MeshCount = 0;
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
    if (pColladaGeometry->m_pMeshes)
    {
        size_t i;

        // iterate through meshes to free
        for (i = 0; i < pColladaGeometry->m_MeshCount; ++i)
            csrColladaMeshRelease(&pColladaGeometry->m_pMeshes[i]);

        // free the meshes container
        free(pColladaGeometry->m_pMeshes);
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

    // geometry node contains attributes?
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

    // iterate through geometry node children
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
            const size_t index = pColladaGeometry->m_MeshCount;

            // add a new mesh in the array
            CSR_Collada_Mesh* pMeshes =
                    (CSR_Collada_Mesh*)csrMemoryAlloc(pColladaGeometry->m_pMeshes,
                                                      sizeof(CSR_Collada_Mesh),
                                                      pColladaGeometry->m_MeshCount + 1);

            // succeeded?
            if (!pMeshes)
                return 0;

            // set new mesh array in the geometry
            pColladaGeometry->m_pMeshes = pMeshes;
            ++pColladaGeometry->m_MeshCount;

            // initialize newly added mesh
            csrColladaMeshInit(&pColladaGeometry->m_pMeshes[index]);

            // read it
            if (!csrColladaMeshRead(pChild, &pColladaGeometry->m_pMeshes[index]))
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
    size_t offset;
    size_t index;
    char*  pNumber;

    if (!pNode)
        return 0;

    if (!pColladaMatrix)
        return 0;

    // matrix node contains attributes?
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

    // iterate through source array chars
    for (i = 0; i < strlen(pNode->text); ++i)
    {
        // found a separator?
        if (pNode->text[i] != ' ')
            continue;

        // calculate next number length, and allocate memory to read it
        len     = (i - offset);
        pNumber = (char*)malloc((len + 1) * sizeof(char));

        // succeeded?
        if (!pNumber)
            return 0;

        // read the next number to convert
        memcpy(pNumber, &pNode->text[offset], len);
        pNumber[len] = 0x0;

        // convert it and write it in the array
        pColladaMatrix->m_Matrix.m_Table[index % 4][index / 4] = (float)atof(pNumber);

        free(pNumber);

        // start to read the next number
        ++index;
        offset = i;
    }

    // something was read?
    if (offset)
        // skip the last read space
        ++offset;

    // calculate last number length, and allocate memory to read it
    len     = (strlen(pNode->text) - offset);
    pNumber = (char*)malloc((len + 1) * sizeof(char));

    // succeeded?
    if (!pNumber)
        return 0;

    // read the last number to convert
    memcpy(pNumber, &pNode->text[offset], len);
    pNumber[len] = 0x0;

    // convert it and write it in the array
    pColladaMatrix->m_Matrix.m_Table[index / 4][index % 4] = (float)atof(pNumber);

    free(pNumber);

    return 1;
}
//---------------------------------------------------------------------------
void csrColladaNodeInit(CSR_Collada_Node* pColladaNode)
{
    if (!pColladaNode)
        return;

    pColladaNode->m_pName     = 0;
    pColladaNode->m_pId       = 0;
    pColladaNode->m_pSid      = 0;
    pColladaNode->m_pType     = 0;
    pColladaNode->m_pNodes    = 0;
    pColladaNode->m_NodeCount = 0;
    pColladaNode->m_pMatrix   = 0;
}
//---------------------------------------------------------------------------
void csrColladaNodeRelease(CSR_Collada_Node* pColladaNode)
{
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

    // free the children nodes
    if (pColladaNode->m_pNodes)
    {
        size_t i;

        // iterate through meshes to free
        for (i = 0; i < pColladaNode->m_NodeCount; ++i)
            csrColladaNodeRelease(&pColladaNode->m_pNodes[i]);

        // free the nodes container
        free(pColladaNode->m_pNodes);
    }

    // free the matrix
    if (pColladaNode->m_pMatrix)
    {
        csrColladaMatrixRelease(pColladaNode->m_pMatrix);
        free(pColladaNode->m_pMatrix);
    }
}
//---------------------------------------------------------------------------
int csrColladaNodeRead(XMLNode* pNode, CSR_Collada_Node* pColladaNode)
{
    size_t i;
    size_t len;

    if (!pNode)
        return 0;

    if (!pColladaNode)
        return 0;

    // geometry node contains attributes?
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

    // iterate through geometry node children
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
            if (!csrColladaNodeRead(pChild, &pColladaNode->m_pNodes[index]))
                return 0;
        }
        else
        if (len == strlen(M_Collada_Matrix_Tag) &&
            memcmp(pChild->tag, M_Collada_Matrix_Tag, len) == 0)
        {
            // allocate memory for matrix
            pColladaNode->m_pMatrix = (CSR_Collada_Matrix*)malloc(sizeof(CSR_Collada_Matrix));

            // initialize newly added matrix
            csrColladaMatrixInit(pColladaNode->m_pMatrix);

            // read it
            if (!csrColladaMatrixRead(pChild, pColladaNode->m_pMatrix))
                return 0;
        }
    }

    return 1;
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

    // geometry node contains attributes?
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

    // iterate through geometry node children
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
            if (!csrColladaNodeRead(pChild, &pColladaVisualScene->m_pNodes[index]))
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

    // iterate through geometry node children
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
int csrColladaBuildSkeleton(CSR_Collada_Node* pNode, CSR_Bone* pBone)
{
    size_t i;

    if (!pNode)
        return 0;

    if (!pBone)
        return 0;

    // copy bone matrix from collada node
    if (pNode->m_pMatrix)
        pBone->m_Matrix = pNode->m_pMatrix->m_Matrix;

    // copy bone name from collada node
    if (pNode->m_pId)
    {
        const size_t idLength = strlen(pNode->m_pId);

        pBone->m_pName = (char*)malloc(idLength + 1);

        if (!pBone->m_pName)
            return 0;

        memcpy(pBone->m_pName, pNode->m_pId, idLength);
        pBone->m_pName[idLength] = 0x0;
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
int csrColladaParse(const CSR_Buffer* pBuffer, CSR_Collada* pCollada)
{
    XMLDoc                     doc;
    size_t                     i;
    size_t                     index;
    size_t                     len;
    size_t                     count;
    CSR_Mesh*                  pMesh;
    CSR_Collada_Geometries*    pGeometries   = 0;
    CSR_Collada_Visual_Scenes* pVisualScenes = 0;

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
            // create new geometries container
            pGeometries = (CSR_Collada_Geometries*)malloc(sizeof(CSR_Collada_Geometries));

            if (!pGeometries)
            {
                // release collada objects
                csrColladaVisualScenesRelease(pVisualScenes);
                free(pVisualScenes);

                // release xml document
                XMLDoc_free(&doc);
                return 0;
            }

            // initialize geometries container
            csrColladaGeometriesInit(pGeometries);

    	    // read geometry library
            if (!csrColladaGeometriesRead(pChild, pGeometries))
            {
                // release collada objects
                csrColladaGeometriesRelease(pGeometries);
                free(pGeometries);
                csrColladaVisualScenesRelease(pVisualScenes);
                free(pVisualScenes);

                // release xml document
                XMLDoc_free(&doc);

                return 0;
            }
        }
        else
        if (len == strlen(M_Collada_Visual_Scenes_Tag) &&
            memcmp(pChild->tag, M_Collada_Visual_Scenes_Tag, len) == 0)
        {
            // create new visual scenes container
            pVisualScenes = (CSR_Collada_Visual_Scenes*)malloc(sizeof(CSR_Collada_Visual_Scenes));

            if (!pVisualScenes)
            {
                // release collada objects
                csrColladaGeometriesRelease(pGeometries);
                free(pGeometries);

                // release xml document
                XMLDoc_free(&doc);
                return 0;
            }

            // initialize geometries container
            csrColladaVisualScenesInit(pVisualScenes);

            // read visual scenes library
            if (!csrColladaVisualScenesRead(pChild, pVisualScenes))
            {
                // release collada objects
                csrColladaGeometriesRelease(pGeometries);
                free(pGeometries);
                csrColladaVisualScenesRelease(pVisualScenes);
                free(pVisualScenes);

                // release xml document
                XMLDoc_free(&doc);

                return 0;
            }
        }
    }

    // todo FIXME finalize
    pCollada->m_pSkeleton = (CSR_Bone*)malloc(sizeof(CSR_Bone));
    csrBoneInit(pCollada->m_pSkeleton);

    // build bone hierarchy
    csrColladaBuildSkeleton(&pVisualScenes->m_pVisualScenes[0].m_pNodes[0],
                            pCollada->m_pSkeleton);

    // release collada objects
    csrColladaGeometriesRelease(pGeometries);
    free(pGeometries);
    csrColladaVisualScenesRelease(pVisualScenes);
    free(pVisualScenes);

    // release xml document
    XMLDoc_free(&doc);

    index = pCollada->m_MeshCount;

    // create a mesh
    pMesh = (CSR_Mesh*)csrMemoryAlloc(pCollada->m_pMesh, sizeof(CSR_Mesh), pCollada->m_MeshCount + 1);

    // succeeded?
    if (!pMesh)
        return 0;

    csrMeshInit(&pMesh[index]);

    pCollada->m_pMesh = pMesh;
    ++pCollada->m_MeshCount;

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
    //CSR_Header_X header;
    //size_t       offset;
    //CSR_Item_X*  pRoot;
    //CSR_Item_X*  pLocalRoot;

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
    if (!csrColladaParse(pBuffer, pCollada))
        return 0;

    /*
    offset = 0;

    // read the header
    csrBufferRead(pBuffer, &offset, sizeof(CSR_Header_X), 1, &header);

    // is a .x file?
    if (header.m_Magic != M_X_FORMAT_MAGIC)
        return 0;

    // only 3.2 or 3.3 versions are supported
    if ((header.m_Major_Version != M_X_FORMAT_VERSION03) ||
        (header.m_Minor_Version != M_X_FORMAT_VERSION03) &&
        (header.m_Minor_Version != M_X_FORMAT_VERSION02))
        return 0;

    // is .x file containing text?
    if (header.m_Format != M_X_FORMAT_TEXT)
        return 0;

    // create the root item
    pRoot = (CSR_Item_X*)malloc(sizeof(CSR_Item_X));

    // succeeded?
    if (!pRoot)
        return 0;

    // initialize it
    csrXInitItem(pRoot);

    // as the root pointer itself may change while parsing, keep a local copy of the root pointer
    pLocalRoot = pRoot;

    // parse the file content
    if (!csrXParse(pBuffer, &offset, &pRoot))
    {
        csrXReleaseItems(pLocalRoot, 0);
        return 0;
    }

    // create the collada model
    pCollada = (CSR_Collada*)malloc(sizeof(CSR_Collada));

    // succeeded?
    if (!pCollada)
    {
        csrXReleaseItems(pLocalRoot, 0);
        return 0;
    }

    csrColladaInit(pCollada);

    // configure it
    pCollada->m_MeshOnly = meshOnly;
    pCollada->m_PoseOnly = poseOnly;

    // convert the read item hierarchy to an x model
    if (!csrXItemToModel(pLocalRoot,
                         pX,
                         0,
                         pVertFormat,
                         pVertCulling,
                         pMaterial,
                         fOnGetVertexColor,
                         fOnLoadTexture,
                         fOnApplySkin,
                         fOnDeleteTexture))
    {
        csrXReleaseItems(pLocalRoot, 0);
        csrColladaRelease(pCollada, fOnDeleteTexture);
        return 0;
    }

    // build the bones parent hierarchy (could not simply keep the pointer while hierarchy was built
    // because the bone pointers may change several time while their hierarchy is built)
    if (pCollada->m_pSkeleton)
    {
        csrXBuildParentHierarchy(pCollada->m_pSkeleton, 0, pCollada);

        // skin weights?
        if (pX->m_pMeshWeights)
        {
            size_t i;
            size_t j;

            // retrieve the bone linked with each skin weights
            for (i = 0; i < pX->m_MeshWeightsCount; ++i)
                for (j = 0; j < pX->m_pMeshWeights[i].m_Count; ++j)
                    pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone =
                            csrBoneFind(pX->m_pSkeleton, pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBoneName);
        }

        // animation set?
        if (!pX->m_PoseOnly && pX->m_pAnimationSet)
        {
            size_t i;
            size_t j;

            // find each bone linked to animation sets
            for (i = 0; i < pX->m_AnimationSetCount; ++i)
                for (j = 0; j < pX->m_pAnimationSet[i].m_Count; ++j)
                    pX->m_pAnimationSet[i].m_pAnimation[j].m_pBone =
                            csrBoneFind(pX->m_pSkeleton, pX->m_pAnimationSet[i].m_pAnimation[j].m_pBoneName);
        }
    }

    // release the parsed items (since now no longer used)
    csrXReleaseItems(pLocalRoot, 0);
    */

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
    pCollada->m_pSkeleton           = 0;
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

    // release the bones
    csrBoneRelease(pCollada->m_pSkeleton, 0, 1);

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
