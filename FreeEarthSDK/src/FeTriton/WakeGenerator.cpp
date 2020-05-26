// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/WakeGenerator.h>
#include <FeTriton/Ocean.h>
#include <FeTriton/WakeManager.h>
#include <FeTriton/Configuration.h>

using namespace Triton;

WakeGeneratorParameters::WakeGeneratorParameters()
{
    sprayEffects = false;
    bowSprayOffset = 50.0;
    sprayVelocityScale = 1.0;
    spraySizeScale = 1.0;

    bowWave = true;
    bowWaveOffset = 50.0;
    bowWaveScale = 1.0;
    bowWaveMax = -1.0;
    bowSize = 0;
    sternWaveOffset = -50.0;
    sternWaves = true;

    length = 100.0;
    beamWidth = 20.0;
    draft = 5.0;

    propWash = true;
    propWashOffset = 0.0;
    propWashWidthMultiplier = 1.5;
    Configuration::GetDoubleValue("prop-wash-default-width-multiplier", propWashWidthMultiplier);
    propWashFadeTime = 1000.0;
    Configuration::GetDoubleValue("prop-wash-default-fade-time", propWashFadeTime);

    bowWakeMultiplier = 1.0;
    Configuration::GetDoubleValue("bow-wake-size-multiplier", bowWakeMultiplier);
    sternWakeMultiplier = 1.0;
    Configuration::GetDoubleValue("stern-wake-size-multiplier", sternWakeMultiplier);

    numHullSprays = 0;
    hullSprayStartOffset = 40.0;
    hullSprayEndOffset = -50.0;
    hullSprayScale = 0.5;
    hullSpraySizeScale = 1.0;
    hullSprayVerticalOffset = 0.0;
}

static int numWakes = 0;

WakeGenerator::WakeGenerator(Ocean *pOcean, const WakeGeneratorParameters& pParams)
    : lastSprayEmitTime(0), firstEmit(true), wakeNumber(0), lastWakeNumber(-1), registered(false),
      lodDistance(0), distanceTravelled(0), lastPosition(0,0,0), registeredWakeManager(false)
{
    if (!pOcean) {
        Utils::DebugMsg("Null Ocean object passed into WakeGenerator!");
        ocean = 0;
        return;
    }

    ocean = pOcean;
    params = pParams;

    hullSpeed = 5.0 * sqrt(params.length * ocean->GetEnvironment()->GetWorldUnits()); // km/hr
    hullSpeed *= (1000.0 / 3600.0); // m/s
    hullSpeed /= ocean->GetEnvironment()->GetWorldUnits(); // back to native units

    minPropSegmentLength = 10.0;
    Configuration::GetDoubleValue("min-prop-wash-segment-length", minPropSegmentLength);
    minPropSegmentLength /= ocean->GetEnvironment()->GetWorldUnits();

    decayRate = 1.0 / 3.0;
    Configuration::GetDoubleValue("wake-wave-decay", decayRate);

    sprayPositionVariation = 2.0;
    Configuration::GetDoubleValue("wake-spray-position-variation", sprayPositionVariation);
    sprayPositionVariation /= ocean->GetEnvironment()->GetWorldUnits();

    curveGenerationFactor = 2.0;
    Configuration::GetDoubleValue("wake-wave-curve-generation-factor", curveGenerationFactor);

    sprayCullDist = 1000.0;
    Configuration::GetDoubleValue("wake-spray-cull-distance", sprayCullDist);
    sprayCullDist /= ocean->GetEnvironment()->GetWorldUnits();

    gravity = 9.81;
    Configuration::GetDoubleValue("wave-gravity-force", gravity);
    gravity /= ocean->GetEnvironment()->GetWorldUnits();

    double maxDistance = 0;
    Configuration::GetDoubleValue("max-wake-generator-distance", maxDistance);
    maxDistance /= ocean->GetEnvironment()->GetWorldUnits();
    maxDistanceSquared = maxDistance * maxDistance;

    maxWakes = 0;
    Configuration::GetIntValue("wake-generator-distance-threshold", maxWakes);
}

