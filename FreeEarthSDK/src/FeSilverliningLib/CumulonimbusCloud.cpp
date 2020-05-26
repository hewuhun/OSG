// Copyright (c) 2006-2015 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CumulonimbusCloud.h>
#include <FeSilverliningLib/Voxel.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Virga.h>
#include <FeSilverliningLib/Lightning.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/EnvironmentMap.h>

using namespace SilverLining;
using namespace std;

CumulonimbusCloud::CumulonimbusCloud(CloudLayer *layer) : CumulusCloud(layer)
{
    const char *dischargeModeStr = NULL;
    Configuration::GetStringValue("lightning-discharge-mode", dischargeModeStr);
    dischargeMode = AUTO_DISCHARGE;
    if (dischargeModeStr) {
        if (strcmp(dischargeModeStr, "auto-discharge") == 0) {
            dischargeMode = AUTO_DISCHARGE;
        } else if (strcmp(dischargeModeStr, "fire-and-forget") == 0) {
            dischargeMode = FIRE_AND_FORGET;
        } else if (strcmp(dischargeModeStr, "fire-on-off") == 0) {
            dischargeMode = FORCE_ON_OFF;
        }
    }
    atmosphere = 0;
}

CumulonimbusCloud::~CumulonimbusCloud()
{
    CleanResources();
}

void CumulonimbusCloud::CleanResources()
{
    SL_VECTOR(Virga*) ::iterator vit;
    for (vit = virgaList.begin(); vit != virgaList.end(); vit++) {
        SL_DELETE *vit;
    }

    virgaList.clear();

    SL_VECTOR(Lightning*) ::iterator lit;
    for (lit = lightningList.begin(); lit != lightningList.end(); lit++) {
        SL_DELETE *lit;
    }

    lightningList.clear();

    litVoxels.clear();
}

void CumulonimbusCloud::ComputeLightningLighting(Lightning * l)
{
    double lightningLight = 1E10;
    Configuration::GetDoubleValue("lightning-light", lightningLight);
    bool illuminateCloud = true;
    Configuration::GetBoolValue("cumulonimbus-illuminate-from-lightning", illuminateCloud);
    if (!illuminateCloud) return;

    int i, j, k;
    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                if (voxels[i][j][k]->GetHasCloud()) {
                    const Vector3f& fVoxelPos = voxels[i][j][k]->GetWorldPosition();
                    double dist = l->GetDistance(Vector3(fVoxelPos.x, fVoxelPos.y, fVoxelPos.z));

                    double I = 0;

                    if (dist > 0) {
                        I = lightningLight / (dist * dist);
                    }

                    if (I > 0.1) {
                        if (I > 1.0) I = 1.0;
                        Color lightningColor = Lightning::GetColor();
                        lightningColor = lightningColor * (float)I;
                        LitVoxel lv;
                        lv.lightningColor = lightningColor;
                        lv.origColor = voxels[i][j][k]->GetMetaballColor();
                        lv.voxel = voxels[i][j][k];
                        lv.hasOrigColor = false;
                        litVoxels[l].push_back(lv);
                    }
                }
            }
        }
    }
}

void CumulonimbusCloud::ApplyLightningLighting(Lightning * l)
{
    SL_VECTOR(LitVoxel) ::iterator it;
    for (it = litVoxels[l].begin(); it != litVoxels[l].end(); it++) {
        LitVoxel& v = *it;

        if (!v.voxel->GetHasLightning()) {
            v.voxel->SetHasLightning(true);
            v.hasOrigColor = true;
            v.origColor = v.voxel->GetMetaballColor();;
            Color litColor = v.origColor + v.lightningColor;
            litColor.ClampToUnitOrLess();
            v.voxel->SetMetaballColor(litColor);
        }
    }
}

void CumulonimbusCloud::RestoreLightningLighting(Lightning * l)
{
    SL_VECTOR(LitVoxel) ::iterator it;
    for (it = litVoxels[l].begin(); it != litVoxels[l].end(); it++) {
        LitVoxel& v = *it;
        if (v.hasOrigColor) {
            v.hasOrigColor = false;
            v.voxel->SetMetaballColor(v.origColor);
            v.voxel->SetHasLightning(false);
        }
    }
}

bool CumulonimbusCloud::ForceLightning(bool value)
{
    int nl = (int)lightningList.size();
    if (nl > 0) {
        int n = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, nl-1);
        lightningList[n]->ForceStrike(*atmosphere, value);
        return true;
    }

    return false;
}

