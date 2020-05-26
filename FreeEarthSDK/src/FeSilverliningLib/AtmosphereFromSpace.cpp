// Copyright (c) 2007-2009 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/AtmosphereFromSpace.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Atmosphere.h>

using namespace SilverLining;
using namespace std;

AtmosphereFromSpace::AtmosphereFromSpace()
{
    numSegments = 20;
    lateralSegments = 10;
    verticalSegments = 10;
    earthRadius = 6371000;
    atmosphereThickness = 8435.0;
    atmosphereHeight = 1000000.0;
    atmosphereBrightness = 3.0;
    enableShader = true;
    atmoShader = 0;
    atmosphereFadeDistance = 1000000;
    doShadow = true;

    Configuration::GetIntValue("num-atmosphere-segments", numSegments);
    Configuration::GetIntValue("atmosphere-segment-lateral-resolution", lateralSegments);
    Configuration::GetIntValue("atmosphere-segment-vertical-resolution", verticalSegments);
    Configuration::GetFloatValue("earth-radius-meters-polar", earthRadius);
    earthRadius *= (float)Atmosphere::GetUnitScale();
    Configuration::GetFloatValue("atmosphere-scale-height-meters", atmosphereThickness);
    atmosphereThickness *= (float)Atmosphere::GetUnitScale();
    Configuration::GetFloatValue("atmosphere-height", atmosphereHeight);
    atmosphereHeight *= (float)Atmosphere::GetUnitScale();
    Configuration::GetFloatValue("atmosphere-brightness", atmosphereBrightness);
    Configuration::GetBoolValue("atmosphere-enable-shader", enableShader);
    Configuration::GetFloatValue("atmosphere-fade-distance", atmosphereFadeDistance);
    atmosphereFadeDistance *= (float)Atmosphere::GetUnitScale();
    Configuration::GetBoolValue("atmosphere-shadow-from-earth", doShadow);

    if (enableShader) {

        char buf[1024];
#if (defined(WIN32) || defined(WIN64)) && (_MSC_VER > 1310)
        if (doShadow) {
            sprintf_s(buf, 1024, "Shaders/Atmosphere.cg");
        } else {
            sprintf_s(buf, 1024, "Shaders/Atmosphere-noshadow.cg");
        }
#else
        if (doShadow) {
            sprintf(buf, "Shaders/Atmosphere.cg");
        } else {
            sprintf(buf, "Shaders/Atmosphere-noshadow.cg");
        }
#endif
        atmoShader = Renderer::GetInstance()->LoadShaderFromFile(buf, VERTEX_PROGRAM);
    }
}

AtmosphereFromSpace::~AtmosphereFromSpace()
{
    SL_VECTOR(AtmosphereSegment *) ::iterator it;
    for (it = segments.begin(); it != segments.end(); it++) {
        SL_DELETE (*it);
    }

    segments.clear();
}

void AtmosphereFromSpace::BuildGeometry()
{
    float segmentWidthRadians = (float)(TWOPI / numSegments);

    for (int i = 0; i < numSegments; i++) {
        AtmosphereSegment *s = SL_NEW AtmosphereSegment();
        s->BuildGeometry(i * segmentWidthRadians, (i + 1) * segmentWidthRadians,
                         earthRadius, atmosphereThickness, atmosphereHeight, lateralSegments,
                         verticalSegments, atmosphereBrightness);
        segments.push_back(s);
    }
}

bool AtmosphereFromSpace::Draw(const Frustum& f, const Vector3& sunPositionGeocentric,
                               const Vector3& earthCenter, double altitude)
{
    bool doAtmosphereFromSpace = true;
    Configuration::GetBoolValue("enable-atmosphere-from-space", doAtmosphereFromSpace);

    if (!doAtmosphereFromSpace)
        return true;

    float alpha = 1.0; // blend away the atmosphere from space into the skybox as you get lower

    if (altitude < atmosphereThickness) return true;

    double fadeStartDist = atmosphereThickness * 2.0;

    if (altitude < fadeStartDist) {
        alpha = (float)(1.0 - ((fadeStartDist - altitude) / (fadeStartDist - atmosphereThickness)));
    }

    Renderer *ren = Renderer::GetInstance();

    ren->PushAllState();

    ren->PushModelviewTransform();

    // Compute the sun direction in camera coordinates
    Vector3 sunPosNormalized = sunPositionGeocentric;
    sunPosNormalized.Normalize();

    Vector3 look, trans;
    double x, y, z, r;

    r = earthRadius;

    // Compute the tangent point from the camera to the sphere of the Earth
    look = Atmosphere::GetCurrentAtmosphere()->GetCamPos() * -1;
    z = look.Length();
    y = (r * r) / z;
    x = sqrt(r * r - y * y);
    look.Normalize();
    trans = (look * -1) * y;

    // Construct a rotation matrix to face toward the camera, and burn in the offset
    // to the tangent point as well
    Vector3 up(0, 0, 1);
    Vector3 right = up.Cross(look);
    right.Normalize();
    up = look.Cross(right);
    up.Normalize();

    Matrix4 m;
    m.elem[0][0] = right.x;
    m.elem[0][1] = up.x;
    m.elem[0][2] = look.x;
    m.elem[0][3] = trans.x;
    m.elem[1][0] = right.y;
    m.elem[1][1] = up.y;
    m.elem[1][2] = look.y;
    m.elem[1][3] = trans.y;
    m.elem[2][0] = right.z;
    m.elem[2][1] = up.z;
    m.elem[2][2] = look.z;
    m.elem[2][3] = trans.z;
    m.elem[3][0] = 0.0;
    m.elem[3][1] = 0.0;
    m.elem[3][2] = 0.0;
    m.elem[3][3] = 1.0;

    // Construct a purely rotational matrix for shadow tests
    Matrix4 world = m;
    world.elem[0][3] = world.elem[1][3] = world.elem[2][3] = 0;
    world.Transpose();

    // Apply our billboard matrix
    ren->MultiplyModelviewMatrix(m);

    // Compute blend

    if (atmoShader) {
        ren->BindShader(atmoShader, VERTEX_PROGRAM);

        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        ren->SetConstantVector(atmoShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        ren->SetConstantVector(atmoShader, "sl_sunPosition", sunPosNormalized);

        Matrix4 modelview, proj, modelviewProj;
        ren->GetModelviewMatrix(&modelview);
        ren->GetProjectionMatrix(&proj);
        modelviewProj = proj * modelview;

        ren->SetConstantMatrix(atmoShader, "sl_world", world);
        ren->SetConstantMatrix(atmoShader, "sl_modelViewProj", modelviewProj);
        ren->SetConstantVector(atmoShader, "sl_fadeDistVec", Vector3(atmosphereFadeDistance / earthRadius,
                               alpha, x));
    }

    ren->EnableTexture2D(false);
    ren->EnableLighting(false);
    ren->EnableBackfaceCulling(false);
    ren->EnableDepthReads(true);
    ren->EnableDepthWrites(false);
    ren->EnableBlending(ONE, ONE);

    SL_VECTOR(AtmosphereSegment *) ::iterator it;

    for (it = segments.begin(); it != segments.end(); it++) {
        if (!atmoShader) {
            (*it)->ComputeVertices(sunPosNormalized, world, atmosphereFadeDistance / earthRadius, alpha, (float)x);
        }
        (*it)->Draw(1);
    }

    ren->UnbindShader(VERTEX_PROGRAM);

    //ren->EnableDepthWrites(true);

    ren->PopAllState();

    ren->PopModelviewTransform();

    return true;
}
