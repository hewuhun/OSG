// Copyright (c) 2004-2015 Sundog Software, LLC All rights reserved worldwide.

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/AtmosphereFromSpace.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Sky.h>
#include <FeSilverliningLib/Ephemeris.h>
#include <FeSilverliningLib/Profiler.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Cloud.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/LicenseManager.h>
#include <FeSilverliningLib/CumulusCloud.h>
#include <FeSilverliningLib/CloudImposter.h>
#include <FeSilverliningLib/PrecipitationManager.h>
#include <FeSilverliningLib/DefaultRandomNumberGenerator.h>
#include <FeSilverliningLib/LensFlare.h>
#include <FeSilverliningLib/ResourceLoader.h>
#include <FeSilverliningLib/CloudBackdropManager.h>
#include <FeSilverliningLib/CrepuscularRays.h>
#include <FeSilverliningLib/ShadowMap.h>
#include <FeSilverliningLib/EnvironmentMap.h>
#include <FeSilverliningLib/Mutex.h>
#include <FeSilverliningLib/Matrix4.h>
#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#if (!defined(WIN32) && !defined(WIN64))
#include <sys/timeb.h>
#endif

using namespace SilverLining;
using namespace std;

ResourceLoader *Atmosphere::resourceLoader = 0;
ResourceLoader *Atmosphere::defaultResourceLoader = 0;
RandomNumberGenerator *Atmosphere::defaultRandomNumberGenerator = 0;
bool Atmosphere::enableHDR = false;
double Atmosphere::unitScale = 1.0;

static Color transmittedLightFromTopOfAtmosphere(1.0, 0.96, 0.94, 1.0);
static int numAtmospheres = 0;
Atmosphere *Atmosphere::currentAtmosphere = 0;

char* Atmosphere::userDefinedVertString = 0;
char* Atmosphere::userDefinedFragString = 0;

Atmosphere::Atmosphere(const char *userName, const char *licenseKey)
{
    conditions = SL_NEW AtmosphericConditions();

    hasLightingOverride = false;
    ambientOverrideR = ambientOverrideG = ambientOverrideB = 0.0f;
    diffuseOverrideR = diffuseOverrideG = diffuseOverrideB = 0.0f;
    randomNumberGenerator = 0;
    sky = 0;
    ephemeris = 0;
    cloudBackdropManager = 0;
    debugMode = false;
    lastTime = 0;
    invalid = false;
    atmosphereFromSpace = 0;
    context = 0;
    hasCameraMatrix = false;
    hasProjectionMatrix = false;
    hasViewport = false;
    hasDepthRange = false;
    infraRed = false;
    disableFarCulling = false;
    enableLensFlare = false;
    enableOcclusionQuery = true;
    lensFlare = 0;
    updated = culled = false;
    firstFrameCalled = false;
    initialized = false;
    crepuscularRays = 0;
    skyCoverage = 0;
    shadowMap = 0;
    insideLoop = false;
    fogOn = false;
    fogDensity = 1E-9;
    fogColor = Vector3(1.0, 1.0, 1.0);
    shadowMapDim = 1024;
    nearDepth = 0.0f;
    farDepth = 1.0f;
    outputScale = 1.0f;

    if (!defaultRandomNumberGenerator) {
        defaultRandomNumberGenerator = SL_NEW DefaultRandomNumberGenerator;
    }
    randomNumberGenerator = defaultRandomNumberGenerator;

    if (!resourceLoader) {
        resourceLoader = defaultResourceLoader = SL_NEW ResourceLoader();
        resourceLoader->SetResourceDirPath(".\\Resources\\");
    }

    numAtmospheres++;

    mutex = 0;
}

Atmosphere::~Atmosphere()
{
    currentAtmosphere = this;

    if (sky) {
        SL_DELETE sky;
    }

    if (lensFlare) {
        SL_DELETE lensFlare;
    }

    if (ephemeris) {
        SL_DELETE ephemeris;
    }

    if (cloudBackdropManager) {
        SL_DELETE cloudBackdropManager;
    }

    if (atmosphereFromSpace) {
        SL_DELETE atmosphereFromSpace;
    }

    if (conditions) {
        SL_DELETE conditions;
    }

    if (crepuscularRays) {
        SL_DELETE crepuscularRays;
    }

    if (shadowMap) {
        SL_DELETE shadowMap;
    }

    if (userDefinedVertString) {
        SL_DELETE userDefinedVertString;
    }

    if (userDefinedFragString) {
        SL_DELETE userDefinedFragString;
    }

    SL_MAP(CameraHandle, EnvironmentMap *)::iterator it;
    for (it = environmentMapMap.begin(); it != environmentMapMap.end(); it++) {
        SL_DELETE it->second;
    }
    environmentMapMap.clear();

    PrecipitationManager::Release(this);

    Billboard::RemoveAtmosphere(this);

    if (context != 0) {
        Renderer::GetInstance()->Shutdown(context);
        context = 0;
    }

    Metaball::ReleaseTexture(this);

    Configuration::Destroy();

    numAtmospheres--;
    if (numAtmospheres <= 0) { // Delete static data when refcount is zero.
        if (defaultResourceLoader) {
            if (resourceLoader == defaultResourceLoader) {
                resourceLoader = 0;
            }
            SL_DELETE defaultResourceLoader;
            defaultResourceLoader = 0;
        }

        if (defaultRandomNumberGenerator) {
            if (randomNumberGenerator == defaultRandomNumberGenerator) {
                randomNumberGenerator = 0;
            }
            SL_DELETE defaultRandomNumberGenerator;
            defaultRandomNumberGenerator = 0;
        }

        Billboard::Destroy();
        Renderer::Destroy();
        Profiler::Destroy();
    }

    if (mutex) {
        SL_DELETE mutex;
    }


}

void Atmosphere::SetSkyModel(SkyModel model)
{
    if (sky) sky->SetSkyModel(model);
}

SkyModel Atmosphere::GetSkyModel() const
{
    if (sky) return sky->GetSkyModel();
    return HOSEK_WILKIE;
}

void Atmosphere::SetSunAlpha(double alpha)
{
    if (sky) sky->SetSunAlpha(alpha);
}

double Atmosphere::GetSunAlpha() const
{
    if (sky) return sky->GetSunAlpha();
    return 1.0;
}

void Atmosphere::SetMoonAlpha(double alpha)
{
    if (sky) sky->SetMoonAlpha(alpha);
}

double Atmosphere::GetMoonAlpha() const
{
    if (sky) return sky->GetMoonAlpha();

    return 1.0;
}

void Atmosphere::OverrideCloudLighting(float ar, float ag, float ab, float dr, float dg, float db)
{
    ambientOverrideR = ar;
    ambientOverrideG = ag;
    ambientOverrideB = ab;
    diffuseOverrideR = dr;
    diffuseOverrideG = dg;
    diffuseOverrideB = db;
    hasLightingOverride = true;
}

void Atmosphere::ClearCloudLightingOverride()
{
    hasLightingOverride = false;
}

void Atmosphere::D3D9DeviceLost()
{
    currentAtmosphere = this;
    // We'll lazily create new environment and shadow maps on demand.

    SL_MAP(CameraHandle, EnvironmentMap*)::iterator it;
    for (it = environmentMapMap.begin(); it != environmentMapMap.end(); it++) {
        SL_DELETE it->second;
    }
    environmentMapMap.clear();

    if (shadowMap) {
        SL_DELETE shadowMap;
        shadowMap = 0;
    }

    if (lensFlare) {
        lensFlare->DeviceLost();
    }

    Renderer::GetInstance()->DeviceLost(context);
}

