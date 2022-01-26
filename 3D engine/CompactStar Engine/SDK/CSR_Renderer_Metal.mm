/****************************************************************************
 * ==> CSR_Renderer_Metal --------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a base to write a Metal renderer      *
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

#include "CSR_Renderer_Metal.h"

// std
#include <stdlib.h>
#include <memory.h>
#include <string>
#include <vector>
#include <map>

// objective-c
#import <CoreGraphics/CGImage.h>

//---------------------------------------------------------------------------
// Global bridge instance
//---------------------------------------------------------------------------
void* g_pOwner = 0;
//---------------------------------------------------------------------------
// Shader functions
//---------------------------------------------------------------------------
void csrMetalShaderEnable(const void* _Nullable pShader)
{
    [(__bridge id)g_pOwner csrMetalShaderEnable :pShader];
}
//---------------------------------------------------------------------------
void csrMetalShaderConnectProjectionMatrix(const void*        _Nullable pShader,
                                           const CSR_Matrix4* _Nullable pMatrix)
{
    [(__bridge id)g_pOwner csrMetalShaderConnectProjectionMatrix :pShader :pMatrix];
}
//---------------------------------------------------------------------------
void csrMetalShaderConnectViewMatrix(const void*        _Nullable pShader,
                                     const CSR_Matrix4* _Nullable pMatrix)
{
    [(__bridge id)g_pOwner csrMetalShaderConnectViewMatrix :pShader :pMatrix];
}
//---------------------------------------------------------------------------
// Draw functions
//---------------------------------------------------------------------------
void csrMetalDrawBegin(const CSR_Color* _Nullable pColor)
{
    [(__bridge id)g_pOwner csrMetalDrawBegin :pColor];
}
//---------------------------------------------------------------------------
void csrMetalDrawEnd(void)
{
    [(__bridge id)g_pOwner csrMetalDrawEnd];
}
//---------------------------------------------------------------------------
void csrMetalDrawLine(const CSR_Line* _Nullable pLine, const void* _Nullable pShader)
{
    [(__bridge id)g_pOwner csrMetalDrawLine :pLine :pShader];
}
//---------------------------------------------------------------------------
void csrMetalDrawVertexBuffer(const CSR_VertexBuffer* _Nullable pVB,
                              const void*             _Nullable pShader,
                              const CSR_Array*        _Nullable pMatrixArray)
{
    [(__bridge id)g_pOwner csrMetalDrawVertexBuffer :pVB :pShader :pMatrixArray];
}
//---------------------------------------------------------------------------
void csrMetalDrawMesh(const CSR_Mesh*    _Nullable pMesh,
                      const void*        _Nullable pShader,
                      const CSR_Array*   _Nullable pMatrixArray,
                      const CSR_fOnGetID _Nullable fOnGetID)
{
    [(__bridge id)g_pOwner csrMetalDrawMesh :pMesh :pShader :pMatrixArray :fOnGetID];
}
//---------------------------------------------------------------------------
void csrMetalDrawModel(const CSR_Model*   _Nullable pModel,
                             size_t                 index,
                       const void*        _Nullable pShader,
                       const CSR_Array*   _Nullable pMatrixArray,
                       const CSR_fOnGetID _Nullable fOnGetID)
{
    [(__bridge id)g_pOwner csrMetalDrawModel :pModel :index :pShader :pMatrixArray :fOnGetID];
}
//---------------------------------------------------------------------------
void csrMetalDrawMDL(const CSR_MDL*     _Nullable pMDL,
                     const void*        _Nullable pShader,
                     const CSR_Array*   _Nullable pMatrixArray,
                           size_t                 skinIndex,
                           size_t                 modelIndex,
                           size_t                 meshIndex,
                     const CSR_fOnGetID _Nullable fOnGetID)
{
    [(__bridge id)g_pOwner csrMetalDrawMDL :pMDL
                                           :pShader
                                           :pMatrixArray
                                           :skinIndex
                                           :modelIndex
                                           :meshIndex
                                           :fOnGetID];
}
//---------------------------------------------------------------------------
void csrMetalDrawX(const CSR_X*       _Nullable pX,
                   const void*        _Nullable pShader,
                   const CSR_Array*   _Nullable pMatrixArray,
                         size_t                 animSetIndex,
                         size_t                 frameIndex,
                   const CSR_fOnGetID _Nullable fOnGetID)
{
    [(__bridge id)g_pOwner csrMetalDrawX :pX
                                         :pShader
                                         :pMatrixArray
                                         :animSetIndex
                                         :frameIndex
                                         :fOnGetID];
}
//---------------------------------------------------------------------------
void csrMetalDrawCollada(const CSR_Collada* _Nullable pCollada,
                         const void*        _Nullable pShader,
                         const CSR_Array*   _Nullable pMatrixArray,
                               size_t                 animSetIndex,
                               size_t                 frameIndex,
                         const CSR_fOnGetID _Nullable fOnGetID)
{
    [(__bridge id)g_pOwner csrMetalDrawCollada :pCollada
                                               :pShader
                                               :pMatrixArray
                                               :animSetIndex
                                               :frameIndex
                                               :fOnGetID];
}
//---------------------------------------------------------------------------
// State functions
//---------------------------------------------------------------------------
void csrMetalStateEnableDepthMask(int value)
{
    [(__bridge id)g_pOwner csrMetalStateEnableDepthMask :value];
}
//---------------------------------------------------------------------------
// Metal shader
//---------------------------------------------------------------------------
@implementation CSR_MetalShader
{}
//---------------------------------------------------------------------------
@synthesize m_pVertexFunction;
@synthesize m_pFragmentFunction;
//---------------------------------------------------------------------------
- (nonnull instancetype) init :(id<MTLLibrary> _Nonnull)pLibrary
                              :(NSString* _Nonnull)pVertexShaderName
                              :(NSString* _Nonnull)pFragmentShaderName
{
    self = [super init];

    if (self)
    {
        m_pVertexFunction   = [pLibrary newFunctionWithName:pVertexShaderName];
        m_pFragmentFunction = [pLibrary newFunctionWithName:pFragmentShaderName];
    }

    return self;
}
//----------------------------------------------------------------------------
- (void) dealloc
{
    // useless because using ARC
    //[super dealloc];
}
//---------------------------------------------------------------------------
@end
//---------------------------------------------------------------------------
// Metal renderer
//---------------------------------------------------------------------------
typedef std::map<const CSR_VertexBuffer* _Nullable, id<MTLBuffer>>              IVerticesDict;
typedef std::map<const void*             _Nullable, id<MTLTexture>>             ITexturesDict;
typedef std::vector<id<MTLBuffer>>                                              IUniformBuffers;
typedef std::map<const void*             _Nullable, IUniformBuffers>            IUniformDict;
typedef std::map<std::string,                       id<MTLRenderPipelineState>> IRenderPipelineDict;
//---------------------------------------------------------------------------
@interface CSR_MetalBasicRenderer()
{
    IVerticesDict            m_VerticesDict;
    ITexturesDict            m_TexturesDict;
    IUniformDict             m_UniformsDict;
    IUniformBuffers          m_SkyboxUniform;
    IRenderPipelineDict      m_RenderPipelineDict;
    bool                     m_DepthEnabled;
    id<MTLDepthStencilState> m_pDepthState;
}

/**
* Draws a vertex array
*@param pRenderEncoder - render encoder to use to draw the vertex array
*@param pVB - vertex buffer containing the vertex array to draw
*@param vertexCount - vertex count
*@param pUniformKey - uniform key
*/
- (void) csrMetalDrawArray :(id<MTLRenderCommandEncoder>)pRenderEncoder
                           :(const CSR_VertexBuffer* _Nullable)pVB
                           :(size_t)vertexCount
                           :(void* _Nullable)pUniformKey;

