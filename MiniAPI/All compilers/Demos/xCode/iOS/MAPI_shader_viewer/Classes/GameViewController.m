/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : View controller in which a MiniAPI project can be created   *
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

//------------------------------------------------------------------------------
// generic vertex shader program
const char* g_pVertexShader =
    "precision mediump float;"
    "attribute vec3  mini_aPosition;"
    "uniform   float mini_uTime;"
    "uniform   vec2  mini_uResolution;"
    "uniform   vec2  mini_uMouse;"
    "varying   float iTime;"
    "varying   vec2  iResolution;"
    "varying   vec2  iMouse;"
    ""
    "void main(void)"
    "{"
    "    iResolution = mini_uResolution;"
    "    iTime       = mini_uTime;"
    "    iMouse      = mini_uMouse;"
    "    gl_Position = vec4(mini_aPosition, 1.0);"
    "}";
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    GLuint            m_ShaderProgram;
    float*            m_pSurfaceVB;
    unsigned int      m_SurfaceVertexCount;
    float             m_MaxTime;
    float             m_Time;
    GLuint            m_TimeSlot;
    GLuint            m_SizeSlot;
    GLuint            m_ResolutionSlot;
    GLuint            m_MouseSlot;
    char              m_ShaderFile[128];
    long              m_ShaderFileSize;
    MINI_Vector2      m_Resolution;
    MINI_Vector2      m_MousePos;
    MINI_VertexFormat m_VertexFormat;
    NSArray*          m_pDirContent;
    CFTimeInterval    m_PreviousTime;
}

@property (strong, nonatomic)          EAGLContext* pContext;
@property (weak, nonatomic)   IBOutlet UIStepper*   pPrevNextBtn;

/**
* Reads a shader file
*@param pFileName - shader file name
*@param fileSize - shader file size
*@param[in, out] pShader - shader
*/
- (void) ReadShader :(const char*)pFileName :(long)fileSize :(char**)pShader;

/**
* Enables OpenGL
*/
- (void) EnableOpenGL;

/**
* Disables OpenGL
*/
- (void) DisableOpenGL;

/**
* Loads a shader from the resources
*@param index - shader index to load
*@return true on success, otherwise false
*/
- (bool) LoadShader :(int)index;