bool CumulonimbusCloud::Draw(int pass, const Vector3& lightPos, const Vector3& lightDir,
                             const Color& lightColor, bool invalid, const Sky *sky)
{
    SL_VECTOR(Virga*) ::iterator vit;
    for (vit = virgaList.begin(); vit != virgaList.end(); vit++) {
        (*vit)->SetColor(lightColor);
        (*vit)->Draw(pass);
    }

    bool doLightning = true;
    if (EnvironmentMap::GetIsDrawingEnvMap()) {
        bool envMapLightning = false;
        Configuration::GetBoolValue("environment-maps-have-lightning", envMapLightning);
        if (!envMapLightning) {
            doLightning = false;
        }
    }

    if (pass == 1 && doLightning) {
        // Apply lighting from lightning.
        SL_VECTOR(Lightning*) ::iterator lit;
        for (lit = lightningList.begin(); lit != lightningList.end(); lit++) {
            (*lit)->Draw(pass, *atmosphere);

            if ((*lit)->GetIsDischarging() && !(*lit)->GetWasDischarging() ) {
                ApplyLightningLighting(*lit);
                Invalidate();
            }

            if (!(*lit)->GetIsDischarging() && (*lit)->GetWasDischarging() ) {
                RestoreLightningLighting(*lit);
                Invalidate();
            }
        }
    }

    return CumulusCloud::Draw(pass, lightPos, lightDir, lightColor, invalid, sky);
}

void CumulonimbusCloud::Visit(int pass)
{
    if (pass == 1) {
        // Update lightning.
        SL_VECTOR(Lightning*) ::iterator lit;
        for (lit = lightningList.begin(); lit != lightningList.end(); lit++) {
            (*lit)->Visit(pass, *atmosphere);
        }
    }
}

void CumulonimbusCloud::ApplyWindShear(const Vector3& wind)
{
    double updraft = 10.0; // m/s
    Configuration::GetDoubleValue("cumulonimbus-updraft-speed", updraft);
    updraft *= Atmosphere::GetUnitScale();
    CumulusCloud::Shear(updraft, wind);
}

double CumulonimbusCloud::GetVaporProbability(int i, int j, int k)
{
    double anvilHeight = 0.85;
    Configuration::GetDoubleValue("cumulonimbus-anvil-height", anvilHeight);
    double shaftConstriction = 0.4;
    Configuration::GetDoubleValue("cumulonimbus-shaft-constriction", shaftConstriction);

    double midX = (double)width * towerCenterX;
    double midY = (double)height * anvilHeight;
    double midZ = (double)depth * towerCenterY;

    double halfWidth = (double)width * 0.5;
    double halfDepth = (double)depth * 0.5;
    double a2 = halfWidth * halfWidth;
    double b2 = midY * midY;
    double c2 = halfDepth * halfDepth;

    double x = (double)i - midX;
    double y = (double)k;
    double z = (double)j - midZ;

    double dist;
    if ( (double)k > (double)height * anvilHeight) {
        dist = ((x*x) + (z*z)) / (a2 + c2);
        double distFromAnvilBottom = (k - (height * anvilHeight));
        distFromAnvilBottom /= height * (1.0 - anvilHeight);
        dist *= (1.0 - distFromAnvilBottom);
    } else {
        x *= shaftConstriction;
        z *= shaftConstriction;
        dist = ((x * x) + (y * y) + (z * z)) / (a2 + b2 + c2);
    }

    double scale = (1.0 - dist);
    if (scale < 0) scale = 0;

    double rnd = UniformRandom();
    rnd *= scale;

    if (rnd > 1.0) rnd = 1.0;

    return rnd;
}

void CumulonimbusCloud::InitializeVoxelState()
{
    // Note - cloud world position not yet set at this point, so don't try and use it.

    int i, j, k;
    Vector3 origin(0, 0, 0);
    origin.x -= width * voxelSize * 0.5;
    origin.z -= depth * voxelSize * 0.5;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                Vector3 delta(i * voxelSize, k * voxelSize, j * voxelSize);

                Vector3 pos = origin + delta;

                pos.x += Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, (int)(voxelSize * 0.5) - 1);
                pos.z += Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, (int)(voxelSize * 0.5) - 1);

                voxels[i][j][k]->SetWorldPosition(pos);

                voxels[i][j][k]->SetHasCloud(false);

                double rnd = GetVaporProbability(i, j, k);
                voxels[i][j][k]->SetVapor(rnd < initialVaporProbability);

                voxels[i][j][k]->SetPhaseTransition(false);
            }
        }
    }

    // seed the cloud
    double rndX = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
    double rndY = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();

    towerCenterX = rndX;
    towerCenterY = rndY;

    int X = (int)(rndX * width);
    int Y = (int)(rndY * depth);

    for (i = 0; i < height; i++) {
        voxels[X][Y][i]->SetPhaseTransition(true);
    }

    lastTimeStep = (unsigned long)time(NULL);
}

