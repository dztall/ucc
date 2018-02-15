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

#include <vcl.h>
#pragma hdrstop
#include "Main.h"

// std
#include <math.h>

#pragma package(smart_init)
#ifdef __llvm__
    #pragma link "glewSL.a"
#else
    #pragma link "glewSL.lib"
#endif
#pragma resource "*.dfm"

//------------------------------------------------------------------------------
// generic vertex shader program
const char* g_pVertexShader =
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
//---------------------------------------------------------------------------
TMainForm* MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* pOwner) :
    TForm(pOwner),
    m_ShaderProgram(0),
    m_pSurfaceVB(0),
    m_SurfaceVertexCount(0),
    m_SurfaceWidth(10.0f),
    m_SurfaceHeight(12.5f),
    m_MaxTime(12.0f * M_PI),
    m_Time(0.0f),
    m_TimeSlot(0),
    m_SizeSlot(0),
    m_ResolutionSlot(0),
    m_MouseSlot(0),
    m_PreviousTime(0),
    m_Initialized(false),
    m_fViewWndProc_Backup(NULL)
{
    // enable OpenGL
    EnableOpenGL(paView->Handle, &m_hDC, &m_hRC);

    // stop GLEW crashing on OSX :-/
    glewExperimental = GL_TRUE;

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        // shutdown OpenGL
        DisableOpenGL(paView->Handle, m_hDC, m_hRC);

        // close the app
        Application->Terminate();
    }

    // build the model dir from the application exe
    UnicodeString shaderDir = ::ExtractFilePath(Application->ExeName);
                  shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
                  shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
                  shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
                  shaderDir = ::ExcludeTrailingPathDelimiter(shaderDir) + L"\\Shaders\\";

    // dir exists?
    if (!::DirectoryExists(shaderDir))
    {
        // build the model dir from the application exe, in his /Debug or /Release path
        shaderDir = ::ExtractFilePath(Application->ExeName);
        shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
        shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
        shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
        shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
        shaderDir = ::ExtractFilePath(::ExcludeTrailingPathDelimiter(shaderDir));
        shaderDir = ::ExcludeTrailingPathDelimiter(shaderDir) + L"\\Shaders\\";
    }

    // get the shader dir global variable
    if (::DirectoryExists(shaderDir))
        m_ShaderDir = AnsiString(shaderDir).c_str();
}
//---------------------------------------------------------------------------
__fastcall TMainForm::~TMainForm()
{
    // restore the normal view procedure
    if (m_fViewWndProc_Backup)
        paView->WindowProc = m_fViewWndProc_Backup;

    DeleteScene();
    DisableOpenGL(paView->Handle, m_hDC, m_hRC);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject* pSender)
{
    // populate the combo box
    PopulateCombo();

    // select the first shader
    if (cbShaders->Items->Count)
        cbShaders->ItemIndex = 0;

    // hook the panel procedure
    m_fViewWndProc_Backup = paView->WindowProc;
    paView->WindowProc    = ViewWndProc;

    // initialize the scene
    InitScene(paView->ClientWidth, paView->ClientHeight);

    // initialize the timer
    m_PreviousTime = ::GetTickCount();

    // listen the application idle
    Application->OnIdle = OnIdle;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject* pSender)
{
    // update the viewport
    CreateViewport(paView->ClientWidth, paView->ClientHeight);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::paViewMouseMove(TObject* pSender, TShiftState shift, int x, int y)
{
    // notify shader about mouse position
    glUniform2f(m_MouseSlot, x, y);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::cbShadersChange(TObject* pSender)
{
    if (cbShaders->ItemIndex < 0 || cbShaders->ItemIndex >= cbShaders->Items->Count)
    {
        ::MessageDlg(L"Cannot load the shader.\r\n\r\nThe shader index is out of bounds.",
                     mtError,
                     TMsgDlgButtons() << mbOK,
                     0);

        return;
    }

    // get the shader file name to load
    const std::string shaderFile =
            m_ShaderDir + AnsiString(cbShaders->Items->Strings[cbShaders->ItemIndex]).c_str();

    // load the shader
    if (shaderFile.length() && !LoadShader(shaderFile))
        ::MessageDlg(UnicodeString(AnsiString("Could not load the shader:\r\n") + shaderFile.c_str()),
                     mtError,
                     TMsgDlgButtons() << mbOK,
                     0);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::btChangeDirClick(TObject* pSender)
{
    // configure the default dir
    odOpen->InitialDir = UnicodeString(AnsiString(m_ShaderDir.c_str()));

    // show the open dialog box and wait for user result
    if (!odOpen->Execute())
        return;

    // update the shader dir global variable
    m_ShaderDir = AnsiString(::ExtractFilePath(odOpen->FileName)).c_str();

    // populate the combo box
    PopulateCombo();

    // is combo box empty?
    if (!cbShaders->Items->Count)
        return;

    // get the loaded shader index
    const int index = cbShaders->Items->IndexOf(::ExtractFileName(odOpen->FileName));

    // found it?
    if (index != -1)
        // select the shader
        cbShaders->ItemIndex = index;

    // load the shader
    cbShadersChange(this);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewWndProc(TMessage& message)
{
    switch (message.Msg)
    {
        case WM_PAINT:
        {
            // is scene initialized?
            if (!m_Initialized)
                break;

            HDC           hDC = NULL;
            ::PAINTSTRUCT ps;

            try
            {
                // get and lock the view device context
                hDC = ::BeginPaint(paView->Handle, &ps);

                // on success, draw the scene
                if (hDC)
                    OnDrawScene(false);
            }
            __finally
            {
                // unlock and release the device context
                ::EndPaint(paView->Handle, &ps);
            }

            return;
        }
    }

    if (m_fViewWndProc_Backup)
        m_fViewWndProc_Backup(message);
}
//---------------------------------------------------------------------------
void TMainForm::EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    // get the device context
    *hDC = ::GetDC(hWnd);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    // set the pixel format for the device context
    iFormat = ChoosePixelFormat(*hDC, &pfd);
    SetPixelFormat(*hDC, iFormat, &pfd);

    // create and enable the OpenGL render context
    *hRC = wglCreateContext(*hDC);
    wglMakeCurrent(*hDC, *hRC);
}
//------------------------------------------------------------------------------
void TMainForm::DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
//------------------------------------------------------------------------------
void TMainForm::PopulateCombo()
{
    // clear the previous combo box
    cbShaders->Clear();

    TSearchRec searchRec;

    // fill the combo box with the shader dir content
    if (!::FindFirst(UnicodeString(AnsiString(m_ShaderDir.c_str())) + L"*.shader", faArchive, searchRec))
    {
        do
        {
            cbShaders->Items->Add(::ExtractFileName(searchRec.Name));
        }
        while (!::FindNext(searchRec));

        FindClose((TSearchRec&)searchRec);
    }
}
//------------------------------------------------------------------------------
bool TMainForm::LoadShader(const std::string& fileName)
{
    // read the shader content
    const std::string shader = ReadShader(fileName);

    // failed?
    if (shader.empty())
        return false;

    // compile, link and use shader
    m_ShaderProgram = miniCompileShaders(g_pVertexShader, shader.c_str());
    glUseProgram(m_ShaderProgram);

    // get shader attributes
    m_Shader.m_VertexSlot = glGetAttribLocation(m_ShaderProgram,  "mini_aPosition");
    m_TimeSlot            = glGetUniformLocation(m_ShaderProgram, "mini_uTime");
    m_ResolutionSlot      = glGetUniformLocation(m_ShaderProgram, "mini_uResolution");
    m_MouseSlot           = glGetUniformLocation(m_ShaderProgram, "mini_uMouse");

    // notify shader about mouse (or finger) position
    glUniform2f(m_MouseSlot, 0.0f, 0.0f);

    // create the viewport
    CreateViewport(paView->ClientWidth, paView->ClientHeight);

    return true;
}
//------------------------------------------------------------------------------
std::string TMainForm::ReadShader(const std::string& fileName) const
{
    // get the file size
    const std::size_t fileSize = miniGetFileSize(fileName.c_str());

    // is file empty?
    if (!fileSize)
        return "";

    // open shader file
    FILE* pFile = fopen(fileName.c_str(), "rb");

    // succeeded?
    if (!pFile)
        return "";

    char*       pBuffer = 0;
    std::string shader;

    try
    {
        // reserve memory for the shader program
        pBuffer = (char*)malloc(fileSize + 1);

        // succeeded?
        if (pBuffer)
        {
            // read the shader program from the file
            fread(pBuffer, 1, fileSize, pFile);
            pBuffer[fileSize] = '\0';
            shader            = pBuffer;
        }
    }
    __finally
    {
        // delete the buffer
        free(pBuffer);

        // close the file
        fclose(pFile);
    }

    return shader;
}
//------------------------------------------------------------------------------
void TMainForm::CreateViewport(float w, float h)
{
    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // notify shader about screen size
    glUniform2f(m_ResolutionSlot, w, h);
}
//------------------------------------------------------------------------------
void TMainForm::InitScene(int w, int h)
{
    // is shader list empty?
    if (!cbShaders->Items->Count)
        // select one
        btChangeDir->Click();

    // load the default shader
    cbShadersChange(this);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    m_VertexFormat.m_UseNormals  = 0;
    m_VertexFormat.m_UseTextures = 0;
    m_VertexFormat.m_UseColors   = 0;

    // generate surface
    miniCreateSurface(&m_SurfaceWidth,
                      &m_SurfaceHeight,
                       0xFFFFFFFF,
                      &m_VertexFormat,
                      &m_pSurfaceVB,
                      &m_SurfaceVertexCount);

    m_Initialized = true;
}
//------------------------------------------------------------------------------
void TMainForm::DeleteScene()
{
    m_Initialized = false;

    // delete surface vertices
    if (m_pSurfaceVB)
    {
        free(m_pSurfaceVB);
        m_pSurfaceVB = 0;
    }

    // delete shader program
    if (m_ShaderProgram)
        glDeleteProgram(m_ShaderProgram);

    m_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void TMainForm::UpdateScene(float elapsedTime)
{
    // calculate next time
    m_Time += elapsedTime * 0.5f;

    // is time out of bounds?
    if (m_Time > m_MaxTime)
        m_Time -= m_MaxTime;

    // notify shader about elapsed time
    glUniform1f(m_TimeSlot, m_Time);
}
//------------------------------------------------------------------------------
void TMainForm::DrawScene()
{
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // draw the surface on which the shader will be executed
    miniDrawSurface(m_pSurfaceVB, m_SurfaceVertexCount, &m_VertexFormat, &m_Shader);

    miniEndScene();
}
//---------------------------------------------------------------------------
void TMainForm::OnDrawScene(bool resize)
{
    // do draw the scene for a resize?
    if (resize)
    {
        if (!m_Initialized)
            return;

        // just process a minimal draw
        UpdateScene(0.0);
        DrawScene();

        ::SwapBuffers(m_hDC);
        return;
    }

    if (!m_Initialized)
        return;

    // calculate time interval
    const unsigned __int64 now            = ::GetTickCount();
    const double           elapsedTime    = (now - m_PreviousTime) / 1000.0;
                           m_PreviousTime =  now;

    // update and draw the scene
    UpdateScene(elapsedTime);
    DrawScene();

    ::SwapBuffers(m_hDC);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnIdle(TObject* pSender, bool& done)
{
    done = false;
    OnDrawScene(false);
}
//---------------------------------------------------------------------------
