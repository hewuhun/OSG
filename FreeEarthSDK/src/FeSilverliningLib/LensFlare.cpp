// Copyright (c) 2010-2015 Sundog Software, LLC All rights reserved worldwide.

#include <FeSilverliningLib/LensFlare.h>
#include <FeSilverliningLib/Ephemeris.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Utils.h>

using namespace SilverLining;
using namespace std;

//#define TIMING

LensFlare::LensFlare(const Ephemeris *pEphemeris)
{
    geocentricMode = false;

    queryHandle = 0;

    occlusion = 0;

    ephemeris = pEphemeris;

    disableOcclusion = false;
    Configuration::GetBoolValue("lens-flare-disable-occlusion", disableOcclusion);

    usePBO = true;
    Configuration::GetBoolValue("lens-flare-use-pbo", usePBO);

    Configuration::GetDoubleValue("sun-width-degrees", sunWidth);

    flareSize = 0.2f;
    Configuration::GetFloatValue("lens-flare-size", flareSize);

    lensFlareDisabled = false;
    Configuration::GetBoolValue("disable-lens-flare", lensFlareDisabled);

    flareFalloff = 4.0f;
    Configuration::GetFloatValue("lens-flare-falloff", flareFalloff);

    flareBrightness = 1.0f;
    Configuration::GetFloatValue("lens-flare-brightness", flareBrightness);

    doShines = true;
    Configuration::GetBoolValue("lens-flare-shine", doShines);

    double cubeDimension = 1000.0;
    Configuration::GetDoubleValue("sky-box-size", cubeDimension);
    cubeDimension *= Atmosphere::GetUnitScale();
    sunDistance = cubeDimension * 0.5;

    sunQuad = SL_NEW Billboard();
    sunQuad->Initialize(false);
    sunQuad->SetColor(Color(0,0,0));

    for (int i = 0; i < NUM_FLARE_TEXTURES; i++) {
        flareTextures[i] = 0;
    }

    for (int j = 0; j < NUM_SHINE_TEXTURES; j++) {
        shineTextures[j] = 0;
    }

    sunTexture = 0;
    ib = 0;
    vb = 0;

    InitFlares();

    flareShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/Flare.cg", VERTEX_PROGRAM);

}

LensFlare::~LensFlare()
{
    for (int i = 0; i < NUM_FLARE_TEXTURES; i++) {
        if (flareTextures[i]) {
            Renderer::GetInstance()->ReleaseTexture(flareTextures[i]);
        }
    }

    for (int j = 0; j < NUM_SHINE_TEXTURES; j++) {
        if (shineTextures[j]) {
            Renderer::GetInstance()->ReleaseTexture(shineTextures[j]);
        }
    }

    if (sunQuad) {
        SL_DELETE sunQuad;
    }

    if (sunTexture) {
        Renderer::GetInstance()->ReleaseTexture(sunTexture);
    }

    if (ib) {
        SL_DELETE ib;
    }

    if (vb) {
        SL_DELETE vb;
    }
}

void LensFlare::DeviceLost()
{
    if (queryHandle) {
        Renderer::GetInstance()->GetOcclusionQueryResults(queryHandle);
        queryHandle = 0;
    }
}

