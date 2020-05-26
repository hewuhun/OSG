// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_PARTICLE_SYSTEM_OPENGL_H
#define TRITON_PARTICLE_SYSTEM_OPENGL_H

/** \file ParticleSystemOpenGL.h
    \brief An implementation of ParticleSystem for OpenGL.
*/

#include <FeTriton/ParticleSystem.h>
#include <FeTriton/TritonOpenGL.h>
#ifndef _WIN32
#include <stdint.h>
#endif

namespace Triton
{
class Environment;

struct UniLocs {
    GLint initialPosLoc, initialVelLoc, sizeLoc, startTimeLoc;
    GLint offsetXLoc, offsetYLoc, texCoordXLoc, texCoordYLoc;
    GLint modelViewLoc, modelViewProjLoc, timeLoc, gLoc;
    GLint particleTextureLoc, lightColorLoc, transparencyLoc, cameraPosLoc;
    GLint heightMapLoc, hasHeightMapLoc, heightMapMatrixLoc, invSizeFactorLoc;
    GLint seaLevelLoc;
    GLint hasUserHeightMapLoc;
    GLint heightMapRangeOffsetLoc;
    GLint refOffsetLoc, fogDensityLoc, fogColorLoc;
};

class ParticleSystemOpenGL : public ParticleSystem
{
public:


public:
    ParticleSystemOpenGL(const Environment *env) : ParticleSystem(env), vboID(0), textureID(0),
        program(0), numParticles(0), vertexArray(0),
        usePointSprites(false), vertsPerParticle(6), vertexDataPtr(0), ul() {}

    virtual ~ParticleSystemOpenGL();

    virtual bool Initialize(TextureHandle texture, ShaderHandle shader, int numParticles, const ProjectedGrid *grid);

    virtual void Start(double pStartTime, const Vector3& direction, float dirVariation,
                       float velocity, float velVariation, const Vector3& position, float posVariation,
                       float size, float sizeVariation);

    virtual void StartBatch(double pStartTime, const TRITON_VECTOR(Vector3)&positions,
                            const Vector3& direction, float dirVariation, float velocity, float velVariation,
                            float posVariation, float size, float sizeVariation);

    virtual void SetupCommonState(double time);

    virtual bool Draw();

    virtual void CleanupCommonState();

    static bool UsingPointSprites();

    virtual void SetShader(ShaderHandle shader);

    virtual UniLocs GetUniLocsFromShader();

    virtual void SetShaderUniLocs(UniLocs ulIn) {
        ul = ulIn;
    }

    virtual UniLocs GetShaderUniLocs() {
        return ul;
    }


private:
    void SetVerts(float *vertBuffer, int startIdx, const Vector3& position, const Vector3& direction, float size, float time);
    GLint textureID;

    UniLocs ul;

    GLuint vboID, vertexArray, programCast;
    GLhandleARB program;
    int numParticles;
    int maxPointSize;
    bool usePointSprites;
    int vertsPerParticle;
    void *vertexDataPtr;
};
}
#endif