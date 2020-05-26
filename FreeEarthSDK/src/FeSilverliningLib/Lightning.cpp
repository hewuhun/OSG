// Copyright (c) 2006-2015 Sundog Software. All rights reserved worldwide.

#include <FeSilverliningLib/Lightning.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Cloud.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/ShadowMap.h>

using namespace SilverLining;
using namespace std;

double LightningBranch::glowWidth = 1.5;
Color LightningBranch::lightningColor = Color(0.8, 0.8, 1.0, 1.0);
Color LightningBranch::glowColor = Color(0.8, 0.8, 1.0, 0.3);
double LightningBranch::mainBranchCoeff = 1.0;
double LightningBranch::branchCoeff = 1.0;
float LightningBranch::hdrBoost = 100.0;
float LightningBranch::fogAlpha = 1.0f;

// Uncomment to force lightning to be on at all times
//#define TEST_LIGHTNING

bool LightningBranch::Draw(bool hdr, double distanceFactor)
{
    double glowLineWidth = lineWidth * glowWidth;

    Color glow = glowColor;
    Color main = lightningColor;

    if (hdr) {
        glow = glow * hdrBoost;
        main = main * hdrBoost;
    }

    if (isMainBranch) {
        glow.a *= (float)mainBranchCoeff;
        main.a *= (float)mainBranchCoeff;
    } else {
        glow.a *= (float)branchCoeff;
        main.a *= (float)branchCoeff;
    }

    glow.a *= fogAlpha;
    main.a *= fogAlpha;

    bool fogEnabled = Renderer::GetInstance()->GetFogEnabled();
    Renderer::GetInstance()->EnableFog(false);

    if (glowLineWidth > 1.0) {
        Renderer::GetInstance()->DrawAALines(glow, glowLineWidth * distanceFactor, points);
    }
    Renderer::GetInstance()->DrawAALines(main, lineWidth * distanceFactor, points);

    Renderer::GetInstance()->EnableFog(fogEnabled);

    return true;
}

Lightning::Lightning(Cloud *parentCloud, double pheight, const Vector3& offset) : discharging(false), wasDischarging(false),
    offsetPos(offset), cloud(parentCloud)
{
    height = pheight;

    initialThickness = 3.0;
    maxSegmentsPerBranch = 30;
    maxSegmentLength = 50.0;
    segmentAngleMean = 50.0;
    maxBranchProbability = 0.15;
    minBranchThickness = 0.3;
    maxBranchThickness = 0.5;
    thicknessReduction = 0.8;
    fireDuration = 500;
    maxDischargePeriod = 20000;
    perspectiveDistance = 50000.0;

    Configuration::GetFloatValue("lightning-hdr-boost", LightningBranch::hdrBoost);

    Configuration::GetDoubleValue("lightning-initial-thickness", initialThickness);
    Configuration::GetIntValue("lightning-max-segments-per-branch", maxSegmentsPerBranch);
    Configuration::GetDoubleValue("lightning-max-segment-length", maxSegmentLength);
    maxSegmentLength *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("lightning-max-segment-angle", segmentAngleMean);
    Configuration::GetDoubleValue("lightning-max-branch-probability", maxBranchProbability);
    Configuration::GetDoubleValue("lightning-min-branch-thickness", minBranchThickness);
    Configuration::GetDoubleValue("lightning-max-branch-thickness", maxBranchThickness);
    Configuration::GetDoubleValue("lightning-thickness-reduction", thicknessReduction);
    Configuration::GetDoubleValue("lightning-max-discharge-period", maxDischargePeriod);

    Configuration::GetDoubleValue("lightning-glow-width", LightningBranch::glowWidth);

    Configuration::GetDoubleValue("lightning-perspective-distance", perspectiveDistance);
    perspectiveDistance *= Atmosphere::GetUnitScale();

    double r, g, b, a;
    Configuration::GetDoubleValue("lightning-color-r", r);
    Configuration::GetDoubleValue("lightning-color-g", g);
    Configuration::GetDoubleValue("lightning-color-b", b);
    Configuration::GetDoubleValue("lightning-color-a", a);

    LightningBranch::lightningColor = Color(r, g, b, a);

    Configuration::GetDoubleValue("lightning-glow-color-r", r);
    Configuration::GetDoubleValue("lightning-glow-color-g", g);
    Configuration::GetDoubleValue("lightning-glow-color-b", b);
    Configuration::GetDoubleValue("lightning-glow-color-a", a);

    LightningBranch::glowColor = Color(r, g, b, a);

    //Configuration::GetIntValue("lightning-auto-discharge", autoDischarge);

    int fireDurationInt;
    Configuration::GetIntValue("lightning-discharge-ms", fireDurationInt);
    fireDuration = fireDurationInt;
    forceDischarge = false;

    lastFireTime = nextFireTime = 0;

    double cwidth, cdepth, cheight;
    parentCloud->GetSize(cwidth, cdepth, cheight);
    double cloudDiameter = sqrt(cwidth * cwidth + cdepth * cdepth + cheight * cheight);
    SetDepthOffset(cloudDiameter);

    Generate();
}

