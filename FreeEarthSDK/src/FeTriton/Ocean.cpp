// Copyright (c) 2011-2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/TritonCommon.h>
#include <FeTriton/Ocean.h>
#include <FeTriton/LicenseManager.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/ParticleSystem.h>
#include <FeTriton/Mutex.h>

#include <FeTriton/ProjectedGridOpenGL.h>
#include <FeTriton/OceanLightingModelOpenGL.h>
#include <FeTriton/FFTWaterModelOpenGL.h>
#include <FeTriton/WakeManagerOpenGL.h>
#include <FeTriton/SprayManagerOpenGL.h>
#include <FeTriton/DecalManagerOpenGL.h>
#include <FeTriton/GodRaysOpenGL.h>
#include <osg/Matrix>


#include <algorithm>

//#if defined(WIN32) || defined(WIN64)
//#if !defined(DIRECTX9_FOUND)
//#define DIRECTX9_FOUND 1
//#endif
//#else
//#define DIRECTX9_FOUND 0
//#endif
//
//#if ((defined(WIN32) || defined(WIN64)) && _MSC_VER >= 1500)
//#if !defined(DIRECTX11_FOUND)
//#define DIRECTX11_FOUND 1
//#endif
//#else
//#define DIRECTX11_FOUND 0
//#endif

#if DIRECTX9_FOUND==1
#include "ProjectedGridDirectX9.h"
#include "OceanLightingModelDirectX9.h"
#include "FFTWaterModelDirectX9.h"
#include "WakeManagerDirectX9.h"
#include "SprayManagerDirectX9.h"
#include "DecalManagerDirectX9.h"
#include "GodRaysDirectX9.h"
#endif
#if DIRECTX11_FOUND==1
#include "ProjectedGridDirectX11.h"
#include "OceanLightingModelDirectX11.h"
#include "FFTWaterModelDirectX11.h"
#include "WakeManagerDirectX11.h"
#include "SprayManagerDirectX11.h"
#include "DecalManagerDirectX11.h"
#include "GodRaysDirectX11.h"
#endif

using namespace Triton;

class Triton::OceanImp
{
public:
	OceanImp()
	{
	}
	~OceanImp()
	{
	}

	TRITON_VECTOR(WakeGenerator *) wakeGenerators;

	TRITON_VECTOR(RotorWash *) rotorWashes;

	TRITON_VECTOR(TidalStreamWake *) tidalStreams;

	TRITON_VECTOR(unsigned int) userShaders;
};

Ocean *Ocean::Create(Environment *env, WaterModelTypes type, bool enableHeightReads, bool enableBreakingWaves, OceanQuality quality)
{
    if (!env) {
        Utils::DebugMsg("Null environment passed into Ocean::Create");
        return NULL;
    }

    Ocean *ocean = TRITON_NEW Ocean(env, type, enableHeightReads, enableBreakingWaves, quality);

    if (ocean->initialized) {
        env->RegisterOcean(ocean);
        return ocean;
    } else {
        TRITON_DELETE ocean;
        return NULL;
    }
}

Ocean *Ocean::Create(Environment *env, const TRITON_VECTOR(unsigned int)& shaders, WaterModelTypes type, bool enableHeightReads, bool enableBreakingWaves,
                     OceanQuality quality)
{
    if (!env) {
        Utils::DebugMsg("Null environment passed into Ocean::Create");
        return NULL;
    }

    Ocean *ocean = TRITON_NEW Ocean(env, type, enableHeightReads, enableBreakingWaves, quality, shaders);

    if (ocean->initialized) {
        env->RegisterOcean(ocean);
        return ocean;
    } else {
        TRITON_DELETE ocean;
        return NULL;
    }
}

Ocean::Ocean(Environment *env, WaterModelTypes type, bool enableHeightReads, bool enableBreakingWaves, OceanQuality pQuality, const TRITON_VECTOR(unsigned int)& shaders) :
    isGlobal(true), environment(env),
	godRays(0),
	linearEnabled(false), hasBreakingWaves(enableBreakingWaves),oceanImp(NULL),isCamAboveWater(true)
{
	oceanImp = TRITON_NEW OceanImp();
    oceanImp->userShaders = shaders;
    quality = pQuality;
    Initialize(env, type, enableHeightReads, enableBreakingWaves);
}

Ocean::Ocean(Environment *env, WaterModelTypes type, bool enableHeightReads, bool enableBreakingWaves, OceanQuality pQuality) : isGlobal(true), environment(env),
	godRays(0),
    linearEnabled(false), hasBreakingWaves(enableBreakingWaves), quality(GOOD),oceanImp(NULL),isCamAboveWater(true)
{
	oceanImp = TRITON_NEW OceanImp();
    oceanImp->userShaders.clear();
    quality = pQuality;
    Initialize(env, type, enableHeightReads, enableBreakingWaves);
}

void Ocean::SetGodRaysFade(float fade)
{
    if (fade < 0) fade = 0;
    if (fade > 1.0f) fade = 1.0f;

    if (grid) {
        grid->SetGodRayFade(1.0f - fade);
    }
}

float Ocean::GetGodRaysFade() const
{
    if (grid) {
        return 1.0f - grid->GetGodRayFade();
    }

    return 0.0f;
}

void Ocean::SetLinearColorSpace(bool on)
{
    linearEnabled = on;

    if (grid) {
        if (linearEnabled) {
            grid->SetGamma(1.0f / 2.2f);
        } else {
            grid->SetGamma(1.0f);
        }
    }
}

bool Ocean::GetLinearColorSpace() const
{
    return linearEnabled;
}

