// Copyright (c) 2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/TidalStreamWake.h>
#include <FeTriton/Ocean.h>
#include <FeTriton/WakeManager.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Decal.h>
#include <FeTriton/DecalManager.h>

using namespace Triton;

TidalStreamWake::TidalStreamWake(Ocean *pOcean, double pSize, double pDraft, double pWaveMax, double pOffset, bool pUseDecals, bool pUseDisplacement) :
    ocean(pOcean), size(pSize), draft(pDraft), waveMax(pWaveMax), registered(false), wakeManagerRegistered(false),
    offset(pOffset), lastTime(0), useDecal(pUseDecals), useDisplacement(pUseDisplacement)
{
    amplitudeVariance = directionVariance = 0;
    amplitudeSpeed = 5.0;
    directionSpeed = 0.5;
    animationScale = 0.5f;

    Configuration::GetDoubleValue("tidal-stream-amplitude-variance", amplitudeVariance);
    Configuration::GetDoubleValue("tidal-stream-direction-variance", directionVariance);
    Configuration::GetDoubleValue("tidal-stream-amplitude-speed", amplitudeSpeed);
    Configuration::GetDoubleValue("tidal-stream-direction-speed", directionSpeed);
    Configuration::GetFloatValue("tidal-stream-animation-scale", animationScale);

    if (pOcean) {
        amplitudeVariance /= pOcean->GetEnvironment()->GetWorldUnits();
        for (int i = 0; i < 256; i++) {
            noiseVerts[i] = (float)(pOcean->GetEnvironment()->GetRandomNumberGenerator()->GetRandomDouble(0.0, 1.0));
        }
    }

    decalPeriod = 3.0f;
    Configuration::GetFloatValue("tidal-stream-decal-period", decalPeriod);

    decalMinScale = 1.0f;
    Configuration::GetFloatValue("tidal-stream-decal-min-scale", decalMinScale);

    decalMaxScale = 1.0f;
    Configuration::GetFloatValue("tidal-stream-decal-max-scale", decalMaxScale);

    decalIntensity = 2.0;
    Configuration::GetDoubleValue("tidal-stream-decal-intensity", decalIntensity);

    for (int i = 0; i < NUM_TS_DECALS; i++) {
        decals[i] = 0;
        decalStartTimes[i] = 0;
    }

    currentDecal = 0;
}

TidalStreamWake::~TidalStreamWake()
{
    if (ocean && registered) {

        for (int i = 0; i < NUM_TS_DECALS; i++) {
            if (decals[i]) {
                ocean->RemoveDecal((DecalHandle)decals[i]);
            }
        }

        ocean->UnregisterTidalStreamWake(this);
        registered = false;
    }

    if (ocean && wakeManagerRegistered) {
        WakeManager *wakeManager = ocean->GetWakeManager();
        if (wakeManager) {
            wakeManager->RemoveWakeGenerator(this);
            wakeManagerRegistered = false;
        }
    }
}