void Atmosphere::D3D9DeviceReset()
{
    currentAtmosphere = this;

    Renderer::GetInstance()->DeviceReset(context);
}

float Atmosphere::GetSunOcclusion()
{
    if (lensFlare) {
        return lensFlare->GetSunOcclusion();
    }

    return 0;
}

void Atmosphere::SetRandomNumberGenerator(const RandomNumberGenerator *rng)
{
    if (rng) {
        randomNumberGenerator = rng;
    }
}

const RandomNumberGenerator *Atmosphere::GetRandomNumberGenerator() const
{
    return randomNumberGenerator;
}

void Atmosphere::SetCameraMatrix(const double *m)
{
    if (mutex) mutex->Lock();

    if (m) {
        for (int i = 0; i < 16; i++) {
            camera[i] = m[i];
        }

        hasCameraMatrix = true;
    }

    if (mutex) mutex->Unlock();
}

void Atmosphere::SetProjectionMatrix(const double *m)
{
    if (mutex) mutex->Lock();

    if (m) {
        for (int i = 0; i < 16; i++) {
            projection[i] = m[i];
        }

        hasProjectionMatrix = true;
    }

    if (mutex) mutex->Unlock();
}

void Atmosphere::SetViewport(int x, int y, int w, int h)
{
    if (mutex) mutex->Lock();

    viewport[0] = x;
    viewport[1] = y;
    viewport[2] = w;
    viewport[3] = h;

    hasViewport = true;

    if (mutex) mutex->Unlock();
}

bool Atmosphere::GetViewport(int& x, int&y, int& w, int& h) const
{
    if (hasViewport) {
        x = viewport[0];
        y = viewport[1];
        w = viewport[2];
        h = viewport[3];
        return true;
    } else {
        return false;
    }
}

void Atmosphere::SetDepthRange(float n, float f)
{
    if (mutex) mutex->Lock();

    nearDepth = n;
    farDepth = f;

    hasDepthRange = true;

    if (mutex) mutex->Unlock();
}

bool Atmosphere::GetDepthRange(float& n, float& f) const
{
    if (hasDepthRange) {
        n = nearDepth;
        f = farDepth;
        return true;
    } else {
        return false;
    }
}

void Atmosphere::SetUpVector(double x, double y, double z)
{
    currentAtmosphere = this;

    Vector3 up(x, y, z);
    up.Normalize();

    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->SetUpVector(up);
    }
}

void Atmosphere::GetUpVector(double &x, double &y, double &z)
{
    Vector3 up = Renderer::GetInstance()->GetUpVector();

    x = up.x;
    y = up.y;
    z = up.z;
}

void Atmosphere::SetRightVector(double x, double y, double z)
{
    currentAtmosphere = this;

    Vector3 rt(x, y, z);
    rt.Normalize();

    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->SetRightVector(rt);
    }
}

void Atmosphere::GetRightVector(double &x, double &y, double &z)
{
    Vector3 rt = Renderer::GetInstance()->GetRightVector();

    x = rt.x;
    y = rt.y;
    z = rt.z;
}

void Atmosphere::SetConditions(const AtmosphericConditions& pConditions)
{
    if (conditions) {
        *conditions = pConditions;
    }
}

const AtmosphericConditions& Atmosphere::GetConditions() const
{
    currentAtmosphere = const_cast<Atmosphere *>(this);
    return *conditions;
}

int Atmosphere::Initialize(int renderer, const char *resourceDir, bool rightHanded, void *environment)
{
    SL_VECTOR(unsigned int) userShaders;
    return Initialize(renderer, resourceDir, rightHanded, environment, userShaders);
}

int Atmosphere::Initialize(int renderer, const char *resourceDir, bool rightHanded, void *environment, const SL_VECTOR(unsigned int)& userShaders)
{
    currentAtmosphere = this;

    if (resourceDir) {
        resourceLoader->SetResourceDirPath(resourceDir);
        char *data;
        unsigned int dataLen;
        bool loaded = resourceLoader->LoadResource("SilverLining.config", data, dataLen, true);
        if (!loaded) {
            return E_RESOURCES_PATH_INVALID;
        }
        resourceLoader->FreeResource(data);
    }

    //Configuration::Destroy();
    if (!Configuration::Initialize()) {
        return E_CONFIGURATION_FILE_NOT_FOUND;
    }

#ifndef ANDROID
#ifndef OPENGLES
    bool threadSafe = true;
    Configuration::GetBoolValue("enable-thread-safety", threadSafe);
    if (threadSafe) mutex = SL_NEW Mutex;
#endif
#endif

    int result = Renderer::GetInstance()->Initialize(renderer, rightHanded, environment, &context);
    if (result == E_NOERROR) {
        Renderer::GetInstance()->SetUserShaders(userShaders);

        Billboard::SetupShaders();

        sky = SL_NEW Sky();

        ephemeris = SL_NEW Ephemeris();

        lensFlare = SL_NEW LensFlare(ephemeris);

        if (!Metaball::CreateTexture(this)) {
            result = E_CANT_LOAD_METABALL_TEXTURE;
        }

        if (cloudBackdropManager) SL_DELETE cloudBackdropManager;

        bool useBackdrops = false;
        Configuration::GetBoolValue("use-cloud-backdrops", useBackdrops);
        if (useBackdrops) {
            int numBackdrops = 8;
            Configuration::GetIntValue("num-cloud-backdrops", numBackdrops);
            double backdropDistance = 10000.0;
            Configuration::GetDoubleValue("cloud-backdrop-distance", backdropDistance);
            backdropDistance *= unitScale;
            cloudBackdropManager = SL_NEW CloudBackdropManager(numBackdrops, backdropDistance);
            if (!cloudBackdropManager->Initialize()) {
                SL_DELETE cloudBackdropManager;
                cloudBackdropManager = 0;
            }
        }

        bool doAtmosphereFromSpace = true;
        Configuration::GetBoolValue("enable-atmosphere-from-space", doAtmosphereFromSpace);

        if (doAtmosphereFromSpace) {
            atmosphereFromSpace = SL_NEW AtmosphereFromSpace();
            if (atmosphereFromSpace) {
                atmosphereFromSpace->BuildGeometry();
            }
        }

        double T = 2.0;
        Configuration::GetDoubleValue("default-turbidity", T);
        if (conditions) conditions->SetTurbidity(T);

        PrecipitationManager::GetInstance(this)->Initialize(this);

        atmosphereHeight = 100000;
        Configuration::GetDoubleValue("atmosphere-height", atmosphereHeight);
        atmosphereHeight *= unitScale;

        Configuration::GetFloatValue("atmosphere-color-at-top-red", transmittedLightFromTopOfAtmosphere.r);
        Configuration::GetFloatValue("atmosphere-color-at-top-green", transmittedLightFromTopOfAtmosphere.g);
        Configuration::GetFloatValue("atmosphere-color-at-top-blue", transmittedLightFromTopOfAtmosphere.b);
        transmittedLightFromTopOfAtmosphere.ScaleToUnitOrLess();

        applyFogFromCloudPrecip = false;
        Configuration::GetBoolValue("apply-fog-from-cloud-precipitation", applyFogFromCloudPrecip);
        precipMonochrome = false;
        Configuration::GetBoolValue("precipitation-fog-monochrome", precipMonochrome);
    }

    initialized = (result == E_NOERROR);

    return result;
}

bool Atmosphere::ReloadCumulusTextures()
{
    Metaball::ReleaseTexture(this);
    return Metaball::CreateTexture(this);
}

