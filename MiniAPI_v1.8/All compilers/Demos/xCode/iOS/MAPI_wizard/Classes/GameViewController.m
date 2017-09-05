/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A Quake (MDL) model showing a wizard. Tap on the left or    *
 *               right to change the animation                               *
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

//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} MINI_MDLAnimation;
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    MINI_VertexFormat m_ModelFormat;
    MINI_MDLModel*    m_pModel;
    unsigned int      m_MeshIndex;
    GLuint            m_ShaderProgram;
    GLuint            m_TextureIndex;
    GLuint            m_TexSamplerSlot;
    float             m_ScreenWidth;
    float             m_Time;
    float             m_Interval;
    unsigned int      m_FPS;
    unsigned int      m_AnimIndex; // 0 = hover, 1 = fly, 2 = attack, 3 = pain, 4 = death
    MINI_MDLAnimation m_Animation[5];
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

    m_ShaderProgram  = 0;
    m_pModel         = 0;
    m_MeshIndex      = 0;
    m_ShaderProgram  = 0;
    m_TextureIndex   = GL_INVALID_VALUE;
    m_TexSamplerSlot = 0;
    m_ScreenWidth    = 0.0f;
    m_Time           = 0.0f;
    m_Interval       = 0.0f;
    m_FPS            = 15;
    m_AnimIndex      = 0; // 0 = hover, 1 = fly, 2 = attack, 3 = pain, 4 = death
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
    MINI_Texture texture;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(m_ShaderProgram);

    // configure the shader slots
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram, "qr_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram, "qr_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram, "qr_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram, "qr_sColorMap");

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
    [MiniObjectiveCHelper ResourceToFileName :@"wizard" :@"mdl" :&pModelFileName];

    // load MDL file and create mesh to draw
    miniLoadMDLModel((unsigned char*)pModelFileName,
                     &m_ModelFormat,
                     0xFFFFFFFF,
                     &m_pModel,
                     &texture);

    free(pModelFileName);

    // create new OpenGL texture
    glGenTextures(1, &m_TextureIndex);
    glBindTexture(GL_TEXTURE_2D, m_TextureIndex);

    // set texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set texture wrapping mode
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // generate texture from bitmap data
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 texture.m_Width,
                 texture.m_Height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 texture.m_pPixels);

    // delete buffers
    free(texture.m_pPixels);

    // create MD2 animation list
    m_Animation[0].m_Range[0] = 0;  m_Animation[0].m_Range[1] = 14; // wizard hovers
    m_Animation[1].m_Range[0] = 15; m_Animation[1].m_Range[1] = 28; // wizard flies
    m_Animation[2].m_Range[0] = 29; m_Animation[2].m_Range[1] = 42; // wizard attacks
    m_Animation[3].m_Range[0] = 43; m_Animation[3].m_Range[1] = 47; // wizard feels pain
    m_Animation[4].m_Range[0] = 48; m_Animation[4].m_Range[1] = 54; // wizard dies

    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    miniReleaseMDLModel(m_pModel);
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
    MINI_Matrix  rotateMatrixX;
    MINI_Matrix  rotateMatrixY;
    MINI_Matrix  scaleMatrix;
    MINI_Matrix  combinedRotMatrix;
    MINI_Matrix  combinedRotTransMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angle;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -150.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.5;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angle = -M_PI * 0.25;

    miniGetRotateMatrix(&angle, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 0.02f;
    factor.m_Y = 0.02f;
    factor.m_Z = 0.02f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniMatrixMultiply(&rotateMatrixX,          &rotateMatrixY,   &combinedRotMatrix);
    miniMatrixMultiply(&combinedRotMatrix,      &translateMatrix, &combinedRotTransMatrix);
    miniMatrixMultiply(&combinedRotTransMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // draw the model
    miniDrawMDL(m_pModel,
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

        if (m_AnimIndex > 4)
            m_AnimIndex = 0;
    }
    else
    {
        if (m_AnimIndex == 0)
            m_AnimIndex = 5;

        --m_AnimIndex;
    }
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
