// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/TritonOpenGL.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/TritonCommon.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>

#include <fstream>

//#if defined(WIN32) || defined(WIN64)
//#if !defined(DIRECTX9_FOUND)
//#define DIRECTX9_FOUND 1
//#endif
//#else
//#define DIRECTX9_FOUND 0
//#endif

#if DIRECTX9_FOUND==1
#include <d3dx9.h>
#endif

#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#else
#include <cstdio>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined( DEBUG ) || defined( _DEBUG )
// enable getting and clearing OpenGL errors (performance bottleneck)
#define ENABLE_CLEAR_GL_ERRORS  0
// enable getting and printing OpenGL errors (performance bottleneck)
#define ENABLE_PRINT_GL_ERRORS  0
#else
// enable getting and clearing OpenGL errors (performance bottleneck)
#define ENABLE_CLEAR_GL_ERRORS  0
// enable getting and printing OpenGL errors (performance bottleneck)
#define ENABLE_PRINT_GL_ERRORS  0
#endif

// User DebugMsg() handler
static void (*Debug_msg_cb)( const char *  ) = 0;

using namespace Triton;

TRITON_STRING Utils::GetUserShaderFileName()
{
    TRITON_STRING filename;

    const char *baseName = 0;
    if (Configuration::GetStringValue("user-shader-functions-filename", baseName)) {
        filename += baseName;
    } else {
        filename = "user-functions.glsl";
    }

    return filename;
}

TRITON_STRING Utils::GetUserVertShaderFileName()
{
    TRITON_STRING filename;

    const char *baseName = 0;
    if (Configuration::GetStringValue("user-vert-shader-functions-filename", baseName)) {
        filename += baseName;
    } else {
        filename = "user-vert-functions.glsl";
    }

    return filename;
}

TRITON_STRING Utils::GetWaterShaderFileName(const Environment *env, bool tessendorf, bool fragment, bool patch)
{
    TRITON_STRING filename;

    const char * prefix = 0;
    if (Configuration::GetStringValue("shader-filename-prefix", prefix)) {
        filename += prefix;
    }

    if (env->IsOpenGL()) {
        TRITON_STRING base = fragment ? "ocean-frag-" : "projgrid-";
        filename += base;
    }

    if (!(env->IsOpenGL() && !tessendorf && fragment)) {
        filename += env->IsGeocentric() ? "ellipsoid-" : "flat-";
    }

    filename += tessendorf ? "fft" : "gerstner";

    if (env->IsOpenGL()) {
        if (patch && !fragment) {
            filename += "-patch";
        }
    } else {
        if (patch) {
            filename += "-patch";
        }
    }

    filename += env->IsOpenGL() ? ".glsl" : ".fx";

    return filename;
}

TRITON_STRING Utils::GetParticleShaderFileName(const Environment *env, bool fragment)
{
    TRITON_STRING filename;

    const char * prefix = 0;
    if (Configuration::GetStringValue("shader-filename-prefix", prefix)) {
        filename += prefix;
    }

    filename += "particle";

    if (fragment) {
        filename += "-frag";
    }

    filename += env->IsOpenGL() ? ".glsl" : ".fx";

    return filename;
}

TRITON_STRING Utils::GetDecalShaderFileName(const Environment *env, bool fragment)
{
    TRITON_STRING filename;

    const char * prefix = 0;
    if (Configuration::GetStringValue("shader-filename-prefix", prefix)) {
        filename += prefix;
    }

    filename += "decal";

    if (fragment) {
        filename += "-frag";
    }

    filename += env->IsOpenGL() ? ".glsl" : ".fx";

    return filename;
}

TRITON_STRING Utils::GetGodRayShaderFileName(const Environment *env, bool fragment)
{
    TRITON_STRING filename;

    const char * prefix = 0;
    if (Configuration::GetStringValue("shader-filename-prefix", prefix)) {
        filename += prefix;
    }

    filename += "godrays";

    if (fragment) {
        filename += "-frag";
    }

    filename += env->IsOpenGL() ? ".glsl" : ".fx";

    return filename;
}

TRITON_STRING TRITONAPI Triton::Utils::GetHydraxGodRayShaderFileName( const Environment *env, bool fragment )
{
	TRITON_STRING filename;

	const char * prefix = 0;
	if (Configuration::GetStringValue("shader-filename-prefix", prefix)) {
		filename += prefix;
	}

	filename += "hydraxGodrays";

	if (fragment) {
		filename += "-frag";
	}

	filename += env->IsOpenGL() ? ".glsl" : ".fx";

	return filename;
}

void Utils::SetDebugMsgCB( void (*debug_msg_cb)( const char * ) )
{
    Debug_msg_cb = debug_msg_cb;
}

