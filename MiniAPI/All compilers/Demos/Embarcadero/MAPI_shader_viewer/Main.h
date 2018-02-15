/*****************************************************************************
 * ==> Main -----------------------------------------------------------------*
 *****************************************************************************
 * Description : A fragment (also named pixel) shader viewer                 *
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
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>

// std
#include <string>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

/**
* Fragment shader viewer main form
*@author
*/
class TMainForm : public TForm
{
    __published:
        TComboBox *cbShaders;
        TPanel *paHeader;
        TButton *btChangeDir;
        TPanel *paView;
        TOpenDialog *odOpen;

        void __fastcall FormShow(TObject* pSender);
        void __fastcall FormResize(TObject* pSender);
        void __fastcall paViewMouseMove(TObject* pSender, TShiftState shift, int x, int y);
        void __fastcall cbShadersChange(TObject* pSender);
        void __fastcall btChangeDirClick(TObject* pSender);

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

    protected:
        /**
        * View panel main procedure
        *@param message- Windows procedure message
        */
        void __fastcall ViewWndProc(TMessage& message);

    private:
        HDC               m_hDC;
        HGLRC             m_hRC;
        MINI_Shader       m_Shader;
        GLuint            m_ShaderProgram;
        float*            m_pSurfaceVB;
        unsigned int      m_SurfaceVertexCount;
        const float       m_SurfaceWidth;
        const float       m_SurfaceHeight;
        const float       m_MaxTime;
        float             m_Time;
        GLuint            m_TimeSlot;
        GLuint            m_SizeSlot;
        GLuint            m_ResolutionSlot;
        GLuint            m_MouseSlot;
        MINI_Vector2      m_Resolution;
        MINI_Vector2      m_MousePos;
        MINI_VertexFormat m_VertexFormat;
        std::string       m_ShaderDir;
        unsigned __int64  m_PreviousTime;
        bool              m_Initialized;
        TWndMethod        m_fViewWndProc_Backup;

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
        * Populates the combo box
        */
        void PopulateCombo();

        /**
        * Loads, builds and compiles a shader from a file
        *@param fileName - shader file name
        *@return true on success, otherwise false
        */
        bool LoadShader(const std::string& fileName);

        /**
        * get the shader program from a file
        *@param fileName - shader file name
        *@return shader program, empty string on error
        */
        std::string ReadShader(const std::string& fileName) const;

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
        * Called when the scene should be drawn
        *@param resize - if true, the scene should be repainted during a resize
        */
        void OnDrawScene(bool resize);

        /**
        * Called while application is idle
        *@param pSender - event sender
        *@param[in, out] done - if true, event is done and will no longer be called
        */
        void __fastcall OnIdle(TObject* pSender, bool& done);
};
extern PACKAGE TMainForm* MainForm;
#endif
