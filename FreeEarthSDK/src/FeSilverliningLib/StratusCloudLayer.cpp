// Copyright 2006-2009 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/StratusCloudLayer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/PrecipitationManager.h>
#include <FeSilverliningLib/Sky.h>
#include <algorithm>

using namespace SilverLining;

StratusCloudLayer::StratusCloudLayer() : CloudLayer()
{

}

StratusCloudLayer::~StratusCloudLayer()
{

}

void StratusCloudLayer::ComputeBounds()
{
    Vector3 minimum, maximum;
    Renderer *ren = Renderer::GetInstance();
    if (!ren) return;

    double x, z;
    if (GetIsInfinite()) {
        Vector3 camPosYUp = Atmosphere::GetCurrentAtmosphere()->GetCamPos() * ren->GetInverseBasis3x3();
        x = camPosYUp.x;
        z = camPosYUp.z;
    } else {
        x = layerX;
        z = layerZ;
    }

    double cloudWidth, cloudDepth, cloudHeight;
    myCloud->GetSize(cloudWidth, cloudDepth, cloudHeight);

    minimum.x = x - cloudWidth * 0.5;
    minimum.y = GetBaseAltitudeGeocentric();
    minimum.z = z - cloudDepth * 0.5;
    maximum.x = x + cloudWidth * 0.5;
    maximum.y = GetBaseAltitudeGeocentric() + GetMaxHeight();
    maximum.z = z + cloudDepth * 0.5;

    Matrix3 basis = GetLocalBasis();
    bounds[0] = Vector3(minimum.x, minimum.y, minimum.z) * basis;
    bounds[1] = Vector3(minimum.x, minimum.y, maximum.z) * basis;
    bounds[2] = Vector3(minimum.x, maximum.y, minimum.z) * basis;
    bounds[3] = Vector3(minimum.x, maximum.y, maximum.z) * basis;
    bounds[4] = Vector3(maximum.x, minimum.y, minimum.z) * basis;
    bounds[5] = Vector3(maximum.x, minimum.y, maximum.z) * basis;
    bounds[6] = Vector3(maximum.x, maximum.y, minimum.z) * basis;
    bounds[7] = Vector3(maximum.x, maximum.y, maximum.z) * basis;
}


bool StratusCloudLayer::HasPrecipitationAtPosition(double x, double y, double z) const
{
    if (precipitationEffects.size() == 0 || !IsRenderable()) return false;

    Matrix3 invBasis = GetLocalBasis().Transpose();
    Vector3 camPos = Vector3(x, y, z) * invBasis;

    if (!GetIsInfinite()) {
        if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return false;
        if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return false;
        if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return false;
        if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return false;
    }

    return (camPos.y < GetBaseAltitudeGeocentric());
}

bool StratusCloudLayer::IsInsideCloud(double x, double y, double z) const
{
    Matrix3 invBasis = GetLocalBasis().Transpose();
    Vector3 camPos = Vector3(x, y, z) * invBasis;

    if (!GetIsInfinite()) {
        if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return false;
        if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return false;
        if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return false;
        if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return false;
    }

    return (camPos.y >= GetBaseAltitudeGeocentric() && camPos.y <= (GetBaseAltitudeGeocentric() + GetThickness()) );
}

void StratusCloudLayer::UpdateOvercast(Sky *s)
{
    if (!IsRenderable()) {
        return;
    }

    Renderer *ren = Renderer::GetInstance();
    Matrix3 invBasis = GetLocalBasis().Transpose();
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos() * invBasis;
    double cloudBottom = GetBaseAltitudeGeocentric();
    double cloudTop = cloudBottom + GetThickness();

    double overcastThreshold = 0.5;
    Configuration::GetDoubleValue("stratus-overcast-threshold", overcastThreshold);

    double ambientTransmission = 0.2, directTransmission = 0;
    Configuration::GetDoubleValue("stratus-light-transmission-ambient", ambientTransmission);
    Configuration::GetDoubleValue("stratus-light-transmission-direct", directTransmission);

    double blend = (cloudTop - camPos.y) / GetThickness();

    if (blend > 1.0) blend = 1.0;
    if (blend < 0) blend = 0;

    if (GetDensity() > 0 && GetIsInfinite()) {
        // Set overcast sky effects
        if (camPos.y < cloudTop) {
            s->SetOvercast(GetDensity() > overcastThreshold);
            s->SetOvercastParams(blend * fade * alpha * GetDensity(), ambientTransmission * GetDensity(),
                                 directTransmission * GetDensity());
        }
    }
}

