/*****************************************************************************
 * ==> Underground demo -----------------------------------------------------*
 *****************************************************************************
 * Description : An underground level demo based on ray casting engines like *
 *               Doom or Wolfenstein. Press the up or down arrow keys to     *
 *               walk, and the left or right arrow keys to rotate            *
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
#include "MiniLevel.h"
#include "MiniRenderer.h"

// map mode, used for debugging
//#define MAP_MODE

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
        HDC                m_hDC;
        HGLRC              m_hRC;
        MINI_Shader        m_Shader;
        MINI_LevelItem*    m_pLevel;
        float*             m_pSurfaceVB;
        unsigned int       m_SurfaceVertexCount;
        const float        m_LevelItemWidth;
        const float        m_LevelItemHeight;
        const float        m_LevelItemLength;
        float              m_PosVelocity;
        float              m_DirVelocity;
        float              m_Angle;
        const unsigned int m_MapWidth;
        const unsigned int m_MapHeight;
        unsigned int       m_MapLength;
        MINI_Sphere        m_Player;
        GLuint             m_ShaderProgram;
        GLuint             m_SoilTextureIndex;
        GLuint             m_WallTextureIndex;
        GLuint             m_CeilTextureIndex;
        GLuint             m_TexSamplerSlot;
        GLuint             m_ViewUniform;
        GLuint             m_ModelviewUniform;
        MINI_VertexFormat  m_VertexFormat;
        unsigned __int64   m_PreviousTime;

        #ifdef MAP_MODE
            float             m_SphereRadius;
            float*            m_pSphereVertices;
            unsigned int      m_SphereVertexCount;
            unsigned int      m_SphereIndexCount;
            MINI_Index*       m_pSphereIndexes;
            MINI_VertexFormat m_SphereVertexFormat;
            GLuint            m_SphereTextureIndex;
        #endif

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