bool Ocean::CreateWaterSimulation()
{
    if (!environment) return false;

    particleSystemManager = TRITON_NEW ParticleSystemManager();

#ifdef __APPLE__
    int gridResolution = 257;
    int meshGridResolution = 256;
    Configuration::GetIntValue("default-grid-resolution-mac", gridResolution);
    Configuration::GetIntValue("mesh-grid-resolution-mac", meshGridResolution);
#else
    int gridResolution = 513;
    int meshGridResolution = 512;
    Configuration::GetIntValue("default-grid-resolution", gridResolution);
    Configuration::GetIntValue("mesh-grid-resolution", meshGridResolution);
#endif

    bool autoPerFragmentPropWash = false;
    Configuration::GetBoolValue("auto-per-fragment-prop-wash", autoPerFragmentPropWash);

   /* if (quality > GOOD) {
        Configuration::Set("detail-displacement", "yes");
        if (autoPerFragmentPropWash) {
            Configuration::Set("per-fragment-prop-wash", "yes");
        }
    } else {
        Configuration::Set("detail-displacement", "no");
        if (autoPerFragmentPropWash) {
            Configuration::Set("per-fragment-prop-wash", "no");
        }
    }*/

    if (quality == BETTER) {
        gridResolution *= 2;
        meshGridResolution *= 2;
    }

    if (quality == BEST) {
        gridResolution *= 4;
        meshGridResolution *= 4;
    }

    bool hasGodRays = true;
    Configuration::GetBoolValue("underwater-god-rays-enabled", hasGodRays);

    decalManager = 0;

    if (environment->IsOpenGL()) {
        grid = TRITON_NEW ProjectedGridOpenGL(hasBreakingWaves, oceanImp->userShaders);
        wakeManager = TRITON_NEW WakeManagerOpenGL(environment->GetWorldUnits(), particleSystemManager, oceanImp->userShaders);
        sprayManager = TRITON_NEW SprayManagerOpenGL(environment, oceanImp->userShaders);
        decalManager = TRITON_NEW DecalManagerOpenGL(environment, oceanImp->userShaders);
        //if (hasGodRays) godRays = TRITON_NEW GodRaysOpenGL(environment);
		if (hasGodRays) godRays = TRITON_NEW HydraxGodRaysOpenGL(environment);
    }
#if DIRECTX9_FOUND==1
    else if (environment->GetRenderer() == DIRECTX_9 || environment->GetRenderer() == DIRECT3D9_EX) {
        grid = TRITON_NEW ProjectedGridDirectX9(hasBreakingWaves);
        wakeManager = TRITON_NEW WakeManagerDirectX9(environment->GetWorldUnits(), particleSystemManager);
        sprayManager = TRITON_NEW SprayManagerDirectX9(environment);
        decalManager = TRITON_NEW DecalManagerDirectX9(environment);
        if (hasGodRays) godRays = TRITON_NEW GodRaysDirectX9(environment);
    }
#endif
#if DIRECTX11_FOUND==1
#if (_MSC_VER >= 1500)
    else if (environment->GetRenderer() == DIRECTX_11) {
        grid = TRITON_NEW ProjectedGridDirectX11(hasBreakingWaves);
        wakeManager = TRITON_NEW WakeManagerDirectX11(environment->GetWorldUnits(), particleSystemManager);
        sprayManager = TRITON_NEW SprayManagerDirectX11(environment);
        decalManager = TRITON_NEW DecalManagerDirectX11(environment);
        if (hasGodRays) godRays = TRITON_NEW GodRaysDirectX11(environment);
    }
#endif
#endif
    else if (environment->GetRenderer() == NO_RENDERER) {
        grid = TRITON_NEW ProjectedGrid(hasBreakingWaves);
        wakeManager = TRITON_NEW WakeManager(environment->GetWorldUnits(), particleSystemManager);
        sprayManager = 0;
        godRays = 0;
    } else {
        Utils::DebugMsg("Invalid renderer in Ocean constructor.");
    }

    if (grid) {
        if (!grid->Initialize(environment, waterModelType, gridResolution, meshGridResolution)) {
            Utils::DebugMsg("Failed to initialize projected grid. Did you create your Ocean from a different "
                            "thread than the one you created your graphics context in?");
            return false;
        }
    }

    if (grid) {
        CoordinateSystem cs = environment->GetCoordinateSystem();
        Matrix3 basis = Matrix3::Identity;
        if (cs == FLAT_ZUP || cs == FLAT_YUP) {
            basis = grid->GetBasis();
        }

        if (wakeManager) {
            if (!wakeManager->Initialize(grid, basis, environment)) {
                Utils::DebugMsg("Failed to initialize the wake manager.");
                return false;
            }
        }

        if (sprayManager) {
            if (!sprayManager->Initialize(grid)) {
                Utils::DebugMsg("Failed to initialize the spray manager.");
                return false;
            }
        }
    }

    if (decalManager) {
        if (!decalManager->LoadShaders()) {
            Utils::DebugMsg("Failed to initialize the decal manager.");
            TRITON_DELETE decalManager;
            decalManager = 0;
        }
    }

	if (godRays) {
		if (!godRays->Initialize()) {
			Utils::DebugMsg("Failed to initialize the god rays.");
			TRITON_DELETE godRays;
			godRays = 0;
		}
	}

    WaterModel *wm = NULL;
    OceanLightingModel *lm = NULL;

    if (environment->IsOpenGL()) {
        wm = TRITON_NEW FFTWaterModelOpenGL(environment, waterModelType, quality);
        lm = TRITON_NEW OceanLightingModelOpenGL(environment);
    }
#if DIRECTX9_FOUND==1
    else if (environment->GetRenderer() == DIRECTX_9 || environment->GetRenderer() == DIRECT3D9_EX) {
        wm = TRITON_NEW FFTWaterModelDirectX9(environment, waterModelType, quality);
        lm = TRITON_NEW OceanLightingModelDirectX9(environment);
    }
#endif
#if DIRECTX11_FOUND==1
#if (_MSC_VER >= 1500)
    else if (environment->GetRenderer() == DIRECTX_11) {
        wm = TRITON_NEW FFTWaterModelDirectX11(environment, waterModelType, quality);
        lm = TRITON_NEW OceanLightingModelDirectX11(environment);
    }
#endif
#endif
    else if (environment->GetRenderer() == NO_RENDERER) {
        wm = TRITON_NEW FFTWaterModel(environment, waterModelType, quality);
    }

    if (grid && wm) {
        if (wm->Initialize(grid->GetShader(), grid->GetPatchShader(), heightReadsEnabled, false)) {
            grid->SetWaterModel(wm);
            initialized = true;
            if (lm && wakeManager) {
                if (lm->Initialize(grid->GetShader(), grid->GetPatchShader())) {
                    grid->SetLightingModel(lm);
                    grid->SetWakeManager(wakeManager);
                }
            } else {
                Utils::DebugMsg("Lighting model failed to initialize.");
            }
        } else {
            Utils::DebugMsg("Water model failed to initialize.");
        }
    } else {
        Utils::DebugMsg("Unable to create grid and associated models.");
    }

    return true;
}

