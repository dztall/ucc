/****************************************************************************
 * ==> Tic-tac-toe game ----------------------------------------------------*
 ****************************************************************************
 * Description : A complete tic-tac-toe game using a mimimax algorithm      *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2019, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

// supported platforms check. NOTE iOS only, but may works on other platforms
#if !defined(_OS_IOS_) && !defined(_OS_ANDROID_) && !defined(_OS_WINDOWS_)
    #error "Not supported platform!"
#endif

#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// opengl
#include <gles2.h>
#include <gles2ext.h>

// compactStar engine
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"

// texture files
#define PLAYFIELD_TEXTURE "Resources/playfield.bmp"
#define CROSS_TEXTURE     "Resources/cross.bmp"
#define ROUND_TEXTURE     "Resources/round.bmp"
#define YOUWON_TEXTURE    "Resources/youwon.bmp"
#define YOULOOSE_TEXTURE  "Resources/youloose.bmp"
#define EQUALITY_TEXTURE  "Resources/equality.bmp"

// player pawn
#define M_Player_Pawn   CSR_PA_Cross
#define M_Computer_Pawn CSR_PA_Round

// configure which player will begin the game
#define M_Starting_Player CSR_PL_Computer

// libraries
#include <ccr.h>

//----------------------------------------------------------------------------
const char g_VSTextured[] =
    "precision mediump float;"
    "attribute vec3 csr_aVertices;"
    "attribute vec4 csr_aColor;"
    "attribute vec2 csr_aTexCoord;"
    "uniform   mat4 csr_uProjection;"
    "uniform   mat4 csr_uView;"
    "uniform   mat4 csr_uModel;"
    "varying   vec4 csr_vColor;"
    "varying   vec2 csr_vTexCoord;"
    "void main(void)"
    "{"
    "    csr_vColor    = csr_aColor;"
    "    csr_vTexCoord = csr_aTexCoord;"
    "    gl_Position   = csr_uProjection * csr_uView * csr_uModel * vec4(csr_aVertices, 1.0);"
    "}";
//----------------------------------------------------------------------------
const char g_FSTextured[] =
    "precision mediump float;"
    "uniform sampler2D csr_sColorMap;"
    "varying lowp vec4 csr_vColor;"
    "varying      vec2 csr_vTexCoord;"
    "void main(void)"
    "{"
    "    gl_FragColor = csr_vColor * texture2D(csr_sColorMap, csr_vTexCoord);"
    "}";
//------------------------------------------------------------------------------
typedef enum
{
    CSR_PL_Computer      = -1,
    CSR_PL_None          =  0,
    CSR_PL_Player        =  1,
    CSR_PL_SkipRendering =  2
} CSR_EPlayer;
//------------------------------------------------------------------------------
typedef enum
{
    CSR_PA_None,
    CSR_PA_Round,
    CSR_PA_Cross
} CSR_EPawn;
//------------------------------------------------------------------------------
/**
* Playfield cell
*/
typedef struct
{
    unsigned    m_X;
    unsigned    m_Y;
    CSR_EPawn   m_Pawn;
    CSR_Vector2 m_Pos;
    CSR_Rect    m_TouchArea;
    CSR_Matrix4 m_Matrix_Cross;
    CSR_Matrix4 m_Matrix_Round;
} CSR_Cell;
//------------------------------------------------------------------------------
CSR_OpenGLShader* g_pShader         = 0;
CSR_Scene*        g_pScene          = 0;
CSR_Mesh*         g_pPlayfield      = 0;
CSR_Mesh*         g_pCross          = 0;
CSR_Mesh*         g_pRound          = 0;
CSR_Mesh*         g_pYouWonMsg      = 0;
CSR_Mesh*         g_pYouLooseMsg    = 0;
CSR_Mesh*         g_pEqualityMsg    = 0;
CSR_EPlayer       g_IsPlaying       = M_Starting_Player;
int               g_UseOpening      = 1; // recommended if starting player is computer, because opening may be very slow otherwise
int               g_GameOver        = 0;
float             g_ScreenWidth     = 0.0f;
float             g_ScreenHeight    = 0.0f;
CSR_Matrix4*      g_pPawnMatrix     = 0;
CSR_Matrix4       g_PlayfieldMatrix;
CSR_Matrix4       g_YouWonMatrix;
CSR_Matrix4       g_YouLooseMatrix;
CSR_Matrix4       g_EqualityMatrix;
CSR_Cell          g_Cells[3][3];
CSR_SceneContext  g_SceneContext;
CSR_OpenGLID      g_ID[6];
//------------------------------------------------------------------------------
int IsOnLeftDiagonal(const CSR_Cell* pCell)
{
    if (pCell->m_X == pCell->m_Y)
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
int IsOnRightDiagonal(const CSR_Cell* pCell)
{
    if (pCell->m_X + pCell->m_Y == 2)
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
int IsOnCompleteLine(const CSR_Cell* pCell, CSR_EPawn pawn)
{
    unsigned i;
    unsigned j;
    unsigned horiz     = 0;
    unsigned vert      = 0;
    unsigned lDiagonal = 0;
    unsigned rDiagonal = 0;

    // iterate through cells
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
        {
            // get cell to check
            CSR_Cell* pCellToCheck = &g_Cells[i][j];

            // check if cell is on same line as reference cell and if cell contains pawn
            if (pCellToCheck->m_X == pCell->m_X && pCellToCheck->m_Pawn == pawn)
                ++horiz;

            // check if cell is on same column as reference cell and if cell contains pawn
            if (pCellToCheck->m_Y == pCell->m_Y && pCellToCheck->m_Pawn == pawn)
                ++vert;

            // check if cell and reference cell are on left diagonal and if cell contains pawn
            if (IsOnLeftDiagonal(pCell) && IsOnLeftDiagonal(pCellToCheck) && pCellToCheck->m_Pawn == pawn)
                ++lDiagonal;

            // check if cell and reference cell are on right diagonal and if cell contains pawn
            if (IsOnRightDiagonal(pCell) && IsOnRightDiagonal(pCellToCheck) && pCellToCheck->m_Pawn == pawn)
                ++rDiagonal;
        }

    // a line is complete if all cells of line, column or diagonal contains same pawn
    if (horiz == 3 || vert == 3 || rDiagonal == 3 || lDiagonal == 3)
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
int IsGameOver()
{
    unsigned i;
    unsigned j;
    unsigned occupiedCell = 0;

    // iterate through cells
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
        {
            // get cell
            CSR_Cell* pCell = &g_Cells[i][j];

            // is cell occupied by pawn?
            if (pCell->m_Pawn != CSR_PA_None)
                ++occupiedCell;
        }

    // are all cells occupied?
    return (occupiedCell >= 9);
}
//------------------------------------------------------------------------------
CSR_EPlayer HasWin()
{
    unsigned i;
    unsigned j;

    // iterate through cells
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
        {
            // get cell
            CSR_Cell* pCell = &g_Cells[i][j];

            // check if player 1 has win (i.e. if he has completed a line)
            if (IsOnCompleteLine(pCell, M_Player_Pawn))
                return CSR_PL_Player;

            // check if player 2 has win (i.e. if he has completed a line)
            if (IsOnCompleteLine(pCell, M_Computer_Pawn))
                return CSR_PL_Computer;
        }

    return CSR_PL_None;
}
//---------------------------------------------------------------------------
int MiniMax(CSR_EPlayer player, CSR_Cell** pBestCell, int deep)
{
    unsigned i;
    unsigned j;

    // calculate min and max sum
    int sum = (1 << 20);
    int max = -sum;
    int min =  sum;

    // any player has win?
    CSR_EPlayer winner = HasWin();

    // is game over?
    if (winner == CSR_PL_Player)
        return CSR_PL_Player;
    else
    if (winner == CSR_PL_Computer)
        return CSR_PL_Computer;
    else
    if (IsGameOver())
        return 0;

    // iterate through cells
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
        {
            CSR_Cell* pCell = &g_Cells[i][j];

            // is cell already occupied?
            if (pCell->m_Pawn != CSR_PA_None)
                continue;

            // search for player to check
            switch (player)
            {
                case CSR_PL_Player:
                {
                    // simulate player 1 move
                    pCell->m_Pawn = M_Player_Pawn;

                    // check next opposite player move
                    int result = MiniMax(CSR_PL_Computer, pBestCell, deep + 1);

                    // revert cell to original value
                    pCell->m_Pawn = CSR_PA_None;

                    // do update max?
                    if (result > max)
                    {
                        max = result;

                        // first level?
                        if (!deep)
                            // get best cell
                            *pBestCell = pCell;
                    }

                    continue;
                }

                case CSR_PL_Computer:
                {
                    // simulate player 2 move
                    pCell->m_Pawn = M_Computer_Pawn;

                    // check next opposite player move
                    int result = MiniMax(CSR_PL_Player, pBestCell, deep + 1);

                    // revert cell to original value
                    pCell->m_Pawn = CSR_PA_None;

                    // do update min?
                    if (result < min)
                    {
                        min = result;

                        // first level?
                        if (!deep)
                            // get best cell
                            *pBestCell = pCell;
                    }

                    continue;
                }

                default:
                    continue;
            }
        }

    // is player?
    if (player == CSR_PL_Player)
        return max;

    return min;
}
//---------------------------------------------------------------------------
void ClearPlayfield()
{
    unsigned i;
    unsigned j;

    // reset current player
    g_IsPlaying = M_Starting_Player;

    // reset game over
    g_GameOver = 0;

    // hide messages
    g_YouWonMatrix.m_Table[3][0]   = 9999.0f;
    g_YouWonMatrix.m_Table[3][1]   = 9999.0f;
    g_YouLooseMatrix.m_Table[3][0] = 9999.0f;
    g_YouLooseMatrix.m_Table[3][1] = 9999.0f;
    g_EqualityMatrix.m_Table[3][0] = 9999.0f;
    g_EqualityMatrix.m_Table[3][1] = 9999.0f;

    // iterate through cells
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
        {
            // clear cells
            g_Cells[i][j].m_Pawn = CSR_PA_None;

            g_Cells[i][j].m_Matrix_Cross.m_Table[3][0] = 9999.0f;
            g_Cells[i][j].m_Matrix_Cross.m_Table[3][1] = 9999.0f;

            g_Cells[i][j].m_Matrix_Round.m_Table[3][0] = 9999.0f;
            g_Cells[i][j].m_Matrix_Round.m_Table[3][1] = 9999.0f;
        }
}
//---------------------------------------------------------------------------
int SetPawn(CSR_EPawn pawn, unsigned x, unsigned y)
{
    // indices out of bounds?
    if (x > 2 || y > 2)
        return 0;

    // slot already occupied?
    if (g_Cells[x][y].m_Pawn != CSR_PA_None)
        return 0;

    // set pawn type
    g_Cells[x][y].m_Pawn = pawn;

    // set pawn on playfield
    switch (pawn)
    {
        case CSR_PA_Cross:
            g_Cells[x][y].m_Matrix_Cross.m_Table[3][0] = g_Cells[x][y].m_Pos.m_X;
            g_Cells[x][y].m_Matrix_Cross.m_Table[3][1] = g_Cells[x][y].m_Pos.m_Y;
            break;

        case CSR_PA_Round:
            g_Cells[x][y].m_Matrix_Round.m_Table[3][0] = g_Cells[x][y].m_Pos.m_X;
            g_Cells[x][y].m_Matrix_Round.m_Table[3][1] = g_Cells[x][y].m_Pos.m_Y;
            break;
    }

    return 1;
}
//---------------------------------------------------------------------------
void ApplySkin(size_t index, const CSR_Skin* pSkin)
{
    g_ID[index].m_pKey     = (void*)(&pSkin->m_Texture);
    g_ID[index].m_ID       = csrOpenGLTextureFromPixelBuffer(pSkin->m_Texture.m_pBuffer);
    g_ID[index].m_UseCount = 1;
}
//---------------------------------------------------------------------------
void OnSceneBegin(const CSR_Scene* pScene, const CSR_SceneContext* pContext)
{
    csrDrawBegin(&pScene->m_Color);
}
//---------------------------------------------------------------------------
void OnSceneEnd(const CSR_Scene* pScene, const CSR_SceneContext* pContext)
{
    csrDrawEnd();
}
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    return g_pShader;
}
//---------------------------------------------------------------------------
void* OnGetID(const void* pKey)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < 6; ++i)
        // found the texture to get?
        if (pKey == g_ID[i].m_pKey)
            return &g_ID[i];

    return 0;
}
//---------------------------------------------------------------------------
void OnDeleteTexture(const CSR_Texture* pTexture)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < 6; ++i)
        // found the texture to delete?
        if (pTexture == g_ID[i].m_pKey)
        {
            // unuse the texture
            if (g_ID[i].m_UseCount)
                --g_ID[i].m_UseCount;

            // is texture no longer used?
            if (g_ID[i].m_UseCount)
                return;

            // delete the texture from the GPU
            if (g_ID[i].m_ID != M_CSR_Error_Code)
            {
                glDeleteTextures(1, (GLuint*)(&g_ID[i].m_ID));
                g_ID[i].m_ID = M_CSR_Error_Code;
            }

            return;
        }
}
//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 0.01f;
    const float zFar   = 100.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    csrMat4Perspective(fov, aspect, zNear, zFar, &g_pScene->m_ProjectionMatrix);

    csrShaderEnable(g_pShader);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_VertexFormat  vf;
    CSR_VertexCulling vc;
    CSR_Material      material;
    size_t            i;
    size_t            j;
    float             step;

    // create the default scene
    g_pScene = csrSceneCreate();

    // create the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnSceneBegin = OnSceneBegin;
    g_SceneContext.m_fOnSceneEnd   = OnSceneEnd;
    g_SceneContext.m_fOnGetShader  = OnGetShader;
    g_SceneContext.m_fOnGetID      = OnGetID;

    // configure the scene color background
    g_pScene->m_Color.m_R = 1.0f;
    g_pScene->m_Color.m_G = 1.0f;
    g_pScene->m_Color.m_B = 1.0f;
    g_pScene->m_Color.m_A = 1.0f;

    // initialize the matrices
    csrMat4Identity(&g_pScene->m_ProjectionMatrix);
    csrMat4Identity(&g_pScene->m_ViewMatrix);
    csrMat4Identity(&g_PlayfieldMatrix);
    csrMat4Identity(&g_YouWonMatrix);
    csrMat4Identity(&g_YouLooseMatrix);
    csrMat4Identity(&g_EqualityMatrix);

    // get the screen width
    g_ScreenWidth = view_w;

    // compile, link and use shader
    g_pShader = csrOpenGLShaderLoadFromStr(&g_VSTextured[0],
                                            sizeof(g_VSTextured),
                                           &g_FSTextured[0],
                                            sizeof(g_FSTextured),
                                            0,
                                            0);
    csrShaderEnable(g_pShader);

    // configure the shader slots
    g_pShader->m_VertexSlot   = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_ColorSlot    = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aColor");
    g_pShader->m_TexCoordSlot = glGetAttribLocation(g_pShader->m_ProgramID, "csr_aTexCoord");
    g_pShader->m_TextureSlot  = glGetAttribLocation(g_pShader->m_ProgramID, "csr_sTexture");

    CreateViewport(view_w, view_h);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // configure the vertex format
    vf.m_HasNormal         = 0;
    vf.m_HasTexCoords      = 1;
    vf.m_HasPerVertexColor = 1;

    vc.m_Type = CSR_CT_Back;
    vc.m_Face = CSR_CF_CCW;

    // create the surfaces
    g_pPlayfield   = csrShapeCreateSurface(0.8f, 0.8f, &vf, &vc, 0, 0);
    g_pCross       = csrShapeCreateSurface(0.2f, 0.2f, &vf, &vc, 0, 0);
    g_pRound       = csrShapeCreateSurface(0.2f, 0.2f, &vf, &vc, 0, 0);
    g_pYouWonMsg   = csrShapeCreateSurface(0.2f, 0.1f, &vf, &vc, 0, 0);
    g_pYouLooseMsg = csrShapeCreateSurface(0.2f, 0.1f, &vf, &vc, 0, 0);
    g_pEqualityMsg = csrShapeCreateSurface(0.2f, 0.1f, &vf, &vc, 0, 0);

    // add playfield to the scene
    csrSceneAddMesh(g_pScene, g_pPlayfield, 0, 0);
    csrSceneAddModelMatrix(g_pScene, g_pPlayfield, &g_PlayfieldMatrix);

    // add pawns to the scene
    csrSceneAddMesh(g_pScene, g_pCross, 0, 0);
    csrSceneAddMesh(g_pScene, g_pRound, 0, 0);

    // add messages to the scene
    csrSceneAddMesh(g_pScene, g_pYouWonMsg,   0, 0);
    csrSceneAddMesh(g_pScene, g_pYouLooseMsg, 0, 0);
    csrSceneAddMesh(g_pScene, g_pEqualityMsg, 0, 0);
    csrSceneAddModelMatrix(g_pScene, g_pYouWonMsg,   &g_YouWonMatrix);
    csrSceneAddModelMatrix(g_pScene, g_pYouLooseMsg, &g_YouLooseMatrix);
    csrSceneAddModelMatrix(g_pScene, g_pEqualityMsg, &g_EqualityMatrix);

    step = 0.28f;

    // calculate and keep the possible pawn positions on the playfield grid
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
        {
            g_Cells[i][j].m_X = i;
            g_Cells[i][j].m_Y = j;

            g_Cells[i][j].m_Pos.m_X = step - (i * step);
            g_Cells[i][j].m_Pos.m_Y = step - (j * step);

            csrMat4Identity(&g_Cells[i][j].m_Matrix_Cross);
            csrMat4Identity(&g_Cells[i][j].m_Matrix_Round);

            csrSceneAddModelMatrix(g_pScene, g_pCross, &g_Cells[i][j].m_Matrix_Cross);
            csrSceneAddModelMatrix(g_pScene, g_pRound, &g_Cells[i][j].m_Matrix_Round);
        }

    // load the playfield texture
    g_pPlayfield->m_Skin.m_Texture.m_pBuffer = csrPixelBufferFromBitmapFile(PLAYFIELD_TEXTURE);
    ApplySkin(0, &g_pPlayfield->m_Skin);
    csrPixelBufferRelease(g_pPlayfield->m_Skin.m_Texture.m_pBuffer);
    g_pPlayfield->m_Skin.m_Texture.m_pBuffer = 0;

    // load the cross pawn texture
    g_pCross->m_Skin.m_Texture.m_pBuffer = csrPixelBufferFromBitmapFile(CROSS_TEXTURE);
    ApplySkin(1, &g_pCross->m_Skin);
    csrPixelBufferRelease(g_pCross->m_Skin.m_Texture.m_pBuffer);
    g_pCross->m_Skin.m_Texture.m_pBuffer = 0;

    // load the round pawn texture
    g_pRound->m_Skin.m_Texture.m_pBuffer = csrPixelBufferFromBitmapFile(ROUND_TEXTURE);
    ApplySkin(2, &g_pRound->m_Skin);
    csrPixelBufferRelease(g_pRound->m_Skin.m_Texture.m_pBuffer);
    g_pRound->m_Skin.m_Texture.m_pBuffer = 0;

    // load the "you loose" texture
    g_pYouLooseMsg->m_Skin.m_Texture.m_pBuffer = csrPixelBufferFromBitmapFile(YOULOOSE_TEXTURE);
    ApplySkin(3, &g_pYouLooseMsg->m_Skin);
    csrPixelBufferRelease(g_pYouLooseMsg->m_Skin.m_Texture.m_pBuffer);
    g_pYouLooseMsg->m_Skin.m_Texture.m_pBuffer = 0;

    // load the "you won" texture
    g_pYouWonMsg->m_Skin.m_Texture.m_pBuffer = csrPixelBufferFromBitmapFile(YOUWON_TEXTURE);
    ApplySkin(4, &g_pYouWonMsg->m_Skin);
    csrPixelBufferRelease(g_pYouWonMsg->m_Skin.m_Texture.m_pBuffer);
    g_pYouWonMsg->m_Skin.m_Texture.m_pBuffer = 0;

    // load the "equality!" texture
    g_pEqualityMsg->m_Skin.m_Texture.m_pBuffer = csrPixelBufferFromBitmapFile(EQUALITY_TEXTURE);
    ApplySkin(5, &g_pEqualityMsg->m_Skin);
    csrPixelBufferRelease(g_pEqualityMsg->m_Skin.m_Texture.m_pBuffer);
    g_pEqualityMsg->m_Skin.m_Texture.m_pBuffer = 0;

    // clear the playfield
    ClearPlayfield();
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // release the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);

    // delete shader program
    csrOpenGLShaderRelease(g_pShader);
    g_pShader = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    int   i;
    int   j;
    float startX;
    float startY;
    float step;
    float length;

    // get the screen width
    g_ScreenWidth  = view_w;
    g_ScreenHeight = view_h;

    glViewport(0, 0, view_w, view_h);
    CreateViewport(view_w, view_h);

    csrMathMin(g_ScreenWidth, g_ScreenHeight, &length);

    step = length / 3.0f;

    startX = (g_ScreenWidth  / 2.0f) - (length / 2.0f);
    startY = (g_ScreenHeight / 2.0f) - (length / 2.0f);

    // calculate and keep the playfield touch areas
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
        {
            g_Cells[i][j].m_TouchArea.m_Min.m_X = startX + (i * step);
            g_Cells[i][j].m_TouchArea.m_Min.m_Y = startY + (j * step);
            g_Cells[i][j].m_TouchArea.m_Max.m_X = g_Cells[i][j].m_TouchArea.m_Min.m_X + step;
            g_Cells[i][j].m_TouchArea.m_Max.m_Y = g_Cells[i][j].m_TouchArea.m_Min.m_Y + step;
        }
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // create a point of view from an arcball
    CSR_ArcBall arcball;
    arcball.m_AngleX = 0.0f;
    arcball.m_AngleY = M_PI;
    arcball.m_Radius = 1.0f;
    csrSceneArcBallToMatrix(&arcball, &g_pScene->m_ViewMatrix);

    // do skip for the next rendering?
    if (g_IsPlaying == CSR_PL_SkipRendering)
    {
        g_IsPlaying = CSR_PL_Computer;
        return;
    }

    // game over?
    if (g_GameOver)
        // search which player won the game and show matching message
        switch (HasWin())
        {
            case CSR_PL_Player:
                g_YouWonMatrix.m_Table[3][0] = 0.7f;
                g_YouWonMatrix.m_Table[3][1] = 0.3f;
                return;

            case CSR_PL_Computer:
                g_YouLooseMatrix.m_Table[3][0] = 0.7f;
                g_YouLooseMatrix.m_Table[3][1] = 0.3f;
                return;

            default:
                g_EqualityMatrix.m_Table[3][0] = 0.7f;
                g_EqualityMatrix.m_Table[3][1] = 0.3f;
                return;
        }

    // is computer playing?
    if (g_IsPlaying == CSR_PL_Computer)
    {
        unsigned i;
        unsigned j;
        unsigned count = 0;

        // use random opening?
        if (g_UseOpening)
            // iterate through cells and count occupied ones
            for (j = 0; j < 3; ++j)
                for (i = 0; i < 3; ++i)
                {
                    CSR_Cell* pCell = &g_Cells[i][j];

                    // is cell already occupied?
                    if (pCell->m_Pawn == CSR_PA_None)
                        continue;

                    ++count;
                }

        // put the next pawn on grid
        if (g_UseOpening && count < 2)
            while (!SetPawn(M_Computer_Pawn, rand() % 3, rand() % 3));
        else
        {
            CSR_Cell* pBestCell = 0;

            // search for best cell
            MiniMax(CSR_PL_Computer, &pBestCell, 0);

            if (pBestCell)
                SetPawn(M_Computer_Pawn, pBestCell->m_X, pBestCell->m_Y);
        }

        // check if game ended
        g_GameOver  = IsGameOver() || (HasWin() != CSR_PL_None);

        // since now player is playing
        g_IsPlaying = CSR_PL_Player;
    }
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    // draw the scene
    csrSceneDraw(g_pScene, &g_SceneContext);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
    int i;
    int j;
    CSR_Vector2 touchPos;

    // game over?
    if (g_GameOver)
    {
        // clear and restart
        ClearPlayfield();
        return;
    }

    // isn't the player turn?
    if (g_IsPlaying != CSR_PL_Player)
        return;

    touchPos.m_X = x;
    touchPos.m_Y = y;

    // iterate through playfield areas and check if player tapped one
    for (j = 0; j < 3; ++j)
        for (i = 0; i < 3; ++i)
            if (csrInsideRect(&touchPos, &g_Cells[i][j].m_TouchArea))
            {
                if (!SetPawn(M_Player_Pawn, i, j))
                    return;

                // check if game ended
                g_GameOver = IsGameOver() || (HasWin() != CSR_PL_None);

                // since now computer is playing
                g_IsPlaying = CSR_PL_SkipRendering;
                return;
            }

    // nothing tapped, restart the game
    ClearPlayfield();
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{}
//------------------------------------------------------------------------------
#if __CCR__ > 2 || (__CCR__ == 2 && (__CCR_MINOR__ > 2 || ( __CCR_MINOR__ == 2 && __CCR_PATCHLEVEL__ >= 1)))
    int main()
    {
        ccrSet_GLES2_Init_Callback(on_GLES2_Init);
        ccrSet_GLES2_Final_Callback(on_GLES2_Final);
        ccrSet_GLES2_Size_Callback(on_GLES2_Size);
        ccrSet_GLES2_Update_Callback(on_GLES2_Update);
        ccrSet_GLES2_Render_Callback(on_GLES2_Render);
        ccrSet_GLES2_TouchBegin_Callback(on_GLES2_TouchBegin);
        ccrSet_GLES2_TouchMove_Callback(on_GLES2_TouchMove);
        ccrSet_GLES2_TouchEnd_Callback(on_GLES2_TouchEnd);

        ccrBegin_GLES2_Drawing();

        while (ccrGetEvent(false) != CCR_EVENT_QUIT);

        ccrEnd_GLES2_Drawing();

        return 0;
    }
#endif
//------------------------------------------------------------------------------
