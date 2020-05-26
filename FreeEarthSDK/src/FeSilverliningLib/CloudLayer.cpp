// Copyright (c) 2004-2015  Sundog Software, LLC. All rights reserved worldwide.
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Cloud.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/LightningListener.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/PrecipitationManager.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Sky.h>
#include <FeSilverliningLib/CloudBackdropManager.h>
#include <fstream>
#include <cfloat>
#include <assert.h>
#include <algorithm>

using namespace SilverLining;
using namespace std;

#define CLOUDLAYER_SERIALIZE_VERSION 10

CloudLayer::CloudLayer()
{
    baseWidth = 2000;
    baseLength = 2000;
    baseAltitude = 1000;
    thickness = 700;
    density = 0.5;
    layerX = 0;
    layerZ = 0;
    layerEnabled = renderingEnabled = true;
    forceLightingUpdate = false;
    lightningListeners.clear();
    fadeMode = NO_FADE;
    lastCloudUpdated = 0;
    fade = fadeBegin = 1.0;
    alpha = 1.0;
    fadeStart = fadeTime = 0;
    firstFrameDrawn = false;
    isInfinite = false;
    localWindX = 0.0;
    localWindZ = 0.0;
    cloudWrap = false;
    fadeTowardEdges = false;
    growth = false;
    culled = false;
    enableCulling = true;
    coverageMultiplier = 1.0;
    usingRuntimeDensity = false;
    curveTowardGround = false;
    hasForcedCloudColor = false;
    forcedCloudColorLighting = true;
    usingCloudAtlas = false;
    soft = false;

    Configuration::GetBoolValue("enable-cloud-layer-culling", enableCulling);

    double eqRad = 6378137.00, polRad = 6356752.3142;

    Configuration::GetDoubleValue("earth-radius-meters-equatorial", eqRad);
    Configuration::GetDoubleValue("earth-radius-meters-polar", polRad);

    eqRad *= Atmosphere::GetUnitScale();
    polRad *= Atmosphere::GetUnitScale();

    edgeFadeThreshold = 0.1;
    Configuration::GetDoubleValue("cloud-edge-fade-threshold", edgeFadeThreshold);

    bool zup = true;
    Configuration::GetBoolValue("geocentric-z-is-up", zup);

    if (zup) {
        earthRadii = Vector3(eqRad, eqRad, polRad);
    } else {
        earthRadii = Vector3(eqRad, polRad, eqRad);
    }

}


void CloudLayer::OverrideCloudColor(const Vector3& color, bool doLighting)
{
    hasForcedCloudColor = true;
    forcedCloudColorLighting = doLighting;
    forcedCloudColor = color;
}

void CloudLayer::ClearCloudColorOverride()
{
    hasForcedCloudColor = false;
}

bool CloudLayer::GetOverriddenCloudColor(Vector3& color, bool& doLighting) const
{
    if (hasForcedCloudColor) {
        color = forcedCloudColor;
        doLighting = forcedCloudColorLighting;
    }
    return hasForcedCloudColor;
}

CloudLayer::~CloudLayer()
{
    ClearClouds();

    lightningListeners.clear();
}

void CloudLayer::ClearClouds()
{
    SL_VECTOR(Cloud*) ::iterator it;

    for (it = clouds.begin(); it != clouds.end(); it++) {
        SL_DELETE (*it);
    }

    clouds.clear();
}

const Vector3& CloudLayer::GetLocalUpVector() const
{
    if (!isInfinite) {
        return localUp;
    } else {
        return Renderer::GetInstance()->GetUpVector();
    }
}

const Matrix3& CloudLayer::GetLocalBasis() const
{
    if (!isInfinite) {
        return localBasis;
    } else {
        return Renderer::GetInstance()->GetBasis3x3();
    }
}

const Matrix4& CloudLayer::GetLocalBasis4() const
{
    if (!isInfinite) {
        return localBasis4;
    } else {
        return Renderer::GetInstance()->GetBasis4x4();
    }
}

void CloudLayer::SetEnabled(bool enabled, unsigned long fadeTimeMS)
{
    layerEnabled = enabled;

    fadeTime = fadeTimeMS;
    fadeStart = 0;
    fadeBegin = fade;

    if (fadeTimeMS <= 0) {
        fadeMode = NO_FADE;
        renderingEnabled = enabled;
        fade = fadeBegin = enabled ? 1.0 : 0.0;
    } else {
        if (enabled) {
            fadeMode = FADE_IN;
            renderingEnabled = true;
        } else {
            fadeMode = FADE_OUT;
        }
    }

    forceLightingUpdate = true;
}

void CloudLayer::ProcessFade(unsigned long now)
{
    if (fadeMode == NO_FADE) {
        fade = 1.0;
    } else {
        if (fadeStart == 0) {
            fadeStart = now;
        }
        unsigned long dt = now - fadeStart;
        double fadePer = (double)dt / (double)fadeTime;
        if (fadePer > 1.0) fadePer = 1.0;

        if (fadeMode == FADE_IN) {
            fade = fadeBegin + fadePer * (1.0 - fadeBegin);
        } else {
            fade = fadeBegin - fadePer * fadeBegin;
        }

        if (fadePer >= 1.0) {
            if (fadeMode == FADE_OUT) renderingEnabled = false;
            fadeMode = NO_FADE;
        }
    }
}

void CloudLayer::SetBaseAltitude(double meters, bool updateCloudPositions)
{
    baseAltitude = meters;

    if (updateCloudPositions) {
        SetLocalUpVector(Renderer::GetInstance()->GetUpVector());
        SetLocalBasis(Renderer::GetInstance()->GetBasis3x3());
        SetLocalBasis4(Renderer::GetInstance()->GetBasis4x4());

        SL_VECTOR(Cloud *) ::iterator it;
        for (it = clouds.begin(); it != clouds.end(); it++) {
            Cloud *cloud = *it;

            Vector3 cloudPos = cloud->GetWorldPosition();
            cloudPos = cloudPos * Renderer::GetInstance()->GetInverseBasis3x3();
            cloudPos.y = meters;
            cloudPos = cloudPos * Renderer::GetInstance()->GetBasis3x3();
            cloud->SetWorldPosition(cloudPos);

            cloud->SetNeedsGeocentricPlacement(true);
        }
    }
}

