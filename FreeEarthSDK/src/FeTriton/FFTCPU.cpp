// Copyright (c) 2011 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/FFTCPU.h>

using namespace Triton;

bool FFTCPU::IsCompatible()
{
    return true;
}

bool FFTCPU::Setup(unsigned int NX, unsigned int NY, unsigned int batches, const Environment *, bool)
{
    dimX = NX;
    dimY = NY;
    batchSize = batches;

    in = (double *)TRITON_MALLOC(NX * NY * sizeof(double) * 2);
    out = (double *)TRITON_MALLOC(NX * NY * sizeof(double) * 2);

    if (!in) {
        Utils::DebugMsg("Unable to allocate memory for FFTCPU input buffer.");
    }

    if (!out) {
        Utils::DebugMsg("Unable to allocate memory for FFTCPU output buffer.");
    }

    // Using FFTSS_MEASURE can speed things up considerably, but sadly it crashes sometimes
    plan = fftss_plan_dft_2d(dimX, dimY, dimX, in, out, FFTSS_BACKWARD, FFTSS_ESTIMATE);

    if (!plan) {
        Utils::DebugMsg("Unable to create FFTSS plan in FFTCPU::Setup.");
    }

    return (in && out && plan);
}

FFTCPU::~FFTCPU()
{
    if (plan) {
        fftss_destroy_plan(plan);
    }

    if (in) {
        TRITON_FREE(in);
    }

    if (out) {
        TRITON_FREE(out);
    }
}

bool FFTCPU::InverseTransform2D(ComplexNumber **inputs, float ** outputs, unsigned int batches)
{
    for (unsigned int batch = 0; batch < batches; batch++) {
        double *di = in;
        ComplexNumber *ci = inputs[batch];
        for (unsigned int i = 0; i < dimX * dimY; i++) {
            *di++ = (*ci).real;
            *di++ = (*ci++).img;
        }

        fftss_execute(plan);

        for (unsigned int i = 0; i < dimY; i++) {
            int rowIdx = i * dimX * 2;
            int rowIdx2 = i * dimX;
            for (unsigned int j = 0; j < dimX; j++) {
                int idx = rowIdx + j * 2;
                int idx2 = rowIdx2 + j;
                outputs[batch][idx2] = (float)(out[idx]);
                if ( (i+j) & 1 ) {
                    outputs[batch][idx2] *= -1.0f;
                }
            }
        }
    }

    return true;
}