Lightning::~Lightning()
{

}

double Lightning::GetDistance(const Vector3& pt)
{
    Vector3 realEnd = endPos + offsetPos;
    Vector3 realStart = startPos + offsetPos;

    Vector3 v = (realEnd - realStart);
    Vector3 w = pt - realStart;

    double c1 = w.Dot(v);
    if (c1 <= 0)
        return (pt - realStart).Length();

    double c2 = v.Dot(v);
    if (c2 <= c1)
        return (pt - realEnd).Length();

    double b = c1 / c2;
    Vector3 Pb = realStart + (v * b);

    return (pt - Pb).Length();
}

void Lightning::ForceStrike(const Atmosphere& atm, bool value)
{
    nextFireTime = atm.GetConditions().GetMillisecondTimer()->GetMilliseconds();
    if (cloud->GetLightningDischargeMode() == FORCE_ON_OFF) {
        forceDischarge = value;
    }
}

bool Lightning::Draw(int pass, const Atmosphere& atm)
{
    // Parameters URS wants adjustible at runtime
    int fireDurationInt;
    Configuration::GetIntValue("lightning-discharge-ms", fireDurationInt);
    fireDuration = fireDurationInt;
    Configuration::GetDoubleValue("lightning-max-discharge-period", maxDischargePeriod);

    hdr = atm.GetHDREnabled();

    if (pass == 1) {
#ifdef TEST_LIGHTNING
        discharging = true;

        LightningBranch::branchCoeff = 1;
        LightningBranch::mainBranchCoeff = 1;

        Renderer::GetInstance()->SubmitBlendedObject(this);
#else
        if (discharging) {
            unsigned long now = atm.GetConditions().GetMillisecondTimer()->GetMilliseconds();
            double t = (double)(now - lastFireTime);
            LightningBranch::branchCoeff = 1.0 - (t / (double)fireDuration);

            LightningBranch::mainBranchCoeff = (cos(t * 0.1) / exp(t * 0.005)) + 0.5;

            if (LightningBranch::mainBranchCoeff < 0) LightningBranch::mainBranchCoeff = 0;
            if (LightningBranch::mainBranchCoeff > 1) LightningBranch::mainBranchCoeff = 1;
            if (LightningBranch::branchCoeff < 0) LightningBranch::branchCoeff = 0;
            if (LightningBranch::branchCoeff > 1) LightningBranch::branchCoeff = 1;

            Renderer::GetInstance()->SubmitBlendedObject(this);
        }
#endif
    }

    return true;
}

void Lightning::Visit(int pass, const Atmosphere& atm)
{
    if (pass == 1) {
        wasDischarging = discharging;

        bool firstFire = (nextFireTime == 0);
        unsigned long now = atm.GetConditions().GetMillisecondTimer()->GetMilliseconds();

        if (now < lastFireTime) { // someone moved time backward
            lastFireTime = nextFireTime = now;
        }

        if (now >= nextFireTime) {
            if (cloud->GetLightningDischargeMode() == AUTO_DISCHARGE) {
                nextFireTime = now + (unsigned long)(UniformRandom() * maxDischargePeriod);
            } else {
                nextFireTime = 4294967295UL; // max 32 bit unsigned
            }

            if (firstFire) return;

            if (cloud->GetLightningDischargeMode() != FORCE_ON_OFF)
                lastFireTime = now;

            cloud->GetParentCloudLayer()->LightningNotify(GetWorldPosition());
        }

        if (cloud->GetLightningDischargeMode() != FORCE_ON_OFF) {
            if (now < (lastFireTime + fireDuration)) {
                discharging = true;
            } else {
                discharging = false;
            }
        } else {
            discharging = forceDischarge;
        }
    }
}

const Vector3& Lightning::GetWorldPosition() const
{
    static Vector3 pos;
    pos = cloud->GetWorldPosition() + offsetPos;
    return pos;
}

Vector3 Lightning::GetSortPosition() const
{
    Vector3 halfHeight = Vector3(0, height * 0.5, 0);
    halfHeight = halfHeight * Renderer::GetInstance()->GetBasis3x3();
    return GetWorldPosition() - halfHeight;
}

