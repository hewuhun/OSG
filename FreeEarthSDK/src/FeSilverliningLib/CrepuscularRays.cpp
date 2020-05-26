// Copyright (c) 2013-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CrepuscularRays.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/CloudLayer.h>

using namespace SilverLining;

CrepuscularRays::CrepuscularRays(Atmosphere *atm) : atmosphere(atm), cleared(false), effectScale(1.0f), isRendering(false), exposureScale(1.0)
{
    exposure = 0.0034f;
    decay = 1.0f;
    density = 0.84f;
    weight = 1.5f;

    Configuration::GetFloatValue("crepuscular-rays-exposure", exposure);
    Configuration::GetFloatValue("crepuscular-rays-decay", decay);
    Configuration::GetFloatValue("crepuscular-rays-density", density);
    Configuration::GetFloatValue("crepuscular-rays-weight", weight);

    texSize = 512;
    Configuration::GetIntValue("crepuscular-rays-texsize", texSize);

    renderTexture = 0;
    Renderer *ren = Renderer::GetInstance();
    ren->InitRenderTexture(texSize, texSize, &renderTexture);

    shader = 0;
    shader = ren->LoadShaderFromFile("Shaders/VolumetricLighting.cg", VERTEX_PROGRAM);

    bool DX = Renderer::GetInstance()->GetIsDirectX();

    float effectDepth = 0.5;
    Configuration::GetFloatValue("crepuscular-rays-depth", effectDepth);
    float nearDepth, farDepth;
    ren->GetDepthRange(nearDepth, farDepth);
    effectDepth = (1.0f - effectDepth) * nearDepth + effectDepth * farDepth;

    double cubeDimension = 1000.0;
    Configuration::GetDoubleValue("sky-box-size", cubeDimension);
    cubeDimension *= Atmosphere::GetUnitScale();
    sunDistance = cubeDimension * 0.5;

    vertexBuffer = SL_NEW VertexBuffer(4);
    if (vertexBuffer && vertexBuffer->LockBuffer()) {
        Vertex *verts = vertexBuffer->GetVertices();
        if (verts) {
            Color white(1,1,1,1);
            verts[0].SetUV(0, DX ? 1.0f : 0.0f);
            verts[0].x = -1;
            verts[0].y = -1;
            verts[0].z = effectDepth;
            verts[0].w = 1.0f;
            verts[0].SetColor(white);

            verts[1].SetUV(1, DX ? 1.0f : 0.0f);
            verts[1].x = 1;
            verts[1].y = -1;
            verts[1].z = effectDepth;
            verts[1].w = 1.0f;
            verts[1].SetColor(white);

            verts[2].SetUV(1, DX ? 0.0f: 1.0f);
            verts[2].x = 1, verts[2].y = 1;
            verts[2].z = effectDepth;
            verts[2].w = 1.0f;
            verts[3].SetColor(white);

            verts[3].SetUV(0, DX ? 0.0f : 1.0f);
            verts[3].x = -1, verts[3].y = 1;
            verts[3].z = effectDepth;
            verts[3].w = 1.0f;
            verts[3].SetColor(white);
        }
        vertexBuffer->UnlockBuffer();
    }

    indexBuffer = SL_NEW IndexBuffer(4);
    if (indexBuffer && indexBuffer->LockBuffer()) {
        Index *indices = indexBuffer->GetIndices();
        if (indices) {
            indices[0] = 3;
            indices[1] = 2;
            indices[2] = 0;
            indices[3] = 1;
        }
        indexBuffer->UnlockBuffer();
    }
}

CrepuscularRays::~CrepuscularRays()
{
    Renderer *ren = Renderer::GetInstance();
    if (renderTexture)
        ren->ReleaseRenderTexture(renderTexture);

    if (vertexBuffer)
        SL_DELETE vertexBuffer;

    if (indexBuffer)
        SL_DELETE indexBuffer;
}

bool CrepuscularRays::DrawToRenderTexture()
{
    if (effectScale == 0.0f || exposureScale <= 0.0f) return true;

    if (renderTexture) {
        Renderer *ren = Renderer::GetInstance();

        if (ren->MakeRenderTextureCurrent(renderTexture, !cleared)) {
            int savedX, savedY, savedW, savedH;
            ren->GetViewport(savedX, savedY, savedW, savedH);
            ren->SetViewport(0, 0, texSize, texSize);
            if (!cleared) ren->ClearScreen(Color(1,1,1,1));
            isRendering = true; // Flag to turn off cirrus
            ren->DrawBlendedObjects(false, false);
            ren->BindRenderTexture(renderTexture);
            isRendering = false;
            cleared = true;
            ren->SetViewport(savedX, savedY, savedW, savedH);

            return true;
        }
    }
    return false;
}

