// Copyright (c) 2005-2009 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CloudImposter.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Configuration.h>

using namespace SilverLining;

int CloudImposter::maxUpdatesPerFrame = 1;
int CloudImposter::updatesThisFrame = 0;

CloudImposter::CloudImposter(Cloud *pCloud)
{
    cloud = pCloud;

    double width, depth, height;
    pCloud->GetSize(width, depth, height);

    double fudge = 1.0;
    Configuration::GetDoubleValue("imposter-slop-value", fudge);

    Configuration::GetIntValue("max-imposter-updates-per-frame", maxUpdatesPerFrame);

    width *= 0.5;
    depth *= 0.5;
    height *= 0.5;

    imposterRadius = sqrt(width * width + depth * depth + height * height) * fudge;

    imposterDim = 256;
    Configuration::GetIntValue("imposter-texture-dimension", imposterDim);

    invalid = true;

    renderTexture = 0;
    billboard = 0;

    Configuration::GetBoolValue("disable-imposters", disabled);

    Renderer *ren = Renderer::GetInstance();
    if (ren && !disabled) {
        if (ren->InitRenderTexture(imposterDim, imposterDim, &renderTexture)) {
            // th changed (avoid duplicate opengl calls)
            billboard = SL_NEW Billboard();
            billboard->Initialize(false, float(imposterRadius * 2.0));
            billboard->SetWorldPosition(cloud->GetWorldPosition());
            //billboard->SetWorldSize(imposterRadius * 2.0);
        }
    }
}

CloudImposter::~CloudImposter()
{
    Renderer *ren = Renderer::GetInstance();
    if (ren && renderTexture) {
        ren->ReleaseRenderTexture(renderTexture);
    }

    if (billboard) {
        SL_DELETE billboard;
    }
}

void CloudImposter::SetContext(bool *useImposter, bool *wantsUpdate)
{
    contextSet = false;

    Renderer *ren = Renderer::GetInstance();
    if (!ren || !renderTexture || disabled) {
        *useImposter = false;
        *wantsUpdate = invalid;
        return;
    }

    // Punt if the imposter resolution is too low.
    Matrix4 modelview;
    ren->GetModelviewMatrix(&modelview);

    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    Vector3 camUp(modelview.elem[0][1], modelview.elem[1][1], modelview.elem[2][1]);

    Vector3 cloudPos = cloud->GetWorldPosition();

    double dist = (cloudPos - camPos).Length();
    double fov;
    ren->GetFOV(fov);
    int x, y, w, h;
    ren->GetViewport(x, y, w, h);
    double screenWidth = w;

    double pixelAngle = fov / screenWidth;
    double imposterAngle = (2.0 * atan2(imposterRadius, dist));
    double texelAngle = imposterAngle / imposterDim;

    if (texelAngle > pixelAngle) {
        *useImposter = false;
        *wantsUpdate = invalid;
        return;
    }

    // Punt if your angle hasn't changed much.
    Vector3 v1 = lastFarPt - camPos;
    Vector3 v2 = lastNearPt - camPos;
    v1.Normalize();
    v2.Normalize();

    double errorAngle = 0;
    errorAngle = myAcos(v1.Dot(v2));

    if (((errorAngle < pixelAngle) && !invalid) ||
            (updatesThisFrame >= maxUpdatesPerFrame)) {
        *useImposter = true;
        *wantsUpdate = false;
        return;
    }

    // Go ahead with the imposter.
    updatesThisFrame++;

    contextSet = true;
    Vector3 zAxis;
    if (ren->GetIsRightHanded())
        zAxis = cloudPos - camPos;
    else
        zAxis = camPos - cloudPos;

    zAxis.Normalize();
    lastFarPt = cloudPos + zAxis * imposterRadius;
    lastNearPt = cloudPos - zAxis * imposterRadius;
    *useImposter = true;
    *wantsUpdate = true;
    invalid = false;

    ren->PushTransforms();

    Frustum f = Atmosphere::GetCurrentAtmosphere()->GetWorldFrustum();

    if (!ren->MakeRenderTextureCurrent(renderTexture, true)) {
        *useImposter = false;
        return;
    }

    Vector3 Up = camUp; //Vector3(0, 0, 1);
    ren->SetModelviewLookat(camPos, cloudPos, Up);

    ren->GetModelviewMatrix(&modelview);

    // hack the billboard matrix
    Matrix4 billboard;

    Vector3 Normal(modelview.elem[2][0], modelview.elem[2][1], modelview.elem[2][2]);
    Vector3 Right = Up.Cross(Normal);
    Up = Normal.Cross(Right);

    if (ren->GetIsRightHanded()) {
        billboard = Matrix4(Right.x, Up.x, Normal.x, 0,
                            Right.y, Up.y, Normal.y, 0,
                            Right.z, Up.z, Normal.z, 0,
                            0,   0,    0,      1);
    } else {
        billboard = Matrix4(Right.x, Up.x, -Normal.x, 0,
                            Right.y, Up.y, -Normal.y, 0,
                            Right.z, Up.z, -Normal.z, 0,
                            0,   0,    0,       1);
    }

    Billboard::OverrideBillboardMatrix(billboard);

    // Set FOV to encompass the imposter
    Plane pfar = f.GetPlane(Frustum::PBACK);
    ren->SetProjectionMatrix(10, pfar.GetDistance(), imposterAngle, imposterAngle);
}

bool CloudImposter::RestoreContext()
{
    Renderer *ren = Renderer::GetInstance();

    if (!ren || !renderTexture || !contextSet) {
        return false;
    }

    Billboard::RestoreBillboardMatrix();

    ren->PopTransforms();

    contextSet = false;

    return ren->BindRenderTexture(renderTexture);
}

bool CloudImposter::Render(const Vector3& worldPos, double fade)
{
    bool ok = false;

    Renderer *ren = Renderer::GetInstance();

    ren->EnableBlending(ONE, INVSRCALPHA);

    // For debugging; show the black background of the imposter so they can
    // be easily recognized.
    // ren->DisableBlending();

    if (worldPos != lastWorldPos) {
        billboard->SetWorldPosition(worldPos);
        lastWorldPos = worldPos;
    }


    // Don't double-fog the billboard
    double originalFog = Billboard::GetFogDensity();
    Billboard::SetFogDensity(1E-9);

    TextureHandle tex;
    ren->GetRenderTextureTextureHandle(renderTexture, &tex);
    ok = billboard->Draw(tex, 0, fade, false);

    Billboard::SetFogDensity(originalFog);

    return ok;
}