void Utils::DebugMsg(const char *message)
{
    if (!message) return;

    bool debug = true;
    Configuration::GetBoolValue("enable-debug-messages", debug);
    bool alert = false;
    Configuration::GetBoolValue("enable-debug-alert-messages", alert);
    if (debug || alert) {
        TRITON_STRING msg("TRITON: ");
        msg += message;
        msg += "\n";

        if (debug) {
            if ( Debug_msg_cb )
                Debug_msg_cb( message );
            else {
#ifdef _WIN32
                OutputDebugStringA(msg.c_str());
#else
                printf("%s\n", msg.c_str());

                /*std::ofstream stream( "triton-output.txt", std::ios_base::app );

                if( stream.is_open( ) )
                {
                    stream << msg << std::endl;
                    stream.close( );
                }

                stream.close( );*/
#endif
            }
        }

        if (alert) {
#ifdef _WIN32
            MessageBoxA(NULL, msg.c_str(), "Triton", MB_OK | MB_ICONWARNING);
#endif

            /*
            #if defined(__APPLE__) && defined(__clang__)
            CFUserNotificationDisplayAlert(0, kCFUserNotificationPlainAlertLevel, NULL, NULL, NULL, CFSTR("Triton"),
                                           CFStringCreateWithCStringNoCopy(NULL, msg.c_str(), kCFStringEncodingMacRoman, NULL), NULL,
                                           NULL, NULL, NULL);
            #endif
            */
        }
    }
}

void Utils::ClearGLErrors()
{
#if ENABLE_CLEAR_GL_ERRORS
    GLenum err;
    do {
        err = glGetError();
    } while (err != GL_NO_ERROR);
#endif  // ENABLE_CLEAR_GL_ERRORS
}

bool Utils::PrintGLErrors(const char *file, int line)
{
#if ENABLE_PRINT_GL_ERRORS
    GLenum err;
    bool noError = true;

    do {
        err = glGetError();
        if (err != GL_NO_ERROR) {
            noError = false;
#ifdef _WIN32
            char buf[1024];
#if _MSC_VER >= 1400
            sprintf_s(buf, 1024, "GL error in %s line %d:", file, line);
#else
            sprintf(buf, "GL error in %s line %d:", file, line);
#endif
            Utils::DebugMsg(buf);
#else
            char buf[1024];
            sprintf(buf, "GL error in %s line %d:", file, line);
            Utils::DebugMsg(buf);
#endif

#ifndef __APPLE__
            Utils::DebugMsg((const char *)gluErrorString(err));
#endif
        }
    } while (err != GL_NO_ERROR);

    return noError;
#else  // ENABLE_PRINT_GL_ERRORS
    return true;
#endif  // !ENABLE_PRINT_GL_ERRORS
}

TRITON_STRING Utils::GetDLLSuffix()
{
#ifdef _WIN32
#ifdef TRITON_MT
    return "-MT";
#endif
#ifdef TRITON_MT_DLL
    return "-MT-DLL";
#endif
#ifdef TRITON_MTD
    return "-MTD";
#endif
#ifdef TRITON_MTD_DLL
    return "-MTD-DLL";
#endif
#ifdef TRITON_DLL_SUFFIX
#ifdef DEBUG
    if (TRITON_DLL_SUFFIX)
        return "-MTD-DLL";
    else
        return "-MTD";
#else
    if (TRITON_DLL_SUFFIX)
        return "-MT-DLL";
    else
        return "-MT";
#endif
#endif
#endif
    return "";
}

struct _D3DXMACRO *Utils::GetDX9Macros(IDirect3DDevice9 *device, bool hdr, bool breakingWaves) {
#if DIRECTX9_FOUND==1
    if (!device) return NULL;

    D3DCAPS9 caps;
    HRESULT hr = device->GetDeviceCaps(&caps);
    if (FAILED(hr)) return NULL;

    bool hasVS30 = caps.VertexShaderVersion >= D3DVS_VERSION(3, 0);
    bool hasPS30 = caps.PixelShaderVersion >= D3DPS_VERSION(3, 0);

    if (!hasVS30) {
        Utils::DebugMsg("Vertex Shader Model 3.0 is required and was not detected.");
        return NULL;
    }

    static D3DXMACRO pDefines[18];
    memset(pDefines, 0, 5 * sizeof(D3DXMACRO));
    int idx = 0;
    pDefines[idx].Name = "DX9";
    pDefines[idx++].Definition = "1";
    if (hasPS30) {
        pDefines[idx].Name = "PS30";
        pDefines[idx++].Definition = "1";
    }

