#ifndef TEXTURE_RENDERER_H
#define TEXTURE_RENDERER_H

#include "SilverLiningOpenGL.h"
#include "TextureLoader.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef MAC
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "GL/gl.h"
#include "GL/glu.h"
#endif

#include <GL/glext.h>

#if defined(WIN32) || defined(WIN64)
#include <gl/wglext.h>
#endif

#include <vector>

namespace SilverLining
{
class TextureRenderer
{
public:
    TextureRenderer() {
    }
    virtual ~TextureRenderer() {
    }

    virtual bool Initialize(int w, int h) = 0;
    virtual bool MakeCurrent(bool clear) = 0;
    virtual bool BindToTexture() = 0;
    virtual TextureHandle GetTextureHandle() = 0;
};

class TextureRendererFactory
{
public:
    static TextureRenderer *MakeTextureRenderer();

private:
    static bool SetupPbuffer();
    static bool SetupFramebufferObjects();

    static bool initialized;
    static bool hasFBO, hasPbuffers;
};

class TextureRendererPbuffer : public TextureRenderer
{
public:
    TextureRendererPbuffer();
    virtual ~TextureRendererPbuffer();

    bool Initialize(int w, int h);
    bool MakeCurrent(bool clear);
    bool BindToTexture();
    TextureHandle GetTextureHandle();

private:
    glTexture render_texture;
#if defined(WIN32) || defined(WIN64)
    HPBUFFERARB buffer;
    HDC dc, oldDC;
    HGLRC context, oldContext;
#endif
    bool bound, cleared;
};

class FrameBuffer
{
public:
    FrameBuffer(int w, int h);
    virtual ~FrameBuffer();

    int w, h, nAttachments;
    GLuint fbo;
};

class FramebufferManager
{
public:
    static FramebufferManager *GetInstance();
    virtual ~FramebufferManager();

    bool GetFramebuffer(int w, int h, GLuint& fbo, GLenum& attachment);

private:
    FramebufferManager();
    SL_VECTOR(FrameBuffer*) framebuffers;
    GLint maxAttachments;
};

class TextureRendererFBO : public TextureRenderer
{
public:
    TextureRendererFBO();
    virtual ~TextureRendererFBO();

    bool Initialize(int w, int h);
    bool MakeCurrent(bool clear);
    bool BindToTexture();
    TextureHandle GetTextureHandle();

private:
    GLuint fbo, savedFbo;
    GLenum attachment;
    glTexture render_texture;
};
}

#endif
