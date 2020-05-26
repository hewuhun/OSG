// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide

#include <GL/glew.h>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else // __APPLE__
#include <GL/gl.h>
#endif // __APPLE__

//#include "glext.h"

#ifndef _WIN32
#ifndef __APPLE__
#include "GL/glx.h"
#endif // __APPLE__
#endif // _WIN32

#include <FeTritonCUDA/FFTCUDAImpl.h>
//#include <FeTriton/ResourceLoader.h>
#include <FeTriton/ComplexNumber.h>
#include <cuda_gl_interop.h>
#if DIRECTX9_FOUND==1
#include <cuda_d3d9_interop.h>
#endif // DIRECTX9_FOUND

#if DIRECTX11_FOUND==1
#include <cuda_d3d11_interop.h>
#endif // DIRECTX11_FOUND

#ifdef _OPENMP
#include <omp.h>
#endif // _OPENMP

#include <assert.h>
#include <cstring>

#define USE_PBO

#define USE_SHARED_MEM

#define ALLOW_OPENMP false

using namespace Triton;

extern "C" void cudaFlipSignsKernel(  float* hptr, unsigned int width, unsigned int height);
extern "C" void cudaCreateTextures(const float *Hdata, const float *chopXData, const float *chopZData, unsigned int width, unsigned int height, float *displacement, float *slopeFoam, float *foam, uint2 dim, float2 twoCellsSize,
                                   float chopScale, float dt);
extern "C" void cudaProcessWater(const float *H0, const float *omega, float *HdataIn, float *chopXDataIn, float *chopZDataIn, float t, uint2 inDim, uint2 outDim, float2 size);

#ifndef __APPLE__
namespace Triton
{
static PFNGLGENBUFFERSPROC glGenBuffers = 0;
static PFNGLBINDBUFFERPROC glBindBuffer = 0;
static PFNGLBUFFERDATAPROC glBufferData = 0;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers = 0;
static PFNGLACTIVETEXTUREPROC glActiveTexture = 0;
}
#endif

static int nextAvailableDevice = 0;
static bool noSync = false;

