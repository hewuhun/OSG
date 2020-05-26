// TritonCUDA.cpp : Defines the exported functions for the DLL application.
//

#ifdef _WIN32
#include <FeTritonCUDA/stdafx.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <string.h>
#include <FeTriton/IFFTCUDA.h>
#include <FeTritonCUDA/FFTCUDAImpl.h>
#include <FeTritonCUDA/Export.h>

FFTCUDAAPI Triton::IFFTCUDA* GetIFFTCUDA(Triton::Allocator *alloc)
{
    Triton::Allocator::SetAllocator(alloc);
    Triton::IFFTCUDA *fft = TRITON_NEW Triton::FFTCUDAImpl();
    return fft;
}

FFTCUDAAPI bool IsCompatible()
{
    const GLubyte *str;
    str = glGetString(GL_VENDOR);
    if (str) {
        bool isNVidia = strstr((const char *)str, "NVIDIA") != 0;
        if (!isNVidia) return false;
    } else {
#if DIRECTX9_FOUND==1
        LPDIRECT3D9 g_pD3D = NULL;
        if( NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
            return false;
        D3DADAPTER_IDENTIFIER9 info;
        HRESULT hr = g_pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &info);
        g_pD3D->Release();
        if (FAILED(hr)) return false;
        bool isNVidia = strstr(info.Description, "NVIDIA") != 0;
        if (!isNVidia) return false;
#endif
    }

    int runtimeVersion = 0;
    cudaError e = cudaRuntimeGetVersion(&runtimeVersion);
    if (e != cudaSuccess) return false;
    if (runtimeVersion < 4000) return false;

    int driverVersion = 0;
    e = cudaDriverGetVersion(&driverVersion);
    if (driverVersion < 4000) return false;

    int deviceCount = 0;
    e = cudaGetDeviceCount(&deviceCount);
    if (e != cudaSuccess) return false;

    return (deviceCount > 0);
}