void Ocean::DestroyWaterSimulation()
{
    if (grid) TRITON_DELETE grid;

    if (particleSystemManager) TRITON_DELETE particleSystemManager;

    if (wakeManager) TRITON_DELETE wakeManager;

    if (sprayManager) TRITON_DELETE sprayManager;

    if (decalManager) TRITON_DELETE decalManager;

    if (godRays) TRITON_DELETE godRays;

    grid = 0;
    particleSystemManager = 0;
    wakeManager = 0;
    sprayManager = 0;
    decalManager = 0;
    godRays = 0;
}

void Ocean::SetQuality(OceanQuality pQuality)
{
    if (pQuality != quality) {
        quality = pQuality;

        DestroyWaterSimulation();
        CreateWaterSimulation();
    }
}

void Ocean::Initialize(const Environment *env, WaterModelTypes type, bool enableHeightReads, bool enableBreakingWaves)
{
    grid = NULL;
    wakeManager = NULL;
    sprayManager = NULL;
    initialized = false;
    sprayEnabled = true;
    simulationUpdated = false;
    lastUpdateTime = 0;
    heightReadsEnabled = enableHeightReads;
    godRays = NULL;

    enableGodRays = false;
	Configuration::GetBoolValue("underwater-god-rays-enabled", enableGodRays);

    waterModelType = type;

    bool threadSafe = true;
    Configuration::GetBoolValue("thread-safe", threadSafe);

    if (threadSafe) {
        mutex = TRITON_NEW Mutex();
    } else {
        mutex = 0;
    }

    enableKelvin = true;
    Configuration::GetBoolValue("wake-kelvin-wakes", enableKelvin);

    dampingDistance = 1000.0;
    Configuration::GetDoubleValue("fft-displacement-damping-distance", dampingDistance);
    dampingDistance /= environment->GetWorldUnits();

    if (!env) {
        Utils::DebugMsg("Null environment passed into Ocean constructor!");
        return;
    }

    CreateWaterSimulation();
}

Ocean::~Ocean()
{
    TRITON_VECTOR(WakeGenerator *)::iterator it;
    for (it = oceanImp->wakeGenerators.begin(); it != oceanImp->wakeGenerators.end(); it++) {
        WakeGenerator *wg = *it;
        wg->OceanDestroyed();
    }

    TRITON_VECTOR(RotorWash *)::iterator rit;
    for (rit = oceanImp->rotorWashes.begin(); rit != oceanImp->rotorWashes.end(); rit++) {
        RotorWash *rw = *rit;
        rw->OceanDestroyed();
    }

    TRITON_VECTOR(TidalStreamWake *)::iterator tsit;
    for (tsit = oceanImp->tidalStreams.begin(); tsit != oceanImp->tidalStreams.end(); tsit++) {
        TidalStreamWake *tsw = *tsit;
        tsw->OceanDestroyed();
    }

    DestroyWaterSimulation();

    if (environment) {
        const_cast<Environment *>(environment)->UnregisterOcean(this);
    }

	if(oceanImp != NULL)
	{
		TRITON_DELETE oceanImp;
		oceanImp = NULL;
	}

    if (mutex) TRITON_DELETE mutex;
}

DecalHandle Ocean::AddDecal(TextureHandle texture, float size, const Vector3& position)
{
    if (decalManager) {
        Decal *decal = decalManager->CreateDecal(texture, size);
        if (decal) {
            decal->SetPosition(position);
            //decal->Setup();
            decalManager->AddDecal(decal);
        }
        return (DecalHandle)decal;
    } else {
        return 0;
    }
}

void Ocean::SetDecalAlpha(DecalHandle decal, float alpha)
{
    if (decal) {
        Decal *d = (Decal *)decal;
        d->SetAlpha(alpha);
    }
}

void Ocean::ScaleDecal(DecalHandle decal, float sx, float sz)
{
    if (decal) {
        Decal *d = (Decal *)decal;
        d->SetScale(sx, sz);
    }
}

