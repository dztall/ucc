/*****************************************************************************
 * ==> Star field -----------------------------------------------------------*
 *****************************************************************************
 * Description : OpenGL starfield demo                                       *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

// supported platforms check. NOTE iOS, Android and Windows only, but may works on other platforms
#if !defined(_OS_IOS_) && !defined(_OS_ANDROID_) && !defined(_OS_WINDOWS_)
    #error "Not supported platform!"
#endif

#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// OpenGL
#include <gles2.h>
#include <gles2ext.h>

//------------------------------------------------------------------------------
#define M_Nb_Star   32
#define M_StarDepth 1000
//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef ANDROID
        GLuint renderbuffer, framebuffer;
    #endif
#endif

GLuint simpleProgram;
//------------------------------------------------------------------------------
struct QR_Vector2
{
    float m_X; // x coordinate for the 2D vector
    float m_Y; // y coordinate for the 2D vector
};
//------------------------------------------------------------------------------
struct QR_Vector3
{
    float m_X; // x coordinate for the 3D vector
    float m_Y; // y coordinate for the 3D vector
    float m_Z; // z coordinate for the 3D vector
};
//------------------------------------------------------------------------------
QR_Vector3 starfield[M_Nb_Star];
//------------------------------------------------------------------------------
struct QR_Vertex
{
    float m_Position[2];
    float m_Color[4];
};
//------------------------------------------------------------------------------
const QR_Vertex vertices[6] =
{
    {{-0.01f, -0.01f}, {1, 0, 0, 1}},
    {{ 0.01f, -0.01f}, {1, 0, 0, 1}},
    {{-0.01f,  0.01f}, {1, 0, 0, 1}},
    {{ 0.01f, -0.01f}, {1, 0, 0, 1}},
    {{-0.01f,  0.01f}, {1, 0, 0, 1}},
    {{ 0.01f,  0.01f}, {1, 0, 0, 1}},
};
//------------------------------------------------------------------------------
const char* pSimpleVertexShader =
    "attribute vec4 Position;"
    "attribute vec4 SourceColor;"
    "varying vec4 DestinationColor;"
    "uniform mat4 Projection;"
    "uniform mat4 Modelview;"
    "void main(void)"
    "{"
    "    DestinationColor = SourceColor;"
    "    gl_Position      = Projection * Modelview * Position;"
    "}";
//------------------------------------------------------------------------------
const char* pSimpleFragmentShader =
    "varying lowp vec4 DestinationColor;"
    "void main(void)"
    "{"
    "    gl_FragColor = DestinationColor;"
    "}";
//------------------------------------------------------------------------------
void InitializeStarfield()
{
    srand(time(NULL));

    // virtual screen width and height
    int screenWidth  = 1500;
    int screenHeight = 1500;

    // initialize star field
    for (int i = 0; i < M_Nb_Star; ++i)
    {
        // get random value inside screen coordinates
        float randX = (rand() % screenWidth);
        float randY = (rand() % screenHeight);

        // origin is located at the center of the screen, so get random value to
        // determine if point is located before or after origin
        int invertX = (rand() % 2);
        int invertY = (rand() % 2);

        // set star position
        starfield[i].m_X = (invertX ? (randX / 2.0f) : -(randX / 2.0f)) / 1000.0f;
        starfield[i].m_Y = (invertY ? (randY / 2.0f) : -(randY / 2.0f)) / 1000.0f;
        starfield[i].m_Z = (rand() % (int)M_StarDepth);
    }
}
//------------------------------------------------------------------------------
void ApplyOrtho(float maxX, float maxY) const
{
    float a         = 1.0f / maxX;
    float b         = 1.0f / maxY;
    float ortho[16] =
    {
        a, 0,  0,    0,
        0, b,  0,    0,
        0, 0, -1.0f, 0,
        0, 0,  0,    1.0f
    };

    GLint projectionUniform = glGetUniformLocation(simpleProgram, "Projection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho[0]);
}
//------------------------------------------------------------------------------
GLuint BuildShader(const char* pSource, GLenum shaderType) const
{
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &pSource, 0);
    glCompileShader(shaderHandle);

    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

    if (compileSuccess == GL_FALSE)
    {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        printf("compile glsl error : %s\n", messages);
    }

    return shaderHandle;
}
//------------------------------------------------------------------------------
GLuint BuildProgram(const char* pVShader,const char* pFShader) const
{
    GLuint vertexShader   = BuildShader(pVShader, GL_VERTEX_SHADER);
    GLuint fragmentShader = BuildShader(pFShader, GL_FRAGMENT_SHADER);

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);

    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);

    if (linkSuccess == GL_FALSE)
    {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        printf("compile glsl error : %s\n",messages);
    }

    return programHandle;
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    // renderer buffers should no more be generated since CCR version 1.1
    #if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
        #ifndef ANDROID
            glGenRenderbuffers(1, &renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
            glGenFramebuffers(1,&framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_RENDERBUFFER, renderbuffer);
        #endif
    #endif

    InitializeStarfield();

    simpleProgram = BuildProgram(pSimpleVertexShader, pSimpleFragmentShader);
    glUseProgram(simpleProgram);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    if (simpleProgram)
        glDeleteProgram(simpleProgram);

    simpleProgram = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
    ApplyOrtho(2, 3);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float modelViewMatrix[16] =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    GLint modelviewUniform = glGetUniformLocation(simpleProgram, "Modelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix[0]);

    GLuint positionSlot = glGetAttribLocation(simpleProgram, "Position");
    GLuint colorSlot    = glGetAttribLocation(simpleProgram, "SourceColor");

    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);

    float focal = 100.0f;

    for (int i = 0; i < M_Nb_Star; ++i)
    {
        // check if z position is out of bounds
        if (starfield[i].m_Z > 1.0f)
            // change star position
            starfield[i].m_Z -= 20.0f;
        else
            // reinitialize star position
            starfield[i].m_Z = M_StarDepth;

        QR_Vector2 starPos;

        // calculate star position from his 3d coordinate to a 2d coordinate
        starPos.m_X = focal * (starfield[i].m_X / (starfield[i].m_Z + focal));
        starPos.m_Y = focal * (starfield[i].m_Y / (starfield[i].m_Z + focal));

        QR_Vertex starVertices[6];

        // iterate through vertices
        for (int j = 0; j < 6; ++j)
        {
            // calculate position on screen
            starVertices[j].m_Position[0] = (starPos.m_X * 10.0f) + vertices[j].m_Position[0];
            starVertices[j].m_Position[1] = (starPos.m_Y * 10.0f) + vertices[j].m_Position[1];

            // calculate star color
            starVertices[j].m_Color[0] = 1.0f - (starfield[i].m_Z / M_StarDepth);
            starVertices[j].m_Color[1] = 1.0f - (starfield[i].m_Z / M_StarDepth);
            starVertices[j].m_Color[2] = 1.0f - (starfield[i].m_Z / M_StarDepth);
            starVertices[j].m_Color[3] = 1.0f;
        }

        GLsizei       stride  = sizeof(QR_Vertex);
        const GLvoid* pCoords = &starVertices[0].m_Position[0];
        const GLvoid* pColors = &starVertices[0].m_Color[0];

        glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, pCoords);
        glVertexAttribPointer(colorSlot,    4, GL_FLOAT, GL_FLOAT, stride, pColors);

        GLsizei vertexCount = sizeof(starVertices) / sizeof(QR_Vertex);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x,float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x,float prev_y,float x, float y)
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
