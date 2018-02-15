/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : An animation using a quaternion                             *
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

//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    GLuint            m_ShaderProgram;
    float*            m_pVertexBuffer;
    unsigned int      m_VertexCount;
    MINI_Index*       m_pIndexes;
    unsigned int      m_IndexCount;
    float*            m_pCylinderVB;
    unsigned int      m_CylinderVBCount;
    float             m_CircleRadius;
    float             m_CylinderRadius;
    float             m_CylinderHeight;
    float             m_Angle;
    unsigned          m_CylFaceCount;
    MINI_Size         m_View;
    MINI_VertexFormat m_VertexFormat;
    MINI_Matrix       m_ProjectionMatrix;
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

@end
//----------------------------------------------------------------------------
@implementation GameViewController
//----------------------------------------------------------------------------
- (void) viewDidLoad
{
    [super viewDidLoad];

    m_ShaderProgram   = 0;
    m_pVertexBuffer   = 0;
    m_VertexCount     = 0;
    m_pIndexes        = 0;
    m_IndexCount      = 0;
    m_pCylinderVB     = 0;
    m_CylinderVBCount = 0;
    m_CircleRadius    = 0.1f;
    m_CylinderRadius  = 0.1f;
    m_CylinderHeight  = 2.0f;
    m_Angle           = 0.0f;
    m_CylFaceCount    = 12;
    m_PreviousTime    = CACurrentMediaTime();

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
    m_View.m_Width  = 0.0f;
    m_View.m_Height = 0.0f;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot  = glGetAttribLocation(m_ShaderProgram, "mini_vColor");

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen] bounds];

    // create the viewport
    [self CreateViewport :screenRect.size.width :screenRect.size.height];

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // disable culling
    glDisable(GL_CULL_FACE);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&m_CircleRadius,
                     5,
                     12,
                     0x0000FFFF,
                     &m_VertexFormat,
                     &m_pVertexBuffer,
                     &m_VertexCount,
                     &m_pIndexes,
                     &m_IndexCount);

    // generate cylinder
    miniCreateCylinder(&m_CylinderRadius,
                       &m_CylinderHeight,
                       m_CylFaceCount,
                       0xFF0000FF,
                       &m_VertexFormat,
                       &m_pCylinderVB,
                       &m_CylinderVBCount);
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
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

    // delete cylinder vertices
    if (m_pCylinderVB)
    {
        free(m_pCylinderVB);
        m_pCylinderVB = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    // calculate next angle value, limit to 2 * PI
    m_Angle = fmodf(m_Angle + (3.0f * elapsedTime), M_PI * 2.0f);
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    float           angle;
    MINI_Vector3    t;
    MINI_Vector3    axis;
    MINI_Quaternion quatX;
    MINI_Quaternion quatZ;
    MINI_Matrix     translateMatrix;
    MINI_Matrix     rotTransMatrix;
    MINI_Matrix     rotMatrixX;
    MINI_Matrix     rotMatrixZ;
    MINI_Matrix     combMatrix1;
    MINI_Matrix     combMatrix2;
    MINI_Matrix     modelMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    t.m_X =  0.95f;
    t.m_Y =  0.0f;
    t.m_Z = -5.0f;

    // set the ball translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    t.m_X = 0.2f;
    t.m_Y = 0.0f;
    t.m_Z = 0.0f;

    // set the ball animation translation
    miniGetTranslateMatrix(&t, &rotTransMatrix);

    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    angle    = m_Angle;

    // calculate the x axis rotation
    miniQuatFromAxis(&angle, &axis, &quatX);
    miniGetMatrix(&quatX, &rotMatrixX);

    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;
    angle    = M_PI / 2.0f;

    // calculate the z axis rotation
    miniQuatFromAxis(&angle, &axis, &quatZ);
    miniGetMatrix(&quatZ, &rotMatrixZ);

    // build the ball final model matrix
    miniMatrixMultiply(&rotTransMatrix, &rotMatrixZ,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1,    &rotMatrixX,      &combMatrix2);
    miniMatrixMultiply(&combMatrix2,    &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the sphere
    miniDrawSphere(m_pVertexBuffer,
                   m_VertexCount,
                   m_pIndexes,
                   m_IndexCount,
                   &m_VertexFormat,
                   &m_Shader);

    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -5.0f;

    // set the cylinder animation translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    // build the cylinder final model matrix
    miniMatrixMultiply(&rotMatrixZ,  &rotMatrixX,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1, &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the cylinder
    miniDrawCylinder(m_pCylinderVB, m_CylFaceCount, &m_VertexFormat, &m_Shader);

    miniEndScene();
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
