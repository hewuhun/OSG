// Copyright (c) 2013-2015 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/ShadowMap.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Configuration.h>

using namespace std;
using namespace SilverLining;

ShadowMap::ShadowMap(Atmosphere *atm, int texDim) : atmosphere(atm), texSize(texDim), cleared(false), hasShadows(false), isRendering(false), shadowBlend(1.0f)
{
    texture = 0;
    Renderer::GetInstance()->InitRenderTexture(texDim, texDim, &texture);
}

ShadowMap::~ShadowMap()
{
    if (texture) {
        Renderer::GetInstance()->ReleaseRenderTexture(texture);
    }
}

bool ShadowMap::SetupShadowMap(const Vector3& lightDirection, const SL_VECTOR(CloudLayer *)& cloudLayers, bool wholeLayers, double maxShadowDistance)
{
    FindFrustumWorldSpace(cloudLayers, wholeLayers, maxShadowDistance);
    ComputeLightMatrix(lightDirection, wholeLayers);
    cleared = false;

    return false;
}

void ShadowMap::SetLightMatrices()
{
    Renderer *ren = Renderer::GetInstance();
    ren->SetModelviewMatrix(lightModelView);
    ren->SetProjectionMatrix(lightProjection);
}

bool ShadowMap::StartDrawing(const Color& shadowColor, bool moonShadows)
{
    Color fadedColor = shadowColor;

    if (atmosphere) {
        float sunx, suny, sunz;
        if (moonShadows) {
            atmosphere->GetSunOrMoonPosition(&sunx, &suny, &sunz);
        } else {
            atmosphere->GetSunPosition(&sunx, &suny, &sunz);
        }

        Vector3 sunPos(sunx, suny, sunz);
        sunPos = sunPos * Renderer::GetInstance()->GetInverseBasis3x3();
        shadowBlend = 1.0f;
        if (sunPos.y < 0.1f) {
            shadowBlend = ((float)(sunPos.y) / 0.1f);
            if (shadowBlend < 0.0f) shadowBlend = 0.0f;
        }

        Color white(1,1,1,1);

        fadedColor = shadowColor * shadowBlend + white * (1.0f - shadowBlend);
        fadedColor.a = 1.0f;
    }

    Billboard::ForceConstantColor(true, fadedColor);

    if (texture) {
        Renderer *ren = Renderer::GetInstance();

        ren->GetViewport(savedX, savedY, savedW, savedH);
        ren->SetViewport(0, 0, texSize, texSize);

        savedCamPos = ren->GetCamPos();
        ren->OverrideCamPos(lightWorldPos);
        //Atmosphere::GetCurrentAtmosphere()->OverrideCamPos(lightWorldPos);

        savedDensity = Billboard::GetFogDensity();
        Billboard::SetFogDensity(1E-9);
        Billboard::SetSpinEnabled(false);
        Billboard::OverrideBillboardMatrix(billboard);

        if (ren->MakeRenderTextureCurrent(texture, false)) {
            ren->ClearScreen(Color(1,1,1,1));
            ren->SortBlendedObjects();
            return true;
        }
    }

    return false;
}

bool ShadowMap::EndDrawing()
{
    Billboard::ForceConstantColor(false, Color(1,1,1,1));

    if (texture) {
        Renderer *ren = Renderer::GetInstance();
        ren->SetViewport(savedX, savedY, savedW, savedH);
        Billboard::RestoreBillboardMatrix();
        Billboard::SetFogDensity(savedDensity);
        Billboard::SetSpinEnabled(true);

        ren->OverrideCamPos(savedCamPos);
        //Atmosphere::GetCurrentAtmosphere()->OverrideCamPos(savedCamPos);

        return ren->BindRenderTexture(texture);
    }

    return false;
}

bool ShadowMap::DrawToRenderTexture()
{
    Renderer *ren = Renderer::GetInstance();
    if (texture) {
        if (hasShadows && shadowBlend > 0) {
            isRendering = true;
            ren->DrawBlendedObjects(false, false);
            isRendering = false;
        }

        return true;
    }
    return false;
}

TextureHandle ShadowMap::GetTextureHandle()
{
    TextureHandle texHandle = 0;
    Renderer::GetInstance()->GetRenderTextureTextureHandle(texture, &texHandle);
    return texHandle;
}

Matrix4 ShadowMap::GetLightModelviewProjectionMatrix()
{
    return lightMVP;
}