void TidalStreamWake::UpdateDecals(double time, const Vector3& position, double dampening, const Vector3& direction, double velocity)
{
    if (!useDecal || !ocean) return;

    Vector3 in, up, right;

    switch (ocean->GetEnvironment()->GetCoordinateSystem()) {
    case FLAT_ZUP: {
        up = Vector3(0, 0, 1);
        in = Vector3(0, 1, 0);
        break;
    }

    case FLAT_YUP: {
        up = Vector3(0, 1, 0);
        in = Vector3(0, 0, -1);
        break;
    }

    case WGS84_ZUP:
    case SPHERICAL_ZUP: {
        up = position;
        up.Normalize();
        Vector3 north(0, 0, 1);
        right = north.Cross(up);
        right.Normalize();
        in = up.Cross(right);
        in.Normalize();
        break;
    }

    case WGS84_YUP:
    case SPHERICAL_YUP: {
        up = position;
        up.Normalize();
        Vector3 north(0, 1, 0);
        right = north.Cross(up);
        right.Normalize();
        in = up.Cross(right);
        in.Normalize();
        break;
    }
    }

    Vector3 out = in * -1.0;

    double rotation = acos(direction.Dot(out));
    // We have the angle between the vectors, but don't know which
    // direction to go.

    Vector3 cross = direction.Cross(out);
    cross.Normalize();
    double dotUp = cross.Dot(up);
    if (acos(dotUp) < TRITON_PI * 0.5) {
        rotation = -rotation;
    }

    Vector3 adjustedPosition = position + direction * size * 0.5;

    float animation = (float)((velocity * time) / size);
    animation *= animationScale;
    animation = fmod(animation, 2048.0f);

#if (NUM_TS_DECALS == 1)
    if (!decals[0] && ocean) {
        decals[0] = (Decal *)(ocean->AddDecal(ocean->GetWakeManager()->GetTidalStreamTexture(),
                                              (float)size, adjustedPosition));
        decals[0]->SetAdditive(true);
    } else {
        decals[0]->SetPosition(adjustedPosition);
    }

    Decal *decal = decals[0];
    if (decal) {
        decal->SetAlpha((float)(dampening));
        decal->SetRotation(rotation);
        decal->SetUVOffset(0, -animation);
    }

#else
    double dt = time - decalStartTimes[currentDecal];
    if (dt > decalPeriod / (float)NUM_TS_DECALS) {
        currentDecal++;
        if (currentDecal >= NUM_TS_DECALS) currentDecal = 0;
        decalStartTimes[currentDecal] = time;

        if (!decals[currentDecal] && ocean) {
            decals[currentDecal] = (Decal *)(ocean->AddDecal(ocean->GetWakeManager()->GetTidalStreamTexture(),
                                             (float)size, adjustedPosition));
            decals[currentDecal]->SetAdditive(true);
        } else {
            decals[currentDecal]->SetPosition(adjustedPosition);
        }
    }

    for (int i = 0; i < NUM_TS_DECALS; i++) {
        Decal *decal = decals[i];
        if (decal) {
            float a = (float)(time - decalStartTimes[i]) / decalPeriod;
            if (a > 1.0f) a = 1.0f;
            float alpha = 1.7f * sinf(TRITON_PIF * a) * (1.0f - a);
            decal->SetAlpha(alpha * (float)(dampening));
            float scale = decalMinScale + a * (decalMaxScale - decalMinScale);
            decal->SetScale(scale, scale);
            decal->SetRotation(rotation);
            decal->SetUVOffset(0, -animation);
        }
    }
#endif
}

static float Mix( const float &a, const float &b, const float &t )
{
    return a * ( 1 - t ) + b * t;
}

static float Smoothstep( const float &a, const float &b, const float &t )
{
    float tRemapSmoothstep = t * t * ( 3 - 2 * t );
    return Mix( a, b, tRemapSmoothstep );
}

float TidalStreamWake::CoherentNoise(float x)
{
    int xi = (int)x - ( x < 0 && x != (int)x );
    float t = x - xi;
    int xMin = xi & 255;
    int xMax = ( xMin + 1 ) & 255;

    return Smoothstep(noiseVerts[xMin], noiseVerts[xMax], t) * 2.0f - 1.0f;
}

void TidalStreamWake::Update(const Vector3& pPosition, const Vector3& pDirection, double pVelocity, double pTime)
{
    if (!ocean) return;
    WakeManager *wakeManager = ocean->GetWakeManager();

    if (!registered) {
        ocean->RegisterTidalStreamWake(this);
        registered = true;
    }

    if (!wakeManagerRegistered) {
        if (wakeManager) {
            if (useDisplacement) {
                wakeManager->AddExplicitWave(this, size);
            }
            wakeManagerRegistered = true;
        }
    }

    if (wakeManager) {
        Vector3 dir = pDirection;
        dir.Normalize();

        Vector3 wavePos = pPosition - dir * offset;

        double U2 = pVelocity * pVelocity;
        double d12 = draft > 0 ? sqrt(draft) : 0;

        double amplitude = U2 * d12 * 0.01;

        amplitude *= 1.0 + (double)CoherentNoise((float)(pTime * amplitudeSpeed)) * amplitudeVariance;

        dir.x += (double)(CoherentNoise((float)(pTime * directionSpeed)) * directionVariance);
        dir.y += (double)(CoherentNoise((float)(pTime * directionSpeed + 64.0)) * directionVariance);
        dir.z += (double)(CoherentNoise((float)(pTime * directionSpeed + 128.0)) * directionVariance);

        dir.Normalize();

        if (waveMax > 0 && amplitude > waveMax) {
            amplitude = waveMax;
        }

        UpdateDecals(pTime, wavePos, waveMax > 0 ? amplitude / waveMax : 1.0, dir, pVelocity);

        if (useDisplacement) {
            wakeManager->UpdateExplicitWave(this, wavePos, dir * -1.0, amplitude, size);
        }
    }
}

