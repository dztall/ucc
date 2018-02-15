/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : An old school plasma effect that use shader for calculation *
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

//------------------------------------------------------------------------------
// plasma vertex shader program
const char* g_pVSPlasma =
    "precision mediump float;"
    "attribute vec3  mini_vPosition;"
    "uniform   float mini_uTime;"
    "uniform   vec2  mini_uSize;"
    "varying   float mini_fTime;"
    "varying   vec2  mini_fSize;"
    "void main(void)"
    "{"
    "    mini_fSize  = mini_uSize;"
    "    mini_fTime  = mini_uTime;"
    "    gl_Position = vec4(mini_vPosition, 1.0);"
    "}";
//------------------------------------------------------------------------------
// plasma fragment shader program. NOTE the above used plasma algorithm is based on Bidouille.org
// oldscool plasma effect post, available here:
// http://www.bidouille.org/prog/plasma
const char* g_pFSPlasma =
    "precision mediump float;"
    "varying float mini_fTime;"
    "varying vec2  mini_fSize;"
    "void main(void)"
    "{"
    "    const float pi      = 3.1415926535897932384626433832795;"
    "    vec2        vK      = vec2(0.34, 0.25);"
    "    vec2        vCoords = vec2((gl_FragCoord.x / mini_fSize.x) * 100.0,"
    "                               (gl_FragCoord.y / mini_fSize.y) * 100.0);"
    "    float v             = 0.0;"
    "    vec2  c             = vCoords * (vK - (vK / 2.0));"
    "    v                  += sin((c.x + mini_fTime));"
    "    v                  += sin((c.y + mini_fTime) / 2.0);"
    "    v                  += sin((c.x + c.y + mini_fTime) / 2.0);"
    "    c                  += vK / 2.0 * vec2(sin(mini_fTime / 3.0), cos(mini_fTime / 2.0));"
    "    v                  += sin(sqrt(c.x * c.x + c.y * c.y + 1.0) + mini_fTime);"
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
- (void)InitScene
{
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(g_pVSPlasma, g_pFSPlasma);
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram,  "mini_vPosition");
    m_TimeSlot            = glGetUniformLocation(m_ShaderProgram, "mini_uTime");
    m_SizeSlot            = glGetUniformLocation(m_ShaderProgram, "mini_uSize");

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen] bounds];

    // create the OpenGL viewport
    glViewport(0, 0, screenRect.size.width, screenRect.size.height);

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
    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // draw the plasma
    miniDrawSurface(m_pSurfaceVB, m_SurfaceVertexCount, &m_VertexFormat, &m_Shader);

    miniEndScene();
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
