// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_FFTCUDA_H
#define TRITON_FFTCUDA_H

//#if (defined(WIN32) || defined(WIN64))
//#if !defined(DIRECTX9_FOUND)
//#define DIRECTX9_FOUND 1
//#endif
//#else
//#define DIRECTX9_FOUND 0
//#endif

/** \file FFTCUDA.h
   \brief An FFT implementation using CUDA on NVidia systems.
 */

#include <FeTriton/FFT.h>
#include <FeTriton/IFFTCUDA.h>
#include <FeTriton/Environment.h>

namespace Triton
{
/** An FFT implemenation using NVidia's CUDA-based CUFFT library; works only on
    Nvidia systems with CUDA 4.0 or newer drivers installed, when OpenGL or
    DirectX9 is used. */
class FFTCUDA : public FFT
{
public:
    FFTCUDA() : FFT(), iFFTCUDA(0) {}

    virtual ~FFTCUDA();

    /** Returns true if you're running on an NVidia system with the necessary CUDA dependencies
        installed, and this implementation has not been disabled via the disable-cuda config flag. */
    static bool IsCompatible(Renderer renderer);

    /** Attempts to load the CUDA FFT's underlying DLL. Returns false if it could not load for
        any reason, in which case an alternate FFT should be used. */
    static bool LoadDLL(ResourceLoader *rl);

    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, const Environment *env,
                       bool enableHeightReads);

    virtual bool ExploitsConjugate() const {
        return true;
    }

    virtual const char *GetName() const {
        return "NVidia CUDA CUFFT";
    }

    virtual bool TransformEndToEnd(ComplexNumber *H0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed);

    virtual bool TransformsEndToEnd() const {
        return true;
    }

    virtual bool InverseTransform2D(ComplexNumber **inputArray, float **outputArray, unsigned int batchSize) {
        return false;
    }

    virtual bool SetOutputTextures(unsigned int displacement, unsigned int slopeFoam);

    virtual bool UploadTextures();

#if DIRECTX9_FOUND==1
    virtual bool SetOutputTextures(IDirect3DTexture9 *displacement, IDirect3DTexture9 *slopeFoam);

    virtual bool D3D9DeviceLost();

    virtual bool D3D9DeviceReset();
#endif

#if DIRECTX11_FOUND==1
    virtual bool SetOutputTextures(DX11Texture *displacement, DX11Texture *slopeFoam);
#endif

    virtual float GetHeight(float s, float t, bool hiRes);

    virtual void GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes);

    virtual const TRITON_VECTOR(Vector3)& GetSprays() const;

    virtual void EnableSpray(bool enabled);

private:
    IFFTCUDA * iFFTCUDA;
    FFTSettings settings;
};
}

#endif