void Ocean::RemoveDecal(DecalHandle decal)
{
    if (decal) {
        Decal *d = (Decal *)decal;
        if (decalManager) {
            decalManager->RemoveDecal(d);
        }
        TRITON_DELETE d;
    }
}

void Ocean::NotifySwellsChanged()
{
    if (grid) {
        WaterModel *wm = grid->GetWaterModel();
        if (wm) {
            FFTWaterModel *fftwm = dynamic_cast<FFTWaterModel*>(wm);
            if (fftwm) {
                fftwm->NotifySwellsChanged();
            }
        }
    }
}

void Ocean::SetConfused(bool)
{
    if (grid) {
        WaterModel *wm = grid->GetWaterModel();
        if (wm) {
            FFTWaterModel *fftwm = dynamic_cast<FFTWaterModel*>(wm);
            if (fftwm) {
                fftwm->NotifySwellsChanged();
            }
        }
    }
}

void Ocean::UpdateHeightMap()
{
    if (grid && heightReadsEnabled) {
        WaterModel *wm = grid->GetWaterModel();
        if (wm) {
            FFTWaterModel *fftwm = dynamic_cast<FFTWaterModel*>(wm);
            if (fftwm) {
                if (mutex) mutex->Lock();
                fftwm->UpdateHeightMap(environment);
                if (mutex) mutex->Unlock();
            }
        }
    }
}

void Ocean::EnvironmentDeleted()
{
    environment = 0;
}

void Ocean::RegisterWakeGenerator(WakeGenerator *wg)
{
    oceanImp->wakeGenerators.push_back(wg);
}

void Ocean::UnregisterWakeGenerator(WakeGenerator *wg)
{
    oceanImp->wakeGenerators.erase(std::remove(oceanImp->wakeGenerators.begin(), oceanImp->wakeGenerators.end(), wg), oceanImp->wakeGenerators.end());
}

void Ocean::RegisterRotorWash(RotorWash *rw)
{
    oceanImp->rotorWashes.push_back(rw);
}

void Ocean::UnregisterRotorWash(RotorWash *rw)
{
    oceanImp->rotorWashes.erase(std::remove(oceanImp->rotorWashes.begin(), oceanImp->rotorWashes.end(), rw), oceanImp->rotorWashes.end());
}

void Ocean::RegisterTidalStreamWake(TidalStreamWake *tsw)
{
    oceanImp->tidalStreams.push_back(tsw);
}

void Ocean::UnregisterTidalStreamWake(TidalStreamWake *tsw)
{
    oceanImp->tidalStreams.erase(std::remove(oceanImp->tidalStreams.begin(), oceanImp->tidalStreams.end(), tsw), oceanImp->tidalStreams.end());
}

void Ocean::SetRefractionColor(const Vector3& color)
{
    if (grid) {
        OceanLightingModel *lm = grid->GetLightingModel();
        if (lm) {
            lm->SetRefractionColor(color);
        }
    }
}

const Vector3& Ocean::GetRefractionColor() const
{
    static Vector3 color;

    if (grid) {
        OceanLightingModel *lm = grid->GetLightingModel();
        if (lm) {
            return lm->GetRefractionColor();
        }
    }

    return color;
}

static double clamp(double x, double min, double max)
{
    if (x <= min) x = min;
    if (x >= max) x = max;

    return x;
}

static float clampf(float x, float min, float max)
{
    if (x <= min) x = min;
    if (x >= max) x = max;

    return x;
}