@end
//---------------------------------------------------------------------------
@implementation CSR_MetalBasicRenderer
//---------------------------------------------------------------------------
- (nonnull instancetype) initWithMetalKitView :(nonnull MTKView*)pView;
{
    self = [super init];

    if (self)
    {
        g_pOwner             = ((__bridge void*)self);
        m_pDevice            = pView.device;
        m_pRenderEncoder     = nil;
        m_DepthEnabled       = true;
        m_Semaphore          = dispatch_semaphore_create(g_ParallelBufferCount);
        m_UniformBufferIndex = 0;
    }

    return self;
}
//---------------------------------------------------------------------------
- (void) dealloc
{
    // useless because using ARC
    //[super dealloc];
}
//---------------------------------------------------------------------------
- (void) drawInMTKView :(nonnull MTKView*)pView
{}
//---------------------------------------------------------------------------
- (void) mtkView :(nonnull MTKView*)pView drawableSizeWillChange:(CGSize)size
{}
//---------------------------------------------------------------------------
- (void) csrMetalShaderEnable :(const void* _Nullable)pShader
{}
//---------------------------------------------------------------------------
- (void) csrMetalShaderConnectProjectionMatrix :(const void* _Nullable)pShader
                                               :(const CSR_Matrix4* _Nullable)pMatrix;
{
    if (!pMatrix)
        return;

    m_Uniforms.m_ProjectionMatrix = [self CSRMat4ToSIMDMat4 :pMatrix];
}
//---------------------------------------------------------------------------
- (void) csrMetalShaderConnectViewMatrix :(const void* _Nullable)pShader
                                         :(const CSR_Matrix4* _Nullable)pMatrix
{
    if (!pMatrix)
        return;

    m_Uniforms.m_ViewMatrix = [self CSRMat4ToSIMDMat4 :pMatrix];
}
//---------------------------------------------------------------------------
- (void) csrMetalDrawBegin :(const CSR_Color* _Nullable)pColor
{}
//---------------------------------------------------------------------------
- (void) csrMetalDrawEnd
{}
//---------------------------------------------------------------------------
- (void) csrMetalDrawLine :(const CSR_Line* _Nullable)pLine :(const void* _Nullable)pShader
{}
//---------------------------------------------------------------------------
- (void) csrMetalDrawVertexBuffer :(const CSR_VertexBuffer* _Nullable)pVB
                                  :(const void* _Nullable)pShader
                                  :(const CSR_Array* _Nullable)pMatrixArray
{
    // no vertex buffer to draw?
    if (!pVB)
        return;

    // no shader?
    if (!pShader)
        return;

    // check if vertex buffer is empty, skip to next if yes
    if (!pVB->m_Count || !pVB->m_Format.m_Stride)
        return;

    // configure the culling
    switch (pVB->m_Culling.m_Type)
    {
        case CSR_CT_None:  [m_pRenderEncoder setCullMode :MTLCullModeNone];  break;
        case CSR_CT_Front: [m_pRenderEncoder setCullMode :MTLCullModeFront]; break;
        case CSR_CT_Back:  [m_pRenderEncoder setCullMode :MTLCullModeBack];  break;
        case CSR_CT_Both:  @throw @"Unsupported culling mode";
        default:           [m_pRenderEncoder setCullMode :MTLCullModeNone];  break;
    }

    // configure the culling face
    switch (pVB->m_Culling.m_Face)
    {
        case CSR_CF_CW:  [m_pRenderEncoder setFrontFacingWinding :MTLWindingClockwise];        break;
        case CSR_CF_CCW: [m_pRenderEncoder setFrontFacingWinding :MTLWindingCounterClockwise]; break;
    }

    // configure the alpha blending
    if (pVB->m_Material.m_Transparent)
        @throw @"Transparency isn't supported by this renderer for now, please implement it.";

    // configure the wireframe mode
    if (pVB->m_Material.m_Wireframe)
        [m_pRenderEncoder setTriangleFillMode :MTLTriangleFillModeLines];
    else
        [m_pRenderEncoder setTriangleFillMode :MTLTriangleFillModeFill];

    // set the depth enabled state
    MTLDepthStencilDescriptor* pDepthDescriptor = [MTLDepthStencilDescriptor new];
    pDepthDescriptor.depthCompareFunction       = MTLCompareFunctionLessEqual;
    pDepthDescriptor.depthWriteEnabled          = m_DepthEnabled;
    m_pDepthState                               = [m_pDevice newDepthStencilStateWithDescriptor:pDepthDescriptor];

    [m_pRenderEncoder setDepthStencilState:m_pDepthState];

    // calculate the vertex count
    const size_t vertexCount = pVB->m_Count / pVB->m_Format.m_Stride;

    // do draw the vertex buffer several times?
    if (pMatrixArray && pMatrixArray->m_Count)
    {
        // iterate through each matrix to use to draw the vertex buffer
        for (size_t i = 0; i < pMatrixArray->m_Count; ++i)
        {
            // connect the model matrix to the shader
            m_Uniforms.m_ModelMatrix =
                    [self CSRMat4ToSIMDMat4 :static_cast<const CSR_Matrix4*>(pMatrixArray->m_pItem[i].m_pData)];

            // draw the next buffer
            [self csrMetalDrawArray :m_pRenderEncoder :pVB :vertexCount :pMatrixArray->m_pItem[i].m_pData];
        }
    }
    else
        // no, simply draw the buffer without worrying about the model matrix
        [self csrMetalDrawArray :m_pRenderEncoder :pVB :vertexCount :nil];
}
//---------------------------------------------------------------------------
- (void) csrMetalDrawMesh :(const CSR_Mesh* _Nullable)pMesh
                          :(const void* _Nullable)pShader
                          :(const CSR_Array* _Nullable)pMatrixArray
                          :(const CSR_fOnGetID _Nullable)fOnGetID
{
    size_t i;

    // no mesh to draw?
    if (!pMesh)
        return;

    // no shader?
    if (!pShader)
        return;

    // enable the shader to use for drawing
    csrMetalShaderEnable(pShader);

    // iterate through the vertex buffers composing the mesh to draw
    for (i = 0; i < pMesh->m_Count; ++i)
    {
        // in order to link textures, the OnGetID callback should be defined
        if (fOnGetID)
        {
            id<MTLTexture> pTexture;
            id<MTLTexture> pBumpmap;
            id<MTLTexture> pCubemap;

            // vertices have UV texture coordinates?
            if (pMesh->m_pVB[i].m_Format.m_HasTexCoords)
            {
                // get the OpenGL texture and bump map resource identifiers for this mesh
                pTexture = (__bridge id<MTLTexture>)fOnGetID(&pMesh->m_Skin.m_Texture);
                pBumpmap = (__bridge id<MTLTexture>)fOnGetID(&pMesh->m_Skin.m_BumpMap);

                // a texture is defined for this mesh?
                if (pTexture && m_pRenderEncoder)
                    // bind the model texture
                    [m_pRenderEncoder setFragmentTexture:pTexture atIndex:0];

                // a bump map is defined for this mesh?
                if (pBumpmap && m_pRenderEncoder)
                    @throw @"Bump mapping isn't supported by this renderer for now, please implement it.";
            }

            // get the OpenGL cubemap resource identifier for this mesh
            pCubemap = (__bridge id<MTLTexture>)fOnGetID(&pMesh->m_Skin.m_CubeMap);

            // a cube map is defined for this mesh?
            if (pCubemap && m_pRenderEncoder)
                // bind the cubemap texure
                [m_pRenderEncoder setFragmentTexture:pCubemap atIndex:0];
        }

        // draw the next mesh vertex buffer
        [self csrMetalDrawVertexBuffer :&pMesh->m_pVB[i] :pShader :pMatrixArray];
    }
}
//---------------------------------------------------------------------------
- (void) csrMetalDrawModel :(const CSR_Model* _Nullable)pModel
                           :(size_t)index
                           :(const void* _Nullable)pShader
                           :(const CSR_Array* _Nullable)pMatrixArray
                           :(const CSR_fOnGetID _Nullable)fOnGetID
{
    // no model to draw?
    if (!pModel)
        return;

    // draw the model mesh
    [self csrMetalDrawMesh :&pModel->m_pMesh[index % pModel->m_MeshCount]
                           :pShader
                           :pMatrixArray
                           :fOnGetID];
}
//---------------------------------------------------------------------------
- (void) csrMetalDrawMDL :(const CSR_MDL* _Nullable)pMDL
                         :(const void* _Nullable)pShader
                         :(const CSR_Array* _Nullable)pMatrixArray
                         :(size_t)skinIndex
                         :(size_t)modelIndex
                         :(size_t)meshIndex
                         :(const CSR_fOnGetID _Nullable)fOnGetID
{
    // get the current model mesh to draw
    const CSR_Mesh* pMesh = csrMDLGetMesh(pMDL, modelIndex, meshIndex);

    // found it?
    if (!pMesh)
        return;

    // normally each mesh should contain only one vertex buffer
    if (pMesh->m_Count != 1)
        // unsupported if not (because cannot know which texture should be binded. If a such model
        // exists, a custom version of this function should also be written for it)
        return;

    // can use texture?
    if (fOnGetID && pMesh->m_pVB->m_Format.m_HasTexCoords && skinIndex < pMDL->m_SkinCount)
    {
        // get the OpenGL identifier matching with the texture
        const id<MTLTexture> pTexture =
                (__bridge id<MTLTexture>)fOnGetID(&pMDL->m_pSkin[skinIndex].m_Texture);

        // found it?
        if (pTexture && m_pRenderEncoder)
            // bind the model texture
            [m_pRenderEncoder setFragmentTexture:pTexture atIndex:0];
    }

    // draw the model mesh
    [self csrMetalDrawMesh :pMesh :pShader :pMatrixArray :fOnGetID];
}
//---------------------------------------------------------------------------
- (void) csrMetalDrawX :(const CSR_X* _Nullable)pX
                       :(const void* _Nullable)pShader
                       :(const CSR_Array* _Nullable)pMatrixArray
                       :(size_t)animSetIndex
                       :(size_t)frameIndex
                       :(const CSR_fOnGetID _Nullable)fOnGetID
{
    size_t i;
    size_t j;
    size_t k;
    size_t l;

    // no model to draw?
    if (!pX || !pX->m_MeshCount)
        return;

    // do draw only the mesh and ignore all other data like bones?
    if (pX->m_MeshOnly)
    {
        // iterate through the meshes to draw
        for (i = 0; i < pX->m_MeshCount; ++i)
            // draw the model mesh
            [self csrMetalDrawMesh :&pX->m_pMesh[i] :pShader :pMatrixArray :fOnGetID];

        return;
    }

    // iterate through the meshes to draw
    for (i = 0; i < pX->m_MeshCount; ++i)
    {
        int        useLocalMatrixArray;
        int        useSourceBuffer;
        CSR_Mesh*  pMesh;
        CSR_Mesh*  pLocalMesh;
        CSR_Array* pLocalMatrixArray;

        // if mesh has no skeleton, perform a simple draw
        if (!pX->m_pSkeleton)
        {
            // draw the model mesh
            [self csrMetalDrawMesh :&pX->m_pMesh[i] :pShader :pMatrixArray :fOnGetID];
            return;
        }

        // get the current model mesh to draw
        pMesh = &pX->m_pMesh[i];

        // found it?
        if (!pMesh)
            continue;

        // normally each mesh should contain only one vertex buffer
        if (pMesh->m_Count != 1)
            // unsupported if not (because cannot know which texture should be binded. If a such model
            // exists, a custom version of this function should also be written for it)
            continue;

        // create a local mesh to contain the processed frame to draw
        pLocalMesh = csrMeshCreate();

        if (!pLocalMesh)
            continue;

        csrMeshInit(pLocalMesh);
        
        // bind the source mesh to the local one. Don't need to take care of copy the pointers, because
        // the source mesh will remain valid during the whole local mesh lifetime. Just don't delete
        // them on the loop end
        pLocalMesh->m_Skin = pMesh->m_Skin;
        pLocalMesh->m_Time = pMesh->m_Time;

        // mesh contains skin weights?
        if (pX->m_pMeshWeights[i].m_pSkinWeights)
        {
            useSourceBuffer = 0;

            // allocate memory for the final vertex buffer to draw
            pLocalMesh->m_pVB   = (CSR_VertexBuffer*)malloc(pMesh->m_Count * sizeof(CSR_VertexBuffer));
            pLocalMesh->m_Count = pMesh->m_Count;

            if (!pLocalMesh->m_pVB || !pLocalMesh->m_Count)
            {
                free(pLocalMesh);
                continue;
            }

            // bind the source vertex buffer to the local one
            pLocalMesh->m_pVB->m_Format   = pMesh->m_pVB->m_Format;
            pLocalMesh->m_pVB->m_Culling  = pMesh->m_pVB->m_Culling;
            pLocalMesh->m_pVB->m_Material = pMesh->m_pVB->m_Material;
            pLocalMesh->m_pVB->m_Time     = pMesh->m_pVB->m_Time;

            // allocate memory for the vertex buffer data
            pLocalMesh->m_pVB->m_pData = (float*)calloc(pMesh->m_pVB->m_Count, sizeof(float));
            pLocalMesh->m_pVB->m_Count = pMesh->m_pVB->m_Count;

            if (!pLocalMesh->m_pVB->m_pData || !pLocalMesh->m_pVB->m_Count)
            {
                free(pLocalMesh->m_pVB);
                free(pLocalMesh);
                continue;
            }

            // do copy the texture file name? NOTE the texture file name may be used as a key
            // to retrieve the associated texture in the resources
            if (pMesh->m_Skin.m_Texture.m_pFileName)
            {
                // measure the file name length and allocate memory for file name in local mesh
                const size_t fileNameLen                 = strlen(pMesh->m_Skin.m_Texture.m_pFileName);
                pLocalMesh->m_Skin.m_Texture.m_pFileName = (char*)calloc(fileNameLen + 1, sizeof(char));

                // copy the file name
                if (pLocalMesh->m_Skin.m_Texture.m_pFileName)
                    memcpy(pLocalMesh->m_Skin.m_Texture.m_pFileName, pMesh->m_Skin.m_Texture.m_pFileName, fileNameLen);
            }

            // iterate through mesh skin weights
            for (j = 0; j < pX->m_pMeshWeights[i].m_Count; ++j)
            {
                CSR_Matrix4 boneMatrix;
                CSR_Matrix4 finalMatrix;

                // get the bone matrix
                if (pX->m_PoseOnly)
                    csrBoneGetMatrix(pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone, 0, &boneMatrix);
                else
                    csrBoneGetAnimMatrix(pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone,
                                        &pX->m_pAnimationSet[animSetIndex],
                                         frameIndex,
                                         0,
                                        &boneMatrix);

                // get the final matrix after bones transform
                csrMat4Multiply(&pX->m_pMeshWeights[i].m_pSkinWeights[j].m_Matrix,
                                &boneMatrix,
                                &finalMatrix);

                // apply the bone and its skin weights to each vertices
                for (k = 0; k < pX->m_pMeshWeights[i].m_pSkinWeights[j].m_IndexTableCount; ++k)
                    for (l = 0; l < pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_Count; ++l)
                    {
                        size_t      iX;
                        size_t      iY;
                        size_t      iZ;
                        CSR_Vector3 inputVertex;
                        CSR_Vector3 outputVertex;

                        // get the next vertex to which the next skin weight should be applied
                        iX = pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l];
                        iY = pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 1;
                        iZ = pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 2;

                        // get input vertex
                        inputVertex.m_X = pMesh->m_pVB->m_pData[iX];
                        inputVertex.m_Y = pMesh->m_pVB->m_pData[iY];
                        inputVertex.m_Z = pMesh->m_pVB->m_pData[iZ];

                        // apply bone transformation to vertex
                        csrMat4Transform(&finalMatrix, &inputVertex, &outputVertex);

                        // apply the skin weights and calculate the final output vertex
                        pLocalMesh->m_pVB->m_pData[iX] += (outputVertex.m_X * pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iY] += (outputVertex.m_Y * pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iZ] += (outputVertex.m_Z * pX->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);

                        // copy the remaining vertex data
                        if (pMesh->m_pVB->m_Format.m_Stride > 3)
                        {
                            const size_t copyIndex = iZ + 1;

                            memcpy(&pLocalMesh->m_pVB->m_pData[copyIndex],
                                   &pMesh->m_pVB->m_pData[copyIndex],
                                    ((size_t)pMesh->m_pVB->m_Format.m_Stride - 3) * sizeof(float));
                        }
                    }
            }
        }
        else
        {
            useSourceBuffer = 1;

            // no weights, just use the existing vertex buffer
            pLocalMesh->m_pVB   = pMesh->m_pVB;
            pLocalMesh->m_Count = pMesh->m_Count;
        }

        // get vertices to update
        IVerticesDict::const_iterator itVert = m_VerticesDict.find(pMesh->m_pVB);

        // update the vertex buffer with the model print content
        if (itVert != m_VerticesDict.end())
        {
            float* pVertices = (float*)itVert->second.contents;
            std::memcpy(pVertices,
                        pLocalMesh->m_pVB->m_pData,
                        pLocalMesh->m_pVB->m_Count * sizeof(float));
        }

        // delete the local vertex buffer
        if (!useSourceBuffer)
        {
            free(pLocalMesh->m_pVB->m_pData);
            free(pLocalMesh->m_pVB);
        }

        // delete the local mesh texture file name
        if (pLocalMesh->m_Skin.m_Texture.m_pFileName)
            free(pLocalMesh->m_Skin.m_Texture.m_pFileName);

        // delete the local mesh
        free(pLocalMesh);

        useLocalMatrixArray = 0;

        // has matrix array to transform, and model contain mesh bones?
        if (pMatrixArray && pMatrixArray->m_Count && pX->m_pMeshToBoneDict[i].m_pBone)
        {
            // create a new local matrix array
            pLocalMatrixArray = (CSR_Array*)malloc(sizeof(CSR_Array));
            csrArrayInit(pLocalMatrixArray);
            useLocalMatrixArray = 1;

            // create as array item as in the source matrix list
            pLocalMatrixArray->m_pItem =
                    (CSR_ArrayItem*)malloc(sizeof(CSR_ArrayItem) * pMatrixArray->m_Count);

            // succeeded?
            if (pLocalMatrixArray->m_pItem)
            {
                // update array count
                pLocalMatrixArray->m_Count = pMatrixArray->m_Count;

                // iterate through source model matrices
                for (j = 0; j < pMatrixArray->m_Count; ++j)
                {
                    CSR_Matrix4 swapMatrix;

                    // initialize the local matrix array item
                    pLocalMatrixArray->m_pItem[j].m_AutoFree = 1;
                    pLocalMatrixArray->m_pItem[j].m_pData    = malloc(sizeof(CSR_Matrix4));

                    // get the final matrix after bones transform
                    csrBoneGetMatrix(pX->m_pMeshToBoneDict[i].m_pBone,
                                     (CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData,
                                     (CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData);

                    // swap the matrices content between matrix array and local matrix array. This
                    // is required because the draw function will retrieve the shader reference
                    // later by using the matrix array pointer
                    swapMatrix                                           = *(CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData;
                    *(CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData      =
                            *(CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData;
                    *(CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData = swapMatrix;
                }
            }
        }
        else
            // no matrix array or no bone, keep the original array
            pLocalMatrixArray = (CSR_Array*)pMatrixArray;

        // draw the model mesh
        [self csrMetalDrawMesh :pMesh :pShader :pMatrixArray :fOnGetID];

        // release the transformed matrix list
        if (useLocalMatrixArray)
        {
            // restore the source model matrices
            for (j = 0; j < pMatrixArray->m_Count; ++j)
                *(CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData =
                        *(CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData;

            csrArrayRelease(pLocalMatrixArray);
        }
    }

    return;
}
//---------------------------------------------------------------------------
- (void) csrMetalDrawCollada :(const CSR_Collada* _Nullable)pCollada
                             :(const void* _Nullable)pShader
                             :(const CSR_Array* _Nullable)pMatrixArray
                             :(size_t)animSetIndex
                             :(size_t)frameIndex
                             :(const CSR_fOnGetID _Nullable)fOnGetID
{
    size_t i;
    size_t j;
    size_t k;
    size_t l;

    // no model to draw?
    if (!pCollada || !pCollada->m_MeshCount)
        return;

    // do draw only the mesh and ignore all other data like bones?
    if (pCollada->m_MeshOnly || !pCollada->m_pMeshWeights)
    {
        // iterate through the meshes to draw
        for (i = 0; i < pCollada->m_MeshCount; ++i)
            // draw the model mesh
            [self csrMetalDrawMesh :&pCollada->m_pMesh[i] :pShader :pMatrixArray :fOnGetID];

        return;
    }

    // iterate through the meshes to draw
    for (i = 0; i < pCollada->m_MeshCount; ++i)
    {
        int        useLocalMatrixArray;
        int        useSourceBuffer;
        CSR_Mesh*  pMesh;
        CSR_Mesh*  pLocalMesh;
        CSR_Array* pLocalMatrixArray;

        // if mesh has no skeleton, perform a simple draw
        if (!pCollada->m_pSkeletons)
        {
            // draw the model mesh
            [self csrMetalDrawMesh :&pCollada->m_pMesh[i] :pShader :pMatrixArray :fOnGetID];
            return;
        }

        // get the current model mesh to draw
        pMesh = &pCollada->m_pMesh[i];

        // found it?
        if (!pMesh)
            continue;

        // normally each mesh should contain only one vertex buffer
        if (pMesh->m_Count != 1)
            // unsupported if not (because cannot know which texture should be binded. If a such model
            // exists, a custom version of this function should also be written for it)
            continue;

        // create a local mesh to contain the processed frame to draw
        pLocalMesh = csrMeshCreate();

        if (!pLocalMesh)
            continue;

        // bind the source mesh to the local one. Don't need to take care of copy the pointers, because
        // the source mesh will remain valid during the whole local mesh lifetime. Just don't delete
        // them on the loop end
        pLocalMesh->m_Skin = pMesh->m_Skin;
        pLocalMesh->m_Time = pMesh->m_Time;

        // mesh contains skin weights?
        if (pCollada->m_pMeshWeights[i].m_pSkinWeights)
        {
            useSourceBuffer = 0;

            // allocate memory for the final vertex buffer to draw
            pLocalMesh->m_pVB   = (CSR_VertexBuffer*)malloc(pMesh->m_Count * sizeof(CSR_VertexBuffer));
            pLocalMesh->m_Count = pMesh->m_Count;

            if (!pLocalMesh->m_pVB || !pLocalMesh->m_Count)
            {
                free(pLocalMesh);
                continue;
            }

            // bind the source vertex buffer to the local one
            pLocalMesh->m_pVB->m_Format   = pMesh->m_pVB->m_Format;
            pLocalMesh->m_pVB->m_Culling  = pMesh->m_pVB->m_Culling;
            pLocalMesh->m_pVB->m_Material = pMesh->m_pVB->m_Material;
            pLocalMesh->m_pVB->m_Time     = pMesh->m_pVB->m_Time;

            // allocate memory for the vertex buffer data
            pLocalMesh->m_pVB->m_pData = (float*)calloc(pMesh->m_pVB->m_Count, sizeof(float));
            pLocalMesh->m_pVB->m_Count = pMesh->m_pVB->m_Count;

            if (!pLocalMesh->m_pVB->m_pData || !pLocalMesh->m_pVB->m_Count)
            {
                free(pLocalMesh->m_pVB);
                free(pLocalMesh);
                continue;
            }

            // do copy the texture file name? NOTE the texture file name may be used as a key
            // to retrieve the associated texture in the resources
            if (pMesh->m_Skin.m_Texture.m_pFileName)
            {
                // measure the file name length and allocate memory for file name in local mesh
                const size_t fileNameLen                 = strlen(pMesh->m_Skin.m_Texture.m_pFileName);
                pLocalMesh->m_Skin.m_Texture.m_pFileName = (char*)calloc(fileNameLen + 1, sizeof(char));

                // copy the file name
                if (pLocalMesh->m_Skin.m_Texture.m_pFileName)
                    memcpy(pLocalMesh->m_Skin.m_Texture.m_pFileName, pMesh->m_Skin.m_Texture.m_pFileName, fileNameLen);
            }

            // iterate through mesh skin weights
            for (j = 0; j < pCollada->m_pMeshWeights[i].m_Count; ++j)
            {
                CSR_Matrix4 boneMatrix;
                CSR_Matrix4 finalMatrix;

                // get the bone matrix
                if (pCollada->m_PoseOnly)
                    csrBoneGetMatrix(pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone,
                                    &pCollada->m_pSkeletons->m_InitialMatrix,
                                    &boneMatrix);
                else
                    csrBoneGetAnimMatrix(pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pBone,
                                        &pCollada->m_pAnimationSet[animSetIndex],
                                         frameIndex,
                                        &pCollada->m_pSkeletons->m_InitialMatrix,
                                        &boneMatrix);

                // get the final matrix after bones transform
                csrMat4Multiply(&pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_Matrix,
                                &boneMatrix,
                                &finalMatrix);

                // apply the bone and its skin weights to each vertices
                for (k = 0; k < pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_IndexTableCount; ++k)
                    for (l = 0; l < pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_Count; ++l)
                    {
                        size_t      iX;
                        size_t      iY;
                        size_t      iZ;
                        CSR_Vector3 inputVertex;
                        CSR_Vector3 outputVertex;

                        // get the next vertex to which the next skin weight should be applied
                        iX = pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l];
                        iY = pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 1;
                        iZ = pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pIndexTable[k].m_pData[l] + 2;

                        // get input vertex
                        inputVertex.m_X = pMesh->m_pVB->m_pData[iX];
                        inputVertex.m_Y = pMesh->m_pVB->m_pData[iY];
                        inputVertex.m_Z = pMesh->m_pVB->m_pData[iZ];

                        // apply bone transformation to vertex
                        csrMat4Transform(&finalMatrix, &inputVertex, &outputVertex);

                        // apply the skin weights and calculate the final output vertex
                        pLocalMesh->m_pVB->m_pData[iX] += (outputVertex.m_X * pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iY] += (outputVertex.m_Y * pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);
                        pLocalMesh->m_pVB->m_pData[iZ] += (outputVertex.m_Z * pCollada->m_pMeshWeights[i].m_pSkinWeights[j].m_pWeights[k]);

                        // copy the remaining vertex data
                        if (pMesh->m_pVB->m_Format.m_Stride > 3)
                        {
                            const size_t copyIndex = iZ + 1;

                            memcpy(&pLocalMesh->m_pVB->m_pData[copyIndex],
                                   &pMesh->m_pVB->m_pData[copyIndex],
                                    ((size_t)pMesh->m_pVB->m_Format.m_Stride - 3) * sizeof(float));
                        }
                    }
            }
        }
        else
        {
            useSourceBuffer = 1;

            // no weights, just use the existing vertex buffer
            pLocalMesh->m_pVB   = pMesh->m_pVB;
            pLocalMesh->m_Count = pMesh->m_Count;
        }

        // get vertices to update
        IVerticesDict::const_iterator itVert = m_VerticesDict.find(pMesh->m_pVB);

        // update the vertex buffer with the model print content
        if (itVert != m_VerticesDict.end())
        {
            float* pVertices = (float*)itVert->second.contents;
            std::memcpy(pVertices,
                        pLocalMesh->m_pVB->m_pData,
                        pLocalMesh->m_pVB->m_Count * sizeof(float));
        }

        // delete the local vertex buffer
        if (!useSourceBuffer)
        {
            free(pLocalMesh->m_pVB->m_pData);
            free(pLocalMesh->m_pVB);
        }

        // delete the local mesh texture file name
        if (pLocalMesh->m_Skin.m_Texture.m_pFileName)
            free(pLocalMesh->m_Skin.m_Texture.m_pFileName);

        // delete the local mesh
        free(pLocalMesh);

        useLocalMatrixArray = 0;

        // has matrix array to transform, and model contain mesh bones?
        if (pMatrixArray                &&
            pMatrixArray->m_Count       &&
            pCollada->m_pMeshToBoneDict &&
            pCollada->m_pMeshToBoneDict[i].m_pBone)
        {
            // create a new local matrix array
            pLocalMatrixArray = (CSR_Array*)malloc(sizeof(CSR_Array));
            csrArrayInit(pLocalMatrixArray);
            useLocalMatrixArray = 1;

            // create as array item as in the source matrix list
            pLocalMatrixArray->m_pItem =
                    (CSR_ArrayItem*)malloc(sizeof(CSR_ArrayItem) * pMatrixArray->m_Count);

            // succeeded?
            if (pLocalMatrixArray->m_pItem)
            {
                // update array count
                pLocalMatrixArray->m_Count = pMatrixArray->m_Count;

                // iterate through source model matrices
                for (j = 0; j < pMatrixArray->m_Count; ++j)
                {
                    CSR_Matrix4 swapMatrix;

                    // initialize the local matrix array item
                    pLocalMatrixArray->m_pItem[j].m_AutoFree = 1;
                    pLocalMatrixArray->m_pItem[j].m_pData    = malloc(sizeof(CSR_Matrix4));

                    // get the final matrix after bones transform
                    csrBoneGetMatrix(pCollada->m_pMeshToBoneDict[i].m_pBone,
                                     (CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData,
                                     (CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData);

                    // swap the matrices content between matrix array and local matrix array. This
                    // is required because the draw function will retrieve the shader reference
                    // later by using the matrix array pointer
                    swapMatrix                                           = *(CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData;
                    *(CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData      =
                            *(CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData;
                    *(CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData = swapMatrix;
                }
            }
        }
        else
            // no matrix array or no bone, keep the original array
            pLocalMatrixArray = (CSR_Array*)pMatrixArray;

        // draw the model mesh
        [self csrMetalDrawMesh :pMesh :pShader :pLocalMatrixArray :fOnGetID];

        // release the transformed matrix list
        if (useLocalMatrixArray)
        {
            // restore the source model matrices
            for (j = 0; j < pMatrixArray->m_Count; ++j)
                *(CSR_Matrix4*)pMatrixArray->m_pItem[j].m_pData =
                        *(CSR_Matrix4*)pLocalMatrixArray->m_pItem[j].m_pData;

            csrArrayRelease(pLocalMatrixArray);
        }
    }
}
//---------------------------------------------------------------------------
- (void) csrMetalStateEnableDepthMask :(int)value
{
    m_DepthEnabled = value;
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromMDL :(const CSR_MDL* _Nullable)pMDL
{
    if (!pMDL)
        return;

    for (size_t i = 0; i < pMDL->m_ModelCount; ++i)
        [self CreateBufferFromModel :&pMDL->m_pModel[i] : false];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromX :(const CSR_X* _Nullable)pX
{
    if (!pX)
        return;

    for (size_t i = 0; i < pX->m_MeshCount; ++i)
        [self CreateBufferFromMesh :&pX->m_pMesh[i] :!pX->m_MeshOnly];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromCollada :(const CSR_Collada* _Nullable)pCollada
{
    if (!pCollada)
        return;

    for (size_t i = 0; i < pCollada->m_MeshCount; ++i)
        [self CreateBufferFromMesh :&pCollada->m_pMesh[i] :!pCollada->m_MeshOnly];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromModel :(const CSR_Model* _Nullable)pModel :(bool)shared
{
    if (!pModel)
        return;

    for (size_t i = 0; i < pModel->m_pMesh->m_Count; ++i)
        [self CreateBufferFromMesh :&pModel->m_pMesh[i] :shared];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromMesh :(const CSR_Mesh* _Nullable)pMesh :(bool)shared
{
    if (!pMesh)
        return;

    for (size_t i = 0; i < pMesh->m_Count; ++i)
        [self CreateBufferFromVB :&pMesh->m_pVB[i] :shared];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromVB :(const CSR_VertexBuffer* _Nullable)pVB :(bool)shared
{
    MTLResourceOptions vbOptions;

    if (shared)
        vbOptions = MTLResourceStorageModeShared;
    else
        vbOptions = MTLResourceCPUCacheModeDefaultCache;

    // create a metal vertex buffer from the compactStar engine one
    id<MTLBuffer> pVertexBuffer = [m_pDevice newBufferWithBytes:pVB->m_pData
                                                         length:pVB->m_Count * sizeof(float)
                                                        options:vbOptions];

    // keep the newly created vertex buffer reference in the vertices dictionary
    m_VerticesDict[pVB] = pVertexBuffer;
}
//---------------------------------------------------------------------------
- (bool) CreateTexture :(void* _Nullable)pKey :(nonnull NSURL*)pUrl
{
    if (!pKey)
        return false;

    if (!pUrl)
        return false;

    // configure texture loader options
    NSDictionary* pTextureLoaderOptions =
    @{
        MTKTextureLoaderOptionTextureUsage:       @(MTLTextureUsageShaderRead),
        MTKTextureLoaderOptionTextureStorageMode: @(MTLStorageModePrivate)
    };

    // load the texture
    NSError*          pError         = NULL;
    MTKTextureLoader* pTextureLoader = [[MTKTextureLoader alloc]initWithDevice :m_pDevice];
    id<MTLTexture>    pTexture       = [pTextureLoader newTextureWithContentsOfURL:pUrl options:pTextureLoaderOptions error:&pError];

    // succeeded?
    if (!pTexture || pError)
    {
        NSLog(@"Error creating texture %@", pError.localizedDescription);
        return false;
    }

    // register the texture
    m_TexturesDict[pKey] = pTexture;

    return true;
}
//---------------------------------------------------------------------------
- (bool) CreateCubemapTexture :(void* _Nullable)pKey :(NSArray* _Nullable)pImageNames
{
    if (!pKey)
        return false;

    if (!pImageNames)
        return false;

    // load the first cubemap texture image and get its size
    UIImage*      pFirstImage = [UIImage imageNamed:[pImageNames firstObject]];
    const CGFloat cubeSize    = pFirstImage.size.width * pFirstImage.scale;

    // configure the byte per pixels, byte ber rows and byte per image values
    const NSUInteger bytesPerPixel = 4;
    const NSUInteger bytesPerRow   = bytesPerPixel * cubeSize;
    const NSUInteger bytesPerImage = bytesPerRow   * cubeSize;

    // create a compatible copy region
    MTLRegion region = MTLRegionMake2D(0, 0, cubeSize, cubeSize);

    // create the texture descriptor
    MTLTextureDescriptor* pTextureDescriptor =
            [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                  size:cubeSize
                                                             mipmapped:NO];

    // create the texture
    id<MTLTexture> pTexture = [m_pDevice newTextureWithDescriptor:pTextureDescriptor];

    // iterate through images to load
    for (std::size_t slice = 0; slice < 6; ++slice)
    {
        // load the image
        NSString*  pImageName = pImageNames[slice];
        UIImage*   pImage     = [UIImage imageNamed:pImageName];
        CGImageRef imageRef   = [pImage CGImage];

        // create a suitable bitmap context for extracting the image bits
        const NSUInteger      width            = CGImageGetWidth(imageRef);
        const NSUInteger      height           = CGImageGetHeight(imageRef);
              CGColorSpaceRef colorSpace       = CGColorSpaceCreateDeviceRGB();
              uint8_t*        pRawData         = (uint8_t *)calloc(height * width * 4, sizeof(uint8_t));
        const NSUInteger      bytesPerPixel    = 4;
        const NSUInteger      bytesPerRow      = bytesPerPixel * width;
        const NSUInteger      bitsPerComponent = 8;
              CGContextRef    context          = CGBitmapContextCreate(pRawData,
                                                                       width,
                                                                       height,
                                                                       bitsPerComponent,
                                                                       bytesPerRow,
                                                                       colorSpace,
                                                                       kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
        CGColorSpaceRelease(colorSpace);
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
        CGContextRelease(context);

        // get the image bytes and certify the image is a squared image
        uint8_t* pImageData = pRawData;
        NSAssert(pImage.size.width  == cubeSize &&
                 pImage.size.height == cubeSize,
                 @"Cube map images must be square and uniformly-sized");

        // copy the image copntent into the texture
        [pTexture replaceRegion:region
                    mipmapLevel:0
                          slice:slice
                      withBytes:pImageData
                    bytesPerRow:bytesPerRow
                  bytesPerImage:bytesPerImage];

        free(pImageData);
    }

    // register the texture
    m_TexturesDict[pKey] = pTexture;

    return true;
}
//---------------------------------------------------------------------------
- (void) CreateUniform :(const void* _Nullable)pKey
{
    IUniformBuffers buffers;

    for (NSUInteger i = 0; i < g_ParallelBufferCount; ++i)
    {
        id<MTLBuffer> buffer;
        buffer       = [m_pDevice newBufferWithLength:sizeof(CSR_Uniforms)
                                              options:MTLResourceStorageModeShared];
        buffer.label = [NSString stringWithFormat:@"CSR_UniformBuffer (%lu)", (unsigned long)i];
        buffers.push_back(buffer);
    }

    m_UniformsDict[pKey] = buffers;
}
//---------------------------------------------------------------------------
- (void) CreateSkyboxUniform
{
    for (NSUInteger i = 0; i < g_ParallelBufferCount; ++i)
    {
        id<MTLBuffer> buffer;
        buffer       = [m_pDevice newBufferWithLength:sizeof(CSR_Uniforms)
                                              options:MTLResourceStorageModeShared];
        buffer.label = [NSString stringWithFormat:@"CSR_UniformBuffer (%lu)", (unsigned long)i];
        m_SkyboxUniform.push_back(buffer);
    }
}
//---------------------------------------------------------------------------
- (bool) CreateRenderPipeline :(nonnull MTKView*)pView
                              :(nonnull NSString*)pKey
                              :(CSR_MetalShader* _Nullable)pShader
{
    if (!pView)
        return false;

    if (!pShader)
        return false;

    NSString* pName = [NSString stringWithFormat:@"CSR_Pipeline (%@)", pKey];

    // create the Metal render pipeline
    MTLRenderPipelineDescriptor* pPipelineStateDescriptor    = [[MTLRenderPipelineDescriptor alloc]init];
    pPipelineStateDescriptor.label                           = pName;
    pPipelineStateDescriptor.sampleCount                     = pView.sampleCount;
    pPipelineStateDescriptor.vertexFunction                  = pShader.m_pVertexFunction;
    pPipelineStateDescriptor.fragmentFunction                = pShader.m_pFragmentFunction;
    pPipelineStateDescriptor.colorAttachments[0].pixelFormat = pView.colorPixelFormat;
    pPipelineStateDescriptor.depthAttachmentPixelFormat      = pView.depthStencilPixelFormat;
    pPipelineStateDescriptor.stencilAttachmentPixelFormat    = pView.depthStencilPixelFormat;

    NSError*                   pError         = NULL;
    id<MTLRenderPipelineState> pPipelineState =
            [m_pDevice newRenderPipelineStateWithDescriptor:pPipelineStateDescriptor error:&pError];

    if (!pPipelineState)
    {
        NSLog(@"Failed to created pipeline state, error %@", pError);
        return false;
    }

    const char*       pStr = [pKey cStringUsingEncoding:NSUTF8StringEncoding];
    const std::string key  = pStr;

    m_RenderPipelineDict[key] = pPipelineState;
    return true;
}
//---------------------------------------------------------------------------
-(id<MTLTexture> _Nullable) GetTexture :(const void* _Nullable)pKey
{
    ITexturesDict::const_iterator it = m_TexturesDict.find(pKey);

    if (it == m_TexturesDict.end())
        return nil;

    return it->second;
}
//---------------------------------------------------------------------------
-(id<MTLRenderPipelineState> _Nullable) GetRenderPipeline :(nonnull NSString*)pKey
{
    const char*       pStr = [pKey cStringUsingEncoding:NSUTF8StringEncoding];
    const std::string key  = pStr;

    IRenderPipelineDict::const_iterator it = m_RenderPipelineDict.find(key);

    if (it == m_RenderPipelineDict.end())
        return nil;

    return it->second;
}
//---------------------------------------------------------------------------
- (matrix_float4x4) CSRMat4ToSIMDMat4 :(const CSR_Matrix4* _Nullable)pMatrix
{
    if (!pMatrix)
        return (matrix_float4x4)
        {{
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        }};

    return (matrix_float4x4)
    {{
        {pMatrix->m_Table[0][0], pMatrix->m_Table[0][1], pMatrix->m_Table[0][2], pMatrix->m_Table[0][3]},
        {pMatrix->m_Table[1][0], pMatrix->m_Table[1][1], pMatrix->m_Table[1][2], pMatrix->m_Table[1][3]},
        {pMatrix->m_Table[2][0], pMatrix->m_Table[2][1], pMatrix->m_Table[2][2], pMatrix->m_Table[2][3]},
        {pMatrix->m_Table[3][0], pMatrix->m_Table[3][1], pMatrix->m_Table[3][2], pMatrix->m_Table[3][3]}
    }};
}
//---------------------------------------------------------------------------
- (void) csrMetalDrawArray :(id<MTLRenderCommandEncoder>)pRenderEncoder
                           :(const CSR_VertexBuffer* _Nullable)pVB
                           :(size_t)vertexCount
                           :(void* _Nullable)pUniformKey
{
    // no render encoder?
    if (!pRenderEncoder)
        return;

    IVerticesDict::const_iterator itVert = m_VerticesDict.find(pVB);

    if (itVert == m_VerticesDict.end())
        return;

    IUniformDict::const_iterator itUniform = m_UniformsDict.find(pUniformKey);

    if (itUniform != m_UniformsDict.end())
    {
        // copy the uniform buffer content on the GPU (weak but don't know how to do that better)
        void* pBufferPointer = itUniform->second[m_UniformBufferIndex].contents;
        memcpy(pBufferPointer, &m_Uniforms, sizeof(CSR_Uniforms));

        // configure the draw call
        [pRenderEncoder setVertexBuffer:itVert->second                          offset:0 atIndex:0];
        [pRenderEncoder setVertexBuffer:itUniform->second[m_UniformBufferIndex] offset:0 atIndex:1];
    }
    else
    if (!m_SkyboxUniform.empty())
    {
        // copy the skybox uniform buffer content on the GPU (weak but don't know how to do that better)
        void* pBufferPointer = m_SkyboxUniform[m_UniformBufferIndex].contents;
        memcpy(pBufferPointer, &m_Uniforms, sizeof(CSR_Uniforms));

        // configure the draw call
        [pRenderEncoder setVertexBuffer:itVert->second                        offset:0 atIndex:0];
        [pRenderEncoder setVertexBuffer:m_SkyboxUniform[m_UniformBufferIndex] offset:0 atIndex:1];
    }
    else
        return;

    // search for array type to draw
    switch (pVB->m_Format.m_Type)
    {
        case CSR_VT_Triangles:
            [pRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                               vertexStart:0
                               vertexCount:vertexCount];
            return;

        case CSR_VT_TriangleStrip:
            [pRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                               vertexStart:0
                               vertexCount:vertexCount];
            return;

        case CSR_VT_TriangleFan:
            @throw @"Unsupported format type - CSR_VT_TriangleFan";

        default:
            return;
    }
}
//---------------------------------------------------------------------------
@end
//---------------------------------------------------------------------------
