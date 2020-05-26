// Copyright (c) 2009-2014 Sundog Software LLC, All rights reserved worldwide.

#include <FeSilverliningLib/PrecipitationManager.h>
#include <FeSilverliningLib/Rain.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Snow.h>
#include <FeSilverliningLib/Sleet.h>
#include <map>

using namespace SilverLining;
using namespace std;

SL_MAP(Atmosphere *, PrecipitationManager *) PrecipitationManager::instances;
ShaderHandle Precipitation::precipShader = 0;

PrecipitationManager *PrecipitationManager::GetInstance(Atmosphere *atm)
{
    SL_MAP(Atmosphere *, PrecipitationManager *) ::iterator it;
    it = instances.find(atm);
    if (it == instances.end()) {
        instances[atm] = SL_NEW PrecipitationManager(atm);
    }

    return instances[atm];
}

void PrecipitationManager::Release(Atmosphere *atm)
{
    SL_MAP(Atmosphere *, PrecipitationManager *) ::iterator it;
    it = instances.find(atm);

    if (it != instances.end()) {
        SL_DELETE instances[atm];
        instances.erase(it);
    }
}

void PrecipitationManager::Initialize(Atmosphere *atm)
{
    rainSystems.clear();
    snowSystems.clear();
    sleetSystems.clear();

    Precipitation::precipShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/Particle.cg", VERTEX_PROGRAM);
}

void PrecipitationManager::ReloadShader()
{
    if (Precipitation::precipShader) {
        Renderer::GetInstance()->DeleteShader(Precipitation::precipShader);
    }
    Precipitation::precipShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/Particle.cg", VERTEX_PROGRAM);
}

ShaderHandle PrecipitationManager::GetShaderProgram()
{
    return Precipitation::precipShader;
}

PrecipitationManager::~PrecipitationManager()
{
    SL_MAP(CameraHandle, Rain *)::iterator rainIt;
    for (rainIt = rainSystems.begin(); rainIt != rainSystems.end(); rainIt++) {
        SL_DELETE rainIt->second;
    }
    rainSystems.clear();

    SL_MAP(CameraHandle, Snow *)::iterator snowIt;
    for (snowIt = snowSystems.begin(); snowIt != snowSystems.end(); snowIt++) {
        SL_DELETE snowIt->second;
    }
    snowSystems.clear();

    SL_MAP(CameraHandle, Sleet *)::iterator sleetIt;
    for (sleetIt = sleetSystems.begin(); sleetIt != sleetSystems.end(); sleetIt++) {
        SL_DELETE sleetIt->second;
    }
    sleetSystems.clear();
}

Rain *PrecipitationManager::GetRain(CameraHandle cam)
{
    SL_MAP(CameraHandle, Rain *) ::iterator it;
    it = rainSystems.find(cam);
    if (it == rainSystems.end()) {
        rainSystems[cam] = SL_NEW Rain(atmosphere);
    }

    return rainSystems[cam];
}

Snow *PrecipitationManager::GetSnow(CameraHandle cam)
{
    SL_MAP(CameraHandle, Snow *) ::iterator it;
    it = snowSystems.find(cam);
    if (it == snowSystems.end()) {
        snowSystems[cam] = SL_NEW Snow(atmosphere);
    }

    return snowSystems[cam];
}

Sleet *PrecipitationManager::GetSleet(CameraHandle cam)
{
    SL_MAP(CameraHandle, Sleet *) ::iterator it;
    it = sleetSystems.find(cam);
    if (it == sleetSystems.end()) {
        sleetSystems[cam] = SL_NEW Sleet(atmosphere);
    }

    return sleetSystems[cam];
}

void PrecipitationManager::SetCurrentCamera(CameraHandle cam)
{
    currentCamera = cam;
}

double PrecipitationManager::GetFogDensity()
{
    double density = 0;

    bool doFog = false;
    Configuration::GetBoolValue("enable-precipitation-visibility-effects", doFog);
    if (doFog) {
        density += GetRain(currentCamera)->GetFogDensity();
        density += GetSnow(currentCamera)->GetFogDensity();
        density += GetSleet(currentCamera)->GetFogDensity();
    }

    return density;
}

void PrecipitationManager::SetClipPlanes(int precipType, double nearClip, double farClip, bool useDepthBuffer)
{
    if (nearClip > 0 && farClip > 0) {
        switch (precipType) {
        case CloudLayer::RAIN:

            GetRain(currentCamera)->SetEffectRange(nearClip, farClip, useDepthBuffer);

            break;

        case CloudLayer::WET_SNOW:
        case CloudLayer::DRY_SNOW:

            GetSnow(currentCamera)->SetEffectRange(nearClip, farClip, useDepthBuffer);

            break;

        case CloudLayer::SLEET:

            GetSleet(currentCamera)->SetEffectRange(nearClip, farClip, useDepthBuffer);

            break;
        }
    }
}

void PrecipitationManager::SetIntensity(int precipType, double mmPerHour)
{
    switch (precipType) {
    case CloudLayer::RAIN:

        GetRain(currentCamera)->SetIntensity(mmPerHour);

        break;

    case CloudLayer::WET_SNOW:

        GetSnow(currentCamera)->SetIsWet(true);
        GetSnow(currentCamera)->SetIntensity(mmPerHour);

        break;

    case CloudLayer::DRY_SNOW:

        GetSnow(currentCamera)->SetIsWet(false);
        GetSnow(currentCamera)->SetIntensity(mmPerHour);

        break;

    case CloudLayer::SLEET:

        GetSleet(currentCamera)->SetIntensity(mmPerHour);

        break;
    }
}

void PrecipitationManager::Render(double dt, const Frustum& f, const Color& lightColor)
{
    if (dt == 0) dt = 1.0;

    Renderer *ren = Renderer::GetInstance();
    ren->PushAllState();
    ren->SetDefaultState();

    Rain *rain = GetRain(currentCamera);
    Snow *snow = GetSnow(currentCamera);
    Sleet *sleet = GetSleet(currentCamera);

    if (rain && rain->GetIntensity() > 0) {
        rain->Render(dt, f, lightColor);
    }

    if (snow && snow->GetIntensity() > 0) {
        snow->Render(dt, f, lightColor);
    }

    if (sleet && sleet->GetIntensity() > 0) {
        sleet->Render(dt, f, lightColor);
    }

    ren->PopAllState();
}

void PrecipitationManager::ClearPrecipitation()
{
    Rain *rain = GetRain(currentCamera);
    Snow *snow = GetSnow(currentCamera);
    Sleet *sleet = GetSleet(currentCamera);

    if (rain) rain->SetIntensity(0);
    if (snow) snow->SetIntensity(0);
    if (sleet) sleet->SetIntensity(0);
}
