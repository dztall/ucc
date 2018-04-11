/****************************************************************************
 * ==> CSR_Renderer --------------------------------------------------------*
 ****************************************************************************
 * Description : This module provides the draw functions                    *
 * Developer   : Jean-Milost Reymond                                        *
 * Copyright   : 2017 - 2018, this file is part of the CompactStar Engine.  *
 *               You are free to copy or redistribute this file, modify it, *
 *               or use it for your own projects, commercial or not. This   *
 *               file is provided "as is", WITHOUT ANY WARRANTY OF ANY      *
 *               KIND. THE DEVELOPER IS NOT RESPONSIBLE FOR ANY DAMAGE OF   *
 *               ANY KIND, ANY LOSS OF DATA, OR ANY LOSS OF PRODUCTIVITY    *
 *               TIME THAT MAY RESULT FROM THE USAGE OF THIS SOURCE CODE,   *
 *               DIRECTLY OR NOT.                                           *
 ****************************************************************************/

#include "CSR_Renderer.h"

// std
#include <stdlib.h>

//---------------------------------------------------------------------------
// Multisample antialiasing shader
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    const char g_MSAA_VertexProgram[] =
        "attribute vec2 csr_vVertex;"
        "attribute vec2 csr_vTexCoord;"
        "varying   vec2 csr_fTexCoord;"
        "void main()"
        "{"
        "    gl_Position   = vec4(csr_vVertex.x, csr_vVertex.y, 0.0f, 1.0f);"
        "    csr_fTexCoord = csr_vTexCoord;"
        "}";
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    const char g_MSAA_FragmentProgram[] =
        "uniform sampler2D csr_sTexture;"
        "varying vec2      csr_fTexCoord;"
        "void main()"
        "{"
        "    gl_FragColor = texture(csr_sTexture, csr_fTexCoord);"
        "}";
#endif
//---------------------------------------------------------------------------
// Multisample antialiasing constants
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    const float g_MSAA_VB[] =
    {
        // x,   y,    tu,   tv
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f
    };
#endif
//---------------------------------------------------------------------------
// Multisample antialiasing private functions
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrMSAALinkStaticVB(const CSR_Shader* pShader, const void* pCustomData)
    {
        CSR_Buffer buffer;

        // get the multisampling antialiasing sent in custom data
        CSR_MSAA* pMSAA = (CSR_MSAA*)pCustomData;

        // found it?
        if (!pMSAA)
            return;

        // configure the vertex buffer
        buffer.m_pData  = g_MSAA_VB;
        buffer.m_Length = sizeof(g_MSAA_VB);

        // create the shape on which the final texture will be drawn
        pMSAA->m_pStaticBuffer           = csrStaticBufferCreate(pShader, &buffer);
        pMSAA->m_pStaticBuffer->m_Stride = 4;
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    int csrMSAAConfigure(size_t width, size_t height, size_t factor, CSR_MSAA* pMSAA)
    {
        GLuint msTexture;
        GLuint texture;

        // enable multisampling
        glEnable(GL_MULTISAMPLE);

        // create and bind the frame buffer
        glGenFramebuffers(1, &pMSAA->m_FrameBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, pMSAA->m_FrameBufferID);

        // create a new texture
        glGenTextures(1, &msTexture);

        // bind texture to sampler
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, factor, GL_RGB, width, height, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

        // add texture to the frame buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D_MULTISAMPLE,
                               msTexture,
                               0);

        // create and bind the render buffer for depth and stencil attachments
        glGenRenderbuffers(1, &pMSAA->m_RenderBufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, pMSAA->m_RenderBufferID);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                         factor,
                                         GL_DEPTH24_STENCIL8,
                                         width,
                                         height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  pMSAA->m_RenderBufferID);

        // build the render buffer
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return 0;

        // unbind the frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // create multisampled output texture
        glGenTextures(1, &pMSAA->m_TextureID);
        glBindTexture(GL_TEXTURE_2D, pMSAA->m_TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

        // configure texture filters to use and bind texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // create and bind a multisampled texture buffer
        glGenFramebuffers(1, &pMSAA->m_TextureBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, pMSAA->m_TextureBufferID);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               pMSAA->m_TextureID,
                               0);

        // build the multisampled texture buffer
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return 0;

        // unbind the texture buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // set the viewport size
        pMSAA->m_Width  = width;
        pMSAA->m_Height = height;
        pMSAA->m_Factor = factor;

        return 1;
    }