static double smoothstep(double edge0, double edge1, double x)
{
    double t;
    t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

bool Ocean::ComputeReflectionViewMatrices( Matrix4& reflectionViewMatrix )
{
	if (!environment) return false;

	Vector3 camPos(environment->GetCameraPosition());
	Vector3 direction = environment->GetUpVector() * -1.0;
	Vector3 P;
	if (GetIntersection(camPos, direction, P)) {
		Vector3 V = environment->GetUpVector();

		double pDotV = P.Dot(V);

		double m[16];
		m[0] = 1.0 - 2.0 * V.x * V.x;
		m[4] = -2.0 * V.x * V.y;
		m[8] = -2.0 * V.x * V.z;
		m[12] = 2.0 * pDotV * V.x;

		m[1] = -2.0 * V.x * V.y;
		m[5] = 1.0 - 2.0 * V.y * V.y;
		m[9] = -2.0 * V.y * V.z;
		m[13] = 2.0 * pDotV * V.y;

		m[2] = -2.0 * V.x * V.z;
		m[6] = -2.0 * V.y * V.z;
		m[10] = 1.0 - 2.0 * V.z * V.z;
		m[14] = 2.0 * pDotV * V.z;

		m[3] = 0.0;
		m[7] = 0.0;
		m[11] = 0.0;
		m[15] = 1.0;

		Matrix4 reflect(m);
		reflectionViewMatrix = reflect;
	}
}
bool Ocean::ComputeReflectionMatrices(Matrix4& reflectionMatrix, Matrix3& textureMatrix)
{
    if (!environment) return false;

    Vector3 camPos(environment->GetCameraPosition());
    Vector3 direction = environment->GetUpVector() * -1.0;
    Vector3 P;
    if (GetIntersection(camPos, direction, P)) 
	{
        Vector3 V = environment->GetUpVector();

        double pDotV = P.Dot(V);

        double m[16];
        m[0] = 1.0 - 2.0 * V.x * V.x;
        m[4] = -2.0 * V.x * V.y;
        m[8] = -2.0 * V.x * V.z;
        m[12] = 2.0 * pDotV * V.x;

        m[1] = -2.0 * V.x * V.y;
        m[5] = 1.0 - 2.0 * V.y * V.y;
        m[9] = -2.0 * V.y * V.z;
        m[13] = 2.0 * pDotV * V.y;

        m[2] = -2.0 * V.x * V.z;
        m[6] = -2.0 * V.y * V.z;
        m[10] = 1.0 - 2.0 * V.z * V.z;
        m[14] = 2.0 * pDotV * V.z;

        m[3] = 0.0;
        m[7] = 0.0;
        m[11] = 0.0;
        m[15] = 1.0;

        Matrix4 reflect(m);
        reflectionMatrix = reflect;
	}

    Matrix4 view(environment->GetCameraMatrix());
    view.elem[3][0] = 0.0;
    view.elem[3][1] = 0.0;
    view.elem[3][2] = 0.0;

    Matrix4 proj(environment->GetProjectionMatrix());
    Matrix4 translate;
    if (environment->IsOpenGL()) {
        translate.elem[3][0] = 1.0;
        translate.elem[3][1] = 1.0;
        translate.elem[3][2] = 1.0;
    } else {
        translate.elem[3][0] = 1.0;
        translate.elem[3][1] = 1.0;
        translate.elem[3][2] = 0.0;
    }
    Matrix4 scale;
    if (environment->IsOpenGL()) {
        scale.elem[0][0] = 0.5;
        scale.elem[1][1] = 0.5;
        scale.elem[2][2] = 0.5;
    } else {
        scale.elem[0][0] = 0.5;
        scale.elem[1][1] = -0.5;
        scale.elem[2][2] = 1.0;
    }

    Matrix4 texMat = view * proj * translate * scale;
    textureMatrix = Matrix3(texMat.elem[0][0], texMat.elem[0][1], texMat.elem[0][2],
                            texMat.elem[1][0], texMat.elem[1][1], texMat.elem[1][2],
                            texMat.elem[2][0], texMat.elem[2][1], texMat.elem[2][2]);

    return true;
}

bool Ocean::GetIntersection(const Vector3& point, const Vector3& direction, Vector3& intersection)
{
    if (!grid) return false;

    if (mutex) mutex->Lock();

    bool ok = grid->GetIntersection(point, direction, intersection);

    // Are we over terrain?
    FFTWaterModel *fftwm = dynamic_cast<FFTWaterModel*>(grid->GetWaterModel());
    if (fftwm) {
        if (fftwm->GetDepth(intersection, grid) < 0) {
            if (mutex) mutex->Unlock();
            return false;
        }
    }

    if (mutex) mutex->Unlock();

    return ok;
}

bool Ocean::GetHeight(const Vector3& point, const Vector3& direction, float& height, Vector3& normal, bool visualCorrelation,
                      bool includeWakes, bool highResolution, bool threadSafe, Vector3 *intersectionPoint)
{
    if (!grid) return false;
    if (!environment) return false;

    if (mutex && threadSafe) mutex->Lock();

    Vector3 intersection;
    if (!grid->GetIntersection(point, direction, intersection)) {
        if (!grid->GetIntersection(point, direction * -1, intersection)) {
            if (mutex && threadSafe) mutex->Unlock();
            return false;
        }
    }

    double fade = 1.0;
    if (visualCorrelation && dampingDistance > 0) {
        Vector3 cameraPos(environment->GetCameraPosition());
        double dist = (intersection - cameraPos).Length();
        if (dist > dampingDistance) {
            height = 0;
            normal = environment->GetUpVector();
            if (mutex && threadSafe) mutex->Unlock();
            if (intersectionPoint) {
                *intersectionPoint = intersection;
            }
            return true;
        }
        double invDampingDistance = 1.0 / dampingDistance;
        fade = smoothstep(1.0, 0.0, dist * invDampingDistance);
    }

    WaterModel *wm = grid->GetWaterModel();
    if (!wm) {
        if (mutex && threadSafe) mutex->Unlock();
        return false;
    }

    // Are we over terrain?
    float depth = 1000.0f;

    if (wm->GetFFT()) {
        FFTWaterModel *fftwm = reinterpret_cast<FFTWaterModel*>(wm);
        depth = fftwm->GetDepth(intersection, grid);
        if (depth< 0) {
            if (mutex && threadSafe) mutex->Unlock();
            return false;
        }
    }

    // Account for wave dampening in shallow water
    double visibility;
    Vector3 fogColor;
    float dampening = 1.0f;
    visibility = environment->GetWaveBlendDepth();
    if (visibility > 0) {
        float fogDensityBelow = (float)(3.912 / visibility);
        float fogExponent = fabs(depth) * fogDensityBelow;
        float transparency = clampf(expf(-fabs(fogExponent)), 0.0f, 1.0f);
        float opacity = 1.0f - transparency;
        dampening = powf(opacity, 6.0f);
    }

    Vector3 waveNormal, wakeNormal;
    height = wm->GetHeight(intersection, grid, waveNormal, highResolution);

    if (includeWakes && wakeManager && enableKelvin) {
        height += wakeManager->GetHeight(intersection, grid, wakeNormal);
    }

    height *= dampening;

    height *= (float)fade;

    normal = (wakeNormal + waveNormal);
    normal.Normalize();

    if (intersectionPoint) {
        *intersectionPoint = intersection + (environment->GetUpVector() * height);
    }

    bool isNan = false;
    if (height != height) isNan = true;
    if (normal.x != normal.x || normal.y != normal.y || normal.z != normal.z) isNan = true;

    if (isNan) {
        Utils::DebugMsg("NaN detected in Ocean::GetHeight()!");
    }

    if (mutex && threadSafe) mutex->Unlock();
    return !isNan;
}

void Ocean::Lock()
{
    if (mutex) mutex->Lock();
}

void Ocean::Unlock()
{
    if (mutex) mutex->Unlock();
}

const char *Ocean::GetFFTName() const
{
    if (grid) {
        const WaterModel *wm = grid->GetWaterModel();
        if (wm) {
            return wm->GetFFTName();
        }
    }

    return "No water model found";
}

unsigned int Ocean::GetNumTriangles() const
{
    if (grid) {
        return grid->GetNumTriangles();
    } else {
        return 0;
    }
}

void Ocean::D3D9DeviceLost()
{
    if (environment->GetRenderer() != DIRECTX_9) {
        return;
    }

    if (grid)
        grid->D3D9DeviceLost();
    if (wakeManager)
        wakeManager->D3D9DeviceLost();
    if (sprayManager)
        sprayManager->D3D9DeviceLost();
    if (decalManager)
        decalManager->D3D9DeviceLost();
    if (godRays)
        godRays->D3D9DeviceLost();
}

void Ocean::D3D9DeviceReset()
{
    if (environment->GetRenderer() != DIRECTX_9) {
        return;
    }

    if (grid)
        grid->D3D9DeviceReset();
    if (wakeManager)
        wakeManager->D3D9DeviceReset();
    if (sprayManager)
        sprayManager->D3D9DeviceReset();
    if (decalManager)
        decalManager->D3D9DeviceReset();
    if (godRays)
        godRays->D3D9DeviceReset();
}

void Ocean::EnableWireframe(bool on)
{
    if (grid)
        grid->EnableWireframe(on);
}

float Ocean::GetChoppiness() const
{
    if (grid) {
        return grid->GetChoppiness();
    } else {
        return 0;
    }
}

void Ocean::SetChoppiness(float chop)
{
    if (grid) {
        grid->SetChoppiness(chop);
    }
}

float Ocean::GetLoopingPeriod() const
{
    if (grid) {
        return grid->GetWaterModel()->GetLoopingPeriod();
    } else {
        return 0;
    }
}

void Ocean::SetLoopingPeriod(float loopingPeriod)
{
    if (grid) {
        grid->GetWaterModel()->SetLoopingPeriod(loopingPeriod);
    }
}


float Ocean::GetDepth(Vector3& floorNormal) const
{
    if (grid) {
        return grid->GetDepth(floorNormal);
    } else {
        return 0;
    }
}

void Ocean::SetDepth(float depth, const Vector3& floorNormal)
{
    if (grid) {
        Vector3 n = floorNormal;
        n.Normalize();
        grid->SetDepth(depth, n);
    }

    ComputeWaveHeading();
}

void Ocean::ComputeWaveHeading()
{
    if (!environment || !grid) return;

    double windSpeed, windDirection, fetch;
    environment->GetWind(Vector3(environment->GetCameraPosition()), windSpeed, windDirection, fetch);

    Vector3 floorNormal;
    float depth = GetDepth(floorNormal);

    if (floorNormal == environment->GetUpVector()) {
        waveHeading = (float)windDirection;
    } else {
        float slopeHeading;
        grid->GetSlope(slopeHeading);

        // In shallow water, influence the wave direction by the slope of the seafloor.
        // Otherwise the wind direction dominates.
        float dirBlend = expf(-0.1f * depth);
        if (dirBlend < 0.0f) dirBlend = 0.0f;
        if (dirBlend > 1.0f) dirBlend = 1.0f;

        waveHeading = (dirBlend * slopeHeading) + ((1.0f - dirBlend) * (float)windDirection);
    }

    grid->GetWaterModel()->SetWaveHeading(waveHeading);
}

void Ocean::UpdateSimulation(double time)
{
    if (time != lastUpdateTime) {
        if (mutex) mutex->Lock();

        if (grid) {
            WaterModel *wm = grid->GetWaterModel();
            if (wm) {
                ComputeWaveHeading();
                if (!wm->UpdateSimulation(time)) {
                    // Update failed, fall back to CPU FFT.

                    Utils::DebugMsg("Original FFT implementation failed; recreating water model class without GPGPU support.");
                    TRITON_DELETE wm;

                    if (environment->IsOpenGL()) {
                        wm = TRITON_NEW FFTWaterModelOpenGL(environment, waterModelType, quality);
                    }
#if DIRECTX9_FOUND==1
                    else if (environment->GetRenderer() == DIRECTX_9 || environment->GetRenderer() == DIRECT3D9_EX) {
                        wm = TRITON_NEW FFTWaterModelDirectX9(environment, waterModelType, quality);
                    }
#endif
#if DIRECTX11_FOUND==1
#if (_MSC_VER >= 1500)
                    else if (environment->GetRenderer() == DIRECTX_11) {
                        wm = TRITON_NEW FFTWaterModelDirectX11(environment, waterModelType, quality);
                    }
#endif
#endif
                    else if (environment->GetRenderer() == NO_RENDERER) {
                        wm = TRITON_NEW FFTWaterModel(environment, waterModelType, quality);
                    }

                    if (wm->Initialize(grid->GetShader(), grid->GetPatchShader(), heightReadsEnabled, true)) {
                        grid->SetWaterModel(wm);
                        Utils::DebugMsg("Water model successfully recreated.");
                    }
                }
            }
        }

        if (wakeManager && environment) {
            wakeManager->Update(environment->GetCameraPosition(), time, environment);
        }

        simulationUpdated = true;

        lastUpdateTime = time;

        if (mutex) mutex->Unlock();
    }
}

bool Ocean::ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders, bool reloadSurface, bool reloadWakes, bool reloadSprays, bool reloadDecals, bool reloadGodRays)
{
    bool ok = false;

    if (grid && wakeManager && sprayManager) {
        if (reloadSurface) ok = grid->ReloadShaders(shaders);
        if (reloadWakes) ok = ok && wakeManager->ReloadShaders(shaders, (unsigned int)grid->GetShader(), (unsigned int)grid->GetPatchShader());
        if (reloadSprays) ok = ok && sprayManager->ReloadShaders(shaders);
    }

    if (decalManager) {
        if (reloadDecals) ok = ok && decalManager->ReloadShaders(shaders);
    }

	if (godRays) {
		if (reloadGodRays) ok = ok && godRays->ReloadShaders(shaders);
	}

    return ok;
}

