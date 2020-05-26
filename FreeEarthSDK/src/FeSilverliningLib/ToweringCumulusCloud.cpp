// Copyright (c) 2014 Sundog Software. All rights reserved worldwide.

#include <FeSilverliningLib/ToweringCumulusCloud.h>
#include <FeSilverliningLib/Voxel.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Configuration.h>
#include <stdlib.h>

using namespace SilverLining;

void ToweringCumulusCloud::InitializeVoxelState()
{
    int i, j, k;
    Vector3 origin = GetWorldPosition();
    origin.x -= width * voxelSize * 0.5;
    origin.z -= depth * voxelSize * 0.5;

    double midX = (double)width * 0.5;
    double midY = (double)height; // Use full height to simulate hemi-ellipsoid
    double midZ = (double)depth * 0.5;

    double a2 = midX * midX;
    double b2 = midY * midY;
    double c2 = midZ * midZ;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                double x = (double)i - midX;
                double y = (double)k;
                double z = (double)j - midZ;
                double dist = (x * x) / a2 + (y * y) / b2 + (z * z) / c2;
                double scale = 1.0 - dist;
                if (scale < 0) scale = 0;

                if (k == 0) scale *= 2.0; // encourage a big base

                Vector3 delta(i * voxelSize, k * voxelSize, j * voxelSize);

                Vector3 pos = origin + delta;

                pos.x += Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, (int)(voxelSize * 0.5) - 1);
                pos.z += Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, (int)(voxelSize * 0.5) - 1);

                voxels[i][j][k]->SetWorldPosition(pos);

                voxels[i][j][k]->SetHasCloud(false);
                double rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
                rnd *= scale;

                voxels[i][j][k]->SetVapor(rnd > (1.0 - initialVaporProbability));

                voxels[i][j][k]->SetPhaseTransition(false);
            }
        }
    }

    // seed the cloud

    if (width >= 4 && depth >= 4) {
        voxels[width >> 2][depth >> 2][0]->SetPhaseTransition(true);
        voxels[width >> 2][depth - (depth >> 2)][0]->SetPhaseTransition(true);
        voxels[width - (width >> 2)][depth - (depth >> 2)][0]->SetPhaseTransition(true);
        voxels[width - (width >> 2)][depth >> 2][0]->SetPhaseTransition(true);
    }

    lastTimeStep = (unsigned long)time(NULL);
}

void ToweringCumulusCloud::ApplyWindShear(const Vector3& wind)
{
    double updraft = 10.0; // m/s
    Configuration::GetDoubleValue("towering-cumulus-updraft-speed", updraft);
    updraft *= Atmosphere::GetUnitScale();
    CumulusCloud::Shear(updraft, wind);
}

void ToweringCumulusCloud::IncrementTimeStep(unsigned long now)
{
    fadeStartTime = now;

    double midX = (double)width * 0.5;
    double midY = (double)height; // Use full height to simulate hemi-ellipsoid
    double midZ = (double)depth * 0.5;

    double a2 = midX * midX;
    double b2 = midY * midY;
    double c2 = midZ * midZ;

    int i, j, k;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                double x = (double)i - midX;
                double y = (double)k;
                double z = (double)j - midZ;
                double dist = (x * x) / a2 + (y * y) / b2 + (z * z) / c2;
                double scale = 1.0 - dist;
                if (scale < 0) scale = 0;

                if (k == 0) scale *= 2.0; // encourage a big base

                bool fAct = (
                                (i+1 < width  ? voxels[i+1][j][k]->GetPhaseTransition() : false) ||
                                (j+1 < depth  ? voxels[i][j+1][k]->GetPhaseTransition() : false) ||
                                //(k+1 < height ? voxels[i][j][k+1]->GetPhaseTransition() : false) ||
                                (i-1 >= 0     ? voxels[i-1][j][k]->GetPhaseTransition() : false) ||
                                (j-1 >= 0     ? voxels[i][j-1][k]->GetPhaseTransition() : false) ||
                                (k-1 >= 0     ? voxels[i][j][k-1]->GetPhaseTransition() : false) ||
                                (i-2 >= 0     ? voxels[i-2][j][k]->GetPhaseTransition() : false) ||
                                (i+2 < width  ? voxels[i+2][j][k]->GetPhaseTransition() : false) ||
                                (j-2 >= 0     ? voxels[i][j-2][k]->GetPhaseTransition() : false) ||
                                (j+2 < depth  ? voxels[i][j+2][k]->GetPhaseTransition() : false) ||
                                (k-2 >= 0     ? voxels[i][j][k-2]->GetPhaseTransition() : false)
                            );

                bool thisAct = voxels[i][j][k]->GetPhaseTransition();

                double rnd;
                rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();

                voxels[i][j][k]->SetPhaseTransition((
                                                        (!thisAct) &&
                                                        voxels[i][j][k]->GetVapor() &&
                                                        fAct) || (rnd < transitionProbability * scale));

                rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
                voxels[i][j][k]->SetVapor(
                    (voxels[i][j][k]->GetVapor() && !thisAct) || (rnd < vaporProbability * scale));

                rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
                bool oldHasCloud = voxels[i][j][k]->GetHasCloud();
                bool newHasCloud = ((oldHasCloud || thisAct) && (rnd > extinctionProbability * (1.0 - scale)));

                float timeStepSeconds = (float)timeStepInterval * 0.001f;
                if (newHasCloud && !oldHasCloud) {
                    voxels[i][j][k]->SetFadeRate(1.0f / timeStepSeconds);
                    voxels[i][j][k]->SetCreated(true);
                    voxels[i][j][k]->SetDestroyed(false);
                } else if (!newHasCloud && oldHasCloud) {
                    voxels[i][j][k]->SetFadeRate(-1.0f / timeStepSeconds);
                    voxels[i][j][k]->SetCreated(false);
                    voxels[i][j][k]->SetDestroyed(true);
                } else {
                    voxels[i][j][k]->SetFadeRate(0.0);
                    voxels[i][j][k]->SetCreated(false);
                    voxels[i][j][k]->SetDestroyed(false);
                }

                voxels[i][j][k]->SetHasCloud(newHasCloud);
            }
        }
    }

    invalid = true;
    lastTimeStep = now;
}
