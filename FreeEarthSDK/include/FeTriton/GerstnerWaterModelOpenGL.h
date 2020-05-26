// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_GERSTNER_WATERMODEL_OPENGL_H
#define TRITON_GERSTNER_WATERMODEL_OPENGL_H

/** \file GerstnerWaterModelOpenGL.h
   \brief A specialization of GerstnerWaterModel for OpenGL.
 */

#include "GerstnerWaterModel.h"
#include "TritonOpenGL.h"

namespace Triton
{
/** A collection of parameters describing a single Gerstner wave associated with
    the underlying OpenGL vertex program. */
class WaveLocsOpenGL : public MemObject
{
public:
    GLint steepnessLoc, amplitudeLoc, frequencyLoc;
    GLint directionLoc, phaseSpeedLoc;
};

/** A specialization of GerstnerWaterModel for OpenGL shaders and textures. */
class GerstnerWaterModelOpenGL : public GerstnerWaterModel
{
public:
    GerstnerWaterModelOpenGL(const Environment *env) : GerstnerWaterModel(env), waveLocs(0) {}
    virtual ~GerstnerWaterModelOpenGL();

    virtual bool Initialize(ShaderHandle shader, ShaderHandle, bool enableHeightReads, bool);
    virtual bool PrepShaders(double time);

private:
    WaveLocsOpenGL *waveLocs;
    GLint numWavesLoc, timeLoc;
};
}

#endif