bool FFTCUDAImpl::CreateCUDAResources()
{
    cudaError e;

    int deviceCount = 0;
    e = cudaGetDeviceCount(&deviceCount);

    hDevice = xDevice = zDevice = nextAvailableDevice++;

    if (nextAvailableDevice > deviceCount) return false;

    if (multiGPU) {
        if (deviceCount > nextAvailableDevice) {
            xDevice = nextAvailableDevice++;
            cudaSetDevice( xDevice );
            e = cudaMalloc((void**)&xDeviceComplexBuffer, sizeof(cufftComplex) * (dimX/2+1)*dimY);
            if (e != cudaSuccess) return false;
            int accessible = 0;
            cudaDeviceCanAccessPeer( &accessible, xDevice, hDevice );
            if( accessible ) {
                cudaDeviceEnablePeerAccess( hDevice, 0 );
            }
        }
        if (deviceCount > nextAvailableDevice) {
            zDevice = nextAvailableDevice++;
            cudaSetDevice(zDevice);
            e = cudaMalloc((void**)&zDeviceComplexBuffer, sizeof(cufftComplex) * (dimX/2+1)*dimY);
            if (e != cudaSuccess) return false;
            int accessible = 0;
            cudaDeviceCanAccessPeer( &accessible, zDevice, hDevice );
            if( accessible ) {
                cudaDeviceEnablePeerAccess( hDevice, 0 );
            }
        }
    }

    e = cudaSetDevice(hDevice);
    if (e != cudaSuccess) return false;

    unsigned int NX = dimX;
    unsigned int NY = dimY;

    e = cudaMalloc((void**)&H0, sizeof(cufftComplex) * (NX+1) * (NY+1));
    if (e != cudaSuccess) return false;
    e = cudaMalloc((void**)&omega, sizeof(float) * NX * NY);
    if (e != cudaSuccess) return false;
    e = cudaMalloc((void**)&HdataIn, sizeof(cufftComplex)*(NX/2+1)*NY);
    if (e != cudaSuccess) return false;
    e = cudaMalloc((void**)&Hdata, sizeof(cufftReal)*NX*NY);
    if (e != cudaSuccess) return false;

    if (heightTest || readFoam) {
#ifdef USE_SHARED_MEM
        e = cudaMallocHost((void**)&heightHost, NX * NY * sizeof(float));
        if (e != cudaSuccess) return false;
#else
        heightHost = (float *)malloc(NX * NY * sizeof(float));
#endif
        memset(heightHost, 0, NX * NY * sizeof(float));
    }

    if (readFoam) {
#ifdef USE_SHARED_MEM
        e = cudaMallocHost((void**)&foamHost, NX * NY * sizeof(float));
        if (e != cudaSuccess) return false;
#else
        foamHost = (float *)malloc(NX * NY * sizeof(float));
#endif
        memset(foamHost, 0, NX * NY * sizeof(float));
        sprays.reserve(NX * NY);
    }

    /* Create a 2D FFT plan. */
    // This will generate a first-chance exception when running inside
    // Visual Studio. This is considered expected behavior from cufft
    // although it is annoying. Hit "continue" in your debugger, or disable
    // breaking on exceptions under Debug/Exceptions. First chance exceptions
    // cannot be caught with try/catch so don't bother trying...
    cufftResult r = cufftPlan2d(&planH, NX, NY, CUFFT_C2R);
    if (r != CUFFT_SUCCESS) return false;

    e = cudaMalloc((void**)&chopXDataIn, sizeof(cufftComplex)*(NX/2+1)*NY);
    if (e != cudaSuccess) return false;

    cudaSetDevice(xDevice);
    e = cudaMalloc((void**)&chopXData, sizeof(cufftReal)*NX*NY);
    if (e != cudaSuccess) return false;
    /* Create a 2D FFT plan. */
    // This will generate a first-chance exception when running inside
    // Visual Studio. This is considered expected behavior from cufft
    // although it is annoying. Hit "continue" in your debugger, or disable
    // breaking on exceptions under Debug/Exceptions
    r = cufftPlan2d(&planX, NX, NY, CUFFT_C2R);
    if (r != CUFFT_SUCCESS) return false;

    if (readFoam || heightTest) {
#ifdef USE_SHARED_MEM
        e = cudaMallocHost((void**)&chopXHost, NX * NY * sizeof(float));
        if (e != cudaSuccess) return false;
#else
        chopXHost = (float*)malloc(NX * NY * sizeof(float));
#endif
        memset(chopXHost, 0, NX * NY * sizeof(float));
    }

    cudaSetDevice(zDevice);
    e = cudaMalloc((void**)&chopZDataIn, sizeof(cufftComplex)*(NX/2+1)*NY);
    if (e != cudaSuccess) return false;
    e = cudaMalloc((void**)&chopZData, sizeof(cufftReal)*NX*NY);
    if (e != cudaSuccess) return false;

    if (readFoam || heightTest) {
#ifdef USE_SHARED_MEM
        e = cudaMallocHost((void**)&chopZHost, NX * NY *sizeof(float));
        if (e != cudaSuccess) return false;
#else
        chopZHost = (float*)malloc(NX *  NY * sizeof(float));
#endif
        memset(chopZHost, 0, NX * NY * sizeof(float));
    }

    /* Create a 2D FFT plan. */
    // This will generate a first-chance exception when running inside
    // Visual Studio. This is considered expected behavior from cufft
    // although it is annoying. Hit "continue" in your debugger, or disable
    // breaking on exceptions under Debug/Exceptions
    r = cufftPlan2d(&planZ, NX, NY, CUFFT_C2R);
    if (r != CUFFT_SUCCESS) return false;

    cudaSetDevice(hDevice);

    e = cudaMalloc((void**)&foam, sizeof(float)*NX*NY);
    if (e != cudaSuccess) return false;

#ifndef USE_PBO
    if (renderer == FFT_OPENGL) {
        e = cudaMalloc((void**)&displacementData, sizeof(float)*NX*NY*4);
        if (e != cudaSuccess) return false;
        e = cudaMalloc((void**)&slopeFoamData, sizeof(float)*NX*NY*4);
        if (e != cudaSuccess) return false;

        cudaMemset(displacementData, 0, NX * NY * 4 * sizeof(float));
        cudaMemset(slopeFoamData, 0, sizeof(float)*NX*NY*4);
    }
#endif

    if (renderer == FFT_DIRECTX9 || renderer == FFT_DIRECTX11) {
        e = cudaMalloc((void**)&displacementData, sizeof(float)*NX*NY*4);
        if (e != cudaSuccess) return false;
        e = cudaMalloc((void**)&slopeFoamData, sizeof(float)*NX*NY*4);
        if (e != cudaSuccess) return false;

        cudaMemset(displacementData, 0, NX * NY * 4 * sizeof(float));
        cudaMemset(slopeFoamData, 0, sizeof(float)*NX*NY*4);
    }

    // We rely on the cuda complex data structure looking like our own to avoid expensive
    // transformations. Make sure this is true.
    assert(sizeof(cufftComplex) == sizeof(ComplexNumber));
    assert(sizeof(cufftReal) == sizeof(float));

    return true;
}

bool FFTCUDAImpl::Setup(unsigned int NX, unsigned int NY, unsigned int batches, ResourceLoader *rl, const FFTSettings& settings)
{
#ifdef _WIN32
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
#else
#ifndef __APPLE__
    glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte*)"glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glDeleteBuffers");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)glXGetProcAddress((const GLubyte*)"glActiveTexture");