void StratusCloudLayer::ProcessAtmosphericEffects(Sky *s)
{
    if (!IsRenderable()) {
        return;
    }

    Renderer *ren = Renderer::GetInstance();
    Matrix3 invBasis = GetLocalBasis().Transpose();
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos() * invBasis;

    if (!GetIsInfinite()) {
        if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return;
        if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return;
        if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return;
        if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return;
    }

    double scudThickness;
    Configuration::GetDoubleValue("stratus-scud-thickness", scudThickness);
    scudThickness *= Atmosphere::GetUnitScale();

    double cloudBottom = GetBaseAltitudeGeocentric();
    double cloudTop = cloudBottom + GetThickness();
    //double cloudMiddle = (cloudBottom + cloudTop) * 0.5;
    double scudBottom = cloudBottom - scudThickness;
    double scudTop = cloudTop + scudThickness;

    double r, g, b;
    Color fogColor;
    Configuration::GetDoubleValue("stratus-fog-red", r);
    Configuration::GetDoubleValue("stratus-fog-blue", b);
    Configuration::GetDoubleValue("stratus-fog-green", g);
    double density;
    Configuration::GetDoubleValue("stratus-fog-density", density);

    //UpdateOvercast(s);

    double blend = (cloudTop - camPos.y) / GetThickness();

    if (blend > 1.0) blend = 1.0;
    if (blend < 0) blend = 0;

    Color horizonColor;
    if (s->GetOvercast()) {
        horizonColor = s->GetOvercastHorizonColor();
    } else {
        horizonColor = s->GetAverageHorizonColor(0);
    }

    Color sun = s->GetSunOrMoonColor();
    sun.ScaleToUnitOrLess();
    sun = sun.ToGrayscale();

    bool lightFog = true;
    Configuration::GetBoolValue("stratus-light-fog", lightFog);
    blend *= blend * blend;
    if (lightFog) {
        r = (blend * horizonColor.r) + ((1.0 - blend) * r * sun.r);
        g = (blend * horizonColor.g) + ((1.0 - blend) * g * sun.g);
        b = (blend * horizonColor.b) + ((1.0 - blend) * b * sun.b);
    }
    fogColor = Color(r, g, b, 1.0);

    double fogDepth = 0;
    // Set sky fog effects
    if (camPos.y <= scudTop && camPos.y >= scudBottom) {
        fogDepth = scudTop - camPos.y;
        s->SetFogVolumeDistance(fogDepth);
    }

    double precipitationFogDensity = PrecipitationManager::GetInstance(Atmosphere::GetCurrentAtmosphere())->GetFogDensity();
    if (precipitationFogDensity > 0) {
        ren->EnableFog(true);
        ren->ConfigureFog(precipitationFogDensity, 1, 100000, fogColor);
        Billboard::SetFogColor(fogColor);
        Billboard::SetFogDensity(precipitationFogDensity);
        s->SetFogVolumeDistance(1.0 / precipitationFogDensity);
    }

    if (myCloud) {
        const double epsilon = 0.001;
        float scud = myCloud->LookupScud(Atmosphere::GetCurrentAtmosphere()->GetCamPos());

        //printf("scud %f\n", scud);

        if (scud > epsilon) {
            double cloudFogDensity = scud * density;// *(fogDepth / scudThickness);
            if (precipitationFogDensity > cloudFogDensity) cloudFogDensity = precipitationFogDensity;

            cloudFogDensity = cloudFogDensity * GetFade() + 1.0E-9 * (1.0 - GetFade());

            if (cloudFogDensity > Billboard::GetFogDensity()) {
                ren->EnableFog(true);
                ren->ConfigureFog(cloudFogDensity, 1, 100000, fogColor);
                Billboard::SetFogColor(fogColor);
                Billboard::SetFogDensity(cloudFogDensity);
            }
        }
    }
}

bool StratusCloudLayer::DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor)
{
    return true;
}

bool StratusCloudLayer::EndDraw(int pass)
{
    return true;
}

bool StratusCloudLayer::SeedClouds(const Atmosphere&)
{
    ClearClouds();

    // Single "cloud" for entire layer, stretching to "infinity" (100km)
    double w, h;

    if (isInfinite) {
        Configuration::GetDoubleValue("stratus-deck-width", w);
        Configuration::GetDoubleValue("stratus-deck-height", h);
        w *= Atmosphere::GetUnitScale();
        h *= Atmosphere::GetUnitScale();
    } else {
        w = GetBaseWidth();
        h = GetBaseLength();
    }

    myCloud = SL_NEW StratusCloud(this);
    double x, z;
    GetLayerPosition(x, z);
    myCloud->SetWorldPosition(Vector3(x, GetBaseAltitude(), z) * Renderer::GetInstance()->GetBasis3x3());
    myCloud->Init(w, h, GetThickness(), GetDensity());
    AddCloud(myCloud);

    return true;
}

void StratusCloudLayer::WrapClouds(bool)
{
}

bool StratusCloudLayer::RestoreClouds(const Atmosphere& atm, std::istream& s)
{
    int nClouds;
    s.read((char *)&nClouds, sizeof(int));
    if (nClouds != 1) return false;

    return SeedClouds(atm);
}

ShaderHandle StratusCloudLayer::GetShader() const
{
    if (!clouds.empty()) {
        return clouds.front()->GetShader();
    }

    return 0;
}

void StratusCloudLayer::ReloadShaders()
{
    if (!clouds.empty()) {
        clouds.front()->ReloadShaders();
    }
}
