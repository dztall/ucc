/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A landscape generator based on a grayscale image, swipe to  *
 *               left or right to increase or decrease the rotation speed    *
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
#include "MiniModels.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

#import "MiniObjectiveCHelper.h"

//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    GLuint            m_ShaderProgram;
    float             m_MapHeight;
    float             m_MapScale;
    float             m_Angle;
    float             m_RotationSpeed;
    float             m_Time;
    float             m_Interval;
    unsigned int      m_FPS;
    GLuint            m_TextureIndex;
    GLuint            m_TexSamplerSlot;
    MINI_Mesh*        m_pLandscapeMesh;
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

    m_ShaderProgram  = 0;
    m_MapHeight      = 3.0f;
    m_MapScale       = 0.2f;
    m_Angle          = 0.0f;
    m_RotationSpeed  = 0.02f;
    m_Time           = 0.0f;
    m_Interval       = 0.0f;
    m_FPS            = 15;
    m_TextureIndex   = GL_INVALID_VALUE;
    m_TexSamplerSlot = 0;
    m_pLandscapeMesh = 0;
    m_PreviousTime   = CACurrentMediaTime();

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
    const float zNear  = 0.1f;
    const float zFar   = 20.0f;
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
    int            landscapeWidth;
    int            landscapeHeight;
    unsigned char* pData = 0;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram, "qr_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram, "qr_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram, "qr_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram, "qr_sColorMap");

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
    glDisable(GL_CULL_FACE);

    char* pMapFileName = 0;

    // get the landscape map file path from resources
    [MiniObjectiveCHelper ResourceToFileName :@"the_face" :@"bmp" :&pMapFileName];

    // load landscape data from grayscale image model
    miniLoadLandscape((unsigned char*)pMapFileName, &pData, &landscapeWidth, &landscapeHeight);

    free(pMapFileName);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 1;
    m_VertexFormat.m_UseColors   = 1;

    // generate landscape
    miniCreateLandscape(pData,
                        landscapeWidth,
                        landscapeHeight,
                        m_MapHeight,
                        m_MapScale,
                        &m_VertexFormat,
                        0xFFFFFFFF,
                        &m_pLandscapeMesh);

    // landscape image data will no longer be used
    if (pData)
        free(pData);

    char* pTextureFileName = 0;

    // get the landscape texture file path from resources
    [MiniObjectiveCHelper ResourceToFileName :@"grass" :@"bmp" :&pTextureFileName];

    // load landscape texture
    m_TextureIndex = miniLoadTexture(pTextureFileName);

    free(pTextureFileName);

    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete the landscape
    miniReleaseLandscape(m_pLandscapeMesh);
    m_pLandscapeMesh = 0;

    if (m_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_TextureIndex);

    m_TextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    unsigned int frameCount = 0;

    // calculate next time
    m_Time += (elapsedTime * 1000.0f);

    // count frames to skip
    while (m_Time > m_Interval)
    {
        m_Time -= m_Interval;
        ++frameCount;
    }

    // calculate next rotation angle
    m_Angle += (m_RotationSpeed * frameCount);

    // is rotating angle out of bounds?
    while (m_Angle >= 6.28f)
        m_Angle -= 6.28f;
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.1f, 0.65f, 0.9f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y = -2.0f;
    t.m_Z = -5.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    miniGetRotateMatrix(&m_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&yRotateMatrix, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // draw the landscape
    miniDrawLandscape(m_pLandscapeMesh,
                      &m_VertexFormat,
                      &m_Shader);

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
