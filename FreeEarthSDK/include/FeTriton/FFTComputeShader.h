// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_FFTCOMPUTESHADER_H
#define TRITON_FFTCOMPUTESHADER_H

/** \file FFTComputeShader.h
   \brief An FFT implementation using DirectX11 Compute Shaders.
 */

#include <FeTriton/FFT.h>
#include <FeTriton/ResourceLoader.h>
#include <FeTriton/IFFTComputeShader.h>

namespace Triton
{
/** An FFT implementation using DirectX11 Compute Shaders. This "end to end" FFT can execute the entire
    ocean simulation on the GPU. */
class FFTComputeShader : public FFT
{
public:
    FFTComputeShader() : iFFTComputeShader(0) {}

    virtual ~FFTComputeShader();

    /** Attempts to load the Compute Shader FFT's underlying DLL. Returns false if it could not load for
        any reason, in which case an alternate FFT should be used. */
    static bool LoadDLL(ResourceLoader *rl);

    /** Returns true if the given device has feature level 11 (and therefore use of compute shaders) and
        this implementation has not been disabled via the disable-compute-shader configuration setting. */
    static bool IsCompatible(ID3D11Device *device);

    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, const Environment *env, bool enableHeightReads);

    virtual bool InverseTransform2D(ComplexNumber **inputArray, float **outputArray, unsigned int batchSize) {
        return false;
    }

    virtual bool TransformEndToEnd(ComplexNumber *H0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed);

    virtual bool UploadTextures();

    virtual bool TransformsEndToEnd() const {
        return true;
    }

    virtual bool ExploitsConjugate() const {
        return false;
    }

    virtual const char *GetName() const {
        return "DirectX11 Compute Shader";
    }

    //virtual bool SetOutputTextures(DX11Texture * displacement, DX11Texture * slopeFoam);

    virtual float GetHeight(float s, float t, bool hiRes);

    virtual void GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes);

    virtual const TRITON_VECTOR(Vector3)& GetSprays() const;

    virtual void EnableSpray(bool enabled);

private:
    IFFTComputeShader *iFFTComputeShader;
};
}

#endif