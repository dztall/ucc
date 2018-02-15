/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A transparency effect, swipe left/right to increase or      *
 *               decrease the rotation speed, swipe up/down to increase or   *
 *               decrease the transparency                                   *
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
    float*            m_pSphereVB;
    unsigned          m_SphereVertexCount;
    MINI_Index*       m_pSphereIndexes;
    unsigned          m_SphereIndexCount;
    float*            m_pSurfaceVB;
    unsigned          m_SurfaceVertexCount;
    float             m_SurfaceWidth;
    float             m_SurfaceHeight;
    float             m_SphereRadius;
    float             m_Angle;
    float             m_RotationSpeed;
    float             m_AlphaLevel;
    GLuint            m_GlassTextureIndex;
    GLuint            m_CloudTextureIndex;
    GLuint            m_TexSamplerSlot;
    GLuint            m_AlphaSlot;
    GLuint            m_ModelviewUniform;
    MINI_VertexFormat m_VertexFormat;
    CFTimeInterval    m_PreviousTime;
    CGPoint           m_TouchStartPos;
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
    m_pSphereVB          = 0;
    m_SphereVertexCount  = 0;
    m_pSphereIndexes     = 0;
    m_SphereIndexCount   = 0;
    m_pSurfaceVB         = 0;
    m_SurfaceVertexCount = 0;
    m_SurfaceWidth       = 20.0f;
    m_SurfaceHeight      = 20.0f;
    m_SphereRadius       = 1.0f;
    m_Angle              = 0.0f;
    m_RotationSpeed      = 0.1f;
    m_AlphaLevel         = 0.5f;
    m_GlassTextureIndex  = GL_INVALID_VALUE;
    m_CloudTextureIndex  = GL_INVALID_VALUE;
    m_TexSamplerSlot     = 0;
    m_AlphaSlot          = 0;
    m_ModelviewUniform   = 0;
    m_PreviousTime       = CACurrentMediaTime();

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
    m_ModelviewUniform      = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen] bounds];

    // create the viewport
    [self CreateViewport :screenRect.size.width :screenRect.size.height];

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

    // generate sphere
    miniCreateSphere(&m_SphereRadius,
                     20,
                     48,
                     0xFFFFFFFF,
                     &m_VertexFormat,
                     &m_pSphereVB,
                     &m_SphereVertexCount,
                     &m_pSphereIndexes,
                     &m_SphereIndexCount);

    char* pGlassTextureName = 0;
    char* pCloudTextureName = 0;

    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"frosted_glass" :@"bmp" :&pGlassTextureName];
    [MiniObjectiveCHelper ResourceToFileName :@"cloud"         :@"bmp" :&pCloudTextureName];

    // load textures
    m_GlassTextureIndex = miniLoadTexture(pGlassTextureName);
    m_CloudTextureIndex = miniLoadTexture(pCloudTextureName);

    free(pGlassTextureName);
    free(pCloudTextureName);
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete sphere buffer index table
    if (m_pSphereIndexes)
    {
        free(m_pSphereIndexes);
        m_pSphereIndexes = 0;
    }

    // delete sphere vertices
    if (m_pSphereVB)
    {
        free(m_pSphereVB);
        m_pSphereVB = 0;
    }

    // delete surface vertices
    if (m_pSurfaceVB)
    {
        free(m_pSurfaceVB);
        m_pSurfaceVB = 0;
    }

    if (m_GlassTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_GlassTextureIndex);

    m_GlassTextureIndex = GL_INVALID_VALUE;

    if (m_CloudTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_CloudTextureIndex);

    m_CloudTextureIndex = GL_INVALID_VALUE;

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

    // is rotating angle out of bounds?
    while (m_Angle >= 6.28f)
        m_Angle -= 6.28f;
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    float        xAngle;
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  xRotateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure OpenGL to draw opaque objects (NOTE all opaque object should be drawn before,
    // because depth test should be deactivated later to allow alpha blending to work correctly)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -10.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(m_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // bind cloud texture
    glBindTexture(GL_TEXTURE_2D, m_CloudTextureIndex);

    // set alpha transparency level to draw surface (NOTE surface doesn't use transparency)
    glUniform1f(m_AlphaSlot, 1.0f);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // draw background image
    miniDrawSurface(m_pSurfaceVB,
                    m_SurfaceVertexCount,
                    &m_VertexFormat,
                    &m_Shader);

    // bind glass texture
    glBindTexture(GL_TEXTURE_2D, m_GlassTextureIndex);

    // populate sphere translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 90 degrees
    xAngle = 1.57075;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&m_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&xRotateMatrix,   &yRotateMatrix,   &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(m_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // set alpha transparency level to draw sphere
    glUniform1f(m_AlphaSlot, m_AlphaLevel);

    // configure OpenGL to draw transparency (NOTE all opaque objects should be drawn before,
    // because depth test should be disabled to allow alpha blending to work correctly)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw the transparent sphere
    miniDrawSphere(m_pSphereVB,
                   m_SphereVertexCount,
                   m_pSphereIndexes,
                   m_SphereIndexCount,
                   &m_VertexFormat,
                   &m_Shader);

    miniEndScene();
}
//----------------------------------------------------------------------------
- (void)OnLongPress :(UIGestureRecognizer*)pSender
{
    CGPoint touchPos;

    switch (pSender.state)
    {
        case UIGestureRecognizerStateBegan:
            m_TouchStartPos = [pSender locationInView :nil];
            touchPos        = m_TouchStartPos;
            break;

        case UIGestureRecognizerStateChanged:
            touchPos = [pSender locationInView :nil];
            break;

        case UIGestureRecognizerStateEnded:
            touchPos = [pSender locationInView :nil];
            break;

        default:
            break;
    }

    // increase or decrease rotation speed
    m_RotationSpeed += (touchPos.x - m_TouchStartPos.x) * 0.001f;

    // increase or decrease alpha level
    m_AlphaLevel += (touchPos.y - m_TouchStartPos.y) * -0.001f;

    // limit alpha level to min and max bounds
    if (m_AlphaLevel > 1.0f)
        m_AlphaLevel = 1.0f;
    else
    if (m_AlphaLevel < 0.0f)
        m_AlphaLevel = 0.0f;

    m_TouchStartPos = touchPos;
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
