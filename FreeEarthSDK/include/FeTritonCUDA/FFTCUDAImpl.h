// Copyright (c) 2011-2013 Sundog Software, LLC. All rights reserved worldwide.

#ifndef FFT_OPENCL_IMPL_H
#define FFT_OPENCL_IMPL_H

/** \file FFTCUDAImpl.h
    \brief The DLL implementation of the CUDA-based FFT.
*/

#include <FeTriton/IFFTCUDA.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cufft.h>

namespace Triton
{
/** The DLL implementation of IFFTCUDA. */
class FFTCUDAImpl : public IFFTCUDA
{
public:
    FFTCUDAImpl() : planH(0), HdataIn(0), chopXDataIn(0), H0(0), Hdata(0), chopXData(0),
        chopZData(0), displacementData(0), slopeFoamData(0), displacementResource(0),
        slopeFoamResource(0), displacementGLTexID(0), slopeFoamGLTexID(0),
        displacementPBO(0), slopeFoamPBO(0), omega(0), omegaCPU(0), lastDepth(-1),
        heightTest(false), readFoam(false), heightHost(0), foamHost(0), chopXHost(0),
        chopZHost(0), foam(0), planX(0), planZ(0), chopZDataIn(0), xDeviceComplexBuffer(0),
        zDeviceComplexBuffer(0), multiGPU(false), lastLoopingPeriod(-1), heightCopied(false),
        heightComputed(false), sprayEnabled(true) {}

    virtual void Release();

    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, ResourceLoader *rl, const FFTSettings& settings);

    virtual bool TransformEndToEnd(ComplexNumber *H0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed);

    virtual bool SetOutputTextures(unsigned int displacement, unsigned int slopeFoam);

    virtual bool UploadTextures();

#if DIRECTX9_FOUND==1
    virtual bool SetOutputTextures(IDirect3DTexture9 *displacement, IDirect3DTexture9 *slopeFoam);

    virtual bool D3D9DeviceLost();

    virtual bool D3D9DeviceReset(IDirect3DDevice9 *device);
#endif

#if DIRECTX11_FOUND==1
    virtual bool SetOutputTextures(ID3D11Texture2D *displacement, ID3D11Texture2D *slopeFoam);
#endif

    virtual float GetHeight(float s, float t, bool hiRes);

    virtual void GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes);

    virtual const TRITON_VECTOR(Vector3)& GetSprays(bool enableOpenMP);

    virtual void EnableSpray(bool enabled) {
        sprayEnabled = enabled;
    }

private:
    bool CreateOpenGLTextures();
    bool CreateDirectXTextures();
    void ComputeOmega(float depth,float loopingPeriod);
    bool CreateCUDAResources();
    bool ReleaseCUDAResources();

    unsigned int dimX, dimY;

    cufftHandle planH, planX, planZ;
    cufftComplex *HdataIn, *chopXDataIn, *chopZDataIn, *H0, *xDeviceComplexBuffer, *zDeviceComplexBuffer;
    cufftReal *Hdata, *chopXData, *chopZData, *omega, *foam;
    float *displacementData, *slopeFoamData, *omegaCPU;
    float *heightHost, *chopXHost, *chopZHost, *foamHost;
    cudaGraphicsResource *displacementResource, *slopeFoamResource;
    uint2 dim;
    float2 twoCellsSize, gridSize;
    float chopScale;
    float2 windDir;
    float dt, breakingSlope, foamFade, foamBlend, foamDecay;
    unsigned int displacementGLTexID, slopeFoamGLTexID, displacementPBO, slopeFoamPBO;
    Renderers renderer;
    float lastDepth, sizeX, sizeY, g, sprayThreshold, lastLoopingPeriod;
    bool heightTest, readFoam, multiGPU, heightCopied, heightComputed;
    TRITON_VECTOR(Vector3) sprays;
    bool sprayEnabled;
    size_t num_bytes;
    float *dispOut, *sfOut;

    int hDevice, xDevice, zDevice;
};
}

#endif