void Atmosphere::SetGamma(double gamma)
{
    if (sky) {
        sky->SetGamma(gamma);
    }
}

double Atmosphere::GetGamma() const
{
    if (sky) {
        return sky->GetGamma();
    }

    return 0;
}

void Atmosphere::GetSunOrMoonPosition(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetSunOrMoonPosition();
    pos.Normalize();

    pos = pos * Renderer::GetInstance()->GetBasis3x3();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetSunOrMoonPositionGeographic(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetSunOrMoonPositionGeographic();
    pos.Normalize();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetSunOrMoonPositionEquatorial(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetSunOrMoonPositionEquatorial();
    pos.Normalize();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetSunPosition(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetSunPosition();
    pos.Normalize();

    pos = pos * Renderer::GetInstance()->GetBasis3x3();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetMoonPosition(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetMoonPosition();
    pos.Normalize();

    pos = pos * Renderer::GetInstance()->GetBasis3x3();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetSunPositionGeographic(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetSunPositionGeographic();
    pos.Normalize();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetMoonPositionGeographic(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetMoonPositionGeographic();
    pos.Normalize();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetSunPositionEquatorial(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetSunPositionEquatorial();
    pos.Normalize();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

void Atmosphere::GetMoonPositionEquatorial(float *x, float *y, float *z) const
{
    if (!sky || !x || !y || !z) return;

    Vector3 pos = sky->GetMoonPositionEquatorial();
    pos.Normalize();

    *x = (float)(pos.x);
    *y = (float)(pos.y);
    *z = (float)(pos.z);
}

Color Atmosphere::Interpolate(const Color& c1, const Color& c2) const
{
    static double spaceFade = 0.5;
    static bool readSpaceFade = false;

    if (!readSpaceFade) {
        Configuration::GetDoubleValue("space-lighting-fadeout", spaceFade);
        readSpaceFade = true;
    }

    double edge = atmosphereHeight + atmosphereHeight * spaceFade;
    double t = (edge - conditions->GetLocation().GetAltitude()) / (atmosphereHeight * spaceFade);
    if (t > 1.0) t = 1.0;
    if (t < 0) t = 0;
    return (c2 * (float)t + c1 * (1.0f - (float)t));
}

void Atmosphere::GetSunOrMoonColor(float *r, float *g, float *b) const
{
    if (sky && r && g && b) {
        Color c = sky->GetSunOrMoonColor();
        c.ScaleToUnitOrLess();

        if (conditions->GetLocation().GetAltitude() > atmosphereHeight) {
            c = Interpolate(transmittedLightFromTopOfAtmosphere, c);
        }

        *r = (float)(c.r);
        *g = (float)(c.g);
        *b = (float)(c.b);
    }
}

void Atmosphere::GetSunColor(float *r, float *g, float *b) const
{
    if (sky && r && g && b) {
        Color c = sky->GetSunColor();
        c.ScaleToUnitOrLess();

        if (conditions->GetLocation().GetAltitude() > atmosphereHeight) {
            c = Interpolate(transmittedLightFromTopOfAtmosphere, c);
        }

        *r = (float)(c.r);
        *g = (float)(c.g);
        *b = (float)(c.b);
    }
}

void Atmosphere::GetMoonColor(float *r, float *g, float *b) const
{
    if (sky && r && g && b) {
        Color c = sky->GetMoonColor();
        c.ScaleToUnitOrLess();

        if (conditions->GetLocation().GetAltitude() > atmosphereHeight) {
            c = Interpolate(transmittedLightFromTopOfAtmosphere, c);
        }

        *r = (float)(c.r);
        *g = (float)(c.g);
        *b = (float)(c.b);
    }
}
void Atmosphere::SetHaze(float hazeR, float hazeG, float hazeB, double hazeDepth, double hazeDensity)
{
    if (hazeR >= 0.0f && hazeG >= 0.0f && hazeB >= 0.0f && hazeDepth >= 0.0 && hazeDensity >= 0.0) {
        currentAtmosphere = this;

        if (sky) {
            Color hazeColor(hazeR, hazeG, hazeB);
            sky->SetHaze(hazeColor, hazeDepth, hazeDensity);
        }
    }
}

void Atmosphere::GetHaze(float& hazeR, float& hazeG, float& hazeB, double& hazeDepth, double& hazeDensity)
{
    if (sky) {
        Color hazeColor;
        sky->GetHaze(hazeColor, hazeDepth, hazeDensity);
        hazeR = (float)(hazeColor.r);
        hazeG = (float)(hazeColor.g);
        hazeB = (float)(hazeColor.b);
    }
}

void Atmosphere::GetAmbientColor(float *r, float *g, float *b) const
{
    if (sky && r && g && b) {
        Color c = sky->GetToneMappedSkyLight();
        c.ScaleToUnitOrLess();

        if (conditions->GetLocation().GetAltitude() > atmosphereHeight) {
            c = Interpolate(Color(0.0f, 0.0f, 0.0f, 1.0f), c);
        }

        *r = (float)(c.r);
        *g = (float)(c.g);
        *b = (float)(c.b);
    }
}

void Atmosphere::GetHorizonColor(float pitchDegrees, float *r, float *g, float *b) const
{
    if (sky && r && g && b) {
        Color c = sky->GetAverageHorizonColor(pitchDegrees);
        c.ScaleToUnitOrLess();
        *r = (float)(c.r);
        *g = (float)(c.g);
        *b = (float)(c.b);
    }
}

void Atmosphere::GetHorizonColor(float yawDegrees, float pitchDegrees, float *r, float *g, float *b) const
{
    if (sky && r && g && b) {
        Color c = sky->GetAverageHorizonColor(yawDegrees, pitchDegrees);
        c.ScaleToUnitOrLess();
        *r = (float)(c.r);
        *g = (float)(c.g);
        *b = (float)(c.b);
    }
}

void Atmosphere::GetZenithColor(float *r, float *g, float *b) const
{
    if (sky && r && g && b) {
        Color color = sky->SkyColorAt(Vector3(0.0, 1.0, 0.0));
        *r = color.r;
        *g = color.g;
        *b = color.b;
    }
}

bool Atmosphere::GetFogEnabled() const
{
    return fogOn;
}

void Atmosphere::GetFogSettings(float *density, float *r, float *g, float *b) const
{
    if (density && r && g && b) {
        *density = (float)fogDensity;

        *r = (float)fogColor.x;
        *g = (float)fogColor.y;
        *b = (float)fogColor.z;
    }
}

void Atmosphere::ShadeClouds()
{
    if (!sky) return;

    currentAtmosphere = this;

    Frustum f = worldFrustum;

    if (disableFarCulling) {
        f.EnableFarClipCulling(false);
    }

    float sx, sy, sz;
    GetSunOrMoonPosition(&sx, &sy, &sz);
    Vector3 sunPos(sx, sy, sz);
    Color sunColor = sky->GetSunOrMoonColorSeaLevel();

    if (hasLightingOverride) sunColor = Color(diffuseOverrideR, diffuseOverrideG, diffuseOverrideB);

    bool wantsUpdate = conditions->WantsLightingUpdate();

    SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();

    SL_MAP(int, CloudLayer *) ::iterator it;
    for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
        Color black(0,0,0);
        (*it).second->Draw(0, &sunPos, infraRed ? &black : &sunColor, invalid,
                           wantsUpdate, conditions->GetMillisecondTimer()->GetMilliseconds(),
                           sky, cloudBackdropManager);
    }

    invalid = false;
}

void Atmosphere::UpdateEphemeris()
{
    currentAtmosphere = this;

    if (ephemeris && conditions) {
        ephemeris->Update(conditions->GetTime(), conditions->GetLocation());
    }

    if (sky && conditions) {
        sky->SetLightPollution(conditions->GetLightPollution());
        sky->SetTurbidity(conditions->GetTurbidity());
        sky->SetEphemeris(ephemeris, conditions->GetLocation().GetAltitude());
        Metaball::SetAmbientColor(infraRed ? Color(0,0,0) : sky->GetToneMappedSkyLightSeaLevel());
        Cloud::SetAmbientColor(infraRed ? Color(0,0,0) : sky->GetToneMappedSkyLightSeaLevel());
        Billboard::SetFogColor(infraRed ? Color(0,0,0) : sky->GetAverageHorizonColor(0.5));
    }
}

void Atmosphere::UpdateSkyAndClouds()
{
    if (!conditions || !firstFrameCalled) return;

    if (mutex) mutex->Lock();

    currentAtmosphere = this;

    if (ephemeris && conditions) {
        ephemeris->Update(conditions->GetTime(), conditions->GetLocation());
    }

    now = conditions->GetMillisecondTimer()->GetMilliseconds();

    dt = 0;
    if (lastTime != 0 && now > lastTime) {
        dt = (long)(now - lastTime) * 0.001;
    }
    lastTime = now;

    conditions->ApplyWind(dt);

    SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
    SL_MAP(int, CloudLayer *) ::iterator it;

    for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
        it->second->DoUpdates(sky, now);
    }

    ComputeCloudBounds();

    updated = true;

    if (mutex) mutex->Unlock();
}

void Atmosphere::CullObjects(bool threadSafe)
{
    if (!firstFrameCalled) return;

    if (mutex && threadSafe) mutex->Lock();

    currentAtmosphere = this;

    Frustum f = worldFrustum;

    if (disableFarCulling) {
        f.EnableFarClipCulling(false);
    }

    SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
    SL_MAP(int, CloudLayer *) ::iterator it;

    for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
        it->second->DoCulling(&f);
    }

    culled = true;

    ComputeSkyCoverage();

    if (mutex && threadSafe) mutex->Unlock();
}

void Atmosphere::GetCloudBounds(double& minX, double& minY, double& minZ,
                                double& maxX, double& maxY, double& maxZ)
{
    minX = minCloudBounds.x;
    minY = minCloudBounds.y;
    minZ = minCloudBounds.z;
    maxX = maxCloudBounds.x;
    maxY = maxCloudBounds.y;
    maxZ = maxCloudBounds.z;
}

void Atmosphere::ComputeSkyCoverage()
{
    currentAtmosphere = this;

    SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
    SL_MAP(int, CloudLayer *) ::iterator it;

    double sunniness = 1.0;
    for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
        CloudLayer *layer = it->second;
        double coverage = layer->ComputeSkyCoverage();
        double percentSun = 1.0 - coverage;
        sunniness *= percentSun;
    }

    skyCoverage = 1.0 - sunniness;
}

void Atmosphere::ComputeCloudBounds()
{
    currentAtmosphere = this;

    maxCloudBounds = Vector3(-DBL_MAX, -DBL_MAX, -DBL_MAX);
    minCloudBounds = Vector3(DBL_MAX, DBL_MAX, DBL_MAX);

    SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
    SL_MAP(int, CloudLayer *) ::iterator it;

    bool foundCloud = false;

    for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
        CloudLayer *layer = it->second;
        if (layer->IsRenderable()) {
            const SL_VECTOR(Cloud*)& clouds = layer->GetClouds();
            SL_VECTOR(Cloud*)::const_iterator cit;
            for (cit = clouds.begin(); cit != clouds.end(); cit++) {
                foundCloud = true;

                Cloud *cloud = *cit;
                double width, depth, height;
                cloud->GetSize(width, depth, height);
                Vector3 size(width, height, depth);
                size = size * Renderer::GetInstance()->GetBasis3x3();
                size.x = fabs(size.x);
                size.y = fabs(size.y);
                size.z = fabs(size.z);
                Vector3 pos = cloud->GetWorldPosition();
                Vector3 min = pos - size;
                Vector3 max = pos + size;

                if (min.x < minCloudBounds.x) minCloudBounds.x = min.x;
                if (min.y < minCloudBounds.y) minCloudBounds.y = min.y;
                if (min.z < minCloudBounds.z) minCloudBounds.z = min.z;
                if (max.x > maxCloudBounds.x) maxCloudBounds.x = max.x;
                if (max.y > maxCloudBounds.y) maxCloudBounds.y = max.y;
                if (max.z > maxCloudBounds.z) maxCloudBounds.z = max.z;
            }
        }
    }

    if (!foundCloud) {
        maxCloudBounds = minCloudBounds = Vector3(0, 0, 0);
    }
}

bool Atmosphere::DrawSky(bool drawSky, bool geocentricMode, double skyboxSize, bool drawStars, bool clearDepth, bool drawSunAndMoon, CameraHandle camHandle)
{
    insideLoop = true;

    if (skyboxSize < 0) return false;

    PrecipitationManager::GetInstance(this)->SetCurrentCamera(camHandle);

    if (lensFlare) lensFlare->SetGeocentricMode(geocentricMode);
    if (crepuscularRays) crepuscularRays->SetGeocentricMode(geocentricMode);

    if (environmentMapMap.find(camHandle) != environmentMapMap.end()) {
        environmentMapMap[camHandle]->SetGeocentricMode(geocentricMode);
    }

    Renderer *ren = Renderer::GetInstance();
    //ren->PushTransforms();
    ren->PushAllState();

    if (hasCameraMatrix && hasProjectionMatrix) {
        ren->SetContext(context);
        Matrix4 modelview(camera);
        Matrix4 proj(projection);
        modelview.Transpose();
        proj.Transpose();
        ren->SetModelviewMatrix(modelview);
        ren->SetProjectionMatrix(proj);
        ren->SetHasExplicitMatrices(true);
    } else {
        ren->SetHasExplicitMatrices(false);
    }

    if (hasViewport) {
        ren->SetViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        ren->SetHasExplicitViewport(true);
    } else {
        ren->SetHasExplicitViewport(false);
    }

    if (hasDepthRange) {
        ren->SetDepthRange(nearDepth, farDepth);
        ren->SetHasExplicitDepthRange(true);
    } else {
        ren->SetHasExplicitDepthRange(false);
    }

    ren->FrameStarted(context);
    firstFrameCalled = true;

    camPos = ren->GetCamPos();
    ren->ExtractViewFrustum(worldFrustum);

    currentAtmosphere = this;

    if (!updated) {
        UpdateSkyAndClouds();
        updated = false;
    }

    if (mutex) mutex->Lock();

    // Turn off any shaders that were left on.
    ren->UnbindShader(VERTEX_PROGRAM);
    ren->UnbindShader(FRAGMENT_PROGRAM);

    ren->SetDefaultState();

    CumulusCloud::FrameStarted();
    CloudImposter::FrameStarted();

    Timer t("Render Atmosphere / Draw Sky");

    if (sky && conditions) {

        sky->SetOvercast(false);

        SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
        SL_MAP(int, CloudLayer *) ::iterator it;

        for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
            it->second->UpdateOvercast(sky);
        }

        sky->SetLightPollution(conditions->GetLightPollution());
        sky->SetTurbidity(conditions->GetTurbidity());
        sky->SetEphemeris(ephemeris, conditions->GetLocation().GetAltitude());
        Color ambient = hasLightingOverride ? Color(ambientOverrideR, ambientOverrideG, ambientOverrideB) : sky->GetToneMappedSkyLightSeaLevel();
        Metaball::SetAmbientColor(infraRed ? Color(0,0,0) : ambient);
        Cloud::SetAmbientColor(infraRed ? Color(0,0,0) : ambient);
        Billboard::SetFogColor(infraRed ? Color(0,0,0) : sky->GetAverageHorizonColor(0));
    }

    double H;
    Configuration::GetDoubleValue("atmosphere-scale-height-meters", H);
    H *= unitScale;
    double isothermalEffect = exp(-(conditions->GetLocation().GetAltitude() / H));
    if (isothermalEffect <= 0) isothermalEffect = 1E-9;
    if (isothermalEffect > 1.0) isothermalEffect = 1.0;
    Billboard::SetFogDensity(1.0 / conditions->GetVisibility() * isothermalEffect);

    // See if the application overrode our fog settings
    bool overrideFog;
    double fogDensity, fogR, fogG, fogB;
    conditions->GetFog(overrideFog, fogDensity, fogR, fogG, fogB);
    if (overrideFog) {
        Billboard::SetFogColor(infraRed ? Color(0,0,0) : Color(fogR, fogG, fogB));
        Billboard::SetFogDensity(fogDensity);
    }

    if (!culled) {
        CullObjects(false);
        culled = false;
    }

    double savedDensity = Billboard::GetFogDensity();
    Billboard::SetFogDensity(1E-9);

    ShadeClouds();

    Billboard::SetFogDensity(savedDensity);

    if (!debugMode) {
        if (clearDepth) {
            ren->ClearDepth();
        }

        //ren->PushAllState();

        if (sky) {
            sky->SetGeocentricMode(geocentricMode);
            if ( drawSky ) {
                sky->Draw(0, skyboxSize, overrideFog, infraRed, drawStars, clearDepth, drawSunAndMoon);
            }
        }

        //ren->PopAllState();

        if (atmosphereFromSpace && !infraRed) {
            Frustum f = worldFrustum;
            if (disableFarCulling) {
                f.EnableFarClipCulling(false);
            }
            atmosphereFromSpace->Draw(f, sky->GetSunPositionGeographic(), Vector3(0,0,0),
                                      conditions->GetLocation().GetAltitude());
        }

        Profiler::GetProfiler()->Display();
    }

    // Turn off any shaders that were left on.
    ren->UnbindShader(VERTEX_PROGRAM);
    ren->UnbindShader(FRAGMENT_PROGRAM);
    ren->PopAllState();
    //ren->PopTransforms();

    if (mutex) mutex->Unlock();

    return true;
}

static bool comp(CloudLayer *c1, CloudLayer *c2)
{
    double x, y, z;
    c1->GetSortPosition(x, y, z);
    Vector3 p1(x, y, z);

    c2->GetSortPosition(x, y, z);
    Vector3 p2(x, y, z);

    double d1 = (Atmosphere::GetCurrentAtmosphere()->GetCamPos() - p1).SquaredLength();
    double d2 = (Atmosphere::GetCurrentAtmosphere()->GetCamPos() - p2).SquaredLength();

    return (d1 > d2);
}

bool Atmosphere::GetShadowMap(void *&texture, Matrix4 *lightViewProjMatrix, Matrix4 *worldToShadowMapTexCoord, bool wholeLayers, float shadowDarkness, bool moonShadows, double maxShadowObjectDistance)
{
    currentAtmosphere = this;

    int newDim = 1024;
    Configuration::GetIntValue("shadow-map-texture-size", newDim);

    if (newDim != shadowMapDim && shadowMap) {
        SL_DELETE shadowMap;
        shadowMap = 0;
    }

    shadowMapDim = newDim;

    if (!shadowMap) {
        shadowMap = SL_NEW ShadowMap(this, shadowMapDim);
    }

    if (shadowMap && sky) {
        Renderer *ren = Renderer::GetInstance();
        ren->PushAllState();

        // Turn off any shaders that were left on.
        ren->UnbindShader(VERTEX_PROGRAM);
        ren->UnbindShader(FRAGMENT_PROGRAM);
        ren->SetDefaultState();

        Vector3 sunPos = sky->GetSunPosition();
        sunPos.Normalize();
        Color sunColor = sky->GetSunColor();

        SL_MAP(int, CloudLayer*)& cloudLayers = conditions->GetCloudLayers();
        SL_MAP(int, CloudLayer *) ::iterator it;
        static SL_VECTOR(CloudLayer *) cloudVector;
        cloudVector.clear();

        for (it = cloudLayers.begin(); it != cloudLayers.end(); it++) {
            if ( (it->second)->IsRenderable()) {
                cloudVector.push_back(it->second);
            }
        }

        ren->PushTransforms();

        shadowMap->SetupShadowMap(sunPos, cloudVector, wholeLayers, maxShadowObjectDistance);

        shadowMap->SetLightMatrices();

        ren->FrameStarted(context);
        CumulusCloud::FrameStarted();
        CloudImposter::FrameStarted();

        Frustum savedFrustum = worldFrustum;
        ren->ExtractViewFrustum(worldFrustum);

        double savedDensity = Billboard::GetFogDensity();
        Billboard::SetFogDensity(1E-9);

        CullObjects();
        culled = false;

        Billboard::SetFogDensity(savedDensity);

        worldFrustum = savedFrustum;

        if (mutex) mutex->Lock();

        currentAtmosphere = this;

        sort(cloudVector.begin(), cloudVector.end(), comp);

        ren->ClearBlendedObjects();

        if (shadowMap->StartDrawing(sky->GetToneMappedSkyLightSeaLevel() * shadowDarkness, moonShadows)) {
            SL_VECTOR(CloudLayer *) ::iterator sit;
            for (sit = cloudVector.begin(); sit != cloudVector.end(); sit++) {
                bool savedFade = (*sit)->GetFadeTowardEdges();

                (*sit)->SetFadeTowardEdges(false);

                // If we encounter a planar cloud type, flush out everything behind it.
                CloudTypes type = (*sit)->GetType();
                bool isPlanar = type == STRATUS || type == CIRRUS_FIBRATUS || type == CIRROCUMULUS || type == STRATOCUMULUS;
                if (isPlanar) {
                    shadowMap->DrawToRenderTexture();
                    ren->ClearBlendedObjects();
                }
                (*sit)->Draw(1, &sunPos, &sunColor, false, false, now, sky, 0);
                if (isPlanar) {
                    shadowMap->DrawToRenderTexture();
                    ren->ClearBlendedObjects();
                }

                (*sit)->SetFadeTowardEdges(savedFade);
            }

            shadowMap->DrawToRenderTexture();
            ren->ClearBlendedObjects();

            shadowMap->EndDrawing();
        }

        ren->UnbindShader(VERTEX_PROGRAM);
        ren->UnbindShader(FRAGMENT_PROGRAM);

        if (mutex) mutex->Unlock();

        ren->PopTransforms();
        ren->PopAllState();

        texture = Renderer::GetInstance()->GetNativeTexture(shadowMap->GetTextureHandle());
        *lightViewProjMatrix = shadowMap->GetLightModelviewProjectionMatrix();
        *worldToShadowMapTexCoord = shadowMap->GetWorldToShadowTexCoordMatrix();

        if (ren->GetIsDirectX()) {
            lightViewProjMatrix->Transpose();
            worldToShadowMapTexCoord->Transpose();
        }

        if (insideLoop) {
            ren->FrameStarted(context);
            CumulusCloud::FrameStarted();
            CloudImposter::FrameStarted();
            CullObjects();
            culled = false;
        }

        return true;
    }
    return false;
}

void Atmosphere::DrawLensFlare()
{
    Renderer *ren = Renderer::GetInstance();

    //ren->PushTransforms();
    ren->PushAllState();

    // Turn off any shaders that were left on.
    ren->UnbindShader(VERTEX_PROGRAM);
    ren->UnbindShader(FRAGMENT_PROGRAM);
    ren->SetDefaultState();

    if (hasCameraMatrix && hasProjectionMatrix) {
        Matrix4 modelview(camera);
        Matrix4 proj(projection);
        modelview.Transpose();
        proj.Transpose();
        ren->SetModelviewMatrix(modelview);
        ren->SetProjectionMatrix(proj);
        ren->SetHasExplicitMatrices(true);
    } else {
        ren->SetHasExplicitMatrices(false);
    }

    if (hasViewport) {
        ren->SetViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        ren->SetHasExplicitViewport(true);
    } else {
        ren->SetHasExplicitViewport(false);
    }

    if (lensFlare) {
        lensFlare->Update();
        lensFlare->Draw();
    }

    // Turn off any shaders that were left on.
    ren->UnbindShader(VERTEX_PROGRAM);
    ren->UnbindShader(FRAGMENT_PROGRAM);

    ren->PopAllState();
    //ren->PopTransforms();
}

bool Atmosphere::DrawObjects(bool drawClouds, bool drawPrecipitation, bool enableDepthTest, float crepuscularRaysIntensity,
                             bool enableDepthWrites, CameraHandle camHandle, bool cullClockWise, bool drawBackdrops)
{
    insideLoop = false;

    if (debugMode) return true;

    Timer t("Render Atmosphere / Draw Objects");

    PrecipitationManager::GetInstance(this)->SetCurrentCamera(camHandle);

    Renderer *ren = Renderer::GetInstance();

    if (ren) {
        if (infraRed) crepuscularRaysIntensity = 0;

        if (conditions->GetCloudLayers().size() == 0) crepuscularRaysIntensity = 0;

        if (skyCoverage >= 1.0) crepuscularRaysIntensity = 0;

        //ren->PushTransforms();
        ren->PushAllState();

        ren->BackfaceCullClockwise(cullClockWise);

        // Turn off any shaders that were left on.
        ren->UnbindShader(VERTEX_PROGRAM);
        ren->UnbindShader(FRAGMENT_PROGRAM);
        ren->SetDefaultState();

        if (hasCameraMatrix && hasProjectionMatrix) {
            Matrix4 modelview(camera);
            Matrix4 proj(projection);
            modelview.Transpose();
            proj.Transpose();
            ren->SetModelviewMatrix(modelview);
            ren->SetProjectionMatrix(proj);
            ren->SetHasExplicitMatrices(true);
        } else {
            ren->SetHasExplicitMatrices(false);
        }

        if (hasViewport) {
            ren->SetViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            ren->SetHasExplicitViewport(true);
        } else {
            ren->SetHasExplicitViewport(false);
        }

        if (crepuscularRaysIntensity > 0) {
            if (!crepuscularRays) {
                crepuscularRays = SL_NEW CrepuscularRays(this);
            }
            if (crepuscularRays) {
                crepuscularRays->SetExposure(crepuscularRaysIntensity);
                crepuscularRays->UpdateLightSource(ephemeris, sky->GetSunColor(), skyCoverage);
            }
        }

        if (mutex) mutex->Lock();

        currentAtmosphere = this;

        Frustum f = worldFrustum;

        if (disableFarCulling) {
            f.EnableFarClipCulling(false);
        }

        // Need to sort cloud layers back to front from camera
        SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
        SL_MAP(int, CloudLayer *) ::iterator it;
        // th changed (avoid new/deletes)
        static SL_VECTOR(CloudLayer *) sortedVector;
        //sortedVector.reserve(cloudLayer.size());
        sortedVector.clear();

        for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
            if ( (it->second)->IsRenderable()) {
                sortedVector.push_back(it->second);
            }
        }

        sort(sortedVector.begin(), sortedVector.end(), comp);

        if (sky) {

            //sky->SetOvercast(false);
            sky->SetFogVolumeDistance(0);
            ren->EnableFog(false);

            Vector3 sunPos = sky->GetSunOrMoonPosition();
            sunPos.Normalize();
            Color sunColor = sky->GetSunOrMoonColorSeaLevel();
            if (hasLightingOverride) {
                sunColor = Color(diffuseOverrideR, diffuseOverrideG, diffuseOverrideB);
            }

            PrecipitationManager::GetInstance(this)->ClearPrecipitation();

            conditions->ApplyPrecipitation();

            SL_VECTOR(CloudLayer *) ::iterator sit;

            if (applyFogFromCloudPrecip) {
                for (sit = sortedVector.begin(); sit != sortedVector.end(); sit++) {
                    if ( (*sit)->HasPrecipitationAtPosition(camPos.x, camPos.y, camPos.z ) ) {
                        (*sit)->ApplyPrecipitation();
                    }
                }
            }

            // apply fog from precipitation
            Color precipFogColor = sky->GetAverageHorizonColor(0);
            if (precipMonochrome) {
                precipFogColor.r = precipFogColor.b = precipFogColor.g;
            }
            double precipitationFogDensity = PrecipitationManager::GetInstance(this)->GetFogDensity();

            if (precipitationFogDensity > 0) {
                ren->EnableFog(true);
                ren->ConfigureFog(precipitationFogDensity, 0, 0, infraRed ? Color(0,0,0) : precipFogColor);
                Billboard::SetFogColor(infraRed ? Color(0,0,0) : precipFogColor);
                Billboard::SetFogDensity(precipitationFogDensity);
                sky->SetFogVolumeDistance(1.0 / precipitationFogDensity);
            }

            for (sit = sortedVector.begin(); sit != sortedVector.end(); sit++) {
                if (sky) {
                    (*sit)->ProcessAtmosphericEffects(sky);
                }
            }

            Color black(0,0,0);
            if (cloudBackdropManager && drawClouds && drawBackdrops) {
                cloudBackdropManager->Update(f, sortedVector, now, *sky, sunPos, infraRed ? black : sunColor);
                cloudBackdropManager->Draw(f);
            }

            for (sit = sortedVector.begin(); sit != sortedVector.end(); sit++) {
                // If we encounter a planar cloud type, flush out everything behind it.
                CloudTypes type = (*sit)->GetType();
                bool isPlanar = type == STRATUS || type == CIRRUS_FIBRATUS || type == CIRROCUMULUS || type == STRATOCUMULUS;
                if (isPlanar && drawClouds) {
                    ren->SortAndDrawBlendedObjects(enableDepthTest, enableDepthWrites);
                    if (crepuscularRays && crepuscularRaysIntensity > 0) {
                        crepuscularRays->DrawToRenderTexture();
                    }

                    ren->ClearBlendedObjects();
                }

                if (!applyFogFromCloudPrecip) {
                    if ( (*sit)->HasPrecipitationAtPosition(camPos.x, camPos.y, camPos.z ) ) {
                        (*sit)->ApplyPrecipitation();
                    }
                }

                (*sit)->ProcessFade(now);

                (*sit)->Draw(1, &sunPos, infraRed ? &black : &sunColor, false, false, now,
                             sky, cloudBackdropManager);

                if (drawClouds) {
                    if (isPlanar) {
                        ren->SortAndDrawBlendedObjects(enableDepthTest, enableDepthWrites);
                        ren->ClearBlendedObjects();
                    }
                }
            }

            if (drawClouds) {
                ren->SortAndDrawBlendedObjects(enableDepthTest, enableDepthWrites);
                if (crepuscularRays && crepuscularRaysIntensity > 0) {
                    crepuscularRays->DrawToRenderTexture();
                }
            }

            if (drawPrecipitation) {
                Color nLightColor = sunColor;
                nLightColor.ScaleToUnitOrLess();
                nLightColor = nLightColor + Metaball::GetAmbientColor();
                nLightColor.ClampToUnitOrLess();

                PrecipitationManager::GetInstance(this)->Render(dt, f, infraRed ? Color(0,0,0) : nLightColor);
            }
        }

        sortedVector.clear();

        if (crepuscularRays && crepuscularRaysIntensity > 0) {
            crepuscularRays->DrawRays();
        }

        if (lensFlare) {
            if (enableLensFlare || enableOcclusionQuery) {
                lensFlare->Update();
            }

            if (enableLensFlare) {
                lensFlare->Draw();
            }
        }

        // Turn off any shaders that were left on.
        ren->UnbindShader(VERTEX_PROGRAM);
        ren->UnbindShader(FRAGMENT_PROGRAM);

        ren->BackfaceCullClockwise(true);

        CalculateFramerate();

        ren->PopAllState();
        //ren->PopTransforms();

        fogOn = ren->GetFogEnabled();
        fogDensity = ren->GetFogDensity();
        fogColor = ren->GetFogColor().ToVector3();

        if (mutex) mutex->Unlock();
    }

    return true;
}

bool Atmosphere::GetEnvironmentMap(void *& texture, int facesToRender, bool floatingPoint, CameraHandle cameraID, bool drawClouds, bool drawSunAndMoon)
{
    bool ok = false;

    currentAtmosphere = this;

    Renderer *ren = Renderer::GetInstance();
    //ren->PushTransforms();
    ren->PushAllState();

    Frustum savedFrustum = worldFrustum;

    bool savedFogOn = fogOn;
    double savedFogDensity = fogDensity;
    Vector3 savedFogColor = fogColor;
    Color savedBillboardColor = Billboard::GetFogColor();
    double savedBillboardDensity = Billboard::GetFogDensity();
    double savedRenderDensity, savedRenderStart, savedRenderEnd;
    Color savedRenderColor;
    ren->GetFog(savedRenderDensity, savedRenderStart, savedRenderEnd, savedRenderColor);

    EnvironmentMap *environmentMap = 0;

    if (environmentMapMap.find(cameraID) != environmentMapMap.end()) {
        environmentMap = environmentMapMap[cameraID];
    }

    if (!environmentMap) {
        environmentMap = SL_NEW EnvironmentMap(this);
        if (sky) {
            environmentMap->SetGeocentricMode(sky->GetGeocentricMode());
        }
        environmentMapMap[cameraID] = environmentMap;
    }

    if (environmentMap) {
        ok = environmentMap->Create(facesToRender, floatingPoint, drawClouds, drawSunAndMoon);
        texture = environmentMap->GetNativeTexture();
    }

    worldFrustum = savedFrustum;

    fogColor = savedFogColor;
    fogOn = savedFogOn;
    fogDensity = savedFogDensity;
    Billboard::SetFogColor(savedBillboardColor);
    Billboard::SetFogDensity(savedBillboardDensity);
    ren->ConfigureFog(savedRenderDensity, savedRenderStart, savedRenderEnd, savedRenderColor);

    ren->PopAllState();
    //ren->PopTransforms();

    return ok;
}

void Atmosphere::CalculateFramerate()
{
#if defined(WIN32) || defined(WIN64)

    static LARGE_INTEGER frequency, start, end;
    static time_t lastSnapshot = 0;

    time_t tnow = time(NULL);

    if (frequency.QuadPart) {
        QueryPerformanceCounter(&end);

        if (tnow > lastSnapshot) {
            double seconds = (double)(end.QuadPart - start.QuadPart) / (double)frequency.QuadPart;

            framerateValue = 1.0 / seconds;

            lastSnapshot = tnow;
        }
    }

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

#else

    static time_t lastSeconds = 0;
    static unsigned short lastMillis = 0;
    static time_t lastSnapshot = 0;

    struct timeb tp;
    ftime(&tp);

    if (tp.time > lastSnapshot) {
        double elapsed = (tp.time - lastSeconds) + ((tp.millitm - lastMillis) * 0.001);
        framerateValue = 1.0 / elapsed;
        lastSnapshot = tp.time;
    }

    lastSeconds = tp.time;
    lastMillis = tp.millitm;
#endif
}

SL_VECTOR(ObjectHandle)& Atmosphere::GetObjects() const
{
    const SL_VECTOR(Renderable *)& objs = Renderer::GetInstance()->GetBlendedObjects();

    static SL_VECTOR(ObjectHandle) ret;

    ret.clear();
    SL_VECTOR(Renderable *) ::const_iterator it;
    for (it = objs.begin(); it != objs.end(); it++) {
        ret.push_back((ObjectHandle)*it);
    }

    return ret;
}

void Atmosphere::ClearObjects()
{
    Renderer::GetInstance()->ClearBlendedObjects();
}

float Atmosphere::GetObjectDistance(ObjectHandle obj, float ox, float oy, float oz, float sx, float sy, float sz)
{
    currentAtmosphere = this;

    Renderable *r = (Renderable *)obj;

    Vector3 pos(ox, oy, oz);
    Vector3 sortPos(sx, sy, sz);

    Renderer *ren = Renderer::GetInstance();
    Matrix4 modelview, proj, modelviewProj;
    ren->GetModelviewMatrix(&modelview);
    ren->GetProjectionMatrix(&proj);
    modelviewProj = proj * modelview;

    return (float)r->GetDistance(sortPos, pos, modelviewProj, ren->GetIsRightHanded()) + (float)r->GetDepthOffset();
}

float Atmosphere::GetObjectDistance(ObjectHandle obj, ObjectHandle obj2, float sx, float sy, float sz)
{
    currentAtmosphere = this;

    Renderable *r = (Renderable *)obj;

    Vector3 pos;
    if (obj2) {
        Renderable *r2 = (Renderable *)obj2;
        pos = r2->GetWorldPosition();
    } else {
        pos = r->GetWorldPosition();
    }

    Vector3 sortPos(sx, sy, sz);

    Renderer *ren = Renderer::GetInstance();
    Matrix4 modelview, proj, modelviewProj;
    ren->GetModelviewMatrix(&modelview);
    ren->GetProjectionMatrix(&proj);
    modelviewProj = proj * modelview;

    return (float)r->GetDistance(sortPos, pos, modelviewProj, ren->GetIsRightHanded()) + (float)r->GetDepthOffset();
}

void Atmosphere::GetObjectPosition(ObjectHandle obj, float& x, float& y, float& z)
{
    currentAtmosphere = this;

    if (obj) {
        Renderable *r = (Renderable *)obj;
        Vector3 pos = r->GetWorldPosition();
        x = (float)pos.x;
        y = (float)pos.y;
        z = (float)pos.z;
    }
}

void Atmosphere::DrawObject(ObjectHandle obj)
{
    currentAtmosphere = this;

    Renderable *r = (Renderable *)obj;

    r->DrawBlendedObject();
}

void Atmosphere::ReloadConfigFile()
{
    currentAtmosphere = this;
    Configuration::Clear();
    Configuration::Load("SilverLining.config");
    Configuration::Load("SilverLining.override");
}

void Atmosphere::SetConfigOption(const char *key, const char *value)
{
    currentAtmosphere = this;
    if (key && value) {
        Configuration::Set(key, value);
    }
}

const char *Atmosphere::GetConfigOptionString(const char *key) const
{
    if (!key) return 0;

    const char * sval;
    if (Configuration::GetStringValue(key, sval)) {
        return sval;
    } else {
        return 0;
    }
}

double Atmosphere::GetConfigOptionDouble(const char *key) const
{
    if (!key) return 0;

    double val = 0;
    Configuration::GetDoubleValue(key, val);
    return val;
}

bool Atmosphere::GetConfigOptionBoolean(const char *key) const
{
    if (!key) return false;

    bool val = 0;
    Configuration::GetBoolValue(key, val);
    return val;
}

bool Atmosphere::Serialize(std::ostream& s)
{
    currentAtmosphere = this;

    s.write((char *)&debugMode, sizeof(bool));
    s.write((char *)&infraRed, sizeof(bool));
    s.write((char *)&disableFarCulling, sizeof(bool));
    s.write((char *)&enableLensFlare, sizeof(bool));
    s.write((char *)&enableOcclusionQuery, sizeof(bool));
    s.write((char *)&enableHDR, sizeof(bool));
    s.write((char *)&unitScale, sizeof(double));

    Vector3 up, right;
    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        up = ren->GetUpVector();
        right = ren->GetRightVector();
    }
    up.Serialize(s);
    right.Serialize(s);

    conditions->Serialize(s);

    return true;
}

