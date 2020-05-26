// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Stars.h>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/Ephemeris.h>
#include <FeSilverliningLib/starcat.h>
#include <FeSilverliningLib/Profiler.h>
#include <FeSilverliningLib/LuminanceMapper.h>
#include <FeSilverliningLib/Glare.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Sky.h>
#include <stdlib.h>

#define MAG_0_LUMINANCE 0.05
#define DUSK RADIANS(5)

using namespace SilverLining;
using namespace std;

static double GetLuminance(double visualMagnitude)
{
    return MAG_0_LUMINANCE * exp(-0.921 * visualMagnitude);
}

void Stars::ReloadShaders()
{
    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->DeleteShader(starShader);
        starShader = ren->LoadShaderFromFile("Shaders/Stars.cg", VERTEX_PROGRAM);
    }
}

Stars::Stars(Ephemeris *eph, GlareManager *gmgr, double pTurbidity)
{
    Vertex *verts = 0;

    Configuration::GetDoubleValue("minimum-star-glare-magnitude", glareThreshold);

    maxLightPollution = 0.01f;
    Configuration::GetFloatValue("star-maximum-light-pollution", maxLightPollution);

    ephemeris = eph;
    turbidity = pTurbidity;
    glareManager = gmgr;

    nStars = 0;

    starShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/Stars.cg", VERTEX_PROGRAM);

    Configuration::GetBoolValue("disable-star-glare", noGlare);
    if (!glareManager->HasGlare()) noGlare = true;

    double minMagnitude = 10;
    Configuration::GetDoubleValue("minimum-star-magnitude", minMagnitude);

    starBrightness = 0;
    Configuration::GetDoubleValue("star-magnitude-adjustment", starBrightness);

    double starDistance = 100.0;
    Configuration::GetDoubleValue("sky-box-size", starDistance);
    starDistance *= Atmosphere::GetUnitScale();
    fStarDistance = 0.5f * (float)starDistance;

    geoZUp = true;
    Configuration::GetBoolValue("geocentric-z-is-up", geoZUp);

    int totalStars = 0;
    int i = 0;
    for (;;) {
        if (stars[i].visualMagnitude != 9999) {
            totalStars++;
            if (stars[i].visualMagnitude < minMagnitude) {
                nStars++;
            }
        } else {
            break;
        }

        i++;
    }

    starVerts = SL_NEW VertexBuffer(nStars);

    if (starVerts) {
        if (starVerts->LockBuffer()) {
            verts = starVerts->GetVertices();

            if (verts) {

                double epochCenturies = eph->GetEpochCenturies();
                double yearCorrection = 50.0 + epochCenturies * 100.0;

                double baseLuminance = GetLuminance(3.0);

                int j = 0;
                for (i = 0; i < totalStars; i++) {
                    if (stars[i].visualMagnitude >= minMagnitude) {
                        continue;
                    }

                    if (j >= nStars) {
                        break;
                    }

                    double ra = stars[i].ra + stars[i].raProperMotion * yearCorrection;
                    double dec = stars[i].dec + stars[i].decProperMotion * yearCorrection;

                    double luminance = GetLuminance(stars[i].visualMagnitude + starBrightness);

                    verts[j].x = fStarDistance * (float)(cos(ra) * cos(dec));
                    verts[j].y = fStarDistance * (float)(sin(ra) * cos(dec));
                    verts[j].z = fStarDistance * (float)(sin(dec));

                    if (starShader) {
                        verts[j].SetColor(stars[i].r, stars[i].g, stars[i].b, 255);
                        verts[j].u = (float)(stars[i].visualMagnitude + starBrightness);
                        verts[j].v = (float)luminance;
                    } else {
                        double it = GetLuminance(stars[i].visualMagnitude + starBrightness) / baseLuminance;
                        verts[j].SetColor(Color(it,it,it));
                    }

                    if ((!noGlare) && stars[i].visualMagnitude < glareThreshold) {
                        Glare *glare = SL_NEW Glare(glareManager);
                        glare->SetUserVector(Vector3(verts[j].x, verts[j].y, verts[j].z));
                        glare->SetIntensity(luminance);
                        glares.push_back(glare);
                    }

                    j++;
                }
            }
        }
        starVerts->UnlockBuffer();
    }

    planetGlares = SL_NEW Glare*[NUM_PLANETS];

    planetVerts = SL_NEW VertexBuffer(NUM_PLANETS);
    if (planetVerts) {
        planetVerts->LockBuffer();
        verts = planetVerts->GetVertices();

        if (verts) {
            for (i = 0; i < NUM_PLANETS; i++) {
                if (i == MARS) {
                    verts[i].SetColor(255, 153, 51, 255);
                } else if (i == EARTH) {
                    verts[i].SetColor(0, 0, 0, 0);
                } else {
                    verts[i].SetColor(255, 255, 255, 255);
                }

                verts[i].x = verts[i].y = verts[i].z = 0;
                verts[i].u = verts[i].v = 0;

                if (!noGlare) {
                    Glare *glare = SL_NEW Glare(glareManager);
                    glare->SetUserVector(Vector3(verts[i].x, verts[i].y, verts[i].z));
                    planetGlares[i] = glare;
                    glares.push_back(glare);
                } else {
                    planetGlares[i] = 0;
                }
            }
        }
        planetVerts->UnlockBuffer();
    }
}

