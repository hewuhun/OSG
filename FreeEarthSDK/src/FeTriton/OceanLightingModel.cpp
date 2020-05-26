// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/Environment.h>
#include <FeTriton/OceanLightingModel.h>
#include <FeTriton/Configuration.h>

using namespace Triton;

bool OceanLightingModel::Initialize(ShaderHandle shader, ShaderHandle patchShader)
{
    shaderHandle = shader;
    shaderHandlePatch = patchShader;

    double r = 0, g = 0.2, b = 0.3;
    Configuration::GetDoubleValue("refract-color-red", r);
    Configuration::GetDoubleValue("refract-color-blue", b);
    Configuration::GetDoubleValue("refract-color-green", g);

    if (r < 0 || g < 0 || b < 0) {
        Utils::DebugMsg("Negative refraction color value specified.");
        return false;
    }

    refractColorReflection = Vector3(r, g, b);

    Configuration::GetDoubleValue("refract-color-no-env-red", r);
    Configuration::GetDoubleValue("refract-color-no-env-blue", b);
    Configuration::GetDoubleValue("refract-color-no-env-green", g);

    if (r < 0 || g < 0 || b < 0) {
        Utils::DebugMsg("Negative refraction color value specified.");
        return false;
    }

    refractColorNoReflection = Vector3(r, g, b);

    return ReadConfigSettings();
}

const Vector3& OceanLightingModel::GetRefractionColor() const
{
    if (hasExplicitRefractColor) {
        return explicitRefractColor;
    }

    if (environment) {
        if (environment->GetEnvironmentMap() != NULL) {
            return refractColorReflection;
        }
    }

    return refractColorNoReflection;
}

bool OceanLightingModel::ReadConfigSettings()
{
    if (environment && environment->IsDirectX()) {
        shininess = 80.0;
        Configuration::GetDoubleValue("shininess-directx", shininess);
    } else {
        shininess = 5.0;
        Configuration::GetDoubleValue("shininess-opengl", shininess);
    }

    if (shininess < 0) {
        Utils::DebugMsg("Negative shininess specified.");
        return false;
    }

    foamScale = 16.0f;
    Configuration::GetFloatValue("foam-scale", foamScale);
    if (foamScale < 0) {
        Utils::DebugMsg("Negative foam-scale specified.");
        return false;
    }
    foamScale /= (float)environment->GetWorldUnits();

    foamBlend = 0.5f;
    Configuration::GetFloatValue("foam-blend", foamBlend);
    if (foamBlend < 0) {
        Utils::DebugMsg("Negative foam-blend specified.");
        return false;
    }

    propWashLodBias = 1.0f;
    Configuration::GetFloatValue("prop-wash-lod-bias", propWashLodBias);

    return true;
}