void CloudLayer::SetLayerPosition(double x, double z)
{
    layerX = x;
    layerZ = z;

    Vector3 layerPos(x, GetBaseAltitude(), z);

    SetLocalUpVector(Renderer::GetInstance()->GetUpVector());
    SetLocalBasis(Renderer::GetInstance()->GetBasis3x3());
    SetLocalBasis4(Renderer::GetInstance()->GetBasis4x4());

    SL_VECTOR(Cloud *) ::iterator it;
    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *cloud = *it;
        Vector3 pos = layerPos + cloud->GetLayerPosition();
        pos = pos * Renderer::GetInstance()->GetBasis3x3();
        cloud->SetWorldPosition(pos);
        cloud->SetNeedsGeocentricPlacement(true);
    }
}

void CloudLayer::MoveClouds(double x, double y, double z)
{
    if (!IsRenderable()) return;

    Vector3 v(x,y,z);

    Vector3 localV = v * Renderer::GetInstance()->GetInverseBasis3x3();

    SL_VECTOR(Cloud *) ::iterator it;
    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *c = *it;
        c->SetWorldPosition(c->GetWorldPosition() + v);

        Vector3 layerPos = c->GetLayerPosition() + localV;
        c->SetLayerPosition(layerPos);
    }

    if (!(isInfinite || cloudWrap)) {
        layerX += localV.x;
        layerZ += localV.z;
        baseAltitude += localV.y;
    }
}

double CloudLayer::ComputeWrapFade(Cloud *c, const Vector3& anchor)
{
    if ((isInfinite || cloudWrap) && fadeTowardEdges) {
        double width, depth, height;
        c->GetSize(width, depth, height);

        double halfWidth = baseWidth * 0.5;
        double halfLength = baseLength * 0.5;

        Vector3 cloudPos = c->GetWorldPosition();
        cloudPos = GetLocalBasis() * cloudPos; //cloudPos * Renderer::GetInstance()->GetInverseBasis3x3();

        double minDist = 1.0; // normalized distance from edge

        double dist = ((anchor.x + halfWidth) - (cloudPos.x + width)) / halfWidth;
        if (dist < minDist) minDist = dist;
        dist = ((cloudPos.x - width) - (anchor.x - halfWidth)) / halfWidth;
        if (dist < minDist) minDist = dist;
        dist = ((anchor.z + halfLength) - (cloudPos.z + depth)) / halfLength;
        if (dist < minDist) minDist = dist;
        dist = ((cloudPos.z - depth) - (anchor.z - halfLength)) / halfLength;
        if (dist < minDist) minDist = dist;

        double t = 1.0 - minDist;
        if (t < 0) t = 0;
        if (t > 1.0) t = 1.0;

        double fade = 1.0 - (t * t * t * t * t);

        return fade;
    }

    return 1.0;
}

bool CloudLayer::IntersectEllipsoid(const Vector3& p0, const Vector3& D, Vector3& intersect) const
{
    Vector3 radii = earthRadii;

    if (radii.x == 0 || radii.y == 0 || radii.z == 0) return false;

    Vector3 oneOverRadii(1.0 / radii.x, 1.0 / radii.y, 1.0 / radii.z);
    Vector3 q = p0 * oneOverRadii;
    Vector3 bUnit = D * oneOverRadii;
    bUnit.Normalize();
    double wMagnitudeSquared = q.Dot(q) - 1.0;

    double t = -bUnit.Dot(q);
    double tSquared = t * t;

    if ((t >= 0.0) && (tSquared >= wMagnitudeSquared)) {
        double temp = t - sqrt(tSquared - wMagnitudeSquared);
        Vector3 r = (q + bUnit * temp);
        intersect = r * radii;
        return true;
    } else {
        return false;
    }
}

void CloudLayer::WrapClouds(bool geocentric)
{
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();

    if (isInfinite || cloudWrap) {
        Vector3 anchor;

        if (!isInfinite) {
            anchor = Vector3(layerX, geocentricAltitude, layerZ);
        } else {
            anchor = camPos * Renderer::GetInstance()->GetInverseBasis3x3();
            layerX = anchor.x;
            layerZ = anchor.z;
        }

        double halfWidth = baseWidth * 0.5;
        double halfLength = baseLength * 0.5;

        SL_VECTOR(Cloud *) ::iterator it;
        for (it = clouds.begin(); it != clouds.end(); it++) {
            Cloud *c = *it;
            const Vector3& cloudPos = c->GetWorldPosition();

            Vector3 newCloudPos = GetLocalBasis() * cloudPos;// cloudPos * Renderer::GetInstance()->GetInverseBasis3x3();

            // Iterate until it's in bounds.
            bool adj;
            bool changed = false;

            double width, depth, height;
            c->GetSize(width, depth, height);

            do {
                adj = false;

                if (newCloudPos.x > anchor.x + halfWidth) {
                    newCloudPos.x = (anchor.x - halfWidth) + (newCloudPos.x - (anchor.x + halfWidth - width));
                    adj = changed = true;
                }
                if (newCloudPos.x < anchor.x - halfWidth) {
                    newCloudPos.x = (anchor.x + halfWidth) - ((anchor.x - halfWidth) - newCloudPos.x + width);
                    adj = changed = true;
                }
                if (newCloudPos.z > anchor.z + halfLength) {
                    newCloudPos.z = (anchor.z - halfLength) + (newCloudPos.z - (anchor.z + halfLength - depth));
                    adj = changed = true;
                }
                if (newCloudPos.z < anchor.z - halfLength) {
                    newCloudPos.z = (anchor.z + halfLength) - ((anchor.z - halfLength) - newCloudPos.z + depth);
                    adj = changed = true;
                }
            } while (adj);

            if (changed) {

                Vector3 layerPos = c->GetLayerPosition();
                layerPos.x = newCloudPos.x;
                layerPos.z = newCloudPos.z;
                c->SetLayerPosition(layerPos);

                newCloudPos = newCloudPos * Renderer::GetInstance()->GetBasis3x3();

                c->SetNeedsGeocentricPlacement(true);

                //if (geocentric) {
                //Vector3 cloudDir = newCloudPos;
                //cloudDir.Normalize();
                //newCloudPos = cloudDir * cloudPos.Length();
                //}

                c->SetWorldPosition(newCloudPos);
            }
        }
    }
}

