/****************************************************************************
 * ==> Raindrop char demo --------------------------------------------------*
 ****************************************************************************
 * Description : A raindrop character demo                                  *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2021, this file is part of the CompactStar Engine.  *
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// opengl
#include <gles2.h>
#include <gles2ext.h>

// compactStar engine
#include "SDK/CSR_Common.h"
#include "SDK/CSR_Geometry.h"
#include "SDK/CSR_Collision.h"
#include "SDK/CSR_Vertex.h"
#include "SDK/CSR_Model.h"
#include "SDK/CSR_Mdl.h"
#include "SDK/CSR_Renderer.h"
#include "SDK/CSR_Renderer_OpenGL.h"
#include "SDK/CSR_Scene.h"
#include "SDK/CSR_AI.h"
#include "SDK/CSR_Sound.h"

#include <ccr.h>

// resource files to load
#define MDL_FILE      "Resources/RaindropChar_Walking.mdl"
#define SKYBOX_LEFT   "Resources/skybox_left_small.bmp"
#define SKYBOX_TOP    "Resources/skybox_top_small.bmp"
#define SKYBOX_RIGHT  "Resources/skybox_right_small.bmp"
#define SKYBOX_BOTTOM "Resources/skybox_bottom_small.bmp"
#define SKYBOX_FRONT  "Resources/skybox_front_small.bmp"
#define SKYBOX_BACK   "Resources/skybox_back_small.bmp"

//----------------------------------------------------------------------------
const char* pCubemapFileNames[6] =
{
    SKYBOX_RIGHT,
    SKYBOX_LEFT,
    SKYBOX_TOP,
    SKYBOX_BOTTOM,
    SKYBOX_FRONT,
    SKYBOX_BACK
};
//------------------------------------------------------------------------------
const char g_VSColored[] =
    "precision mediump float;"
    "const float air = 1.0;"
    "const float water = 0.25;"//1.3333;"//1.51714;"
    "const float eta = air / water;"
    "const float r0 = ((air - water) * (air - water)) / ((air + water) * (air + water));"
    "attribute vec3 csr_aVertices;"
    "attribute vec3 csr_aNormal;"
    "uniform   float csr_uAlpha;"
    "uniform mat4 csr_uProjection;"
    "uniform mat4 csr_uView;"
    "uniform mat4 csr_uModel;"
    "uniform vec4 csr_uCamera;"
    "varying   vec3 csr_vRefraction;"
    "varying   vec3 csr_vReflection;"
    "varying   float csr_fFresnel;"
    "varying   float csr_fAlpha;"
    "void main()"
    "{"
    "    vec4 vertex = csr_uModel * vec4(csr_aVertices, 1.0);"
    "    vec3 incident = normalize(vec3(vertex - csr_uCamera));"
    "    vec3 normal = mat3(csr_uModel) * csr_aNormal;"
    "    csr_vRefraction = refract(incident, normal, eta);"
    "    csr_vReflection = reflect(incident, normal);"
    "    csr_fFresnel = r0 + (1.0 - r0) * pow((1.0 - dot(-incident, normal)), 5.0);"
    "    csr_fAlpha  = csr_uAlpha;"
    "    gl_Position = csr_uProjection * csr_uView * vertex;"
    "}";
//----------------------------------------------------------------------------
const char g_FSColored[] =
    "precision mediump float;"
    "uniform samplerCube csr_sCubemap;"
    "varying vec3        csr_vRefraction;"
    "varying vec3        csr_vReflection;"
    "varying float       csr_fFresnel;"
    "varying float       csr_fAlpha;"
    "void main()"
    "{"
    "    vec4 refractionColor = textureCube(csr_sCubemap, normalize(csr_vRefraction));"
    "    vec4 reflectionColor = textureCube(csr_sCubemap, normalize(csr_vReflection));"
    "    vec4 finalColor      = mix(refractionColor, reflectionColor, csr_fFresnel);"
    "    gl_FragColor         = vec4(finalColor.xyz, finalColor.w * csr_fAlpha);"
    "}";
//------------------------------------------------------------------------------
const char g_VSSkybox[] =
    "precision mediump float;"
    "attribute vec3 csr_aVertices;"
    "uniform   mat4 csr_uProjection;"
    "uniform   mat4 csr_uView;"
    "varying   vec3 csr_vTexCoord;"
    "void main()"
    "{"
    "    csr_vTexCoord = csr_aVertices;"
    "    gl_Position   = csr_uProjection * csr_uView * vec4(csr_aVertices, 1.0);"
    "}";
//------------------------------------------------------------------------------
const char g_FSSkybox[] =
    "precision mediump float;"
    "uniform samplerCube csr_sCubemap;"
    "varying vec3        csr_vTexCoord;"
    "void main()"
    "{"
    "    gl_FragColor = textureCube(csr_sCubemap, csr_vTexCoord);"
    "}";
//------------------------------------------------------------------------------
CSR_Scene*        g_pScene          = 0;
CSR_OpenGLShader* g_pShader         = 0;
CSR_OpenGLShader* g_pSkyboxShader   = 0;
CSR_MDL*          g_pCharModel;
CSR_SceneContext  g_SceneContext;
CSR_Matrix4       g_CharMatrix;
GLint             g_CameraSlot      = 0;
GLint             g_AlphaSlot       = 0;
size_t            g_AnimIndex       = 0;
size_t            g_TextureIndex    = 0;
size_t            g_ModelIndex      = 0;
size_t            g_MeshIndex       = 0;
size_t            g_FrameCount      = 0;
size_t            g_FPS             = 20;
float             g_Angle           = 0.0f;
double            g_TextureLastTime = 0.0;
double            g_ModelLastTime   = 0.0;
double            g_MeshLastTime    = 0.0;
CSR_OpenGLID      g_ID[2];
//---------------------------------------------------------------------------
void* OnGetShader(const void* pModel, CSR_EModelType type)
{
    if (pModel == g_pScene->m_pSkybox)
        return g_pSkyboxShader;

    csrShaderEnable(g_pShader);

    if (g_AlphaSlot)
        glUniform1f(g_AlphaSlot, 0.8f);

    return g_pShader;
}
//---------------------------------------------------------------------------
void* OnGetID(const void* pKey)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < 1; ++i)
        // found the texture to get?
        if (pKey == g_ID[i].m_pKey){
        	printf("Key - %d\n", i);
            return &g_ID[i];}

    return 0;
}
//---------------------------------------------------------------------------
void OnDeleteTexture(const CSR_Texture* pTexture)
{
    size_t i;

    // iterate through resource ids
    for (i = 0; i < 2; ++i)
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
//---------------------------------------------------------------------------
void OnGetMDLIndex(const CSR_MDL* pMDL,
                         size_t*  pSkinIndex,
                         size_t*  pModelIndex,
                         size_t*  pMeshIndex)
{
    *pSkinIndex  = g_TextureIndex;
    *pModelIndex = g_ModelIndex;
    *pMeshIndex  = g_MeshIndex;
}
//---------------------------------------------------------------------------
void BuildCharMatrix(CSR_Matrix4* pMatrix)
{
    CSR_Vector3 axis;
    CSR_Vector3 factor;
    CSR_Matrix4 rotateXMatrix;
    CSR_Matrix4 rotateYMatrix;
    CSR_Matrix4 scaleMatrix;
    CSR_Matrix4 intermediateMatrix;
    float       normalMat[3][3];

    csrMat4Identity(pMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // create the rotation matrix
    csrMat4Rotate(-((float)M_PI / 2.0), &axis, &rotateXMatrix);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    // create the rotation matrix
    csrMat4Rotate((float)(-M_PI / 4.0) + g_Angle, &axis, &rotateYMatrix);

    // set scale factor
    factor.m_X = 0.4f;
    factor.m_Y = 0.4f;
    factor.m_Z = 0.4f;

    // create the scale matrix
    csrMat4Scale(&factor, &scaleMatrix);

    // build the model matrix
    csrMat4Multiply(&scaleMatrix,        &rotateXMatrix, &intermediateMatrix);
    csrMat4Multiply(&intermediateMatrix, &rotateYMatrix, pMatrix);

    pMatrix->m_Table[3][0] =  0.1f;
    pMatrix->m_Table[3][1] =  0.5f;
    pMatrix->m_Table[3][2] = -2.0f;
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
    GLint projectionSlot = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uProjection");
    glUniformMatrix4fv(projectionSlot, 1, 0, &g_pScene->m_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    CSR_VertexFormat  vertexFormat;
    CSR_VertexCulling vertexCulling;
    CSR_Material      material;
    CSR_Vector3       camera;

    // initialize the scene
    g_pScene = csrSceneCreate();

    // configure the scene background color
    g_pScene->m_Color.m_R = 0.08f;
    g_pScene->m_Color.m_G = 0.05f;
    g_pScene->m_Color.m_B = 0.25f;
    g_pScene->m_Color.m_A = 1.0f;

    // configure the scene view matrix
    csrMat4Identity(&g_pScene->m_ViewMatrix);

    // configure the scene context
    csrSceneContextInit(&g_SceneContext);
    g_SceneContext.m_fOnGetShader     = OnGetShader;
    g_SceneContext.m_fOnGetID         = OnGetID;
    g_SceneContext.m_fOnGetMDLIndex   = OnGetMDLIndex;
    g_SceneContext.m_fOnDeleteTexture = OnDeleteTexture;

    // compile, link and use shader
    g_pShader = csrOpenGLShaderLoadFromStr(&g_VSColored[0],
                                            sizeof(g_VSColored),
                                           &g_FSColored[0],
                                            sizeof(g_FSColored),
                                            0,
                                            0);

    // succeeded?
    if (!g_pShader)
    {
    	printf("FAILED to compile shader\n");
        return;
    }

    csrShaderEnable(g_pShader);

    // get shader attributes
    g_pShader->m_VertexSlot  = glGetAttribLocation (g_pShader->m_ProgramID, "csr_aVertices");
    g_pShader->m_NormalSlot  = glGetAttribLocation (g_pShader->m_ProgramID, "csr_aNormal");
    g_pShader->m_CubemapSlot = glGetUniformLocation(g_pShader->m_ProgramID, "csr_sCubemap");
    g_CameraSlot             = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uCamera");
    g_AlphaSlot              = glGetUniformLocation(g_pShader->m_ProgramID, "csr_uAlpha");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    BuildCharMatrix(&g_CharMatrix);

    // extract the camera position from the model matrix
    camera.m_X = g_CharMatrix.m_Table[3][0];
    camera.m_Y = g_CharMatrix.m_Table[3][1];
    camera.m_Z = g_CharMatrix.m_Table[3][2];

    // todo FIXME -cCheck -oJean: What's that? How it works?
    //glusMatrix4x4ExtractMatrix3x3f(&normalMat, pMatrix);

    // link the camera with the matrix
    glUniform3fv(g_CameraSlot, 1, &camera);

    // configure the vertex format
    vertexFormat.m_HasNormal         = 1;
    vertexFormat.m_HasTexCoords      = 0;
    vertexFormat.m_HasPerVertexColor = 0;

    vertexCulling.m_Face = CSR_CF_CCW;
    vertexCulling.m_Type = CSR_CT_None;

    // configure the material
    material.m_Color       = 0x0207DEFF;
    material.m_Transparent = 1;
    material.m_Wireframe   = 0;

    /*
    CSR_Mesh* pMesh = csrShapeCreateSphere(1,
                                 20,
                                 20,
                                &vertexFormat,
                                 0,
                                &material,
                                 0);
    CSR_SceneItem* pSceneItem = csrSceneAddMesh(g_pScene, pMesh, 1, 0);
    csrSceneAddModelMatrix(g_pScene, pMesh, &g_CharMatrix);
   */

    // load the MDL model
    g_pCharModel = csrMDLOpen(MDL_FILE,
                              0,
                             &vertexFormat,
                             &vertexCulling,
                             &material,
                              0,
                              0,
                              0);

    // add the model to the scene
    CSR_SceneItem* pSceneItem = csrSceneAddMDL(g_pScene, g_pCharModel, 1, 0);
    csrSceneAddModelMatrix(g_pScene, g_pCharModel, &g_CharMatrix);

    // load the cubemap texture
    g_ID[0].m_pKey     = &g_pCharModel->m_pSkin->m_CubeMap;
    //g_ID[0].m_pKey     = &pMesh->m_Skin.m_CubeMap;
    g_ID[0].m_ID       = csrOpenGLCubemapLoad(pCubemapFileNames);
    g_ID[0].m_UseCount = 1;

    // load the skybox shader
    g_pSkyboxShader = csrOpenGLShaderLoadFromStr(&g_VSSkybox[0],
                                                  sizeof(g_VSSkybox),
                                                 &g_FSSkybox[0],
                                                  sizeof(g_FSSkybox),
                                                  0,
                                                  0);

    // succeeded?
    if (!g_pSkyboxShader)
    {
        // show the error message to the user
        printf("Failed to load the skybox shader.\n");
        return;
    }

    // enable the shader program
    csrShaderEnable(g_pSkyboxShader);

    // get shader attributes
    g_pSkyboxShader->m_VertexSlot  = glGetAttribLocation (g_pSkyboxShader->m_ProgramID, "csr_aVertices");
    g_pSkyboxShader->m_CubemapSlot = glGetUniformLocation(g_pSkyboxShader->m_ProgramID, "csr_sCubemap");

    // create the skybox
    g_pScene->m_pSkybox = csrSkyboxCreate(1.0f, 1.0f, 1.0f);

    // succeeded?
    if (!g_pScene->m_pSkybox)
    {
        // show the error message to the user
        printf("Failed to create the skybox.\n");
        return;
    }

    // load the cubemap texture
    g_ID[1].m_pKey     = &g_pScene->m_pSkybox->m_Skin.m_CubeMap;
    g_ID[1].m_ID       = csrOpenGLCubemapLoad(pCubemapFileNames);
    g_ID[1].m_UseCount = 1;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete the scene
    csrSceneRelease(g_pScene, OnDeleteTexture);
    g_pScene = 0;

    // delete scene shader
    csrOpenGLShaderRelease(g_pShader);
    g_pShader = 0;

    // delete skybox shader
    csrOpenGLShaderRelease(g_pSkyboxShader);
    g_pSkyboxShader = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
    CreateViewport(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // rebuild the bot matrix
    BuildCharMatrix(&g_CharMatrix);

    // calculate next model indexes to show
    csrMDLUpdateIndex(g_pCharModel,
                      (size_t)g_FPS,
                      g_AnimIndex,
                     &g_TextureIndex,
                     &g_ModelIndex,
                     &g_MeshIndex,
                     &g_TextureLastTime,
                     &g_ModelLastTime,
                     &g_MeshLastTime,
                      timeStep_sec);
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
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
	g_Angle += (x - prev_x) * 0.001f;
}
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
