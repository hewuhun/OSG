// Copyright (c) 2011 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/FFTIPP.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>
#include <FeTriton/DynamicLibrary.h>
#include <stdlib.h>

using namespace Triton;

static DynamicLibrary dll;

bool FFTIPP::LoadDLL(ResourceLoader *rl)
{
#ifdef _WIN32
    TRITON_STRING libName = "TritonIPP";
#else
    TRITON_STRING libName = "libTritonIPP";
    setenv("KMP_DUPLICATE_LIB_OK", "TRUE", 1);
#endif
    TRITON_STRING dllPath = rl->GetResourceDirPath() + Utils::GetDLLPath() + libName
                            + Utils::GetDLLSuffix() + Utils::GetDLLExtension();
    try {
        dll.Open(dllPath);
    } catch (...) {

    }

    if (!dll) {
        Utils::DebugMsg("(INFO) Failed to load the TritonIPP dll.");
        Utils::DebugMsg(dll.GetErrorMessage().c_str());
        Utils::DebugMsg("The DLL path we attempted to load was:");
        Utils::DebugMsg(dllPath.c_str());
        return false;
    }

    return true;
}

bool FFTIPP::IsCompatible()
{
    bool disabled = false;
    Configuration::GetBoolValue("disable-ipp", disabled);
    if (disabled) {
        Utils::DebugMsg("(INFO) FFTIPP explicitly disabled with disable-ipp setting.");
        return false;
    }

    return true;
}

bool FFTIPP::Setup(unsigned int NX, unsigned int NY, unsigned int batches, const Environment *env, bool)
{
    dimX = NX;
    dimY = NY;

    if (dll) {
        GETIFFTIPPPROC GetIFFtIPP = (GETIFFTIPPPROC)dll.GetProcAddress("GetIFFTIPP");
        if (GetIFFtIPP) {
            iFFTIPP = GetIFFtIPP(Allocator::GetAllocator());
            if (iFFTIPP) {
                Utils::DebugMsg("(INFO) TritonIPP dll successfully loaded.");
            }
        }
    }

    if (iFFTIPP) {
        if (iFFTIPP->Setup(NX, NY, batches, env->GetResourceLoader())) {
            return true;
        } else {
            Utils::DebugMsg("IFFTIPP::Setup failed.");
            return false;
        }
    }

    Utils::DebugMsg("No DLL interface present in FFTIPP::Setup");
    return false;
}

FFTIPP::~FFTIPP()
{
    if (iFFTIPP) {
        iFFTIPP->Release();
        TRITON_DELETE iFFTIPP;
        iFFTIPP = 0;
    }

    if (dll) {
        dll.Close();
    }
}

bool FFTIPP::InverseTransform2D(ComplexNumber **inputs, float ** outputs, unsigned int batches)
{
    if (iFFTIPP) {
        if (iFFTIPP->InverseTransform2D(inputs, outputs, batches)) {
            return true;
        } else {
            Utils::DebugMsg("IFFTIPP::InverseTransform2D failed.");
            return false;
        }
    }

    Utils::DebugMsg("No DLL interface present in FFTIPP::InverseTransform2D");
    return false;
}
