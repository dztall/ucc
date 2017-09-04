/*****************************************************************************
 * ==> GameViewController ---------------------------------------------------*
 *****************************************************************************
 * Description : A ray picking demo with aligned-axis bounding box. Tap      *
 *               anywhere on the sphere to select a polygon, swipe to the    *
 *               left or right to rotate the sphere                          *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#import "GameViewController.h"

// openGL
#import <OpenGLES/ES2/glext.h>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
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
    MINI_AABBNode*    m_pAABBRoot;
    MINI_Polygon*     m_pCollidePolygons;
    unsigned int      m_CollidePolygonsCount;
    float             m_Radius;
    float             m_RayX;
    float             m_RayY;
    float             m_Angle;
    float             m_RotationSpeed;
    float             m_Time;
    float             m_Interval;
    unsigned int      m_FPS;
    float             m_PolygonArray[21];
    MINI_VertexFormat m_VertexFormat;
    MINI_Matrix       m_ProjectionMatrix;
    MINI_Matrix       m_ViewMatrix;
    CGRect            m_ScreenRect;
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
* Draws the scene
*/
- (void) DrawScene;

/**
* Called when screen is tapped
*@param pRecognizer - recognizer that raised the event
*/
- (void) OnScreenTapped :(UITapGestureRecognizer*)pRecognizer;

/**
* Called when screen is swipped on the left
*@param pRecognizer - recognizer that raised the event
*/
- (void) OnScreenSwippedLeft :(UISwipeGestureRecognizer*)pRecognizer;

/**
* Called when screen is swipped on the right
*@param pRecognizer - recognizer that raised the event
*/
- (void) OnScreenSwippedRight :(UISwipeGestureRecognizer*)pRecognizer;

