// Copyright (c) 2013-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/RotorWash.h>
#include <FeTriton/Ocean.h>
#include <FeTriton/WakeManager.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Decal.h>
#include <FeTriton/DecalManager.h>

using namespace Triton;

RotorWash::RotorWash(Ocean *pOcean, double pRotorDiameter, bool pSprayEffects, bool pUseDecals,double pFadeInTime)
    : ocean(pOcean), sprayEffects(pSprayEffects), lastWaveEmitTime(0), lastSprayEmitTime(0),
      firstEmit(true), wakeNumber(0), lastWakeNumber(-1), rotorDiameter(pRotorDiameter),
      useDecal(pUseDecals), phaseOffset(0), registered(false), lastTime(0),waveFadeInTime(pFadeInTime)
{
    waveGenerationPeriod = 2.0;
    Configuration::GetDoubleValue("wash-wave-generation-period", waveGenerationPeriod);

    sprayGenerationPeriod = 1.0;
    Configuration::GetDoubleValue("wash-spray-generation-period", sprayGenerationPeriod);

    double washSize = 3.0;
    Configuration::GetDoubleValue("wash-size", washSize);

    windScale = 2.0;
    Configuration::GetDoubleValue("wash-wind-scale", windScale);

    rotorDecalSpeedLimit = 10.0;
    Configuration::GetDoubleValue("wash-decal-speed-limit", rotorDecalSpeedLimit);
    if (pOcean) rotorDecalSpeedLimit /= pOcean->GetEnvironment()->GetWorldUnits();

    phaseRandomness = 0.5;
    Configuration::GetDoubleValue("wash-phase-randomness", phaseRandomness);

    washDiameter = pRotorDiameter * washSize;

    maxVelocity = 10.0;
    Configuration::GetDoubleValue("wash-max-velocity", maxVelocity);
    if (pOcean) maxVelocity /= pOcean->GetEnvironment()->GetWorldUnits();

    particleSize = 10.0;
    Configuration::GetDoubleValue("wash-spray-particle-size", particleSize);
    if (pOcean) particleSize /= pOcean->GetEnvironment()->GetWorldUnits();

    transparency = 0.2f;
    Configuration::GetFloatValue("wash-spray-transparency", transparency);

    decayRate = 1.0 / 0.9;
    Configuration::GetDoubleValue("wash-wave-decay", decayRate);

    decalPeriod = 3.0f;
    Configuration::GetFloatValue("wash-decal-period", decalPeriod);

    decalMinScale = 1.0f;
    Configuration::GetFloatValue("wash-decal-min-scale", decalMinScale);

    decalMaxScale = 5.0f;
    Configuration::GetFloatValue("wash-decal-max-scale", decalMaxScale);

    decalIntensity = 2.0;
    Configuration::GetDoubleValue("wash-decal-intensity", decalIntensity);

    for (int i = 0; i < NUM_DECALS; i++) {
        decals[i] = 0;
        decalStartTimes[i] = 0;
    }

    for (int j = 0; j < SMOOTH_BUFFER_SIZE; j++) {
        velocitySmoothBuffer[j] = 0;
    }
    bufferEntry = 0;

    currentDecal = 0;
}

RotorWash::~RotorWash()
{
    if (ocean && registered) {

        for (int i = 0; i < NUM_DECALS; i++) {
            if (decals[i]) {
                ocean->RemoveDecal((DecalHandle)decals[i]);
            }
        }

        ocean->UnregisterRotorWash(this);
    }
}

