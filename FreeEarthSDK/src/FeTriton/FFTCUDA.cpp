// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#endif

#include <FeTriton/FFTCUDA.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>
#include <FeTriton/DynamicLibrary.h>
#include <assert.h>

using namespace Triton;

static DynamicLibrary dll;
static GETIFFTCUDAPROC GetIFFtCUDA = 0;

#ifdef WIN32
std::string GetEnvironment()  
{  
	//获取环境变量内存块指针  
	PTSTR pEnvBlock=GetEnvironmentStrings();
	PTSTR pszCurrent = pEnvBlock;  

	bool bFindVersion = false;
	std::string strVersion = "65";
	std::string strCudaWord = "CUDA_PATH_V";
	std::string strCuda = "";

	while(pszCurrent !=NULL)  
	{  
		std::string strCurrentEnvironment = pszCurrent;
		if (strstr(strCurrentEnvironment.c_str(), strCudaWord.c_str()))
		{
			bFindVersion = true;
			strCuda =  strCurrentEnvironment;
			break;
		}

		while(*pszCurrent !=TEXT('\0'))  
			pszCurrent++;  

		//循环后再自增一下跳过0，到达变量的开头部分 
		pszCurrent++; 

		//如果到达以0开头的变量名，说明遍历结束  
		if (*pszCurrent==TEXT('\0'))  break;
	}  

	if (bFindVersion)
	{
		strVersion.assign(strCuda.substr(strCudaWord.size(), 1)+strCuda.substr(strCudaWord.size()+2, 1)) ;
	}

	FreeEnvironmentStrings(pEnvBlock);  

	return strVersion;  
}  
#endif

bool FFTCUDA::LoadDLL(ResourceLoader *rl)
{
    if (!rl) {
        Utils::DebugMsg("Null ResourceLoader passed in to FFTCUDA::LoadDLL.");
        return false;
    }

	DynamicLibrary cudaRT;

	//windows
#ifdef _WIN32
        std::string strCUDAVersion = GetEnvironment();
        TRITON_STRING cudartName;
#ifdef _WIN64
	cudartName = "cudart64_";
	cudaRT.Open(cudartName+strCUDAVersion.c_str()+".dll");
#else
	cudartName = "cudart32_";
	cudaRT.Open(cudartName+strCUDAVersion.c_str()+".dll");
#endif
#else
	//apple
#ifdef __APPLE__
    cudaRT.Open(rl->GetResourceDirPath() + "libcudart.dylib");
#else
    cudaRT.Open(std::string("/usr/lib/x86_64-linux-gnu/") + "libcudart.so");
#endif
#endif

    if (!cudaRT) {
        Utils::DebugMsg("(INFO) CUDA runtime DLL not found; FFTCUDA::LoadDLL fails.");
        return false;
    }
    cudaRT.Close();


    DynamicLibrary cufft;

#ifdef _WIN32
    TRITON_STRING cufftName;
#ifdef _WIN64
	cufftName = "cufft64_";
	cufft.Open(cufftName+strCUDAVersion.c_str()+".dll");
#else
	cufftName = "cufft32_";
	cufft.Open(cufftName+strCUDAVersion.c_str()+".dll");
#endif
#else
#ifdef __APPLE__
    cufft.Open(rl->GetResourceDirPath() + "libcufft.dylib");
#else
    cufft.Open(std::string("/usr/lib/x86_64-linux-gnu/") + "libcufft.so");
#endif
#endif

    if (!cufft) {
        Utils::DebugMsg("(INFO) CUDA CUFFT DLL not found; FFTCUDA::LoadDLL fails.");
        Utils::DebugMsg(cufft.GetErrorMessage().c_str());
        return false;
    }
    cufft.Close();

#ifdef _WIN32
    TRITON_STRING libName = "FeTritonCUDA";
#else
    TRITON_STRING libName = "libFeTritonCUDA";
#endif

    #ifdef _DEBUG
	       libName += "d";
	#else
	#endif
	  
    TRITON_STRING dllPath = libName + Utils::GetDLLSuffix() + Utils::GetDLLExtension();
    dll.Open(dllPath);

    if (dll) {
        GetIFFtCUDA = (GETIFFTCUDAPROC)dll.GetProcAddress("GetIFFTCUDA");
    } else {
        Utils::DebugMsg("(INFO) Failed to load the TritonCUDA dll.");
        Utils::DebugMsg(dll.GetErrorMessage().c_str());
        Utils::DebugMsg("The DLL path we attempted to load was:");
        Utils::DebugMsg(dllPath.c_str());
        return false;
    }

    return true;
}