WakeGenerator::~WakeGenerator()
{
    if (ocean && registered) {
        ocean->UnregisterWakeGenerator(this);
    }
    ClearWakes();

    numWakes--;
}

const WakeGeneratorParameters& WakeGenerator::GetParameters() const
{
    return params;
}

void WakeGenerator::SetParameters(const WakeGeneratorParameters& p)
{
    params = p;
}

void WakeGenerator::ClearWakes()
{
    if (ocean && registeredWakeManager) {
        WakeManager *wakeManager = ocean->GetWakeManager();
        if (wakeManager) {
            wakeManager->RemoveWakeGenerator(this);
            wakeManager->RemoveLeewardDampener(this);
            registeredWakeManager = false;
        }
    }
}

void WakeGenerator::Update(const Vector3& pPosition, const Vector3& direction, double pVelocity, double pTime)
{

    if (maxDistanceSquared > 0 && numWakes > maxWakes && ocean) {
        double distSq = (pPosition - ocean->GetEnvironment()->GetCameraPosition()).SquaredLength();
        if (distSq > maxDistanceSquared) return;
    }

    if (!registered) {
        if (ocean) {
            ocean->RegisterWakeGenerator(this);
            registered = true;
            numWakes++;
        }
    }

    if (!registeredWakeManager) {
        if (ocean) {
            WakeManager *wakeManager = ocean->GetWakeManager();
            if (wakeManager) {
                if (params.bowWave) wakeManager->AddExplicitWave(this, params.length * 0.5);
                wakeManager->AddLeewardDampener(this);
                registeredWakeManager = true;
            }
        }
    }

    if (lastPosition.SquaredLength() > 0) {
        distanceTravelled += (pPosition - lastPosition).Length();
    }
    lastPosition = pPosition;

    Vector3 dir = direction;
    dir.Normalize();

    position = pPosition;
    washPosition = position + dir * (params.propWashOffset);
    sternPosition = position + dir * params.sternWaveOffset;
    Vector3 bowPosition = position + dir * params.bowWaveOffset;

    WakeManager *wakeManager = 0;

    if (ocean) wakeManager = ocean->GetWakeManager();

    // Update leeward dampener
    if (wakeManager) {
        Vector3 bow = position + dir * params.bowSprayOffset;
        if ((bow - washPosition).SquaredLength() > (bow - sternPosition).SquaredLength()) {
            wakeManager->UpdateLeewardDampener(this, bow, washPosition, pVelocity);
        } else {
            wakeManager->UpdateLeewardDampener(this, bow, sternPosition, pVelocity);
        }
    }

    velocity = pVelocity;

    if (ocean && velocity > 0) {

        if (wakeManager) {

            if (params.bowWave && ocean->GetEnvironment()) {
                double bowWakeWavelength = (velocity / hullSpeed) * params.length * params.bowWakeMultiplier;
                if (bowWakeWavelength < params.bowSize) bowWakeWavelength = params.bowSize;

                // Bow wake size reference: Bow Wave Dynamics - T. A. Waniewski, C. E. Brennen, and F. Raichlen
                // Journal of Ship Research, Vol. 46, No. 1, March 2002, pp. 1?5

                double mVelocity = velocity * ocean->GetEnvironment()->GetWorldUnits();
                double mDraft = params.draft * ocean->GetEnvironment()->GetWorldUnits();
                double U2 = mVelocity * mVelocity;
                double d12 = mDraft > 0 ? sqrt(mDraft) : 0;

                double amplitude = U2 * d12 * 0.01;
                amplitude *= params.bowWaveScale;

                amplitude /= ocean->GetEnvironment()->GetWorldUnits();

                if (params.bowWaveMax > 0 && amplitude > params.bowWaveMax) {
                    amplitude = params.bowWaveMax;
                }

                wakeManager->UpdateExplicitWave(this, bowPosition, dir, amplitude, bowWakeWavelength);
            }

            if (lastWakeNumber >= 0 && params.propWash) {
                Vector3 washDir = dir;
                washDir.Normalize();
                Vector3 updatedPosition = lastEmitSourcePosition + washDir * params.propWashOffset;
                Vector3 updatedDelta = lastEmitPosition - washPosition;
                wakeManager->UpdatePropWash(lastWakeNumber, washPosition, updatedDelta, pTime, this);
            }

            double distanceSinceLastWash = (lastEmitPosition - washPosition).Length();
            double distanceSinceLastWake = (lastWakePosition - position).Length();

            double lengthThreshold = ((TRITON_TWOPI * velocity * velocity) / gravity) * params.sternWakeMultiplier;

            if ((distanceSinceLastWake > lengthThreshold) && params.sternWaves) {
                lastWakePosition = position;
                wakeManager->AddKelvinWake(velocity, sternPosition, pTime, decayRate, this);
            }

            double propWashLengthThreshold = params.length * wakeManager->GetWaveGenerationDistance();

            if (!firstEmit) {
                Vector3 prevVector = (lastEmitPosition - lastLastEmitPosition);
                prevVector.Normalize();
                Vector3 thisVector = (washPosition - lastEmitPosition);
                thisVector.Normalize();
                double cosAngle = prevVector.Dot(thisVector);
                double angle = acos(cosAngle);
                cosAngle = cos(angle * curveGenerationFactor);
                propWashLengthThreshold *= cosAngle;
            }

            if (distanceSinceLastWash > propWashLengthThreshold && params.propWash ) {
                if ( distanceSinceLastWash > minPropSegmentLength) {
                    Vector3 delta;
                    if (!firstEmit) {
                        delta = lastEmitPosition - washPosition;
                        lastLastEmitPosition = lastEmitPosition;
                    } else {
                        delta = dir * -propWashLengthThreshold;
                        firstEmit = false;
                        lastLastEmitPosition = washPosition;
                    }

                    lastEmitSourcePosition = pPosition;
                    lastEmitPosition = washPosition;
                    lastWakeNumber = wakeNumber;
                    wakeManager->AddPropWash(wakeNumber++, washPosition, delta, params.length,
                                             params.beamWidth * params.propWashWidthMultiplier, pTime, params.propWashFadeTime, this);
                }
            }

            if (params.sprayEffects) {

                double period = wakeManager->GetSprayGenerationPeriod();
                if ((pTime - lastSprayEmitTime) > period) {
                    lastSprayEmitTime = pTime;

                    Vector3 sprayOrigin = pPosition + dir * params.bowSprayOffset - (dir * sprayPositionVariation);

                    Vector3 camPos(ocean->GetEnvironment()->GetCameraPosition());
                    double zoomedCull = sprayCullDist * (double)ocean->GetEnvironment()->GetZoomLevel();
                    double sprayCullDistSq = zoomedCull * zoomedCull;
                    if ((camPos - sprayOrigin).SquaredLength() < sprayCullDistSq) {

                        wakeManager->AddSpray(velocity * params.sprayVelocityScale, sprayOrigin, pTime, params.bowSize > 0 ? params.bowSize : -1.0, params.spraySizeScale);

                        if (ocean->GetEnvironment() && params.numHullSprays > 0) {
                            // Add some particle fx on either side of the ship
                            Vector3 up = ocean->GetEnvironment()->GetUpVector();
                            Vector3 right = dir.Cross(up);

                            for (int i = 0; i < params.numHullSprays; i++) {
                                double sideSprayPosition = ocean->GetEnvironment()->GetRandomNumberGenerator()->GetRandomDouble(params.hullSprayStartOffset, params.hullSprayEndOffset);
                                double whichSide = ocean->GetEnvironment()->GetRandomNumberGenerator()->GetRandomInt(0, 2) == 0 ? -1.0 : 1.0;
                                sprayOrigin = pPosition + dir * sideSprayPosition + right * (whichSide * params.beamWidth * 0.5);
                                if (ocean) {
                                    sprayOrigin = sprayOrigin + ocean->GetEnvironment()->GetUpVector() * params.hullSprayVerticalOffset;
                                }
                                wakeManager->AddSpray(velocity * params.hullSprayScale, sprayOrigin, pTime, -1.0, params.hullSpraySizeScale);
                            }
                        }
                    }
                }
            }
        }
    }
}