void Lightning::DrawBlendedObject()
{
    double distance = 0;
    const Atmosphere *atm = Atmosphere::GetCurrentAtmosphere();
    if (atm) {
        const ShadowMap *sm = atm->GetShadowMapObject();
        if (sm && sm->IsRendering()) return;

        distance = (atm->GetCamPos() - GetWorldPosition()).Length();
        double fogDensity = Billboard::GetFogDensity();
        double fogExponent = fogDensity * distance;
        if (fogExponent < 0) fogExponent = 0;
        if (fogExponent > 1.0) fogExponent = 1.0;

        fogExponent *= fogExponent;

        LightningBranch::fogAlpha = 1.0f - (float)fogExponent;
        if (LightningBranch::fogAlpha < 0) LightningBranch::fogAlpha = 0;
        if (LightningBranch::fogAlpha > 1.0f) LightningBranch::fogAlpha = 1.0f;
    }

    Renderer *renderer = Renderer::GetInstance();

    renderer->PushAllState();
    renderer->UnbindShader(VERTEX_PROGRAM);
    renderer->UnbindShader(FRAGMENT_PROGRAM);
    renderer->EnableLighting(false);

    for (int i = 0; i < 8; i++) {
        renderer->DisableTexture(i);
    }

    renderer->PushTransforms();

    Matrix4 oldMatrix, scalingMatrix;
    renderer->GetModelviewMatrix(&oldMatrix);

    renderer->EnableBlending(SRCALPHA, INVSRCALPHA, true);

    Vector3 pos = GetWorldPosition();
    scalingMatrix.elem[0][3] = pos.x;
    scalingMatrix.elem[1][3] = pos.y;
    scalingMatrix.elem[2][3] = pos.z;

    renderer->MultiplyModelviewMatrix(scalingMatrix);

    renderer->EnableTexture2D(false);

    renderer->EnableDepthReads(true);

    double normDistance = distance / perspectiveDistance;
    if (normDistance > 1.0) normDistance = 1.0;
    double distanceFactor = 1.0 - normDistance;
    SL_VECTOR(LightningBranch) ::iterator it;
    for (it = branches.begin(); it != branches.end(); it++) {
        (*it).Draw(hdr, distanceFactor);
    }

    renderer->SetModelviewMatrix(oldMatrix);

    renderer->EnableTexture2D(true);

    renderer->PopTransforms();
    renderer->PopAllState();
}

void Lightning::GetBranchAngles(double seedTheta, double seedPhi, double& theta, double& phi)
{
    theta = RADIANS(segmentAngleMean);
    phi = RADIANS(segmentAngleMean);

    theta = seedTheta + ((UniformRandom() * (theta * 2.0)) - theta);
    phi = seedPhi + ((UniformRandom() * (phi * 2.0)) - phi);
}

Vector3 Lightning::GenerateSegment(const Vector3& lastPoint, double seedTheta, double seedPhi,
                                   double& theta, double& phi)
{
    GetBranchAngles(seedTheta, seedPhi, theta, phi);

    double segmentLength = UniformRandom() * maxSegmentLength;

    Vector3 delta;
    delta.x = segmentLength * sin(theta) * cos(phi);
    delta.z = -(segmentLength * sin(theta) * sin(phi));
    delta.y = -(segmentLength * cos(theta));

    delta = delta * Renderer::GetInstance()->GetBasis3x3();

    return lastPoint + delta;
}

void Lightning::GenerateBranch(double theta, double phi, double thickness, const Vector3& startPos,
                               int segmentsLeft, LightningBranch& branch)
{
    Matrix3 invBasis = Renderer::GetInstance()->GetInverseBasis3x3();

    Vector3 pt = startPos;

    branch.points.push_back(startPos);

    while (segmentsLeft > 0) {
        double newTheta, newPhi;
        pt = GenerateSegment(pt, theta, phi, newTheta, newPhi);

        branch.points.push_back(pt);
        segmentsLeft--;

        if ((pt * invBasis).y < -height)
            return;

        if (UniformRandom() < BranchProbability((pt * invBasis).y)) {
            LightningBranch newBranch;
            newBranch.lineWidth = thickness * thicknessReduction;
            GenerateBranch(newTheta, newPhi, thickness * thicknessReduction, pt, segmentsLeft, newBranch);
            branches.push_back(newBranch);
        }
    }
}

double Lightning::BranchProbability(double y)
{
    double pFromTop = fabs(y) / height;
    return pFromTop * maxBranchProbability;
}

bool Lightning::Generate()
{
    LightningBranch mainChannel;
    mainChannel.lineWidth = initialThickness;

    Vector3 lastPt(0,0,0);
    startPos = lastPt;
    mainChannel.points.push_back(lastPt);
    mainChannel.isMainBranch = true;

    while ((lastPt * Renderer::GetInstance()->GetInverseBasis3x3()).y > -height) {
        double theta, phi;
        Vector3 nextPoint = GenerateSegment(lastPt, 0, 0, theta, phi);
        mainChannel.points.push_back(nextPoint);

        // Spawn a branch?
        if (UniformRandom() < BranchProbability((lastPt * Renderer::GetInstance()->GetInverseBasis3x3()).y)) {
            double branchThickness = UniformRandom() * (maxBranchThickness - minBranchThickness) +
                                     minBranchThickness;
            branchThickness *= initialThickness;
            LightningBranch newBranch;
            newBranch.lineWidth = branchThickness;
            int numSegments = (int)(UniformRandom() * maxSegmentsPerBranch);
            GenerateBranch(theta, phi, branchThickness, nextPoint, numSegments, newBranch);
            branches.push_back(newBranch);
        }

        lastPt = nextPoint;
    }

    endPos = lastPt;
    branches.push_back(mainChannel);

    return true;
}
