/*****************************************************************************
 * ==> Software rasterizer --------------------------------------------------*
 *****************************************************************************
 * Description : A Quake (MDL) model showing a wizard, the model is drawn    *
 *               entirely with a software rasterizer. OpenGL isn't used.     *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// supported platforms check. NOTE iOS only, but may works on other platforms
#if !defined(_OS_IOS_) && !defined(_OS_ANDROID_) && !defined(_OS_WINDOWS_)
    #error "Not supported platform!"
#endif

#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <time.h>

// sdl
#include <SDL.h>

// mini API
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Shader.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_SoftwareRaster.h"
#include "SDK/CSR_MobileC_Debug.h"

// NOTE the mdl model was extracted from the Quake game package
#define MDL_FILE "Resources/wizard.mdl"

//------------------------------------------------------------------------------
CSR_Matrix4      g_ProjectionMatrix;
CSR_Raster       g_Raster;
CSR_MDL*         g_pModel           = 0;
CSR_PixelBuffer* g_pModelTexture    = 0;
CSR_FrameBuffer* g_pFrameBuffer     = 0;
CSR_DepthBuffer* g_pDepthBuffer     = 0;
float            g_Width            = 0.0f;
float            g_Height           = 0.0f;
const float      g_zNear            = 1.0f;
const float      g_zFar             = 200.0f;
double           g_TextureLastTime  = 0.0;
double           g_ModelLastTime    = 0.0;
double           g_MeshLastTime     = 0.0;
const unsigned   g_FPS              = 10;
size_t           g_AnimIndex        = 0;
size_t           g_TextureIndex     = 0;
size_t           g_ModelIndex       = 0;
size_t           g_MeshIndex        = 0;
SDL_Window*      g_pWindow          = 0;
SDL_Texture*     g_pTexture         = 0;
SDL_Renderer*    g_pRenderer        = 0;
SDL_Texture*     g_pTexture         = 0;
//------------------------------------------------------------------------------
void OnTextureRead(size_t index, const CSR_PixelBuffer* pPixelBuffer, int* pNoGPU)
{
    // disable the texture loading on the GPU
    if (pNoGPU)
        *pNoGPU = 1;

    // no pixel buffer?
    if (!pPixelBuffer)
        return;

    // release the previously existing texture, if any
    if (g_pModelTexture)
        csrPixelBufferRelease(g_pModelTexture);

    // copy the pixel buffer content (the source buffer will be released sooner)
    g_pModelTexture = (CSR_PixelBuffer*)malloc(sizeof(CSR_PixelBuffer));
    memcpy(g_pModelTexture, pPixelBuffer, sizeof(CSR_PixelBuffer));

    // copy the texture pixel data
    g_pModelTexture->m_pData = (unsigned char*)malloc(pPixelBuffer->m_DataLength);
    memcpy(g_pModelTexture->m_pData, pPixelBuffer->m_pData, pPixelBuffer->m_DataLength);
}
//------------------------------------------------------------------------------
void OnApplyFragmentShader(const CSR_Matrix4*  pMatrix,
                           const CSR_Polygon3* pPolygon,
                           const CSR_Vector2*  pST,
                           const CSR_Vector3*  pSampler,
                                 float         z,
                                 CSR_Color*    pColor)
{
    float  stX;
    float  stY;
    size_t x;
    size_t y;
    size_t line;

    // limit the texture coordinate between 0 and 1 (equivalent to OpenGL clamp mode)
    csrMathClamp(pST->m_X, 0.0f, 1.0f, &stX);
    csrMathClamp(pST->m_Y, 0.0f, 1.0f, &stY);

    // calculate the x and y coordinate to pick in the texture, and the line length in pixels
    x    = stX * g_pModelTexture->m_Width;
    y    = stY * g_pModelTexture->m_Height;
    line = g_pModelTexture->m_Width * g_pModelTexture->m_BytePerPixel;

    // calculate the pixel index to get
    const size_t index = (y * line) + (x * g_pModelTexture->m_BytePerPixel);

    // get the pixel color from texture
    pColor->m_R = (float)(((unsigned char*)(g_pModelTexture->m_pData))[index])     / 255.0f;
    pColor->m_G = (float)(((unsigned char*)(g_pModelTexture->m_pData))[index + 1]) / 255.0f;
    pColor->m_B = (float)(((unsigned char*)(g_pModelTexture->m_pData))[index + 2]) / 255.0f;
    pColor->m_A = 1.0f;
}
//------------------------------------------------------------------------------
int InitSDL(float width, float height)
{
    g_Width  = width;
    g_Height = height;

    // start SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Error while SDL was initialized: %d\n", SDL_GetError());
        return 0;
    }

    // create a target window
    g_pWindow = SDL_CreateWindow("CSR_software_rasterizer",
                                 0,
                                 0,
                                 width,
                                 height,
                                 SDL_WINDOW_SHOWN);

    // succeeded?
    if (!g_pWindow)
    {
        printf("Error while SDL window was created: %d\n", SDL_GetError());
        return 0;
    }

    // create a window renderer
    g_pRenderer = SDL_CreateRenderer(g_pWindow,
                                    -1,
                                     SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // suceeded?
    if (!g_pRenderer)
    {
        SDL_DestroyWindow(g_pWindow);
        printf("Error while SDL renderer was created: %d\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    // create a target texture
    g_pTexture = SDL_CreateTexture(g_pRenderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_TARGET,
                                   width,
                                   height);

    // suceeded?
    if (!g_pTexture)
    {
        SDL_DestroyRenderer(g_pRenderer);
        SDL_DestroyWindow(g_pWindow);
        printf("Error while SDL target texture was created: %d\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    return 1;
}
//------------------------------------------------------------------------------
void ShutdownSDL()
{
    // release SDL objects
    SDL_DestroyTexture(g_pTexture);
    SDL_DestroyRenderer(g_pRenderer);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();
}
//------------------------------------------------------------------------------
void CreatePerspectiveMatrix(float w, float h)
{
    // calculate matrix items
    const float fov    = 45.0f;
    const float aspect = w / h;

    csrMat4Perspective(fov, aspect, g_zNear, g_zFar, &g_ProjectionMatrix);
}
//------------------------------------------------------------------------------
int OnInit(int width, int height)
{
    CSR_VertexFormat  vertexFormat;
    CSR_VertexCulling vertexCulling;

    // startup SDL
    if (!InitSDL(width, height))
        return 0;

    // create the perspective matrix
    CreatePerspectiveMatrix(width, height);

    // initialize the software rasterizer
    csrRasterInit(&g_Raster);
    g_pFrameBuffer = csrFrameBufferCreate(width, height);
    g_pDepthBuffer = csrDepthBufferCreate(width, height);

    // configure the vertex format
    vertexFormat.m_HasNormal         = 0;
    vertexFormat.m_HasTexCoords      = 1;
    vertexFormat.m_HasPerVertexColor = 0;

    // configure the vertex culling
    vertexCulling.m_Type = CSR_CT_Back;
    vertexCulling.m_Face = CSR_CF_CW;

    // load the MDL model
    g_pModel = csrMDLOpen(MDL_FILE, 0, &vertexFormat, &vertexCulling, 0, 0, OnTextureRead);

    return 1;
}
//------------------------------------------------------------------------------
void OnRelease()
{
    // release the model texture
    if (g_pModelTexture)
        csrPixelBufferRelease(g_pModelTexture);

    // delete the model
    csrMDLRelease(g_pModel);
    g_pModel = 0;

    // shutdown SDL
    ShutdownSDL();
}
//------------------------------------------------------------------------------
void OnUpdate(float elapsedTime)
{
    // calculate next model indexes to show
    csrMDLUpdateIndex(g_pModel,
                      g_FPS,
                      g_AnimIndex,
                     &g_TextureIndex,
                     &g_ModelIndex,
                     &g_MeshIndex,
                     &g_TextureLastTime,
                     &g_ModelLastTime,
                     &g_MeshLastTime,
                      elapsedTime);
}
//------------------------------------------------------------------------------
void OnRender()
{
    CSR_Vector3 t;
    CSR_Vector3 axis;
    CSR_Vector3 factor;
    CSR_Matrix4 translateMatrix;
    CSR_Matrix4 rotateMatrixX;
    CSR_Matrix4 rotateMatrixY;
    CSR_Matrix4 scaleMatrix;
    CSR_Matrix4 combinedMatrixLevel1;
    CSR_Matrix4 combinedMatrixLevel2;
    CSR_Matrix4 modelViewMatrix;
    CSR_Matrix4 matrix;
    CSR_Pixel   pixel;
    CSR_Mesh*   pMesh;
    float       angle;
    GLint       modelviewUniform;
    size_t      x;
    size_t      y;

    pixel.m_R = 0;
    pixel.m_G = 0;
    pixel.m_B = 0;
    pixel.m_A = 255;

    // clear the buffers
    csrFrameBufferClear(g_pFrameBuffer, &pixel);
    csrDepthBufferClear(g_pDepthBuffer, g_zFar);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -150.0f;

    csrMat4Translate(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = M_PI * 0.5;

    csrMat4Rotate(angle, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = M_PI * 0.25;

    csrMat4Rotate(angle, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 0.5f;
    factor.m_Y = 0.5f;
    factor.m_Z = 0.5f;

    csrMat4Scale(&factor, &scaleMatrix);

    // calculate model view matrix
    csrMat4Multiply(&scaleMatrix,          &rotateMatrixX,   &combinedMatrixLevel1);
    csrMat4Multiply(&combinedMatrixLevel1, &rotateMatrixY,   &combinedMatrixLevel2);
    csrMat4Multiply(&combinedMatrixLevel2, &translateMatrix, &modelViewMatrix);

    // build the final matrix
    csrMat4Multiply(&modelViewMatrix, &g_ProjectionMatrix, &matrix);

    // get the current model mesh to draw
    pMesh = csrMDLGetMesh(g_pModel, g_ModelIndex, g_MeshIndex);

    // found it?
    if (!pMesh)
        return;

    // draw the model
    csrRasterDraw(&matrix,
                  g_zNear,
                  g_zFar,
                  pMesh->m_pVB,
                  &g_Raster,
                  g_pFrameBuffer,
                  g_pDepthBuffer,
                  0,
                  OnApplyFragmentShader);

    // begin a SDL drawing
    SDL_SetRenderTarget(g_pRenderer, g_pTexture);
    SDL_SetRenderDrawColor(g_pRenderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(g_pRenderer);

    // copy the frame buffer content in the SDL texture
    for (y = 0; y < g_Height; ++y)
        for (x = 0; x < g_Width; ++x)
        {
            // thaw the pixel
            SDL_SetRenderDrawColor(g_pRenderer,
                                   g_pFrameBuffer->m_pPixel[(y * g_Width) + x].m_R,
                                   g_pFrameBuffer->m_pPixel[(y * g_Width) + x].m_G,
                                   g_pFrameBuffer->m_pPixel[(y * g_Width) + x].m_B,
                                   g_pFrameBuffer->m_pPixel[(y * g_Width) + x].m_A);
            SDL_RenderDrawPoint(g_pRenderer, x, y);
        }

    // end the SDL drawing
    SDL_SetRenderTarget(g_pRenderer, NULL);
    SDL_RenderCopy(g_pRenderer, g_pTexture, NULL, NULL);
    SDL_RenderPresent(g_pRenderer);
}
//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    //SDL_Event   event;
    float       elapsedTime;
    const float ratio = 320.0f / 480.0f;

    if (!OnInit(640.0f, 640.0f * ratio))
        return 1;

    elapsedTime = 0.0f;

    OnUpdate(elapsedTime);
    OnRender();

    while (1)
    {
        /*
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT)
            break;
        */
    }

    OnRelease();
    return 0;
}
//------------------------------------------------------------------------------