void RotorWash::UpdateDecals(double time, const Vector3& position, double dampening)
{
    if (!useDecal) return;

    double modelVelocity = 0, smoothedVelocity = 0;

    if (lastTime > 0 && time > lastTime) {
        modelVelocity = (position - lastPosition).Length() / (time - lastTime);
        velocitySmoothBuffer[bufferEntry] = modelVelocity;
        bufferEntry++;
        if (bufferEntry >= SMOOTH_BUFFER_SIZE) {
            bufferEntry = 0;
        }
    }

    double totalVel = 0;
    for (int i = 0; i < SMOOTH_BUFFER_SIZE; i++) {
        totalVel += velocitySmoothBuffer[i];
    }
    smoothedVelocity = totalVel / (double)SMOOTH_BUFFER_SIZE;

    lastPosition = position;
    lastTime = time;

    double velocityDampening = (rotorDecalSpeedLimit - smoothedVelocity) / rotorDecalSpeedLimit;
    if (velocityDampening > 1.0) velocityDampening = 1.0;
    if (velocityDampening < 0) velocityDampening = 0;

    dampening *= velocityDampening;
    dampening *= decalIntensity;

    if (dampening > 1.0) dampening = 1.0;
    if (dampening < 0) dampening = 0;

    double dt = time - decalStartTimes[currentDecal];
    if (dt > decalPeriod / (float)NUM_DECALS) {
        currentDecal++;
        if (currentDecal >= NUM_DECALS) currentDecal = 0;
        decalStartTimes[currentDecal] = time;

        if (!decals[currentDecal] && ocean) {
            decals[currentDecal] = (Decal *)(ocean->AddDecal(ocean->GetWakeManager()->GetRotorWashTexture(),
                                             (float)rotorDiameter, position));
            if (decals[currentDecal]) {
                decals[currentDecal]->SetAdditive(true);
            }
        } else {
            decals[currentDecal]->SetPosition(position);
        }
    }

    for (int i = 0; i < NUM_DECALS; i++) {
        Decal *decal = decals[i];
        if (decal) {
            float a = (float)(time - decalStartTimes[i]) / decalPeriod;
            if (a > 1.0f) a = 1.0f;
            float alpha = 1.7f * sinf(TRITON_PIF * a) * (1.0f - a);
            decal->SetAlpha(alpha * (float)(velocity / maxVelocity) * (float)dampening);
            float scale = decalMinScale + a * (decalMaxScale - decalMinScale);
            decal->SetScale(scale, scale);
        }
    }
}

void RotorWash::Update(const Vector3& pPosition, const Vector3& direction, double pVelocity, double pTime)
{
    if (!registered) {
        if (ocean) {
            ocean->RegisterRotorWash(this);
            registered = true;
        }
    }

    Vector3 dir = direction;
    dir.Normalize();

    if (ocean && ocean->GetIntersection(pPosition, dir, position)) {

        double distance = (pPosition - position).Length();
        double distanceDampening = exp(-distance / rotorDiameter) * windScale;
        velocity = pVelocity * distanceDampening;
        if (velocity > maxVelocity) {
            velocity = maxVelocity;
        }

        UpdateDecals(pTime, position, distanceDampening);

        WakeManager *wakeManager = ocean->GetWakeManager();
        if (wakeManager) {

            double period = waveGenerationPeriod + phaseOffset;
            if ((pTime - lastWaveEmitTime) > period) {
                lastWaveEmitTime = pTime;
                Vector3 delta;
                if (!firstEmit) {
                    delta = lastEmitPosition - position;
                } else {
                    firstEmit = false;
                }
                lastLastEmitPosition = lastEmitPosition;
                lastEmitSourcePosition = pPosition;
                lastEmitPosition = position;
                lastWakeNumber = wakeNumber;
                wakeManager->AddCircularWave(velocity, position, pTime,waveFadeInTime,Vector3(0, 0, 0), rotorDiameter, decayRate, NULL);

                if (ocean && ocean->GetEnvironment()) {
                    phaseOffset = ocean->GetEnvironment()->GetRandomNumberGenerator()->GetRandomDouble(0.0, waveGenerationPeriod * phaseRandomness);
                }
            }

            if (sprayEffects) {
                period = sprayGenerationPeriod;
                if ((pTime - lastSprayEmitTime) > period) {
                    lastSprayEmitTime = pTime;

                    const Environment *env = ocean->GetEnvironment();
                    Vector3 up = env->GetUpVector();
                    Vector3 xAxis = up.Cross(dir);
                    if (xAxis == Vector3(0, 0, 0)) {
                        Vector3 right = env->GetRightVector();
                        xAxis = right.Cross(dir);
                    }
                    xAxis.Normalize();
                    Vector3 yAxis = xAxis.Cross(dir);

                    double t = 2.0 * TRITON_PI * env->GetRandomNumberGenerator()->GetRandomDouble(0.0, 1.0);
                    double r = env->GetRandomNumberGenerator()->GetRandomDouble(0.5, 1.0);
                    r *= washDiameter;
                    double x = r * cos(t);
                    double y = r * sin(t);

                    Vector3 originPoint = pPosition + xAxis * x + yAxis * y;

                    Vector3 sprayIntersection;
                    if (ocean->GetIntersection(originPoint, dir, sprayIntersection)) {
                        // Reflect off the water surface to get the spray direction
                        Vector3 L = pPosition - sprayIntersection;
                        L.Normalize();
                        Vector3 R = (up * (2.0 * (up.Dot(L)))) - L;
                        wakeManager->AddDirectionalSpray(velocity, sprayIntersection, R, pTime, particleSize, transparency);
                    }
                }
            }
        }
    }
}