/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : An underground level demo based on ray casting engines like *
 *               Doom or Wolfenstein. Swipe up or down to walk, and left or  *
 *               right to rotate                                             *
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
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniLevel.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

#import "MiniObjectiveCHelper.h"

// map mode, used for debugging
//#define MAP_MODE

//------------------------------------------------------------------------------
const char* g_pLevelMap =
    "***   ***   ******"
    "********* *   *  *"
    "********* ** **  *"
    "*** * ************"
    "*** * ********* **"
    "***   ***  *    **"
    "********* *** ****"
    "********* *** ****"
    "***   *** *** ****";
//------------------------------------------------------------------------------
const char* miniVSTextured2 =
    "precision mediump float;"
    "attribute vec4 mini_vPosition;"
    "attribute vec4 mini_vColor;"
    "attribute vec2 mini_vTexCoord;"
    "uniform   mat4 mini_uProjection;"
    "uniform   mat4 mini_uView;"
    "uniform   mat4 mini_uModelview;"
    "varying   vec4 mini_fColor;"
    "varying   vec2 mini_fTexCoord;"
    "void main(void)"
    "{"
    "    mini_fColor    = mini_vColor;"
    "    mini_fTexCoord = mini_vTexCoord;"
    "    gl_Position  = mini_uProjection * mini_uView * mini_uModelview * mini_vPosition;"
    "}";
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    MINI_LevelItem*   m_pLevel;
    GLuint            m_ShaderProgram;
    float*            m_pSurfaceVB;
    unsigned int      m_SurfaceVertexCount;
    float             m_LevelItemWidth;
    float             m_LevelItemHeight;
    float             m_LevelItemLength;
    float             m_PosVelocity;
    float             m_DirVelocity;
    float             m_ControlRadius;
    float             m_Angle;
    unsigned int      m_MapWidth;
    unsigned int      m_MapHeight;
    unsigned int      m_MapLength;
    MINI_Sphere       m_Player;
    MINI_Vector2      m_TouchOrigin;
    MINI_Vector2      m_TouchPosition;
    GLuint            m_SoilTextureIndex;
    GLuint            m_WallTextureIndex;
    GLuint            m_CeilTextureIndex;
    GLuint            m_TexSamplerSlot;
    GLuint            m_ViewUniform;
    GLuint            m_ModelviewUniform;
    MINI_VertexFormat m_VertexFormat;
    CFTimeInterval    m_PreviousTime;

    #ifdef MAP_MODE
        float             m_SphereRadius;
        float*            m_pSphereVertices;
        unsigned int      m_SphereVertexCount;
        unsigned int      m_SphereIndexCount;
        MINI_Index*       m_pSphereIndexes;
        MINI_VertexFormat m_SphereVertexFormat;
        GLuint            m_SphereTextureIndex;
    #endif
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
    m_pLevel             = 0;
    m_ShaderProgram      = 0;
    m_pSurfaceVB         = 0;
    m_SurfaceVertexCount = 0;
    m_LevelItemWidth     = 0.05f;
    m_LevelItemHeight    = 0.05f;
    m_LevelItemLength    = 0.05f;
    m_PosVelocity        = 1.0f;
    m_DirVelocity        = 20.0f;
    m_ControlRadius      = 40.0f;
    m_Angle              = 0.0f;
    m_MapWidth           = 18;
    m_MapHeight          = 9;
    m_MapLength          = m_MapWidth * m_MapHeight;
    m_SoilTextureIndex   = GL_INVALID_VALUE;
    m_WallTextureIndex   = GL_INVALID_VALUE;
    m_CeilTextureIndex   = GL_INVALID_VALUE;
    m_TexSamplerSlot     = 0;
    m_ViewUniform        = 0;
    m_ModelviewUniform   = 0;
    m_PreviousTime       = CACurrentMediaTime();

    #ifdef MAP_MODE
        m_pSphereVertices    = 0;
        m_pSphereIndexes     = 0;
        m_SphereTextureIndex = GL_INVALID_VALUE;
    #endif

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
    const float zNear  = 0.001f;
    const float zFar   = 1000.0f;
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
    // initialize the player
    m_Player.m_Pos.m_X = 0.0f;
    m_Player.m_Pos.m_Y = 0.0f;
    m_Player.m_Pos.m_Z = 0.0f;
    m_Player.m_Radius  = 0.01f;

    m_Angle = 0.0f;

    // initialize the touch
    m_TouchOrigin.m_X   = 0.0f;
    m_TouchOrigin.m_Y   = 0.0f;
    m_TouchPosition.m_X = 0.0f;
    m_TouchPosition.m_Y = 0.0f;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniVSTextured2, miniGetFSTextured());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram, "mini_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram, "mini_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram, "mini_sColorMap");

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

    // generate the level
    miniGenerateLevel(g_pLevelMap,
                      m_MapWidth,
                      m_MapHeight,
                      &m_LevelItemWidth,
                      &m_LevelItemHeight,
                      &m_pLevel);

    #ifdef MAP_MODE
        m_SphereRadius                     = m_Player.m_Radius;
        m_SphereVertexFormat.m_UseNormals  = 0;
        m_SphereVertexFormat.m_UseTextures = 1;
        m_SphereVertexFormat.m_UseColors   = 1;

        // generate sphere
        miniCreateSphere(&m_SphereRadius,
                         5,
                         5,
                         0xFFFFFFFF,
                         &m_SphereVertexFormat,
                         &m_pSphereVertices,
                         &m_SphereVertexCount,
                         &m_pSphereIndexes,
                         &m_SphereIndexCount);
    #endif

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 1;
    m_VertexFormat.m_UseColors   = 1;

    // calculate the stride
    miniCalculateStride(&m_VertexFormat);

    // generate surface
    miniCreateSurface(&m_LevelItemWidth,
                      &m_LevelItemHeight,
                      0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);

    char* pSoilTextureName = 0;
    char* pWallTextureName = 0;
    char* pCeilTextureName = 0;

    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"Soil_25_256x256"           :@"bmp" :&pSoilTextureName];
    [MiniObjectiveCHelper ResourceToFileName :@"Wall_Tilleable_64_256x256" :@"bmp" :&pWallTextureName];
    [MiniObjectiveCHelper ResourceToFileName :@"Soil_2_25_256x256"         :@"bmp" :&pCeilTextureName];

    // load textures
    m_SoilTextureIndex = miniLoadTexture(pSoilTextureName);
    m_WallTextureIndex = miniLoadTexture(pWallTextureName);
    m_CeilTextureIndex = miniLoadTexture(pCeilTextureName);

    free(pSoilTextureName);
    free(pWallTextureName);
    free(pCeilTextureName);

    #ifdef MAP_MODE
        char* pSphereTextureName = 0;

        // get the sphere texture file paths from resources
        [MiniObjectiveCHelper ResourceToFileName :@"cloud" :@"bmp" :&pSphereTextureName];

        m_SphereTextureIndex = miniLoadTexture(pSphereTextureName);

        free(pSphereTextureName);
    #endif
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete objects used in map mode
    #ifdef MAP_MODE
        // delete sphere vertex buffer
        if (m_pSphereVertices)
        {
            free(m_pSphereVertices);
            m_pSphereVertices = 0;
        }

        // delete sphere index buffer
        if (m_pSphereIndexes)
        {
            free(m_pSphereIndexes);
            m_pSphereIndexes = 0;
        }
    #endif

    // delete surface vertices
    if (m_pSurfaceVB)
    {
        free(m_pSurfaceVB);
        m_pSurfaceVB = 0;
    }

    // delete the level
    if (m_pLevel)
    {
        free(m_pLevel);
        m_pLevel = 0;
    }

    // delete textures
    if (m_SoilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_SoilTextureIndex);

    m_SoilTextureIndex = GL_INVALID_VALUE;

    if (m_WallTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_WallTextureIndex);

    m_WallTextureIndex = GL_INVALID_VALUE;

    if (m_CeilTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_CeilTextureIndex);

    m_CeilTextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    float        angle;
    MINI_Vector3 newPos;

    // if screen isn't touched, do nothing
    if (!m_TouchOrigin.m_X || !m_TouchOrigin.m_Y)
        return;

    newPos = m_Player.m_Pos;

    // calculate the angle formed by the touch gesture x and y distances
    if ((m_TouchPosition.m_X < m_TouchOrigin.m_X || m_TouchPosition.m_Y < m_TouchOrigin.m_Y) &&
        !(m_TouchPosition.m_X < m_TouchOrigin.m_X && m_TouchPosition.m_Y < m_TouchOrigin.m_Y))
        angle = -atanf((m_TouchPosition.m_Y - m_TouchOrigin.m_Y) / (m_TouchPosition.m_X - m_TouchOrigin.m_X));
    else
        angle =  atanf((m_TouchPosition.m_Y - m_TouchOrigin.m_Y) / (m_TouchPosition.m_X - m_TouchOrigin.m_X));

    // calculate the possible min and max values for each axis
    float minX = m_TouchOrigin.m_X - (cosf(angle) * m_ControlRadius);
    float maxX = m_TouchOrigin.m_X + (cosf(angle) * m_ControlRadius);
    float minY = m_TouchOrigin.m_Y - (sinf(angle) * m_ControlRadius);
    float maxY = m_TouchOrigin.m_Y + (sinf(angle) * m_ControlRadius);

    // limit the touch gesture in a radius distance
    if (m_TouchPosition.m_X > maxX)
        m_TouchPosition.m_X = maxX;
    else
    if (m_TouchPosition.m_X < minX)
        m_TouchPosition.m_X = minX;

    if (m_TouchPosition.m_Y > maxY)
        m_TouchPosition.m_Y = maxY;
    else
    if (m_TouchPosition.m_Y < minY)
        m_TouchPosition.m_Y = minY;

    // calculate the final pos and_dir velocity
    float posVelocity = (m_PosVelocity * ((m_TouchPosition.m_Y - m_TouchOrigin.m_Y) / m_TouchOrigin.m_Y));
    float dirVelocity = (m_DirVelocity * ((m_TouchPosition.m_X - m_TouchOrigin.m_X) / m_TouchOrigin.m_X));

    // calculate the next player direction
    m_Angle += dirVelocity * elapsedTime;

    // validate it
    if (m_Angle > M_PI * 2.0f)
        m_Angle -= M_PI * 2.0f;
    else
    if (m_Angle < 0.0f)
        m_Angle += M_PI * 2.0f;

    // calculate the next player position
    newPos.m_X += posVelocity * cosf(m_Angle + (M_PI * 0.5f)) * elapsedTime;
    newPos.m_Z += posVelocity * sinf(m_Angle + (M_PI * 0.5f)) * elapsedTime;

    // validate and apply it
    miniValidateNextPos(m_pLevel,
                        m_LevelItemWidth,
                        m_LevelItemHeight,
                        m_MapLength,
                        &m_Player,
                        &newPos);

    m_Player.m_Pos = newPos;
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_LevelDrawInfo drawInfo;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);

    // configure the draw info
    drawInfo.m_pSurfaceVB         =  m_pSurfaceVB;
    drawInfo.m_SurfaceVertexCount =  m_SurfaceVertexCount;
    drawInfo.m_pVertexFormat      = &m_VertexFormat;
    drawInfo.m_pShader            = &m_Shader;
    drawInfo.m_ShaderProgram      =  m_ShaderProgram;
    drawInfo.m_SoilTextureIndex   =  m_SoilTextureIndex;
    drawInfo.m_WallTextureIndex   =  m_WallTextureIndex;
    drawInfo.m_CeilTextureIndex   =  m_CeilTextureIndex;

    #ifdef MAP_MODE
        drawInfo.m_pSphereVB           =  m_pSphereVertices;
        drawInfo.m_SphereRadius        =  m_SphereRadius;
        drawInfo.m_pSphereIndexes      =  m_pSphereIndexes;
        drawInfo.m_SphereVertexCount   =  m_SphereVertexCount;
        drawInfo.m_SphereIndexCount    =  m_SphereIndexCount;
        drawInfo.m_pSphereShader       = &m_Shader;
        drawInfo.m_pSphereVertexFormat = &m_SphereVertexFormat;
        drawInfo.m_SphereTextureIndex  =  m_SphereTextureIndex;
        drawInfo.m_MapMode             =  1;
    #else
        drawInfo.m_MapMode = 0;
    #endif

    // draw the level
    miniDrawLevel(m_pLevel,
                  m_LevelItemWidth,
                  m_LevelItemHeight,
                  m_LevelItemLength,
                  m_MapLength,
                  &m_Player,
                  m_Angle,
                  &drawInfo);

    miniEndScene();
}
//----------------------------------------------------------------------------
- (void)OnLongPress :(UIGestureRecognizer*)pSender
{
    CGPoint touchPos = [pSender locationInView :nil];

    switch (pSender.state)
    {
        case UIGestureRecognizerStateBegan:
            // initialize the position
            m_TouchOrigin.m_X   = touchPos.x;
            m_TouchOrigin.m_Y   = touchPos.y;
            m_TouchPosition.m_X = touchPos.x;
            m_TouchPosition.m_Y = touchPos.y;
            break;

        case UIGestureRecognizerStateChanged:
            // get the next position
            m_TouchPosition.m_X = touchPos.x;
            m_TouchPosition.m_Y = touchPos.y;
            break;

        case UIGestureRecognizerStateEnded:
            // reset the position
            m_TouchOrigin.m_X   = 0;
            m_TouchOrigin.m_Y   = 0;
            m_TouchPosition.m_X = 0;
            m_TouchPosition.m_Y = 0;
            break;

        default:
            break;
    }
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
