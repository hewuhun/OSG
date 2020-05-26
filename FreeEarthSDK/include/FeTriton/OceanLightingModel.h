// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_OCEANLIGHTINGMODEL_H
#define TRITON_OCEANLIGHTINGMODEL_H

/** \file OceanLightingModel.h
    \brief A class that sets lighting parameters for the ProjectedGrid's pixel shaders.
*/

#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>

namespace Triton
{
class Environment;

/** A class that sets lighting and texturing parameters for the ProjectedGrid's pixel shaders. */
class OceanLightingModel : public MemObject
{
public:
    /** Construct with your Environment object. */
    OceanLightingModel(const Environment *env) : environment(env), shaderHandle(0), shininess(5.0),
        hiResFoam(false), hasExplicitRefractColor(false) {}

    /** Virtual destructor. */
    virtual ~OceanLightingModel() {}

    /** Associates the OceanLightingModel with the shader from the ProjectedGrid, and extracts the
        location of the parameters it needs from the shader.
        \param shader The ProjectedGrid's shader to associate with this object.
        \return True if the expected parameters were found in the shader.
    */
    virtual bool Initialize(ShaderHandle shader, ShaderHandle patchShader);

    /** Sets the parameters for lighting with the shader passed in via Initialize().
        \return True if the parameters were set successfully.
    */
    virtual bool PrepShaders() = 0;

    virtual bool PrepPatchShader() {
        return false;
    }

    /** Modifies the color used for refracted light rays that go into deep water. You can use this to modify the
        color of the water in areas that are not purely reflective.
        \param refractionColor the RGB color value of the deep water color; each component should be in the range 0-1.
    */
    void SetRefractionColor(const Vector3& refractionColor) {
        explicitRefractColor = refractionColor;
        hasExplicitRefractColor = true;
    }

    /** Returns the color of light refracted into the water.
        \sa SetRefractionColor();
        \return The RGB value of the refraction color.
    */
    const Vector3& GetRefractionColor() const;

    /** Reload uniform locations from grid shader when it is reloaded at runtime. */
    virtual void ReloadGridUniforms(ShaderHandle gridShader, ShaderHandle gridShaderPatch) {
        shaderHandle = gridShader;
        shaderHandlePatch = gridShaderPatch;
    }

protected:
    bool ReadConfigSettings();

    const Environment *environment;
    ShaderHandle shaderHandle, shaderHandlePatch;
    Vector3 explicitRefractColor, refractColorReflection, refractColorNoReflection;
    bool hasExplicitRefractColor;
    double shininess;
    float foamScale, foamBlend;
    float propWashLodBias;
    bool hiResFoam;
};
}

#endif