bool FFTCUDA::IsCompatible(Renderer renderer)
{
    bool disabled = false;
    Configuration::GetBoolValue("disable-cuda", disabled);
    if (disabled) {
        Utils::DebugMsg("(INFO) CUDA explicitly disabled via disable-cuda setting.");
        return false;
    }

    if (dll) {
        ISCOMPATIBLEPROC IsCompatibleProc = (ISCOMPATIBLEPROC)dll.GetProcAddress("IsCompatible");
        if (IsCompatibleProc) {
            if (IsCompatibleProc()) {
                return true;
            } else {
                Utils::DebugMsg("(INFO) CUDA compatibility test failed.");
                return false;
            }
        }
    }

    Utils::DebugMsg("FFTCUDA::IsCompatible failed because no DLL interface was present.");
    return false;
}

bool FFTCUDA::Setup(unsigned int NX, unsigned int NY, unsigned int batches, const Environment *env, bool enableHeightReads)
{
    dimX = NX;
    dimY = NY;

    if (!env) {
        Utils::DebugMsg("Null environment passed in to FFTCUDA::Setup");
        return false;
    }

    if (GetIFFtCUDA) {
        iFFTCUDA = GetIFFtCUDA(Allocator::GetAllocator());
        if (iFFTCUDA) {
            Utils::DebugMsg("(INFO) FFTCUDA DLL successfully loaded.");
        }
    }

    if (iFFTCUDA) {
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
        Configuration::GetBoolValue("fft-enable-multi-gpu", settings.enableMultiGPU);
        settings.gravity = 9.81f;
        Configuration::GetFloatValue("wave-gravity-force", settings.gravity);
        settings.enableSync = true;
        Configuration::GetBoolValue("fft-enable-sync", settings.enableSync);

        switch (env->GetRenderer()) {
        case OPENGL_2_0:
        case OPENGL_3_2:
        case OPENGL_4_0:
        case OPENGL_4_1:
            settings.renderer = FFT_OPENGL;
            break;

        case DIRECTX_9:
        case DIRECT3D9_EX:
            settings.renderer = FFT_DIRECTX9;
            settings.d3d9Device = (IDirect3DDevice9 *)env->GetDevice();
            break;

        case DIRECTX_11:
            settings.renderer = FFT_DIRECTX11;
            settings.d3d11Device = (ID3D11Device *)env->GetDevice();
            break;

        case NO_RENDERER:
            settings.renderer = FFT_NONE;
            break;

        default:
            Utils::DebugMsg("Invalid renderer present in FFTCUDA::Setup");
            return false;
            break;
        }

        return iFFTCUDA->Setup(NX, NY, batches, env->GetResourceLoader(), settings);
    } else {
        Utils::DebugMsg("No valid DLL interface present in FFTCUDA::Setup");
        return false;
    }
}

FFTCUDA::~FFTCUDA()
{
    if (iFFTCUDA) {
        iFFTCUDA->Release();
        TRITON_DELETE iFFTCUDA;
        iFFTCUDA = 0;
    }

    if (dll) {
        dll.Close();
    }
}

