// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/SprayManager.h>
#include <FeTriton/Environment.h>
#include <FeTriton/ParticleSystem.h>
#include <FeTriton/ProjectedGrid.h>
#include <FeTriton/FFT.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/FFTWaterModel.h>

using namespace Triton;

SprayManager::SprayManager(const Environment *env) : environment(env), grid(0), system(0),
    numSprays(0), sprayEnabled(false)
{

}

SprayManager::~SprayManager()
{
    TRITON_DELETE system;
}

bool SprayManager::Initialize(const ProjectedGrid *pGrid)
{
    if (!pGrid || !environment) return false;

    grid = pGrid;

    return ReadConfigSettings();
}

bool SprayManager::ReadConfigSettings()
{
    sprayEnabled = false;
    Configuration::GetBoolValue("fft-enable-spray", sprayEnabled);

    numSprays = 10000;
    Configuration::GetIntValue("fft-max-sprays", numSprays);

    sprayDirectionVariance = 0.2f;
    Configuration::GetFloatValue("fft-spray-direction-variance", sprayDirectionVariance);
    sprayVelocity = 0.7f;
    Configuration::GetFloatValue("fft-spray-velocity", sprayVelocity);
    sprayVelocityVariance = 0.5f;
    Configuration::GetFloatValue("fft-spray-velocity-variance", sprayVelocityVariance);
    sprayPositionVariance = 0.2f;
    Configuration::GetFloatValue("fft-spray-position-variance", sprayPositionVariance);
    sprayParticleSize = 1.3f;
    Configuration::GetFloatValue("fft-spray-particle-size", sprayParticleSize);
    sprayParticleSizeVariance = 0.3f;
    Configuration::GetFloatValue("fft-spray-particle-size-variance", sprayParticleSizeVariance);
    sprayPositionLead = 0.5f;
    Configuration::GetFloatValue("fft-spray-position-lead", sprayPositionLead);
    sprayBudget = 10;
    Configuration::GetIntValue("fft-spray-budget", sprayBudget);

    sprayPositions.reserve(sprayBudget);

    sprayParticleSize /= (float)environment->GetWorldUnits();
    sprayParticleSizeVariance /= (float)environment->GetWorldUnits();
    sprayPositionVariance /= (float)environment->GetWorldUnits();
    sprayPositionLead /= (float)environment->GetWorldUnits();

    sprayTransparency = 1.0f;
    Configuration::GetFloatValue("fft-spray-transparency", sprayTransparency);

    spraySparsity = 4;
    Configuration::GetIntValue("fft-spray-sparsity", spraySparsity);

    sprayOriginDepth = 0.8f;
    Configuration::GetFloatValue("fft-spray-origin-depth", sprayOriginDepth);
    sprayOriginDepth /= (float)environment->GetWorldUnits();

    sprayGravity = 9.81f;
    Configuration::GetFloatValue("fft-spray-gravity", sprayGravity);

    sprayCullDistance = 1000.0f;
    Configuration::GetFloatValue("fft-spray-cull-distance", sprayCullDistance);
    sprayCullDistance /= (float)environment->GetWorldUnits();

    return true;
}

double inline Repeat(double x, double y)
{
    if (0 == y)
        return x;

    return x - y * floor(x/y);
}

