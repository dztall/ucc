/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A Quake II (MD2) model showing a robot. Tap on the left or  *
 *               right to change the animation                               *
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
#include "MiniModels.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

#import "MiniObjectiveCHelper.h"

//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} MINI_MD2Animation;
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    MINI_VertexFormat m_ModelFormat;
    MINI_MD2Model*    m_pModel;
    unsigned int      m_MeshIndex;
    GLuint            m_ShaderProgram;
    GLuint            m_TextureIndex;
    GLuint            m_PositionSlot;
    GLuint            m_ColorSlot;
    GLuint            m_TexCoordSlot;
    GLuint            m_TexSamplerSlot;
    float             m_Time;
    float             m_Interval;
    float             m_ScreenWidth;
    unsigned int      m_FPS;
    unsigned int      m_AnimIndex; // can only be 0 (robot walks) or 1 (robot dies)
    MINI_MD2Animation m_Animation[2];
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
* Called when screen is tapped
*@param pRecognizer - recognizer that raised the event
*/
- (void) OnScreenTapped :(UITapGestureRecognizer*)pRecognizer;

@end
//----------------------------------------------------------------------------
@implementation GameViewController
//----------------------------------------------------------------------------
- (void) viewDidLoad
{
    [super viewDidLoad];

    m_pModel         = 0;
    m_MeshIndex      = 0;
    m_ShaderProgram  = 0;
    m_TextureIndex   = GL_INVALID_VALUE;
    m_PositionSlot   = 0;
    m_ColorSlot      = 0;
    m_TexCoordSlot   = 0;
    m_TexSamplerSlot = 0;
    m_Time           = 0.0f;
    m_Interval       = 0.0f;
    m_ScreenWidth    = 0.0f;
    m_FPS            = 15;
    m_AnimIndex      = 0; // can only be 0 (robot walks) or 1 (robot dies)
    m_PreviousTime   = CACurrentMediaTime();

    // add a tap gesture to the view
    UITapGestureRecognizer* pTapGesture =
    [[UITapGestureRecognizer alloc]initWithTarget:self
                                           action:@selector(OnScreenTapped:)];
    [self.view addGestureRecognizer:pTapGesture];

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
    const float zFar   = 150.0f;
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
    m_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(m_ShaderProgram);

    // configure the shader slots
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram, "mini_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram, "mini_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram, "mini_sColorMap");

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen] bounds];

    // create the viewport
    [self CreateViewport :screenRect.size.width :screenRect.size.height];

    // get the screen width
    m_ScreenWidth = screenRect.size.width;

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    m_ModelFormat.m_UseNormals  = 0;
    m_ModelFormat.m_UseTextures = 1;
    m_ModelFormat.m_UseColors   = 1;

    char* pModelFileName = 0;

    // get the model file path from resources
    [MiniObjectiveCHelper ResourceToFileName :@"chip" :@"md2" :&pModelFileName];

    // load MD2 file and create mesh to draw
    miniLoadMD2Model((unsigned char*)pModelFileName, &m_ModelFormat, 0xFFFFFFFF, &m_pModel);

    free(pModelFileName);

    char* pTextureFileName = 0;

    // get the model texture file path from resources
    [MiniObjectiveCHelper ResourceToFileName :@"chipskin" :@"bmp" :&pTextureFileName];

    // load MD2 texture
    m_TextureIndex = miniLoadTexture(pTextureFileName);

    free(pTextureFileName);

    // create MD2 animation list
    m_Animation[0].m_Range[0] = 0;  m_Animation[0].m_Range[1] = 20; // robot walks
    m_Animation[1].m_Range[0] = 21; m_Animation[1].m_Range[1] = 29; // robot dies

    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    miniReleaseMD2Model(m_pModel);
    m_pModel = 0;

    if (m_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_TextureIndex);

    m_TextureIndex = GL_INVALID_VALUE;

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
    unsigned int deltaRange = m_Animation[m_AnimIndex].m_Range[1] -
                              m_Animation[m_AnimIndex].m_Range[0];

    // calculate next time
    m_Time += (elapsedTime * 1000.0f);

    // count frames
    while (m_Time > m_Interval)
    {
        m_Time -= m_Interval;
        ++frameCount;
    }

    // calculate next mesh index to show. Index should always be between animation range
    m_MeshIndex = ((m_MeshIndex + frameCount) % deltaRange);
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_Vector3 t;
    MINI_Vector3 axis;
    MINI_Vector3 factor;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrix;
    MINI_Matrix  scaleMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angle;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -100.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = 0.0f;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrix);

    // set scale factor
    factor.m_X = 0.02f;
    factor.m_Y = 0.02f;
    factor.m_Z = 0.02f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniGetIdentity(&modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &rotateMatrix,    &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // draw the model
    miniDrawMD2(m_pModel,
                &m_Shader,
                (int)(m_Animation[m_AnimIndex].m_Range[0] + m_MeshIndex));

    miniEndScene();
}
//----------------------------------------------------------------------------
- (void) OnScreenTapped :(UITapGestureRecognizer*)pRecognizer
{
    // get the view tap location
    const CGPoint location = [pRecognizer locationInView:self.view];

    if (location.x > m_ScreenWidth * 0.5f)
    {
        ++m_AnimIndex;

        if (m_AnimIndex > 1)
            m_AnimIndex = 0;
    }
    else
    {
        if (m_AnimIndex == 0)
            m_AnimIndex = 2;

        --m_AnimIndex;
    }
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