Vector3 CrepuscularRays::ComputeSunPosClip(const Vector3& sunWorld, bool& clipped)
{
    Renderer *ren = Renderer::GetInstance();

    // Compute a camera-centered modelview matrix.
    Matrix4 modelview;
    ren->GetModelviewMatrix(&modelview);

    // Blow away any translation
    for (int row = 0; row < 3; row++) {
        modelview.elem[row][3] = 0;
    }
    modelview.elem[3][3] = 1;

    // Account for which way is up...
    if (!geocentric) modelview = modelview * ren->GetInverseBasis4x4();

    // Get position of sun in clip space
    Matrix4 proj, mvproj;
    ren->GetProjectionMatrix(&proj);
    mvproj = proj * modelview;

    Vector4 sunClip4 = mvproj * sunWorld;
    Vector3 sunClip = Vector3(sunClip4.x / sunClip4.w, sunClip4.y / sunClip4.w, sunClip4.z / sunClip4.w);

    clipped = (sunClip4.x <= -sunClip4.w || sunClip4.x > sunClip4.w ||
               sunClip4.y <= -sunClip4.w || sunClip4.y > sunClip4.w ||
               sunClip4.z <= 0 || sunClip4.z > sunClip4.w);

    return sunClip;
}

static float smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = ((x - edge0)/(edge1 - edge0));
    if (x < 0.0f) x = 0.0f;
    if (x > 1.0f) x = 1.0f;

    // Evaluate polynomial
    return x*x*(3 - 2*x);
}

void CrepuscularRays::UpdateLightSource(const Ephemeris *ephemeris, const Color& sunColor, double cloudCoverage)
{
    lightColor = Vector3(sunColor.r, sunColor.g, sunColor.b);

    effectScale = 1.0;

    // Compute the location of the sun in world coordinates
    Vector3 sunPosHorizon = ephemeris->GetSunPositionHorizon();
    //double sunAltitude = DEGREES(asin(sunPosHorizon.y));
    Vector3 sunPosGeo = ephemeris->GetSunPositionGeographic();
    Vector3 sunWorld = geocentric ? sunPosGeo : sunPosHorizon;
    sunWorld.Normalize();

    bool alwaysOn = false;
    Configuration::GetBoolValue("crepuscular-rays-always-on", alwaysOn);

    bool clipped = false;
    sunPosClip = ComputeSunPosClip(sunWorld * sunDistance, clipped);

    float sunDistanceClip = 1.0;

    if (!alwaysOn) {
        if (clipped) {
            // Sun is behind the camera
            effectScale = 0;
        }
        /*
        if (Renderer::GetInstance()->GetIsDirectX()) {
            if (sunPosClip.z <= 0 || sunPosClip.z > 1.0)
            {
                // Sun is behind the camera
                effectScale = 0;
            }
        } else {
            if (sunPosClip.z > 0)
            {
                // Sun is behind the camera
                effectScale = 0;
            }
        }
        */
        sunPosClip.z = 0;
        sunPosTex = sunPosClip * 0.5 + 0.5;

        if (Renderer::GetInstance()->GetIsDirectX()) {
            sunPosTex.y = 1.0 - sunPosTex.y;
        }

        sunDistanceClip = (float)sunPosClip.Length();
    } else {
        float rayPosX = 0.5f, rayPosY = 0.5f;
        if (Configuration::GetFloatValue("crepuscular-rays-sun-pos-x", rayPosX) &&
                Configuration::GetFloatValue("crepuscular-rays-sun-pos-y", rayPosY)) {
            sunPosTex.z = 0;
            sunPosTex.x = rayPosX;
            sunPosTex.y = rayPosY;
        } else {
            sunPosClip.z = 0;
            sunPosTex = sunPosClip * 0.5 + 0.5;

            if (Renderer::GetInstance()->GetIsDirectX()) {
                sunPosTex.y = 1.0 - sunPosTex.y;
            }
        }
    }


    // Fade out effect as sun goes out of the frame
    effectScale *= 1.0f - smoothstep(1.5f, 2.5f, sunDistanceClip);

    // Fade out effect as occluders thin out
    effectScale *= (float)cloudCoverage;
}

void * CrepuscularRays::GetNativeCloudTexture()
{
    Renderer *ren = Renderer::GetInstance();
    if (renderTexture) {
        TextureHandle tex;
        ren->GetRenderTextureTextureHandle(renderTexture, &tex);
        return ren->GetNativeTexture(tex);
    }
    return 0;
}

bool CrepuscularRays::DrawRays()
{
    cleared = false;

    if (effectScale == 0.0f || exposureScale <= 0.0f) return true;

    Renderer *ren = Renderer::GetInstance();

    if (renderTexture && shader && vertexBuffer && indexBuffer) {
        //ren->PushAllState();
        ren->EnableTexture2D(true);
        ren->EnableLighting(false);
        ren->EnableBackfaceCulling(false);
        ren->EnableBlending(SRCALPHA, ONE);
        // ren->DisableBlending();
        TextureHandle tex;
        ren->GetRenderTextureTextureHandle(renderTexture, &tex);
        ren->EnableTexture(tex, 0);
        ren->EnableDepthReads(true);
        ren->EnableDepthWrites(false);
        ren->BindShader(shader, VERTEX_PROGRAM);

        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        Renderer::GetInstance()->SetConstantVector(shader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        ren->SetConstantVector(shader, "sl_lightPositionOnScreen", sunPosTex);

        ren->SetConstantVector4(shader, "sl_parameters", Vector4(exposure * effectScale * exposureScale, decay, density, weight));

        ren->SetConstantVector(shader, "sl_lightColor", lightColor);

        ren->DrawStrip(vertexBuffer->GetHandle(), indexBuffer->GetHandle(), 0, 4, 4, false);

        ren->UnbindShader(VERTEX_PROGRAM);

        ren->DisableTexture(0);

        //ren->PopAllState();

        return true;

    }

    return false;
}
