// Copyright (c) 2006-2013 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/Virga.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Cloud.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/ShadowMap.h>
#include <string>

using namespace SilverLining;
using namespace std;

TextureHandle Virga::texture = 0;

Virga::Virga(Cloud *parentCloud, double pDiameter, double height, const Vector3& offset,
             const Atmosphere& atm) :  offsetPos(offset), cloud(parentCloud), atmosphere(atm)
{
    diameter = pDiameter;

    LoadTexture();
    billboard = SL_NEW Billboard();
    billboard->Initialize(false, (float)diameter);

    double cwidth, cdepth, cheight;
    parentCloud->GetSize(cwidth, cdepth, cheight);
    double cloudRadius = sqrt(cwidth * cwidth + cdepth * cdepth + cheight * cheight) * 0.5;
    SetDepthOffset(cloudRadius);

    virgaAlpha = 0.3f;
    Configuration::GetFloatValue("virga-alpha", virgaAlpha);

    driftSpeed = 0.1f;
    driftRadius = 0.25f;
    Configuration::GetFloatValue("virga-drift-speed", driftSpeed);
    Configuration::GetFloatValue("virga-drift-radius", driftRadius);

    fadeDistance = 1000.0f;
    Configuration::GetFloatValue("virga-fade-distance", fadeDistance);
    fadeDistance *= (float)atm.GetUnitScale();

    yScale = height / diameter;

    driftOffset = atm.GetRandomNumberGenerator()->UniformRandomFloat() * 100.0f;
}

Virga::~Virga()
{
    SL_DELETE billboard;
}

void Virga::SetColor(const Color& c)
{
    Color color = c;
    color.ClampToUnitOrLess();
    color = color.ToGrayscale();

    float virgaDistance = (float)(GetWorldPosition() - Atmosphere::GetCurrentAtmosphere()->GetCamPos()).Length();
    float fade = 1.0f - expf(-virgaDistance / fadeDistance);
    color.a = virgaAlpha * fade;

    billboard->SetColor(color);
}

bool Virga::Draw(int pass)
{
    if (pass == 1) {
        Renderer::GetInstance()->SubmitBlendedObject(this);
    }

    return true;
}

void Virga::UpdateDrift()
{
    unsigned long millis = Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetMillisecondTimer()->GetMilliseconds();
    float t = ((float)millis * 0.001f + driftOffset) * driftSpeed;

    float radius = (float)diameter * driftRadius;
    drift.x = cosf(t) * radius;
    drift.z = sinf(t) * radius;
}

const Vector3& Virga::GetWorldPosition() const
{
    static Vector3 pos;
    pos = cloud->GetWorldPosition() + offsetPos;
    return pos;
}

void Virga::DrawBlendedObject()
{
    const ShadowMap *sm = atmosphere.GetShadowMapObject();
    if (sm && sm->IsRendering()) return;

    Renderer *renderer = Renderer::GetInstance();

    Vector3 pos = GetWorldPosition();
    //Vector3 posXlate = pos * renderer->GetInverseBasis3x3();

    Matrix4 rotMatrix = renderer->GetBillboardMatrix();

#if 0
    double windVel, windHead;
    atmosphere.GetConditions().GetWind(windVel, windHead, posXlate.y);
    double windX, windZ;
    atmosphere.GetConditions().GetPrecipitationWind(windX, windZ);
    double dx = windX + sin(RADIANS(windHead)) * windVel;
    double dz = windZ + cos(RADIANS(windHead)) * windVel;
    Vector3 windVelVector(dx, 0, dz);
    double rainTerminalVelocity = 8.0;
    double Cd = 0.1;
    double m = 0.034 * 0.001;
    double dropRadius = 0.0013;
    double maxWindVelocity = 5.0;
    const double p = 1.29; // kg / m3

    Configuration::GetDoubleValue("raindrop-drag-coefficient", Cd);
    Configuration::GetDoubleValue("raindrop-mass-kg", m);
    Configuration::GetDoubleValue("raindrop-radius-m", dropRadius);
    dropRadius *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("raindrop-terminal-velocity", rainTerminalVelocity);
    rainTerminalVelocity *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("raindrop-max-wind-velocity", maxWindVelocity);
    maxWindVelocity *= Atmosphere::GetUnitScale();

    double windVelocity = windVelVector.Length();
    if (windVelocity > maxWindVelocity) windVelocity = maxWindVelocity;
    double A = PI * dropRadius * dropRadius;
    double Fd = 0.5 * p * Cd * A * windVelocity * windVelocity;
    double a = Fd / m;
    double t = posXlate.y / rainTerminalVelocity;
    if (t < 0) t = 0;
    double d = 0.5 * a * t * t;

    windVelVector.Normalize();
    windVelVector = windVelVector * d;

    Vector3 rainDir = Vector3(0, -rainTerminalVelocity, 0);
    if (t > 0) {
        rainDir = Vector3(windVelVector.x, -rainTerminalVelocity * t, windVelVector.z);
    }

    rainDir = rainDir * renderer->GetBasis3x3();

    Matrix4 modelview;
    renderer->GetModelviewMatrix(&modelview);
    Vector3 camDir(-modelview.elem[2][0], -modelview.elem[2][1], -modelview.elem[2][2]);

    Vector3 up = rainDir * -1.0;
    up.Normalize();

    Vector3 in = camDir * -1;
    Vector3 right = up.Cross(in);
    right.Normalize();
    in = right.Cross(up);
    in.Normalize();

    rotMatrix.elem[0][0] = right.x;
    rotMatrix.elem[0][1] = right.y;
    rotMatrix.elem[0][2] = right.z;
    rotMatrix.elem[1][0] = up.x;
    rotMatrix.elem[1][1] = up.y;
    rotMatrix.elem[1][2] = up.z;
    rotMatrix.elem[2][0] = in.x;
    rotMatrix.elem[2][1] = in.y;
    rotMatrix.elem[2][2] = in.z;

    rotMatrix.Transpose();
#endif

    Matrix4 scale;
    scale.elem[1][1] = yScale;

    if (renderer->GetType() == Atmosphere::DIRECTX11 || renderer->GetType() == Atmosphere::DIRECTX11_1) {
        scale.elem[1][1] *= -1.0;
    }

    UpdateDrift();
    billboard->SetWorldPosition(pos + drift * renderer->GetBasis3x3());

    renderer->PushAllState();

    renderer->SetDefaultState();

    renderer->EnableBlending(SRCALPHA, INVSRCALPHA);
    renderer->EnableDepthWrites(false);
    Billboard::OverrideBillboardMatrix(rotMatrix * scale);
    billboard->Draw(texture, 1, cloud->GetFade() * cloud->GetAlpha(), false);
    Billboard::RestoreBillboardMatrix();
    renderer->EnableBlending(SRCALPHA, INVSRCALPHA);
    renderer->EnableDepthWrites(true);
    //renderer->EnableBackfaceCulling(true);
    renderer->PopAllState();
}

bool Virga::LoadTexture()
{
    if (texture == 0) {
        const char *rainTex;
        Configuration::GetStringValue("cumulonimbus-rain-texture", rainTex);
        return Renderer::GetInstance()->LoadTextureFromFile(rainTex, &texture, true, false);
    }

    return false;
}

