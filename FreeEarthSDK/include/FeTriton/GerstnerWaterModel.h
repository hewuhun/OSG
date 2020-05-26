// Copyright (c) 2011 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_GERSTNER_WATERMODEL_H
#define TRITON_GERSTNER_WATERMODEL_H

/** \file GerstnerWaterModel.h
   \brief A physical model of water based on sums of Gerstner waves.
 */

#include "WaterModel.h"

namespace Triton
{
/** A single Gerstner wave of given amplitude, phase speed, frequency, direction,
    steepness, and directional variance. */
class GerstnerWave : public MemObject
{
public:
    GerstnerWave() {}

    float amplitude, phaseSpeed, frequency, dirX, dirY, steepness, dirVariance, L;
};

/** The Gerstner WaterModel implementation is based on the sum of a small number of waves in the
    spatial domain. They are "Gerstner" waves and so may be choppy in appearance, but due to the
    limited number of them it's only appropriate for simulating calm, shallow water that has
    few waves. It's faster than the FFTWaterModel implementations, but there's a big tradeoff
    in quality for that speed. */
class GerstnerWaterModel : public WaterModel
{
public:
    GerstnerWaterModel(const Environment *env) : WaterModel(env), waves(0), numWaves(0), lastTime(0) {}

    virtual ~GerstnerWaterModel();

    virtual bool Initialize(ShaderHandle shader, ShaderHandle, bool enableHeightReads, bool);

    virtual bool PrepShaders(double time);

    virtual float GetHeight(const Vector3& position, const ProjectedGrid *grid, Vector3& normal, bool hiRes);

protected:
    int numWaves;
    GerstnerWave *waves;
    double midL, midA, aOverL;
    double lastTime;
};
}

#endif