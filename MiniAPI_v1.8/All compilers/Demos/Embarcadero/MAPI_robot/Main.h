/*****************************************************************************
 * ==> Quake II (MD2) model demo --------------------------------------------*
 *****************************************************************************
 * Description : A Quake II (MD2) model showing a robot. Press the left or   *
 *               right arrow key to change the animation                     *
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
#include "MiniVertex.h"
#include "MiniModels.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

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
        struct IMD2Animation
        {
            float m_Range[2];
        };

        HDC                m_hDC;
        HGLRC              m_hRC;
        MINI_Shader        m_Shader;
        MINI_VertexFormat  m_ModelFormat;
        MINI_MD2Model*     m_pModel;
        unsigned int       m_MeshIndex;
        GLuint             m_ShaderProgram;
        GLuint             m_TextureIndex;
        GLuint             m_PositionSlot;
        GLuint             m_ColorSlot;
        GLuint             m_TexCoordSlot;
        GLuint             m_TexSamplerSlot;
        float              m_Time;
        float              m_Interval;
        float              m_ScreenWidth;
        const unsigned int m_FPS;
        unsigned int       m_AnimIndex; // can only be 0 (robot walks) or 1 (robot dies)
        IMD2Animation      m_Animation[2];
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
        *@param timeStep_sec - elapsed time since last update, in milliseconds
        */
        void UpdateScene(float timeStep_sec);

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
};
extern PACKAGE TMainForm* MainForm;
#endif