Matrix4 ShadowMap::GetWorldToShadowTexCoordMatrix()
{
    Renderer *ren = Renderer::GetInstance();
    Matrix4 bias;
    if (ren->GetIsDirectX()) {
        bias = Matrix4(
                   0.5,  0.0, 0.0, 0.5,
                   0.0, -0.5, 0.0, 0.5,
                   0.0,  0.0, 0.0, 1.0,
                   0.0,  0.0, 0.0, 1.0
               );
    } else {
        bias = Matrix4(
                   0.5f, 0.0f, 0.0f, 0.5f,
                   0.0f, 0.5f, 0.0f, 0.5f,
                   0.0f, 0.0f, 0.5f, 0.5f,
                   0.0f, 0.0f, 0.0f, 1.0f
               );
    }

    return bias * lightMVP;
}

void ShadowMap::FindFrustumWorldSpace(const SL_VECTOR(CloudLayer *)& cloudLayers, bool wholeLayers, double maxShadowDistance)
{
    hasShadows = false;

    if (cloudLayers.size() == 0) return;

    // Compute camera view frustum corners in world space
    Renderer *ren = Renderer::GetInstance();
    Matrix4 mv, proj, mvp, invMvp;
    ren->GetModelviewMatrix(&mv);
    ren->GetProjectionMatrix(&proj);
    mvp = proj * mv;
    invMvp = mvp.Inverse();

    float nearZ, farZ, zInc;
    if (ren->GetIsDirectX()) {
        nearZ = 0;
        farZ = 1.0f;
        Configuration::GetFloatValue("directx-near-clip-plane", nearZ);
        Configuration::GetFloatValue("directx-far-clip-plane", farZ);
    } else {
        nearZ = -1.0f;
        farZ = 1.0f;
        Configuration::GetFloatValue("opengl-near-clip-plane", nearZ);
        Configuration::GetFloatValue("opengl-far-clip-plane", farZ);
    }

    zInc = farZ - nearZ;

    Vector3 *worldCorners = SL_NEW Vector3[8 + cloudLayers.size() * 8];

    int numCorners = 0;
    Vector3 total(0, 0, 0);

    if (!wholeLayers) {
        for (float x = -1.0; x <= 1.0; x += 2.0) {
            for (float y = -1.0; y <= 1.0; y += 2.0) {
                for (float z = nearZ; z <= 1.0; z += zInc) {
                    Vector3 NDC(x, y, z);
                    Vector3 world = invMvp * NDC;

                    if (maxShadowDistance > 0) {
                        Vector3 cornerFromCamera = world - ren->GetCamPos();
                        double dist = cornerFromCamera.SquaredLength();
                        if (dist > maxShadowDistance * maxShadowDistance) {
                            Vector3 toCornerDir = cornerFromCamera;
                            toCornerDir.Normalize();
                            world = ren->GetCamPos() + (toCornerDir * maxShadowDistance);
                        }
                    }

                    total = total + world;
                    worldCorners[numCorners] = world;
                    frustumCornersWorld[numCorners] = world;
                    numCorners++;
                }
            }
        }
    }

    if (wholeLayers) {
        SL_VECTOR(CloudLayer *)::const_iterator it;
        for (it = cloudLayers.begin(); it != cloudLayers.end(); it++) {
            CloudLayer *layer = *it;
            if (layer->GetType() == CIRROCUMULUS ||
                    layer->GetType() == CIRRUS_FIBRATUS) continue;
            if (layer->GetCulled()) continue;
            const Vector3 *bounds = layer->GetBounds();
            for (int i = 0; i < 8; i++) {
                Vector3 corner = bounds[i];

                if (maxShadowDistance > 0) {
                    Vector3 cornerFromCamera = corner - ren->GetCamPos();
                    double dist = cornerFromCamera.SquaredLength();
                    if (dist > maxShadowDistance * maxShadowDistance) {
                        Vector3 toCornerDir = cornerFromCamera;
                        toCornerDir.Normalize();
                        corner = ren->GetCamPos() + (toCornerDir * maxShadowDistance);
                    }
                }

                worldCorners[numCorners] = corner;
                total = total + corner;
                numCorners++;
            }
        }
    }

    frustumCentroidWorld = total * (1.0/(double)numCorners);

    double maxDist = 0;
    for (int corner = 0; corner < numCorners; corner++) {
        double dist = (worldCorners[corner] - frustumCentroidWorld).Length();
        if (dist > maxDist) {
            maxDist = dist;
        }
    }

    SL_DELETE[] worldCorners;

    frustumBoundRadius = maxDist;

    hasShadows = true;
}

void ShadowMap::GetFrustumBounds(const Matrix4& xform, Vector3&  min, Vector3& max)
{
    for (int corner = 0; corner < 8; corner++) {
        Vector3 pos = xform * frustumCornersWorld[corner];
        if (corner == 0) {
            min.x = max.x = pos.x;
            min.y = max.y = pos.y;
            min.z = max.z = pos.z;
        } else {
            if (pos.x < min.x) min.x = pos.x;
            if (pos.x > max.x) max.x = pos.x;
            if (pos.y < min.y) min.y = pos.y;
            if (pos.y > max.y) max.y = pos.y;
            if (pos.z < min.z) min.z = pos.z;
            if (pos.z > max.z) max.z = pos.z;
        }
    }
}

