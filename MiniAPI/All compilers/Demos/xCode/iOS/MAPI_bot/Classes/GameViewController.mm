/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : An intelligent bot demo                                     *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#import "GameViewController.h"

// std
#include <memory>

// openGL
#import <OpenGLES/ES2/glext.h>

// mini api
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniLevel.h"
#include "MiniRenderer.h"
#include "MiniPlayer.h"

#import "MiniObjectiveCHelper.h"

//------------------------------------------------------------------------------
typedef struct
{
    MINI_Sphere m_Shape;
    float       m_Angle;
    int         m_Fired;
} MINI_Bullet;
//------------------------------------------------------------------------------
typedef struct
{
    float m_Range[2];
} MINI_MDLAnimation;
//------------------------------------------------------------------------------
const char* m_pLevelMap =
"***   ***"
"*********"
"*********"
"*** * ***"
"*** * ***"
"***   ***"
"*********"
"*********"
"***   ***";
//------------------------------------------------------------------------------
const int g_BotPath[] =
{
    0, 0,  0,  0,  0,  0,  0,  0, 0,
    0, 13, 12, 11, 10, 9,  8,  7, 0,
    0, 14, 0,  0,  0,  0,  0,  6, 0,
    0, 15, 0,  0,  0,  0,  0,  5, 0,
    0, 16, 0,  0,  0,  0,  0,  4, 0,
    0, 17, 0,  0,  0,  0,  0,  3, 0,
    0, 18, 0,  0,  0,  0,  0,  2, 0,
    0, 19, 20, 21, 22, 23, 24, 1, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0
};
//------------------------------------------------------------------------------
const char* g_BotVS =
"precision mediump float;"
"attribute vec4  qr_vPosition;"
"attribute vec4  qr_vColor;"
"attribute vec2  qr_vTexCoord;"
"uniform   mat4  qr_uProjection;"
"uniform   mat4  qr_uView;"
"uniform   mat4  qr_uModelview;"
"uniform   float qr_uAlpha;"
"uniform   float qr_uFadeFactor;"
"uniform   int   qr_uRedFilter;"
"varying   vec4  qr_fColor;"
"varying   vec2  qr_fTexCoord;"
"varying   float qr_fAlpha;"
"varying   float qr_fRedFilter;"
"varying   float qr_fFadeFactor;"
"void main(void)"
"{"
"    qr_fColor      = qr_vColor;"
"    qr_fTexCoord   = qr_vTexCoord;"
"    qr_fAlpha      = qr_uAlpha;"
"    qr_fFadeFactor = qr_uFadeFactor;"
"    gl_Position    = qr_uProjection * qr_uView * qr_uModelview * qr_vPosition;"
""
"    if (qr_uRedFilter == 1)"
"        qr_fRedFilter = 1.0;"
"    else"
"        qr_fRedFilter = 0.0;"
"}";
//------------------------------------------------------------------------------
const char* g_BotFS =
"precision mediump float;"
"uniform sampler2D  qr_sColorMap;"
"varying lowp vec4  qr_fColor;"
"varying      vec2  qr_fTexCoord;"
"varying      float qr_fAlpha;"
"varying      float qr_fRedFilter;"
"varying      float qr_fFadeFactor;"
"void main(void)"
"{"
"    vec4 color = qr_fColor * texture2D(qr_sColorMap, qr_fTexCoord);"
""
"    if (qr_fRedFilter > 0.5)"
"        gl_FragColor = vec4(color.x, 0.0, 0.0, qr_fAlpha);"
"    else"
"        gl_FragColor = vec4(color.x * qr_fFadeFactor, color.y * qr_fFadeFactor, color.z * qr_fFadeFactor, qr_fAlpha);"
"}";
//------------------------------------------------------------------------------
// MINI_CallbackManager
//------------------------------------------------------------------------------
MINI_CallbackManager::MINI_CallbackManager()
{}
//------------------------------------------------------------------------------
MINI_CallbackManager::MINI_CallbackManager(void* pOwner) :
    m_pOwner(pOwner)
{}
//------------------------------------------------------------------------------
MINI_CallbackManager::~MINI_CallbackManager()
{}
//------------------------------------------------------------------------------
int MINI_CallbackManager::OnWatching(float elapsedTime, MINI_BotItem* pItem)
{
    return [(__bridge id)pItem->m_pData OnBotWatching :elapsedTime :pItem];
}
//------------------------------------------------------------------------------
int MINI_CallbackManager::OnSearching(float elapsedTime, MINI_BotItem* pItem)
{
    return [(__bridge id)pItem->m_pData OnBotSearching :elapsedTime :pItem];
}
//------------------------------------------------------------------------------
int MINI_CallbackManager::OnAttacking(float elapsedTime, MINI_BotItem* pItem)
{
    return [(__bridge id)pItem->m_pData OnBotAttacking :elapsedTime :pItem];
}
//------------------------------------------------------------------------------
int MINI_CallbackManager::OnBeingHurt(float elapsedTime, MINI_BotItem* pItem)
{
    return [(__bridge id)pItem->m_pData OnBotBeingHurt :elapsedTime :pItem];
}
//------------------------------------------------------------------------------
int MINI_CallbackManager::OnDying(float elapsedTime, MINI_BotItem* pItem)
{
    return [(__bridge id)pItem->m_pData OnBotDying :elapsedTime :pItem];
}
//------------------------------------------------------------------------------
void MINI_CallbackManager::OnNewTask(float elapsedTime, MINI_BotItem* pItem)
{
    [(__bridge id)pItem->m_pData OnNewTask :elapsedTime :pItem];
}
//------------------------------------------------------------------------------
void MINI_CallbackManager::ExecuteTasks(float elapsedTime, MINI_BotItem* pItems)
{
    pItems->m_pData = m_pOwner;
    
    // execute the bot tasks
    miniExecuteTasks(elapsedTime,
                     pItems,
                     1,
                     OnWatching,
                     OnSearching,
                     OnAttacking,
                     OnBeingHurt,
                     OnDying,
                     OnNewTask);
}
//----------------------------------------------------------------------------
// GameViewController
//----------------------------------------------------------------------------
@interface GameViewController()
{
    MINI_Shader       m_Shader;
    MINI_LevelItem*   m_pLevel;
    MINI_MDLModel*    m_pModel;
    MINI_Index*       m_pBulletIndexes;
    GLuint            m_ShaderProgram;
    float*            m_pSurfaceVB;
    float*            m_pBulletVertices;
    float             m_LevelItemWidth;
    float             m_LevelItemHeight;
    float             m_LevelItemLength;
    float             m_PosVelocity;
    float             m_DirVelocity;
    float             m_BotVelocity;
    float             m_BulletVelocity;
    float             m_BotFadeVelocity;
    float             m_ControlRadius;
    float             m_Angle;
    float             m_BotAngle;
    float             m_Time;
    float             m_StepTime;
    float             m_Interval;
    float             m_StepInterval;
    float             m_BotAlphaLevel;
    float             m_BulletAlphaLevel;
    float             m_FadeLevel;
    float             m_FadeLevelVelocity;
    float             m_HitTime;
    unsigned int      m_MapWidth;
    unsigned int      m_MapHeight;
    unsigned int      m_MapLength;
    unsigned int      m_FPS;
    unsigned int      m_SurfaceVertexCount;
    unsigned int      m_MeshIndex;
    unsigned int      m_CurMeshIndex;
    unsigned int      m_AnimLooped;
    unsigned int      m_BotPathPos;
    unsigned int      m_BotHurt;
    unsigned int      m_BotDied;
    unsigned int      m_BotBulletStopped;
    unsigned int      m_PlayerWasFound;
    unsigned int      m_PlayerHit;
    unsigned int      m_PlayerDied;
    unsigned int      m_PlayerEnergy;
    unsigned int      m_BotEnergy;
    unsigned int      m_BulletVertexCount;
    unsigned int      m_BulletIndexCount;
    MINI_Sphere       m_Player;
    MINI_Sphere       m_Bot;
    MINI_Bullet       m_PlayerBullet;
    MINI_Bullet       m_BotBullet;
    MINI_Vector2      m_TouchOrigin;
    MINI_Vector2      m_TouchPosition;
    GLuint            m_SoilTextureIndex;
    GLuint            m_WallTextureIndex;
    GLuint            m_CeilTextureIndex;
    GLuint            m_ModelTextureIndex;
    GLuint            m_BulletTextureIndex;
    GLuint            m_TexSamplerSlot;
    GLuint            m_AlphaSlot;
    GLuint            m_FadeFactorSlot;
    GLuint            m_RedFilterSlot;
    GLuint            m_ViewUniform;
    GLuint            m_ModelviewUniform;
    ALCdevice*        m_pOpenALDevice;
    ALCcontext*       m_pOpenALContext;
    ALuint            m_PlayerStepBufferID;
    ALuint            m_PlayerStepSoundID;
    ALuint            m_PlayerFireBufferID;
    ALuint            m_PlayerFireSoundID;
    MINI_VertexFormat m_VertexFormat;
    MINI_VertexFormat m_ModelFormat;
    MINI_VertexFormat m_BulletVertexFormat;
    MINI_MDLAnimation m_Animation[5];
    MINI_BotItem      m_Items[1];
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
* Converts a path position to an level item index
*@param pathPos - path position to convert
*@param pPath - path
*@param pathLength - path length
*@return level item index matching with path position
*/
- (int) PathPosToLevelIndex :(unsigned)pathPos :(const int*)pPath :(unsigned)pathLength;

/**
* Checks if a bullet hit a body
*@param pBullet - bullet to check
*@param pBody - body to check against
*@return 1 if the bullet hit the body, otherwise 0
*/
- (int) BulletHitBody :(MINI_Bullet*)pBullet :(MINI_Sphere*)pBody;

/**
* Checks if a bullet hit the player
*@return 1 if the bullet hit the player, otherwise 0
*/
- (int) BulletHitPlayer;

/**
* Checks if a bullet hit the bot
*@return 1 if the bullet hit the bot, otherwise 0
*/
- (int) BulletHitBot;

/**
* Runs and loops the bot animation
*@param animIndex - animation index
*@param elapsedTime - elapsed time since last process
*/
- (void) BotRunAnimation :(int)animIndex :(float)elapsedTime;

/**
* Runs the bot animation once
*@param animIndex - animation index
*@param elapsedTime - elapsed time since last process
*@return 1 if animation end was reached, otherwise 0
*/
- (int) BotRunAnimationOnce :(int)animIndex :(float)elapsedTime;

/**
* Checks if the bot spotted the player
*@return 1 if the bot spotted the player, otherwise 0
*/
- (int) BotSpottedPlayer;

/**
* Calculates the angle to apply to bot to look at the player
*@return angle in radian
*/
- (float) BotLookAtPlayerAngle;

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
*@param pSender - recognizer that raised the event
*/
- (void) OnLongPress :(UIGestureRecognizer*)pSender;

@end
//----------------------------------------------------------------------------
@implementation GameViewController
//----------------------------------------------------------------------------
- (void) viewDidLoad
{
    [super viewDidLoad];

    m_pLevel             =  0;
    m_pModel             =  0;
    m_pBulletIndexes     =  0;
    m_ShaderProgram      =  0;
    m_pSurfaceVB         =  0;
    m_pBulletVertices    =  0;
    m_LevelItemWidth     =  5.0f;
    m_LevelItemHeight    =  5.0f;
    m_LevelItemLength    =  5.0f;
    m_PosVelocity        =  100.0f;
    m_DirVelocity        = -20.0f;
    m_BotVelocity        =  10.0f;
    m_BulletVelocity     =  25.0f;
    m_BotFadeVelocity    =  0.5f;
    m_ControlRadius      =  40.0f;
    m_Angle              =  0.0f;
    m_BotAngle           =  0.0f;
    m_Time               =  0.0f;
    m_StepTime           =  0.0f;
    m_Interval           =  0.0f;
    m_StepInterval       =  350.0f;
    m_BotAlphaLevel      =  1.0f;
    m_BulletAlphaLevel   =  0.8f;
    m_FadeLevel          =  1.0f;
    m_FadeLevelVelocity  =  0.5f;
    m_HitTime            =  0.0f;
    m_MapWidth           =  9;
    m_MapHeight          =  9;
    m_MapLength          =  m_MapWidth * m_MapHeight;
    m_FPS                =  15;
    m_SurfaceVertexCount =  0;
    m_MeshIndex          =  0;
    m_CurMeshIndex       =  0;
    m_AnimLooped         =  0;
    m_BotPathPos         =  1;
    m_BotHurt            =  0;
    m_BotDied            =  0;
    m_BotBulletStopped   =  0;
    m_PlayerWasFound     =  0;
    m_PlayerHit          =  0;
    m_PlayerDied         =  0;
    m_PlayerEnergy       =  3; // the number of impacts the player can support before die
    m_BotEnergy          =  3; // the number of impacts the bot can support before die
    m_SoilTextureIndex   =  GL_INVALID_VALUE;
    m_WallTextureIndex   =  GL_INVALID_VALUE;
    m_CeilTextureIndex   =  GL_INVALID_VALUE;
    m_ModelTextureIndex  =  GL_INVALID_VALUE;
    m_BulletTextureIndex =  GL_INVALID_VALUE;
    m_TexSamplerSlot     =  0;
    m_AlphaSlot          =  0;
    m_FadeFactorSlot     =  0;
    m_RedFilterSlot      =  0;
    m_ViewUniform        =  0;
    m_ModelviewUniform   =  0;
    m_pOpenALDevice      =  0;
    m_pOpenALContext     =  0;
    m_PlayerStepBufferID =  0;
    m_PlayerStepSoundID  =  0;
    m_PlayerFireBufferID =  0;
    m_PlayerFireSoundID  =  0;
    m_PreviousTime       =  CACurrentMediaTime();

    // add a long press gesture to the view
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
- (int) PathPosToLevelIndex :(unsigned)pathPos :(const int*)pPath :(unsigned)pathLength
{
    unsigned i;
    
    // search the level item the path position represents
    for (i = 0; i < pathLength; ++i)
        if (pPath[i] == pathPos)
            return i;
    
    return -1;
}
//------------------------------------------------------------------------------
- (int) BulletHitBody :(MINI_Bullet*)pBullet :(MINI_Sphere*)pBody
{
    // no bullet?
    if (!pBullet)
        return 0;
    
    // no body?
    if (!pBody)
        return 0;
    
    // was bullet fired?
    if (!pBullet->m_Fired)
        return 0;
    
    // was body hit by bullet? NOTE just do a simple approximation for the hit detection. It's
    // enough for this demo, but in a real game a more complex polygon collision detection
    // should be done. See miniPopulateTree() and miniResolveTree() for further information
    if (!miniSpheresIntersect(pBody, &pBullet->m_Shape))
        return 0;
    
    // stop the bullet
    pBullet->m_Fired = 0;
    
    return 1;
}
//------------------------------------------------------------------------------
- (int) BulletHitPlayer
{
    // has bullet hit the player?
    if (![self BulletHitBody :&m_BotBullet :&m_Player])
        return 0;
    
    // player loose a point of energy
    if (m_PlayerEnergy)
        --m_PlayerEnergy;
    
    return 1;
}
//------------------------------------------------------------------------------
- (int) BulletHitBot
{
    // has bullet hit the bot?
    if (![self BulletHitBody :&m_PlayerBullet :&m_Bot])
        return 0;
    
    // bot loose a point of energy
    if (m_BotEnergy)
        --m_BotEnergy;
    
    // notify that bot was hurt
    m_BotHurt = 1;
    
    return 1;
}
//------------------------------------------------------------------------------
- (void) BotRunAnimation :(int)animIndex :(float)elapsedTime
{
    int frameCount = 0;
    int deltaRange = (int)(m_Animation[animIndex].m_Range[1] -
                           m_Animation[animIndex].m_Range[0]);
    
    // calculate next time
    m_Time += (elapsedTime * 1000.0f);
    
    // count frames
    while (m_Time > m_Interval)
    {
        m_Time -= m_Interval;
        ++frameCount;
    }
    
    // calculate next mesh index to show. Index should always be between animation range
    m_CurMeshIndex = ((m_CurMeshIndex + frameCount) % deltaRange);
    m_MeshIndex    = (int)m_Animation[animIndex].m_Range[0] + m_CurMeshIndex;
}
//------------------------------------------------------------------------------
- (int) BotRunAnimationOnce :(int)animIndex :(float)elapsedTime
{
    unsigned int frameCount = 0;
    
    // calculate next time
    m_Time += (elapsedTime * 1000.0f);
    
    // count frames
    while (m_Time > m_Interval)
    {
        m_Time -= m_Interval;
        ++frameCount;
    }
    
    // calculate next mesh index to show
    m_MeshIndex += frameCount;
    
    // was animation end reached?
    if (m_MeshIndex >= (int)m_Animation[animIndex].m_Range[1])
    {
        m_MeshIndex = (int)m_Animation[animIndex].m_Range[1];
        return 1;
    }
    
    return 0;
}
//------------------------------------------------------------------------------
- (int) BotSpottedPlayer
{
    MINI_Vector2 start;
    MINI_Vector2 end;
    MINI_Vector3 dir;
    MINI_Vector3 nDir;
    MINI_Plane   botFieldOfView;
    float        distToFOV;
    
    // player is no longer visible if he died
    if (m_PlayerDied)
        return 0;
    
    dir.m_Y = 0.0f;
    
    // calculate the direction for the bot left field of view plane
    dir.m_X = -cosf(((M_PI * 2.0f) - m_BotAngle) - 0.35f);
    dir.m_Z = -sinf(((M_PI * 2.0f) - m_BotAngle) - 0.35f);
    
    // normalize it
    miniNormalize(&dir, &nDir);
    
    // calculate the bot left field of view plane
    miniPlaneFromPointNormal(&m_Bot.m_Pos, &nDir, &botFieldOfView);
    
    // calculate if the player is inside the bot left field of view
    miniDistanceToPlane(&m_Player.m_Pos, &botFieldOfView, &distToFOV);
    
    if (distToFOV >= 0.0f)
        return 0;
    
    // calculate the direction for the bot field of view right plane
    dir.m_X = -cosf(((M_PI * 2.0f) - m_BotAngle) + 0.35f);
    dir.m_Z = -sinf(((M_PI * 2.0f) - m_BotAngle) + 0.35f);
    
    // normalize it
    miniNormalize(&dir, &nDir);
    
    // calculate the bot right field of view plane
    miniPlaneFromPointNormal(&m_Bot.m_Pos, &nDir, &botFieldOfView);
    
    // calculate if the player is inside the bot right field of view
    miniDistanceToPlane(&m_Player.m_Pos, &botFieldOfView, &distToFOV);
    
    if (distToFOV >= 0.0f)
        return 0;
    
    start.m_X = m_Player.m_Pos.m_X;
    start.m_Y = m_Player.m_Pos.m_Z;
    
    end.m_X = m_Bot.m_Pos.m_X;
    end.m_Y = m_Bot.m_Pos.m_Z;
    
    // check if a wall is hiding the player
    if (!miniBodyIntersectWall(&start,
                               &end,
                               m_pLevel,
                               m_LevelItemWidth,
                               m_LevelItemHeight,
                               m_MapLength))
    {
        // unfortunately not :-) the player was shown by the bot
        m_PlayerWasFound = 1;
        return 1;
    }
    
    return 0;
}
//------------------------------------------------------------------------------
- (float) BotLookAtPlayerAngle
{
    MINI_Vector3 dir;
    MINI_Vector3 normal;
    float        angle;
    float        botAngle;
    
    // calculate the normal from bot position and pointing to player
    miniSub(&m_Bot.m_Pos, &m_Player.m_Pos, &dir);
    miniNormalize(&dir, &normal);
    
    // calculate the angle to apply to bot to look at the player
    angle = normal.m_Z / normal.m_X;
    
    if (normal.m_X <= 0.0f)
        botAngle = (M_PI * 2.0f) - atan(angle);
    else
        botAngle = M_PI - atan(angle);
    
    // and validate it
    if (botAngle > M_PI * 2.0f)
        botAngle -= M_PI * 2.0f;
    else
    if (botAngle < 0.0f)
        botAngle += M_PI * 2.0f;

    return botAngle;
}
//------------------------------------------------------------------------------
- (int) OnBotWatching :(float)elapsedTime :(MINI_BotItem*)pItem
{
    // run and loop the bot animation
    [self BotRunAnimation :0 :elapsedTime];

    // do fade in the bot?
    if (m_BotAlphaLevel != 1.0f)
    {
        // increase the bot alpha level
        m_BotAlphaLevel += (m_BotFadeVelocity * elapsedTime);
        
        // maximal alpha level reached?
        if (m_BotAlphaLevel > 1.0f)
            m_BotAlphaLevel = 1.0f;

        return 0;
    }

    // bot will execute the next task if:
    // 1. he is hit by a bullet
    // 2. he shows the player
    // 3. he randomly decides to search the player
    return ([self BulletHitBot] || [self BotSpottedPlayer] || (rand() % 200 == 15));
}
//------------------------------------------------------------------------------
- (int) OnBotSearching :(float)elapsedTime :(MINI_BotItem*)pItem
{
    int   startLevelItemIndex;
    int   endLevelItemIndex;
    int   doGetNextItem;
    float startX;
    float startY;
    float endX;
    float endY;
    float lengthX;
    float lengthY;
    
    // run and loop the bot animation
    [self BotRunAnimation :1 :elapsedTime];
    
    // is the search starting?
    if (m_BotPathPos == 1)
    {
        // set the bot to the first position and select the next level item to reach
        m_Bot.m_Pos.m_X = m_pLevel[70].m_X + (m_MapWidth  * 0.5f);
        m_Bot.m_Pos.m_Z = m_pLevel[70].m_Y + (m_MapHeight * 0.5f);
        ++m_BotPathPos;
    }
    
    // get the level item on which the bot is located, and the next item to reach. NOTE a simple pre-calculated
    // table is used here. This is enough for this demo but in a real game the search should use a path finding
    // algorithm to be really efficient
    startLevelItemIndex = [self PathPosToLevelIndex :m_BotPathPos     :g_BotPath :m_MapLength];
    endLevelItemIndex   = [self PathPosToLevelIndex :m_BotPathPos + 1 :g_BotPath :m_MapLength];
    
    // is the last search pos reached?
    if (endLevelItemIndex == -1)
        endLevelItemIndex = [self PathPosToLevelIndex :1 :g_BotPath :m_MapLength];
    
    // is the search end reached?
    if (startLevelItemIndex == -1)
    {
        // set the bot to the last position, and reset the search task
        m_Bot.m_Pos.m_X = m_pLevel[70].m_X + (m_MapWidth  * 0.5f);
        m_Bot.m_Pos.m_Z = m_pLevel[70].m_Y + (m_MapHeight * 0.5f);
        m_BotAngle      = M_PI * 0.5f;
        m_BotPathPos    = 1;
        
        // notify that the task is completed
        return 1;
    }
    
    // get the real start and end position, and calculate the length
    startX  = m_pLevel[startLevelItemIndex].m_X + (m_MapWidth  * 0.5f);
    startY  = m_pLevel[startLevelItemIndex].m_Y + (m_MapHeight * 0.5f);
    endX    = m_pLevel[endLevelItemIndex].m_X   + (m_MapWidth  * 0.5f);
    endY    = m_pLevel[endLevelItemIndex].m_Y   + (m_MapHeight * 0.5f);
    lengthX = startX - endX;
    lengthY = endY   - startY;
    
    // calculate the both direction
    if (!lengthY)
    {
        if (lengthX < 0)
            m_BotAngle = 0.0f;
        else
            m_BotAngle = M_PI;
    }
    else
    if (!lengthX)
    {
        if (lengthY < 0)
            m_BotAngle = (M_PI * 0.5f);
        else
            m_BotAngle = -(M_PI * 0.5f);
    }
    else
        m_BotAngle = atanf(lengthX / lengthY) + (M_PI * 0.5f);
    
    // and validate it
    if (m_BotAngle > M_PI * 2.0f)
        m_BotAngle -= M_PI * 2.0f;
    else
    if (m_BotAngle < 0.0f)
        m_BotAngle += M_PI * 2.0f;
    
    // calculate the bot next position
    m_Bot.m_Pos.m_X += m_BotVelocity * cosf(m_BotAngle) * elapsedTime;
    m_Bot.m_Pos.m_Z -= m_BotVelocity * sinf(m_BotAngle) * elapsedTime;
    
    doGetNextItem = 0;
    
    // do get the next item (from the x axis)
    if (endX > startX && m_Bot.m_Pos.m_X >= endX)
        doGetNextItem = 1;
    else
    if (endX < startX && m_Bot.m_Pos.m_X <= endX)
        doGetNextItem = 1;
    
    // do get the next item (from the z axis)
    if (endY > startY && m_Bot.m_Pos.m_Z >= endY)
        doGetNextItem = 1;
    else
    if (endY < startY && m_Bot.m_Pos.m_Z <= endY)
        doGetNextItem = 1;
    
    // get the next item, if required
    if (doGetNextItem)
        ++m_BotPathPos;
    
    // bot will execute the next task if:
    // 1. he is hit by a bullet
    // 2. he shows the player
    return ([self BulletHitBot] || [self BotSpottedPlayer]);
}
//------------------------------------------------------------------------------
- (int)OnBotAttacking :(float)elapsedTime :(MINI_BotItem*)pItem
{
    // a player also fired a bullet, and this bullet hit the bot?
    if ([self BulletHitBot])
        return 1;
    
    // run the bot animation
    [self BotRunAnimationOnce :2 :elapsedTime];

    // make the bot looking the player
    m_BotAngle = [self BotLookAtPlayerAngle];

    // bullet hit something?
    if (m_BotBulletStopped)
    {
        m_BotBulletStopped = 0;
        
        // is the player still visible?
        if (![self BotSpottedPlayer])
            return 1;
        
        // reload the animation to allow to fire a new bullet
        m_MeshIndex = (int)m_Animation[2].m_Range[0];
    }
    else
    // did the bot fired?
    if (!m_BotBullet.m_Fired)
    {
        m_BotBulletStopped = 0;

        // fire a new bullet
        m_BotBullet.m_Shape.m_Pos.m_X =  m_Bot.m_Pos.m_X;
        m_BotBullet.m_Shape.m_Pos.m_Z =  m_Bot.m_Pos.m_Z;
        m_BotBullet.m_Angle           = (M_PI * 2.0f) - (m_BotAngle - (M_PI * 0.5f));
        m_BotBullet.m_Fired           =  1;

        miniPlaySound(m_PlayerFireSoundID);
    }

    // the task ends when the animation was executed completely
    if (m_AnimLooped)
        // and only if user is no longer visible
        return (![self BotSpottedPlayer]);
    
    return 0;
}
//------------------------------------------------------------------------------
- (int) OnBotBeingHurt :(float)elapsedTime :(MINI_BotItem*)pItem
{
    // the task ends when the animation was executed completely
    return [self BotRunAnimationOnce :3 :elapsedTime];
}
//------------------------------------------------------------------------------
- (int) OnBotDying :(float)elapsedTime :(MINI_BotItem*)pItem
{
    // bot animation was executed?
    if (m_BotDied)
    {
        // decrease the bot alpha level
        m_BotAlphaLevel -= (m_BotFadeVelocity * elapsedTime);
        
        // minimal alpha level reached?
        if (m_BotAlphaLevel > 0.0f)
            return 0;
        
        // yes, it's time to restart
        m_BotAlphaLevel = 0.0f;
        return 1;
    }
    
    // run dying animation once
    if ([self BotRunAnimationOnce :4 :elapsedTime])
        m_BotDied = 1;
    
    return 0;
}
//------------------------------------------------------------------------------
- (void) OnNewTask :(float)elapsedTime :(MINI_BotItem*)pItem
{
    // bot died?
    if (m_BotDied)
    {
        m_BotDied = 0;
        
        // reset all bot data and restart to wait
        m_Bot.m_Pos.m_X   =  m_pLevel[70].m_X + (m_MapWidth * 0.5f);
        m_Bot.m_Pos.m_Y   = -1.0f;
        m_Bot.m_Pos.m_Z   =  m_pLevel[70].m_Y + (m_MapHeight * 0.5f);
        m_Bot.m_Radius    =  2.0f;
        m_BotAngle        =  M_PI * 0.5f;
        m_BotEnergy       =  3;
        m_BotPathPos      =  1;
        m_Items[0].m_Task =  E_BT_Watching;
        m_MeshIndex       = (int)m_Animation[0].m_Range[0];
        
        return;
    }
    
    // bot spotted the player?
    if (m_PlayerWasFound)
    {
        m_PlayerWasFound = 0;
        
        // attack it
        pItem->m_Task = E_BT_Attacking;
        
        // set the animation index to start pos, to avoid jumps in animation
        m_MeshIndex = (int)m_Animation[2].m_Range[0];
        
        return;
    }
    
    // bot was hurt?
    if (m_BotHurt)
    {
        m_BotHurt = 0;
        
        // bot still have energy?
        if (m_BotEnergy)
        {
            // set the animation index to start pos, to avoid jumps in animation
            m_MeshIndex = (int)m_Animation[3].m_Range[0];
            
            pItem->m_Task = E_BT_Being_Hurt;
            return;
        }

        // set the animation index to start pos, to avoid jumps in animation
        m_MeshIndex = (int)m_Animation[4].m_Range[0];
        
        pItem->m_Task = E_BT_Dying;
        return;
    }
    
    // bot was searching?
    if (pItem->m_Task == E_BT_Searching)
    {
        // return to watch
        pItem->m_Task = E_BT_Watching;
        m_MeshIndex   = (int)m_Animation[0].m_Range[0];
        return;
    }
    
    // begin the search
    pItem->m_Task = E_BT_Searching;
    m_MeshIndex   = (int)m_Animation[1].m_Range[0];
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
    projectionUniform = glGetUniformLocation(m_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//----------------------------------------------------------------------------
- (void)InitScene
{
    float          bulletRadius;
    unsigned int   playerStepSoundFileLen;
    unsigned int   playerFireSoundFileLen;
    unsigned char* pPlayerStepSndBuffer;
    unsigned char* pPlayerFireSndBuffer;
    MINI_Texture   texture;
    
    bulletRadius = 0.1f;
    
    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(g_BotVS, g_BotFS);
    glUseProgram(m_ShaderProgram);
    
    // get shader attributes
    m_Shader.m_VertexSlot   = glGetAttribLocation(m_ShaderProgram,  "qr_vPosition");
    m_Shader.m_ColorSlot    = glGetAttribLocation(m_ShaderProgram,  "qr_vColor");
    m_Shader.m_TexCoordSlot = glGetAttribLocation(m_ShaderProgram,  "qr_vTexCoord");
    m_TexSamplerSlot        = glGetAttribLocation(m_ShaderProgram,  "qr_sColorMap");
    m_AlphaSlot             = glGetUniformLocation(m_ShaderProgram, "qr_uAlpha");
    m_FadeFactorSlot        = glGetUniformLocation(m_ShaderProgram, "qr_uFadeFactor");
    m_RedFilterSlot         = glGetUniformLocation(m_ShaderProgram, "qr_uRedFilter");
    
    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);
    
    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // get the screen rect
    CGRect screenRect = [[UIScreen mainScreen] bounds];

    // create the viewport
    [self CreateViewport :screenRect.size.width :screenRect.size.height];

    // generate the level
    miniGenerateLevel(m_pLevelMap,
                      m_MapWidth,
                      m_MapHeight,
                      &m_LevelItemWidth,
                      &m_LevelItemHeight,
                      &m_pLevel);
    
    m_BulletVertexFormat.m_UseNormals  = 0;
    m_BulletVertexFormat.m_UseTextures = 1;
    m_BulletVertexFormat.m_UseColors   = 1;
    
    // generate sphere
    miniCreateSphere(&bulletRadius,
                     5,
                     5,
                     0xFFFFFFFF,
                     &m_BulletVertexFormat,
                     &m_pBulletVertices,
                     &m_BulletVertexCount,
                     &m_pBulletIndexes,
                     &m_BulletIndexCount);
    
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
    
    char* pSoilTextureName   = 0;
    char* pWallTextureName   = 0;
    char* pCeilTextureName   = 0;
    char* pBulletTextureName = 0;
    
    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"Soil_25_256x256"           :@"bmp" :&pSoilTextureName];
    [MiniObjectiveCHelper ResourceToFileName :@"Wall_Tilleable_64_256x256" :@"bmp" :&pWallTextureName];
    [MiniObjectiveCHelper ResourceToFileName :@"Soil_2_25_256x256"         :@"bmp" :&pCeilTextureName];
    [MiniObjectiveCHelper ResourceToFileName :@"bullet"                    :@"bmp" :&pBulletTextureName];
    
    // load textures
    m_SoilTextureIndex   = miniLoadTexture(pSoilTextureName);
    m_WallTextureIndex   = miniLoadTexture(pWallTextureName);
    m_CeilTextureIndex   = miniLoadTexture(pCeilTextureName);
    m_BulletTextureIndex = miniLoadTexture(pBulletTextureName);
    
    free(pSoilTextureName);
    free(pWallTextureName);
    free(pCeilTextureName);
    free(pBulletTextureName);
    
    m_ModelFormat.m_UseNormals  = 0;
    m_ModelFormat.m_UseTextures = 1;
    m_ModelFormat.m_UseColors   = 1;
    
    char* pModelName = 0;

    // get the resource file path
    [MiniObjectiveCHelper ResourceToFileName :@"wizard" :@"mdl" :&pModelName];

    // load MDL file and create mesh to draw
    miniLoadMDLModel((unsigned char*)pModelName,
                    &m_ModelFormat,
                     0xFFFFFFFF,
                    &m_pModel,
                    &texture);

    free(pModelName);

    // create new OpenGL texture
    glGenTextures(1, &m_ModelTextureIndex);
    glBindTexture(GL_TEXTURE_2D, m_ModelTextureIndex);
    
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
    
    miniInitializeOpenAL(&m_pOpenALDevice, &m_pOpenALContext);
    
    char* playerStepSoundFileName = 0;
    char* playerFireSoundFileName = 0;
    
    // get the resource file paths
    [MiniObjectiveCHelper ResourceToFileName :@"human_walk_step" :@"wav" :&playerStepSoundFileName];
    [MiniObjectiveCHelper ResourceToFileName :@"fire_and_reload" :@"wav" :&playerFireSoundFileName];

    // get the sound files length
    playerStepSoundFileLen = miniGetFileSize(playerStepSoundFileName);
    playerFireSoundFileLen = miniGetFileSize(playerFireSoundFileName);
    
    // allocate buffers
    pPlayerStepSndBuffer = (unsigned char*)calloc(playerStepSoundFileLen, sizeof(unsigned char));
    pPlayerFireSndBuffer = (unsigned char*)calloc(playerFireSoundFileLen, sizeof(unsigned char));
    
    // load step sound file
    miniLoadSoundBuffer(playerStepSoundFileName,
                        playerStepSoundFileLen,
                        &pPlayerStepSndBuffer);
    
    // load fire sound file and get length
    miniLoadSoundBuffer(playerFireSoundFileName,
                        playerFireSoundFileLen,
                        &pPlayerFireSndBuffer);

    free(playerStepSoundFileName);
    free(playerFireSoundFileName);

    // create step sound file
    miniCreateSound(m_pOpenALDevice,
                    m_pOpenALContext,
                    pPlayerStepSndBuffer,
                    playerStepSoundFileLen,
                    48000,
                    &m_PlayerStepBufferID,
                    &m_PlayerStepSoundID);
    
    // create fire sound file
    miniCreateSound(m_pOpenALDevice,
                    m_pOpenALContext,
                    pPlayerFireSndBuffer,
                    playerFireSoundFileLen,
                    48000,
                    &m_PlayerFireBufferID,
                    &m_PlayerFireSoundID);
    
    // delete step sound resource
    if (pPlayerStepSndBuffer)
        free(pPlayerStepSndBuffer);
    
    // delete fire sound resource
    if (pPlayerFireSndBuffer)
        free(pPlayerFireSndBuffer);
    
    // initialize the player
    m_Player.m_Pos.m_X = 0.0f;
    m_Player.m_Pos.m_Y = 0.0f;
    m_Player.m_Pos.m_Z = 0.0f;
    m_Player.m_Radius  = 2.0f;
    
    m_Angle = 0.0f;
    
    // initialize the bot
    m_Bot.m_Pos.m_X =  m_pLevel[70].m_X + (m_MapWidth * 0.5f);
    m_Bot.m_Pos.m_Y = -1.0f;
    m_Bot.m_Pos.m_Z =  m_pLevel[70].m_Y + (m_MapHeight * 0.5f);
    m_Bot.m_Radius  =  2.0f;
    
    m_BotAngle = M_PI * 0.5f;
    
    // initialize the bot bullet
    m_BotBullet.m_Shape.m_Pos.m_X = m_pLevel[70].m_X + (m_MapWidth * 0.5f);
    m_BotBullet.m_Shape.m_Pos.m_Y = 0.0f;
    m_BotBullet.m_Shape.m_Pos.m_Z = m_pLevel[70].m_Y + (m_MapHeight * 0.5f);
    m_BotBullet.m_Shape.m_Radius  = bulletRadius;
    m_BotBullet.m_Angle           = 0.0f;
    m_BotBullet.m_Fired           = 0;

    // initialize the bot task
    m_Items[0].m_Task = E_BT_Watching;

    // initialize the player bullet
    m_PlayerBullet.m_Shape.m_Pos.m_X = 0.0f;
    m_PlayerBullet.m_Shape.m_Pos.m_Y = 0.0f;
    m_PlayerBullet.m_Shape.m_Pos.m_Z = 0.0f;
    m_PlayerBullet.m_Shape.m_Radius  = bulletRadius;
    m_PlayerBullet.m_Angle           = 0.0f;
    m_PlayerBullet.m_Fired           = 0;
    
    // initialize the touch
    m_TouchOrigin.m_X   = 0.0f;
    m_TouchOrigin.m_Y   = 0.0f;
    m_TouchPosition.m_X = 0.0f;
    m_TouchPosition.m_Y = 0.0f;
    
    // create MD2 animation list
    m_Animation[0].m_Range[0] = 0;  m_Animation[0].m_Range[1] = 14; // wizard hovers
    m_Animation[1].m_Range[0] = 15; m_Animation[1].m_Range[1] = 28; // wizard flies
    m_Animation[2].m_Range[0] = 29; m_Animation[2].m_Range[1] = 42; // wizard attacks
    m_Animation[3].m_Range[0] = 43; m_Animation[3].m_Range[1] = 47; // wizard feels pain
    m_Animation[4].m_Range[0] = 48; m_Animation[4].m_Range[1] = 53; // wizard dies
    
    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    miniReleaseMDLModel(m_pModel);
    m_pModel = 0;
    
    if (m_ModelTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &m_ModelTextureIndex);
    
    m_ModelTextureIndex = GL_INVALID_VALUE;
    
    // delete bullet vertex buffer
    if (m_pBulletVertices)
    {
        free(m_pBulletVertices);
        m_pBulletVertices = 0;
    }
    
    // delete bullet index buffer
    if (m_pBulletIndexes)
    {
        free(m_pBulletIndexes);
        m_pBulletIndexes = 0;
    }
    
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
    
    // stop running step sound, if needed
    if (miniIsSoundPlaying(m_PlayerStepSoundID))
        miniStopSound(m_PlayerStepSoundID);
    
    // stop running fire sound, if needed
    if (miniIsSoundPlaying(m_PlayerFireSoundID))
        miniStopSound(m_PlayerFireSoundID);
    
    // release OpenAL interface
    miniReleaseSound(m_PlayerStepBufferID, m_PlayerStepSoundID);
    miniReleaseSound(m_PlayerFireBufferID, m_PlayerFireSoundID);
    miniReleaseOpenAL(m_pOpenALDevice, m_pOpenALContext);
}
//----------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    float        angle;
    MINI_Vector3 newPos;
    MINI_Vector2 bulletStart;
    MINI_Vector2 bulletEnd;

    std::auto_ptr<MINI_CallbackManager> pCallbackMgr(new MINI_CallbackManager((__bridge void*)self));

    // execute the bot tasks
    pCallbackMgr->ExecuteTasks(elapsedTime, m_Items);

    // player died?
    if (m_PlayerDied)
    {
        // fade to black
        m_FadeLevel -= (m_FadeLevelVelocity * elapsedTime);
        
        // reset the player hit value
        m_PlayerHit = 0;
        
        // minimal fade value was reached?
        if (m_FadeLevel <= 0.0f)
        {
            m_FadeLevel = 0.0f;
            
            // resurrect the player
            m_Player.m_Pos.m_X = 0.0f;
            m_Player.m_Pos.m_Y = 0.0f;
            m_Player.m_Pos.m_Z = 0.0f;
            m_Angle            = 0.0f;
            
            // and restore his status and energy
            m_PlayerEnergy = 3;
            m_PlayerDied   = 0;
        }
        
        return;
    }
    else
    if (m_FadeLevel < 1.0f)
    {
        // fade from black
        m_FadeLevel += (m_FadeLevelVelocity * elapsedTime);
            
        // maximal fade value was reached?
        if (m_FadeLevel >= 1.0f)
            m_FadeLevel = 1.0f;
    }

    // bot bullet hit the player?
    if ([self BulletHitPlayer])
    {
        m_PlayerHit = 1;
        m_HitTime   = 0.0f;
        
        // remove one energy, notify that player died if no energy remains
        if (m_PlayerEnergy)
            --m_PlayerEnergy;
        else
            m_PlayerDied = 1;
    }
    else
    if (m_HitTime < 0.1f)
        m_HitTime += elapsedTime;
    else
        m_PlayerHit = 0;
    
    // did the player fired?
    if (m_PlayerBullet.m_Fired)
    {
        bulletStart.m_X = m_PlayerBullet.m_Shape.m_Pos.m_X;
        bulletStart.m_Y = m_PlayerBullet.m_Shape.m_Pos.m_Z;
        
        // calculate the bullet next position
        m_PlayerBullet.m_Shape.m_Pos.m_X -= m_BulletVelocity * cosf(m_PlayerBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;
        m_PlayerBullet.m_Shape.m_Pos.m_Z -= m_BulletVelocity * sinf(m_PlayerBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;
        
        bulletEnd.m_X = m_PlayerBullet.m_Shape.m_Pos.m_X;
        bulletEnd.m_Y = m_PlayerBullet.m_Shape.m_Pos.m_Z;
        
        // check if the bullet intersected a wall
        if (miniBodyIntersectWall(&bulletStart,
                                  &bulletEnd,
                                  m_pLevel,
                                  m_LevelItemWidth,
                                  m_LevelItemHeight,
                                  m_MapLength))
            m_PlayerBullet.m_Fired = 0;
    }
    
    // did the bot fired?
    if (m_BotBullet.m_Fired)
    {
        // calculate the next bullet position
        bulletStart.m_X = m_BotBullet.m_Shape.m_Pos.m_X;
        bulletStart.m_Y = m_BotBullet.m_Shape.m_Pos.m_Z;
        
        m_BotBullet.m_Shape.m_Pos.m_X -= m_BulletVelocity * cosf(m_BotBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;
        m_BotBullet.m_Shape.m_Pos.m_Z -= m_BulletVelocity * sinf(m_BotBullet.m_Angle + (M_PI * 0.5f)) * elapsedTime;
        
        bulletEnd.m_X = m_BotBullet.m_Shape.m_Pos.m_X;
        bulletEnd.m_Y = m_BotBullet.m_Shape.m_Pos.m_Z;
        
        // check if the bullet intersected a wall
        if (miniBodyIntersectWall(&bulletStart,
                                  &bulletEnd,
                                  m_pLevel,
                                  m_LevelItemWidth,
                                  m_LevelItemHeight,
                                  m_MapLength))
        {
            // yes, stop the bullet
            m_BotBullet.m_Fired = 0;
            
            // and notify that bullet was stopped
            m_BotBulletStopped = 1;
        }
    }

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

    // calculate the final pos and dir velocity
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

    // calculate next time where the step sound should be played
    m_StepTime += (elapsedTime * 1000.0f);
    
    // count frames
    while (m_StepTime > m_StepInterval)
    {
        miniPlaySound(m_PlayerStepSoundID);
        m_StepTime = 0.0f;
    }
    
    // player and bot intersected physically?
    if (miniSpheresIntersect(&m_Player, &m_Bot))
    {
        // todo -cFeature -oJean: see what to do with that
    }
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_Vector3       t;
    MINI_Vector3       axis;
    MINI_Vector3       factor;
    MINI_Matrix        translateMatrix;
    MINI_Matrix        rotateMatrixX;
    MINI_Matrix        rotateMatrixY;
    MINI_Matrix        scaleMatrix;
    MINI_Matrix        combinedMatrixLevel1;
    MINI_Matrix        combinedMatrixLevel2;
    MINI_Matrix        modelViewMatrix;
    MINI_LevelDrawInfo drawInfo;
    float              angle;
    GLint              modelviewUniform;
    
    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    
    glDisable(GL_BLEND);
    
    // apply the fade level (change while player is dying)
    glUniform1f(m_FadeFactorSlot, m_FadeLevel);
    
    // apply the red filter (change the scene to red while player is hit)
    glUniform1i(m_RedFilterSlot, m_PlayerHit);
    
    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_TexSamplerSlot, GL_TEXTURE0);
    
    glFrontFace(GL_CCW);
    
    // configure the draw info
    drawInfo.m_pSurfaceVB         =  m_pSurfaceVB;
    drawInfo.m_SurfaceVertexCount =  m_SurfaceVertexCount;
    drawInfo.m_pVertexFormat      = &m_VertexFormat;
    drawInfo.m_pShader            = &m_Shader;
    drawInfo.m_ShaderProgram      =  m_ShaderProgram;
    drawInfo.m_SoilTextureIndex   =  m_SoilTextureIndex;
    drawInfo.m_WallTextureIndex   =  m_WallTextureIndex;
    drawInfo.m_CeilTextureIndex   =  m_CeilTextureIndex;
    drawInfo.m_MapMode            =  0;
    
    // draw the level
    miniDrawLevel(m_pLevel,
                  m_LevelItemWidth,
                  m_LevelItemHeight,
                  m_LevelItemLength,
                  m_MapLength,
                  &m_Player,
                  m_Angle,
                  &drawInfo);
    
    glFrontFace(GL_CW);
    
    // set translation
    t.m_X = m_Bot.m_Pos.m_X - m_Player.m_Pos.m_X;
    t.m_Y = m_Bot.m_Pos.m_Y;
    t.m_Z = m_Bot.m_Pos.m_Z - m_Player.m_Pos.m_Z;
    
    miniGetTranslateMatrix(&t, &translateMatrix);
    
    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    
    // set rotation angle
    angle = -M_PI * 0.5f;
    
    miniGetRotateMatrix(&angle, &axis, &rotateMatrixX);
    
    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;
    
    // set rotation angle
    miniGetRotateMatrix(&m_BotAngle, &axis, &rotateMatrixY);
    
    // set scale factor
    factor.m_X = 0.05f;
    factor.m_Y = 0.05f;
    factor.m_Z = 0.05f;
    
    miniGetScaleMatrix(&factor, &scaleMatrix);
    
    // calculate model view matrix
    miniMatrixMultiply(&scaleMatrix,          &rotateMatrixX,   &combinedMatrixLevel1);
    miniMatrixMultiply(&combinedMatrixLevel1, &rotateMatrixY,   &combinedMatrixLevel2);
    miniMatrixMultiply(&combinedMatrixLevel2, &translateMatrix, &modelViewMatrix);
    
    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);
    
    // bind the model texture
    glBindTexture(GL_TEXTURE_2D, m_ModelTextureIndex);
    
    // set alpha transparency value to draw the bot
    glUniform1f(m_AlphaSlot, m_BotAlphaLevel);
    
    // configure OpenGL to draw transparency (NOTE all opaque objects should be drawn before,
    // because depth test should be disabled to allow alpha blending to work correctly)
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // draw the bot model
    miniDrawMDL(m_pModel, &m_Shader, m_MeshIndex);
    
    // did the player fired?
    if (m_PlayerBullet.m_Fired)
    {
        // set alpha transparency value to draw the bullet
        glUniform1f(m_AlphaSlot, m_BulletAlphaLevel);
        
        glFrontFace(GL_CCW);
        
        // set translation
        t.m_X = m_PlayerBullet.m_Shape.m_Pos.m_X - m_Player.m_Pos.m_X;
        t.m_Y = 0.0f;
        t.m_Z = m_PlayerBullet.m_Shape.m_Pos.m_Z - m_Player.m_Pos.m_Z;
        
        // get the bullet model view matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);
        
        // connect model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);
        
        // bind the model texture
        glBindTexture(GL_TEXTURE_2D, m_BulletTextureIndex);
        
        // draw the bullet
        miniDrawSphere(m_pBulletVertices,
                       m_BulletVertexCount,
                       m_pBulletIndexes,
                       m_BulletIndexCount,
                       &m_VertexFormat,
                       &m_Shader);
    }
    
    // did the bot fired?
    if (m_BotBullet.m_Fired)
    {
        // set alpha transparency value to draw the bullet
        glUniform1f(m_AlphaSlot, m_BulletAlphaLevel);
        
        glFrontFace(GL_CCW);
        
        // set translation
        t.m_X = m_BotBullet.m_Shape.m_Pos.m_X - m_Player.m_Pos.m_X;
        t.m_Y = 0.0f;
        t.m_Z = m_BotBullet.m_Shape.m_Pos.m_Z - m_Player.m_Pos.m_Z;
        
        // get the bullet model view matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);
        
        // connect model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);
        
        // bind the model texture
        glBindTexture(GL_TEXTURE_2D, m_BulletTextureIndex);
        
        // draw the bullet
        miniDrawSphere(m_pBulletVertices,
                       m_BulletVertexCount,
                       m_pBulletIndexes,
                       m_BulletIndexCount,
                       &m_VertexFormat,
                       &m_Shader);
    }
    
    miniEndScene();
}
//----------------------------------------------------------------------------
- (void)OnLongPress :(UIGestureRecognizer*)pSender
{
    const CGPoint touchPos = [pSender locationInView :nil];

    switch (pSender.state)
    {
        case UIGestureRecognizerStateBegan:
            // initialize the position. NOTE inverted because the screen is in landscape mode
            m_TouchOrigin.m_X   = touchPos.y;
            m_TouchOrigin.m_Y   = touchPos.x;
            m_TouchPosition.m_X = touchPos.y;
            m_TouchPosition.m_Y = touchPos.x;
            break;

        case UIGestureRecognizerStateChanged:
            // get the next position. NOTE inverted because the screen is in landscape mode
            m_TouchPosition.m_X = touchPos.y;
            m_TouchPosition.m_Y = touchPos.x;
            break;

        case UIGestureRecognizerStateEnded:
            // player want to fire?
            if (!m_PlayerDied && touchPos.y == m_TouchOrigin.m_X && touchPos.x == m_TouchOrigin.m_Y)
            {
                m_PlayerBullet.m_Shape.m_Pos.m_X = m_Player.m_Pos.m_X;
                m_PlayerBullet.m_Shape.m_Pos.m_Z = m_Player.m_Pos.m_Z;
                m_PlayerBullet.m_Angle           = m_Angle;
                m_PlayerBullet.m_Fired           = 1;
                
                miniPlaySound(m_PlayerFireSoundID);
            }

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
