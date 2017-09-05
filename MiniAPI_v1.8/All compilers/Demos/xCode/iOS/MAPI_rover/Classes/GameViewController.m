/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A simple rover model, swipe to left or right to increase or *
 *               decrease the rotation speed                                 *
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

//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    MINI_VertexFormat m_VertexFormat;
    float*            m_pVertices;
    unsigned          m_VertexCount;
    MINI_MdlCmds*     m_pMdlCmds;
    MINI_Index*       m_pIndexes;
    unsigned          m_IndexCount;
    GLuint            m_ShaderProgram;
    float             m_Angle;
    float             m_Time;
    float             m_Interval;
    float             m_RotationSpeed;
    unsigned int      m_FPS;
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

    m_pVertices     = 0;
    m_VertexCount   = 0;
    m_pMdlCmds      = 0;
    m_pIndexes      = 0;
    m_IndexCount    = 0;
    m_ShaderProgram = 0;
    m_Angle         = 0.0f;
    m_Time          = 0.0f;
    m_Interval      = 0.0f;
    m_RotationSpeed = 0.1f;
    m_FPS           = 15;
    m_PreviousTime  = CACurrentMediaTime();

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

    // todo: dispose here any resource that can be recreated
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
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram, "qr_vPosition");
    m_Shader.m_ColorSlot  = glGetAttribLocation(m_ShaderProgram, "qr_vColor");

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

    // create the rover
    miniCreateRover(&m_VertexFormat,
                    &m_pVertices,
                    &m_VertexCount,
                    &m_pMdlCmds,
                    &m_pIndexes,
                    &m_IndexCount);

    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete model commands
    if (m_pMdlCmds)
    {
        free(m_pMdlCmds);
        m_pMdlCmds = 0;
    }

    // delete buffer index table
    if (m_pIndexes)
    {
        free(m_pIndexes);
        m_pIndexes = 0;
    }

    // delete vertices
    if (m_pVertices)
    {
        free(m_pVertices);
        m_pVertices = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
    {
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }
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
    MINI_Vector3 axis;
    MINI_Vector3 factor;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrixX;
    MINI_Matrix  rotateMatrixY;
    MINI_Matrix  scaleMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angleX;
    float        angleY;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =   0.0f;
    t.m_Y =   0.0f;
    t.m_Z = -15.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angleX = 0.0f;

    miniGetRotateMatrix(&angleX, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    angleY = m_Angle;

    miniGetRotateMatrix(&angleY, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 1.0f;
    factor.m_Y = 1.0f;
    factor.m_Z = 1.0f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniMatrixMultiply(&rotateMatrixY,   &rotateMatrixX,   &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the rover model
    miniDrawRover(m_pVertices,
                  m_VertexCount,
                  m_pMdlCmds,
                  m_pIndexes,
                  m_IndexCount,
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
