// Copyright (c) 2006-2009 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CumulonimbusCloudLayer.h>
#include <FeSilverliningLib/CumulonimbusCloud.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Configuration.h>

using namespace SilverLining;

CumulonimbusCloudLayer::CumulonimbusCloudLayer() : CumulusCloudLayer()
{
    ReadConfiguration("cumulonimbus");
}

CumulusCloud *CumulonimbusCloudLayer::CreateCloud(CloudLayer *parentLayer)
{
    return SL_NEW CumulonimbusCloud(parentLayer);
}

bool CumulonimbusCloudLayer::SeedClouds(const Atmosphere& atm)
{
    if (!atm.IsInitialized()) return false;

    ClearClouds();

    ScanCloudFiles(atm);

    SetupSurfaces();

    parameters.width = GetBaseWidth();
    parameters.depth = GetBaseLength();
    parameters.height = GetThickness();
    parameters.spinRate = GetSpinRate();

    if (!CreateCloudFromFile(atm, parameters.width)) {
        CumulusCloud *cloud = CreateCloud(this);
        cloud->Init(parameters);

        double worldWidth, worldDepth, worldHeight;
        cloud->GetSize(worldWidth, worldDepth, worldHeight);
        maxHeight = worldHeight;

        double velocity, heading;
        atm.GetConditions().GetWind(velocity, heading, GetBaseAltitude());
        double dx = localWindX + sin(RADIANS(heading)) * velocity;
        double dz = localWindZ + cos(RADIANS(heading)) * velocity;
        cloud->ApplyWindShear(Vector3(dx, 0, dz) * Renderer::GetInstance()->GetBasis3x3());

        double x, z;
        GetLayerPosition(x, z);
        Vector3 pos(x, GetBaseAltitude(), z);
        pos = pos * Renderer::GetInstance()->GetBasis3x3();
        cloud->SetWorldPosition(pos);
        cloud->SetNeedsGeocentricPlacement(true);

        AddCloud(cloud);

        for (int i = 0; i < parameters.initialEvolveSteps; i++) {
            cloud->IncrementTimeStep(0);
        }

        cloud->CreateImposter();
        cloud->CloudPlaced(atm);
    }

    return true;
}

bool CumulonimbusCloudLayer::ForceLightning(bool value)
{
    if (clouds.size() > 0) {
        CumulonimbusCloud *cl = dynamic_cast<CumulonimbusCloud *>(clouds[0]);
        if (cl) {
            return cl->ForceLightning(value);
        }
    }

    return false;
}
void CumulonimbusCloudLayer::SetLightningDischargeMode(LightningDischargeMode value)
{
    if (clouds.size() > 0) {
        CumulonimbusCloud *cl = dynamic_cast<CumulonimbusCloud *>(clouds[0]);
        if (cl) {
            cl->SetLightningDischargeMode(value);
        }
    }
}
LightningDischargeMode CumulonimbusCloudLayer::GetLightningDischargeMode()
{
    if (clouds.size() > 0) {
        CumulonimbusCloud *cl = dynamic_cast<CumulonimbusCloud *>(clouds[0]);
        if (cl) {
            return cl->GetLightningDischargeMode();
        }
    }

    return CloudLayer::GetLightningDischargeMode();
}
