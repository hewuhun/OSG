// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_WAKE_MANAGER_OPENGL_H
#define TRITON_WAKE_MANAGER_OPENGL_H

/** \file WakeManagerOpenGL.h
    \brief OpenGL implementation of ship wakes
*/

#include <FeTriton/TritonCommon.h>
#include <FeTriton/WakeManager.h>
#include <FeTriton/TritonOpenGL.h>
#ifndef _WIN32
#include <stdint.h>
#endif

namespace Triton
{
/** A collection of parameters describing a single circular wave associated with
the underlying OpenGL vertex program. */
class CircularWaveLocsOpenGL : public MemObject
{
public:
    int amplitudeLoc, radiusLoc, kLoc;
    int halfWavelengthLoc, positionLoc;
};

/** A collection of parameters describing a single Kelvin wake segment associated with
the underlying OpenGL vertex program. */
class KelvinWakeLocsOpenGL : public MemObject
{
public:
    int amplitudeLoc, positionLoc;
    int shipPositionLoc, foamAmountLoc;
};

/** A collection of parameters describing a single prop wash segment associated with
the underlying OpenGL vertex program. */
class PropWashLocsOpenGL : public MemObject
{
public:
    int propPositionLoc;
    int washWidthLoc, deltaPosLoc;
    int distFromSourceLoc;
    int washLengthLoc;
    int alphaStartLoc, alphaEndLoc;
};

/** A collection of parameters describing leeward dampening beside a ship. */
class LeewardDampenerLocsOpenGL : public MemObject
{
public:
    int bowPosLoc, sternPosLoc;
    int velocityDampeningLoc;
};

/** An implementation of WakeManager for OpenGL. */
class WakeManagerOpenGL : public WakeManager
{
public:
    WakeManagerOpenGL(double unitScale, ParticleSystemManager *psm, const TRITON_VECTOR(unsigned int)& shaders) : vertShader(0),
        fragShader(0), program(0), doWakesLoc(-1), washLengthLoc(-1), circularWaves(0), patchCircularWaves(0), sprayTexture(0),
        displacementTexture(0), displacementTextureLoc(-1), kelvinWakes(0), patchKelvinWakes(0), propWashes(0),
        patchPropWashes(0), userShaders(shaders), userShader(0), userVertShader(0), doWakesLocPatch(-1), washLengthLocPatch(-1),
        displacementTextureLocPatch(-1), numPropWashesLoc(-1), numPropWashesLocPatch(-1), numCircularWavesLoc(-1),
        numCircularWavesLocPatch(-1), numKelvinWakesLoc(-1), numKelvinWakesLocPatch(-1), numLeewardDampenersLoc(-1),
        numLeewardDampenersLocPatch(-1), leewardDampeningStrengthLoc(-1), leewardDampeningStrengthLocPatch(-1),
        leewardDampeningStrength(0.1f), WakeManager(unitScale, psm), usingUBO(false), blockBuffer(0), uboHandle(0),
        leewardDampeners(0), patchLeewardDampeners(0) {}

    virtual ~WakeManagerOpenGL();

    virtual bool Initialize(const ProjectedGrid *grid, const Matrix3& basis, const Environment *env);

    virtual bool PrepShaders(const Environment *env);

    virtual bool PrepPatchShader(const Environment *env);

    virtual ShaderHandle GetShader() const {
        return (ShaderHandle)programCast;
    }

    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders, unsigned int gridShader, unsigned int gridPatchShader);

    virtual GLuint GetUboHandle() const {
        return uboHandle;
    }

    virtual GLuint GetDisplacementTextureHandle() const {
        return displacementTexture;
    }

private:

    bool SetupShaders();
    void DeleteShaders();

    void GetGridUniforms();

    GLhandleARB LoadShader(const char *shaderName, bool vertexShader, bool userShader, const Environment *env);

    int SetCircularWaves(GLint program, CircularWaveLocsOpenGL *waves, const Environment *env);
    int SetKelvinWakes(GLint program, KelvinWakeLocsOpenGL *wakes, const Environment *env);
    int SetPropWashes(GLint program, PropWashLocsOpenGL *washes, const Environment *env);
    int SetLeewardDampeners(GLint program, LeewardDampenerLocsOpenGL *dampeners, const Environment *env);

    GLhandleARB vertShader, fragShader, userShader, userVertShader, program;
    GLuint vertCastShader, fragCastShader, userCastShader, userVertCastShader, programCast;

    CircularWaveLocsOpenGL *circularWaves, *patchCircularWaves;
    KelvinWakeLocsOpenGL *kelvinWakes, *patchKelvinWakes;
    PropWashLocsOpenGL *propWashes, *patchPropWashes;
    LeewardDampenerLocsOpenGL *leewardDampeners, *patchLeewardDampeners;

    int doWakesLoc, washLengthLoc, displacementTextureLoc;
    int doWakesLocPatch, washLengthLocPatch, displacementTextureLocPatch;

    int numPropWashesLoc, numPropWashesLocPatch, numCircularWavesLoc, numCircularWavesLocPatch;
    int numKelvinWakesLoc, numKelvinWakesLocPatch, numLeewardDampenersLoc, numLeewardDampenersLocPatch;
    int leewardDampeningStrengthLoc, leewardDampeningStrengthLocPatch;

    bool usingUBO;

    GLuint sprayTexture, displacementTexture;

    TRITON_VECTOR(unsigned int) userShaders;

    GLuint blockIndex, blockIndexPatch;
    GLubyte *blockBuffer;
    GLint blockSize;
    GLuint uboHandle;

    float leewardDampeningStrength;
};
}

#endif