void CloudLayer::AdjustForCurvature(bool geocentric)
{
    bool curve = true;
    Configuration::GetBoolValue("cumulus-round-earth", curve);

    double earthRadius = 6371000;
    Configuration::GetDoubleValue("earth-radius-meters-polar", earthRadius);
    earthRadius *= Atmosphere::GetUnitScale();

    double radius = earthRadius;
    double alpha = 1.0;

    if (GetCurveTowardGround()) {
        double scale = 1.0;
        Configuration::GetDoubleValue("curve-toward-ground-radius-scale", scale);
        double a = sqrt(GetBaseWidth() * GetBaseWidth() + GetBaseLength() * GetBaseLength()) / 2.0;
        double b = GetBaseAltitude();

        if (b > 0) {
            radius = (a*a + b*b) / (2.0 * b);
            radius *= scale;

            if (b < earthRadius) {
                double altitude = Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetLocation().GetAltitude();
                alpha = altitude / b;
                if (alpha > 1.0) alpha = 1.0;
                if (alpha < 0) alpha = 0;

                alpha = 1.0 - alpha;
            }

            curve = true;
        } else {
            curve = false;
        }
    }

    if (curve) {
        Vector3 cloudLayerPos(layerX, GetBaseAltitude(), layerZ);
        Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();

        SL_VECTOR(Cloud *) ::iterator it;
        for (it = clouds.begin(); it != clouds.end(); it++) {
            Cloud *c = *it;
            Vector3 layerPos = c->GetLayerPosition();

            double dist = 0;
            if (isInfinite) {
                dist = (camPos - c->GetWorldPosition()).Length();
            } else {
                Vector3 planar(layerPos.x, 0, layerPos.z);
                dist = planar.Length();
            }

            if (dist < radius) {
                double displacement = radius - (sqrt(radius * radius - dist * dist));

                displacement *= alpha;
                if (!geocentric || GetIsInfinite()) {
                    Vector3 cloudPos = c->GetWorldPosition();
                    cloudPos = cloudPos * Renderer::GetInstance()->GetInverseBasis3x3();
                    cloudPos.y = geocentricAltitude + layerPos.y - displacement;
                    cloudPos = cloudPos * Renderer::GetInstance()->GetBasis3x3();
                    c->SetWorldPosition(cloudPos);
                } else {
                    Vector3 localPos = c->GetLayerPosition();
                    localPos.y += geocentricAltitude - displacement;
                    localPos.x += layerX;
                    localPos.z += layerZ;
                    c->SetWorldPosition(localPos * GetLocalBasis());
                }
            }
        }
    }

}

void CloudLayer::AdjustGeocentricPositions(bool geocentric)
{
    if (!geocentric) {
        geocentricAltitude = GetBaseAltitude();
        return;
    }

    Vector3 bUnit = Renderer::GetInstance()->GetUpVector();
    bUnit.x /= earthRadii.x;
    bUnit.y /= earthRadii.y;
    bUnit.z /= earthRadii.z;
    bUnit.Normalize();
    Vector3 intersection = earthRadii * bUnit;

    //Vector3 intersection;
    //if (IntersectEllipsoid(Vector3(0,0,0), Renderer::GetInstance()->GetUpVector(), intersection))
    {
        double earthRadius = intersection.Length();
        double baseAlt = GetBaseAltitude();
        bool geocentricBaseAltitude = (baseAlt > earthRadii.z || baseAlt > earthRadii.y);
        if (!geocentricBaseAltitude) {
            geocentricAltitude = earthRadius + baseAlt;
        } else {
            geocentricAltitude = baseAlt;
            return; // SetBaseAltitude() already updated the cloud heights.
        }

        SL_VECTOR(Cloud *) ::iterator it;
        for (it = clouds.begin(); it != clouds.end(); it++) {
            Cloud *c = *it;
            if (c->GetNeedsGeocentricPlacement()) {
                double radius = earthRadius;

                double baseAlt = GetBaseAltitude();
                if (geocentricBaseAltitude) {
                    // Old-school geocentric altitude specified
                    radius = baseAlt;
                } else {
                    radius += baseAlt;
                }

                bool ursMethod = false;

                Configuration::GetBoolValue("alternate-geocentric-adjustment", ursMethod);

                if (ursMethod) {
                    Vector3 localPos = c->GetWorldPosition();
                    localPos = localPos * Renderer::GetInstance()->GetInverseBasis3x3();

                    localPos.y = radius;

                    c->SetWorldPosition(localPos * GetLocalBasis());
                    c->SetNeedsGeocentricPlacement(false);
                } else {
                    Vector3 localPos = c->GetLayerPosition();
                    localPos.y += radius;

                    localPos.x += layerX;
                    localPos.z += layerZ;

                    c->SetWorldPosition(localPos * GetLocalBasis());
                    c->SetNeedsGeocentricPlacement(false);
                }
            }
        }
    }
}