Matrix4 ShadowMap::MakeOrthoMatrix(double left, double right, double top, double bottom, double pnear, double pfar)
{
    Matrix4 m;

    if (Renderer::GetInstance()->GetIsDirectX()) {
        if (Renderer::GetInstance()->GetIsRightHanded()) {
            m.elem[0][0] = 2.0 / (right - left);
            m.elem[0][3] = (left + right) / (left - right);
            m.elem[1][1] = 2.0 / (top - bottom);
            m.elem[1][3] = (top + bottom) / (bottom - top);
            m.elem[2][2] = 1.0 / (pnear - pfar);
            m.elem[2][3] = pnear / (pnear - pfar);
        } else {
            m.elem[0][0] = 2.0 / (right - left);
            m.elem[0][3] = (left + right) / (left - right);
            m.elem[1][1] = 2.0 / (top - bottom);
            m.elem[1][3] = (top + bottom) / (bottom - top);
            m.elem[2][2] = 1.0 / (pfar - pnear);
            m.elem[2][3] = pnear / (pnear - pfar);
        }
    } else {
        if (Renderer::GetInstance()->GetIsRightHanded()) {
            m.elem[0][0] = 2.0 / (right - left);
            m.elem[0][3] = -((right + left) / (right - left));

            m.elem[1][1] = 2.0 / (top - bottom);
            m.elem[1][3] = -((top + bottom) / (top - bottom));

            m.elem[2][2] = -2.0 / (pfar - pnear);
            m.elem[2][3] = -((pfar + pnear) / (pfar - pnear));
        } else {
            m.elem[0][0] = 2.0 / (right - left);
            m.elem[1][1] = 2.0 / (top - bottom);
            m.elem[2][2] = 1.0 / (pfar - pnear);
            m.elem[2][3] = pnear / (pnear - pfar);
        }
    }

    return m;
}

void ShadowMap::ComputeLightMatrix(const Vector3& lightDir, bool wholeLayers)
{
    // Place the light eye position

    Vector3 lightDirWorld = lightDir * Renderer::GetInstance()->GetBasis3x3();
    Vector3 lightPos = frustumCentroidWorld + (lightDirWorld * frustumBoundRadius);

    lightWorldPos = lightPos;

    Vector3 view;

    // Create a look-at matrix toward the center of the camera's frustum
    if (Renderer::GetInstance()->GetIsRightHanded())
        view = lightPos - frustumCentroidWorld;
    else
        view = frustumCentroidWorld - lightPos;

    view.Normalize();

    Vector3 worldUp = Renderer::GetInstance()->GetUpVector();
    Vector3 vright = worldUp.Cross(view);
    vright.Normalize();

    Vector3 vup = view.Cross(vright);

    Matrix4 lightView;
    lightView.elem[0][0] = vright.x;
    lightView.elem[0][1] = vright.y;
    lightView.elem[0][2] = vright.z;
    lightView.elem[0][3] = -vright.Dot(lightPos);
    lightView.elem[1][0] = vup.x;
    lightView.elem[1][1] = vup.y;
    lightView.elem[1][2] = vup.z;
    lightView.elem[1][3] = -vup.Dot(lightPos);
    lightView.elem[2][0] = view.x;
    lightView.elem[2][1] = view.y;
    lightView.elem[2][2] = view.z;
    lightView.elem[2][3] = -view.Dot(lightPos);

    // Compute billboard matrix for this view
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            billboard.elem[row][col] = lightView.elem[col][row];
        }
    }

    if (!Renderer::GetInstance()->GetIsRightHanded()) {
        billboard.elem[2][0] *= -1;
        billboard.elem[2][1] *= -1;
        billboard.elem[2][2] *= -1;
    }

    double left = -frustumBoundRadius, right = frustumBoundRadius;
    double top = frustumBoundRadius, bottom = -frustumBoundRadius;
    double pnear = -frustumBoundRadius * 2, pfar = frustumBoundRadius * 2;

    // Tighten up the light's frustum if we can
    if (!wholeLayers) {
        Vector3 min, max;
        GetFrustumBounds(lightView, min, max);
        left = min.x;
        right = max.x;
        bottom = min.y;
        top = max.y;
    }

    Matrix4 lightProj;
    // Create ortho proj matrix encompassing the frustum
    {
        Matrix4 m = MakeOrthoMatrix(left, right, top, bottom, pnear, pfar);

        lightProj = m;
    }

    lightMVP = lightProj * lightView;
    lightModelView = lightView;
    lightProjection = lightProj;
}