#endif
#endif

    noSync = !settings.enableSync;

    dimX = NX;
    dimY = NY;

    heightTest = settings.enableHeightTests;
    readFoam = settings.enableSpray;
    sprayThreshold = settings.sprayThreshold;

    cudaError e = cudaSuccess;

    int savedDevice = 0;
    cudaGetDevice(&savedDevice);

    if (settings.renderer == FFT_OPENGL) {
#ifndef __APPLE__
        if (!glGenBuffers || !glBindBuffer || !glBufferData || !glDeleteBuffers || !glActiveTexture) {
            return false;
        }
#endif

        int deviceCount = 0;
        e = cudaGetDeviceCount(&deviceCount);
        if (nextAvailableDevice < deviceCount) {
            e = cudaGLSetGLDevice(nextAvailableDevice);
        }

        /*
        if (settings.enableMultiGPU) {
            int deviceCount = 0;
            e = cudaGetDeviceCount(&deviceCount);
            if (nextAvailableDevice < deviceCount) {
                e = cudaGLSetGLDevice(nextAvailableDevice);
            }
        } else {
            unsigned int numDevices;
            int deviceArray[8];
            int arraySize = 8;
            e = cudaGLGetDevices(&numDevices, deviceArray, 8, cudaGLDeviceListAll);

            int max_multiprocessors = 0, max_device = 0;
            for (int i = 0; i < (int)numDevices; i++) {
                if (deviceArray[i] >= nextAvailableDevice) {
                    cudaDeviceProp properties;
                    cudaGetDeviceProperties(&properties, deviceArray[i]);
                    if (max_multiprocessors < properties.multiProcessorCount) {
                        max_multiprocessors = properties.multiProcessorCount;
                        max_device = deviceArray[i];
                    }
                }
            }

            if (max_device >= 0) {
                nextAvailableDevice = max_device;
                e = cudaGLSetGLDevice(nextAvailableDevice);
            }
        }
        */
    }
#if DIRECTX9_FOUND==1
    else if (settings.renderer == FFT_DIRECTX9) {
        unsigned int numDevices;
        int deviceArray[8];
        int arraySize = 8;
        e = cudaD3D9GetDevices(&numDevices, deviceArray, 8, settings.d3d9Device, cudaD3D9DeviceListAll);

        int firstAvailableDevice = -1;
        for (int i = 0; i < (int)numDevices; i++) {
            if (deviceArray[i] >= nextAvailableDevice) {
                firstAvailableDevice = deviceArray[i];
            }
        }

        if (firstAvailableDevice >= 0) {
            nextAvailableDevice = firstAvailableDevice;
            e = cudaD3D9SetDirect3DDevice(settings.d3d9Device, nextAvailableDevice);
        }
    }
#endif
#if DIRECTX11_FOUND==1
    else if (settings.renderer == FFT_DIRECTX11) {
        unsigned int numDevices;
        int deviceArray[8];
        int arraySize = 8;
        e = cudaD3D11GetDevices(&numDevices, deviceArray, 8, settings.d3d11Device, cudaD3D11DeviceListAll);

        int firstAvailableDevice = -1;
        for (int i = 0; i < (int)numDevices; i++) {
            if (deviceArray[i] >= nextAvailableDevice) {
                firstAvailableDevice = deviceArray[i];
            }
        }

        if (firstAvailableDevice >= 0) {
            nextAvailableDevice = firstAvailableDevice;
            e = cudaD3D11SetDirect3DDevice(settings.d3d11Device, nextAvailableDevice);
        }
    }
#endif

    if (e != cudaSuccess || nextAvailableDevice < 0) {
        hDevice = xDevice = zDevice = nextAvailableDevice;
        cudaSetDevice(savedDevice);
        return false;
    }

    renderer = settings.renderer;

    omegaCPU = TRITON_NEW float[NX * NY];
    if (!omegaCPU) return false;

    g = settings.gravity / settings.worldUnits;

    dim.x = NX;
    dim.y = NY;
    twoCellsSize.x = ((settings.fftGridSizeX * 2.0f) / (float)NX);
    twoCellsSize.y = ((settings.fftGridSizeY * 2.0f) / (float)NY);

    sizeX = settings.fftGridSizeX;
    sizeY = settings.fftGridSizeY;

    gridSize.x = settings.fftGridSizeX;
    gridSize.y = settings.fftGridSizeY;

    multiGPU = settings.enableMultiGPU;

    // This will generate 3 first-chance exceptions when running inside
    // Visual Studio. This is considered expected behavior from cufft
    // although it is annoying. Hit "continue" in your debugger, or disable
    // breaking on exceptions under Debug/Exceptions
    return CreateCUDAResources();
}