void CloudLayer::SetPrecipitation(int type, double intensity, double nearClip, double farClip, bool useDepthBuffer)
{
    if (intensity < 0) intensity = 0;

    if (type < NONE || type >= NUM_PRECIP_TYPES) type = NONE;

    if (type == NONE) {
        precipitationEffects.clear();
        precipitationNearClips.clear();
        precipitationFarClips.clear();
        precipitationUseDepthBuffer.clear();
    } else {
        precipitationEffects[type] = intensity;
        precipitationNearClips[type] = nearClip;
        precipitationFarClips[type] = farClip;
        precipitationUseDepthBuffer[type] = useDepthBuffer;
    }
}

static double clamp(double x, double low, double high)
{
    if (x < low) return low;
    if (x > high) return high;
    return x;
}

static double smoothstep(double edge0, double edge1, double x)
{
    // Scale, bias and saturate x to 0..1 range
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    // Evaluate polynomial
    return x*x*(3 - 2*x);
}

void CloudLayer::ProcessAtmosphericEffects(Sky *s)
{
    if (!IsRenderable()) {
        return;
    }

    // A little hacky, but this one method doesn't really justify the effort of a new class IMO.
    if (GetType() == SANDSTORM) {
        Renderer *ren = Renderer::GetInstance();
        Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos() * ren->GetInverseBasis3x3();

        if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return;
        if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return;
        if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return;
        if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return;

        double cloudBottom = GetBaseAltitudeGeocentric();
        double cloudTop = cloudBottom + GetMaxHeight();

        double r = 0, g = 0, b = 0;
        Color fogColor;
        Configuration::GetDoubleValue("sandstorm-fog-red", r);
        Configuration::GetDoubleValue("sandstorm-fog-blue", b);
        Configuration::GetDoubleValue("sandstorm-fog-green", g);
        double density = 0.001;
        Configuration::GetDoubleValue("sandstorm-fog-density", density);

        double blend = 1.0 - ((cloudTop - camPos.y) / GetMaxHeight());

        if (blend > 1.0) blend = 1.0;
        if (blend < 0) blend = 0;

        Color horizonColor = s->GetAverageHorizonColor(0);
        Color sun = s->GetSunOrMoonColor();
        sun.ScaleToUnitOrLess();
        sun = sun.ToGrayscale();

        bool smoothHorizon = true;
        Configuration::GetBoolValue("sandstorm-smooth-horizon-blend", smoothHorizon);

        if (smoothHorizon) {
            r = (blend * horizonColor.r) + ((1.0 - blend) * r * sun.r);
            g = (blend * horizonColor.g) + ((1.0 - blend) * g * sun.g);
            b = (blend * horizonColor.b) + ((1.0 - blend) * b * sun.b);
        } else {
            //Color the sandstorm fog to match the sandstorm cloud color
            double sr = 0, sg = 0, sb = 0;
            Configuration::GetDoubleValue("sandstorm-dust-red", sr);
            Configuration::GetDoubleValue("sandstorm-dust-blue", sb);
            Configuration::GetDoubleValue("sandstorm-dust-green", sg);
            //Use horizoncolor to darken the fog at night
            horizonColor = horizonColor.ToGrayscale();
            sr *= horizonColor.r;
            sb *= horizonColor.b;
            sg *= horizonColor.g;

            //Allow color of fog to be set externally
            bool lightFog = true;
            Configuration::GetBoolValue("sandstorm-light-fog", lightFog);
            if (lightFog) {
                r = (blend * sr) + ((1.0 - blend) * r * sun.r);
                g = (blend * sg) + ((1.0 - blend) * g * sun.g);
                b = (blend * sb) + ((1.0 - blend) * b * sun.b);
            }
        }

        fogColor = Color(r, g, b, 1.0);

        density *= (1.0 - blend);

        double width = GetBaseWidth();
        double depth = GetBaseLength();
        double f;
        double fade = 1.0;
        double left = layerX - width * 0.5;
        double right = layerX + width * 0.5;
        double bottom = layerZ - depth * 0.5;
        double top = layerZ + depth * 0.5;

        if (camPos.x < left + width * edgeFadeThreshold) {
            f = smoothstep(left, left + width * edgeFadeThreshold, camPos.x);
            if (f < fade) fade = f;
        }

        if (camPos.x > right - width * edgeFadeThreshold) {
            f = 1.0 - smoothstep(right - width * edgeFadeThreshold, right, camPos.x);
            if (f < fade) fade = f;
        }

        if (camPos.z < bottom + depth * edgeFadeThreshold) {
            f = smoothstep(bottom, bottom + depth * edgeFadeThreshold, camPos.z);
            if (f < fade) fade = f;
        }

        if (camPos.z > top - depth * edgeFadeThreshold) {
            f = 1.0 - smoothstep(top - depth * edgeFadeThreshold, top, camPos.z);
            if (f < fade) fade = f;
        }

        density *= fade;

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
}

void CloudLayer::ApplyPrecipitation()
{
    if (IsRenderable()) {
        double edgeFade = GetEdgeFadeFactor();

        PrecipitationManager *pm = PrecipitationManager::GetInstance(Atmosphere::GetCurrentAtmosphere());
        SL_MAP(int, double) ::iterator it;
        for (it = precipitationEffects.begin(); it != precipitationEffects.end(); it++) {
            int type = (*it).first;
            double intensity = (*it).second;
            pm->SetIntensity(type, intensity * edgeFade * fade);

            double nearClip = precipitationNearClips[type];
            double farClip = precipitationFarClips[type];
            bool   useDepthBuffer = precipitationUseDepthBuffer[type];
            pm->SetClipPlanes(type, nearClip, farClip, useDepthBuffer);
        }
    }
}

double CloudLayer::GetEdgeFadeFactor(double cloudScale)
{
    if (edgeFadeThreshold <= 0) return 1.0;

    CloudTypes type = GetType();
    bool isPlanar = type == STRATUS || type == CIRRUS_FIBRATUS || type == CIRROCUMULUS || type == STRATOCUMULUS;

    if (isPlanar && isInfinite) return 1.0;

    Renderer *ren = Renderer::GetInstance();
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos() * ren->GetInverseBasis3x3();

    if (camPos.y > GetBaseAltitudeGeocentric() + GetMaxHeight()) return 0;
    if (camPos.x < (layerX - GetBaseWidth() * 0.5 * cloudScale)) return 0;
    if (camPos.x > (layerX + GetBaseWidth() * 0.5 * cloudScale)) return 0;
    if (camPos.z > (layerZ + GetBaseLength() * 0.5 * cloudScale)) return 0;
    if (camPos.z < (layerZ - GetBaseLength() * 0.5 * cloudScale)) return 0;

    SL_VECTOR(Cloud *) ::const_iterator it;

    double fade = 1.0;

    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *cloud = *it;
        double width, depth, height;
        Vector3 pos = cloud->GetWorldPosition();
        pos = pos * ren->GetInverseBasis3x3();
        cloud->GetSize(width, depth, height);

        width *= cloudScale;
        depth *= cloudScale;
        height *= cloudScale;

        double left = pos.x - width * 0.5;
        double right = pos.x + width * 0.5;
        double bottom = pos.z - depth * 0.5;
        double top = pos.z + depth * 0.5;

        if (camPos.x < left) continue;
        if (camPos.x > right) continue;
        if (camPos.z < bottom) continue;
        if (camPos.z > top) continue;

        double f;

        if (camPos.x < left + width * edgeFadeThreshold) {
            f = smoothstep(left, left + width * edgeFadeThreshold, camPos.x);
            if (f < fade) fade = f;
        }

        if (camPos.x > right - width * edgeFadeThreshold) {
            f = smoothstep(right - width * edgeFadeThreshold, right, camPos.x);
            if (f < fade) fade = f;
        }

        if (camPos.z < bottom + depth * edgeFadeThreshold) {
            f = smoothstep(bottom, bottom + depth * edgeFadeThreshold, camPos.z);
            if (f < fade) fade = f;
        }

        if (camPos.z > top - depth * edgeFadeThreshold) {
            f = smoothstep(top - depth * edgeFadeThreshold, top, camPos.z);
            if (f < fade) fade = f;
        }

        break;
    }

    if (fade < 0) fade = 0;
    if (fade > 1.0) fade = 1.0;

    return fade;

}

