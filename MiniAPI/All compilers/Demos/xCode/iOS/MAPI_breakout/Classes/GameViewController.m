/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A simple breakout game                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#import "GameViewController.h"

// openGL
#import <OpenGLES/ES2/glext.h>

// std
#include <string.h>

// mini api
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"
#include "MiniPlayer.h"

#import "MiniObjectiveCHelper.h"

//------------------------------------------------------------------------------
#define M_BALL_VELOCITY_X 0.005f
#define M_BALL_VELOCITY_Y 0.005f
//------------------------------------------------------------------------------
const int g_Level1[15] =
{
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int g_Level2[15] =
{
    1, 0, 0, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int g_Level3[15] =
{
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int g_Level4[15] =
{
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
};
//------------------------------------------------------------------------------
const int g_Level5[15] =
{
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
};
//------------------------------------------------------------------------------
typedef struct
{
    float m_Left;
    float m_Right;
    float m_Top;
    float m_Bottom;
    float m_BarY;
    float m_Width;
} MINI_Screen;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Rect    m_Geometry;
    MINI_Vector2 m_L;
    MINI_Vector2 m_R;
    int          m_Exploding;
    MINI_Vector2 m_ExpLOffset;
    MINI_Vector2 m_ExpROffset;
    ALuint       m_bufferID;
    ALuint       m_SoundID;
} MINI_Bar;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Circle  m_Geometry;
    MINI_Vector2 m_Offset;
    MINI_Vector2 m_Inc;
    MINI_Vector2 m_Max;
    ALuint       m_bufferID;
    ALuint       m_SoundID;
} MINI_Ball;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Rect m_Geometry;
    int       m_Visible;
} MINI_Block;
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    MINI_Screen       m_Screen;
    MINI_Ball         m_Ball;
    MINI_Bar          m_Bar;
    MINI_Block        m_Blocks[15];
    float*            m_pBarVertices;
    unsigned          m_BarVerticesCount;
    float*            m_pBlockVertices;
    unsigned          m_BlockVerticesCount;
    float*            m_pBallVertices;
    unsigned          m_BallVerticesCount;
    int               m_BlockColumns;
    int               m_BlockLines;
    int               m_Level;
    ALCdevice*        m_pOpenALDevice;
    ALCcontext*       m_pOpenALContext;
    MINI_VertexFormat m_VertexFormat;
    GLuint            m_ShaderProgram;
    CGRect            m_ScreenRect;
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
* Get the screen info
*@param width - screen width
*@param height - screen height
*@param[out] pScreen - screen info
*/
- (void) GetScreen :(float)width :(float)height :(MINI_Screen*)pScreen;

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

    m_pBarVertices       = NULL;
    m_BarVerticesCount   = 0;
    m_pBlockVertices     = NULL;
    m_BlockVerticesCount = 0;
    m_pBallVertices      = NULL;
    m_BallVerticesCount  = 0;
    m_BlockColumns       = 5;
    m_BlockLines         = 3;
    m_Level              = 0;
    m_pOpenALDevice      = NULL;
    m_pOpenALContext     = NULL;

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
//------------------------------------------------------------------------------
- (void) GetScreen :(float)width :(float)height :(MINI_Screen*)pScreen
{
    // transform the width and height to keep the correct aspect ratio
    width  /= height;
    height /= height;

    // get the screen width
    pScreen->m_Width = width;

    // calculate the screen bounds (in the OpenGL view)
    pScreen->m_Left   = -(width  * 0.5f);
    pScreen->m_Right  =  (width  * 0.5f);
    pScreen->m_Top    =  (height * 0.5f);
    pScreen->m_Bottom = -(height * 0.5f);

    // calculate bar start y position
    pScreen->m_BarY = pScreen->m_Bottom + 0.05f;
}
//------------------------------------------------------------------------------
- (void) CreateViewport :(float)w :(float)h
{
    const float zNear = -1.0f;
    const float zFar  =  1.0f;
    MINI_Matrix ortho;

    // configure screen
    [self GetScreen :w :h :&m_Screen];

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // get orthogonal projection matrix
    miniGetOrtho(&m_Screen.m_Left,
                 &m_Screen.m_Right,
                 &m_Screen.m_Bottom,
                 &m_Screen.m_Top,
                 &zNear,
                 &zFar,
                 &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//----------------------------------------------------------------------------
- (void)InitScene
{
    int            i;
    int            j;
    int            index;
    float          blockWidth;
    float          blockTop;
    float          surfaceWidth;
    float          surfaceHeight;
    unsigned int   ballSoundFileLen;
    unsigned int   barSoundFileLen;
    unsigned char* pBallSndBuffer;
    unsigned char* pBarSndBuffer;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    // get vertex and color slots
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram, "mini_vPosition");
    m_Shader.m_ColorSlot  = glGetAttribLocation(m_ShaderProgram, "mini_vColor");

    // get the screen rect
    m_ScreenRect = [[UIScreen mainScreen]bounds];

    // create the viewport
    [self CreateViewport :m_ScreenRect.size.width :m_ScreenRect.size.height];

    // initialize ball data
    m_Ball.m_Geometry.m_Pos.m_X = 0.0f;
    m_Ball.m_Geometry.m_Pos.m_Y = 0.0f;
    m_Ball.m_Geometry.m_Radius  = 0.015f;
    m_Ball.m_Offset.m_X         = M_BALL_VELOCITY_X;
    m_Ball.m_Offset.m_Y         = M_BALL_VELOCITY_Y;
    m_Ball.m_Inc.m_X            = 0.001f;
    m_Ball.m_Inc.m_Y            = 0.0015f;
    m_Ball.m_Max.m_X            = 0.3f;
    m_Ball.m_Max.m_Y            = 0.3f;
    m_Ball.m_SoundID            = M_OPENAL_ERROR_ID;

    // initialize bar data
    m_Bar.m_Geometry.m_Pos.m_X       = 0.0f;
    m_Bar.m_Geometry.m_Pos.m_Y       = m_Screen.m_Bottom + m_Ball.m_Max.m_Y;
    m_Bar.m_Geometry.m_Size.m_Width  = 0.06f;
    m_Bar.m_Geometry.m_Size.m_Height = 0.0125f;
    m_Bar.m_R.m_X                    = 0.0f;
    m_Bar.m_R.m_Y                    = 0.0f;
    m_Bar.m_L.m_X                    = 0.0f;
    m_Bar.m_L.m_Y                    = 0.0f;
    m_Bar.m_ExpROffset.m_X           = 0.01f;
    m_Bar.m_ExpROffset.m_Y           = 0.0125f;
    m_Bar.m_ExpLOffset.m_X           = 0.01f;
    m_Bar.m_ExpLOffset.m_Y           = 0.015f;
    m_Bar.m_SoundID                  = M_OPENAL_ERROR_ID;

    // calculate block width (block width * block count + inter space width * block count - 1)
    blockWidth = (m_BlockColumns * 0.06f) + ((m_BlockColumns - 1) * 0.0075f);
    blockTop   =  m_Screen.m_Top - 0.15f;

    // iterate through block lines
    for (j = 0; j < m_BlockLines; ++j)
    {
        // iterate through block columns
        for (i = 0; i < m_BlockColumns; ++i)
        {
            index = (j * m_BlockColumns) + i;

            m_Blocks[index].m_Geometry.m_Pos.m_X       = -(blockWidth * 0.5f) + 0.03f + (i * 0.065f);
            m_Blocks[index].m_Geometry.m_Pos.m_Y       =   blockTop - (j * 0.045f);
            m_Blocks[index].m_Geometry.m_Size.m_Width  =   0.06f;
            m_Blocks[index].m_Geometry.m_Size.m_Height =   0.0125f;
            m_Blocks[index].m_Visible                  =   1;
        }
    }

    // populate vertex format
    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate disk to draw
    miniCreateDisk(0.0f,
                   0.0f,
                   m_Ball.m_Geometry.m_Radius,
                   20,
                   0xFFFF00FF,
                   &m_VertexFormat,
                   &m_pBallVertices,
                   &m_BallVerticesCount);

    surfaceWidth  = 0.06f;
    surfaceHeight = 0.0125f;

    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                      0xFF0033FF,
                      &m_VertexFormat,
                      &m_pBarVertices,
                      &m_BarVerticesCount);

    surfaceWidth  = 0.06f;
    surfaceHeight = 0.025f;

    // update some colors
    m_pBarVertices[11] = 0.4f;
    m_pBarVertices[12] = 0.0f;
    m_pBarVertices[18] = 0.2f;
    m_pBarVertices[19] = 0.4f;
    m_pBarVertices[25] = 0.2f;
    m_pBarVertices[26] = 0.4f;

    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                      0x0000FFFF,
                      &m_VertexFormat,
                      &m_pBlockVertices,
                      &m_BlockVerticesCount);

    // update some colors
    m_pBlockVertices[10] = 0.2f;
    m_pBlockVertices[11] = 0.2f;
    m_pBlockVertices[12] = 0.9f;
    m_pBlockVertices[17] = 0.0f;
    m_pBlockVertices[18] = 0.1f;
    m_pBlockVertices[19] = 0.3f;
    m_pBlockVertices[24] = 0.3f;
    m_pBlockVertices[25] = 0.5f;
    m_pBlockVertices[26] = 0.8f;

    miniInitializeOpenAL(&m_pOpenALDevice, &m_pOpenALContext);

    // hard code file length values, not a good way but for now...
    ballSoundFileLen = 57416;
    barSoundFileLen  = 820644;

    // allocate buffers
    pBallSndBuffer = (unsigned char*)calloc(ballSoundFileLen, sizeof(unsigned char));
    pBarSndBuffer  = (unsigned char*)calloc(barSoundFileLen,  sizeof(unsigned char));

    char* pReboundFileName = 0;
    char* pExplodeFileName = 0;

    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"ball_rebound" :@"wav" :&pReboundFileName];
    [MiniObjectiveCHelper ResourceToFileName :@"bar_explode"  :@"wav" :&pExplodeFileName];

    // load ball sound file
    miniLoadSoundBuffer(pReboundFileName, ballSoundFileLen, &pBallSndBuffer);

    // load bar sound file and get length
    miniLoadSoundBuffer(pExplodeFileName, barSoundFileLen, &pBarSndBuffer);

    free(pReboundFileName);
    free(pExplodeFileName);

    // create ball rebound sound file
    miniCreateSound(m_pOpenALDevice,
                    m_pOpenALContext,
                    pBallSndBuffer,
                    ballSoundFileLen,
                    48000,
                    &m_Ball.m_bufferID,
                    &m_Ball.m_SoundID);

    // create bar explode sound file
    miniCreateSound(m_pOpenALDevice,
                    m_pOpenALContext,
                    pBarSndBuffer,
                    barSoundFileLen,
                    48000,
                    &m_Bar.m_bufferID,
                    &m_Bar.m_SoundID);

    // delete ball sound resource
    if (pBallSndBuffer)
        free(pBallSndBuffer);

    // delete bar sound resource
    if (pBarSndBuffer)
        free(pBarSndBuffer);
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete ball vertices
    if (m_pBallVertices)
    {
        free(m_pBallVertices);
        m_pBallVertices = 0;
    }

    // delete bar vertices
    if (m_pBarVertices)
    {
        free(m_pBarVertices);
        m_pBarVertices = 0;
    }

    // delete block vertices
    if (m_pBlockVertices)
    {
        free(m_pBlockVertices);
        m_pBlockVertices = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;

    // stop running ball rebound sound, if needed
    if (miniIsSoundPlaying(m_Ball.m_SoundID))
        miniStopSound(m_Ball.m_SoundID);

    // stop running bar explode sound, if needed
    if (miniIsSoundPlaying(m_Bar.m_SoundID))
        miniStopSound(m_Bar.m_SoundID);

    // release OpenAL interface
    miniReleaseSound(m_Ball.m_bufferID, m_Ball.m_SoundID);
    miniReleaseSound(m_Bar.m_bufferID,  m_Bar.m_SoundID);
    miniReleaseOpenAL(m_pOpenALDevice, m_pOpenALContext);
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    int   i;
    int   collisionX;
    int   collisionY;
    int   allBlocksBroken;
    int   blockCount;
    int   rebuildLevel;
    int   doPlaySound;
    float left;
    float right;
    float top;

    // set bar y position (may change if the screen change)
    m_Bar.m_Geometry.m_Pos.m_Y = m_Screen.m_BarY;

    // is bar exploding?
    if (m_Bar.m_Exploding)
    {
        // move bar polygons
        m_Bar.m_R.m_X += m_Bar.m_ExpROffset.m_X * (elapsedTime * 50.0f);
        m_Bar.m_R.m_Y += m_Bar.m_ExpROffset.m_Y * (elapsedTime * 50.0f);
        m_Bar.m_L.m_X -= m_Bar.m_ExpLOffset.m_X * (elapsedTime * 50.0f);
        m_Bar.m_L.m_Y += m_Bar.m_ExpLOffset.m_Y * (elapsedTime * 50.0f);

        // explosion ends?
        if (m_Bar.m_L.m_Y > m_Screen.m_Top)
        {
            // reset values
            m_Bar.m_R.m_X     = 0.0f;
            m_Bar.m_R.m_Y     = 0.0f;
            m_Bar.m_L.m_X     = 0.0f;
            m_Bar.m_L.m_Y     = 0.0f;
            m_Bar.m_Exploding = 0;

            // reset x offset velocity
            if (m_Ball.m_Offset.m_X < 0.0f)
                m_Ball.m_Offset.m_X = -M_BALL_VELOCITY_X;
            else
                m_Ball.m_Offset.m_X =  M_BALL_VELOCITY_X;

            // reset y offset velocity
            if (m_Ball.m_Offset.m_Y < 0.0f)
                m_Ball.m_Offset.m_Y = -M_BALL_VELOCITY_Y;
            else
                m_Ball.m_Offset.m_Y =  M_BALL_VELOCITY_Y;
        }
    }

    // move ball
    m_Ball.m_Geometry.m_Pos.m_X += m_Ball.m_Offset.m_X * (elapsedTime * 50.0f);
    m_Ball.m_Geometry.m_Pos.m_Y += m_Ball.m_Offset.m_Y * (elapsedTime * 50.0f);

    collisionX      = 0;
    collisionY      = 0;
    allBlocksBroken = 1;
    blockCount      = m_BlockColumns * m_BlockLines;

    // iterate through blocks, and check each block to find a collision
    for (i = 0; i < blockCount; ++i)
    {
        // ignore broken blocks
        if (!m_Blocks[i].m_Visible)
            continue;

        // at least 1 block is visible
        allBlocksBroken = 0;

        // is ball in collision with block?
        if (miniCircleRectIntersect(&m_Ball.m_Geometry, &m_Blocks[i].m_Geometry))
        {
            // break the block
            m_Blocks[i].m_Visible = 0;

            // rebound happened on the left or right edge?
            if (m_Ball.m_Geometry.m_Pos.m_Y < m_Blocks[i].m_Geometry.m_Pos.m_Y &&
                m_Ball.m_Geometry.m_Pos.m_Y > m_Blocks[i].m_Geometry.m_Pos.m_Y - m_Blocks[i].m_Geometry.m_Size.m_Height)
                collisionX = 1;
            else
                collisionY = 1;
        }
    }

    rebuildLevel = 0;

    // calculate the rounded value of the screen edges, otherwise rounding errors may drive to
    // incorrect collision detections
    left  = m_Screen.m_Left  + m_Ball.m_Geometry.m_Radius;
    right = m_Screen.m_Right - m_Ball.m_Geometry.m_Radius;
    top   = m_Screen.m_Top   - m_Ball.m_Geometry.m_Radius;

    // left or right edge reached?
    if (m_Ball.m_Geometry.m_Pos.m_X > right)
    {
        m_Ball.m_Geometry.m_Pos.m_X = right;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (m_Ball.m_Geometry.m_Pos.m_X < left)
    {
        m_Ball.m_Geometry.m_Pos.m_X = left;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }

    // ball was moving down and is colliding with bar?
    if (m_Ball.m_Offset.m_Y < 0.0f && miniCircleRectIntersect(&m_Ball.m_Geometry, &m_Bar.m_Geometry))
    {
        collisionY = 1;

        // can increase x velocity?
        if (m_Ball.m_Offset.m_X < m_Ball.m_Max.m_X)
            // increase x velocity
            if (m_Ball.m_Offset.m_X > 0.0f)
                m_Ball.m_Offset.m_X += m_Ball.m_Inc.m_X;
            else
                m_Ball.m_Offset.m_X -= m_Ball.m_Inc.m_X;

        // can increase y velocity?
        if (m_Ball.m_Offset.m_X < m_Ball.m_Max.m_Y)
            // increase y velocity
            if (m_Ball.m_Offset.m_Y > 0.0f)
                m_Ball.m_Offset.m_Y += m_Ball.m_Inc.m_Y;
            else
                m_Ball.m_Offset.m_Y -= m_Ball.m_Inc.m_Y;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    // top edge reached?
    if (m_Ball.m_Geometry.m_Pos.m_Y > top)
    {
        m_Ball.m_Geometry.m_Pos.m_Y = top;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    // bottom edge reached?
    if (m_Ball.m_Geometry.m_Pos.m_Y < m_Screen.m_Bottom)
    {
        m_Ball.m_Geometry.m_Pos.m_Y = m_Screen.m_Bottom;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;

        // bottom reached? Game over...
        m_Bar.m_Exploding = 1;

        miniPlaySound(m_Bar.m_SoundID);
    }

    // rebuild level, if needed
    if (rebuildLevel)
    {
        // go to next level
        ++m_Level;

        // last level reached?
        if (m_Level >= 5)
            m_Level = 0;

        // iterate through blocks to regenerate
        for (i = 0; i < blockCount; ++i)
            switch (m_Level)
            {
                case 0: m_Blocks[i].m_Visible = g_Level1[i]; break;
                case 1: m_Blocks[i].m_Visible = g_Level2[i]; break;
                case 2: m_Blocks[i].m_Visible = g_Level3[i]; break;
                case 3: m_Blocks[i].m_Visible = g_Level4[i]; break;
                case 4: m_Blocks[i].m_Visible = g_Level5[i]; break;
            }
    }

    doPlaySound = 0;

    // collision on the x axis?
    if (collisionX)
    {
        m_Ball.m_Offset.m_X = -m_Ball.m_Offset.m_X;
        doPlaySound         = 1;
    }

    // collision on the y axis?
    if (collisionY)
    {
        m_Ball.m_Offset.m_Y = -m_Ball.m_Offset.m_Y;

        // is bar already exploding?
        if (!m_Bar.m_Exploding)
            doPlaySound = 1;
    }

    // play ball rebound sound
    if (doPlaySound == 1)
        miniPlaySound(m_Ball.m_SoundID);
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;
    int          i;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set ball position
    t.m_X = m_Ball.m_Geometry.m_Pos.m_X;
    t.m_Y = m_Ball.m_Geometry.m_Pos.m_Y;
    t.m_Z = 0.0f;

    // calculate model view matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(m_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the ball
    miniDrawDisk(m_pBallVertices, m_BallVerticesCount, &m_VertexFormat, &m_Shader);

    // is bar currently exploding?
    if (!m_Bar.m_Exploding)
    {
        // set bar position
        t.m_X = m_Bar.m_Geometry.m_Pos.m_X;
        t.m_Y = m_Bar.m_Geometry.m_Pos.m_Y;

        // get bar matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the bar
        miniDrawSurface(m_pBarVertices,
                        m_BarVerticesCount,
                        &m_VertexFormat,
                        &m_Shader);
    }
    else
    {
        // connect vertex buffer slots to shader
        glEnableVertexAttribArray(m_Shader.m_VertexSlot);
        glEnableVertexAttribArray(m_Shader.m_ColorSlot);

        // set bar left vertex position
        t.m_X = m_Bar.m_Geometry.m_Pos.m_X + m_Bar.m_L.m_X;
        t.m_Y = m_Bar.m_Geometry.m_Pos.m_Y + m_Bar.m_L.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the first polygon composing the broken bar
        miniDrawBuffer(m_pBarVertices,
                       3,
                       E_Triangles,
                       &m_VertexFormat,
                       &m_Shader);

        // set bar right vertex position
        t.m_X = m_Bar.m_Geometry.m_Pos.m_X + m_Bar.m_R.m_X;
        t.m_Y = m_Bar.m_Geometry.m_Pos.m_Y + m_Bar.m_R.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the second polygon composing the broken bar
        miniDrawBuffer(m_pBarVertices + m_VertexFormat.m_Stride,
                       3,
                       E_Triangles,
                       &m_VertexFormat,
                       &m_Shader);

        // disconnect vertex buffer slots from shader
        glDisableVertexAttribArray(m_Shader.m_VertexSlot);
        glDisableVertexAttribArray(m_Shader.m_ColorSlot);
    }

    // iterate through blocks to draw
    for (i = 0; i < m_BlockColumns * m_BlockLines; ++i)
    {
        // is block visible?
        if (!m_Blocks[i].m_Visible)
            continue;

        // set block position
        t.m_X = m_Blocks[i].m_Geometry.m_Pos.m_X;
        t.m_Y = m_Blocks[i].m_Geometry.m_Pos.m_Y;

        // get block matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect block model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the block
        miniDrawSurface(m_pBlockVertices,
                        m_BlockVerticesCount,
                        &m_VertexFormat,
                        &m_Shader);
    }

    miniEndScene();
}
//----------------------------------------------------------------------------
- (void)OnLongPress :(UIGestureRecognizer*)pSender
{
    // is bar exploding?
    if (m_Bar.m_Exploding)
        return;

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

    // calculate bar next position
    m_Bar.m_Geometry.m_Pos.m_X += ((touchPos.x - m_TouchStartPos.x) / (m_Screen.m_Width)) * 0.001f;

    // is bar out of bounds?
    if (m_Bar.m_Geometry.m_Pos.m_X >= m_Screen.m_Right - (m_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        m_Bar.m_Geometry.m_Pos.m_X = m_Screen.m_Right - (m_Bar.m_Geometry.m_Size.m_Width * 0.5f);
    else
    if (m_Bar.m_Geometry.m_Pos.m_X <= m_Screen.m_Left + (m_Bar.m_Geometry.m_Size.m_Width * 0.5f))
        m_Bar.m_Geometry.m_Pos.m_X = m_Screen.m_Left + (m_Bar.m_Geometry.m_Size.m_Width * 0.5f);

    m_TouchStartPos = touchPos;
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