void CumulonimbusCloud::CloudPlaced(const Atmosphere& atm)
{
    CleanResources();

    atmosphere = &atm;

    double lightningDensity = 0.1;
    Configuration::GetDoubleValue("cumulonimbus-lightning-density", lightningDensity);
    double virgaHeight = 1.3;
    Configuration::GetDoubleValue("cumulonimbus-virga-height", virgaHeight);
    float earthRadius = 6356752.3f;
    Configuration::GetFloatValue("earth-radius-meters-polar", earthRadius);
    earthRadius *= (float)Atmosphere::GetUnitScale();


    // Add virga & lightning
    const char *rainTexStr;
    Configuration::GetStringValue("cumulonimbus-rain-texture", rainTexStr);
    bool hasVirga = (strcmp(rainTexStr, "none") != 0);

    float heightOffset = 0.3f;
    Configuration::GetFloatValue("virga-height-offset", heightOffset);

    Vector3 xformPos = GetWorldPosition() * Renderer::GetInstance()->GetInverseBasis3x3();

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < depth; j++) {
            if (hasVirga) {
                double maxY = voxelSize;
                for (int k = 0; k < height; k++) {
                    if (voxels[i][j][k]->GetHasCloud()) {
                        const Vector3f& fVirgaPos = voxels[i][j][k]->GetWorldPosition();
                        Vector3 virgaPos(fVirgaPos.x, fVirgaPos.y, fVirgaPos.z);
                        Vector3 xformVoxelPos = virgaPos;// * Renderer::GetInstance()->GetInverseBasis3x3();
                        if (xformVoxelPos.y <= maxY) {
                            double adjustedHeight = (xformPos.y + xformVoxelPos.y) * virgaHeight;
                            xformVoxelPos.y = -xformPos.y * 0.5 - heightOffset * adjustedHeight;
                            Virga *virga = SL_NEW Virga(this, voxelSize,
                                                        adjustedHeight,
                                                        xformVoxelPos * Renderer::GetInstance()->GetBasis3x3(),
                                                        atm);
                            virgaList.push_back(virga);
                        }
                    }
                }
            }

            // Add lightning
            if (UniformRandom() < lightningDensity && height > 0) {
                if (voxels[i][j][height-1]->GetHasCloud()) {
                    const Vector3f fVoxelPos = voxels[i][j][height - 1]->GetWorldPosition();
                    Vector3 xformVoxelPos = Vector3(fVoxelPos.x, fVoxelPos.y, fVoxelPos.z);

                    double lightningHeight = xformPos.y + xformVoxelPos.y;
                    if (lightningHeight > earthRadius) { // assume we are in an ecef system
                        lightningHeight -= earthRadius;
                    }
                    Lightning *lightning = SL_NEW Lightning(this, lightningHeight,
                                                            Vector3(fVoxelPos.x, fVoxelPos.y, fVoxelPos.z) * Renderer::GetInstance()->GetBasis3x3());
                    lightningList.push_back(lightning);
                    ComputeLightningLighting(lightning);
                }
            }
        }
    }
}

void CumulonimbusCloud::IncrementTimeStep(unsigned long now)
{
    int i, j, k;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                bool fAct = (
                                (i+1 < width  ? voxels[i+1][j][k]->GetPhaseTransition() : false) ||
                                (j+1 < depth  ? voxels[i][j+1][k]->GetPhaseTransition() : false) ||
                                (i-1 >= 0     ? voxels[i-1][j][k]->GetPhaseTransition() : false) ||
                                (j-1 >= 0     ? voxels[i][j-1][k]->GetPhaseTransition() : false) ||
                                (k-1 >= 0     ? voxels[i][j][k-1]->GetPhaseTransition() : false) ||
                                (i-2 >= 0     ? voxels[i-2][j][k]->GetPhaseTransition() : false) ||
                                (i+2 < width  ? voxels[i+2][j][k]->GetPhaseTransition() : false) ||
                                (j-2 >= 0     ? voxels[i][j-2][k]->GetPhaseTransition() : false) ||
                                (j+2 < depth  ? voxels[i][j+2][k]->GetPhaseTransition() : false)
                            );

                if (k+2 > height) fAct = true; // anvil growth at top

                bool thisAct = voxels[i][j][k]->GetPhaseTransition();

                double rnd = GetVaporProbability(i, j, k);

                voxels[i][j][k]->SetPhaseTransition((
                                                        (!thisAct) &&
                                                        voxels[i][j][k]->GetVapor() &&
                                                        fAct) || (rnd < transitionProbability));

                rnd = GetVaporProbability(i, j, k);
                voxels[i][j][k]->SetVapor(
                    (voxels[i][j][k]->GetVapor() && !thisAct) || (rnd < vaporProbability));

                rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
                bool oldHasCloud = voxels[i][j][k]->GetHasCloud();
                bool newHasCloud = ((oldHasCloud || thisAct) && (rnd > extinctionProbability * (1.0 - rnd)));

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
