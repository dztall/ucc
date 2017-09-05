/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : An old school plasma effect that use shader for calculation *
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

//------------------------------------------------------------------------------
// plasma vertex shader program
const char* g_pVSPlasma =
    "precision mediump float;"
    "attribute vec4  qr_vPosition;"
    "uniform   float qr_uTime;"
    "uniform   vec2  qr_uSize;"
    "uniform   mat4  qr_uProjection;"
    "uniform   mat4  qr_uModelview;"
    "varying   float qr_fTime;"
    "varying   vec2  qr_fSize;"
    "void main(void)"
    "{"
    "    qr_fSize    = qr_uSize;"
    "    qr_fTime    = qr_uTime;"
    "    gl_Position = qr_uProjection * qr_uModelview * qr_vPosition;"
    "}";
//------------------------------------------------------------------------------
// plasma fragment shader program. NOTE the above used plasma algorithm is based on Bidouille.org
// oldscool plasma effect post, available here:
// http://www.bidouille.org/prog/plasma
const char* g_pFSPlasma =
    "precision mediump float;"
    "varying float qr_fTime;"
    "varying vec2  qr_fSize;"
    "void main(void)"
    "{"
    "    const float pi      = 3.1415926535897932384626433832795;"
    "    vec2        vK      = vec2(0.34, 0.25);"
    "    vec2        vCoords = vec2((gl_FragCoord.x / qr_fSize.x) * 100.0,"
    "                               (gl_FragCoord.y / qr_fSize.y) * 100.0);"
    "    float v             = 0.0;"
    "    vec2  c             = vCoords * (vK - (vK / 2.0));"
    "    v                  += sin((c.x + qr_fTime));"
    "    v                  += sin((c.y + qr_fTime) / 2.0);"
    "    v                  += sin((c.x + c.y + qr_fTime) / 2.0);"
    "    c                  += vK / 2.0 * vec2(sin(qr_fTime / 3.0), cos(qr_fTime / 2.0));"
    "    v                  += sin(sqrt(c.x * c.x + c.y * c.y + 1.0) + qr_fTime);"
    "    v                   = v / 2.0;"
    "    vec3  col           = vec3(1, sin(pi * v), cos(pi * v));"
    "    gl_FragColor        = vec4(col * 0.5 + 0.5, 1);"
    "}";
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    GLuint            m_ShaderProgram;
    float*            m_pSurfaceVB;
    unsigned int      m_SurfaceVertexCount;
    float             m_SurfaceWidth;
    float             m_SurfaceHeight;
    float             m_MaxTime;
    float             m_Time;
    GLuint            m_TimeSlot;
    GLuint            m_SizeSlot;
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

    m_ShaderProgram      = 0;
    m_pSurfaceVB         = 0;
    m_SurfaceVertexCount = 0;
    m_SurfaceWidth       = 10.0f;
    m_SurfaceHeight      = 12.5f;
    m_MaxTime            = 12.0f * M_PI;
    m_Time               = 0.0f;
    m_TimeSlot           = 0;
    m_SizeSlot           = 0;
    m_PreviousTime       = CACurrentMediaTime();

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
    // get orthogonal matrix
    float left;
    float right;
    float top;
    float bottom;
    float zNear;
    float zFar;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // transform the width and height to keep the correct aspect ratio
    w /= h;
    h /= h;

    // calculate the screen bounds (in the OpenGL view)
    left   = -(w * 0.5f);
    right  =  (w * 0.5f);
    top    =  (h * 0.5f);
    bottom = -(h * 0.5f);
    zNear  = -1.0f;
    zFar   =  1.0f;

    MINI_Matrix matrix;
    miniGetOrtho(&left, &right, &bottom, &top, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//----------------------------------------------------------------------------
- (void)InitScene
{
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(g_pVSPlasma, g_pFSPlasma);
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram,  "qr_vPosition");
    m_TimeSlot            = glGetUniformLocation(m_ShaderProgram, "qr_uTime");
    m_SizeSlot            = glGetUniformLocation(m_ShaderProgram, "qr_uSize");

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
    m_VertexFormat.m_UseColors   = 0;

    // generate surface
    miniCreateSurface(&m_SurfaceWidth,
                      &m_SurfaceHeight,
                      0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);

    // notify shader about screen size
    glUniform2f(m_SizeSlot, screenRect.size.width, screenRect.size.height);
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

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    // calculate next time
    m_Time += elapsedTime * 5.0f;

    // is time out of bounds?
    if (m_Time > m_MaxTime)
        m_Time -= m_MaxTime;

    // notify shader about elapsed time
    glUniform1f(m_TimeSlot, m_Time);
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -1.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the plasma
    miniDrawSurface(m_pSurfaceVB,
                    m_SurfaceVertexCount,
                    &m_VertexFormat,
                    &m_Shader);

    miniEndScene();
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
