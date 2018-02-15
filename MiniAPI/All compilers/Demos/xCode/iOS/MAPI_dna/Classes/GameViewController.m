/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A demo showing a dna spiral                                 *
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
    float*            m_pVertices;
    unsigned          m_VertexCount;
    MINI_Index*       m_pIndexes;
    unsigned          m_IndexCount;
    float             m_Pos;
    float             m_Velocity;
    float             m_Angle;
    GLuint            m_TextureIndex;
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

@end
//----------------------------------------------------------------------------
@implementation GameViewController
//----------------------------------------------------------------------------
- (void) viewDidLoad
{
    [super viewDidLoad];

    m_ShaderProgram  = 0;
    m_pVertices      = 0;
    m_VertexCount    = 0;
    m_pIndexes       = 0;
    m_IndexCount     = 0;
    m_Pos            = 0.0f;
    m_Velocity       = 5.0f;
    m_Angle          = 2.0f * M_PI;
    m_TextureIndex   = GL_INVALID_VALUE;
    m_TexSamplerSlot = 0;
    m_AlphaSlot      = 0;
    m_PreviousTime   = CACurrentMediaTime();

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

    // disable OpenGL depth testing
    glDisable(GL_DEPTH_TEST);
    
    // disable culling
    glDisable(GL_CULL_FACE);
    
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // configure the vertex format for the spiral shape
    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 1;
    m_VertexFormat.m_UseColors   = 1;
    miniCalculateStride(&m_VertexFormat);
    
    // create the spiral shape
    miniCreateSpiral(0.0f,
                     0.0f,
                     1.0f,
                     2.2f,
                     0.0f,
                     0.0f,
                     m_Velocity / 36.0f,
                     25,
                     36,
                     0xFFFFFFFF,
                     0x404040FF,
                     &m_VertexFormat,
                     &m_pVertices,
                     &m_VertexCount,
                     &m_pIndexes,
                     &m_IndexCount);
    
    char* pTextureFileName = 0;

    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"texture_dna" :@"bmp" :&pTextureFileName];
    
    // load dna texture
    m_TextureIndex = miniLoadTexture(pTextureFileName);

    free(pTextureFileName);
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete indices
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
        glDeleteProgram(m_ShaderProgram);
    
    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    // rotate the spiral
    m_Angle -= 0.5f * elapsedTime;
    
    // is angle out of boumds?
    if (m_Angle < 0.0f)
        m_Angle += 2.0f * M_PI;
    
    // calculate the camera movement to follow the spiral
    m_Pos = (m_Angle * m_Velocity) / (2.0f * M_PI);
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrix;
    MINI_Matrix  modelViewMatrix;
    
    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    
    // populate translation vector
    t.m_X = -0.5f;
    t.m_Y =  0.0f;
    t.m_Z = -(-7.5f + m_Pos);
    
    // get translation matrix
    miniGetTranslateMatrix(&t, &translateMatrix);
    
    // populate rotation vector
    r.m_X = 0.0f;
    r.m_Y = 0.0f;
    r.m_Z = 1.0f;
    
    // get rotation matrix
    miniGetRotateMatrix(&m_Angle, &r, &rotateMatrix);
    
    // build the final view matrix
    miniMatrixMultiply(&rotateMatrix, &translateMatrix, &modelViewMatrix);
    
    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);
    
    // set alpha transparency level to draw sphere
    glUniform1f(m_AlphaSlot, 0.75f);
    
    // draw the spiral
    miniDrawSpiral(m_pVertices,
                   m_VertexCount,
                   m_pIndexes,
                   m_IndexCount,
                   &m_VertexFormat,
                   &m_Shader);
    
    miniEndScene();
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