bool LensFlare::InitFlares()
{
    bool useHexFlares = true;
    Configuration::GetBoolValue("use-hex-flares", useHexFlares);
    char buf[1024];

    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->LoadTextureFromFile("lensflare/black.tga", &sunTexture, false, false);

        int i;

        if (useHexFlares) {
            ren->LoadTextureFromFile("lensflare/hex.tga", &flareTextures[0], true, true);
        } else {
            for (i = 0; i < NUM_FLARE_TEXTURES; i++) {
#if (defined(WIN32) || defined(WIN64)) && (_MSC_VER > 1310)
                sprintf_s(buf, 1024, "lensflare\\Flare%d.tga", i+1);
#else
                sprintf(buf, "lensflare/Flare%d.tga", i);
#endif
                ren->LoadTextureFromFile(buf, &flareTextures[i], true, true);
            }
        }

        for (i = 0; i < NUM_SHINE_TEXTURES; i++) {
#if (defined(WIN32) || defined(WIN64)) && (_MSC_VER > 1310)
            sprintf_s(buf, 1024, "lensflare\\Shine%d.tga", i+1);
#else
            sprintf(buf, "lensflare/Shine%d.tga", i);
#endif
            ren->LoadTextureFromFile(buf, &shineTextures[i], true, true);
        }

        ib = SL_NEW IndexBuffer(4);
        if (ib->LockBuffer()) {
            Index *idx = ib->GetIndices();
            idx[0] = 0;
            idx[1] = 1;
            idx[2] = 2;
            idx[3] = 3;
            ib->UnlockBuffer();
        }

        vb = SL_NEW VertexBuffer(4);
        if (vb->LockBuffer()) {
            Color white(1,1,1,1);

            Vertex *v = vb->GetVertices();
            v[0].x = -1;
            v[0].y = -1;
            v[0].z = 0;
            v[0].w = 1.0f;
            v[0].SetUV(0, 0);
            v[0].SetColor(white);

            v[1].x = -1;
            v[1].y = 1;
            v[1].z = 0;
            v[1].w = 1.0f;
            v[1].SetUV(0, 1);
            v[1].SetColor(white);

            v[2].x = 1;
            v[2].y = -1;
            v[2].z = 0;
            v[2].w = 1.0f;
            v[2].SetUV(1, 0);
            v[2].SetColor(white);

            v[3].x = 1;
            v[3].y = 1;
            v[3].z = 0;
            v[3].w = 1.0f;
            v[3].SetUV(1, 1);
            v[3].SetColor(white);

            vb->UnlockBuffer();
        }
    }

    Color red(1.0, 0.0, 0.0);
    Color green(0.0, 1.0, 0.0);
    Color blue(0.0, 0.0, 1.0);
    Color darkGreen(136.0/255.0,165.0/255.0,22.0/255.0);
    Color brown(174.0/255.0,87.0/255.0,0.0);
    Color greenBlue(0.0,55.0/255.0,111.0/255.0);

    if (doShines) {
        flares.push_back(Flare(-1, 1.0f, 0.3f, blue));
        flares.push_back(Flare(-1, 1.0f, 0.2f, green));
        flares.push_back(Flare(-1, 1.0f, 0.25f, red));
    }

    if (useHexFlares) {
        flares.push_back(Flare(0, 1.0f, 0.5f, red));
        flares.push_back(Flare(0, 0.75f, 0.1f, greenBlue));
        flares.push_back(Flare(0, 0.7f, 0.1f, greenBlue));
        flares.push_back(Flare(0, 0.5f, 0.2f, greenBlue));
        flares.push_back(Flare(0, 0.4f, 0.25f, greenBlue));
        flares.push_back(Flare(0, 0.2f, 0.3f, greenBlue));
        flares.push_back(Flare(0, 0.1f, 0.3f, greenBlue));
        flares.push_back(Flare(0, 0.0f, 0.2f, brown));
        flares.push_back(Flare(0, 0.1f, 0.25f, brown));
        flares.push_back(Flare(0, -0.25f, 0.3f, brown));
        flares.push_back(Flare(0, -0.4f, 0.2f, brown));
        flares.push_back(Flare(0, -0.6f, 0.25f, darkGreen));
        flares.push_back(Flare(0, -1.f, 0.3f, darkGreen));
        flares.push_back(Flare(0, -1.3f, 0.4f, darkGreen));
        flares.push_back(Flare(0, -1.6f, 0.7f, darkGreen));
        flares.push_back(Flare(0, -2.0f, 1.0f, brown));
    } else {
        flares.push_back(Flare(3, 1.0f, 0.5f, red));
        flares.push_back(Flare(5, 0.75f, 0.1f, greenBlue));
        flares.push_back(Flare(5, 0.7f, 0.1f, greenBlue));
        flares.push_back(Flare(1, 0.5f, 2.0f, greenBlue));
        flares.push_back(Flare(2, 0.2f, 0.5f, greenBlue));
        flares.push_back(Flare(1, 0.5f, 2.0f, greenBlue));
        flares.push_back(Flare(3, 0.2f, 0.5f, greenBlue));
        flares.push_back(Flare(0, 0.0f, 0.4f, brown));
        flares.push_back(Flare(5, -0.25f, 0.7f, brown));
        flares.push_back(Flare(5, -0.4f, 0.2f, brown));
        flares.push_back(Flare(5, -0.6f, 0.4f, darkGreen));
        flares.push_back(Flare(5, -1.f, 0.3f, darkGreen));
        flares.push_back(Flare(3, -2.0f, 1.5f, brown));
    }
    return true;
}