bool Atmosphere::Unserialize(std::istream& s)
{
    currentAtmosphere = this;

    s.read((char *)&debugMode, sizeof(bool));
    s.read((char *)&infraRed, sizeof(bool));
    s.read((char *)&disableFarCulling, sizeof(bool));
    s.read((char *)&enableLensFlare, sizeof(bool));
    s.read((char *)&enableOcclusionQuery, sizeof(bool));
    s.read((char *)&enableHDR, sizeof(bool));
    s.read((char *)&unitScale, sizeof(double));

    Vector3 up, right;
    up.Unserialize(s);
    right.Unserialize(s);
    SetUpVector(up.x, up.y, up.z);
    SetRightVector(right.x, right.y, right.z);

    conditions->Unserialize(this, s);

    return true;
}

SL_VECTOR(unsigned int) Atmosphere::GetActivePlanarCloudShaders()
{
    currentAtmosphere = this;

    Renderer *ren = Renderer::GetInstance();
    SL_VECTOR(unsigned int) shaders;

    SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
    SL_MAP(int, CloudLayer*) ::iterator it;
    for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
        ShaderHandle shader = it->second->GetShader();
        if (shader) {
            if (ren) shaders.push_back(ren->GetShaderProgram(shader));
        }
    }

    return shaders;
}