Stars::~Stars()
{
    if (starVerts) {
        SL_DELETE starVerts;
    }

    if (planetVerts) {
        SL_DELETE planetVerts;
    }

    SL_VECTOR(Glare*) ::iterator it;
    for (it = glares.begin(); it != glares.end(); it++) {
        SL_DELETE (*it);
    }

    SL_DELETE[] planetGlares;
}

void Stars::UpdatePlanets()
{
    if (planetVerts && planetVerts->LockBuffer()) {
        Vertex *verts = planetVerts->GetVertices();
        if (verts) {
            for (int planet = 0; planet < NUM_PLANETS; planet++) {
                if (planet != EARTH) {
                    double ra, dec, mag;
                    ephemeris->GetPlanetPosition(planet, ra, dec, mag);

                    verts[planet].x = fStarDistance * (float)(cos(ra) * cos(dec));
                    verts[planet].y = fStarDistance * (float)(sin(ra) * cos(dec));
                    verts[planet].z = fStarDistance * (float)(sin(dec));

                    verts[planet].u = (float)(mag + starBrightness);
                    verts[planet].v = (float)GetLuminance(mag + starBrightness);

                    if (!starShader) {
                        // Convert magnitude to luminance
                        double luminance = pow(100.0, (-(mag + starBrightness) / 5.0));

                        double r2 = luminance / (pow(luminance, 2.0/3.0) + 0.02); // The .02 term controls the overall star brightness.

                        Color planetColor(r2, r2, r2);
                        planetColor.ClampToUnitOrLess();
                        verts[planet].SetColor(planetColor);
                    }

                    if (planetGlares[planet]) {
                        Vector3 worldPos(0, 0, 0);
                        float luminance = 0;
                        if (mag < glareThreshold) {
                            worldPos = Vector3(verts[planet].x, verts[planet].y, verts[planet].z);
                            planetGlares[planet]->SetUserVector(worldPos);
                            luminance = (float)GetLuminance(mag + starBrightness);
                        }
                        planetGlares[planet]->SetWorldPosition(worldPos);
                        planetGlares[planet]->SetIntensity(luminance);
                    }
                }
            }
        }
        planetVerts->UnlockBuffer();
    }
}