void LensFlare::Update()
{
#ifdef TIMING
    DWORD start = timeGetTime();
#endif

    if (!ephemeris || lensFlareDisabled) return;

    // Compute the location of the sun in world coordinates
    Vector3 sunPosHorizon = ephemeris->GetSunPositionHorizon();
    double sunAltitude = DEGREES(asin(sunPosHorizon.y));
    if (sunAltitude < 0) {
        occlusion = 1.0f;
        return;
    }

    sunPosHorizon = sunPosHorizon * Renderer::GetInstance()->GetBasis3x3();

    Vector3 sunPosGeo = ephemeris->GetSunPositionGeographic();
    sunPos = geocentricMode ? sunPosGeo : sunPosHorizon;
    sunPos.Normalize();

    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        // Compute a camera-centered modelview matrix.
        ren->GetModelviewMatrix(&modelview);

        // Blow away any translation
        for (int row = 0; row < 3; row++) {
            modelview.elem[row][3] = 0;
        }
        modelview.elem[3][3] = 1;

        // Get position of sun in clip space
        Matrix4 proj, mvproj;
        ren->GetProjectionMatrix(&proj);
        mvproj = proj * modelview;

        Vector4 sunClip4 = mvproj * (sunPos * sunDistance);
        sunClip = Vector3(sunClip4.x / sunClip4.w, sunClip4.y / sunClip4.w, sunClip4.z / sunClip4.w);

        // Bail if the sun isn't in the scene at all.

        if (sunClip4.x <= -sunClip4.w || sunClip4.x > sunClip4.w ||
                sunClip4.y <= -sunClip4.w || sunClip4.y > sunClip4.w ||
                sunClip4.z <= 0 || sunClip4.z > sunClip4.w) {
            occlusion = 1.0f;
#ifdef TIMING
            printf("Bailed out of scene\n");
#endif
            return;
        }

        sunClip.z = 0;

        occlusion = 0;

        if (!disableOcclusion) {
#ifdef TIMING
            DWORD occlusionStart = timeGetTime();
#endif
            // Push our occlusion test billboard back to just inside the far clip plane
            double znear, zfar;
            ren->GetNearFarClip(znear, zfar);
            sunPos.Normalize();
            sunPos = sunPos * zfar * 0.99;

            sunQuad->SetWorldPosition(sunPos);

            Configuration::GetDoubleValue("sun-width-degrees", sunWidth);

            const double discPer = (256.0 - (57.0 * 2.0)) / 256.0;
            const double sunDiscSize = sunWidth * (1.0 / discPer);
            sunQuad->SetAngularSize(sunDiscSize);

            // Now, start an occlusion query to see if there's anything in front of it
            // in the depth buffer. This will pick up an occluders from the scene, but won't
            // pick up on clouds that occlude the sun, since they do not write to the depth
            // buffer.

            // We read the query from the previous frame (if any) in order to avoid stalling.
            unsigned int fragments = queryHandle ? ren->GetOcclusionQueryResults(queryHandle) : 0;

            if (ren->StartOcclusionQuery(&queryHandle) && sunTexture) {
                // Compute the total fragments the sun billboard takes up
                unsigned int totalFragments;
                double fovv;
                ren->GetFOV(fovv);
                fovv = DEGREES(fovv);
                int x, y, w, h;
                ren->GetViewport(x, y, w, h);
                double sunWidthScreen = (sunDiscSize / fovv) * (double)h;
                totalFragments = (unsigned int)(sunWidthScreen * sunWidthScreen);

                ren->PushTransforms();

                ren->EnableDepthReads(true);
                ren->EnableDepthWrites(false);
                ren->EnableBlending(ZERO, ONE); // Make it invisible
                ren->EnableLighting(false);

                ren->SetModelviewMatrix(modelview);

                sunQuad->Draw(sunTexture, 0, 1.0, false);

                ren->EndOcclusionQuery(queryHandle);

                ren->PopTransforms();
                ren->DisableBlending();
                ren->EnableDepthWrites(true);


                // Compute the percent of the sun billboard that was occluded.
                occlusion = 1.0f - ( (float)fragments / (float)totalFragments );
                if (occlusion < 0.0f) occlusion = 0.0f;
                if (occlusion > 1.0f) occlusion = 1.0f;

#ifdef TIMING
                DWORD elapsed = timeGetTime() - occlusionStart;
                printf("Occlusion query took %d ms\n", elapsed);
#endif
            }
        }

        // Now, if we think we are not completely occluded, move on to test for
        // occlusion against the clouds, by reading back a pixel from the framebuffer
        // and see if it's white or not. Yes, this stalls as well, see comment above.
        if (occlusion < 1.0f) {

#ifdef TIMING
            DWORD readStart = timeGetTime();
#endif
            // Get the sun position in screen coordinates
            int x, y, w, h;
            ren->GetViewport(x, y, w, h);

            Vector3 sunPosScreen;
            sunPosScreen.x = x + (1 + sunClip.x) * w / 2;
            sunPosScreen.y = y + (1 + sunClip.y) * h / 2;

            // Grab a pixel from the framebuffer
            // This too may be delayed one frame under the hood.
            unsigned int pixel = 0xffffffff;
            if (ren->GetPixels((int)sunPosScreen.x, (int)sunPosScreen.y, 1, 1, &pixel, !usePBO)) {
                Vector3 screenColor;
                screenColor.x = (double)((pixel & 0x000000ff) >> 0) / 255.0;
                screenColor.y = (double)((pixel & 0x0000ff00) >> 8) / 255.0;
                screenColor.z = (double)((pixel & 0x00ff0000) >> 16) / 255.0;

                //printf("Color %f %f %f fragments %d\n", screenColor.x, screenColor.y, screenColor.z, fragments);

                // Occlude if it's not white, more so the less white it is.
                double colorDistance = (screenColor - Vector3(1,1,1)).SquaredLength() * flareFalloff;
                if (colorDistance > 0) {
                    occlusion += (float)colorDistance;
                }
                if (occlusion > 1.0f) occlusion = 1.0f;
            }

#ifdef TIMING
            DWORD elapsed = timeGetTime() - readStart;
            printf("GetPixels took %d ms\n", elapsed);
#endif
        }
    }

