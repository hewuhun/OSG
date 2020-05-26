// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_FFTIPP_H
#define TRITON_FFTIPP_H

/** \file FFTIPP.h
   \brief An FFT implementation using Intel's Integrated Performance Primitives (IPP) library.
 */

#include <FeTriton/FFT.h>
#include <FeTriton/IFFTIPP.h>

namespace Triton
{
class FFTIPP : public FFT
{
public:
    FFTIPP() : iFFTIPP(0) {}

    virtual ~FFTIPP();

    /** Attempts to load the IPP FFT's underlying DLL. Returns false if it could not load for
        any reason, in which case an alternate FFT should be used. */
    static bool LoadDLL(ResourceLoader *rl);

    static bool IsCompatible();

    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, const Environment *, bool);

    virtual bool InverseTransform2D(ComplexNumber **inputArray, float **outputArray, unsigned int batchSize);

    virtual bool ExploitsConjugate() const {
        return true;
    }

    virtual const char *GetName() const {
        return "Intel Integrated Performance Primitives";
    }

private:
    IFFTIPP *iFFTIPP;
};
}

#endif