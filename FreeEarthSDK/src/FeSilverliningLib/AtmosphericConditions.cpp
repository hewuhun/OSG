// Copyright 2004-2009 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/AtmosphericConditions.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Vector3.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/PrecipitationManager.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/CloudLayerFactory.h>
#if defined(__INTEL_COMPILER)
#include <mathimf.h>
#else
#include <math.h>
#endif

using namespace SilverLining;
using namespace std;

AtmosphericConditions::AtmosphericConditions() : turbidity(2.0), visibility(30000), lightPollution(0),
    precipitationWindX(0.0), precipitationWindZ(0.0), timePassage(false),
    lastRelightPass(0), baseTimeMS(0), relightFrequency(-1), wantsLightingUpdate(true),  overrideFog(false),
    fogDensity(0.001), fogR(1), fogG(1), fogB(1)
{
    defaultTimer = SL_NEW MillisecondTimer();
    timer = defaultTimer;
}

AtmosphericConditions::~AtmosphericConditions()
{
    RemoveAllCloudLayers();

    if (defaultTimer) {
        SL_DELETE defaultTimer;
    }
}

void AtmosphericConditions::SetFog(double density, double r, double g, double b)
{
    if (density > 0 && r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0) {
        overrideFog = true;
        fogR = r;
        fogG = g;
        fogB = b;
        fogDensity = density;
    }
}

void AtmosphericConditions::ClearFog()
{
    overrideFog = false;
}

void AtmosphericConditions::GetFog(bool& fogIsSet, double& density, double& r, double& g, double& b)
{
    fogIsSet = overrideFog;
    density = fogDensity;
    r = fogR;
    g = fogG;
    b = fogB;
}

void AtmosphericConditions::SetLocation(const Location& pLocation)
{
    location = pLocation;
}

const Location& AtmosphericConditions::GetLocation() const
{
    return location;
}

void AtmosphericConditions::SetTime(const LocalTime& pTime)
{
    localTime = pTime;
    wantsLightingUpdate = true;
    baseTimeMS = timer->GetMilliseconds();
}

const LocalTime& AtmosphericConditions::GetTime()
{
    if (timePassage) {
        long dt = timer->GetMilliseconds() - baseTimeMS;
        long seconds = (long)((float(dt) * 0.001f));
        adjustedTime = localTime;
        adjustedTime.AddSeconds(seconds);
        return adjustedTime;
    }

    return localTime;
}

void AtmosphericConditions::ClearWindVolumes()
{
    windVolumes.clear();
}

int AtmosphericConditions::SetWind(const WindVolume& windVolume)
{
    static int newWindVolumeHandle = 0;

    newWindVolumeHandle++;

    windVolumes[newWindVolumeHandle] = windVolume;

    return newWindVolumeHandle;
}

bool AtmosphericConditions::RemoveWindVolume(int windVolumeHandle)
{
    SL_MAP(int, WindVolume) ::iterator it;
    it = windVolumes.find(windVolumeHandle);
    if (it != windVolumes.end()) {
        return (windVolumes.erase(windVolumeHandle) > 0);
    }

    return false;
}

void AtmosphericConditions::GetWind(double& velocity, double& heading, double altitude) const
{
    SL_MAP(int, WindVolume) ::const_iterator it;
    for (it = windVolumes.begin(); it != windVolumes.end(); it++) {
        if (it->second.Inside(altitude)) {
            velocity = it->second.GetWindSpeed();
            heading = it->second.GetDirection();
            return;
        }
    }

    velocity = 0;
    heading = 0;
}

void AtmosphericConditions::SetPresetConditions(ConditionPresets preset, Atmosphere& atm)
{
    RemoveAllCloudLayers();

    // All conditions have cirrus
    CloudLayer *cirrusCloudLayer;

    cirrusCloudLayer = CloudLayerFactory::Create(CIRRUS_FIBRATUS);
    cirrusCloudLayer->SetBaseAltitude(8000);
    cirrusCloudLayer->SetThickness(0);
    cirrusCloudLayer->SetBaseLength(100000);
    cirrusCloudLayer->SetBaseWidth(100000);
    cirrusCloudLayer->SetLayerPosition(0, 0);
    cirrusCloudLayer->SeedClouds(atm);
    cirrusCloudLayer->SetIsInfinite(true);
    AddCloudLayer(cirrusCloudLayer);

    // If overcast, add stratus
    if (preset == OVERCAST) {
        CloudLayer *stratusLayer;

        stratusLayer = CloudLayerFactory::Create(STRATUS);
        stratusLayer->SetIsInfinite(true);
        stratusLayer->SetBaseAltitude(1500);
        stratusLayer->SetThickness(600);
        stratusLayer->SetDensity(1.0);
        stratusLayer->SetLayerPosition(0, 0);
        stratusLayer->SeedClouds(atm);
        AddCloudLayer(stratusLayer);
    } else if (preset == PARTLY_CLOUDY || preset == MOSTLY_CLOUDY) {
        // Otherwise, add in some cumulus congestus
        CloudLayer *cumulusCongestusLayer;

        cumulusCongestusLayer = CloudLayerFactory::Create(CUMULUS_CONGESTUS);
        cumulusCongestusLayer->SetIsInfinite(true);
        cumulusCongestusLayer->SetBaseAltitude(3000);
        cumulusCongestusLayer->SetThickness(100);
        cumulusCongestusLayer->SetBaseLength(60000);
        cumulusCongestusLayer->SetBaseWidth(60000);
        cumulusCongestusLayer->SetDensity(preset == PARTLY_CLOUDY ? 0.2 : 0.8);
        cumulusCongestusLayer->SetLayerPosition(0, 0);
        cumulusCongestusLayer->SetCloudAnimationEffects(0.1, false);
        cumulusCongestusLayer->SeedClouds(atm);
        cumulusCongestusLayer->SetAlpha(0.8);
        cumulusCongestusLayer->SetFadeTowardEdges(true);

        AddCloudLayer(cumulusCongestusLayer);
    }
}

