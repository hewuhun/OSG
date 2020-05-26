// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#ifndef I_FFT_COMPUTESHADER_H
#define I_FFT_COMPUTESHADER_H

/** \file IFFTComputeShader.h
   \brief An interface shared between Triton and the underlying DirectX11 ComputeShader DLL.
 */

#if defined(FFTCOMPUTESHADER_EXPORT) // inside DLL
#   define FFTCOMPUTESHADERAPI  extern "C" __declspec(dllexport)
#else // outside DLL
#   define FFTCOMPUTESHADERAPI  extern "C"
#endif  // FFTCOMPUTESHADER_EXPORT

#include <FeTriton/IFFT.h>
#include <FeTriton/MemAlloc.h>
#include <FeTriton/Environment.h>

struct ID3DX11EffectShaderResourceVariable;
struct ID3D11Device;

namespace Triton
{
class ResourceLoader;
class ComplexNumber;
class Allocator;

/** An interface shared between Triton and the underlying DirectX11 Compute Shader DLL. We've separated most
    platform-specific code into DLL's of their own, so we can avoid loading them and running into DLL dependency
    pain at runtime. */
class IFFTComputeShader : public MemObject
{
public:
    /** Called at shutdown to release the underlying resources. */
    virtual void Release() = 0;

    /** Returns whether the current system is likely to be compatible with DirectX11 compute shaders. We won't know for
        sure until Setup() succeeds. */
    virtual bool IsCompatible(ID3D11Device *device) = 0;

    /** Initializes and precomputes the underlying compute shaders and FFT transformations.
        \param Nx   The X dimension of the FFT grid.
        \param Ny   The Y dimension of the FFT grid.
        \param batchSize    The number of grids transformed at once; should be 3 for this FFT implementation.
        \param rl   A ResourceLoader initialized to point to Triton's resources.
        \param settings The device and simulation parameters of the ocean simulation.
        \return Returns true if the compute shaders and FFT transformations initialized successfully.
    */
    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, ResourceLoader *rl, const FFTSettings& settings) = 0;

    /** Conducts the ocean simluation entirely on the GPU, populating the displacement and slope/foam textures directly.
        \param H0   A X+1 by Y+1 2D array of complex numbers representing the H0 frequency domain data described in
                    Jerry Tessendorf's 1999 paper "Simulating Ocean Waves." Setup() and SetOutputTextures() must be called
                    previously.
        \param t    The current time sample in seconds.
        \param dt   The elapsed time in seconds since the last frame.
        \param choppiness   The simulated choppiness of the waves (0-3.0)
        \param depth    The simulated depth of the water in world units.
        \param loopingPeriod  The wave simulation period. If non zero wave simulation repeats after that period.
        \param H0changed Whether the data in H0 changed since the previous call.
        \return True if the transformation succeeded.
    */
    virtual bool TransformEndToEnd(ComplexNumber *H0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed) = 0;

    virtual bool UploadTextures() = 0;

    /** Associates DirectX11 ARGB floating point texture resources with the underlying compute shaders. */
    virtual bool SetOutputTextures(DX11Texture *displacement, DX11Texture *slopeFoam) = 0;

    /** Retrieves the height at a given texture coordinate in the FFT patch. */
    virtual float GetHeight(float s, float t, bool hiRes) = 0;

    /** Retrieves the displacement due to local choppiness at the given texture coordinate in the FFT patch. */
    virtual void GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes) = 0;

    /** Retrieves a vector of positions at which spray particles should be generated. Valid only if
        TransformsEndToEnd() returns true. */
    virtual const TRITON_VECTOR(Vector3)& GetSprays(bool enableOpenMP) = 0;

    /** Enables or disables spray particles on wave crests at runtime. */
    virtual void EnableSpray(bool enabled) = 0;
};
}

/// Retrieves the IFFTComputeShader implementation from the DLL
FFTCOMPUTESHADERAPI Triton::IFFTComputeShader* GetIFFTComputeShader(Triton::Allocator *);

typedef Triton::IFFTComputeShader *    (*GETIFFTCOMPUTESHADERPROC)(Triton::Allocator *);

#endif