#if DIRECTX9_FOUND==1
bool FFTCUDAImpl::D3D9DeviceLost()
{
    // We are handling a device lost
    cudaSetDevice(hDevice);
    cudaDeviceSynchronize();
    ReleaseCUDAResources();
    cudaError e = cudaDeviceReset();
    nextAvailableDevice = hDevice;
    return (e == cudaSuccess);
}

bool FFTCUDAImpl::D3D9DeviceReset(IDirect3DDevice9 *d3dDevice)
{
    cudaError e = cudaD3D9SetDirect3DDevice(d3dDevice);
    if (e != cudaSuccess) return false;
    if (!CreateCUDAResources()) return false;

    return true;
}
#endif

float FFTCUDAImpl::GetHeight(float s, float t, bool hiRes)
{
    if (s < 0 || t < 0 || s >= 1.0f || t >= 1.0f) {
        return 0.0f;
    }

    if (heightTest && heightHost && heightCopied) {
        int xlow = (int)(s * (float)dimX);
        int ylow = (int)(t * (float)dimY);

        if (hiRes) {
            int xhigh = xlow + 1;
            int yhigh = ylow + 1;
            if (xhigh >= (int)dimX) {
                xhigh = 0;
            }
            if (yhigh >= (int)dimY) {
                yhigh = 0;
            }
            const float A = 0.5f;

            float quadS = (s - (float)xlow / (float)dimX) * dimX;
            float quadT = (t - (float)ylow / (float)dimY) * dimY;

            if (quadS < quadT) {
                float h0 = heightHost[xlow + yhigh * dimX];
                float h1 = heightHost[xlow + ylow * dimX];
                float h2 = heightHost[xhigh + yhigh * dimX];
                float A1 = (1.0f - quadT) * 0.5f;
                float A2 = quadS * 0.5f;
                float A0 = A - (A1 + A2);

                return (h0 * A0 + h1 * A1 + h2 * A2) / A;
            } else {
                float h0 = heightHost[xlow + ylow * dimX];
                float h1 = heightHost[xhigh + ylow * dimX];
                float h2 = heightHost[xhigh + yhigh * dimX];
                float A2 = quadT * 0.5f;
                float A0 = (1.0f - quadS) * 0.5f;
                float A1 = A - (A0 + A2);

                return (h0 * A0 + h1 * A1 + h2 * A2) / A;
            }
        } else {
            float h = heightHost[xlow + ylow * dimX];
            return h;
        }
    } else {
        return 0.0f;
    }
}

void FFTCUDAImpl::GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes)
{
    if (s < 0 || t < 0 || s >= 1.0f || t >= 1.0f || !heightCopied) {
        chopX = chopZ = 0.0f;
        return;
    }

    if (chopXHost && chopZHost) {
        int xlow = (int)(s * (float)dimX);
        int ylow = (int)(t * (float)dimY);

        if (hiRes) {
            int xhigh = xlow + 1;
            int yhigh = ylow + 1;
            if (xhigh >= (int)dimX) xhigh = 0;
            if (yhigh >= (int)dimY) yhigh = 0;

            const float A = 0.5f;

            float quadS = (s - (float)xlow / (float)dimX) * dimX;
            float quadT = (t - (float)ylow / (float)dimY) * dimY;

            if (quadS < quadT) {
                float x0 = chopXHost[xlow + yhigh * dimX];
                float x1 = chopXHost[xlow + ylow * dimX];
                float x2 = chopXHost[xhigh + yhigh * dimX];

                float z0 = chopZHost[xlow + yhigh * dimX];
                float z1 = chopZHost[xlow + ylow * dimX];
                float z2 = chopZHost[xhigh + yhigh * dimX];

                float A1 = (1.0f - quadT) * 0.5f;
                float A2 = quadS * 0.5f;
                float A0 = A - (A1 + A2);

                chopX = (x0 * A0 + x1 * A1 + x2 * A2) / A;
                chopZ = (z0 * A0 + z1 * A1 + z2 * A2) / A;
            } else {
                float x0 = chopXHost[xlow + ylow * dimX];
                float x1 = chopXHost[xhigh + ylow * dimX];
                float x2 = chopXHost[xhigh + yhigh * dimX];

                float z0 = chopZHost[xlow + ylow * dimX];
                float z1 = chopZHost[xhigh + ylow * dimX];
                float z2 = chopZHost[xhigh + yhigh * dimX];

                float A2 = quadT * 0.5f;
                float A0 = (1.0f - quadS) * 0.5f;
                float A1 = A - (A0 + A2);

                chopX = (x0 * A0 + x1 * A1 + x2 * A2) / A;
                chopZ = (z0 * A0 + z1 * A1 + z2 * A2) / A;
            }
        } else {
            chopX = chopXHost[xlow + ylow * dimX];
            chopZ = chopZHost[xlow + ylow * dimX];
        }
    }
}