int AtmosphericConditions::AddCloudLayer(CloudLayer *layer)
{
    if (!layer) return 0;

    static int newCloudHandle = 0;

    newCloudHandle++;

    cloudLayers[newCloudHandle] = layer;

    return newCloudHandle;
}

bool AtmosphericConditions::RemoveCloudLayer(int layerHandle)
{
    SL_MAP(int, CloudLayer *) ::iterator it;
    it = cloudLayers.find(layerHandle);
    if (it != cloudLayers.end()) {
        SL_DELETE (*it).second;

        return (cloudLayers.erase(layerHandle) > 0);
    }

    return false;
}

void AtmosphericConditions::RemoveAllCloudLayers()
{
    SL_MAP(int, CloudLayer *) ::iterator it;
    for (it = cloudLayers.begin(); it != cloudLayers.end(); it++) {
        SL_DELETE (*it).second;
    }
    cloudLayers.clear();
}

bool AtmosphericConditions::GetCloudLayer(int handle, CloudLayer **layer)
{
    if (layer) {
        SL_MAP(int, CloudLayer*) ::iterator it;
        it = cloudLayers.find(handle);
        if (it != cloudLayers.end()) {
            *layer = (*it).second;
            return true;
        }
    }

    return false;
}

static inline double toRadians(double x)
{
    return x * (3.14159265 / 180.0);
}

void AtmosphericConditions::ApplyWind(double dt)
{
    SL_MAP(int, CloudLayer *) ::iterator it;
    for (it = cloudLayers.begin(); it != cloudLayers.end(); it++) {
        CloudLayer *cl = (*it).second;
        if (cl) {
            double alt = cl->GetBaseAltitude();
            double velocity, heading;
            GetWind(velocity, heading, alt);

            double windX;
            double windZ;
            cl->GetWind(windX, windZ);

            double dx = (windX + sin(toRadians(heading)) * velocity) * dt;
            double dz = (windZ + cos(toRadians(heading)) * velocity) * dt;

            Vector3 vWind(-dx, 0, dz);
            vWind = vWind * Renderer::GetInstance()->GetBasis3x3();
            cl->MoveClouds(vWind.x, vWind.y, vWind.z);
        }
    }
}

void AtmosphericConditions::SetPrecipitation(int type, double intensity, double fNear, double fFar, bool bUseDepthBuffer)
{
    if (intensity < 0) intensity = 0;

    if (type < CloudLayer::NONE || type >= CloudLayer::NUM_PRECIP_TYPES) type = CloudLayer::NONE;

    if (type == CloudLayer::NONE) {
        precipitationEffects.clear();
        precipitationNearClips.clear();
        precipitationFarClips.clear();
        precipitationUseDepthBuffer.clear();
    } else {
        precipitationEffects[type] = intensity;
        precipitationNearClips[type] = fNear;
        precipitationFarClips[type] = fFar;
        precipitationUseDepthBuffer[type] = bUseDepthBuffer;
    }
}

void AtmosphericConditions::ApplyPrecipitation()
{
    PrecipitationManager *pm = PrecipitationManager::GetInstance(Atmosphere::GetCurrentAtmosphere());
    SL_MAP(int, double) ::iterator it;
    for (it = precipitationEffects.begin(); it != precipitationEffects.end(); it++) {
        int type = (*it).first;
        double intensity = (*it).second;
        pm->SetIntensity(type, intensity);

        double nearClip = precipitationNearClips[type];
        double farClip = precipitationFarClips[type];
        bool   useDepthBuffer = precipitationUseDepthBuffer[type];
        pm->SetClipPlanes(type, nearClip, farClip, useDepthBuffer);
    }
}

