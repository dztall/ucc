/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A textured sphere representing Saturn, swipe to left or     *
 *               right to increase or decrease the rotation speed            *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
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

//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    GLuint            m_ShaderProgram;
    float*            m_pRingVertexBuffer;
    unsigned          m_RingVertexCount;
    float*            m_pVertexBuffer;
    unsigned          m_VertexCount;
    MINI_Index*       m_pIndexes;
    unsigned          m_IndexCount;
    float             m_Radius;
    float             m_Angle;
    float             m_RotationSpeed;
    GLuint            m_TextureIndex;
    GLuint            m_RingTextureIndex;
    GLuint            m_TexSamplerSlot;
    GLuint            m_AlphaSlot;
    MINI_VertexFormat m_VertexFormat;
    CFTimeInterval    m_PreviousTime;
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
 * Called when screen is swipped on the left
 *@param pRecognizer - recognizer that raised the event
 */
- (void) OnScreenSwippedLeft :(UISwipeGestureRecognizer*)pRecognizer;

/**
 * Called when screen is swipped on the right
 *@param pRecognizer - recognizer that raised the event
 */
- (void) OnScreenSwippedRight :(UISwipeGestureRecognizer*)pRecognizer;

@end
//----------------------------------------------------------------------------
@implementation GameViewController
//----------------------------------------------------------------------------
- (void) viewDidLoad
{
    [super viewDidLoad];
    
    m_ShaderProgram     = 0;
    m_pRingVertexBuffer = 0;
    m_RingVertexCount   = 0;
    m_pVertexBuffer     = 0;
    m_VertexCount       = 0;
    m_pIndexes          = 0;
    m_IndexCount        = 0;
    m_Radius            = 1.0f;
    m_Angle             = 0.0f;
    m_RotationSpeed     = 0.1f;
    m_TextureIndex      = GL_INVALID_VALUE;
    m_RingTextureIndex  = GL_INVALID_VALUE;
    m_TexSamplerSlot    = 0;
    m_AlphaSlot         = 0;
    m_PreviousTime      = CACurrentMediaTime();
    
    // add a swipe gesture (to the left) to the view
    UISwipeGestureRecognizer* pSwipeGestureLeft =
    [[UISwipeGestureRecognizer alloc]initWithTarget:self
                                             action:@selector(OnScreenSwippedLeft:)];
    pSwipeGestureLeft.direction = UISwipeGestureRecognizerDirectionLeft;
    [self.view addGestureRecognizer:pSwipeGestureLeft];
    
    // add a swipe gesture (to the right) to the view
    UISwipeGestureRecognizer* pSwipeGestureRight =
    [[UISwipeGestureRecognizer alloc]initWithTarget:self
                                             action:@selector(OnScreenSwippedRight:)];
    pSwipeGestureRight.direction = UISwipeGestureRecognizerDirectionRight;
    [self.view addGestureRecognizer:pSwipeGestureRight];
    
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
    // calculate time interval
    const CFTimeInterval now            =  CACurrentMediaTime();
    const double         elapsedTime    = (now - m_PreviousTime);
    m_PreviousTime =  now;
    
    [self UpdateScene :elapsedTime];
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
    const float zFar   = 20.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;
    
    // create the OpenGL viewport
    glViewport(0, 0, w, h);
    
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);
    
    // connect projection matrix to shader
    projectionUniform = glGetUniformLocation(m_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//----------------------------------------------------------------------------
- (void)InitScene
{
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSTexAlpha(), miniGetFSTexAlpha());
    glUseProgram(m_ShaderProgram);
    
    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram,  "mini_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram,  "mini_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram,  "mini_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram,  "mini_sColorMap");
    m_AlphaSlot             = glGetUniformLocation(m_ShaderProgram, "mini_uAlpha");

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    
    // create the viewport
    [self CreateViewport :screenRect.size.width :screenRect.size.height];
    
    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);
    
    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 1;
    m_VertexFormat.m_UseColors   = 1;
    
    // generate sphere
    miniCreateSphere(&m_Radius,
                     20,
                     48,
                     0xFFFFFFFF,
                     &m_VertexFormat,
                     &m_pVertexBuffer,
                     &m_VertexCount,
                     &m_pIndexes,
                     &m_IndexCount);
    
    // generate ring
    miniCreateRing(0.0f,
                   0.0f,
                   m_Radius + 0.1f,
                   m_Radius + 0.8f,
                   48,
                   0xFFFFFFFF,
                   0xFFFFFFFF,
                   &m_VertexFormat,
                   &m_pRingVertexBuffer,
                   &m_RingVertexCount);
    
    char* pTextureFileName    = 0;
    char* pRingTextureFileName = 0;

    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"texture_saturn" :@"bmp" :&pTextureFileName];
    [MiniObjectiveCHelper ResourceToFileName :@"texture_saturn_ring" :@"bmp" :&pRingTextureFileName];

    // load textures
    m_TextureIndex     = miniLoadTexture(pTextureFileName);
    m_RingTextureIndex = miniLoadTexture(pRingTextureFileName);

    free(pTextureFileName);
    free(pRingTextureFileName);
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete ring vertices
    if (m_pRingVertexBuffer)
    {
        free(m_pRingVertexBuffer);
        m_pRingVertexBuffer = 0;
    }
    
    // delete buffer index table
    if (m_pIndexes)
    {
        free(m_pIndexes);
        m_pIndexes = 0;
    }
    
    // delete vertices
    if (m_pVertexBuffer)
    {
        free(m_pVertexBuffer);
        m_pVertexBuffer = 0;
    }
    
    if (m_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_TextureIndex);
    
    m_TextureIndex = GL_INVALID_VALUE;
    
    if (m_RingTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_RingTextureIndex);
    
    m_RingTextureIndex = GL_INVALID_VALUE;
    
    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    // calculate next rotation angle
    m_Angle += (m_RotationSpeed * elapsedTime * 10.0f);
    
    // is angle out of bounds?
    while (m_Angle >= 6.28f)
        m_Angle -= 6.28f;
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    float        xAngle;
    float        yAngle;
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  xRotateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  zRotateMatrix;
    MINI_Matrix  buildMatrix1;
    MINI_Matrix  buildMatrix2;
    MINI_Matrix  modelViewMatrix;
    
    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    
    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;
    
    miniGetTranslateMatrix(&t, &translateMatrix);
    
    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;
    
    // rotate 45 degrees
    xAngle = -M_PI / 3.0f;
    
    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);
    
    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;
    
    // rotate 30 degrees
    yAngle = M_PI / 12.0f;
    
    miniGetRotateMatrix(&yAngle, &r, &yRotateMatrix);
    
    // set rotation on Z axis
    r.m_X = 0.0f;
    r.m_Y = 0.0f;
    r.m_Z = 1.0f;
    
    miniGetRotateMatrix(&m_Angle, &r, &zRotateMatrix);
    
    // build model view matrix
    miniMatrixMultiply(&zRotateMatrix, &yRotateMatrix,   &buildMatrix1);
    miniMatrixMultiply(&buildMatrix1,  &xRotateMatrix,   &buildMatrix2);
    miniMatrixMultiply(&buildMatrix2,  &translateMatrix, &modelViewMatrix);
    
    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);
    
    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);
    
    // bind the texture
    glBindTexture(GL_TEXTURE_2D, m_TextureIndex);
    
    // configure the culling
    glEnable(GL_CULL_FACE);
    
    // configure OpenGL to draw transparency
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // set alpha transparency level to draw the sphere
    glUniform1f(m_AlphaSlot, 0.65f);
    
    // draw the sphere
    miniDrawSphere(m_pVertexBuffer,
                   m_VertexCount,
                   m_pIndexes,
                   m_IndexCount,
                   &m_VertexFormat,
                   &m_Shader);
    
    // prepare OpenGL to draw the ring
    glDisable(GL_CULL_FACE);
    
    // set alpha transparency level to draw the ring
    glUniform1f(m_AlphaSlot, 0.5f);
    
    // bind the texture
    glBindTexture(GL_TEXTURE_2D, m_RingTextureIndex);
    
    // draw the ring
    miniDrawRing(m_pRingVertexBuffer, m_RingVertexCount, &m_VertexFormat, &m_Shader);
    
    miniEndScene();
}
//----------------------------------------------------------------------------
- (void) OnScreenSwippedLeft :(UISwipeGestureRecognizer*)pRecognizer
{
    // decrease rotation speed
    m_RotationSpeed -= 0.05f;
}
//----------------------------------------------------------------------------
- (void) OnScreenSwippedRight :(UISwipeGestureRecognizer*)pRecognizer
{
    // increase rotation speed
    m_RotationSpeed += 0.05f;
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------