bool FFTCUDA::SetOutputTextures(unsigned int displacement, unsigned int slopeFoam)
{
    if (iFFTCUDA) {
        bool ok = iFFTCUDA->SetOutputTextures(displacement, slopeFoam);
        if (!ok) {
            Utils::DebugMsg("IFFTCUDA::SetOutputTextures failed for OpenGL.");
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DebugMsg("No DLL interface present in FFTCUDA::SetOutputTextures for OpenGL");
        return false;
    }
}

bool FFTCUDA::UploadTextures()
{
    if (iFFTCUDA) {
        return iFFTCUDA->UploadTextures();
    } else {
        return false;
    }
}

#if DIRECTX9_FOUND==1
bool FFTCUDA::SetOutputTextures(IDirect3DTexture9 *displacement, IDirect3DTexture9 *slopeFoam)
{
    if (iFFTCUDA) {
        bool ok = iFFTCUDA->SetOutputTextures(displacement, slopeFoam);
        if (!ok) {
            Utils::DebugMsg("IFFTCUDA::SetOutputTextures failed for DX9.");
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DebugMsg("No DLL interface present in FFTCUDA::SetOutputTextures for DX9.");
        return false;
    }
}
#endif

#if DIRECTX11_FOUND==1
bool FFTCUDA::SetOutputTextures(DX11Texture *displacement, DX11Texture *slopeFoam)
{
    if (iFFTCUDA) {
        bool ok = iFFTCUDA->SetOutputTextures(displacement->tex2D, slopeFoam->tex2D);
        if (!ok) {
            Utils::DebugMsg("IFFTCUDA::SetOutputTextures failed for DX11.");
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DebugMsg("No DLL interface present in FFTCUDA::SetOutputTextures for DX11.");
        return false;
    }
}

#endif

bool FFTCUDA::TransformEndToEnd(ComplexNumber *hostH0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed)
{
    if (iFFTCUDA) {
        bool ok = iFFTCUDA->TransformEndToEnd(hostH0, t, dt, choppiness, depth, loopingPeriod, H0changed);
        if (!ok) {
            Utils::DebugMsg("IFFTCUDA::TransformEndToEnd failed.");
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DebugMsg("No DLL interface present in FFTCUDA::TransformEndToEnd");
        return false;
    }
}

#if DIRECTX9_FOUND==1
bool FFTCUDA::D3D9DeviceLost()
{
    if (iFFTCUDA) {
        if (iFFTCUDA->D3D9DeviceLost()) {
            return true;
        } else {
            Utils::DebugMsg("IFFTCUDA::D3D9DeviceLost failed.");
            return false;
        }
    } else {
        Utils::DebugMsg("No DLL interface present in FFTCUDA::D3D9DeviceLost.");
        return false;
    }
}

bool FFTCUDA::D3D9DeviceReset()
{
    if (iFFTCUDA) {
        if (iFFTCUDA->D3D9DeviceReset(settings.d3d9Device)) {
            return true;
        } else {
            Utils::DebugMsg("IFFTCUDA::D3D9DeviceReset failed.");
            return false;
        }
    } else {
        Utils::DebugMsg("No DLL interface present in FFTCUDA::D3D9DeviceReset.");
        return false;
    }
}
#endif

float FFTCUDA::GetHeight(float s, float t, bool hiRes)
{
    if (iFFTCUDA) {
        return iFFTCUDA->GetHeight(s, t, hiRes);
    } else {
        return 0.0f;
    }
}

void FFTCUDA::GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes)
{
    if (iFFTCUDA) {
        return iFFTCUDA->GetChoppinessDisplacement(s, t, chopX, chopZ, hiRes);
    } else {
        chopX = chopZ = 0.0f;
    }
}

const TRITON_VECTOR(Vector3)& FFTCUDA::GetSprays() const
{
    if (iFFTCUDA && environment) {
        return iFFTCUDA->GetSprays(environment->GetOpenMPEnabled());
    } else {
        static TRITON_VECTOR(Vector3) empty;
        return empty;
    }
}

void FFTCUDA::EnableSpray(bool enabled)
{
    if (iFFTCUDA) {
        iFFTCUDA->EnableSpray(enabled);
    }
}
