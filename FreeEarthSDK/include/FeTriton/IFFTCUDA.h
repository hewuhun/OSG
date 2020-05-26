// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#ifndef I_FFT_CUDA_H
#define I_FFT_CUDA_H

/** \file IFFTCUDA.h
   \brief An interface to CUDA shared between Triton and our TritonCUDA DLLs.
 */

//#if (defined(WIN32) || defined(WIN64))
//#if !defined(DIRECTX9_FOUND)
//#define DIRECTX9_FOUND 1
//#endif
//#else
//#define DIRECTX9_FOUND 0
//#endif

#if _WIN32
#define FFTCUDA_EXPORT 1
#endif

#if defined(FFTCUDA_EXPORT) // inside DLL
#   define FFTCUDAAPI  extern "C" __declspec(dllexport)
#else // outside DLL
#   define FFTCUDAAPI  extern "C"
#endif  // FFTCUDA_EXPORT

#include <FeTriton/IFFT.h>
#include <FeTriton/MemAlloc.h>
#include <FeTriton/Vector3.h>
#include <vector>

namespace Triton
{
class ResourceLoader;
class ComplexNumber;
class Allocator;

/** A shared interface between Triton and the TritonCUDA DLL's. */
class IFFTCUDA : public MemObject
{
public:
    virtual ~IFFTCUDA() {}

    /** Releases the CUDA resources at shutdown. */
    virtual void Release() = 0;

    /** Initializes and precomputes the underlying CUDA functions and FFT transformations.
        \param Nx   The X dimension of the FFT grid.
        \param Ny   The Y dimension of the FFT grid.
        \param batchSize    The number of grids transformed at once; should be 3 for this FFT implementation.
        \param rl   A ResourceLoader initialized to point to Triton's resources.
        \param settings The device and simulation parameters of the ocean simulation.
        \return Returns true if CUDA and the FFT transformations initialized successfully.
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

    /** Associates OpenGL RGBA floating point textures with the underlying CUDA methods; they'll populate them as directly
        and efficiently as possible. */
    virtual bool SetOutputTextures(unsigned int displacement, unsigned int slopeFoam) = 0;

    /** Upload the generated textures; called from the rendering thread, unlike TransformEndToEnd which may be called from an Update thread. */
    virtual bool UploadTextures() = 0;

#if DIRECTX9_FOUND==1
    /** Associates DirectX9 ARGB floating point textures with the underlying CUDA methods; they'll populate them as
        directly and efficiently as possible. */
    virtual bool SetOutputTextures(IDirect3DTexture9 *displacement, IDirect3DTexture9 *slopeFoam) = 0;

    /** Called in response to a lost device under DirectX9 prior to resetting it. */
    virtual bool D3D9DeviceLost() = 0;

    /** Called after a device reset in DirectX9 to restore the CUDA FFT resources. */
    virtual bool D3D9DeviceReset(IDirect3DDevice9 *device) = 0;
#endif

#if DIRECTX11_FOUND==1
    /** Associates DirectX11 floating point textures with the underlying CUDA methods. */
    virtual bool SetOutputTextures(ID3D11Texture2D *displacement, ID3D11Texture2D *slopeFoam) = 0;
#endif

    /** Retrieves the height at the given texture coordinates of the FFT patch. */
    virtual float GetHeight(float s, float t, bool hiRes) = 0;

    /** Retrieves the displacement due to choppiness at the given texture coordinates of the FFT patch. */
    virtual void GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes) = 0;

    /** Retrieves a vector of positions at which spray particles should be generated. Valid only if
        TransformsEndToEnd() returns true. */
    virtual const TRITON_VECTOR(Triton::Vector3)& GetSprays(bool enableOpenMP) = 0;

    /** Enables or disables spray particles on wave crests at runtime. */
    virtual void EnableSpray(bool enabled) = 0;
};
}

/// Retrieves the DLL's IFFTCUDA implementation.
FFTCUDAAPI Triton::IFFTCUDA* GetIFFTCUDA(Triton::Allocator *);

typedef Triton::IFFTCUDA *    (*GETIFFTCUDAPROC)(Triton::Allocator *);

/// Perform a compatibility test for CUDA on this system. We won't know for sure until Setup is called.
FFTCUDAAPI bool IsCompatible();

typedef bool (*ISCOMPATIBLEPROC)();

#endif
