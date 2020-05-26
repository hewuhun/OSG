// Copyright (c) 2011 Sundog Software, LLC. All rights reserved worldwide.

#include "GerstnerWaterModel.h"
#include "Configuration.h"
#include "Environment.h"
#include "Matrix4.h"
#include "ProjectedGrid.h"

using namespace Triton;

GerstnerWaterModel::~GerstnerWaterModel()
{
    if (waves) {
        TRITON_DELETE[] waves;
        numWaves = 0;
    }
}

bool GerstnerWaterModel::Initialize(ShaderHandle pShader, ShaderHandle, bool, bool)
{
    if (!pShader) {
        Utils::DebugMsg("Null shader passed into GerstnerWaterModel::Initialize");
        return false;
    }

    shader = pShader;

    numWaves = 4;
    Configuration::GetIntValue("num-gerstner-waves", numWaves);
    if (numWaves <= 0) {
        Utils::DebugMsg("Invalid num-gerstner-waves specified.");
        return false;
    }

    waves = TRITON_NEW GerstnerWave[numWaves];

    float Q = 0.7f;
    Configuration::GetFloatValue("gerstner-wave-steepness", Q);
    if (Q < 0 || Q > 1.0) {
        Utils::DebugMsg("gerstner-waves-steepness should be between 0 and 1.");
    }

    midL = 100.0;
    Configuration::GetDoubleValue("gerstner-median-wavelength-meters", midL);
    midL /= environment->GetWorldUnits();
    if (midL <= 0) {
        Utils::DebugMsg("Invalid gerstner-median-wavelength-meters specified.");
        return false;
    }

    midA = 3.0;
    Configuration::GetDoubleValue("gerstner-median-amplitude-meters", midA);
    midA /= environment->GetWorldUnits();

    double var = 20.0;
    Configuration::GetDoubleValue("gerstner-directional-variance-degrees", var);
    double maxDir = var * 0.5;
    double minDir = -maxDir;

    aOverL = midA / midL;

    float gravity = 9.81f;
    Configuration::GetFloatValue("wave-gravity-force", gravity);
    const float g = gravity / (float)environment->GetWorldUnits();

    maxAmplitude = 0;

    for (int i = 0; i < numWaves; i++) {
        waves[i].L = (float)environment->GetRandomNumberGenerator()->GetRandomDouble(midL * 0.5, midL * 2.0);
        waves[i].amplitude = (float)(waves[i].L * aOverL);

        maxAmplitude += waves[i].amplitude;

        waves[i].frequency = (float)(TRITON_TWOPI / waves[i].L);
        waves[i].phaseSpeed = (float)sqrt(g * (TRITON_TWOPI / waves[i].L));
        waves[i].steepness = Q / (waves[i].frequency * waves[i].amplitude * numWaves);
        waves[i].dirVariance = (float)(RADIANS(environment->GetRandomNumberGenerator()->GetRandomDouble(minDir, maxDir)));
    }

    return true;
}

bool GerstnerWaterModel::PrepShaders(double time)
{
    if (!environment) return false;

    lastTime = time;

    Vector3 camPos(environment->GetCameraPosition());

    double windSpeed, windDirection, fetch;
    environment->GetWind(camPos, windSpeed, windDirection, fetch);

    double sigWaveAmp = 2.0 * sqrt(windSpeed) * 0.66 * 0.5;
    aOverL = sigWaveAmp / midL;

    maxAmplitude = 0;

    for (int i = 0; i < numWaves; i++) {
        waves[i].amplitude = (float)(waves[i].L * aOverL);
        maxAmplitude += waves[i].amplitude;

        double dir = windDirection + waves[i].dirVariance;
        waves[i].dirX = (float)sin(dir);
        waves[i].dirY = (float)cos(dir);
    }

    return true;
}

float GerstnerWaterModel::GetHeight(const Vector3& position, const ProjectedGrid *grid, Vector3& normal, bool)
{
    double s, t;

    // Convert position to texture coords
    if (environment->IsGeocentric()) {
        Vector3 north = grid->GetNorth();
        Vector3 up = environment->GetUpVector();
        Vector3 east = north.Cross(up);
        north = up.Cross(east);
        s = position.Dot(east);
        t = position.Dot(north);
    } else {
        // Transform to z-is-up
        Vector3 xPos = position * grid->GetBasis();
        s = xPos.x;
        t = xPos.y;
    }

    float height = 0;

    Vector3 pt(s, t, 0.0);
    normal = Vector3(0.0, 0.0, 0.0);

    int i;
    for (i = 0; i < numWaves; i++) {
        Vector3 waveDir(waves[i].dirX, waves[i].dirY, 0.0);
        float tmp = waves[i].frequency * (float)(waveDir.Dot(pt)) + waves[i].phaseSpeed * (float)lastTime;
        float S = sin(tmp);
        float WA = waves[i].frequency * waves[i].amplitude;
        float C = cos(tmp);
        float WAC = WA * C;
        height += waves[i].amplitude * S;

        normal = normal + Vector3(waves[i].dirX * WAC,
                                  waves[i].dirY * WAC,
                                  waves[i].steepness * WA * S);
    }

    normal = Vector3(normal.x * -1.0, normal.y * -1.0, 1.0 - normal.z);
    normal.Normalize();

    return height;
}