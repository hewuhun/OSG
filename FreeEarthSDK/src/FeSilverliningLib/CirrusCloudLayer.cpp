// Copyright 2006-2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CirrusCloudLayer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Sky.h>
#include <FeSilverliningLib/Configuration.h>
#include <algorithm>

using namespace SilverLining;

CirrusCloudLayer::CirrusCloudLayer() : CloudLayer()
{

}

CirrusCloudLayer::~CirrusCloudLayer()
{

}

bool CirrusCloudLayer::DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor)
{
    return true;
}

bool CirrusCloudLayer::EndDraw(int pass)
{
    return true;
}

bool CirrusCloudLayer::SeedClouds(const Atmosphere&)
{
    // Single "cloud" for entire layer:
    ClearClouds();
    CirrusCloud *cloud = SL_NEW CirrusCloud(this);
    double x, z;
    GetLayerPosition(x, z);
    double w = GetBaseWidth();
    double h = GetBaseLength();
    cloud->SetWorldPosition(Vector3(x, GetBaseAltitude(), z) * Renderer::GetInstance()->GetBasis3x3());
    cloud->Init(w, h);
    AddCloud(cloud);

    return true;
}

void CirrusCloudLayer::WrapClouds(bool)
{
    if (!clouds.empty() && GetIsInfinite()) clouds.front()->SetNeedsGeocentricPlacement(true);
}

void CirrusCloudLayer::ProcessAtmosphericEffects(Sky *s)
{
    if (!IsRenderable() || GetThickness() <= 0) {
        return;
    }

    Renderer *ren = Renderer::GetInstance();
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos() * ren->GetInverseBasis3x3();

    if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return;
    if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return;
    if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return;
    if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return;

    double cloudBottom = GetBaseAltitudeGeocentric();
    double cloudTop = cloudBottom + GetThickness();

    double r, g, b;
    Color fogColor;
    Configuration::GetDoubleValue("cirrus-fog-red", r);
    Configuration::GetDoubleValue("cirrus-fog-blue", b);
    Configuration::GetDoubleValue("cirrus-fog-green", g);
    double density;
    Configuration::GetDoubleValue("cirrus-fog-density", density);

    double blend = (cloudTop - camPos.y) / GetThickness();

    if (blend > 1.0) blend = 1.0;
    if (blend < 0) blend = 0;

    Color horizonColor = s->GetAverageHorizonColor(0);
    Color sun = s->GetSunOrMoonColor();
    sun.ScaleToUnitOrLess();
    sun = sun.ToGrayscale();

    r = (blend * horizonColor.r) + ((1.0 - blend) * r * sun.r);
    g = (blend * horizonColor.g) + ((1.0 - blend) * g * sun.g);
    b = (blend * horizonColor.b) + ((1.0 - blend) * b * sun.b);
    fogColor = Color(r, g, b, 1.0);

    double fogDepth = 0;
    // Set sky fog effects
    if (camPos.y <= cloudTop && camPos.y >= cloudBottom) {
        fogDepth = cloudTop - camPos.y;
        s->SetFogVolumeDistance(fogDepth);

        ren->EnableFog(true);
        ren->ConfigureFog(density, 1, 100000, fogColor);
        Billboard::SetFogColor(fogColor);
        Billboard::SetFogDensity(density);
    }
}

bool CirrusCloudLayer::RestoreClouds(const Atmosphere& atm, std::istream& s)
{
    int nClouds;
    s.read((char *)&nClouds, sizeof(int));
    if (nClouds != 1) return false;

    return SeedClouds(atm);
}

ShaderHandle CirrusCloudLayer::GetShader() const
{
    if (!clouds.empty()) {
        return clouds.front()->GetShader();
    }

    return 0;
}

void CirrusCloudLayer::ReloadShaders()
{
    if (!clouds.empty()) {
        clouds.front()->ReloadShaders();
    }
}