void Ocean::Draw(double time, bool depthWrites, bool drawWater, bool drawParticles)
{
   //LicenseManager::Heartbeat();

	/*if (!simulationUpdated && drawWater) {
	UpdateSimulation(time);
	simulationUpdated = false;
	}

	bool aboveWater = IsCameraAboveWater();*/

    if (mutex) mutex->Lock();

    if (grid) {
        grid->UpdateBasisMatrices();

        grid->PushAllState();
        grid->SetDefaultState();

        if (drawWater) {

            if (decalManager) {
                bool doDepthTexture = decalManager->HasDecals();
                grid->SetGenerateDepthTexture(doDepthTexture);
            }

            //grid->Draw(time, depthWrites, aboveWater);
			grid->Draw(time, depthWrites, isCamAboveWater);

            if (decalManager) {
                decalManager->DrawDecals(grid->GetDepthTexture());
            }
        }

        //if (drawParticles && aboveWater) 
		if (drawParticles && isCamAboveWater) 
		{
            if (particleSystemManager) {
                particleSystemManager->Update(time);
            }

			if (sprayEnabled && sprayManager) {
				sprayManager->Draw(time, waveHeading);
			}
        }

		//if (godRays && enableGodRays && !aboveWater)
		/*if (godRays && enableGodRays && !isCamAboveWater)
		{
		WaterModel *wm = grid->GetWaterModel();
		if (wm) {
		TextureHandle slopeFoam, displacement;
		wm->GetTextures(slopeFoam, displacement);
		godRays->SetTextures(slopeFoam, displacement);
		godRays->Draw(grid);
		}
		}*/

        grid->PopAllState();
    }

    if (mutex) mutex->Unlock();
}

