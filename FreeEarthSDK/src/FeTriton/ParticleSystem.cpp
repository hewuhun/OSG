// Copyright (c) 2011-2013 Sundog Software. All rights reserved worldwide.

#include <FeTriton/ParticleSystem.h>
#include <FeTriton/Vector3.h>
#include <FeTriton/Environment.h>
#include <FeTriton/Configuration.h>

using namespace Triton;

double ParticleSystem::referenceTime = 0;

ParticleSystem::ParticleSystem(const Environment *env) : environment(env), expirationTime(0), offset(0),
    transparency(1.0f), gravity(9.81f)
{
    pureAdditive = false;
    Configuration::GetBoolValue("particles-pure-additive-blend", pureAdditive);

    alphaMask = false;
    Configuration::GetBoolValue("alpha-mask", alphaMask);
}

void ParticleSystem::GetLocalReference(Vector3& referencePt, Vector3& diffFromCamera)
{
#define BLOCKSIZE 100000.0
#define OFFSET     50000.0

    if (environment) {
        Vector3 camPos = Vector3(environment->GetCameraPosition());

        if (environment->IsGeocentric()) {
            Vector3 refPt = camPos + Vector3(OFFSET, OFFSET, OFFSET);
            Vector3 localReference = Vector3(fmod(refPt.x, BLOCKSIZE), fmod(refPt.y, BLOCKSIZE), fmod(refPt.z, BLOCKSIZE));

            referencePt = refPt - localReference;
            diffFromCamera = camPos - referencePt;
        } else {
            referencePt = Vector3(0,0,0);
            diffFromCamera = camPos;
        }
    }
}

void ParticleSystem::ComputeExpirationTime(double pStartTime, double maxP0, double maxV0)
{
    // p(t) = p0 + v0t + 0.5gt^2
    double a = 0.5 * (-gravity / environment->GetWorldUnits());
    double b = maxV0;
    double c = maxP0;
    double discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0) {
        expirationTime = 0;
    } else {
        double root = sqrt(discriminant);
        double t1 = (-b + root) / (2.0 * a);
        double t2 = (-b - root) / (2.0 * a);
        double maxT = t1 > t2 ? t1 : t2;
        expirationTime = pStartTime + maxT;
    }

    startTime = pStartTime;
}

ParticleSystemManager::~ParticleSystemManager()
{
    particleSystems.clear();
}

void ParticleSystemManager::ClearParticleSystems()
{
    particleSystems.clear();
}

void ParticleSystemManager::AddParticleSystem(ParticleSystem *ps)
{
    particleSystems.push_back(ps);
}

void ParticleSystemManager::Update(double time)
{
    if (particleSystems.size() > 0) {

        (*(particleSystems.begin()))->SetupCommonState(time);

        TRITON_LIST(ParticleSystem *)::iterator it = particleSystems.begin();
        while (it != particleSystems.end()) {
            ParticleSystem *ps = *it++;
            double expTime = ps->GetExpirationTime();
            if (time < expTime && expTime > 0) {
                ps->Draw();
            }
        }

        (*(particleSystems.begin()))->CleanupCommonState();
    }
}