bool CloudLayer::HasPrecipitationAtPosition(double x, double y, double z) const
{
    // Bail if we have no precip at all.
    if (precipitationEffects.size() == 0 || !IsRenderable()) return false;

    // Don't assume which way is up.
    Renderer *ren = Renderer::GetInstance();
    Vector3 camPos = Vector3(x, y, z) * ren->GetInverseBasis3x3();

    // Bail if you're not under the entire layer at all.
    if (camPos.y > GetBaseAltitudeGeocentric() + GetThickness()) return false;
    if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return false;
    if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return false;
    if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return false;
    if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return false;

    // Now check individual clouds.
    SL_VECTOR(Cloud *) ::const_iterator it;
    bool hit = false;
    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *cloud = *it;
        double width, depth, height;
        Vector3 pos = cloud->GetWorldPosition();
        pos = pos * ren->GetInverseBasis3x3();
        cloud->GetSize(width, depth, height);

        if (camPos.x < (pos.x - width * 0.5)) continue;
        if (camPos.x > (pos.x + width * 0.5)) continue;
        if (camPos.z < (pos.z - depth * 0.5)) continue;
        if (camPos.z > (pos.z + depth * 0.5)) continue;

        hit = true;
        break;
    }

    return hit;
}

bool CloudLayer::IsInsideCloud(double x, double y, double z) const
{
    if (!renderingEnabled) return false;

    // Don't assume which way is up.
    Renderer *ren = Renderer::GetInstance();
    Vector3 camPos = Vector3(x, y, z) * ren->GetInverseBasis3x3();

    // Bail if you're not in the entire layer at all.
    if (camPos.y < GetBaseAltitudeGeocentric()) return false;
    if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return false;
    if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return false;
    if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return false;
    if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return false;

    // Now check individual clouds.
    SL_VECTOR(Cloud *) ::const_iterator it;
    bool hit = false;
    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *cloud = *it;
        double width, depth, height;
        Vector3 pos = cloud->GetWorldPosition();
        pos = pos * ren->GetInverseBasis3x3();
        cloud->GetSize(width, depth, height);

        if (camPos.x < (pos.x - width * 0.5)) continue;
        if (camPos.x > (pos.x + width * 0.5)) continue;
        if (camPos.z < (pos.z - depth * 0.5)) continue;
        if (camPos.z > (pos.z + depth * 0.5)) continue;
        if (camPos.y < (pos.y - height * 0.5)) continue;
        if (camPos.y > (pos.y + height * 0.5)) continue;

        hit = true;
        break;
    }

    return hit;
}

void CloudLayer::ComputeBounds()
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

    minimum.x = x - GetBaseWidth() * 0.5;
    minimum.y = GetBaseAltitudeGeocentric();
    minimum.z = z - GetBaseLength() * 0.5;
    maximum.x = x + GetBaseWidth() * 0.5;
    maximum.y = GetBaseAltitudeGeocentric() + GetMaxHeight();
    maximum.z = z + GetBaseLength() * 0.5;

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

bool CloudLayer::Cull(const Frustum& f)
{
    if (GetIsInfinite()) return false;

    if (!renderingEnabled) return true;

    for (int i = 0; i < f.GetNumCullingPlanes(); i++) {
        const Plane& p = f.GetPlane(i);
        double dist = p.GetDistance();
        const Vector3& N = p.GetNormal();

        if (N.Dot(bounds[0]) + dist > 0) continue;
        if (N.Dot(bounds[1]) + dist > 0) continue;
        if (N.Dot(bounds[2]) + dist > 0) continue;
        if (N.Dot(bounds[3]) + dist > 0) continue;
        if (N.Dot(bounds[4]) + dist > 0) continue;
        if (N.Dot(bounds[5]) + dist > 0) continue;
        if (N.Dot(bounds[6]) + dist > 0) continue;
        if (N.Dot(bounds[7]) + dist > 0) continue;
        return true;
    }
    return false;
}

