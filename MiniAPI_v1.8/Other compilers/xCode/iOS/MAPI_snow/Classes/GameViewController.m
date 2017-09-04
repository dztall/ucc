/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : Snow particle system demo                                   *
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
#include "MiniParticles.h"
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
    float             m_Radius;
    float             m_Angle;
    float             m_RotationSpeed;
    float             m_ElapsedTime;
    float             m_Interval;
    unsigned int      m_ParticleCount;
    unsigned int      m_FPS;
    int               m_Initialized;
    MINI_Particles    m_Particles;
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

    m_ShaderProgram = 0;
    m_pVertexBuffer = 0;
    m_VertexCount   = 0;
    m_pIndexes      = 0;
    m_IndexCount    = 0;
    m_Radius        = 0.02f;
    m_Angle         = 0.0f;
    m_RotationSpeed = 0.1f;
    m_ElapsedTime   = 0.0f;
    m_Interval      = 0.0f;
    m_ParticleCount = 1000;
    m_FPS           = 10;
    m_Initialized   = 0;
    m_PreviousTime  = CACurrentMediaTime();

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
    srand((unsigned)time(0));

    m_Particles.m_Count = 0;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
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
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&m_Radius,
                     5,
                     5,
                     0xFFFFFFFF,
                     &m_VertexFormat,
                     &m_pVertexBuffer,
                     &m_VertexCount,
                     &m_pIndexes,
                     &m_IndexCount);

    m_Interval = 1000.0f / m_FPS;
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    miniClearParticles(&m_Particles);

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

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    unsigned       i;
    unsigned       frameCount;
    MINI_Vector3   startPos;
    MINI_Vector3   startDir;
    MINI_Vector3   startVelocity;
    MINI_Particle* pNewParticle;

    frameCount = 0;

    // calculate next time
    m_ElapsedTime += (elapsedTime * 1000.0f);

    // count frames to skip
    while (m_ElapsedTime > m_Interval)
    {
        m_ElapsedTime -= m_Interval;
        ++frameCount;
    }

    startPos.m_X      =  0.0f; // between -2.2 to 2.2
    startPos.m_Y      =  2.0f;
    startPos.m_Z      = -3.0f; // between -1.0 to -5.0
    startDir.m_X      =  1.0f;
    startDir.m_Y      = -1.0f;
    startDir.m_Z      =  0.0f;
    startVelocity.m_X =  0.0f;
    startVelocity.m_Y =  0.05f;
    startVelocity.m_Z =  0.0f;

    // iterate through particles to update
    for (i = 0; i < m_ParticleCount; ++i)
    {
        // emit a new particle
        if ((pNewParticle = miniEmitParticle(&m_Particles,
                                             &startPos,
                                             &startDir,
                                             &startVelocity,
                                             m_ParticleCount)))
        {
            // initialize default values
            pNewParticle->m_Position.m_X =  ((rand() % 44) - 22.0f) * 0.1f; // between -2.2 to  2.2
            pNewParticle->m_Position.m_Z = -((rand() % 40) + 10.0f) * 0.1f; // between -1.0 to -5.0
            pNewParticle->m_Velocity.m_X =  ((rand() % 4)  - 2.0f)  * 0.01f;
            pNewParticle->m_Velocity.m_Y =  ((rand() % 4)  + 2.0f)  * 0.01f;

            // select a random start height the first time particles are emitted
            if (!m_Initialized)
                pNewParticle->m_Position.m_Y = 2.0f + ((rand() % 200) * 0.01f);
        }

        // no particles to show? (e.g all were removed in this loop)
        if (!m_Particles.m_Count)
            continue;

        // move particle
        if (i >= m_Particles.m_Count)
            miniMoveParticle(&m_Particles.m_pParticles[m_Particles.m_Count - 1],
                             frameCount);
        else
            miniMoveParticle(&m_Particles.m_pParticles[i], frameCount);

        // is particle out of screen?
        if (m_Particles.m_pParticles[i].m_Position.m_Y <= -2.0f ||
            m_Particles.m_pParticles[i].m_Position.m_X <= -4.0f ||
            m_Particles.m_pParticles[i].m_Position.m_X >=  4.0f)
        {
            // delete it from system
            miniDeleteParticle(&m_Particles, i);
            continue;
        }
    }

    m_Initialized = 1;
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    unsigned     i;
    MINI_Vector3 t;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.1f, 0.35f, 0.66f, 1.0f);

    // iterate through particles to draw
    for (i = 0; i < m_Particles.m_Count; ++i)
    {
        // set translation
        t.m_X = m_Particles.m_pParticles[i].m_Position.m_X;
        t.m_Y = m_Particles.m_pParticles[i].m_Position.m_Y;
        t.m_Z = m_Particles.m_pParticles[i].m_Position.m_Z;

        miniGetTranslateMatrix(&t, &translateMatrix);

        // build model view matrix
        miniGetIdentity(&modelViewMatrix);
        miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);

        // connect model view matrix to shader
        GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the particle
        miniDrawSphere(m_pVertexBuffer,
                       m_VertexCount,
                       m_pIndexes,
                       m_IndexCount,
                       &m_VertexFormat,
                       &m_Shader);
    }

    miniEndScene();
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
