/*****************************************************************************
 * ==> Main -----------------------------------------------------------------*
 *****************************************************************************
 * Description : An intelligent bot demo                                     *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

#ifndef MainH
#define MainH

// vcl
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniModels.h"
#include "MiniLevel.h"
#include "MiniShader.h"
#include "MiniRenderer.h"
#include "MiniPlayer.h"
#include "MiniAI.h"

/**
* Main form
*@author
*/
class TMainForm : public TForm
{
    __published:
        void __fastcall FormShow(TObject* pSender);
        void __fastcall FormResize(TObject* pSender);
        void __fastcall FormPaint(TObject* pSender);
        void __fastcall FormKeyDown(TObject* pSender, WORD& key, TShiftState shift);
        void __fastcall FormKeyUp(TObject* pSender, WORD& key, TShiftState shift);

    public:
        /**
        * Constructor
        *@param pOwner - form owner
        */
        __fastcall TMainForm(TComponent* pOwner);

        /**
        * Destructor
        */
        virtual __fastcall ~TMainForm();

    private:
        struct IBullet
        {
            MINI_Sphere m_Shape;
            float       m_Angle;
            int         m_Fired;
        };

        struct IMDLAnimation
        {
            float m_Range[2];
        };

        HDC                m_hDC;
        HGLRC              m_hRC;
        MINI_Shader        m_Shader;
        MINI_LevelItem*    m_pLevel;
        MINI_MDLModel*     m_pModel;
        MINI_Index*        m_pBulletIndexes;
        GLuint             m_ShaderProgram;
        float*             m_pSurfaceVB;
        float*             m_pBulletVertices;
        const float        m_LevelItemWidth;
        const float        m_LevelItemHeight;
        const float        m_LevelItemLength;
        const float        m_BotVelocity;
        const float        m_BulletVelocity;
        const float        m_BotFadeVelocity;
        const float        m_ControlRadius;
        float              m_PosVelocity;
        float              m_DirVelocity;
        float              m_Angle;
        float              m_BotAngle;
        float              m_Time;
        float              m_StepTime;
        float              m_Interval;
        float              m_StepInterval;
        float              m_BotAlphaLevel;
        float              m_BulletAlphaLevel;
        float              m_FadeLevel;
        float              m_FadeLevelVelocity;
        float              m_HitTime;
        const unsigned int m_FPS;
        const unsigned int m_MapWidth;
        const unsigned int m_MapHeight;
        unsigned int       m_MapLength;
        unsigned int       m_SurfaceVertexCount;
        unsigned int       m_MeshIndex;
        unsigned int       m_CurMeshIndex;
        unsigned int       m_AnimLooped;
        unsigned int       m_BotPathPos;
        unsigned int       m_BotHurt;
        unsigned int       m_BotDied;
        unsigned int       m_BotBulletStopped;
        unsigned int       m_PlayerWasFound;
        unsigned int       m_PlayerHit;
        unsigned int       m_PlayerDied;
        unsigned int       m_PlayerEnergy; // the number of impacts the player can support before die
        unsigned int       m_BotEnergy;    // the number of impacts the bot can support before die
        unsigned int       m_BulletVertexCount;
        unsigned int       m_BulletIndexCount;
        MINI_Sphere        m_Player;
        MINI_Sphere        m_Bot;
        IBullet            m_PlayerBullet;
        IBullet            m_BotBullet;
        MINI_Vector2       m_TouchOrigin;
        MINI_Vector2       m_TouchPosition;
        GLuint             m_SoilTextureIndex;
        GLuint             m_WallTextureIndex;
        GLuint             m_CeilTextureIndex;
        GLuint             m_ModelTextureIndex;
        GLuint             m_BulletTextureIndex;
        GLuint             m_TexSamplerSlot;
        GLuint             m_AlphaSlot;
        GLuint             m_FadeFactorSlot;
        GLuint             m_RedFilterSlot;
        GLuint             m_ViewUniform;
        GLuint             m_ModelviewUniform;
        ALCdevice*         m_pOpenALDevice;
        ALCcontext*        m_pOpenALContext;
        ALuint             m_PlayerStepBufferID;
        ALuint             m_PlayerStepSoundID;
        ALuint             m_PlayerFireBufferID;
        ALuint             m_PlayerFireSoundID;
        MINI_VertexFormat  m_VertexFormat;
        MINI_VertexFormat  m_ModelFormat;
        MINI_VertexFormat  m_BulletVertexFormat;
        IMDLAnimation      m_Animation[5];
        MINI_BotItem       m_Items[1];
        unsigned __int64   m_PreviousTime;

