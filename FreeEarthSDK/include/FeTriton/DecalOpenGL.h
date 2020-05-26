// Copyright (c) 2013-2014 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_DECAL_OPENGL_H
#define TRITON_DECAL_OPENGL_H

/** \file DecalOpenGL.h
\brief OpenGL implementation of a decal texture applied to the dynamic ocean surface.
*/

#include <FeTriton/Decal.h>
#include <FeTriton/TritonOpenGL.h>

namespace Triton
{
/** OpenGL specific implementation of Decal. */
class DecalOpenGL : public Decal
{
public:

    DecalOpenGL(const Environment *env, TextureHandle pTexture, float pSize);

    virtual ~DecalOpenGL();

    virtual void Setup();

    virtual void Draw(bool inside);

    virtual void BindArrays();

    virtual void UnbindArrays();

private:

    GLuint vboID, idxID, vertexArray;
};
}
#endif