const TRITON_VECTOR(Vector3)& FFTCUDAImpl::GetSprays(bool enableOpenMP)
{
    sprays.clear();

    if (!readFoam || !heightCopied || !sprayEnabled) {
        return sprays;
    }

    Vector3 v;

    float invDimX = 1.0f / (float)dimX;
    float invDimY = 1.0f / (float)dimY;

    if (enableOpenMP && ALLOW_OPENMP) {
        #pragma omp parallel for
        for (int x = 0; x < (int)dimX; x++) {
            for (int y = 0; y < (int)dimY; y++) {

                int idx = x + y * dimY;
                if (foamHost[idx] > sprayThreshold) {
                    float height = heightHost[idx];
                    float chopX = chopXHost[idx];
                    float chopZ = chopZHost[idx];
                    float fx = sizeX * (float)x * invDimX;
                    float fy = sizeY * (float)y * invDimY;
                    v.x = fx + chopX;
                    v.y = fy + chopZ;
                    v.z = height;
                    #pragma omp critical
                    {
                        sprays.push_back(v);
                    }
                }
            }
        }
    } else {

        for (int y = 0; y < (int)dimY; y++) {
            int rowIdx = y * dimY;
            for (int x = 0; x < (int)dimX; x++) {
                int idx = rowIdx + x;
                if (foamHost[idx] > sprayThreshold) {
                    float height = heightHost[idx];
                    float chopX = chopXHost[idx];
                    float chopZ = chopZHost[idx];
                    float fx = sizeX * (float)x * invDimX;
                    float fy = sizeY * (float)y * invDimY;
                    v.x = fx + chopX;
                    v.y = fy + chopZ;
                    v.z = height;
                    sprays.push_back(v);
                }
            }
        }
    }

    return sprays;
}

#if DIRECTX9_FOUND==1
bool FFTCUDAImpl::SetOutputTextures(IDirect3DTexture9 *displacementTex, IDirect3DTexture9 *slopeFoamTex)
{
    cudaSetDevice(hDevice);

    cudaError e;

    if (displacementTex) {
        e = cudaGraphicsD3D9RegisterResource(&displacementResource, displacementTex, cudaGraphicsRegisterFlagsNone);
        if (e != cudaSuccess) return false;
    } else {
        e = cudaGraphicsUnregisterResource(displacementResource);
        displacementResource = NULL;
        if (e != cudaSuccess) return false;
    }

    if (slopeFoamTex) {
        e = cudaGraphicsD3D9RegisterResource(&slopeFoamResource, slopeFoamTex, cudaGraphicsRegisterFlagsNone);
        if (e != cudaSuccess) return false;
    } else {
        e = cudaGraphicsUnregisterResource(slopeFoamResource);
        slopeFoamResource = NULL;
        if (e != cudaSuccess) return false;
    }

    return true;
}
#endif

#if DIRECTX11_FOUND==1
bool FFTCUDAImpl::SetOutputTextures(ID3D11Texture2D *displacementTex, ID3D11Texture2D *slopeFoamTex)
{
    cudaSetDevice(hDevice);

    cudaError e;

    if (displacementTex) {
        e = cudaGraphicsD3D11RegisterResource(&displacementResource, displacementTex, cudaGraphicsRegisterFlagsNone);
        if (e != cudaSuccess) return false;
    } else {
        e = cudaGraphicsUnregisterResource(displacementResource);
        displacementResource = NULL;
        if (e != cudaSuccess) return false;
    }

    if (slopeFoamTex) {
        e = cudaGraphicsD3D11RegisterResource(&slopeFoamResource, slopeFoamTex, cudaGraphicsRegisterFlagsNone);
        if (e != cudaSuccess) return false;
    } else {
        e = cudaGraphicsUnregisterResource(slopeFoamResource);
        slopeFoamResource = NULL;
        if (e != cudaSuccess) return false;
    }

    return true;
}
#endif

bool FFTCUDAImpl::SetOutputTextures(unsigned int displacementTex, unsigned int slopeFoamTex)
{
    cudaSetDevice(hDevice);

    displacementGLTexID = displacementTex;
    slopeFoamGLTexID = slopeFoamTex;

#ifndef USE_PBO
    cudaError e1 = cudaGraphicsGLRegisterImage(&displacementResource, displacementGLTexID, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);
    cudaError e2 = cudaGraphicsGLRegisterImage(&slopeFoamResource, slopeFoamGLTexID, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);

    return (e1 == cudaSuccess && e2 == cudaSuccess);
#else

    size_t texBytes = dimX * dimY * 4 * sizeof(float);

    glGenBuffers(1, &displacementPBO);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, displacementPBO);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, texBytes, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    glGenBuffers(1, &slopeFoamPBO);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, slopeFoamPBO);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, texBytes, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    cudaError e1 = cudaGraphicsGLRegisterBuffer(&displacementResource, displacementPBO, cudaGraphicsMapFlagsNone);
    cudaError e2 = cudaGraphicsGLRegisterBuffer(&slopeFoamResource, slopeFoamPBO, cudaGraphicsMapFlagsNone);

    cudaError e = cudaGraphicsMapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsResourceGetMappedPointer((void **)&dispOut, &num_bytes, displacementResource);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsUnmapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;

    e = cudaGraphicsMapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsResourceGetMappedPointer((void **)&sfOut, &num_bytes, slopeFoamResource);
    if (e != cudaSuccess) return false;

    e = cudaGraphicsUnmapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;

    return (e1 == cudaSuccess && e2 == cudaSuccess);