void CloudLayer::ClearCulledChildren()
{
    ClearCulled();

    SL_VECTOR(Cloud*) ::iterator it;
    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *cloud = *it;
        cloud->ClearCulled();
    }
}

void CloudLayer::DoCulling(const Frustum *f)
{
    ComputeBounds();

    if (renderingEnabled == false || (enableCulling && Cull(*f))) {
        SetCulled();
    } else {
        ClearCulled();
        SL_VECTOR(Cloud*) ::iterator it;
        for (it = clouds.begin(); it != clouds.end(); it++) {
            Cloud *cloud = *it;

            bool cull = cloud->Cull(*f);
            if (cull) {
                cloud->SetCulled();
            } else {
                cloud->ClearCulled();
            }
        }
    }
}

void CloudLayer::DoUpdates(const Sky * sky, unsigned long now)
{
    if (!renderingEnabled) return;

    AdjustGeocentricPositions(sky->GetGeocentricMode());

    WrapClouds(sky->GetGeocentricMode());

    AdjustForCurvature(sky->GetGeocentricMode());

    // Check for runtime density adjustment
    if (usingRuntimeDensity) {
        int nClouds = (int)(clouds.size());
        for (int cloudNum = 0; cloudNum < nClouds; cloudNum++) {
            Cloud *c = clouds[cloudNum];
            double val = c->GetCoverageThreshold();
            if (coverageMultiplier < val) {
                c->FadeOut();
            } else {
                c->FadeIn();
            }
        }
    }

    if (growth) {
        int automataUpdates = 0;
        int automataUpdateBudget = 10;
        Configuration::GetIntValue("max-cellular-automata-updates-per-frame", automataUpdateBudget);
        int nClouds = (int)(clouds.size());

        if (lastCloudUpdated >= nClouds) {
            lastCloudUpdated = 0;
        }

        int cloudsUpdated = 0;
        while (cloudsUpdated < nClouds) {
            Cloud *cloud = clouds[lastCloudUpdated++];
            if (cloud->Update(now, false)) {
                automataUpdates++;
            }
            if (automataUpdates >= automataUpdateBudget) {
                break;
            }
            if (lastCloudUpdated >= nClouds) {
                lastCloudUpdated = 0;
            }
            cloudsUpdated++;
        }
    }
}

bool CloudLayer::Draw(int pass, const Vector3 *lightDir,
                      const Color *lightColor, bool invalid, bool wantsUpdate,
                      unsigned long now, const Sky* sky, CloudBackdropManager *backdropManager)
{
    if (GetCulled()) {
        if (pass == 1) ClearCulled();
        return false;
    }

    if (!renderingEnabled) return true;

    Color finalLightColor = *lightColor;

    if (hasForcedCloudColor) {

        Color originalLight = Color(lightColor->r, lightColor->g, lightColor->b);
        Color forcedColor = Color(forcedCloudColor.x, forcedCloudColor.y, forcedCloudColor.z);
        if (forcedCloudColorLighting) {
            finalLightColor = originalLight * forcedColor;
        } else {
            finalLightColor = forcedColor;
        }
    }

    if (forceLightingUpdate) {
        wantsUpdate = true;
        forceLightingUpdate = false;
    }

    assert(lightDir);
    Metaball::SetLightPosition(*lightDir);

    Vector3 topCenter = Vector3(layerX, geocentricAltitude + thickness, layerZ) * Renderer::GetInstance()->GetBasis3x3();
    double deckRadius = sqrt(GetBaseWidth() * GetBaseWidth() + GetBaseLength() * GetBaseLength());
    Vector3 lightPos = (*lightDir) * deckRadius;
    lightPos = lightPos + topCenter;

    if ( drawList.size() == 0 )
        drawList.reserve( 1024 );
    drawList.clear();

    bool needsFirstPass = invalid;

    SL_VECTOR(Cloud*) ::iterator it;
    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *cloud = *it;

        cloud->Visit(pass);

        if (pass == 0 || !cloud->GetCulled()) {
            drawList.push_back(cloud);
            if (invalid) {
                cloud->Invalidate();
                cloud->SetNeedsRelighting(true);
            } else {
                if (wantsUpdate) {
                    if (cloud->LightingChanged(*lightDir) || (cloud->IsInvalid() && !cloud->GetCulled())) {
                        cloud->SetNeedsRelighting(true);
                        needsFirstPass = true;
                    }
                }
            }
        }

        if (pass == 1) cloud->ClearCulled();
    }

    if (pass == 0 && !needsFirstPass) {
        return false;
    }

    static int cloudLightingBudget = 1000;
    static bool first = true;
    if ( first ) {
        first = false;
        Configuration::GetIntValue("cloud-relight-budget", cloudLightingBudget);
    }

    int cloudsDrawn = 0;

    if (DrawSetup(pass, &lightPos, &finalLightColor)) {
        Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();

        Vector3 anchor;

        if (!isInfinite) {
            anchor = Vector3(layerX, 0, layerZ);
        } else {
            anchor = camPos * Renderer::GetInstance()->GetInverseBasis3x3();
        }

        int skipped = 0;
        SL_VECTOR(Cloud*) ::iterator it;
        for (it = drawList.begin(); it != drawList.end(); it++) {
            Cloud *cloud = *it;
            double wrapFade = ComputeWrapFade(cloud, anchor);
            cloud->SetFade(fade);
            cloud->SetAlpha(alpha * wrapFade);
            if ( (pass > 0) || (cloudsDrawn < cloudLightingBudget) || !firstFrameDrawn ) {
                if (pass == 0 && !cloud->GetNeedsRelighting()) continue;

                if (pass == 1 && backdropManager) {
                    if (backdropManager->IsCloudInBackdrop(cloud, camPos)) {
                        skipped++;
                        continue;
                    }
                }

                if (cloud->Draw(pass, lightPos, *lightDir, finalLightColor, invalid, sky)) {
                    if (pass == 0) {
                        cloud->UpdateLightPos(*lightDir);
                        cloud->SetNeedsRelighting(false);
                    }
                    cloudsDrawn++;
                }
            }
        }

        EndDraw(pass);
    }

    firstFrameDrawn = true;
    return true;
}

