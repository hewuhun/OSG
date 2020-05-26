// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#ifndef I_FFT_IPP_H
#define I_FFT_IPP_H

/** \file IFFTIPP.h
    \brief A shared interface between Triton and the Intel Integrated Performance Primitives-based FFT DLL.
*/

#if defined(FFTIPP_EXPORT) // inside DLL
#   define FFTIPPAPI  extern "C" __declspec(dllexport)
#else // outside DLL
#   define FFTIPPAPI  extern "C"
#endif  // FFTOPENCL_EXPORT

#include <FeTriton/MemAlloc.h>

namespace Triton
{
class ResourceLoader;
class ComplexNumber;
class Allocator;

/** A shared interface between Triton and the Intel Integrated Performance Primitives-based DLL. */
class IFFTIPP : public MemObject
{
public:
    virtual ~IFFTIPP() {}

    /** Call to release resources at shutdown. */
    virtual void Release() = 0;

    /** Initializes and precomputes the underlying FFT transformations.
        \param Nx   The X dimension of the FFT grid.
        \param Ny   The Y dimension of the FFT grid.
        \param batchSize    The number of grids transformed at once
        \param rl   A ResourceLoader initialized to point to Triton's resources.
        \return Returns true if the FFT transformations initialized successfully.
    */
    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, ResourceLoader *rl) = 0;

    /** Performs an inverse fast Fourier transform of an array of complex arrays to an array of floating
        point real numbers.
        \param inputArray   For an N by M grid of complex numbers, pass in only N by M/2+1 values as we
                            can take advantage of the symmetry of the input data to reduce processing.
                            This is actually an array of N by M/2+1 arrays as we can process multiple
                            arrays at once.
        \param outputArray  An array of N by M arrays of real, floating point values.
        \param batchSize    The number of arrays to transform, reflecting both the number of 2D arrays passed
                            in via inputArray and outputArray.
    */
    virtual bool InverseTransform2D(ComplexNumber **inputArray, float **outputArray, unsigned int batchSize) = 0;

};
}

/// Retrieves the IFFTIPP implementation from the DLL.
FFTIPPAPI Triton::IFFTIPP* GetIFFTIPP(Triton::Allocator *allocator);

typedef Triton::IFFTIPP *    (*GETIFFTIPPPROC)(Triton::Allocator *);

#endif