        /**
        * Enables OpenGL
        *@param hWnd - Windows handle
        *@param hDC - device context
        *@param hRC - OpenGL rendering context
        */
        void EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC);

        /**
        * Disables OpenGL
        *@param hWnd - Windows handle
        *@param hDC - device context
        *@param hRC - OpenGL rendering context
        */
        void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);

        /**
        * Creates the viewport
        *@param w - viewport width
        *@param h - viewport height
        */
        void CreateViewport(float w, float h);

        /**
        * Initializes the scene
        *@param w - scene width
        *@param h - scene height
        */
        void InitScene(int w, int h);

        /**
        * Deletes the scene
        */
        void DeleteScene();

        /**
        * Updates the scene
        *@param elapsedTime - elapsed time since last update, in milliseconds
        */
        void UpdateScene(float elapsedTime);

        /**
        * Draws the scene
        */
        void DrawScene();

        /**
        * Called while application is idle
        *@param pSender - event sender
        *@param[in, out] done - if true, event is done and will no longer be called
        */
        void __fastcall OnIdle(TObject* pSender, bool& done);

        /**
        * Checks if a bullet hit the player
        *@return 1 if the bullet hit the player, otherwise 0
        */
        int BulletHitPlayer();

        /**
        * Checks if a bullet hit the bot
        *@return 1 if the bullet hit the bot, otherwise 0
        */
        int BulletHitBot();

        /**
        * Checks if the bot spotted the player
        *@return 1 if the bot spotted the player, otherwise 0
        */
        int BotSpottedPlayer();

        /**
        * Runs and loops the bot animation
        *@param animIndex - animation index
        *@param elapsedTime - elapsed time since last process
        */
        void BotRunAnimation(int animIndex, float elapsedTime);

        /**
        * Runs the bot animation once
        *@param animIndex - animation index
        *@param elapsedTime - elapsed time since last process
        *@return 1 if animation end was reached, otherwise 0
        */
        int BotRunAnimationOnce(int animIndex, float elapsedTime);

        /**
        * Calculates the angle to apply to bot to look at the player
        *@return angle in radian
        */
        float BotLookAtPlayerAngle();

        /**
        * Converts a path position to an level item index
        *@param pathPos - path position to convert
        *@param pPath - path
        *@param pathLength - path length
        *@return level item index matching with path position
        */
        static int PathPosToLevelIndex(int pathPos, const int* pPath, unsigned pathLength);

        /**
        * Checks if a bullet hit a body
        *@param pBullet - bullet to check
        *@param pBody - body to check against
        *@return 1 if the bullet hit the body, otherwise 0
        */
        static int BulletHitBody(IBullet* pBullet, MINI_Sphere* pBody);

        /**
        * Called when bot watching task should be executed
        *@param elapedTime - elapsed time since last process
        *@param pItem - bot item for which the task should be executed
        *@return 1 if the task ended and the next one should be executed, otherwise 0
        */
        static int OnBotWatching(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called when bot searching task should be executed
        *@param elapedTime - elapsed time since last process
        *@param pItem - bot item for which the task should be executed
        *@return 1 if the task ended and the next one should be executed, otherwise 0
        */
        static int OnBotSearching(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called when bot attacking task should be executed
        *@param elapedTime - elapsed time since last process
        *@param pItem - bot item for which the task should be executed
        *@return 1 if the task ended and the next one should be executed, otherwise 0
        */
        static int OnBotAttacking(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called when bot being hurt task should be executed
        *@param elapedTime - elapsed time since last process
        *@param pItem - bot item for which the task should be executed
        *@return 1 if the task ended and the next one should be executed, otherwise 0
        */
        static int OnBotBeingHurt(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called when bot dying task should be executed
        *@param elapedTime - elapsed time since last process
        *@param pItem - bot item for which the task should be executed
        *@return 1 if the task ended and the next one should be executed, otherwise 0
        */
        static int OnBotDying(float elapsedTime, MINI_BotItem* pItem);

        /**
        * Called when a new task should be sheluded for the bot
        *@param elapedTime - elapsed time since last process
        *@param pItem - bot item for which the task should be executed
        */
        static void OnNewTask(float elapsedTime, MINI_BotItem* pItem);
};
extern PACKAGE TMainForm* MainForm;
#endif