#endif
//---------------------------------------------------------------------------
// Multisample antialiasing functions
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    CSR_MSAA* csrMSAACreate(size_t width, size_t height, size_t factor)
    {
        // create a new multisample antialiasing
        CSR_MSAA* pMSAA = (CSR_MSAA*)malloc(sizeof(CSR_MSAA));

        // succeeded?
        if (!pMSAA)
            return 0;

        // initialize the multisample antialiasing content
        if (!csrMSAAInit(width, height, factor, pMSAA))
        {
            csrMSAARelease(pMSAA);
            return 0;
        }

        return pMSAA;
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrMSAARelease(CSR_MSAA* pMSAA)
    {
        // no multisampling antialiasing to release?
        if (!pMSAA)
            return;

        // enable the MSAA shader
        if (pMSAA->m_pShader)
            csrShaderEnable(pMSAA->m_pShader);

        // delete the multisampled texture
        if (pMSAA->m_TextureID)
            glDeleteTextures(1, &pMSAA->m_TextureID);

        // delete the multisampled texture buffer
        if (pMSAA->m_TextureBufferID)
            glDeleteFramebuffers(1, &pMSAA->m_TextureBufferID);

        // delete the render buffer
        if (pMSAA->m_RenderBufferID)
            glDeleteRenderbuffers(1, &pMSAA->m_RenderBufferID);

        // delete the frame buffer
        if (pMSAA->m_FrameBufferID)
            glDeleteFramebuffers(1, &pMSAA->m_FrameBufferID);

        // delete the shader
        if (pMSAA->m_pShader)
            csrShaderRelease(pMSAA->m_pShader);

        // delete the static buffer
        if (pMSAA->m_pStaticBuffer)
            csrStaticBufferRelease(pMSAA->m_pStaticBuffer);

        // delete the multisampling antialiasing
        free(pMSAA);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    int csrMSAAInit(size_t width, size_t height, size_t factor, CSR_MSAA* pMSAA)
    {
        // no multisample antialiasing to initialize?
        if (!pMSAA)
            return 0;

        // validate input
        if (factor != 2 && factor != 4 && factor != 8)
            return 0;

        // load the shader to use for MSAA
        pMSAA->m_pShader = csrShaderLoadFromStr(g_MSAA_VertexProgram,
                                                sizeof(g_MSAA_VertexProgram),
                                                g_MSAA_FragmentProgram,
                                                sizeof(g_MSAA_FragmentProgram),
                                                csrMSAALinkStaticVB,
                                                pMSAA);

        // succeeded?
        if (!pMSAA->m_pShader)
            return 0;

        // the static buffer containing the final surface to show was generated successfully?
        if (!pMSAA->m_pStaticBuffer)
            return 0;

        // enable the MSAA shader
        csrShaderEnable(pMSAA->m_pShader);

        // get the vertex slot
        pMSAA->m_pShader->m_VertexSlot =
                glGetAttribLocation(pMSAA->m_pShader->m_ProgramID, "csr_vVertex");

        // found it?
        if (pMSAA->m_pShader->m_VertexSlot == -1)
            return 0;

        // get the texture coordinates slot
        pMSAA->m_pShader->m_TexCoordSlot =
                glGetAttribLocation(pMSAA->m_pShader->m_ProgramID, "csr_vTexCoord");

        // found it?
        if (pMSAA->m_pShader->m_TexCoordSlot == -1)
            return 0;

        // get the texture sampler slot
        pMSAA->m_pShader->m_TextureSlot =
                glGetUniformLocation(pMSAA->m_pShader->m_ProgramID, "csr_sTexture");

        // found it?
        if (pMSAA->m_pShader->m_TextureSlot == -1)
            return 0;

        // configure the multisample antialiasing
        return csrMSAAConfigure(width, height, factor, pMSAA);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    int csrMSAAChangeSize(size_t width, size_t height, CSR_MSAA* pMSAA)
    {
        // validate the input
        if (!pMSAA)
            return 0;

        // delete the multisampled texture
        if (pMSAA->m_TextureID)
        {
            glDeleteTextures(1, &pMSAA->m_TextureID);
            pMSAA->m_TextureID = M_CSR_Error_Code;
        }

        // delete the multisampled texture buffer
        if (pMSAA->m_TextureBufferID)
        {
            glDeleteFramebuffers(1, &pMSAA->m_TextureBufferID);
            pMSAA->m_TextureBufferID = M_CSR_Error_Code;
        }

        // delete the render buffer
        if (pMSAA->m_RenderBufferID)
        {
            glDeleteRenderbuffers(1, &pMSAA->m_RenderBufferID);
            pMSAA->m_RenderBufferID = M_CSR_Error_Code;
        }

        // delete the frame buffer
        if (pMSAA->m_FrameBufferID)
        {
            glDeleteFramebuffers(1, &pMSAA->m_FrameBufferID);
            pMSAA->m_FrameBufferID = M_CSR_Error_Code;
        }

        // recreate the multisample antialiasing
        return csrMSAAConfigure(width, height, pMSAA->m_Factor, pMSAA);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrMSAADrawBegin(const CSR_Color* pColor, const CSR_MSAA* pMSAA)
    {
        // do apply a multisample antialiasing?
        if (pMSAA && pMSAA->m_pShader)
        {
            // enable the MSAA shader
            csrShaderEnable(pMSAA->m_pShader);

            // enable multisampling
            glEnable(GL_MULTISAMPLE);

            // bind the frame buffer on which the final rendering should be performed
            glBindFramebuffer(GL_FRAMEBUFFER, pMSAA->m_FrameBufferID);
        }

        // begin the draw
        csrDrawBegin(pColor);
    }
#endif
//---------------------------------------------------------------------------
#ifndef CSR_OPENGL_2_ONLY
    void csrMSAADrawEnd(const CSR_MSAA* pMSAA)
    {
        // end the draw
        csrDrawEnd();

        // do finalize the multisampling antialiasing effect?
        if (pMSAA && pMSAA->m_pShader && pMSAA->m_pStaticBuffer)
        {
            // configure the culling
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glFrontFace(GL_CW);

            // disable the alpha blending
            glDisable(GL_BLEND);

            // set polygon mode to fill
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // enable the MSAA shader
            csrShaderEnable(pMSAA->m_pShader);

            // blit the multisampled buffer containing the drawing to the output texture buffer
            glBindFramebuffer(GL_READ_FRAMEBUFFER, pMSAA->m_FrameBufferID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pMSAA->m_TextureBufferID);
            glBlitFramebuffer(0,
                              0,
                              pMSAA->m_Width,
                              pMSAA->m_Height,
                              0,
                              0,
                              pMSAA->m_Width,
                              pMSAA->m_Height,
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);

            // unbind the frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // configure the depth testing
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            // select the texture sampler to use (GL_TEXTURE0 for normal textures)
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(pMSAA->m_pShader->m_TextureSlot, GL_TEXTURE0);

            // bind the texure to use
            glBindTexture(GL_TEXTURE_2D, pMSAA->m_TextureID);

            // bind the VBO containing the shape to draw
            glBindBuffer(GL_ARRAY_BUFFER, pMSAA->m_pStaticBuffer->m_BufferID);

            // enable the vertices
            glEnableVertexAttribArray(pMSAA->m_pShader->m_VertexSlot);
            glVertexAttribPointer(pMSAA->m_pShader->m_VertexSlot,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  pMSAA->m_pStaticBuffer->m_Stride * sizeof(float),
                                  0);

            // enable the texture coordinates
            glEnableVertexAttribArray(pMSAA->m_pShader->m_TexCoordSlot);
            glVertexAttribPointer(pMSAA->m_pShader->m_TexCoordSlot,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  pMSAA->m_pStaticBuffer->m_Stride * sizeof(float),
                                  (void*)(2 * sizeof(float)));

            // draw the surface
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // disable the vertex attribute arrays
            glDisableVertexAttribArray(pMSAA->m_pShader->m_TexCoordSlot);
            glDisableVertexAttribArray(pMSAA->m_pShader->m_VertexSlot);

            // unbind the VBO
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
#endif
//---------------------------------------------------------------------------
// Draw private functions
//---------------------------------------------------------------------------
void csrDrawArray(const CSR_VertexBuffer* pVB, size_t vertexCount)
{
    // search for array type to draw
    switch (pVB->m_Format.m_Type)
    {
        case CSR_VT_Triangles:     glDrawArrays(GL_TRIANGLES,      0, vertexCount); break;
        case CSR_VT_TriangleStrip: glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount); break;
        case CSR_VT_TriangleFan:   glDrawArrays(GL_TRIANGLE_FAN,   0, vertexCount); break;
    }
}
//---------------------------------------------------------------------------
// Draw functions
//---------------------------------------------------------------------------
void csrDrawBegin(const CSR_Color* pColor)
{
    // no background color?
    if (!pColor)
        return;

    // clear background and depth buffer
    glClearColor(pColor->m_R, pColor->m_G, pColor->m_B, pColor->m_A);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // configure the OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);
}
//---------------------------------------------------------------------------
void csrDrawEnd(void)
{}
//---------------------------------------------------------------------------
void csrDrawVertexBuffer(const CSR_VertexBuffer* pVB,
                         const CSR_Shader*       pShader,
                         const CSR_Array*        pMatrixArray)
{
    GLvoid* pCoords;
    GLvoid* pNormals;
    GLvoid* pTexCoords;
    GLvoid* pColors;
    size_t  i;
    size_t  offset;
    size_t  vertexCount;

    // no vertex buffer to draw?
    if (!pVB)
        return;

    // no shader?
    if (!pShader)
        return;

    // check if vertex buffer is empty, skip to next if yes
    if (!pVB->m_Count || !pVB->m_Format.m_Stride)
        return;

    // configure the culling
    switch (pVB->m_Culling.m_Type)
    {
        case CSR_CT_None:  glDisable(GL_CULL_FACE); glCullFace(GL_NONE);           break;
        case CSR_CT_Front: glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT);          break;
        case CSR_CT_Back:  glEnable(GL_CULL_FACE);  glCullFace(GL_BACK);           break;
        case CSR_CT_Both:  glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT_AND_BACK); break;
        default:           glDisable(GL_CULL_FACE); glCullFace(GL_NONE);           break;
    }

    // configure the culling face
    switch (pVB->m_Culling.m_Face)
    {
        case CSR_CF_CW:  glFrontFace(GL_CW);  break;
        case CSR_CF_CCW: glFrontFace(GL_CCW); break;
    }

    // configure the alpha blending
    if (pVB->m_Material.m_Transparent)
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    // configure the wireframe mode
    #ifndef CSR_OPENGL_2_ONLY
        if (pVB->m_Material.m_Wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    #endif

    // enable vertex slot
    glEnableVertexAttribArray(pShader->m_VertexSlot);

    // enable normal slot
    if (pVB->m_Format.m_HasNormal)
        glEnableVertexAttribArray(pShader->m_NormalSlot);

    // enable texture slot
    if (pVB->m_Format.m_HasTexCoords)
        glEnableVertexAttribArray(pShader->m_TexCoordSlot);

    // enable color slot
    if (pVB->m_Format.m_HasPerVertexColor)
        glEnableVertexAttribArray(pShader->m_ColorSlot);

    offset = 0;

    // send vertices to shader
    pCoords = &pVB->m_pData[offset];
    glVertexAttribPointer(pShader->m_VertexSlot,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          pVB->m_Format.m_Stride * sizeof(float),
                          pCoords);

    offset += 3;

    // vertices have normals?
    if (pVB->m_Format.m_HasNormal)
    {
        // send normals to shader
        pNormals = &pVB->m_pData[offset];
        glVertexAttribPointer(pShader->m_TexCoordSlot,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              pVB->m_Format.m_Stride * sizeof(float),
                              pNormals);

        offset += 3;
    }

    // vertices have UV texture coordinates?
    if (pVB->m_Format.m_HasTexCoords)
    {
        // send textures to shader
        pTexCoords = &pVB->m_pData[offset];
        glVertexAttribPointer(pShader->m_TexCoordSlot,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              pVB->m_Format.m_Stride * sizeof(float),
                              pTexCoords);

        offset += 2;
    }

    // vertices have per-vertex color?
    if (pVB->m_Format.m_HasPerVertexColor)
    {
        // send colors to shader
        pColors = &pVB->m_pData[offset];
        glVertexAttribPointer(pShader->m_ColorSlot,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              pVB->m_Format.m_Stride * sizeof(float),
                              pColors);
    }
    else
    if (pShader->m_ColorSlot != -1)
    {
        // get the color component values
        const float r = (float)((pVB->m_Material.m_Color >> 24) & 0xFF) / 255.0f;
        const float g = (float)((pVB->m_Material.m_Color >> 16) & 0xFF) / 255.0f;
        const float b = (float)((pVB->m_Material.m_Color >> 8)  & 0xFF) / 255.0f;
        const float a = (float) (pVB->m_Material.m_Color        & 0xFF) / 255.0f;

        // connect the vertex color to the shader
        glVertexAttrib4f(pShader->m_ColorSlot, r, g, b, a);
    }

    // calculate the vertex count
    vertexCount = pVB->m_Count / pVB->m_Format.m_Stride;

    // do draw the vertex buffer several times?
    if (pMatrixArray && pMatrixArray->m_Count)
        // yes, iterate through each matrix to use to draw the vertex buffer
        for (i = 0; i < pMatrixArray->m_Count; ++i)
        {
            // get the model matrix slot from shader
            const GLint slot = glGetUniformLocation(pShader->m_ProgramID, "csr_uModel");

            // found it?
            if (slot < 0)
                continue;

            // connect the model matrix to the shader
            glUniformMatrix4fv(slot, 1, 0, &((CSR_Matrix4*)pMatrixArray->m_pItem[i].m_pData)->m_Table[0][0]);

            // draw the next buffer
            csrDrawArray(pVB, vertexCount);
        }
    else
        // no, draw the buffer
        csrDrawArray(pVB, vertexCount);

    // disable vertices slots from shader
    glDisableVertexAttribArray(pShader->m_VertexSlot);

    // disable normal slot
    if (pVB->m_Format.m_HasNormal)
        glDisableVertexAttribArray(pShader->m_NormalSlot);

    // disable texture slot
    if (pVB->m_Format.m_HasTexCoords)
        glDisableVertexAttribArray(pShader->m_TexCoordSlot);

    // disable color slot
    if (pVB->m_Format.m_HasPerVertexColor)
        glDisableVertexAttribArray(pShader->m_ColorSlot);
}
//---------------------------------------------------------------------------
void csrDrawMesh(const CSR_Mesh*   pMesh,
                 const CSR_Shader* pShader,
                 const CSR_Array*  pMatrixArray)
{
    size_t i;

    // no mesh to draw?
    if (!pMesh)
        return;

    // no shader?
    if (!pShader)
        return;

    // enable the shader to use for drawing
    csrShaderEnable(pShader);

    // iterate through the vertex buffers composing the mesh to draw
    for (i = 0; i < pMesh->m_Count; ++i)
    {
        // vertices have UV texture coordinates?
        if (pMesh->m_pVB[i].m_Format.m_HasTexCoords)
        {
            // a texture is defined for this mesh?
            if (pMesh->m_Shader.m_TextureID != M_CSR_Error_Code)
            {
                // select the texture sampler to use (GL_TEXTURE0 for normal textures)
                glActiveTexture(GL_TEXTURE0);
                glUniform1i(pShader->m_TextureSlot, GL_TEXTURE0);

                // bind the texure to use
                glBindTexture(GL_TEXTURE_2D, pMesh->m_Shader.m_TextureID);
            }

            // a bump map is defined for this mesh?
            if (pMesh->m_Shader.m_BumpMapID != M_CSR_Error_Code)
            {
                // select the texture sampler to use (GL_TEXTURE1 for bump map textures)
                glActiveTexture(GL_TEXTURE1);
                glUniform1i(pShader->m_BumpMapSlot, GL_TEXTURE1);

                // bind the texure to use
                glBindTexture(GL_TEXTURE_2D, pMesh->m_Shader.m_BumpMapID);
            }
        }

        // draw the next mesh vertex buffer
        csrDrawVertexBuffer(&pMesh->m_pVB[i], pShader, pMatrixArray);
    }
}
//---------------------------------------------------------------------------
void csrDrawModel(const CSR_Model*  pModel,
                        size_t      index,
                  const CSR_Shader* pShader,
                  const CSR_Array*  pMatrixArray)
{
    // no model to draw?
    if (!pModel)
        return;

    // draw the model mesh
    csrDrawMesh(&pModel->m_pMesh[index % pModel->m_MeshCount], pShader, pMatrixArray);
}
//---------------------------------------------------------------------------
void csrDrawMDL(const CSR_MDL*    pMDL,
                const CSR_Shader* pShader,
                const CSR_Array*  pMatrixArray,
                      size_t      textureIndex,
                      size_t      modelIndex,
                      size_t      meshIndex)
{
    // get the current model mesh to draw
    const CSR_Mesh* pMesh = csrMDLGetMesh(pMDL, modelIndex, meshIndex);

    // found it?
    if (!pMesh)
        return;

    // normally each mesh should contain only one vertex buffer
    if (pMesh->m_Count != 1)
        // unsupported if not (because cannot know which texture should be binded. If a such model
        // exists, a custom version of this function should also be written for it)
        return;

    // can use texture?
    if (pMesh->m_pVB->m_Format.m_HasTexCoords && textureIndex < pMDL->m_TextureCount &&
            pMDL->m_pTexture[textureIndex].m_TextureID != M_CSR_Error_Code)
        {
            // select the texture sampler to use (GL_TEXTURE0 for normal textures)
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(pShader->m_TextureSlot, GL_TEXTURE0);

            // bind the texure to use
            glBindTexture(GL_TEXTURE_2D, pMDL->m_pTexture[textureIndex].m_TextureID);
        }

    // draw the model mesh
    csrDrawMesh(pMesh, pShader, pMatrixArray);
}
//---------------------------------------------------------------------------
