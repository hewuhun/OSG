// Copyright (c) 2011 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_FFTCPU_H
#define TRITON_FFTCPU_H

/** \file FFTCPU.h
   \brief An FFT implementation using the FFTSS library.
 */

#include <FeTriton/FFT.h>
#include <FeTriton/fftss.h>

namespace Triton
{
/** An FFT implementation using the FFTSS library. This is the least efficient FFT
    implementation we offer, but as it has no DLL dependencies or special system
    requirements, it's the most reliable one to fall back on as a last resort. */
class FFTCPU : public FFT
{
public:
    FFTCPU() : in(0), out(0), plan(0) {}

    virtual ~FFTCPU();

    /** This always returns true, as the FFTCPU is compatible with anything. */
    static bool IsCompatible();

    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, const Environment *, bool);

    virtual bool InverseTransform2D(ComplexNumber **inputArray, float **outputArray, unsigned int batchSize);

    virtual bool ExploitsConjugate() const {
        return false;
    }

    virtual const char *GetName() const {
        return "CPU-based FFTSS library";
    }

private:
    double *in, *out;
    fftss_plan plan;
};
}

#endif