@end
//----------------------------------------------------------------------------
@implementation GameViewController
//----------------------------------------------------------------------------
- (void) viewDidLoad
{
    [super viewDidLoad];

    m_ShaderProgram        = 0;
    m_pVertexBuffer        = 0;
    m_VertexCount          = 0;
    m_pIndexes             = 0;
    m_IndexCount           = 0;
    m_pAABBRoot            = 0;
    m_pCollidePolygons     = 0;
    m_CollidePolygonsCount = 0;
    m_Radius               = 1.0f;
    m_RayX                 = 2.0f;
    m_RayY                 = 2.0f;
    m_Angle                = 0.0f;
    m_RotationSpeed        = 0.0f;
    m_Time                 = 0.0f;
    m_Interval             = 0.0f;
    m_FPS                  = 15;
    m_PreviousTime         = CACurrentMediaTime();

    // add a tap gesture to the view
    UITapGestureRecognizer* pTapGesture =
        [[UITapGestureRecognizer alloc]initWithTarget:self
                                               action:@selector(OnScreenTapped:)];
    [self.view addGestureRecognizer:pTapGesture];

    // add a swipe gesture (to the left) to the view
    UISwipeGestureRecognizer* pSwipeGestureLeft =
    [[UISwipeGestureRecognizer alloc]initWithTarget:self
                                             action:@selector(OnScreenSwippedLeft:)];
    pSwipeGestureLeft.direction = UISwipeGestureRecognizerDirectionLeft;
    [self.view addGestureRecognizer:pSwipeGestureLeft];

    // add a swipe gesture (to the right) to the view
    UISwipeGestureRecognizer* pSwipeGestureRight =
    [[UISwipeGestureRecognizer alloc]initWithTarget:self
                                             action:@selector(OnScreenSwippedRight:)];
    pSwipeGestureRight.direction = UISwipeGestureRecognizerDirectionRight;
    [self.view addGestureRecognizer:pSwipeGestureRight];

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
- (void) CreateViewport :(float)w :(float)h
{
    // calculate matrix items
    const float zNear  =  1.0f;
    const float zFar   =  20.0f;
    const float aspect =  w / h;
    const float left   = -aspect;
    const float right  =  aspect;
    const float top    =  1.0f;
    const float bottom = -1.0f;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    miniGetFrustum(&left, &right, &bottom, &top, &zNear, &zFar, &m_ProjectionMatrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(m_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &m_ProjectionMatrix.m_Table[0][0]);
}
//----------------------------------------------------------------------------
- (void)InitScene
{
    unsigned i;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram, "qr_vPosition");
    m_Shader.m_ColorSlot  = glGetAttribLocation(m_ShaderProgram, "qr_vColor");

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
    m_ScreenRect = [[UIScreen mainScreen]bounds];

    // create the viewport
    [self CreateViewport :m_ScreenRect.size.width :m_ScreenRect.size.height];

    miniGetIdentity(&m_ViewMatrix);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&m_Radius,
                     10,
                     12,
                     0x0000FFFF,
                     &m_VertexFormat,
                     &m_pVertexBuffer,
                     &m_VertexCount,
                     &m_pIndexes,
                     &m_IndexCount);

    // iterate through vertex indexes
    for (i = 0; i < m_IndexCount; ++i)
        // get collide polygons
        miniGetPolygonsFromVB(&m_pVertexBuffer[m_pIndexes[i].m_Start],
                              m_pIndexes[i].m_Length,
                              1,
                              m_VertexFormat.m_Stride,
                              &m_pCollidePolygons,
                              &m_CollidePolygonsCount);

    // create Aligned-Axis bounding box tree
    m_pAABBRoot = (MINI_AABBNode*)malloc(sizeof(MINI_AABBNode));
    miniPopulateTree(m_pAABBRoot, m_pCollidePolygons, m_CollidePolygonsCount);

    // fill polygon array colors
    m_PolygonArray[3]  = 1.0f;
    m_PolygonArray[4]  = 0.0f;
    m_PolygonArray[5]  = 0.0f;
    m_PolygonArray[6]  = 1.0f;
    m_PolygonArray[10] = 0.8f;
    m_PolygonArray[11] = 0.0f;
    m_PolygonArray[12] = 0.2f;
    m_PolygonArray[13] = 1.0f;
    m_PolygonArray[17] = 1.0f;
    m_PolygonArray[18] = 0.12f;
    m_PolygonArray[19] = 0.2f;
    m_PolygonArray[20] = 1.0f;

    // calculate frame interval
    m_Interval = 1000.0f / m_FPS;
}
//----------------------------------------------------------------------------
- (void) DeleteScene
{
    // delete aabb tree
    if (m_pAABBRoot)
        miniReleaseTree(m_pAABBRoot);

    m_pAABBRoot = 0;

    // delete collide polygons
    if (m_pCollidePolygons)
        miniReleasePolygons(m_pCollidePolygons);

    m_pCollidePolygons = 0;

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
//------------------------------------------------------------------------------
- (void) UpdateScene :(float)elapsedTime
{
    unsigned int frameCount = 0;

    // calculate next time
    m_Time += (elapsedTime * 1000.0f);

    // count frames to skip
    while (m_Time > m_Interval)
    {
        m_Time -= m_Interval;
        ++frameCount;
    }

    // calculate next rotation angle
    m_Angle += (m_RotationSpeed * frameCount);

    // is rotating angle out of bounds?
    while (m_Angle >= 6.28f)
        m_Angle -= 6.28f;
}
//----------------------------------------------------------------------------
- (void) DrawScene;
{
    MINI_Polygon* pPolygonList;
    MINI_Polygon* pPolygonsToDraw;
    unsigned      polygonsCount;
    unsigned      polygonsToDrawCount;
    unsigned      i;
    unsigned      j;
    float         determinant;
    float         xAngle;
    MINI_Vector3  t;
    MINI_Vector3  r;
    MINI_Vector3  rayPos;
    MINI_Vector3  rayDir;
    MINI_Matrix   translateMatrix;
    MINI_Matrix   xRotateMatrix;
    MINI_Matrix   yRotateMatrix;
    MINI_Matrix   rotateMatrix;
    MINI_Matrix   modelMatrix;
    MINI_Matrix   invModelMatrix;
    MINI_Ray      ray;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -2.1f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 90 degrees
    xAngle = 1.57075f;

    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    miniGetRotateMatrix(&m_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&xRotateMatrix, &yRotateMatrix,   &rotateMatrix);
    miniMatrixMultiply(&rotateMatrix,  &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelUniform = glGetUniformLocation(m_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // set ray in 3d world
    rayPos.m_X =  m_RayX;
    rayPos.m_Y =  m_RayY;
    rayPos.m_Z =  0.0f;
    rayDir.m_X =  m_RayX;
    rayDir.m_Y =  m_RayY;
    rayDir.m_Z = -1.0f;

    miniNormalize(&rayDir, &rayDir);

    // put the ray in the 3d world coordinates
    miniUnproject(&m_ProjectionMatrix,
                  &m_ViewMatrix,
                  &rayPos,
                  &rayDir);

    // put the ray in the model coordinates
    miniInverse(&modelMatrix, &invModelMatrix, &determinant);
    miniApplyMatrixToVector(&invModelMatrix, &rayPos, &ray.m_Pos);
    miniApplyMatrixToNormal(&invModelMatrix, &rayDir, &ray.m_Dir);
    miniNormalize(&ray.m_Dir, &ray.m_Dir);

    // calculate inverted ray dir
    ray.m_InvDir.m_X = ray.m_Dir.m_X ? (1.0f / ray.m_Dir.m_X) : INFINITY;
    ray.m_InvDir.m_Y = ray.m_Dir.m_Y ? (1.0f / ray.m_Dir.m_Y) : INFINITY;
    ray.m_InvDir.m_Z = ray.m_Dir.m_Z ? (1.0f / ray.m_Dir.m_Z) : INFINITY;

    pPolygonList  = 0;
    polygonsCount = 0;

    // resolve aligned-axis bounding box tree
    miniResolveTree(&ray, m_pAABBRoot, &pPolygonList, &polygonsCount);

    pPolygonsToDraw     = 0;
    polygonsToDrawCount = 0;

    // iterate through polygons to check
    for (i = 0; i < polygonsCount; ++i)
        // is polygon intersecting ray?
        if (miniRayPolygonIntersect(&ray, &pPolygonList[i]))
        {
            // add new polygon to draw list
            if (!polygonsToDrawCount)
            {
                pPolygonsToDraw     = (MINI_Polygon*)malloc(sizeof(MINI_Polygon));
                polygonsToDrawCount = 1;
            }
            else
            {
                ++polygonsToDrawCount;
                pPolygonsToDraw = (MINI_Polygon*)realloc(pPolygonsToDraw,
                                                         polygonsToDrawCount * sizeof(MINI_Polygon));
            }

            // copy polygon
            for (j = 0; j < 3; ++j)
                miniCopy(&pPolygonList[i].m_v[j], &pPolygonsToDraw[polygonsToDrawCount - 1].m_v[j]);
        }

    // delete found polygons (no more needed from now)
    if (polygonsCount)
        free(pPolygonList);

    // draw the sphere
    miniDrawSphere(m_pVertexBuffer,
                   m_VertexCount,
                   m_pIndexes,
                   m_IndexCount,
                   &m_VertexFormat,
                   &m_Shader);

    // enable position and color slots
    glEnableVertexAttribArray(m_Shader.m_VertexSlot);
    glEnableVertexAttribArray(m_Shader.m_ColorSlot);

    // found collide polygons to draw?
    for (i = 0; i < polygonsToDrawCount; ++i)
    {
        // set vertex 1 in vertex buffer
        m_PolygonArray[0]  = pPolygonsToDraw[i].m_v[0].m_X;
        m_PolygonArray[1]  = pPolygonsToDraw[i].m_v[0].m_Y;
        m_PolygonArray[2]  = pPolygonsToDraw[i].m_v[0].m_Z;

        // set vertex 2 in vertex buffer
        m_PolygonArray[7]  = pPolygonsToDraw[i].m_v[1].m_X;
        m_PolygonArray[8]  = pPolygonsToDraw[i].m_v[1].m_Y;
        m_PolygonArray[9]  = pPolygonsToDraw[i].m_v[1].m_Z;

        // set vertex 3 in vertex buffer
        m_PolygonArray[14] = pPolygonsToDraw[i].m_v[2].m_X;
        m_PolygonArray[15] = pPolygonsToDraw[i].m_v[2].m_Y;
        m_PolygonArray[16] = pPolygonsToDraw[i].m_v[2].m_Z;

        // draw the polygon
        miniDrawBuffer(m_PolygonArray,
                       3,
                       E_Triangles,
                       &m_VertexFormat,
                       &m_Shader);
    }

    if (polygonsToDrawCount)
        free(pPolygonsToDraw);

    // disconnect slots from shader
    glDisableVertexAttribArray(m_Shader.m_VertexSlot);
    glDisableVertexAttribArray(m_Shader.m_ColorSlot);

    miniEndScene();
}
//----------------------------------------------------------------------------
- (void) OnScreenTapped :(UITapGestureRecognizer*)pRecognizer
{
    // get the view tap location
    const CGPoint location = [pRecognizer locationInView:self.view];

    // convert screen coordinates to ray world coordinate and get ray position
    m_RayX = -1.0f + ((location.x * 2.0f) / m_ScreenRect.size.width);
    m_RayY =  1.0f - ((location.y * 2.0f) / m_ScreenRect.size.height);
}
//----------------------------------------------------------------------------
- (void) OnScreenSwippedLeft :(UISwipeGestureRecognizer*)pRecognizer
{
    // decrease rotation speed
    m_RotationSpeed -= 0.05f;
}
//----------------------------------------------------------------------------
- (void) OnScreenSwippedRight :(UISwipeGestureRecognizer*)pRecognizer
{
    // increase rotation speed
    m_RotationSpeed += 0.05f;
}
//----------------------------------------------------------------------------
@end
//----------------------------------------------------------------------------