bool Ocean::IsCameraAboveWater()
{
    if (!environment) return false;

    Vector3 cameraPos(environment->GetCameraPosition());
    Vector3 up = environment->GetUpVector();
    Vector3 pt = cameraPos;
    Vector3 intersection;

    bool intersected = GetIntersection(pt, up * -1.0, intersection);
    if (!intersected) {
        intersected = GetIntersection(pt, up, intersection);
    }

    if (intersected)
	{
        Vector3 normal;
        float waveHeight = 0;
        if (GetHeight(intersection + up * 100.0, up * -1.0, waveHeight, normal, true, true, false)) 
		{
            intersection = intersection + up * waveHeight;

			environment->SetCamHeight((cameraPos - intersection).Length());

            CoordinateSystem cs = environment->GetCoordinateSystem();
            if (cs == WGS84_ZUP || cs == WGS84_YUP || cs == SPHERICAL_ZUP || cs == SPHERICAL_YUP) {
                return (cameraPos.SquaredLength() > intersection.SquaredLength());
            }

            if (cs == FLAT_YUP) {
                return cameraPos.y > intersection.y;
            }

            if (cs == FLAT_ZUP) {
                return cameraPos.z > intersection.z;
            }
        }
    }

    return true;
}

bool Ocean::SetPatchShader(double time, int stride, int offset, bool doublePrecision, const double *modelMat, bool depthPass)
{
    bool ok = false;

    if (!depthPass) {
        LicenseManager::Heartbeat();

        if (!simulationUpdated) {
            UpdateSimulation(time);
            simulationUpdated = false;
        }
    } else {
        if (decalManager) {
            bool doDepthTexture = decalManager->HasDecals();
            grid->SetGenerateDepthTexture(doDepthTexture);
        }
    }

    //bool aboveWater = IsCameraAboveWater();
	isCamAboveWater = IsCameraAboveWater();

    if (mutex) mutex->Lock();

    if (wakeManager && environment && !depthPass) {
        wakeManager->Update(environment->GetCameraPosition(), time, environment);
    }

    if (grid) {
        grid->UpdateBasisMatrices();

        Matrix4 modelMatrix;
        if (modelMat) {
            modelMatrix = Matrix4(modelMat);
        }

        grid->PushAllState();

        //ok = grid->SetPatchShader(time, stride, offset, doublePrecision, modelMatrix, aboveWater, depthPass);
		ok = grid->SetPatchShader(time, stride, offset, doublePrecision, modelMatrix, isCamAboveWater, depthPass);

    }

    if (mutex) mutex->Unlock();

    return ok;
}

bool Ocean::SetPatchMatrix(const double *modelMat)
{
    Matrix4 modelMatrix;
    modelMatrix = Matrix4(modelMat);
    grid->SetPatchMatrix(modelMatrix);
    return true;
}