void AtmosphericConditions::EnableTimePassage(bool enabled, long relightFrequencyMS)
{
    timePassage = enabled;
    relightFrequency = relightFrequencyMS;
    lastRelightPass = baseTimeMS = timer->GetMilliseconds();
}

bool AtmosphericConditions::WantsLightingUpdate()
{
    if (wantsLightingUpdate) {
        wantsLightingUpdate = false;
        return true;
    }

    if (!timePassage) {
        return true;
    }

    if (relightFrequency == -1) {
        return false;
    }

    long dt = timer->GetMilliseconds() - lastRelightPass;
    if (dt > relightFrequency) {
        lastRelightPass = timer->GetMilliseconds();
        return true;
    }

    return false;
}

void AtmosphericConditions::SetMillisecondTimer(const MillisecondTimer *pTimer)
{
    if (pTimer) {
        baseTimeMS = pTimer->GetMilliseconds();
        timer = pTimer;
    } else {
        timer = defaultTimer;
    }
}

bool AtmosphericConditions::Serialize(std::ostream& s)
{
    location.Serialize(s);
    localTime.Serialize(s);
    adjustedTime.Serialize(s);

    s.write((char *)&turbidity, sizeof(double));
    s.write((char *)&visibility, sizeof(double));
    s.write((char *)&lightPollution, sizeof(double));

    {
        int nWindVolumes = (int)windVolumes.size();
        s.write((char *)&nWindVolumes, sizeof(int));
        SL_MAP(int, WindVolume) ::iterator it;
        for (it = windVolumes.begin(); it != windVolumes.end(); it++) {
            int handle = it->first;
            s.write((char *)&handle, sizeof(int));
            it->second.Serialize(s);
        }
    }

    {
        int nCloudLayers = (int)cloudLayers.size();
        s.write((char *)&nCloudLayers, sizeof(int));
        SL_MAP(int, CloudLayer *) ::iterator it;
        for (it = cloudLayers.begin(); it != cloudLayers.end(); it++) {
            int handle = it->first;
            s.write((char *)&handle, sizeof(int));
            CloudLayer *layer = it->second;
            CloudLayerFactory::Serialize(layer, s);
        }
    }

    int nPrecipEffects = (int)precipitationEffects.size();
    s.write((char *)&nPrecipEffects, sizeof(int));
    SL_MAP(int, double) ::iterator it2;
    for (it2 = precipitationEffects.begin(); it2 != precipitationEffects.end(); it2++) {
        int type = it2->first;
        double intensity = it2->second;
        s.write((char *)&type, sizeof(int));
        s.write((char *)&intensity, sizeof(double));
    }

    s.write((char *)&precipitationWindX, sizeof(double));
    s.write((char *)&precipitationWindZ, sizeof(double));

    s.write((char *)&timePassage, sizeof(bool));
    s.write((char *)&fogDensity, sizeof(double));
    s.write((char *)&fogR, sizeof(double));
    s.write((char *)&fogG, sizeof(double));
    s.write((char *)&fogB, sizeof(double));

    return true;
}

bool AtmosphericConditions::Unserialize(const Atmosphere *atm, std::istream& s)
{
    int i;

    location.Unserialize(s);
    localTime.Unserialize(s);
    adjustedTime.Unserialize(s);

    s.read((char *)&turbidity, sizeof(double));
    s.read((char *)&visibility, sizeof(double));
    s.read((char *)&lightPollution, sizeof(double));

    windVolumes.clear();
    int nWindVolumes;
    s.read((char *)&nWindVolumes, sizeof(int));
    for (i = 0; i < nWindVolumes; i++) {
        int handle;
        s.read((char *)&handle, sizeof(int));
        WindVolume wv;
        wv.Unserialize(s);
        windVolumes[handle] = wv;
    }

    cloudLayers.clear();
    int nCloudLayers;
    s.read((char *)&nCloudLayers, sizeof(int));
    for (i = 0; i < nCloudLayers; i++) {
        int handle;
        s.read((char *)&handle, sizeof(int));
        CloudLayer *layer = CloudLayerFactory::Unserialize(*atm, s);
        cloudLayers[handle] = layer;
    }

    precipitationEffects.clear();
    int nPrecipEffects;
    s.read((char *)&nPrecipEffects, sizeof(int));
    for (i = 0; i < nPrecipEffects; i++) {
        int type;
        double intensity;
        s.read((char *)&type, sizeof(int));
        s.read((char *)&intensity, sizeof(double));
        precipitationEffects[type] = intensity;
    }

    s.read((char *)&precipitationWindX, sizeof(double));
    s.read((char *)&precipitationWindZ, sizeof(double));

    s.read((char *)&timePassage, sizeof(bool));
    s.read((char *)&fogDensity, sizeof(double));
    s.read((char *)&fogR, sizeof(double));
    s.read((char *)&fogG, sizeof(double));
    s.read((char *)&fogB, sizeof(double));

    return true;
}

