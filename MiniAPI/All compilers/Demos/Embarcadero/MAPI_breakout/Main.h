/*****************************************************************************
 * ==> Breakout game demo ---------------------------------------------------*
 *****************************************************************************
 * Description : A simple breakout game                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
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
#include "MiniShader.h"
#include "MiniRenderer.h"
#include "MiniPlayer.h"

/**
* Demo main form
*@author Jean-Milost Reymond
*/
class TMainForm : public TForm
{
    __published:
        void __fastcall FormShow(TObject* pSender);
        void __fastcall FormResize(TObject* pSender);
        void __fastcall FormPaint(TObject* pSender);
        void __fastcall FormKeyDown(TObject* pSender, WORD& key, TShiftState shift);
        void __fastcall FormMouseMove(TObject* pSender, TShiftState shift, int x, int y);

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
        /**
        * The screen in the OpenGL viewport
        */
        struct IScreen
        {
            float m_Left;
            float m_Right;
            float m_Top;
            float m_Bottom;
            float m_BarY;
        };

        /**
        * The bar info
        */
        struct IBar
        {
            MINI_Rect    m_Geometry;
            MINI_Vector2 m_L;
            MINI_Vector2 m_R;
            int          m_Exploding;
            MINI_Vector2 m_ExpLOffset;
            MINI_Vector2 m_ExpROffset;
            ALuint       m_BufferID;
            ALuint       m_SoundID;
        };

        /**
        * The ball info
        */
        struct IBall
        {
            MINI_Circle  m_Geometry;
            MINI_Vector2 m_Offset;
            MINI_Vector2 m_Inc;
            MINI_Vector2 m_Max;
            ALuint       m_BufferID;
            ALuint       m_SoundID;
        };

        /**
        * The block info
        */
        struct IBlock
        {
            MINI_Rect m_Geometry;
            int       m_Visible;
        };

        HDC               m_hDC;
        HGLRC             m_hRC;
        MINI_Shader       m_Shader;
        IScreen           m_Screen;
        IBall             m_Ball;
        IBar              m_Bar;
        IBlock            m_Blocks[15];
        float*            m_pBarVertices;
        unsigned          m_BarVerticesCount;
        float*            m_pBlockVertices;
        unsigned          m_BlockVerticesCount;
        float*            m_pBallVertices;
        MINI_Index*       m_pBallIndexes;
        unsigned          m_BallVerticesCount;
        unsigned          m_BallIndexCount;
        int               m_BlockColumns;
        int               m_BlockLines;
        int               m_Level;
        MINI_VertexFormat m_VertexFormat;
        GLuint            m_ShaderProgram;
        ALCdevice*        m_pOpenALDevice;
        ALCcontext*       m_pOpenALContext;
        unsigned __int64  m_PreviousTime;

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
        * Get the screen info
        *@param width - screen width
        *@param height - screen height
        *@param[out] screen - screen info
        */
        void GetScreen(float width, float height, IScreen& screen);

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
        * Draws the level item
        *@param pTranslate - translation vector
        *@param angle - rotation angle
        *@param pRotateAxis - rotation axis
        */
        void DrawItem(const MINI_Vector3* pTranslate,
                            float         angle,
                      const MINI_Vector3* pRotateAxis);

        /**
        * Called while application is idle
        *@param pSender - event sender
        *@param[in, out] done - if true, event is done and will no longer be called
        */
        void __fastcall OnIdle(TObject* pSender, bool& done);
};
extern PACKAGE TMainForm* MainForm;
#endif