unsigned int Atmosphere::GetBillboardShader() const
{
    ShaderHandle handle = Billboard::GetShader();
    return Renderer::GetInstance()->GetShaderProgram(handle);
}

unsigned int Atmosphere::GetSkyShader() const
{
    if (sky) {
        ShaderHandle handle = sky->GetSkyShader();
        return Renderer::GetInstance()->GetShaderProgram(handle);
    }

    return 0;
}

unsigned int Atmosphere::GetStarShader() const
{
    if (sky) {
        ShaderHandle handle = sky->GetStarShader();
        return Renderer::GetInstance()->GetShaderProgram(handle);
    }

    return 0;
}

unsigned int Atmosphere::GetPrecipitationShader() const
{
    ShaderHandle handle = PrecipitationManager::GetShaderProgram();
    if (handle) {
        return Renderer::GetInstance()->GetShaderProgram(handle);
    }

    return 0;
}

void Atmosphere::ReloadShaders(const SL_VECTOR(unsigned int)& userShaders)
{
    currentAtmosphere = this;

    Renderer::GetInstance()->SetUserShaders(userShaders);

    if (sky) {
        sky->ReloadShaders();
    }

    Billboard::ReloadShaders();

    PrecipitationManager::ReloadShader();

    SL_MAP(int, CloudLayer*)& cloudLayer = conditions->GetCloudLayers();
    SL_MAP(int, CloudLayer*) ::iterator it;
    for (it = cloudLayer.begin(); it != cloudLayer.end(); it++) {
        it->second->ReloadShaders();
    }
}

