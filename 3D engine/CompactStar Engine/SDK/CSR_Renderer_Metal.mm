/****************************************************************************
 * ==> CSR_Renderer_Metal --------------------------------------------------*
 ****************************************************************************
 * Description : This module provides a base to write a Metal renderer      *
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
    IVerticesDict       m_VerticesDict;
    ITexturesDict       m_TexturesDict;
    IUniformDict        m_UniformsDict;
    IUniformBuffers     m_SkyboxUniform;
    IRenderPipelineDict m_RenderPipelineDict;
    bool                m_DepthEnabled;
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
    
    MTLDepthStencilDescriptor* pDepthDescriptor = [MTLDepthStencilDescriptor new];
    pDepthDescriptor.depthCompareFunction       = MTLCompareFunctionLessEqual;
    pDepthDescriptor.depthWriteEnabled          = m_DepthEnabled;
    id<MTLDepthStencilState>   pDepthState      = [m_pDevice newDepthStencilStateWithDescriptor:pDepthDescriptor];
    
    [m_pRenderEncoder setDepthStencilState:pDepthState];

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
        [self CreateBufferFromModel :&pMDL->m_pModel[i]];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromModel :(const CSR_Model* _Nullable)pModel
{
    if (!pModel)
        return;

    for (size_t i = 0; i < pModel->m_pMesh->m_Count; ++i)
        [self CreateBufferFromMesh :&pModel->m_pMesh[i]];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromMesh :(const CSR_Mesh* _Nullable)pMesh
{
    if (!pMesh)
        return;

    for (size_t i = 0; i < pMesh->m_Count; ++i)
        [self CreateBufferFromVB :&pMesh->m_pVB[i]];
}
//---------------------------------------------------------------------------
- (void) CreateBufferFromVB :(const CSR_VertexBuffer* _Nullable)pVB
{
    // create a metal vertex buffer from the compactStar engine one
    id<MTLBuffer> pVertexBuffer = [m_pDevice newBufferWithBytes:pVB->m_pData
                                                         length:pVB->m_Count * sizeof(float)
                                                        options:MTLResourceOptionCPUCacheModeDefault];

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
    
    // load the first cubemap texture image and get his size
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