#ifdef TIMING
    printf("Total update %d Occlusion is %f\n", timeGetTime() - start, occlusion);
#endif
}

void LensFlare::Draw()
{
    if (occlusion >= 1.0f || lensFlareDisabled) return;

#ifdef TIMING
    DWORD start = timeGetTime();
#endif

    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        // Render direct to clip space
        ren->PushTransforms();
        Matrix4 ident;
        ren->SetProjectionMatrix(ident);

        // Set render state
        ren->EnableTexture2D(true);
        ren->EnableDepthReads(false);
        ren->EnableDepthWrites(false);
        ren->EnableBlending(ONE, INVSRCCOLOR);
        ren->EnableBackfaceCulling(false);

        // Get the aspect ratio so the flares aren't squished
        int x, y, w, h;
        ren->GetViewport(x, y, w, h);
        double aspect = (double)w / (double)h;

        // Render each flare component
        SL_VECTOR(Flare) ::iterator it;
        for (it = flares.begin(); it != flares.end(); it++) {
            Flare f = *it;
            double s = f.size * flareSize;
            Vector3 p = sunClip * f.position;
            Matrix4 mv(s, 0, 0, p.x,
                       0, s * aspect, 0, p.y,
                       0, 0, 1, 0,
                       0, 0, 0, 1);

            Color c = f.color * (1.0f - occlusion) * flareBrightness;

            ren->SetCurrentColor(c);

            ren->SetModelviewMatrix(mv);

            TextureHandle tex = 0;
            if (f.texture == -1) { // animate shine textures
                static unsigned int glareTex = 0;
                tex = shineTextures[glareTex];
                glareTex++;
                glareTex = glareTex % NUM_SHINE_TEXTURES;
            } else {
                tex = flareTextures[f.texture];
            }

            if (tex) {
                ren->EnableTexture(tex, 0);

                if (flareShader) {
                    float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
                    Renderer::GetInstance()->SetConstantVector(flareShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));
                    Matrix4 modelview, proj;
                    ren->GetModelviewMatrix(&modelview);
                    ren->GetProjectionMatrix(&proj);
                    Matrix4 mvp = proj * modelview;
                    ren->SetConstantMatrix(flareShader, "sl_modelViewProj", mvp);
                    ren->SetConstantVector4(flareShader, "sl_lightingColor", Vector4(c.r, c.g, c.b, c.a));
                    ren->BindShader(flareShader, VERTEX_PROGRAM);
                    ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), 0, 4, 4, false);
                    ren->UnbindShader(VERTEX_PROGRAM);
                } else {
                    ren->UnbindShader(VERTEX_PROGRAM);
                    ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), 0, 4, 4, false);
                }
            }
        }

        // Reset
        ren->EnableDepthReads(true);
        ren->EnableDepthWrites(true);
        ren->DisableBlending();
        //ren->EnableBackfaceCulling(true);

        ren->PopTransforms();

#ifdef TIMING
        printf("Total draw %d\n", timeGetTime() - start);
#endif
    }
}
