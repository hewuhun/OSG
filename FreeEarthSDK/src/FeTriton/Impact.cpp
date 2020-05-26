// Copyright (c) 2013 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/Impact.h>
#include <FeTriton/Ocean.h>
#include <FeTriton/WakeManager.h>
#include <FeTriton/Configuration.h>

using namespace Triton;

Impact::Impact(Ocean *pOcean, double pImpactorDiameter, double pMass, bool pSprayEffects, double pSprayScale,double waveFadeInTime)
    : ocean(pOcean), mass(pMass), sprayEffects(pSprayEffects), wakeNumber(0), impactorDiameter(pImpactorDiameter),
      sprayScale(pSprayScale),waveFadeInTime(waveFadeInTime)
{
    numSprays = 10;
    Configuration::GetIntValue("impact-num-sprays", numSprays);

    energyScale = 0.00001;
    Configuration::GetDoubleValue("impact-energy-scale", energyScale);

    maxEnergy = 20.0;
    Configuration::GetDoubleValue("impact-max-energy", maxEnergy);

    positionVariation = 1.0;
    Configuration::GetDoubleValue("impact-spray-position-variation", positionVariation);

    transparency = 0.2f;
    Configuration::GetFloatValue("impact-spray-transparency", transparency);

    decayRate = 1.0 / 3.0;
    Configuration::GetDoubleValue("impact-wave-decay", decayRate);
}

void Impact::Trigger(const Vector3& pPosition, const Vector3& direction, double pVelocity, double pTime)
{
    if (pVelocity <= 0) return;

    Vector3 dir = direction;
    dir.Normalize();

    velocity = pVelocity;

    if (ocean && ocean->GetIntersection(pPosition, dir, position)) {

        double distance = (pPosition - position).Length();
        double energy = 0.5 * mass * velocity * velocity;
        energy *= energyScale;

        if (energy > maxEnergy) {
            energy = maxEnergy;
        }

        WakeManager *wakeManager = ocean->GetWakeManager();
        if (wakeManager) {

            wakeManager->AddCircularWave(energy, position, pTime,waveFadeInTime,Vector3(0, 0, 0), 0, decayRate, NULL);

            if (sprayEffects) {
                const Environment *env = ocean->GetEnvironment();
                Vector3 up = env->GetUpVector();
                Vector3 xAxis = up.Cross(dir);
                if (xAxis == Vector3(0, 0, 0)) {
                    Vector3 right = env->GetRightVector();
                    xAxis = right.Cross(dir);
                }
                xAxis.Normalize();
                Vector3 yAxis = xAxis.Cross(dir);

                for (int i = 0; i < numSprays; i++) {
                    double t = 2.0 * TRITON_PI * env->GetRandomNumberGenerator()->GetRandomDouble(0.0, 1.0);
                    double u = env->GetRandomNumberGenerator()->GetRandomDouble(0.0, 1.0) +
                               env->GetRandomNumberGenerator()->GetRandomDouble(0.0, 1.0);
                    double r = u > 1.0 ? 2.0 - u : u;
                    r *= impactorDiameter;
                    double x = r * cos(t);
                    double y = r * sin(t);

                    Vector3 originPoint = pPosition + xAxis * x + yAxis * y;

                    Vector3 sprayIntersection;
                    if (ocean->GetIntersection(originPoint, dir, sprayIntersection)) {
                        // Reflect off the water surface to get the spray direction
                        Vector3 L = pPosition - sprayIntersection;
                        L.Normalize();
                        Vector3 R = (up * (2.0 * (up.Dot(L)))) - L;
                        wakeManager->AddDirectionalSpray(energy * sprayScale, sprayIntersection, R, pTime, -1.0, transparency, positionVariation);
                    }
                }
            }
        }
    }
}