/**
* Creates the viewport
*@param w - viewport width
*@param h - viewport height
*/
- (void) CreateViewport :(float)w :(float)h;

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
    m_pSurfaceVB         = 0;
    m_SurfaceVertexCount = 0;
    m_MaxTime            = 12.0f * M_PI;
    m_Time               = 0.0f;
    m_TimeSlot           = 0;
    m_SizeSlot           = 0;
    m_ResolutionSlot     = 0;
    m_MouseSlot          = 0;
    m_ShaderFileSize     = 0;
    m_PreviousTime       = CACurrentMediaTime();

    memset(m_ShaderFile, 0x0, sizeof(m_ShaderFile));

    // create long press gesture recognizer
    UILongPressGestureRecognizer* pGestureRecognizer =
    [[UILongPressGestureRecognizer alloc]initWithTarget:self
                                                 action:@selector(OnLongPress:)];

    pGestureRecognizer.minimumPressDuration = 0.1;

    // add gesture recognizer to view
    [self.view addGestureRecognizer: pGestureRecognizer];

    // get the shaders resource dir
    NSString* pResourcePath = [[NSBundle mainBundle]resourcePath];
    NSString* pShaderPath   = [pResourcePath stringByAppendingPathComponent:@"Shaders"];

    // get the shaders resource content
    m_pDirContent = [[NSFileManager defaultManager]contentsOfDirectoryAtPath:pShaderPath error:nil];

    // configure the stepper control
    self.pPrevNextBtn.minimumValue = 0;
    self.pPrevNextBtn.maximumValue = m_pDirContent.count - 1;
    self.pPrevNextBtn.value        = 4;
    self.pPrevNextBtn.wraps        = YES;
    self.pPrevNextBtn.autorepeat   = YES;

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
//------------------------------------------------------------------------------
- (IBAction) OnStepperClicked :(UIStepper*)pSender
{
    // Load the next shader
    if (![self LoadShader :self.pPrevNextBtn.value])
        [NSException raise:@"Failed to open shader file" format:@"Failed to open the file"];
}
//------------------------------------------------------------------------------
- (void) ReadShader :(const char*)pFileName :(long)fileSize :(char**)pShader
{
    // open shader file
    FILE* pFile = fopen(pFileName, "rb");

    // succeeded?
    if (!pFile)
        return;

    // reserve memory for the shader program
    *pShader = (char*)malloc(fileSize + 1);

    // succeeded?
    if (*pShader)
    {
        // read the shader program from the file
        fread(*pShader, 1, fileSize, pFile);
        (*pShader)[fileSize] = '\0';
    }

    // close the file
    fclose(pFile);
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
- (bool) LoadShader :(int)index
{
    // is index out of bounds?
    if (index >= m_pDirContent.count)
        return false;

    // delete the previous shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;

    char* pShaderFileName = 0;

    // build the shader file name to load
    NSString* pShaderFileNS = [NSString stringWithFormat :@"%@/%@", @"Shaders", m_pDirContent[index]];

    // get the shader file path from resources
    [MiniObjectiveCHelper ResourceToFileName :[pShaderFileNS componentsSeparatedByString:@"."][0]
                                             :@"shader"
                                             :&pShaderFileName];

    // failed?
    if (!pShaderFileName)
        return false;

    m_ShaderFileSize = miniGetFileSize(pShaderFileName);

    // is file empty?
    if (!m_ShaderFileSize)
        return false;

    char* pShader = 0;

    // read the shader content
    [self ReadShader :pShaderFileName :m_ShaderFileSize :&pShader];

    free(pShaderFileName);

    // failed?
    if (!pShader)
        return false;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(g_pVertexShader, pShader);
    glUseProgram(m_ShaderProgram);

    free(pShader);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram,  "mini_aPosition");
    m_TimeSlot            = glGetUniformLocation(m_ShaderProgram, "mini_uTime");
    m_ResolutionSlot      = glGetUniformLocation(m_ShaderProgram, "mini_uResolution");
    m_MouseSlot           = glGetUniformLocation(m_ShaderProgram, "mini_uMouse");

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen]bounds];

    // create the viewport
    [self CreateViewport :screenRect.size.width :screenRect.size.height];

    return true;
}
//----------------------------------------------------------------------------
- (void) CreateViewport :(float)w :(float)h
{
    // calculate the aspect ratio
    const float aspect = w / h;

    // create the OpenGL viewport
    glViewport(0, 0, w * aspect, h);

    // set the screen resolution
    m_Resolution.m_X = w * [[UIScreen mainScreen]scale];
    m_Resolution.m_Y = h * 2.0f;

    // notify shader about screen size
    glUniform2f(m_ResolutionSlot, m_Resolution.m_X, m_Resolution.m_Y);

    // initialize the mouse (or finger) position in the shader
    glUniform2f(m_MouseSlot, 0, 0);
}
//----------------------------------------------------------------------------
- (void)InitScene
{
    // load the default shader
    if (![self LoadShader :self.pPrevNextBtn.value])
        [NSException raise:@"Failed to open shader file" format:@"Failed to open the file"];

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

    const float surfaceWidth  = 2.0f;
    const float surfaceHeight = 2.0f;

    // generate surface
    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                      0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);
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
    m_Time += elapsedTime * 0.5f;

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

    // draw the surface on which the shader will be executed
    miniDrawSurface(m_pSurfaceVB,
                    m_SurfaceVertexCount,
                    &m_VertexFormat,
                    &m_Shader);

    miniEndScene();
}
//----------------------------------------------------------------------------
- (void)OnLongPress :(UIGestureRecognizer*)pSender
{
    CGPoint touchPos = [pSender locationInView :nil];

    // notify shader about mouse (or finger) position
    glUniform2f(m_MouseSlot,
                m_Resolution.m_X - (touchPos.x * [[UIScreen mainScreen]scale]),
                touchPos.y * 2.0f);
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