void Atmosphere::ForceSunAndMoon(double sunLat, double sunLon, double moonLat, double moonLon, double moonPhase)
{
    if (ephemeris) {
        ephemeris->ForceSunPosition(sunLon, sunLat);
        ephemeris->ForceMoonPosition(moonLon, moonLat);
        ephemeris->ForceMoonPhase(moonPhase);
    }
}

void Atmosphere::ForceSunAndMoon(double sunLat, double sunLon, double moonLat, double moonLon)
{
    if (ephemeris) {
        ephemeris->ForceSunPosition(sunLon, sunLat);
        ephemeris->ForceMoonPosition(moonLon, moonLat);
    }
}

void Atmosphere::ForceSunAndMoonHorizon(double sunAltitude, double sunAzimuth, double moonAltitude, double moonAzimuth, double moonPhase)
{
    if (ephemeris) {
        ephemeris->ForceSunPositionHoriz(sunAltitude, sunAzimuth);
        ephemeris->ForceMoonPositionHoriz(moonAltitude, moonAzimuth);
        ephemeris->ForceMoonPhase(moonPhase);
    }
}

void Atmosphere::ForceSunAndMoonHorizon(double sunAltitude, double sunAzimuth, double moonAltitude, double moonAzimuth)
{
    if (ephemeris) {
        ephemeris->ForceSunPositionHoriz(sunAltitude, sunAzimuth);
        ephemeris->ForceMoonPositionHoriz(moonAltitude, moonAzimuth);
    }
}