void Ocean::UnsetPatchShader(double time, const TBoundingBox* patchBounds, bool depthPass)
{
    grid->UnsetPatchShader(depthPass);

    //JWH - Added spray for patch shader
    if (depthPass == false && patchBounds != NULL) {
        if (mutex) mutex->Lock();
        if (grid->IsAboveWater()) {
            if (particleSystemManager) {
                particleSystemManager->Update(time);
            }
            if (sprayEnabled && sprayManager) {
                sprayManager->Draw(time, waveHeading, patchBounds);
            }
        }
        if (mutex) mutex->Unlock();
    }



    if (depthPass) {
        if (decalManager) {
            decalManager->DrawDecals(grid->GetDepthTexture());
        }
    }

    grid->PopAllState();
}

ShaderHandle Ocean::GetShaderObject(Shaders shader) const
{
    ShaderHandle object = 0;

    switch (shader) {
    case WATER_SURFACE:
        if (grid) object = grid->GetShader();
        break;

    case WATER_SURFACE_PATCH:
        if (grid) object = grid->GetPatchShader();
        break;

    case SPRAY_PARTICLES:
        if (sprayManager) object = sprayManager->GetShader();
        break;

    case WAKE_SPRAY_PARTICLES:
        if (wakeManager) object = wakeManager->GetShader();
        break;

    case GOD_RAYS:
        if (godRays) object = godRays->GetShader();
        break;

    case WATER_DECALS:
        if (decalManager) object = decalManager->GetShader();
        break;
    }

    return object;
}

void Ocean::SetPlanarReflectionBlend(float blend)
{
    if (grid) {
        grid->SetPlanarReflectionBlend(blend);
    }
}

float Ocean::GetPlanarReflectionBlend() const
{
    if (grid) {
        return grid->GetPlanarReflectionBlend();
    }
    return 0.0f;
}

void Ocean::SetReflectionScale(float scale)
{
    if (grid) {
        grid->SetReflectionScale(scale);
    }
}

float Ocean::GetReflectionScale() const
{
    if (grid) {
        return grid->GetReflectionScale();
    }

    return 1.0f;
}

void Ocean::SetDepthOffset(float offset)
{
    if (grid) {
        grid->SetDepthOffset(offset);
    }
}

float Ocean::GetDepthOffset() const
{
    if (grid) {
        return grid->GetDepthOffset();
    }
    return 0.0f;
}

void Ocean::SetDisplacementDampingDistance(double distance)
{
    dampingDistance = distance;
    if (grid) {
        if (grid->GetWaterModel()) {
            grid->GetWaterModel()->SetDisplacementDampingDistance(distance);
        }
    }
}

double Ocean::GetDisplacementDampingDistance() const
{
    return dampingDistance;
}

void Ocean::EnableSpray(bool enabled)
{
    sprayEnabled = enabled;

    if (grid) {
        if (grid->GetWaterModel()) {
            grid->GetWaterModel()->EnableSpray(enabled);
        }
    }
}

const TRITON_VECTOR(unsigned int)& Ocean::GetUserShaders() const
{
	if(oceanImp != NULL)
		return oceanImp->userShaders;
	else
	{
		static TRITON_VECTOR(unsigned int) empty;
		return empty;
	}
}

bool Triton::Ocean::updateGodRay()
{
	if (godRays && enableGodRays && !IsCameraAboveWater() && environment) 
	{
		Triton::Vector3 sunPos = environment->GetLightDirection() * environment->GetSunDistance();
		const double* cp = environment->GetCameraPosition();
		Triton::Vector3 camPos(cp[0],cp[1],cp[2]);

		Triton::Vector3 sunDir = camPos - sunPos;
		sunDir.Normalize();
		sunPos = camPos - sunDir * 50000;

		Triton::Vector3 hit;
		if(GetIntersection(sunPos,sunDir,hit))
		{
			hit = hit - sunDir * 1;
			float farClip = (camPos - hit).Length() * 1.0;

			osg::Quat q;

			osg::Vec3 rayDir(0,0,-1);
			osg::Vec3 sDir(sunDir.x,sunDir.y,sunDir.z);
			osg::Vec3 axis = rayDir ^ sDir;
			axis.normalize();

			double dotLC = rayDir * sDir;
			dotLC = osg::clampBetween(dotLC,-1.0,1.0);
			double rAngle = acos(dotLC);

			q.makeRotate(rAngle,axis);
			osg::Matrix rotateMat(q);

			osg::Matrix transMat;
			transMat.makeTranslate(osg::Vec3(hit.x,hit.y,hit.z));

			osg::Matrix mat = rotateMat * transMat;

			Triton::Matrix4 tm = Triton::Matrix4(mat(0,0),mat(0,1),mat(0,2),mat(0,3), 
				mat(1,0),mat(1,1),mat(1,2),mat(1,3), 
				mat(2,0),mat(2,1),mat(2,2),mat(2,3), 
				mat(3,0),mat(3,1),mat(3,2),mat(3,3)); 

			godRays->setGodRayPosMat(tm);
			godRays->setGodRayPosition(hit);
			godRays->setFarCilp(farClip);

			godRays->update();

			return true;
		}
	}
	return false;
}

bool TRITONAPI Triton::Ocean::update(double time)
{
	if (!simulationUpdated )
	{
		UpdateSimulation(time);
		simulationUpdated = false;
	}

	isCamAboveWater = IsCameraAboveWater();

	return true;
}

void TRITONAPI Triton::Ocean::DrawGodRay()
{
    if (mutex) mutex->Lock();

    if (grid)
	{
        grid->PushAllState();

		if (godRays && enableGodRays && !isCamAboveWater)
		{
			godRays->Draw(grid);
		}

        grid->PopAllState();
    }

    if (mutex) mutex->Unlock();
}
