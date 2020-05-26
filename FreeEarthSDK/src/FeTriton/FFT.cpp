// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/FFT.h>
#include <FeTriton/FFTCPU.h>
#include <FeTriton/FFTIPP.h>
#include <FeTriton/FFTCUDA.h>
#include <FeTriton/FFTComputeShader.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>
#include <FeTriton/ResourceLoader.h>

using namespace Triton;

FFT *FFT::Create(const Environment *env, bool noGPU)
{
    if (!env) {
        Utils::DebugMsg("Null environment received in FFT::Create");
        return 0;
    }

    FFT *instance = 0;

    bool forceCPU = false;

    Configuration::GetBoolValue("fft-force-cpu", forceCPU);
    if (forceCPU) {
        Utils::DebugMsg("(INFO) CPU-based FFT forced by fft-force-cpu setting.");
        instance = TRITON_NEW FFTCPU();
    } else {

        if (!noGPU && FFTCUDA::LoadDLL(env->GetResourceLoader()) && FFTCUDA::IsCompatible(env->GetRenderer())) {
            Utils::DebugMsg("(INFO) Using FFTCUDA.");
            instance = TRITON_NEW FFTCUDA();
            std::cout << "CUDA" << std::endl;
        } else
#if DIRECTX11_FOUND==1
            if (!noGPU && env->GetRenderer() == DIRECTX_11 && FFTComputeShader::LoadDLL(env->GetResourceLoader()) &&
                    FFTComputeShader::IsCompatible((ID3D11Device *)env->GetDevice())) {
                Utils::DebugMsg("(INFO) Using FFTComputeShader.");
                instance = TRITON_NEW FFTComputeShader();
            } else
#endif
                /*if (!noGPU && FFTOpenCL::LoadDLL(env->GetResourceLoader()) && FFTOpenCL::IsCompatible(env->GetRenderer())) 
				{
                    Utils::DebugMsg("(INFO) Using FFTOpenCL.");
                    instance = TRITON_NEW FFTOpenCL();
                }
				else */
				if (FFTIPP::LoadDLL(env->GetResourceLoader()) && FFTIPP::IsCompatible()) 
				{
                    Utils::DebugMsg("(INFO) Using FFTIPP.");
                    instance = TRITON_NEW FFTIPP();
                }
				else if (FFTCPU::IsCompatible()) 
				{
#if defined(WIN32) || defined(WIN64)
                    Utils::DebugMsg("Using FFTCPU, which really shouldn't happen. Did you move Triton's FFT DLL's out "
                                    "of your resources folder?");
#else
                    Utils::DebugMsg("Using FFTCPU");
#endif
                    instance = TRITON_NEW FFTCPU();
                }
    }

    if (!instance) {
        Utils::DebugMsg("No FFT implementation could be created!");
    }

    instance->environment = env;

    return instance;
}

void FFT::GetSlope(float s, float t, float& sx, float &sy, const Environment *env, bool hiRes)
{
    if (!env) return;

    if (!computedSlopeSteps) {
        float gridDimX, gridDimY;
        Configuration::GetFloatValue("fft-grid-size-x", gridDimX);
        Configuration::GetFloatValue("fft-grid-size-y", gridDimY);
        gridDimX /= (float)env->GetWorldUnits();
        gridDimY /= (float)env->GetWorldUnits();

        sInc = 1.0f / (float)dimX;
        tInc = 1.0f / (float)dimY;

        gridSizeX2 = ((float)dimX / gridDimX) * 2.0f;
        gridSizeY2 = ((float)dimY / gridDimY) * 2.0f;

        computedSlopeSteps = true;
    }

    float prevX = s - sInc;
    if (prevX < 0.0f) prevX += 1.0f;
    float nextX = s + sInc;
    if (nextX > 1.0f) nextX -= 1.0f;

    float prevY = t - tInc;
    if (prevY < 0.0f) prevY += 1.0f;
    float nextY = t + tInc;
    if (nextY > 1.0f) nextY -= 1.0f;

    sx = (GetHeight(nextX, t, hiRes) - GetHeight(prevX, t, hiRes)) / (gridSizeX2);
    sy = (GetHeight(s, nextY, hiRes) - GetHeight(s, prevY, hiRes)) / (gridSizeY2);
}
