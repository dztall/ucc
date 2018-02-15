/*****************************************************************************
 * ==> Shader viewer --------------------------------------------------------*
 *****************************************************************************
 * Description : A shader viewer tool                                        *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <stdio.h>
#include <math.h>
#include <stdexcept>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

// libraries
#include <sys/stat.h>

// windows
#include <Windows.h>
#include <Windowsx.h>
#include <commctrl.h>

// resources
#include "resource.rh"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
// generic vertex shader program
const char* g_pVertexSader =
    "precision mediump float;"
    "attribute vec3  mini_aPosition;"
    "uniform   float mini_uTime;"
    "uniform   vec2  mini_uResolution;"
    "uniform   vec2  mini_uMouse;"
    "varying   float iTime;"
    "varying   vec2  iResolution;"
    "varying   vec2  iMouse;"
    ""
    "void main(void)"
    "{"
    "    iResolution = mini_uResolution;"
    "    iTime       = mini_uTime;"
    "    iMouse      = mini_uMouse;"
    "    gl_Position = vec4(mini_aPosition, 1.0);"
    "}";
//------------------------------------------------------------------------------
HWND              g_hView              = NULL;
HDC               g_hDC                = NULL;
HGLRC             g_hRC                = NULL;
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram;
float*            g_pSurfaceVB         =  0;
unsigned int      g_SurfaceVertexCount =  0;
const float       g_SurfaceWidth       =  10.0f;
const float       g_SurfaceHeight      =  12.5f;
const float       g_MaxTime            =  12.0f * M_PI;
float             g_Time               =  0.0f;
GLuint            g_TimeSlot           =  0;
GLuint            g_SizeSlot           =  0;
GLuint            g_ResolutionSlot     =  0;
GLuint            g_MouseSlot          =  0;
unsigned          g_ShaderFileSize     =  0;
std::string       g_ShaderFile;
MINI_Vector2      g_Resolution;
MINI_Vector2      g_MousePos;
MINI_VertexFormat g_VertexFormat;
int               g_SceneInitialized   = 0;
unsigned __int64  g_PreviousTime       = 0L;
//------------------------------------------------------------------------------
void EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    // get the device context (DC)
    *hDC = GetDC(hWnd);

    // set the pixel format for the DC
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    // create and enable the render context (RC)
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}
//------------------------------------------------------------------------------
void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
//------------------------------------------------------------------------------
std::string ExtractFilePath(const std::string& path)
{
    // search for the last separator
    std::size_t pos = path.rfind('\\', path.length());

    // found it?
    if (pos != path.npos)
        return path.substr(0, pos);

    return path;
}
//---------------------------------------------------------------------------
bool DirExists(const std::string& name)
{
    struct stat statbuf;

    if (stat(name.c_str(), &statbuf) != 0)
        return false;

    return S_ISDIR(statbuf.st_mode);
}
//------------------------------------------------------------------------------
std::string GetShaderDir()
{
    HMODULE hModule = ::GetModuleHandleW(NULL);

    if (!hModule)
        return "";

    CHAR path[MAX_PATH];
    ::GetModuleFileNameA(hModule, path, MAX_PATH);

    // build the model dir from the application exe
    std::string dir  = ExtractFilePath(path);
                dir  = ExtractFilePath(dir);
                dir  = ExtractFilePath(dir);
                dir  = ExtractFilePath(dir);
                dir += "\\Shaders";

    // dir exists?
    if (DirExists(dir))
        return dir;

    // build the model dir from the application exe, in his /Debug or /Release path
    dir  = ExtractFilePath(path);
    dir  = ExtractFilePath(dir);
    dir  = ExtractFilePath(dir);
    dir  = ExtractFilePath(dir);
    dir  = ExtractFilePath(dir);
    dir  = ExtractFilePath(dir);
    dir += "\\Shaders";

    // get the shader dir global variable
    if (DirExists(dir))
        return dir;

    return "";
}
//------------------------------------------------------------------------------
void PopulateComboBox(HWND hCombo, const std::string& dir)
{
    // build the shader dir to read from, with the filter to use
    std::string shaderDir = dir + "\\*.shader";

    // get the first file in dir
    WIN32_FIND_DATA data;
    HANDLE hFind = ::FindFirstFile(shaderDir.c_str(), &data);
    bool   found = false;

    // iterate through files contained in dir
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // add file item to combo box
            ::SendMessage(hCombo, CB_ADDSTRING, WPARAM(0), LPARAM(data.cFileName));

            found = true;
        } while (::FindNextFile(hFind, &data));

        ::FindClose(hFind);
    }

    // select the first item in combo box
    if (found)
        ::SendMessage(hCombo, CB_SETCURSEL, WPARAM(0), LPARAM(0));
}
//------------------------------------------------------------------------------
void ReadShader(const char* pFileName, long fileSize, char** pShader)
{
    // open shader file
    FILE* pFile = fopen(pFileName, "rb");

    // succeeded?
    if (!pFile)
        return;

    if (pFile)
    {
        *pShader = (char*)malloc(fileSize + 1);

        if (*pShader)
        {
            fread(*pShader, 1, fileSize, pFile);
            (*pShader)[fileSize] = '\0';
        }

        fclose(pFile);
    }
}
//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
{
    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // set the screen resolution
    g_Resolution.m_X = w;
    g_Resolution.m_Y = h;

    // notify shader about screen size
    glUniform2f(g_ResolutionSlot, g_Resolution.m_X, g_Resolution.m_Y);

    // initialize the mouse (or finger) position in the shader
    glUniform2f(g_MouseSlot, 0, 0);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    char* pShader = 0;

    // read the shader content
    ReadShader(g_ShaderFile.c_str(), g_ShaderFileSize, &pShader);

    // failed?
    if (!pShader)
        throw std::runtime_error("ERROR - Failed to open the file - " + std::string(g_ShaderFile));

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(g_pVertexSader, pShader);
    glUseProgram(g_ShaderProgram);

    free(pShader);

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram,  "mini_aPosition");
    g_TimeSlot            = glGetUniformLocation(g_ShaderProgram, "mini_uTime");
    g_ResolutionSlot      = glGetUniformLocation(g_ShaderProgram, "mini_uResolution");
    g_MouseSlot           = glGetUniformLocation(g_ShaderProgram, "mini_uMouse");

    CreateViewport(w, h);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 0;

    // generate surface
    miniCreateSurface(&g_SurfaceWidth,
                      &g_SurfaceHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime)
{
    // calculate next time
    g_Time += elapsedTime * 0.5f;

    // is time out of bounds?
    if (g_Time > g_MaxTime)
        g_Time -= g_MaxTime;

    // notify shader about elapsed time
    glUniform1f(g_TimeSlot, g_Time);
}
//------------------------------------------------------------------------------
void DrawScene()
{
    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // draw the surface on which the shader will be executed
    miniDrawSurface(g_pSurfaceVB,
                    g_SurfaceVertexCount,
                    &g_VertexFormat,
                    &g_Shader);

    miniEndScene();
}
//------------------------------------------------------------------------------
BOOL CALLBACK DlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // dispatch messages
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // load application icon from resources
            HICON hIcon = (HICON)::LoadImage(::GetModuleHandle(NULL),
                                             MAKEINTRESOURCE(IDI_MAIN_ICON),
                                             IMAGE_ICON,
                                             16,
                                             16,
                                             0);

            // succeeded?
            if (hIcon)
                ::SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

            // get the combo box
            HWND hCombo = ::GetDlgItem(hWnd, 0);

            // found it?
            if (!hCombo)
            {
                // terminate the dialog box
                ::EndDialog(hWnd, 0);

                return FALSE;
            }

            // populate the combo box with the shader dir content
            PopulateComboBox(hCombo, GetShaderDir());

            // use the static text to show the view content. It's a hack but there is no better control to do that
            g_hView = ::GetDlgItem(hWnd, 1);

            // found it?
            if (!g_hView)
            {
                // terminate the dialog box
                ::EndDialog(hWnd, 0);

                return FALSE;
            }

            // enable OpenGL for the window
            EnableOpenGL(g_hView, &g_hDC, &g_hRC);

            // stop GLEW crashing on OSX :-/
            glewExperimental = GL_TRUE;

            // initialize GLEW
            if (glewInit() != GLEW_OK)
            {
                // shutdown OpenGL
                DisableOpenGL(g_hView, g_hDC, g_hRC);

                // terminate the dialog box
                ::EndDialog(hWnd, 0);

                return FALSE;
            }

            // get the currently selected item
            const int itemIndex = ::SendMessage(hCombo, CB_GETCURSEL, WPARAM(0), LPARAM(0));

            // not found?
            if (itemIndex < 0)
            {
                // shutdown OpenGL
                DisableOpenGL(g_hView, g_hDC, g_hRC);

                // terminate the dialog box
                ::EndDialog(hWnd, 0);

                return FALSE;
            }

            // get the item content (as text)
            TCHAR itemContent[256];
            ::SendMessage(hCombo, CB_GETLBTEXT, WPARAM(itemIndex), LPARAM(itemContent));

            // build the shader file name to read
            g_ShaderFile     = GetShaderDir() + "\\" + std::string(itemContent);
            g_ShaderFileSize = miniGetFileSize(g_ShaderFile.c_str());

            RECT clientRect;
            ::GetClientRect(g_hView, &clientRect);

            // initialize the scene
            InitScene(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

            // initialize the timer
            g_PreviousTime = GetTickCount();

            // set a timer to send a refresh message every 20 milliseconds
            ::SetTimer(hWnd, 1, 20, NULL);

            return TRUE;
        }

        case WM_WINDOWPOSCHANGED:
        {
            // get the dialog box client rect
            RECT clientRect;
            ::GetClientRect(hWnd, &clientRect);

            // get the combo box
            HWND hCombo = ::GetDlgItem(hWnd, 0);

            RECT comboRect;
            comboRect.top    = 0;
            comboRect.bottom = 0;

            // found it?
            if (hCombo)
            {
                // get the combo box rect
                ::GetWindowRect(hCombo, &comboRect);

                // update the combo box position to make it always on the top of the dialog client rect
                ::SetWindowPos(hCombo,
                               NULL,
                               clientRect.left  + 3,
                               clientRect.top   + 3,
                               clientRect.right - 6,
                               clientRect.top   + (comboRect.bottom - comboRect.top) + 3,
                               SWP_NOZORDER);
            }

            // update the view to make it always filling the client area
            if (g_hView)
            {
                ::SetWindowPos(g_hView,
                               NULL,
                               clientRect.left,
                               (comboRect.bottom - comboRect.top) + 6,
                               clientRect.right,
                               clientRect.bottom,
                               SWP_NOZORDER);

                if (g_SceneInitialized)
                    CreateViewport(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
            }

            return FALSE;
        }

        case WM_MOUSEMOVE:
        {
            const float x = GET_X_LPARAM(lParam);
            const float y = GET_Y_LPARAM(lParam);

            glUniform2f(g_MouseSlot, x, y);

            return FALSE;
        }

        case WM_TIMER:
        {
            // calculate time interval
            const unsigned __int64 now            =  GetTickCount();
            const double           elapsedTime    = (now - g_PreviousTime) / 1000.0;
                                   g_PreviousTime =  now;

            UpdateScene(elapsedTime);
            DrawScene();

            ::SwapBuffers(g_hDC);
            return TRUE;
        }

        case WM_COMMAND:
        {
            if (!g_SceneInitialized)
                return TRUE;

            switch (HIWORD(wParam))
            {
                case CBN_SELCHANGE:
                {
                    // get the newly selected item
                    const int itemIndex = ::SendMessage(HWND(lParam), CB_GETCURSEL, WPARAM(0), LPARAM(0));

                    // not found?
                    if (itemIndex < 0)
                        return TRUE;

                    // get the item content (as text)
                    TCHAR itemContent[256];
                    ::SendMessage(HWND(lParam), CB_GETLBTEXT, WPARAM(itemIndex), LPARAM(itemContent));

                    // build the shader file name to read
                    g_ShaderFile     = GetShaderDir() + "\\" + std::string(itemContent);
                    g_ShaderFileSize = miniGetFileSize(g_ShaderFile.c_str());

                    char* pShader = 0;

                    // read the shader content
                    ReadShader(g_ShaderFile.c_str(), g_ShaderFileSize, &pShader);

                    // failed?
                    if (!pShader)
                        throw std::runtime_error("ERROR - Failed to open the file - " + std::string(g_ShaderFile));

                    // compile, link and use shader
                    g_ShaderProgram = miniCompileShaders(g_pVertexSader, pShader);
                    glUseProgram(g_ShaderProgram);

                    free(pShader);

                    // get shader attributes
                    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram,  "mini_aPosition");
                    g_TimeSlot            = glGetUniformLocation(g_ShaderProgram, "mini_uTime");
                    g_ResolutionSlot      = glGetUniformLocation(g_ShaderProgram, "mini_uResolution");
                    g_MouseSlot           = glGetUniformLocation(g_ShaderProgram, "mini_uMouse");

                    // recreate the viewport
                    CreateViewport(g_Resolution.m_X, g_Resolution.m_Y);
                }
            }

            return TRUE;
        }

        case WM_CLOSE:
            // delete the scene
            DeleteScene();

            // shutdown OpenGL
            DisableOpenGL(g_hView, g_hDC, g_hRC);

            ::EndDialog(hWnd, 0);
            return TRUE;
    }

    return FALSE;
}
//------------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    ::InitCommonControls();

    return ::DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}
//------------------------------------------------------------------------------
