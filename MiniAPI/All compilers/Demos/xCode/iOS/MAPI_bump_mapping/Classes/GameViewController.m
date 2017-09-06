/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A bump mapped stone wall with diffuse light, swipe on       *
 *               screen to modify the light position                         *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#import "GameViewController.h"

// openGL
#import <OpenGLES/ES2/glext.h>

// mini api
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

#import "MiniObjectiveCHelper.h"

//------------------------------------------------------------------------------
const char* m_pVSDiffuseBumpMap =
    "precision mediump float;"
    "attribute vec4 qr_vPosition;"
    "attribute vec4 qr_vColor;"
    "attribute vec2 qr_vTexCoord;"
    "uniform   vec3 qr_vLightPos;"
    "uniform   mat4 qr_uProjection;"
    "uniform   mat4 qr_uModelview;"
    "varying   vec4 qr_fColor;"
    "varying   vec2 qr_fTexCoord;"
    "varying   vec3 qr_fLightPos;"
    "void main(void)"
    "{"
    "    qr_fColor    = qr_vColor;"
    "    qr_fTexCoord = qr_vTexCoord;"
    "    qr_fLightPos = qr_vLightPos;"
    "    gl_Position  = qr_uProjection * qr_uModelview * qr_vPosition;"
    "}";
//------------------------------------------------------------------------------
// NOTE this shader was written on the base of the following article:
// http://www.swiftless.com/tutorials/glsl/8_bump_mapping.html
const char* m_pFSDiffuseBumpMap =
    "precision mediump float;"
    "uniform sampler2D qr_sColorMap;"
    "uniform sampler2D qr_sBumpMap;"
    "varying lowp vec4 qr_fColor;"
    "varying      vec2 qr_fTexCoord;"
    "varying      vec3 qr_fLightPos;"
    "void main(void)"
    "{"
    "    vec3  normal  = normalize(texture2D(qr_sBumpMap, qr_fTexCoord).rgb * 2.0 - 1.0);"
    "    float diffuse = clamp(dot(normal, qr_fLightPos), 0.0, 2.5);"
    "    vec3  color   = diffuse * texture2D(qr_sColorMap, qr_fTexCoord).rgb;"
    "    gl_FragColor  = vec4(color, 1.0);"
    "}";
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader        m_Shader;
    GLuint             m_ShaderProgram;
    float*             m_pSurfaceVB;
    unsigned           m_SurfaceVertexCount;
    float              m_SurfaceWidth;
    float              m_SurfaceHeight;
    GLuint             m_TextureIndex;
    GLuint             m_BumpMapIndex;
    GLuint             m_LightPos;
    GLuint             m_TexSamplerSlot;
    GLuint             m_BumpMapSamplerSlot;
    MINI_Size          m_View;
    MINI_VertexFormat  m_VertexFormat;
}

@property (strong, nonatomic) EAGLContext* pContext;

/**
* Enables OpenGL
*/
- (void) EnableOpenGL;

/**
* Disables OpenGL
*/
- (void) DisableOpenGL;

/**
* Creates the viewport
*@param w - viewport width
*@param h - viewport height
*/
- (void) CreateViewport :(float)w  :(float)h;

/**
* Initializes the scene
*/
- (void) InitScene;

/**
* Deletes the scene
*/
- (void) DeleteScene;

/**
* Updates the scene
*@param elapsedTime - elapsed time since last update, in milliseconds
*/
- (void) UpdateScene :(float)elapsedTime;

/**
* Draws the scene
*/
- (void) DrawScene;

/**
* Called when screen is long pressed
*@param pRecognizer - recognizer that raised the event
*/
- (void)OnLongPress :(UIGestureRecognizer*)pSender;

