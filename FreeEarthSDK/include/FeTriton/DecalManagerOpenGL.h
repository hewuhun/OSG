// Copyright (c) 2013-2015 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_DECAL_MANAGER_OPENGL_H
#define TRITON_DECAL_MANAGER_OPENGL_H

/** \file DecalManagerOpenGL.h
\brief OpenGL implementation of deferred decals
*/

#include <FeTriton/DecalManager.h>
#include <FeTriton/TritonOpenGL.h>
#include <vector>

namespace Triton
{
/** OpenGL specific implementation of DecalManager. */
class DecalManagerOpenGL : public DecalManager
{
public:

    DecalManagerOpenGL(const Environment *env, const TRITON_VECTOR(unsigned int)& userShaders);

    virtual ~DecalManagerOpenGL();

    virtual Decal *CreateDecal(TextureHandle texture, float size);

    virtual bool LoadShaders();

    virtual void DeleteShaders();

    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders);

    virtual void DrawDecals(TextureHandle depthTexture);

    /** Retrieve the underlying shader object for the decals. */
    virtual ShaderHandle GetShader() const {
        return (ShaderHandle)programCast;
    }

private:

    GLhandleARB LoadShader(const char *filename, bool vertexProgram, bool userShader );
    void SetUniforms(Decal *decal);
    void SetupShaders(TextureHandle depthTexture);

    GLhandleARB vertShader, fragShader, userShader, userVertShader, program;
    GLuint vertShaderCast, fragShaderCast, userShaderCast, userVertShaderCast, programCast;
    GLint inverseViewLoc, decalTextureLoc, decalMatrixLoc, depthTextureLoc, mvpLoc;
    GLint inverseViewportLoc, projMatrixLoc, viewportLoc, inverseProjectionLoc, depthRangeLoc;
    GLint depthOffsetLoc, alphaLoc, lightColorLoc, positionLoc, uvOffsetLoc, upLoc;

    TRITON_VECTOR(unsigned int) userShaders;
};
}

#endif