#endif
}

bool FFTCUDAImpl::UploadTextures()
{
    bool ok = false;
    switch (renderer) {
    case FFT_OPENGL:
        ok = CreateOpenGLTextures();
        break;

    case FFT_DIRECTX9:
    case FFT_DIRECTX11:
        ok = CreateDirectXTextures();
        break;

    default:
        break;
    }

    return ok;
}

bool FFTCUDAImpl::ReleaseCUDAResources()
{
    cudaError e = cudaSetDevice(hDevice);
    if (e != cudaSuccess) return false;

    /* Destroy the CUFFT plan. */
    cufftDestroy(planH);
    cufftDestroy(planX);
    cufftDestroy(planZ);

    if (HdataIn) {
        cudaFree(HdataIn);
        HdataIn = 0;
    }
    if (Hdata) {
        cudaFree(Hdata);
        Hdata = 0;
    }
    if (H0) {
        cudaFree(H0);
        H0 = 0;
    }

    if (chopXDataIn) {
        cudaFree(chopXDataIn);
        chopXDataIn = 0;
    }

    if (chopZDataIn) {
        cudaFree(chopZDataIn);
        chopZDataIn = 0;
    }

    if (heightHost) {
#ifdef USE_SHARED_MEM
        cudaFreeHost(heightHost);
#else
        free(heightHost);
#endif
        heightHost = 0;
        heightCopied = false;
    }

    if (foamHost) {
#ifdef USE_SHARED_MEM
        cudaFreeHost(foamHost);
#else
        free(foamHost);
#endif
        foamHost = 0;
    }

    cudaSetDevice(xDevice);

    if (chopXData) {
        cudaFree(chopXData);
        chopXData = 0;
    }

    if (xDeviceComplexBuffer) {
        cudaFree(xDeviceComplexBuffer);
        xDeviceComplexBuffer = 0;
    }

    if (chopXHost) {
#ifdef USE_SHARED_MEM
        cudaFreeHost(chopXHost);
#else
        free(chopXHost);
#endif
        chopXHost = 0;
    }

    cudaSetDevice(zDevice);

    if (chopZData) {
        cudaFree(chopZData);
        chopZData = 0;
    }

    if (zDeviceComplexBuffer) {
        cudaFree(zDeviceComplexBuffer);
        zDeviceComplexBuffer = 0;
    }

    if (chopZHost) {
#ifdef USE_SHARED_MEM
        cudaFreeHost(chopZHost);
#else
        free(chopZHost);
#endif
        chopZHost = 0;
    }

    cudaSetDevice(hDevice);

    if (omega) {
        cudaFree(omega);
        omega = 0;
        lastDepth = -1;
    }

    if (displacementData) {
        cudaFree(displacementData);
        displacementData = 0;
    }

    if (slopeFoamData) {
        cudaFree(slopeFoamData);
        slopeFoamData = 0;
    }

    if (foam) {
        cudaFree(foam);
        foam = 0;
    }

    if (displacementResource) {
        cudaGraphicsUnregisterResource(displacementResource);
        displacementResource = 0;
    }

    if (slopeFoamResource) {
        cudaGraphicsUnregisterResource(slopeFoamResource);
        slopeFoamResource = 0;
    }

    return true;
}

void FFTCUDAImpl::Release()
{
    int deviceCount = 0;
    cudaError e = cudaGetDeviceCount(&deviceCount);
    if (e != cudaSuccess) return;

    if (nextAvailableDevice > deviceCount) return;

    e = cudaSetDevice(hDevice);
    if (e != cudaSuccess) return;

    cudaDeviceSynchronize();

    if (omegaCPU) {
        TRITON_DELETE[] omegaCPU;
        omegaCPU = 0;
    }

    if (displacementPBO) {
        glDeleteBuffers(1, &displacementPBO);
        displacementPBO = 0;
    }

    if (slopeFoamPBO) {
        glDeleteBuffers(1, &slopeFoamPBO);
        slopeFoamPBO = 0;
    }

    cudaDeviceSynchronize();

    ReleaseCUDAResources();

    cudaSetDevice(hDevice);
    cudaDeviceSynchronize();
    cudaDeviceReset();

    if (xDevice != hDevice) {
        cudaSetDevice(zDevice);
        cudaDeviceSynchronize();
        cudaDeviceReset();
    }

    if (zDevice != xDevice) {
        cudaSetDevice(hDevice);
        cudaDeviceSynchronize();
        cudaDeviceReset();
    }

    // cudaDeviceSynchronize();

    nextAvailableDevice = hDevice;
}

