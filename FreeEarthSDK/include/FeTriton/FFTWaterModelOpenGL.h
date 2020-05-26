// Copyright (c) 2011-2015 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_FFT_WATERMODELOPENGL_H
#define TRITON_FFT_WATERMODELOPENGL_H

/** \file FFTWaterModelOpenGL.h
   \brief A specialization of FFTWaterModel for OpenGL contexts.
 */

#include <FeTriton/TritonOpenGL.h>
#include <FeTriton/FFTWaterModel.h>

namespace Triton
{
class Environment;

/** A specialization of FFTWaterModel that integrates with OpenGL shaders
    and textures. */
class FFTWaterModelOpenGL : public FFTWaterModel
{
public:
    FFTWaterModelOpenGL(const Environment *env, WaterModelTypes type, OceanQuality quality);

    virtual ~FFTWaterModelOpenGL();

    virtual bool Initialize(ShaderHandle shader, ShaderHandle patchShader, bool enableHeightReads, bool noGPU);

    virtual bool PrepShaders(double time);

    virtual bool PrepPatchShader(double time);

    virtual void UpdateHeightMap(Environment *env);

    virtual void ReloadGridUniforms(ShaderHandle gridShader, ShaderHandle gridPatchShader);

protected:

    virtual float GetHeightData(const Vector3& position, float defaultDepth);

    virtual bool HasHeightMap();

private:
    GLint displacementLoc, slopeFoamLoc, textureSizeLoc, noiseAmplitudeLoc;
    GLint invNoiseDistanceLoc, invDampingDistanceLoc, lodBiasLoc, timeLoc;
    GLint displacementLocPatch, slopeFoamLocPatch, textureSizeLocPatch, noiseAmplitudeLocPatch;
    GLint invNoiseDistanceLocPatch, invDampingDistanceLocPatch, lodBiasLocPatch, timeLocPatch;
    GLuint displacementTexture, slopeFoamTexture;
    GLuint pbo[2];
    GLsync sync[2];
    bool pboMapped[2];
    bool pboValid[2];
    bool hasFBO, hasPBO;
    int pboIndex;
    float *mappedData;
    Matrix4 newHeightMapMatrix;
};
}

#endif