// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_OCEANLIGHTINGMODELOPENGL_H
#define TRITON_OCEANLIGHTINGMODELOPENGL_H

/** \file OceanLightingModelOpenGL.h
    \brief A specialization of OceanLightingModel for OpenGL.
*/

#include <FeTriton/OceanLightingModel.h>
#include <FeTriton/TritonOpenGL.h>

namespace Triton
{
/** A specialization of OceanLightingModel for OpenGL. */
class OceanLightingModelOpenGL : public OceanLightingModel
{
public:
    OceanLightingModelOpenGL(const Environment *env) : foamTexture(0), lightFoamTexture(0), noiseTexture(0), washTexture(0), breakerTexture(0), OceanLightingModel(env) {}

    virtual ~OceanLightingModelOpenGL();

    virtual bool Initialize(ShaderHandle shader, ShaderHandle patchShader);

    virtual bool PrepShaders();

    virtual bool PrepPatchShader();

    virtual void ReloadGridUniforms(ShaderHandle gridShader, ShaderHandle gridShaderPatch);

private:
    GLint Lloc, lightColorLoc, ambientColorLoc, refractColorLoc, cubeMapLoc, planarReflectionMapLoc;
    GLint shininessLoc, foamScaleLoc, noiseTexLoc, hasEnvMapLoc, hasPlanarReflectionMapLoc;
    GLint planarReflectionDisplacementScaleLoc, foamBlendLoc, washTexLoc;
    GLint LlocPatch,LlocCustomPatch, lightColorLocPatch, ambientColorLocPatch, refractColorLocPatch, cubeMapLocPatch, planarReflectionMapLocPatch;
    GLint shininessLocPatch, foamScaleLocPatch, noiseTexLocPatch, hasEnvMapLocPatch, hasPlanarReflectionMapLocPatch;
    GLint planarReflectionDisplacementScaleLocPatch, foamBlendLocPatch, washTexLocPatch, lightFoamTexLoc, lightFoamTexLocPatch;
    GLint breakerTexLoc, breakerTexLocPatch;
    GLuint foamTexture, lightFoamTexture, noiseTexture, washTexture, breakerTexture;
};
}

#endif