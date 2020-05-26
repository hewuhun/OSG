// Copyright (c) 2011-2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/WaterModel.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>

using namespace Triton;

WaterModel::WaterModel(const Environment *env) : environment(env), maxAmplitude(0), loopingPeriod(0.0f), slopeHeading(0.0f), sprayEnabled(true)
{
    if (!env) {
        Utils::DebugMsg("Null environment passed into WaterModel constructor");
    }

    chopScale = 0.2f;
    Configuration::GetFloatValue("fft-choppiness", chopScale);
    if (chopScale < 0) {
        Utils::DebugMsg("Negative fft-choppiness specified");
    }

    depth = 1000.0f;
    Configuration::GetFloatValue("fft-depth", depth);
    depth /= (float)env->GetWorldUnits();

    slope = 0.0f;

    waveHeading = 0.0f;

    seaFloorNormal = Vector3(0, 1, 0);

    shader = patchShader = 0;
}

float WaterModel::GetDepth(Vector3& floorNormal) const
{
    if (seaFloorNormal.SquaredLength() > 0) {
        floorNormal = seaFloorNormal;
    } else {
        floorNormal = environment->GetUpVector();
    }
    return depth;
}