    if (hasVS30) {
        pDefines[idx].Name = "VS30";
        pDefines[idx++].Definition = "1";
    }

    bool sparkle = false;
    Configuration::GetBoolValue("enable-specular-sparkle", sparkle);
    if (sparkle && hasPS30) {
        pDefines[idx].Name = "SPARKLE";
        pDefines[idx++].Definition = "1";
    }

    bool enableHighAlt = false;
    Configuration::GetBoolValue("high-altitude-effects", enableHighAlt);
    if (enableHighAlt && hasPS30) {
        pDefines[idx].Name = "HIGHALT";
        pDefines[idx++].Definition = "1";
    }

    if (hdr) {
        pDefines[idx].Name = "HDR";
        pDefines[idx++].Definition = "1";
    }

    if (breakingWaves && hasPS30) {
        pDefines[idx].Name = "BREAKING_WAVES";
        pDefines[idx++].Definition = "1";
    }

    int maxWakes = 25;
    Configuration::GetIntValue("max-wake-waves-dx9", maxWakes);
    pDefines[idx].Name = "MAX_KELVIN_WAKES";
    static char buf[32];
#if (_MSC_VER > 1310)
    sprintf_s(buf, 32, "%d", maxWakes);
#else
    sprintf(buf, "%d", maxWakes);
#endif
    pDefines[idx++].Definition = buf;

    int maxCircularWakes = 0;
    Configuration::GetIntValue("max-circular-waves-dx9", maxCircularWakes);
    pDefines[idx].Name = "MAX_CIRCULAR_WAVES";
    static char buf2[32];
#if (_MSC_VER > 1310)
    sprintf_s(buf2, 32, "%d", maxCircularWakes);
#else
    sprintf(buf2, "%d", maxCircularWakes);
#endif
    pDefines[idx++].Definition = buf2;

    int maxPropWakes = 0;
    Configuration::GetIntValue("max-prop-wash-dx9", maxPropWakes);
    pDefines[idx].Name = "MAX_PROP_WASHES";
    static char buf3[32];
#if (_MSC_VER > 1310)
    sprintf_s(buf3, 32, "%d", maxPropWakes);
#else
    sprintf(buf3, "%d", maxPropWakes);
#endif
    pDefines[idx++].Definition = buf3;

    int numOctaves = 3;
    Configuration::GetIntValue("num-detail-octaves", numOctaves);
    pDefines[idx].Name = "NUM_OCTAVES";
    static char buf4[32];
#if (_MSC_VER > 1310)
    sprintf_s(buf4, 32, "%d", numOctaves);
#else
    sprintf(buf4, "%d", numOctaves);
#endif
    pDefines[idx++].Definition = buf4;

    bool enableWash = false;
    Configuration::GetBoolValue("wake-propeller-backwash", enableWash);
    if (enableWash && hasPS30 && maxPropWakes > 0) {
        pDefines[idx].Name = "PROPELLER_WASH";
        pDefines[idx++].Definition = "1";
    }

    bool enableKelvin = false;
    Configuration::GetBoolValue("wake-kelvin-wakes", enableKelvin);
    if (enableKelvin && maxWakes > 0) {
        pDefines[idx].Name = "KELVIN_WAKES";
        pDefines[idx++].Definition = "1";
    }

    bool displacementDetail = false;
    Configuration::GetBoolValue("detail-displacement", displacementDetail);
    if (displacementDetail) {
        pDefines[idx].Name = "DISPLACEMENT_DETAIL";
        pDefines[idx++].Definition = "1";
    }

    bool perFragmentPropWash = false;
    Configuration::GetBoolValue("per-fragment-prop-wash", perFragmentPropWash);
    if (perFragmentPropWash && hasPS30) {
        pDefines[idx].Name = "PER_FRAGMENT_PROP_WASH";
        pDefines[idx++].Definition = "1";
    }

    float displacementFactor = 0.5f;
    Configuration::GetFloatValue("detail-displacement-intensity", displacementFactor);
    static char buf5[256];
    pDefines[idx].Name = "DISPLACEMENT_DETAIL_FACTOR";
#if (_MSC_VER > 1310)
    sprintf_s(buf5, 32, "%f", displacementFactor);
#else
    sprintf(buf5, "%f", displacementFactor);
#endif
    pDefines[idx++].Definition = buf5;

    bool fresnelApproximation = false;
    Configuration::GetBoolValue("fresnel-approximation", fresnelApproximation);
    if (fresnelApproximation || !hasPS30) {
        pDefines[idx].Name = "FAST_FRESNEL";
        pDefines[idx++].Definition = "1";
    }

    return pDefines;
#else
    return NULL;
#endif
}