void Atmosphere::ForceMoonPhase(double moonPhase)
{
    if (ephemeris) {
        ephemeris->ForceMoonPhase(moonPhase);
    }
}

void Atmosphere::ClearForcedSunAndMoon()
{
    if (ephemeris) {
        ephemeris->ClearForcedMoon();
        ephemeris->ClearForcedSun();
    }
}

void Atmosphere::ClearForcedMoonPhase()
{
    if (ephemeris) {
        ephemeris->ClearForcedMoonPhase();
    }
}

void Atmosphere::SetUserDefinedVertString( const char *userString )
{
    if (userDefinedVertString) SL_FREE(userDefinedVertString);

    userDefinedVertString = (char *)SL_MALLOC(strlen(userString) + 1);

#ifdef _WIN32
#if _MSC_VER < 1400
    strncpy(userDefinedVertString, userString, strlen(userString) + 1);
#else
    strcpy_s(userDefinedVertString, strlen(userString) + 1, userString);
#endif
#else
    strncpy(userDefinedVertString, userString, strlen(userString) + 1);
#endif
}

void Atmosphere::SetUserDefinedFragString( const char *userString )
{
    if (userDefinedFragString) SL_FREE(userDefinedFragString);

    userDefinedFragString = (char *)SL_MALLOC(strlen(userString) + 1);

#ifdef _WIN32
#if _MSC_VER < 1400
    strncpy(userDefinedFragString, userString, strlen(userString) + 1);
#else
    strcpy_s(userDefinedFragString, strlen(userString) + 1, userString);
#endif
#else
    strncpy(userDefinedFragString, userString, strlen(userString) + 1);
#endif
}

const char * Atmosphere::GetUserDefinedVertString()
{
    return userDefinedVertString;
}

const char * Atmosphere::GetUserDefinedFragString()
{
    return userDefinedFragString;
}
