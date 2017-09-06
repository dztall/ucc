/*****************************************************************************
 * ==> Aligned-axis bounding box ray picking demo ---------------------------*
 *****************************************************************************
 * Description : A ray picking demo with aligned-axis bounding box. Move the *
 *               mouse above the sphere to select a polygon, press the left  *
 *               or right arrow keys to rotate the sphere                    *
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
        HDC                m_hDC;
        HGLRC              m_hRC;
        MINI_Shader        m_Shader;
        GLuint             m_ShaderProgram;
        float*             m_pVertexBuffer;
        unsigned int       m_VertexCount;
        MINI_Index*        m_pIndexes;
        unsigned int       m_IndexCount;
        MINI_AABBNode*     m_pAABBRoot;
        MINI_Polygon*      m_pCollidePolygons;
        unsigned int       m_CollidePolygonsCount;
        float              m_Radius;
        float              m_RayX;
        float              m_RayY;
        float              m_Angle;
        float              m_RotationSpeed;
        float              m_Time;
        float              m_Interval;
        const unsigned int m_FPS;
        float              m_PolygonArray[21];
        MINI_VertexFormat  m_VertexFormat;
        MINI_Matrix        m_ProjectionMatrix;
        MINI_Matrix        m_ViewMatrix;
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
