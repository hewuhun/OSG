// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#endif
#include <FeTriton/TritonCommon.h>
#include <FeTriton/FFTComputeShader.h>
#include <FeTriton/Environment.h>
#include <FeTriton/DynamicLibrary.h>
#include <FeTriton/Configuration.h>
#include <assert.h>
#include <string>

using namespace Triton;

static DynamicLibrary dll;
static GETIFFTCOMPUTESHADERPROC GetIFFtComputeShader = 0;
#ifdef _WIN32
bool FFTComputeShader::LoadDLL(ResourceLoader *rl)
{

    TRITON_STRING dllPath = rl->GetResourceDirPath() + Utils::GetDLLPath() + "TritonComputeShader"
                            + Utils::GetDLLSuffix() + ".dll";
    dll.Open(dllPath);

    if (dll) {
        GetIFFtComputeShader = (GETIFFTCOMPUTESHADERPROC)dll.GetProcAddress("GetIFFTComputeShader");
    } else {
        DWORD err = GetLastError();
        LPSTR buf;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, NULL, (LPSTR)&buf, 0, NULL);
        unsigned int bufLen = (unsigned int)strlen((char *)buf) + 1;
        char *out = TRITON_NEW char[bufLen];
#if _MSC_VER >= 1400
        strcpy_s(out, bufLen, (char *)buf);
#else
        strcpy(out, (char *)buf);
#endif
        Utils::DebugMsg("(INFO) Failed to load the TritonComputeShader dll.");
        Utils::DebugMsg(out);
        Utils::DebugMsg("The DLL path we attempted to load was:");
        Utils::DebugMsg(dllPath.c_str());
        TRITON_DELETE[] out;
        return false;
    }

    return true;
}

#if DIRECTX11_FOUND==1
bool FFTComputeShader::IsCompatible(ID3D11Device *device)
{
    if (device == NULL) {
        Utils::DebugMsg("Null device passed to FFTComputeShader::IsCompatible");
        return false;
    }

    bool disabled = false;
    Configuration::GetBoolValue("disable-compute-shader", disabled);
    if (disabled) {
        Utils::DebugMsg("(INFO) Compute shader explicitly disabled with disable-compute-shader.");
        return false;
    }

    bool ok = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0);

    if (!ok) {
        Utils::DebugMsg("(INFO) Compatibility test for compute shader failed.");
    }

    return ok;
}
#else
bool FFTComputeShader::IsCompatible(ID3D11Device *device)
{
    return false;
}
#endif

bool FFTComputeShader::Setup(unsigned int NX, unsigned int NY, unsigned int batches, const Environment *env, bool enableHeightReads)
{
    dimX = NX;
    dimY = NY;

    if (!env) {
        Utils::DebugMsg("Null environment passed to FFTComputeShader::Setup");
        return false;
    }

    if (dll) {
        if (GetIFFtComputeShader) {
            iFFTComputeShader = GetIFFtComputeShader(Allocator::GetAllocator());
            if (iFFTComputeShader) {
                Utils::DebugMsg("(INFO) Compute Shader DLL loaded.");
            }
        }
    }

    if (iFFTComputeShader) {
        FFTSettings settings;

        Configuration::GetBoolValue("fft-enable-spray", settings.enableSpray);
        settings.enableHeightTests = enableHeightReads;
        settings.fftGridSizeX = 1024.0;
        settings.fftGridSizeY = 1024.0;
        Configuration::GetFloatValue("fft-grid-size-x", settings.fftGridSizeX);
        Configuration::GetFloatValue("fft-grid-size-y", settings.fftGridSizeY);
        settings.fftGridSizeX /= (float)env->GetWorldUnits();
        settings.fftGridSizeY /= (float)env->GetWorldUnits();
        settings.worldUnits = (float)env->GetWorldUnits();
        Configuration::GetFloatValue("fft-spray-threshold", settings.sprayThreshold);
        settings.gravity = 9.81f;
        Configuration::GetFloatValue("wave-gravity-force", settings.gravity);

        settings.renderer = FFT_DIRECTX11;
        settings.d3d11Device= (ID3D11Device *)env->GetDevice();

        bool ok = iFFTComputeShader->Setup(NX, NY, batches, env->GetResourceLoader(), settings);
        if (!ok) {
            Utils::DebugMsg("IFFTComputeShader::Setup failed.");
        }
        return ok;
    } else {
        Utils::DebugMsg("No IFFTComputeShader present in FFTComputeShader::Setup");
        return false;
    }
}

#if DIRECTX11_FOUND == 1
bool FFTComputeShader::SetOutputTextures(DX11Texture *displacementTex, DX11Texture *slopeFoamTex)
{
    if (iFFTComputeShader) {
        bool ok = iFFTComputeShader->SetOutputTextures(displacementTex, slopeFoamTex);
        if (!ok) {
            Utils::DebugMsg("IFFTComputeShader::SetOutputTextures failed.");
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DebugMsg("No IFFTComputeShader present in FFTComputeShader::SetOutputTextures.");
        return false;
    }
}
#endif

FFTComputeShader::~FFTComputeShader()
{
    if (iFFTComputeShader) {
        iFFTComputeShader->Release();
        TRITON_DELETE iFFTComputeShader;
        iFFTComputeShader = 0;
    }

    if (dll) {
        dll.Close();
    }
}

bool FFTComputeShader::TransformEndToEnd(ComplexNumber *hostH0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed)
{
    if (iFFTComputeShader) {
        bool ok = iFFTComputeShader->TransformEndToEnd(hostH0, t, dt, choppiness, depth, loopingPeriod, H0changed);
        if (!ok) {
            Utils::DebugMsg("IFFTComputeShader::TransformEndToEnd failed.");
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DebugMsg("No IFFTComputeShader present in FFTComputeShader::TransformEndtoEnd");
        return false;
    }
}

bool FFTComputeShader::UploadTextures()
{
    if (iFFTComputeShader) {
        return iFFTComputeShader->UploadTextures();
    } else {
        return false;
    }
}

float FFTComputeShader::GetHeight(float s, float t, bool highRes)
{
    if (iFFTComputeShader) {
        return iFFTComputeShader->GetHeight(s, t, highRes);
    } else {
        return 0.0f;
    }
}

void FFTComputeShader::GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes)
{
    if (iFFTComputeShader) {
        return iFFTComputeShader->GetChoppinessDisplacement(s, t, chopX, chopZ, hiRes);
    } else {
        chopX = chopZ = 0.0f;
    }
}

const TRITON_VECTOR(Vector3)& FFTComputeShader::GetSprays() const
{
    if (iFFTComputeShader && environment) {
        return iFFTComputeShader->GetSprays(environment->GetOpenMPEnabled());
    } else {
        static TRITON_VECTOR(Vector3) empty;
        return empty;
    }
}

void FFTComputeShader::EnableSpray(bool enabled)
{
    if (iFFTComputeShader) {
        iFFTComputeShader->EnableSpray(enabled);
    }
}
#endif