void FFTCUDAImpl::ComputeOmega(float depth, float loopingPeriod)
{
    if (omegaCPU) {
        unsigned int halfX = dimX / 2;
        unsigned int halfY = dimY / 2;

        float loopingFrequenceReciprocal = float( loopingPeriod / TRITON_TWOPI );
        float *o = omegaCPU;
        for (unsigned int x = 0; x < dimX; x++) {
            for (unsigned int y = 0; y < dimY; y++) {
                float kPosX = (float)x - (float)halfX;
                float kPosY = (float)y - (float)halfY;
                float Kx = ((float)TRITON_TWOPI * kPosX) / sizeX;
                float Ky = ((float)TRITON_TWOPI * kPosY) / sizeY;
                float k = sqrt(Kx * Kx + Ky * Ky);
                float w2 = g * k * tanh(k * depth);
                float w = sqrt(w2);

                if( loopingFrequenceReciprocal > 0.f )
                    w = int( w * loopingFrequenceReciprocal ) / loopingFrequenceReciprocal;

                *o++ = w;
            }
        }
    }
}

bool FFTCUDAImpl::TransformEndToEnd(ComplexNumber *hostH0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed)
{
    cudaError e;

    chopScale = choppiness;

    cudaSetDevice(hDevice);

    bool spraysOn = readFoam && sprayEnabled;

    // GPU->CPU transfers intentionally one frame behind to avoid stalling
    if (spraysOn && foamHost) {
        cudaMemcpyAsync(foamHost, foam, dimX * dimY * sizeof(float), cudaMemcpyDeviceToHost);
    }
    if ((spraysOn || heightTest) && heightHost && heightComputed) {
        cudaMemcpyAsync(heightHost, Hdata, dimX * dimY * sizeof(float), cudaMemcpyDeviceToHost);
        heightCopied = true;
    }
    cudaSetDevice(xDevice);
    if ((spraysOn || heightTest) && chopXHost && heightComputed) {
        cudaMemcpyAsync(chopXHost, chopXData, dimX * dimY * sizeof(float), cudaMemcpyDeviceToHost);
    }
    cudaSetDevice(zDevice);
    if ((spraysOn || heightTest) && chopZHost && heightComputed) {
        cudaMemcpyAsync(chopZHost, chopZData, dimX * dimY * sizeof(float), cudaMemcpyDeviceToHost);
    }
    cudaSetDevice(hDevice);

    if (H0changed) {
        e = cudaMemcpyAsync(H0, hostH0, (dimX + 1) * (dimY + 1) * sizeof(cufftComplex), cudaMemcpyHostToDevice);
        if (e != cudaSuccess) return false;
    }

    if (depth != lastDepth || loopingPeriod != lastLoopingPeriod) {
        ComputeOmega(depth, loopingPeriod);
        lastDepth = depth;
        lastLoopingPeriod = loopingPeriod;
        e = cudaMemcpyAsync(omega, omegaCPU, dimX * dimY * sizeof(float), cudaMemcpyHostToDevice);
        if (e != cudaSuccess) return false;
    }

    uint2 inDim, outDim;
    inDim.x = dimX;
    inDim.y = dimY;
    outDim.x = (dimX / 2) + 1;
    outDim.y = dimY;
    cudaProcessWater((const float *)H0, (const float*)omega, (float *)HdataIn, (float *)chopXDataIn, (float *)chopZDataIn, t, inDim, outDim, gridSize);
    if (xDeviceComplexBuffer) {
        cudaMemcpyPeerAsync(xDeviceComplexBuffer, xDevice, chopXDataIn, hDevice, sizeof(cufftComplex) * (dimX/2+1) * dimY);
    }
    if (zDeviceComplexBuffer) {
        cudaMemcpyPeerAsync(zDeviceComplexBuffer, zDevice, chopZDataIn, hDevice, sizeof(cufftComplex) * (dimX/2+1) * dimY);
    }

    if (!noSync) {
        cudaDeviceSynchronize();
    }

    cufftResult r = cufftExecC2R(planH, HdataIn, Hdata);
    if (r != CUFFT_SUCCESS) return false;

    cudaFlipSignsKernel(Hdata, dimX, dimY);

    cudaSetDevice(xDevice);
    r = cufftExecC2R(planX, xDeviceComplexBuffer ? xDeviceComplexBuffer : chopXDataIn, chopXData);
    if (r != CUFFT_SUCCESS) return false;
    cudaFlipSignsKernel(chopXData, dimX, dimY);


    cudaSetDevice(zDevice);
    r = cufftExecC2R(planZ, zDeviceComplexBuffer ? zDeviceComplexBuffer : chopZDataIn, chopZData);
    if (r != CUFFT_SUCCESS) return false;
    cudaFlipSignsKernel(chopZData, dimX, dimY);

    cudaSetDevice(hDevice);

    heightComputed = true;

    return true;
}

