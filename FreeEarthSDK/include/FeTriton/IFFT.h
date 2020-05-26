// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_IFFT_H
#define TRITON_IFFT_H

/** \file IFFT.h
   \brief Some common structures for FFT DLL interfaces.
 */

#include <FeTriton/MemAlloc.h>

//#if ((defined(WIN32) || defined(WIN64)) && _MSC_VER >= 1500)
//#if !defined(DIRECTX11_FOUND)
//#define DIRECTX11_FOUND 1
//#endif
//#else
//#define DIRECTX11_FOUND 0
//#endif

#if DIRECTX11_FOUND==1
#include <D3D11.h>
#else
struct ID3D11Device;
#endif

#if DIRECTX9_FOUND==1
#include <d3d9.h>
#else
struct IDirect3DDevice9;
#endif

namespace Triton
{
/** Definitions for the main renderer types supported by Triton. */
enum Renderers {
    FFT_OPENGL = 0,
    FFT_DIRECTX9 = 1,
    FFT_DIRECTX11 = 2,
    FFT_NONE = 3
};

/** Parameters configuring the water simulation and allowing integration
    with our rendering devices, to be passed into the underlying FFT DLL's. */
class FFTSettings : public MemObject
{
public:
    float fftGridSizeX, fftGridSizeY;
    float worldUnits;
    bool enableHeightTests;
    bool enableSpray;
    bool enableMultiGPU;
    float sprayThreshold;
    float gravity;
    Renderers renderer;
    IDirect3DDevice9 *d3d9Device;
    ID3D11Device *d3d11Device;
    bool enableSync;
};
}

#endif