void CloudLayer::GetSortPosition(double& x, double& y, double& z) const
{
    Vector3 camPosYUp = Atmosphere::GetCurrentAtmosphere()->GetCamPos() *
                        Renderer::GetInstance()->GetInverseBasis3x3();

    Vector3 pos = camPosYUp;

    double bottom = geocentricAltitude;
    double top = geocentricAltitude + GetMaxHeight();

    double dBottom = fabs(camPosYUp.y - bottom);
    double dTop = fabs(camPosYUp.y - top);

    if (dBottom < dTop) {
        pos.y = bottom;
    } else {
        pos.y = top;
    }

    pos = pos * Renderer::GetInstance()->GetBasis3x3();

    x = pos.x;
    y = pos.y;
    z = pos.z;

    /*
    if (isInfinite)
    {
        Vector3 pos = Atmosphere::GetCurrentAtmosphere()->GetCamPos() *
            Renderer::GetInstance()->GetInverseBasis3x3();

        pos.y = baseAltitude + GetMaxHeight() * 0.5;

        pos = pos * Renderer::GetInstance()->GetBasis3x3();

        x = pos.x;
        y = pos.y;
        z = pos.z;
    }
    else
    {
        Vector3 pos(layerX, baseAltitude + GetThickness() * 0.5, layerZ);
        pos = pos * Renderer::GetInstance()->GetBasis3x3();
        x = pos.x;
        y = pos.y;
        z = pos.z;
    }
    */
}

void CloudLayer::AddLightningListener(LightningListener *listener)
{
    if (listener)
        lightningListeners.push_back(listener);
}

void CloudLayer::ClearLightningListeners()
{
    lightningListeners.clear();
}

void CloudLayer::LightningNotify(const Vector3& pos)
{
    SL_VECTOR(LightningListener*) ::iterator it;
    for (it = lightningListeners.begin(); it != lightningListeners.end(); it++) {
        (*it)->LightningStartedEvent(pos.x, pos.y, pos.z);
    }
}