//JWH - added override for patch bounds
bool SprayManager::Draw(double time, float waveHeading, const TBoundingBox* patchBounds)
{
    if (!sprayEnabled || !environment || !grid) {
        return false;
    }

    ReadConfigSettings();

    WaterModel *wm = grid->GetWaterModel();
    if (!wm) {
        return false;
    }
    FFTWaterModel *fwm = dynamic_cast<FFTWaterModel*>(wm);
    if (!fwm) {
        return false;
    }

    double radius = 128;
    double inc = 10.0;

    Vector3 camPos = Vector3(environment->GetCameraPosition());
    const double *cm = environment->GetCameraMatrix();
    Vector3 viewDir(-cm[8], -cm[9], -cm[10]);

    Vector3 camPosZup;
    Vector3 camSeaLevel;
    double elevationSquared = 0;
    Vector3 localReference, north, east;

    if (environment->IsGeocentric()) {
        north = grid->GetNorth();
        Vector3 up = environment->GetUpVector();
        east = north.Cross(up);
        east.Normalize();
        north = up.Cross(east);
        north.Normalize();

        // Sync up with shader reference frame
#define BLOCKSIZE 100000.0
#define OFFSET     50000.0
        Vector3 refPt = camPos + Vector3(OFFSET, OFFSET, OFFSET);
        localReference = Vector3(fmod(refPt.x, BLOCKSIZE), fmod(refPt.y, BLOCKSIZE), fmod(refPt.z, BLOCKSIZE));

        grid->GetIntersection(camPos, environment->GetUpVector() * -1, camSeaLevel);
        camPosZup.x = camSeaLevel.Dot(east);
        camPosZup.y = camSeaLevel.Dot(north);
        elevationSquared = (camPos - camSeaLevel).SquaredLength();

    } else {
        camPosZup = camPos * grid->GetBasis();
        double elevation = camPosZup.z - environment->GetSeaLevel();
        elevationSquared = elevation * elevation;
    }

    double zoomedCullDistance = (double)sprayCullDistance * (double)environment->GetZoomLevel();
    double sprayCullDistanceSquared = zoomedCullDistance * zoomedCullDistance;
    if (elevationSquared > sprayCullDistanceSquared) {
        return true;
    }

    double Lx, Ly;
    Lx = Ly = 1024.0;
    Configuration::GetDoubleValue("fft-grid-size-x", Lx);
    Configuration::GetDoubleValue("fft-grid-size-y", Ly);
    Lx /= environment->GetWorldUnits();
    Ly /= environment->GetWorldUnits();

    double camOffsetX = 0, camOffsetY = 0;
    if (environment->IsGeocentric()) {
        camOffsetX = localReference.Dot(east);
        camOffsetY = localReference.Dot(north);
    }
    double xOffset = Repeat(camPosZup.x + camOffsetX, Lx);
    double yOffset = Repeat(camPosZup.y + camOffsetY, Ly);

    Vector3 tileOrigin[9];
    tileOrigin[0] = Vector3(camPosZup.x - xOffset, camPosZup.y - yOffset, 0);
    tileOrigin[1] = tileOrigin[0] + Vector3(0, Ly, 0);
    tileOrigin[2] = tileOrigin[0] + Vector3(Lx, Ly, 0);
    tileOrigin[3] = tileOrigin[0] + Vector3(Lx, 0, 0);
    tileOrigin[4] = tileOrigin[0] + Vector3(Lx, -Ly, 0);
    tileOrigin[5] = tileOrigin[0] + Vector3(0, -Ly, 0);
    tileOrigin[6] = tileOrigin[0] + Vector3(-Lx, -Ly, 0);
    tileOrigin[7] = tileOrigin[0] + Vector3(-Lx, 0, 0);
    tileOrigin[8] = tileOrigin[0] + Vector3(-Lx, Ly, 0);

    Vector3 tileCenters[9];
    for (int i = 0; i < 9; i++) {
        tileCenters[i] = (tileOrigin[i] + Vector3(Lx * 0.5, Ly * 0.5, 0)) * grid->GetInverseBasis();
        if (environment->IsGeocentric()) {
            tileCenters[i] = tileCenters[i] + camSeaLevel;
        }
    }

    bool somethingIsVisible = false;

    bool tileVisible[9];
    for (int i = 0; i < 9; i++) {
        if (environment->CullSphere(tileCenters[i], Lx * 0.5)) {
            tileVisible[i] = false;
        } else {
            tileVisible[i] = true;
            somethingIsVisible = true;
        }
    }

    if (!somethingIsVisible) {
        return true;
    }

    double windSpeed, windDirection, fetch;
    environment->GetWind(camPos, windSpeed, windDirection, fetch);
    Vector3 waveDir(-sin(waveHeading), -cos(waveHeading), 0.0f);
    waveDir = waveDir * grid->GetInverseBasis();

    int sprayCount = 0;
    sprayPositions.clear();

    const TRITON_VECTOR(Vector3)& sprays = fwm->GetSprays();
	int numSpray = sprays.size();
    //printf("Num sprays %d\n", sprays.size());
    if (numSpray > 0) {
        //TRITON_VECTOR(Vector3)::const_iterator it;
        for (int j = 0; j < numSpray; j++) {
            Vector3 fftPos = sprays[j];
            for (int i = 0; i < 9; i++) {
                if (tileVisible[i]) {
                    Vector3 zUpPos = fftPos + tileOrigin[i];
                    if (!environment->IsGeocentric()) {
                        zUpPos.z += environment->GetSeaLevel();
                    }
                    zUpPos.z -= sprayOriginDepth;

                    Vector3 sprayPos = zUpPos * grid->GetInverseBasis() + waveDir * sprayPositionLead;
                    if (environment->IsGeocentric()) {
                        sprayPos = sprayPos + camSeaLevel;
                    }

                    double zoomedCullDistance = (double)sprayCullDistance * (double)environment->GetZoomLevel();
                    double sprayCullDistanceSquared = zoomedCullDistance * zoomedCullDistance;
                    if ((camPos - sprayPos).SquaredLength() > sprayCullDistanceSquared) continue;

                    if (environment->CullSphere(sprayPos, sprayParticleSize)) continue;

                    //JWH - added patch bounds reject
                    if (patchBounds && !patchBounds->Contains(sprayPos)) {
                        continue;
                    }

                    if (environment->GetRandomNumberGenerator()->GetRandomInt(0, 32768) % spraySparsity == 0) {
                        sprayPositions.push_back(sprayPos);
                        sprayCount++;
                    }
                }
            }

            if (sprayCount > sprayBudget) {
                // printf("Hit spray budget\n");
                break;
            }
        }

        system->StartBatch(time, sprayPositions, waveDir, sprayDirectionVariance,
                           (float)windSpeed * sprayVelocity, sprayVelocityVariance, sprayPositionVariance,
                           sprayParticleSize, sprayParticleSizeVariance);

    }

    system->SetupCommonState(time);
    system->Draw();
    system->CleanupCommonState();

    return true;
}