bool Stars::Draw(const Sky * sky)
{
    Timer t("Render stars");

    UpdatePlanets();

    Vector4 vFog;
    Vector4 vFogDistance;
    Color fogColor;
    double fogDensity, fogDistance;
    sky->GetFog(fogColor, fogDensity, fogDistance);

    vFog.x = fogColor.r;
    vFog.y = fogColor.g;
    vFog.z = fogColor.b;
    vFog.w = fogDensity;
    vFogDistance.x = fabs(fogDistance);

    Renderer *renderer = Renderer::GetInstance();
    if (renderer) {
        renderer->EnableBackfaceCulling(false);
        renderer->EnableTexture2D(false);
        renderer->EnableLighting(false);

        if (starShader) {
            renderer->EnableBlending(ONE, ONE);
        } else {
            renderer->EnableBlending(INVDSTCOLOR, ONE);
        }

        // Fade out stars with light pollution
        float lightPollutionFading = 1.0f;
        float lightPollution = (float)Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetLightPollution();
        float normalizedLightPollution = lightPollution / maxLightPollution;
        if (normalizedLightPollution > 1.0f) normalizedLightPollution = 1.0f;
        lightPollutionFading -= normalizedLightPollution;
        vFogDistance.y = lightPollutionFading;


        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        Renderer::GetInstance()->SetConstantVector(starShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        renderer->SetConstantVector4(starShader, "sl_fog", vFog);
        renderer->SetConstantVector4(starShader, "sl_fogDistance", vFogDistance);

        Vector3 up;
        if (sky->GetGeocentricMode()) {
            up = renderer->GetUpVector();
        } else {
            // The basis is already baked into the modelview matrix here.
            up = Vector3(0,1,0);
        }

        renderer->SetConstantVector(starShader, "sl_up", up);

        Matrix4 modelview, proj, modelviewProj;
        renderer->GetModelviewMatrix(&modelview);

        // Blow away any translation
        for (int row = 0; row < 3; row++) {
            modelview.elem[row][3] = 0;
        }
        modelview.elem[3][3] = 1;

        renderer->GetProjectionMatrix(&proj);
        modelviewProj = proj * modelview;

        renderer->SetConstantMatrix(starShader, "sl_modelViewProj", modelviewProj);

        Matrix4 invBasis = renderer->GetInverseBasis4x4();
        renderer->SetConstantMatrix(starShader, "sl_invBasis", invBasis);

        static Matrix3 coordXform(0, 1, 0,
                                  0, 0, 1,
                                  1, 0, 0);

        Matrix3 eh;
        if (sky->GetGeocentricMode()) {
            if (geoZUp) {
                eh = ephemeris->GetEquatorialToGeographicMatrix();
            } else {
                static Matrix3 xform(1,  0, 0,
                                     0,  0, 1,
                                     0, -1, 0);
                eh = xform * ephemeris->GetEquatorialToGeographicMatrix();
            }
        } else {
            eh = coordXform * ephemeris->GetEquatorialToHorizonMatrix();
        }

        Matrix4 eqToHoriz(eh.elem[0][0], eh.elem[0][1], eh.elem[0][2], 0,
                          eh.elem[1][0], eh.elem[1][1], eh.elem[1][2], 0,
                          eh.elem[2][0], eh.elem[2][1], eh.elem[2][2], 0,
                          0,             0,             0, 1);

        renderer->SetConstantMatrix(starShader, "sl_equatorialToHorizon", eqToHoriz);

        renderer->PushTransforms();

        if (!starShader) {
            renderer->MultiplyModelviewMatrix(eqToHoriz);
        }

        renderer->BindShader(starShader, VERTEX_PROGRAM);

        double starPointSize = 1.0;
        Configuration::GetDoubleValue("star-point-size", starPointSize);

        renderer->DrawPoints(starVerts->GetHandle(), starPointSize, nStars);

        renderer->DrawPoints(planetVerts->GetHandle(), starPointSize, NUM_PLANETS);

        renderer->UnbindShader(VERTEX_PROGRAM);

        DrawGlares(eqToHoriz);

        renderer->PopTransforms();

        //renderer->EnableBackfaceCulling(true);
        renderer->EnableLighting(false);
        renderer->DisableBlending();
    }

    return true;
}

void Stars::DrawGlares(const Matrix4& eqToHorizon)
{
    if (glareManager->EnableShaders()) {
        SL_VECTOR(Glare *) ::iterator it;
        for (it = glares.begin(); it != glares.end(); it++) {
            Vector3 pos = (*it)->GetUserVector();
            pos = eqToHorizon * pos;

            (*it)->SetWorldPosition(pos);
            (*it)->Draw(0);
        }

        glareManager->DisableShaders();
    }
}