bool CloudLayer::Save(const char *filePath)
{
    std::fstream f(filePath, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    if (f.fail()) return false;
    return Serialize(f);
}

bool CloudLayer::Restore(const Atmosphere& atm, const char *filePath)
{
    std::fstream f(filePath, std::fstream::in | std::fstream::binary);
    if (f.fail()) return false;
    return Unserialize(atm, f);
}

bool CloudLayer::Serialize(std::ostream& s)
{
    int version = CLOUDLAYER_SERIALIZE_VERSION;
    s.write((char *)&version, sizeof(int));

    s.write((char *)&layerX, sizeof(double));
    s.write((char *)&layerZ, sizeof(double));
    s.write((char *)&baseWidth, sizeof(double));
    s.write((char *)&baseLength, sizeof(double));
    s.write((char *)&baseAltitude, sizeof(double));
    s.write((char *)&thickness, sizeof(double));
    s.write((char *)&density, sizeof(double));
    s.write((char *)&layerEnabled, sizeof(bool));
    s.write((char *)&renderingEnabled, sizeof(bool));
    s.write((char *)&isInfinite, sizeof(bool));

    s.write((char *)&localWindX, sizeof(double));
    s.write((char *)&localWindZ, sizeof(double));

    s.write((char *)&cloudWrap, sizeof(bool));
    s.write((char *)&fadeTowardEdges, sizeof(bool));

    s.write((char *)&coverageMultiplier, sizeof(double));

    int nPrecip = (int)precipitationEffects.size();
    s.write((char *)&nPrecip, sizeof(int));
    SL_MAP(int, double) ::const_iterator it;
    for (it = precipitationEffects.begin(); it != precipitationEffects.end(); it++) {
        int type = it->first;
        double intensity = it->second;
        s.write((char *)&type, sizeof(int));
        s.write((char *)&intensity, sizeof(double));
    }

    s.write((char *)&curveTowardGround, sizeof(bool));

    this->SaveClouds(s);

    return true;
}

bool CloudLayer::SaveClouds(std::ostream& s) const
{
    int numClouds = (int)clouds.size();
    s.write((char *)&numClouds, sizeof(int));
    for (int i = 0; i < numClouds; i++) {
        clouds[i]->Serialize(s);
    }

    return true;
}

bool CloudLayer::Unserialize(const Atmosphere& atm, std::istream& s)
{
    int version;
    s.read((char *)&version, sizeof(int));
    if (version != CLOUDLAYER_SERIALIZE_VERSION) return false;

    s.read((char *)&layerX, sizeof(double));
    s.read((char *)&layerZ, sizeof(double));
    s.read((char *)&baseWidth, sizeof(double));
    s.read((char *)&baseLength, sizeof(double));
    s.read((char *)&baseAltitude, sizeof(double));
    s.read((char *)&thickness, sizeof(double));
    s.read((char *)&density, sizeof(double));
    s.read((char *)&layerEnabled, sizeof(bool));
    s.read((char *)&renderingEnabled, sizeof(bool));
    s.read((char *)&isInfinite, sizeof(bool));

    s.read((char *)&localWindX, sizeof(double));
    s.read((char *)&localWindZ, sizeof(double));

    s.read((char *)&cloudWrap, sizeof(bool));
    s.read((char *)&fadeTowardEdges, sizeof(bool));

    s.read((char *)&coverageMultiplier, sizeof(double));

    precipitationEffects.clear();
    int nPrecip = 0;
    s.read((char *)&nPrecip, sizeof(int));
    for (int i = 0; i < nPrecip; i++) {
        int precipType;
        double intensity;
        s.read((char *)&precipType, sizeof(int));
        s.read((char *)&intensity, sizeof(double));
        precipitationEffects[precipType] = intensity;
    }

    s.read((char *)&curveTowardGround, sizeof(bool));

    SetLocalUpVector(Renderer::GetInstance()->GetUpVector());
    SetLocalBasis(Renderer::GetInstance()->GetBasis3x3());
    SetLocalBasis4(Renderer::GetInstance()->GetBasis4x4());

    this->RestoreClouds(atm, s);

    return true;
}

bool CloudLayer::ExportToVRML(const char *dirPath)
{
    if (!dirPath) return false;

    int numClouds = (int)clouds.size();

    char buf[1024];
    for (int i = 0; i < numClouds; i++) {
#if (defined(WIN32) || defined(WIN64)) && (_MSC_VER > 1310)
        sprintf_s(buf, 1024, "%s/%03d.wrl", dirPath, i);
#else
        sprintf(buf, "%s/%03d.wrl", dirPath, i);
#endif
        clouds[i]->ExportToVRML(buf);
    }

    return true;
}

double CloudLayer::ComputeSkyCoverage() const
{
    double coverage = 0;

    if (!GetCulled()) {
        Vector3 camPosYUp = Atmosphere::GetCurrentAtmosphere()->GetCamPos() *
                            Renderer::GetInstance()->GetInverseBasis3x3();

        Vector3 pos = camPosYUp;

        double bottom = geocentricAltitude;
        double top = geocentricAltitude + GetMaxHeight();

        if (camPosYUp.y < bottom) {
            coverage = density;
        } else if (camPosYUp.y < top) {
            double factor = (top - camPosYUp.y) / (top - bottom);
            coverage = density * factor;
        }
    }

    return coverage;
}

bool CloudLayer::Intersect(const Vector3& Origin, const Vector3& Direction, double& range) const
{
    Renderer* renderer = Renderer::GetInstance();

    Vector3 camOrigin = Origin * renderer->GetInverseBasis3x3();
    Vector3 camDirection = Direction * renderer->GetInverseBasis3x3();

    SL_VECTOR(double) hit_ranges;
    SL_VECTOR(Cloud*) ::const_iterator it;
    for (it = clouds.begin(); it != clouds.end(); it++) {
        Cloud *cloud = *it;

        //
        // The intersection algorithm employed in the following section
        // is based on the slab method.  The AABB for each cloud is computed
        // using the cloud's position and size.  The candidate ray is
        // then tested for intersection between each pair of parallel
        // planes making up the AABB.  If any portion of the ray intersects
        // any slab then the test passes; otherwise, it fails.
        //
        // The 3D cloud layer used here is assumed to be infinite in SilverLining
        // terms and thus follows the camera.  It has no DCS per se and similarly
        // each cloud comprising this layer does not have a DCS.  However, because
        // SilverLining will place its clouds according to the curvature of the
        // earth, there may be some error introduced at the edge of large 3D
        // layers due to the use of AABB's.
        //

        // Get the size of the cloud
        Vector3 cloudSize;
        cloud->GetSize(cloudSize.x, cloudSize.z, cloudSize.y);

        // Compute corners of the cloud box
        Vector3 cloudPos = cloud->GetWorldPosition();
        cloudPos = cloudPos * renderer->GetInverseBasis3x3();

        Vector3 cloudMinPos = cloudPos - ( cloudSize * 0.5 );
        Vector3 cloudMaxPos = cloudPos + ( cloudSize * 0.5 );

        // Calculate intersection parameters for each slab in 3 dimensions
        // If the ray does not intersect any slab then the signs of t_min & t_max
        // will flip causing the test to fail.

        double t_min = -DBL_MAX;
        double t_max = DBL_MAX;

        // Test the slab perpendicular to x axis
        if( camDirection.x != 0.0 ) {
            double x_inv = 1.0 / camDirection.x;

            double tx_1 = ( cloudMinPos.x - camOrigin.x ) * x_inv;
            double tx_2 = ( cloudMaxPos.x - camOrigin.x ) * x_inv;

            t_min = SLMIN( tx_1, tx_2 );
            t_max = SLMAX( tx_1, tx_2 );
        }

        // Test the slab perpendicular to y axis
        if( camDirection.y != 0.0 ) {
            double y_inv = 1.0 / camDirection.y;

            double ty_1 = ( cloudMinPos.y - camOrigin.y ) * y_inv;
            double ty_2 = ( cloudMaxPos.y - camOrigin.y ) * y_inv;

            t_min = SLMAX( t_min, SLMIN( ty_1, ty_2 ) );
            t_max = SLMIN( t_max, SLMAX( ty_1, ty_2 ) );
        }

        // Test the slab perpendicular to z axis
        if( camDirection.z != 0.0 ) {
            double z_inv = 1.0 / camDirection.z;

            double tz_1 = ( cloudMinPos.z - camOrigin.z ) * z_inv;
            double tz_2 = ( cloudMaxPos.z - camOrigin.z ) * z_inv;

            t_min = SLMAX( t_min, SLMIN( tz_1, tz_2 ) );
            t_max = SLMIN( t_max, SLMAX( tz_1, tz_2 ) );
        }

        // If coarse test passes move on to fine-grained test
        if( t_max > t_min ) {
            cloud->Intersect( Origin, Direction, hit_ranges );
        }
    }

    // Sort results and find closest intersection
    double rMin = DBL_MAX;
    SL_VECTOR(double)::const_iterator i;
    for( i = hit_ranges.begin(); i != hit_ranges.end(); ++i ) {
        double r = *i;

        // Ignore hits which are behind the ray origin
        if( r < rMin && r > 0.0 )
            rMin = r;
    }

    if( hit_ranges.size() > 0 && rMin < DBL_MAX ) {
        range = rMin;
        return true;
    }

    return false;
}