@end
//----------------------------------------------------------------------------
@implementation GameViewController
//----------------------------------------------------------------------------
- (void) viewDidLoad
{
    [super viewDidLoad];

    m_ShaderProgram      = 0;
    m_pSurfaceVB         = 0;
    m_SurfaceVertexCount = 0;
    m_SurfaceWidth       = 10.0f;
    m_SurfaceHeight      = 12.5f;
    m_TextureIndex       = GL_INVALID_VALUE;
    m_BumpMapIndex       = GL_INVALID_VALUE;
    m_LightPos           = 0;
    m_TexSamplerSlot     = 0;
    m_BumpMapSamplerSlot = 0;

    // create long press gesture recognizer
    UILongPressGestureRecognizer* pGestureRecognizer =
    [[UILongPressGestureRecognizer alloc]initWithTarget:self
                                                 action:@selector(OnLongPress:)];

    pGestureRecognizer.minimumPressDuration = 0;

    // add gesture recognizer to view
    [self.view addGestureRecognizer: pGestureRecognizer];

    [self EnableOpenGL];
    [self InitScene];
}
//----------------------------------------------------------------------------
- (void) dealloc
{
    [self DeleteScene];
    [self DisableOpenGL];

    if ([EAGLContext currentContext] == self.pContext)
        [EAGLContext setCurrentContext:nil];
}
//----------------------------------------------------------------------------
- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view]window] == nil))
    {
        self.view = nil;

        [self DeleteScene];
        [self DisableOpenGL];

        if ([EAGLContext currentContext] == self.pContext)
            [EAGLContext setCurrentContext:nil];

        self.pContext = nil;
    }
}
//----------------------------------------------------------------------------
- (BOOL) prefersStatusBarHidden
{
    return YES;
}
//----------------------------------------------------------------------------
- (void)glkView :(GLKView*)view drawInRect:(CGRect)rect
{
    [self UpdateScene:0.0f];
    [self DrawScene];
}
//----------------------------------------------------------------------------
- (void) EnableOpenGL
{
    self.pContext = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES2];

    if (!self.pContext)
        NSLog(@"Failed to create ES context");

    GLKView* pView            = (GLKView*)self.view;
    pView.context             = self.pContext;
    pView.drawableDepthFormat = GLKViewDrawableDepthFormat24;

    [EAGLContext setCurrentContext:self.pContext];
}
//----------------------------------------------------------------------------
- (void) DisableOpenGL
{
    [EAGLContext setCurrentContext:self.pContext];
}
//----------------------------------------------------------------------------
- (void) CreateViewport :(float)w :(float)h
{
    MINI_Matrix matrix;
    GLint       projectionUniform;

    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 100.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    projectionUniform = glGetUniformLocation(m_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//----------------------------------------------------------------------------
- (void)InitScene
{
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(m_pVSDiffuseBumpMap, m_pFSDiffuseBumpMap);
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram,  "qr_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram,  "qr_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram,  "qr_vTexCoord");
    m_LightPos              = glGetUniformLocation(m_ShaderProgram, "qr_vLightPos");
    m_TexSamplerSlot        = glGetUniformLocation(m_ShaderProgram, "qr_sColorMap");
    m_BumpMapSamplerSlot    = glGetUniformLocation(m_ShaderProgram, "qr_sBumpMap");

    // notify shader about default light position
    glUniform3f(m_LightPos, 0.0f, 0.0f, 2.0f);

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen]bounds];

    // create the viewport
    [self CreateViewport :screenRect.size.width :screenRect.size.height];

    // get the view size
    m_View.m_Width  = screenRect.size.width;
    m_View.m_Height = screenRect.size.height;

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 1;
    m_VertexFormat.m_UseColors   = 1;

    // generate surface
    miniCreateSurface(&m_SurfaceWidth,
                      &m_SurfaceHeight,
                      0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);

    char* pTextureFileName = 0;
    char* pBumpmapFileName = 0;

    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"stone"      :@"bmp" :&pTextureFileName];
    [MiniObjectiveCHelper ResourceToFileName :@"stone_bump" :@"bmp" :&pBumpmapFileName];

    // load wall texture and his bump map
    m_TextureIndex = miniLoadTexture(pTextureFileName);
    m_BumpMapIndex = miniLoadTexture(pBumpmapFileName);

    free(pTextureFileName);
    free(pBumpmapFileName);
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete surface vertices
    if (m_pSurfaceVB)
    {
        free(m_pSurfaceVB);
        m_pSurfaceVB = 0;
    }

    if (m_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_TextureIndex);

    m_TextureIndex = GL_INVALID_VALUE;

    if (m_BumpMapIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_BumpMapIndex);

    m_BumpMapIndex = GL_INVALID_VALUE;

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    int          stride;
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // calculate vertex stride
    stride = m_VertexFormat.m_Stride;

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -15.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // enable OpenGL texturing engine
    glEnable(GL_TEXTURE_2D);

    // connect texture to shader
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, 0);
    glBindTexture(GL_TEXTURE_2D, m_TextureIndex);

    // connect bump map to shader
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(m_BumpMapSamplerSlot, 1);
    glBindTexture(GL_TEXTURE_2D, m_BumpMapIndex);

    // draw the bump mapped image
    miniDrawSurface(m_pSurfaceVB,
                    m_SurfaceVertexCount,
                    &m_VertexFormat,
                    &m_Shader);

    miniEndScene();
}
//----------------------------------------------------------------------------
- (void)OnLongPress :(UIGestureRecognizer*)pSender
{
    const float maxX = 2.0f;
    const float maxY = 2.0f;

    CGPoint touchPos;

    switch (pSender.state)
    {
        case UIGestureRecognizerStateBegan:
        case UIGestureRecognizerStateChanged:
        case UIGestureRecognizerStateEnded:
            touchPos = [pSender locationInView :nil];
            break;

        default:
            break;
    }

    // convert screen coordinates to light world coordinate and notify shader about new light position
    glUniform3f(m_LightPos,
                ((touchPos.x * maxX) / m_View.m_Width) - 1.0f,
                1.0f - ((touchPos.y * maxY) / m_View.m_Height),
                2.0f);
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