bool FFTCUDAImpl::CreateDirectXTextures()
{
    cudaSetDevice(hDevice);

    cudaCreateTextures(Hdata, chopXData, chopZData, dimX, dimY, displacementData, slopeFoamData, foam, dim, twoCellsSize, chopScale,
                       dt);

    cudaError e = cudaGraphicsMapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsMapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;

    cudaArray *displacementTex, *slopeFoamTex;
    e = cudaGraphicsSubResourceGetMappedArray(&displacementTex, displacementResource, 0, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsSubResourceGetMappedArray(&slopeFoamTex, slopeFoamResource, 0, 0);
    if (e != cudaSuccess) return false;

    int num_texels = dimX * dimY;
    int num_values = num_texels * 4;
    int size_tex_data = sizeof(float) * num_values;
    e = cudaMemcpyToArray(displacementTex, 0, 0, displacementData, size_tex_data, cudaMemcpyDeviceToDevice);
    if (e != cudaSuccess) return false;
    e = cudaMemcpyToArray(slopeFoamTex, 0, 0, slopeFoamData, size_tex_data, cudaMemcpyDeviceToDevice);
    if (e != cudaSuccess) return false;

    e = cudaGraphicsUnmapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsUnmapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;

    return true;
}

bool FFTCUDAImpl::CreateOpenGLTextures()
{
    cudaSetDevice(hDevice);

#ifdef USE_PBO

    cudaError e = cudaGraphicsMapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsMapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;

    cudaCreateTextures(Hdata, chopXData, chopZData, dimX, dimY, dispOut, sfOut, foam, dim, twoCellsSize, chopScale,
                       dt);

    e = cudaGraphicsUnmapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsUnmapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;

    glActiveTexture(GL_TEXTURE2);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, displacementPBO);
    glBindTexture(GL_TEXTURE_2D, displacementGLTexID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    dimX, dimY,
                    GL_RGBA, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE3);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, slopeFoamPBO);
    glBindTexture(GL_TEXTURE_2D, slopeFoamGLTexID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    dimX, dimY,
                    GL_RGBA, GL_FLOAT, NULL);

    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);


#else
    cudaCreateTextures(Hdata, chopXData, chopZData, dimX, dimY, displacementData, slopeFoamData, foam, dim, twoCellsSize, chopScale,
                       dt);
#if 0
    float *displacementTex = TRITON_NEW float[dimX * dimY * 4];
    float *slopeFoamTex = TRITON_NEW float[dimX * dimY * 4];
    cudaMemcpy(displacementTex, displacementData, dimX * dimY * 4 * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(slopeFoamTex, slopeFoamData, dimX * dimY * 4 * sizeof(float), cudaMemcpyDeviceToHost);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, displacementGLTexID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dimX, dimY, GL_RGBA, GL_FLOAT, displacementTex);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, slopeFoamGLTexID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dimX, dimY, GL_RGBA, GL_FLOAT, slopeFoamTex);

    delete[] displacementTex;
    delete[] slopeFoamTex;
#else

    cudaError e = cudaGraphicsMapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsMapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;

    cudaArray *displacementTex, *slopeFoamTex;
    e = cudaGraphicsSubResourceGetMappedArray(&displacementTex, displacementResource, 0, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsSubResourceGetMappedArray(&slopeFoamTex, slopeFoamResource, 0, 0);
    if (e != cudaSuccess) return false;

    int num_texels = dimX * dimY;
    int num_values = num_texels * 4;
    int size_tex_data = sizeof(float) * num_values;
    e = cudaMemcpyToArray(displacementTex, 0, 0, displacementData, size_tex_data, cudaMemcpyDeviceToDevice);
    if (e != cudaSuccess) return false;
    e = cudaMemcpyToArray(slopeFoamTex, 0, 0, slopeFoamData, size_tex_data, cudaMemcpyDeviceToDevice);
    if (e != cudaSuccess) return false;

    e = cudaGraphicsUnmapResources(1, &displacementResource, 0);
    if (e != cudaSuccess) return false;
    e = cudaGraphicsUnmapResources(1, &slopeFoamResource, 0);
    if (e != cudaSuccess) return false;
#endif
#endif
    return true;
}
