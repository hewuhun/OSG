// Copyright (c) 2004-2015  Sundog Software, LLC All rights reserved worldwide.

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Sky.h>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/IndexBuffer.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Profiler.h>
#include <FeSilverliningLib/Stars.h>
#include <FeSilverliningLib/LuminanceMapper.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/ArHosekSkyModelData_CIEXYZ.h>
#include <assert.h>
#include <stdlib.h>

using namespace SilverLining;

#define RENDER_BOTTOM_FACE


// CIE matrix
static const
Matrix3 XYZ2RGB_CIE( 2.370670, -0.5138850,  0.00529818,
                     -0.900040,  1.4253000, -0.0146949,
                     -0.470634,  0.0885814,  1.00940);

static const
Matrix4 XYZ2RGB4_CIE( 2.370670, -0.5138850,  0.00529818, 0,
                      -0.900040,  1.4253000, -0.0146949, 0,
                      -0.470634,  0.0885814,  1.00940, 0,
                      0,0,0,1);

// rec.709 HDTV matrix
static const Matrix3 XYZ2RGB_HDTV( 3.240479, -0.969256,   0.055648,
                                   -1.537150,  1.875992,  -0.204043,
                                   -0.498535,  0.041556,   1.057311 );

static const Matrix4 XYZ2RGB4_HDTV( 3.240479, -0.969256,  0.055648, 0,
                                    -1.537150,  1.875992,  -0.204043, 0,
                                    -0.498535,  0.041556,   1.057311, 0,
                                    0, 0, 0, 1);

// Luminance at the zenith below which we start drawing stars.
static const double duskZenithLuminance = 0.01;

static void XYZtoxyY(const Vector3& XYZ, Vector3& xyY)
{
    xyY.x = XYZ.x / (XYZ.x + XYZ.y + XYZ.z);
    xyY.y = XYZ.y / (XYZ.x + XYZ.y + XYZ.z);
    xyY.z = XYZ.y;
}

enum Faces {
    UP = 0,
#ifdef RENDER_BOTTOM_FACE
    DOWN,
#endif
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NUM_FACES
};

void Sky::ApplyGamma(Vector3& v) const
{
    double min = 0;
    if (v.x < min) min = v.x;
    if (v.y < min) min = v.y;
    if (v.z < min) min = v.z;
    min = -min;

    v = v + min;

    if (!Atmosphere::GetHDREnabled()) {
        double max = v.x;
        if (v.y > max) max = v.y;
        if (v.z > max) max = v.z;
        if (max > 1.0) {
            v.x /= max;
            v.y /= max;
            v.z /= max;
        }

        double oneOverGamma = skyModel == HOSEK_WILKIE ? oneOverGammaHosek : oneOverGammaPreetham;
        if (v.x > 0)
            v.x = pow(v.x, oneOverGamma);
        if (v.y > 0)
            v.y = pow(v.y, oneOverGamma);
        if (v.z > 0)
            v.z = pow(v.z, oneOverGamma);
    }
}

Sky::Sky() :
    ephemeris(0), lastSunT(0), lastMoonT(0), lastSunZenith(0), lastMoonZenith(0),
    lightingChanged(true),
    lastFogDistance(-1), lastFogDensity(-1), fogVolumeDistance(0), lastIsOvercast(false),
    hazeVolumeDistance(0), hazeColor(1, 1, 1), hazeDensity(0),
    sunGlowBillboard(0), moonGlowBillboard(0), sunGlare(0), moonGlare(0),
    isOvercast(false), overcastBlend(1.0), overcastTransmissionAmbient(0.2), isothermalEffect(1.0),
    geocentricMode(false), overcastTransmissionDirect(0.2),
    isInfraRed(false), isApplicationFog(false), disablePerCloudFog(false),
    sunAlpha(1.0), moonAlpha(1.0), skyModel(HOSEK_WILKIE), sunAboveHorizon(true), perezBlend(0),
    moonBrightness(2.0), sunAltitude(0), lastOvercastBlend(0)
{
    float yOffset = 0;

    Configuration::GetDoubleValue("sky-box-size", cubeDimension);
    cubeDimension *= Atmosphere::GetUnitScale();
    Configuration::GetIntValue("sky-box-resolution", rows);
    Configuration::GetIntValue("sky-box-resolution", cols);
    Configuration::GetDoubleValue("default-turbidity", T);
    Configuration::GetDoubleValue("sun-width-degrees", sunWidth);
    Configuration::GetDoubleValue("moon-width-degrees", moonWidth);
    Configuration::GetDoubleValue("max-skylight-luminance", maxSkylightLuminance);
    Configuration::GetFloatValue("sky-box-y-offset", yOffset);
    yOffset *= (float)Atmosphere::GetUnitScale();
    Configuration::GetBoolValue("disable-per-cloud-fog", disablePerCloudFog);
    Configuration::GetDoubleValue("starlight-scale", nightSkyScale);
    Configuration::GetDoubleValue("moon-brightness", moonBrightness);

    useGlowSun = true;
    useGlowMoon = true;
    glowSizeSun = 4.0;
    glowSizeMoon = 4.0;
    glowIntensitySun = 0.2;
    glowIntensityMoon = 0.5;
    glowSizeSunInfrared = 10.0;
    monochromeAmbient = false;
    daylightMoonDimming = 0.2;
    Configuration::GetBoolValue("enable-sun-glow", useGlowSun);
    Configuration::GetBoolValue("enable-moon-glow", useGlowMoon);
    Configuration::GetDoubleValue("sun-glow-size", glowSizeSun);
    Configuration::GetDoubleValue("moon-glow-size", glowSizeMoon);
    Configuration::GetDoubleValue("sun-glow-intensity", glowIntensitySun);
    Configuration::GetDoubleValue("moon-glow-intensity", glowIntensityMoon);
    Configuration::GetDoubleValue("sun-glow-size-infrared", glowSizeSunInfrared);
    Configuration::GetBoolValue("force-monochrome-ambient", monochromeAmbient);
    Configuration::GetDoubleValue("daylight-moon-dimming", daylightMoonDimming);

    yOffsetAngle = atan2((double)yOffset, cubeDimension / 2.0);

    oneOverGammaHosek = oneOverGammaPreetham = 0.45;
    double gamma;
    Configuration::GetDoubleValue("sky-box-gamma-preetham", gamma);
    oneOverGammaPreetham = 1.0 / gamma;
    Configuration::GetDoubleValue("sky-box-gamma-hosek", gamma);
    oneOverGammaHosek = 1.0 / gamma;

    drawSunBelowHorizon = true;
    Configuration::GetBoolValue("draw-sun-below-horizon", drawSunBelowHorizon);

    sunHasColor = false;
    Configuration::GetBoolValue("scatter-sun-color", sunHasColor);
    moonHasColor = false;
    Configuration::GetBoolValue("scatter-moon-color", moonHasColor);

    rowInc = cubeDimension / (rows - 1);
    halfRowInc = (cubeDimension * 0.5) / (rows - 2);
    colInc = cubeDimension / (cols - 1);
    halfDim = cubeDimension / 2;

    int nStrips = rows - 1;
    int nVerts = rows * cols;

    nIndices = nStrips * (cols * 2 + 2) - 2;

    indexBuffer = SL_NEW IndexBuffer(nIndices);
    if (indexBuffer && indexBuffer->LockBuffer()) {
        Index *indices = indexBuffer->GetIndices();
        if (indices) {
            int idx = 0;
            for (int strip = 0; strip < nStrips; strip++) {
                for (int col = 0; col < cols; col++) {
                    Index a = (Index)(strip * cols + col);
                    //assert (a < nVerts);
                    Index b = (Index)((strip + 1) * cols + col);
                    //assert (b < nVerts);

                    indices[idx++] = a;
                    indices[idx++] = b;
                }

                // Degenerate triangles to wrap back to next strip
                // (unless we're the last strip)

                if (strip < nStrips - 1) {
                    //assert(indices[idx-1] < nVerts);
                    indices[idx] = indices[idx-1];
                    ++idx;
                    assert((strip + 1) * cols < nVerts);
                    indices[idx] = (Index)((strip+1) * cols);
                    assert (idx < nIndices);
                    idx++;
                }
            }
        }
    }

    indexBuffer->UnlockBuffer();

    const float epsilon = 1.0;

    for (int face = 0; face < NUM_FACES; face++) {
        cubeFace[face] = SL_NEW VertexBuffer(nVerts);
        if (!cubeFace[face]) continue;

        localVerts[face] = SL_NEW Vector3f[nVerts];

        if (!cubeFace[face]->LockBuffer()) continue;

        Vertex *verts = cubeFace[face]->GetVertices();

        if (verts) {
            for (int row = 0; row < rows; row++) {
                for (int col = 0; col < cols; col++) {
                    int idx = row * cols + col;

                    verts[idx].w = 1.0;

                    switch (face) {
                    case UP:
                        verts[idx].x = localVerts[face][idx].x = (float)(col * colInc - halfDim);
                        verts[idx].y = localVerts[face][idx].y = (float)halfDim - (float)halfRowInc + epsilon;
                        verts[idx].z = localVerts[face][idx].z = (float)(row * rowInc - halfDim);
                        break;
#ifdef RENDER_BOTTOM_FACE
                    case DOWN:
                        verts[idx].x = localVerts[face][idx].x = (float)(col * colInc - halfDim);
                        verts[idx].y = localVerts[face][idx].y = (float)(-halfDim);
                        verts[idx].z = localVerts[face][idx].z = (float)(row * rowInc - halfDim);
                        break;
#endif
                    case NORTH:
                        verts[idx].x = localVerts[face][idx].x = (float)(col * colInc - halfDim);
                        if (row == 0) {
                            verts[idx].y = localVerts[face][idx].y = -(float)halfDim;
                        } else {
                            verts[idx].y = localVerts[face][idx].y = (float)((row - 1) * halfRowInc) + epsilon;
                        }
                        verts[idx].z = localVerts[face][idx].z = -(float)halfDim;
                        break;

                    case SOUTH:
                        verts[idx].x = localVerts[face][idx].x = (float)(col * colInc - halfDim);
                        if (row == 0) {
                            verts[idx].y = localVerts[face][idx].y = -(float)halfDim;
                        } else {
                            verts[idx].y = localVerts[face][idx].y = (float)((row - 1) * halfRowInc) + epsilon;
                        }
                        verts[idx].z = localVerts[face][idx].z = (float)halfDim;
                        break;

                    case EAST:
                        verts[idx].x = localVerts[face][idx].x = (float)halfDim;
                        if (row == 0) {
                            verts[idx].y = localVerts[face][idx].y = -(float)halfDim;
                        } else {
                            verts[idx].y = localVerts[face][idx].y = (float)((row - 1) * halfRowInc) + epsilon;
                        }
                        verts[idx].z = localVerts[face][idx].z = (float)(col * colInc - halfDim);
                        break;

                    case WEST:
                        verts[idx].x = localVerts[face][idx].x = -(float)halfDim;
                        if (row == 0) {
                            verts[idx].y = localVerts[face][idx].y = -(float)halfDim;
                        } else {
                            verts[idx].y = localVerts[face][idx].y = (float)((row - 1) * halfRowInc) + epsilon;
                        }
                        verts[idx].z = localVerts[face][idx].z = (float)(col * colInc - halfDim);
                        break;
                    }

                    verts[idx].y += yOffset;

                    verts[idx].SetColor(Color(1, 0, 0));

                    // Shove angle from zenith in u
                    const Vector3 zenith(0, 1, 0);
                    Vector3 v;
                    v.x = localVerts[face][idx].x;
                    v.y = localVerts[face][idx].y;
                    v.z = localVerts[face][idx].z;
                    v.Normalize();
                    float dot = (float)zenith.Dot(v);
                    const float epsilon = 1E-10f;
                    verts[idx].u = dot > 0 ? dot : epsilon;
                }
            }
        }
        cubeFace[face]->UnlockBuffer();
    }

    faceDirs[NORTH] = Vector3(0, 0, -1);
    faceDirs[SOUTH] = Vector3(0, 0, 1);
    faceDirs[EAST] = Vector3(1, 0, 0);
    faceDirs[WEST] = Vector3(-1, 0, 0);
    faceDirs[UP] = Vector3(0, 1, 0);

#ifdef RENDER_BOTTOM_FACE
    faceDirs[DOWN] = Vector3(0, -1, 0);
#endif

    Renderer *renderer = Renderer::GetInstance();

#ifdef ANDROID
    simpleShader = true;
#else
    Configuration::GetBoolValue("sky-simple-shader", simpleShader);
#endif

    skyShader = skyShaderHDR = skyShaderHW = skyShaderHDRHW = 0;

    LoadShaders();

    sunTexture = 0;
    sunBillboard = 0;
    glowTexture = 0;
    sunGlowBillboard = 0;
    moonGlowBillboard = 0;

    if (!renderer->LoadTextureFromFile("sun.tga", &sunTexture, false, false)) {
#ifdef _DEBUG
        printf("Error loading sun texture.\n");
#endif
    } else {
        sunBillboard = SL_NEW Billboard();
        sunBillboard->Initialize(false);
        sunBillboard->SetColor(Color(1, 1, 1, 1));
    }

    if (!renderer->LoadTextureFromFile("radial-glow.tga", &glowTexture, false, false)) {
#ifdef _DEBUG
        printf("Error loading glow texture.\n");
#endif
    } else {
        sunGlowBillboard = SL_NEW Billboard();
        sunGlowBillboard->Initialize(false);
        sunGlowBillboard->SetColor(Color(glowIntensitySun, glowIntensitySun, glowIntensitySun));
        moonGlowBillboard = SL_NEW Billboard();
        moonGlowBillboard->Initialize(false);
        moonGlowBillboard->SetColor(Color(glowIntensityMoon, glowIntensityMoon, glowIntensityMoon));
    }

    for (int day = 0; day < 30; day++) {
        moonTexture[day] = 0;
        moonBillboard[day] = 0;

        char buf[1024];
#if (defined(WIN32) || defined(WIN64)) && (_MSC_VER > 1310)
        sprintf_s(buf, 1024, "moonday%d.tga", day+1);
#else
        sprintf(buf, "moonday%d.tga", day+1);
#endif
        if (!renderer->LoadTextureFromFile(buf, &moonTexture[day], false, false)) {
#ifdef _DEBUG
            printf("Error loading moon texture.\n");
#endif
        } else {
            moonBillboard[day] = SL_NEW Billboard();
            moonBillboard[day]->Initialize(false);
            moonBillboard[day]->SetColor(Color(1, 1, 1, 1));
        }
    }

    stars = 0;

    glareMgr = SL_NEW GlareManager();

    bool noSunGlare, noMoonGlare;

    Configuration::GetBoolValue("disable-sun-glare", noSunGlare);
    Configuration::GetBoolValue("disable-moon-glare", noMoonGlare);

    if (!noSunGlare) {
        sunGlare = SL_NEW Glare(glareMgr);
    }

    if (!noMoonGlare) {
        moonGlare = SL_NEW Glare(glareMgr);
    }

    InitTwilightZenithLuminanceLookup();
}

void Sky::LoadShaders()
{
    Renderer *renderer = Renderer::GetInstance();

    renderer->DeleteShader(skyShader);
    renderer->DeleteShader(skyShaderHW);
    renderer->DeleteShader(skyShaderHDR);
    renderer->DeleteShader(skyShaderHDRHW);

    if (!simpleShader) {
        skyShader = renderer->LoadShaderFromFile("Shaders/Sky-complex.cg", VERTEX_PROGRAM);
        skyShaderHW = renderer->LoadShaderFromFile("Shaders/Sky-Hosek-Wilkie.cg", VERTEX_PROGRAM);
    }

    if (!skyShader || !skyShaderHW) {
        skyShader = renderer->LoadShaderFromFile("Shaders/Sky.cg", VERTEX_PROGRAM);
        simpleShader = true;
    }

    skyShaderHDR = renderer->LoadShaderFromFile("Shaders/SkyHDR.cg", VERTEX_PROGRAM);
    skyShaderHDRHW = renderer->LoadShaderFromFile("Shaders/Sky-Hosek-Wilkie-HDR.cg", VERTEX_PROGRAM);
}

Sky::~Sky()
{
    Renderer *renderer = Renderer::GetInstance();

    for (int face = 0; face < NUM_FACES; face++) {
        SL_DELETE cubeFace[face];
        SL_DELETE[] localVerts[face];
    }

    SL_DELETE indexBuffer;

    if (stars) {
        SL_DELETE stars;
    }

    if (sunTexture) {
        renderer->ReleaseTexture(sunTexture);
    }

    if (glowTexture) {
        renderer->ReleaseTexture(glowTexture);
    }

    if (sunBillboard) {
        SL_DELETE sunBillboard;
    }

    if (moonGlowBillboard) {
        SL_DELETE moonGlowBillboard;
    }

    if (sunGlowBillboard) {
        SL_DELETE sunGlowBillboard;
    }

    for (int day = 0; day < 30; day++) {
        if (moonTexture[day]) {
            renderer->ReleaseTexture(moonTexture[day]);
        }

        if (moonBillboard[day]) {
            SL_DELETE moonBillboard[day];
        }
    }

    if (sunGlare) {
        SL_DELETE sunGlare;
    }

    if (moonGlare) {
        SL_DELETE moonGlare;
    }

    if (glareMgr) {
        SL_DELETE glareMgr;
    }

    renderer->DeleteShader(skyShader);
    renderer->DeleteShader(skyShaderHW);
    renderer->DeleteShader(skyShaderHDR);
    renderer->DeleteShader(skyShaderHDRHW);
}

double Sky::PerezY(double theta, double gamma) const
{
    return
        (1.0 + AY * exp(BY / cos(theta))) *
        (1.0 + CY * exp(DY * gamma) + EY * cos(gamma) * cos(gamma));
}

double Sky::Perezx(double theta, double gamma) const
{
    return
        (1.0 + Ax * exp(Bx / cos(theta))) *
        (1.0 + Cx * exp(Dx * gamma) + Ex * cos(gamma) * cos(gamma));

}

double Sky::Perezy(double theta, double gamma) const
{
    return
        (1.0 + Ay * exp(By / cos(theta))) *
        (1.0 + Cy * exp(Dy * gamma) + Ey * cos(gamma) * cos(gamma));
}

double Sky::HosekWilkie(int channel, double theta, double gamma) const
{
    const double *configuration = hosekWilkieCoeffs[channel];
    const double expM = exp(configuration[4] * gamma);
    const double rayM = cos(gamma)*cos(gamma);
    const double mieM = (1.0 + cos(gamma)*cos(gamma)) / pow((1.0 + configuration[8]*configuration[8] - 2.0*configuration[8]*cos(gamma)), 1.5);
    const double zenith = sqrt(cos(theta));

    return (1.0 + configuration[0] * exp(configuration[1] / (cos(theta) + 0.01))) *
           (configuration[2] + configuration[3] * expM + configuration[5] * rayM + configuration[6] * mieM + configuration[7] * zenith);
}

double Sky::HosekWilkieX(double theta, double gamma) const
{
    return HosekWilkie(0, theta, gamma);
}

double Sky::HosekWilkieY(double theta, double gamma) const
{
    return HosekWilkie(1, theta, gamma);
}

double Sky::HosekWilkieZ(double theta, double gamma) const
{
    return HosekWilkie(2, theta, gamma);
}

void Sky::SetEphemeris(Ephemeris *pEph, double altitude)
{
    ephemeris = pEph;
    if (!stars) {
        stars = SL_NEW Stars(ephemeris, glareMgr, T);
    }

    lightingChanged = false;

    ComputeSun(altitude);
    ComputeMoon(altitude);

    if (lightingChanged) {
        UpdatePerezCoefficients();
        UpdateHosekWilkieCoefficients();
    }

    UpdateZenith(altitude);

    sunx = Perezx(0, thetaS);
    suny = Perezy(0, thetaS);
    sunY = PerezY(0, thetaS);
    moonY = PerezY(0, thetaM);
    moonx = Perezx(0, thetaM);
    moony = Perezy(0, thetaM);

    /*
    Vector3 sunXYZ;
    sunXYZ.x = HosekWilkieX(0, thetaS);
    sunXYZ.y = HosekWilkieY(0, thetaS);
    sunXYZ.z = HosekWilkieZ(0, thetaS);

    Vector3 sunxyY;
    XYZtoxyY(sunXYZ, sunxyY);

    sunx = sunxyY.x;
    suny = sunxyY.y;
    sunY = sunxyY.z;

    moonY = PerezY(0, thetaM);
    moonx = Perezx(0, thetaM);
    moony = Perezy(0, thetaM);
    */
    ComputeLogAvg();
    ComputeToneMappedSkyLight();
}

void Sky::PtToxyY(const Vector3& pt, double &x, double &y, double &Y, double zenithAngleOffset /* = 0.0 */) const
{
    x = y = Y = 0;

    if (ephemeris) {
        Vector3 sunPos = ephemeris->GetSunPositionHorizon();
        sunPos.Normalize();
        double gammaS = AngleBetween(sunPos, pt);

        const Vector3 zenith(0, 1, 0);
        double theta = AngleBetween(zenith, pt) + zenithAngleOffset;

        if (skyModel == PREETHAM || !sunAboveHorizon) {
            if (sunY) Y = YZenith * (PerezY(theta, gammaS) / sunY);

            if (sunx) x = xZenith * (Perezx(theta, gammaS) / sunx);

            if (suny) y = yZenith * (Perezy(theta, gammaS) / suny);
        } else {
            Vector3 sunXYZ;

            sunXYZ.x = HosekWilkieX(theta, gammaS) * hosekWilkieRadiances[0];
            sunXYZ.y = HosekWilkieY(theta, gammaS) * hosekWilkieRadiances[1];
            sunXYZ.z = HosekWilkieZ(theta, gammaS) * hosekWilkieRadiances[2];

            Vector3 sunxyY;
            XYZtoxyY(sunXYZ, sunxyY);
            x = sunxyY.x;
            y = sunxyY.y;
            Y = sunxyY.z * isothermalEffect;
        }

        if (isOvercast) {
            double Yo = YZenith * ((1 + 2 * cos(theta)) / 3);
            double xo = 0.310;
            double yo = 0.316;

            Y = overcastBlend * Yo + (1.0 - overcastBlend) * Y;
            x = overcastBlend * xo + (1.0 - overcastBlend) * x;
            y = overcastBlend * yo + (1.0 - overcastBlend) * y;
        }

        if (Y < 0) Y = 0;
    }
}

Color Sky::GetOvercastHorizonColor() const
{
    double Yo = YZenith * (1.0 / 3.0);
    double xo = 0.310;
    double yo = 0.316;

    double X = 0, Y = 0, Z = 0;
    Yo *= 1000.0;
    Y = Yo;
    if (yo) X = xo * (Yo / yo);
    if (yo) Z = (1.0 - xo - yo) * (Yo / yo);

    Vector3 XYZ(X, Y, Z);

    if (!Atmosphere::GetHDREnabled()) {
        LuminanceMapper::DurandMapperXYZ(&XYZ);
    } else {
        // Convert to kCD
        XYZ = XYZ * 0.001;
    }

    Vector3 rgb;

    rgb = XYZ * XYZ2RGB_HDTV;

    ApplyGamma(rgb);

    return Color(rgb.x, rgb.y, rgb.z);
}

void Sky::PtToxyYMoon(const Vector3& pt, double &x, double&y, double &Y, double zenithAngleOffset /* = 0.0 */) const
{
    x = y = Y = 0;
    if (ephemeris) {
        Vector3 moonPos = ephemeris->GetMoonPositionHorizon();
        moonPos.Normalize();
        double gammaM = AngleBetween(moonPos, pt);

        const Vector3 zenith(0, 1, 0);
        double theta = AngleBetween(zenith, pt) + zenithAngleOffset;

        if (moonY) Y = YMoon * (PerezY(theta, gammaM) / moonY);

        if (moonx) x = xMoon * (Perezx(theta, gammaM) / moonx);

        if (moony) y = yMoon * (Perezy(theta, gammaM) / moony);

        if (isOvercast) {
            double Yo = YMoon * (1 + 2 * cos(theta) / 3);
            double xo = 0.310;
            double yo = 0.316;

            Y = overcastBlend * Yo + (1.0 - overcastBlend) * Y;
            x = overcastBlend * xo + (1.0 - overcastBlend) * x;
            y = overcastBlend * yo + (1.0 - overcastBlend) * y;
        }

        if (Y < 0) Y = 0;
    }
}

double Sky::NightSkyLuminance()
{
    const double Wm2 = lightPollution
                       + 2.0E-6 // Bright planets
                       + 1.2E-7  // Zodiacal light
                       + 3.0E-8  // Integrated starlight
                       + 5.1E-8  // Airglow
                       + 9.1E-9  // Diffuse galactic light
                       + 9.1E-10; // Cosmic light

    double nits = NITS(Wm2);

    return nits * isothermalEffect * 0.001 * nightSkyScale;
}

double Sky::MoonLuminance()
{
    double luminance = 0;

    if (ephemeris) {
        Vector3 moonPos = ephemeris->GetMoonPositionHorizon();
        moonPos.Normalize();
        double moonAngle = DEGREES(asin(moonPos.y));

        //if (moonAngle > -18)
        {
            const double Esm = 1905.0; // W/m2
            const double C = 0.072;
            const double Rm = 1738.1 * 1000.0; // m
            double d = ephemeris->GetMoonDistanceKM() * 1000.0;

            // The equations for the illumination from the moon below assume that
            // the moon phase angle is 1 when full and 0 when new, which is the
            // opposite of the convention assumed by the Ephemeris class. So,
            // we assign the Earth's phase to the moon phase angle (which is always
            // its opposite) and take the opposite of that to determine the moon
            // phase angle for the purposes of these calculations.

            double epsilon = 0.001;

            double ePhase = ephemeris->GetMoonPhaseAngle();
            assert(ePhase >= 0);
            if (ePhase < epsilon) ePhase = epsilon;

            double alpha = PI - ePhase;
            while (alpha < 0) {
                alpha += 2.0 * PI;
            }
            assert(alpha >= 0);
            if (alpha < epsilon) alpha = epsilon;

            // Earthshine:
            double Eem = 0.19 * 0.5 *
                         (1.0 - sin(ePhase / 2.0) * tan(ePhase / 2.0) * log(1.0 / tan(ePhase / 4.0)));

            // Total moonlight:
            double Em = ((2.0 * C * Rm * Rm) / (3.0 * d * d)) *
                        (Eem + Esm *
                         (1.0 - sin(alpha / 2.0) * tan(alpha / 2.0) * log(1.0 / tan(alpha / 4.0))));

            double nits = NITS(Em);

            nits *= 0.001;

            if (moonAngle < 0) {
                nits = nits * exp(1.1247 * moonAngle);
            }

            luminance = nits;
        }
    }

    return luminance;
}

static void scaleDownToOne(Vector3 &v)
{
    if (!Atmosphere::GetHDREnabled()) {
        double minC = 0;
        if (v.x < minC) minC = v.x;
        if (v.y < minC) minC = v.y;
        if (v.z < minC) minC = v.z;
        v = v + -minC;

        double maxC = v.x;
        if (v.y > maxC) maxC = v.y;
        if (v.z > maxC) maxC = v.z;

        if (maxC > 1.0) {
            v.x /= maxC;
            v.y /= maxC;
            v.z /= maxC;
        }
    }
}

Color Sky::GetAverageHorizonColor(double pitch)
{
    Matrix4 mv;
    Renderer::GetInstance()->GetModelviewMatrix(&mv);

    mv = mv * Renderer::GetInstance()->GetInverseBasis4x4();

    Vector3 lookAt;

    if (Renderer::GetInstance()->GetIsRightHanded()) {
        lookAt = Vector3(-mv.elem[2][0], -mv.elem[2][1], -mv.elem[2][2]);
    } else {
        lookAt = Vector3(mv.elem[2][0], mv.elem[2][1], mv.elem[2][2]);
    }

    double yaw = DEGREES(atan2(lookAt.x, -lookAt.z));

    return GetAverageHorizonColor(yaw, pitch);
}

Color Sky::GetAverageHorizonColor(double yawDegrees, double pitchDegrees)
{
    double fov=-1.0;
    Configuration::GetDoubleValue("average-horizon-color-fov", fov);
    if (fov < 0.0)
        Renderer::GetInstance()->GetFOV(fov);

    // Make relative to X axis instead of -Z
    yawDegrees -= 90.0;
    double forwardRad = RADIANS(yawDegrees);

    int nSamples = 8;
    Configuration::GetIntValue("average-horizon-color-num-samples", nSamples);
    double radStep = fov / (nSamples - 1);
    double rad = forwardRad - (fov * 0.5);

    double pty = sin(-yOffsetAngle + RADIANS(pitchDegrees));

    Vector3 XYZ;
    for (int sample = 0; sample < nSamples; sample++) {
        double x, y, Y, xm, ym, Ym, X, Z, Xm, Zm;

        X = Y = Z = Xm = Ym = Zm = 0;

        Vector3 pt(cos(rad), pty, sin(rad));
        PtToxyY(pt, x, y, Y);
        PtToxyYMoon(pt, xm, ym, Ym);

        Y *= 1000.0;
        Ym *= 1000.0;

        if (y) X = x * (Y / y);
        if (ym) Xm = xm * (Ym / ym);
        if (y) Z = (1.0 - x - y) * (Y / y);
        if (ym) Zm = (1.0 - xm - ym) * (Ym / ym);

        if (skyShader && simpleShader) {
            if (YZenith < YMoon) {
                X = Xm;
                Y = Ym;
                Z = Zm;
            }
        } else {
            X += Xm;
            Y += Ym;
            Z += Zm;
        }

        XYZ = XYZ + Vector3(X, Y, Z);

        rad += radStep;
    }

    XYZ = XYZ * (1.0 / (double)nSamples);

    if (!Atmosphere::GetHDREnabled()) {
        LuminanceMapper::DurandMapperXYZ(&XYZ);
    } else {
        // Convert to kCD
        XYZ = XYZ * 0.001;
    }

    Vector3 rgb;

    rgb = XYZ * XYZ2RGB_HDTV;

    ApplyGamma(rgb);

    return Color(rgb.x, rgb.y, rgb.z);
}

Color Sky::SkyColorAt(const Vector3& direction, double zenithAngleOffset /* = 0.0 */) const
{
    if (isInfraRed) {
        return Color(0,0,0);
    }

    if (isApplicationFog || disablePerCloudFog) {
        return Billboard::GetFogColor();
    }

    Color fogColor;
    double fogDensity, fogDistance;
    GetFog(fogColor, fogDensity, fogDistance);
    const Vector3 zenith(0, 1, 0);

    Vector3 pt = direction;

    Vector3 XYZ;
    double x, y, Y, xm, ym, Ym, X, Z, Xm, Zm;

    X = Y = Z = Xm = Ym = Zm = 0;

    if (pt.y < 0.01) pt.y = 0.01; // avoid singularity

    PtToxyY(pt, x, y, Y, zenithAngleOffset);
    PtToxyYMoon(pt, xm, ym, Ym, zenithAngleOffset);

    Y *= 1000.0;
    Ym *= 1000.0;

    if (y) X = x * (Y / y);
    if (ym) Xm = xm * (Ym / ym);
    if (y) Z = (1.0 - x - y) * (Y / y);
    if (ym) Zm = (1.0 - xm - ym) * (Ym / ym);

    if (skyShader && simpleShader) {
        if (YZenith < YMoon) {
            X = Xm;
            Y = Ym;
            Z = Zm;
        }
    } else {
        X += Xm;
        Y += Ym;
        Z += Zm;
    }

    XYZ = Vector3(X, Y, Z);

    if (!Atmosphere::GetHDREnabled()) {
        LuminanceMapper::DurandMapperXYZ(&XYZ);
    } else {
        // Convert to kCD
        XYZ = XYZ * 0.001;
    }

    Vector3 rgb;

    rgb = XYZ * XYZ2RGB_HDTV;

    ApplyGamma(rgb);

    Color c(rgb.x, rgb.y, rgb.z);

    if (fogDistance > 0 && fogDensity > 0) {
        pt.Normalize();
        double cosTheta = zenith.Dot(pt);
        if (cosTheta > 0) {
            double z = fabs(fogDistance) / cosTheta;
            double f = exp(-(fogDensity * z));

            c = (c * (float)f) + (fogColor * (1.0f-(float)f));
        }
    }

    return c;
}

void Sky::ComputeLogAvg()
{
    double Y, R;
    Vector3 scatteredLuminance;

    if (skyShader && simpleShader) {
        if (YMoon > YZenith)
            scatteredLuminance = moonScatteredLuminance + moonTransmittedLuminance;
        else
            scatteredLuminance = sunScatteredLuminance;
    } else {
        scatteredLuminance = sunScatteredLuminance + moonScatteredLuminance + moonTransmittedLuminance;
    }

    scatteredLuminance.y += NightSkyLuminance() * 1000.0;

    Y = scatteredLuminance.y;
    R = -0.702 * scatteredLuminance.x + 1.039 * scatteredLuminance.y + 0.433 * scatteredLuminance.z;

    LuminanceMapper::SetSceneLogAvg(R,Y);
}

void Sky::SetHaze(const Color& c, double depth, double density)
{
    hazeColor = c;
    hazeVolumeDistance = depth;
    hazeDensity = density;
}

void Sky::GetHaze(Color& c, double& depth, double& density)
{
    c = hazeColor;
    depth = hazeVolumeDistance;
    density = hazeDensity;
}

void Sky::GetFog(Color& c, double& density, double& distance) const
{
    if (fogVolumeDistance != 0) {
        Color fogColor;
        double fogDensity, fogStart, fogEnd;
        Renderer::GetInstance()->GetFog(fogDensity, fogStart, fogEnd, fogColor);
        if (fogVolumeDistance < 0) fogDensity = 1.0;

        density = fogDensity;
        c = fogColor;
        distance = fogVolumeDistance;
    } else {
        if (hazeVolumeDistance != 0) {
            c = hazeColor;
            density = hazeDensity;
            distance = hazeVolumeDistance;
        } else {
            c = Color(1.0f, 1.0f, 1.0f);
            density = 0;
            distance = 1E20;
        }
    }
}

void Sky::ColorVertices()
{
    Color fogColor;
    double fogDensity, fogDistance;
    GetFog(fogColor, fogDensity, fogDistance);

    bool fogChanged = false;
    if (fogDistance != lastFogDistance || fogDensity != lastFogDensity) {
        lastFogDistance = fogDistance;
        lastFogDensity = fogDensity;
        fogChanged = true;
    }

    if (!lightingChanged && !fogChanged) return;

    const Vector3 zenith(0, 1, 0);

    for (int face = 0; face < NUM_FACES; face++) {
        if (!cubeFace[face]->LockBuffer()) continue;

        Vertex *verts = cubeFace[face]->GetVertices();

        if (verts) {
            int nVerts = rows * cols;

            for (int vert = 0; vert < nVerts; vert++) {
                Vector3f ptf = localVerts[face][vert];
                Vector3 pt(ptf.x, ptf.y, ptf.z);

                double x, y, Y;
                double xm, ym, Ym;

                double X, Z;

                PtToxyY(pt, x, y, Y);
                PtToxyYMoon(pt, xm, ym, Ym);

                Vector3 rgb(1,1,1);

                if (pt.y < 0) {
                    rgb = Vector3(0, 0, 0);
                } else {
                    Y *= 1000.0;
                    Ym *= 1000.0;

                    LuminanceMapper::DurandMapper(&x, &y, &Y);
                    LuminanceMapper::DurandMapper(&xm, &ym, &Ym);

                    X = x * (Y / y) + xm * (Ym / ym);
                    Z = (1.0 - x - y) * (Y / y) + (1.0 - xm - ym) * (Ym / ym);
                    Y += Ym;

                    Vector3 XYZ(X, Y, Z);

                    rgb = XYZ * XYZ2RGB_HDTV;

                    scaleDownToOne(rgb);
                }

                ApplyGamma(rgb);
                Color c(rgb.x, rgb.y, rgb.z, 1.0);

                //Fog
                if (fogDistance > 0 && fogDensity > 0) {
                    pt.Normalize();
                    double cosTheta = zenith.Dot(pt);
                    if (cosTheta > 0) {
                        double z = fabs(fogDistance) / cosTheta;
                        double f = exp(-(fogDensity * z));

                        c = (c * (float)f) + (fogColor * (1.0f-(float)f));
                    }
                }
                verts[vert].SetColor(c);

            }
        }
        cubeFace[face]->UnlockBuffer();
    }
}

ShaderHandle Sky::GetSkyShader() const
{
    bool disableToneMapping = false;
    Configuration::GetBoolValue("disable-tone-mapping", disableToneMapping);
    if (Atmosphere::GetHDREnabled()) disableToneMapping = true;

    ShaderHandle currentShader = 0;

    if (skyModel == HOSEK_WILKIE) {
        currentShader = disableToneMapping ? skyShaderHDRHW : skyShaderHW;
    } else {
        currentShader = disableToneMapping ? skyShaderHDR : skyShader;
    }

    return currentShader;
}

ShaderHandle Sky::GetStarShader() const
{
    if (stars) return stars->GetShader();

    return 0;
}

void Sky::ReloadShaders()
{
    LoadShaders();

    if (stars) {
        stars->ReloadShaders();
    }
}

void Sky::UpdateVertices()
{
    Timer t("Compute Sky");

    bool disableToneMapping = false;
    Configuration::GetBoolValue("disable-tone-mapping", disableToneMapping);
    if (Atmosphere::GetHDREnabled()) disableToneMapping = true;

    ShaderHandle currentShader = 0;

    if (skyModel == HOSEK_WILKIE) {
        currentShader = disableToneMapping ? skyShaderHDRHW : skyShaderHW;
    } else {
        currentShader = disableToneMapping ? skyShaderHDR : skyShader;
    }

    if (currentShader) {
        Vector3 sunPerez(sunx, suny, sunY);
        Vector3 zenithPerez(xZenith, yZenith, YZenith);
        Vector3 moonPerez(moonx, moony, moonY);
        Vector3 zenithMoonPerez(xMoon, yMoon, YMoon);

        Vector3 xPerezABC(Ax, Bx, Cx);
        Vector3 xPerezDE(Dx, Ex, 0);
        Vector3 yPerezABC(Ay, By, Cy);
        Vector3 yPerezDE(Dy, Ey, 0);
        Vector3 YPerezABC(AY, BY, CY);
        Vector3 YPerezDE(DY, EY, 0);

        if (skyModel == HOSEK_WILKIE) {
            Vector3 XHosekABC(hosekWilkieCoeffs[0][0], hosekWilkieCoeffs[0][1], hosekWilkieCoeffs[0][2]);
            Vector3 XHosekDEF(hosekWilkieCoeffs[0][3], hosekWilkieCoeffs[0][4], hosekWilkieCoeffs[0][5]);
            Vector3 XHosekGHI(hosekWilkieCoeffs[0][6], hosekWilkieCoeffs[0][7], hosekWilkieCoeffs[0][8]);

            Vector3 YHosekABC(hosekWilkieCoeffs[1][0], hosekWilkieCoeffs[1][1], hosekWilkieCoeffs[1][2]);
            Vector3 YHosekDEF(hosekWilkieCoeffs[1][3], hosekWilkieCoeffs[1][4], hosekWilkieCoeffs[1][5]);
            Vector3 YHosekGHI(hosekWilkieCoeffs[1][6], hosekWilkieCoeffs[1][7], hosekWilkieCoeffs[1][8]);

            Vector3 ZHosekABC(hosekWilkieCoeffs[2][0], hosekWilkieCoeffs[2][1], hosekWilkieCoeffs[2][2]);
            Vector3 ZHosekDEF(hosekWilkieCoeffs[2][3], hosekWilkieCoeffs[2][4], hosekWilkieCoeffs[2][5]);
            Vector4 ZHosekGHI(hosekWilkieCoeffs[2][6], hosekWilkieCoeffs[2][7], hosekWilkieCoeffs[2][8], isothermalEffect);

            Vector4 HosekRadiances(hosekWilkieRadiances[0], hosekWilkieRadiances[1], hosekWilkieRadiances[2], perezBlend);

            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_XHosekABC", XHosekABC);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_XHosekDEF", XHosekDEF);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_XHosekGHI", XHosekGHI);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_YHosekABC", YHosekABC);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_YHosekDEF", YHosekDEF);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_YHosekGHI", YHosekGHI);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_ZHosekABC", ZHosekABC);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_ZHosekDEF", ZHosekDEF);
            Renderer::GetInstance()->SetConstantVector4(currentShader, "sl_ZHosekGHI", ZHosekGHI);
            Renderer::GetInstance()->SetConstantVector4(currentShader, "sl_HosekRadiances", HosekRadiances);
        }

        double sfRod, sfCone;
        LuminanceMapper::GetLuminanceScales(&sfRod, &sfCone);
        Vector3 luminanceScales(sfRod, sfCone, 0);

        Vector4 kAndLdmax(LuminanceMapper::GetRodConeBlend(),
                          LuminanceMapper::GetMaxDisplayLuminance(), skyModel == PREETHAM ? oneOverGammaPreetham : oneOverGammaHosek,
                          1.0);

        Vector3 sunPos = ephemeris->GetSunPositionHorizon();
        sunPos.Normalize();

        Vector3 moonPos = ephemeris->GetMoonPositionHorizon();
        moonPos.Normalize();

        Vector4 fog;
        Color fogColor;
        double fogDensity, fogDistance;
        GetFog(fogColor, fogDensity, fogDistance);

        fog.x = fogColor.r;
        fog.y = fogColor.g;
        fog.z = fogColor.b;
        fog.w = fogDensity;

        Vector4 overcast(isOvercast ? 1 : 0, isOvercast ? overcastBlend : 0, overcastTransmissionAmbient,
                         fabs(fogDistance));

        if (simpleShader) {
            if (YZenith > YMoon) {
                Renderer::GetInstance()->SetConstantVector(currentShader, "sl_sunPos", sunPos);
                Renderer::GetInstance()->SetConstantVector(currentShader, "sl_sunPerez", sunPerez);
                Renderer::GetInstance()->SetConstantVector(currentShader, "sl_zenithPerez", zenithPerez);
            } else {
                Renderer::GetInstance()->SetConstantVector(currentShader, "sl_sunPos", moonPos);
                Renderer::GetInstance()->SetConstantVector(currentShader, "sl_sunPerez", moonPerez);
                Renderer::GetInstance()->SetConstantVector(currentShader, "sl_zenithPerez", zenithMoonPerez);
            }
        } else {
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_sunPos", sunPos);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_moonPos", moonPos);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_sunPerez", sunPerez);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_moonPerez", moonPerez);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_zenithMoonPerez", zenithMoonPerez);
            Renderer::GetInstance()->SetConstantVector(currentShader, "sl_zenithPerez", zenithPerez);
        }

        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_xPerezABC", xPerezABC);
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_xPerezDE", xPerezDE);
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_yPerezABC", yPerezABC);
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_yPerezDE", yPerezDE);
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_YPerezABC", YPerezABC);
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_YPerezDE", YPerezDE);
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_luminanceScales", luminanceScales);

        if (!disableToneMapping) {
            Renderer::GetInstance()->SetConstantVector4(currentShader, "sl_kAndLdmax", kAndLdmax);
        }

        Renderer::GetInstance()->SetConstantVector4(currentShader, "sl_overcast", overcast);
        Renderer::GetInstance()->SetConstantVector4(currentShader, "sl_fog", fog);

        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        Renderer::GetInstance()->SetConstantMatrix(currentShader, "sl_XYZtoRGB", XYZ2RGB4_HDTV);
    } else {
        ColorVertices();
    }
}

void Sky::SetGamma(double gamma)
{
    if (skyModel == PREETHAM) {
        oneOverGammaPreetham = 1.0 / gamma;
    } else {
        oneOverGammaHosek = 1.0 / gamma;
    }
}

double Sky::GetGamma() const
{
    return 1.0 / (skyModel == PREETHAM ? oneOverGammaPreetham : oneOverGammaHosek);
}

bool Sky::Draw(int pass, double skyboxSize, bool applicationFog, bool infraRed, bool drawStars, bool clearDepth, bool drawSunAndMoon)
{
    Renderer *renderer = Renderer::GetInstance();

    isInfraRed = infraRed;
    isApplicationFog = applicationFog;

    bool disableToneMapping = Atmosphere::GetHDREnabled();
    if (!disableToneMapping) {
        Configuration::GetBoolValue("disable-tone-mapping", disableToneMapping);
    }
    LuminanceMapper::EnableToneMapping(!disableToneMapping);

    Timer t("Draw sky");
    if (pass == 0) {
        UpdateVertices();

        if (renderer) {
            bool hadFog = renderer->GetFogEnabled();
            renderer->EnableFog(false);

            renderer->PushTransforms();

            float zmin = 0,zmax = 1.0f;
            if( clearDepth ) {
                renderer->EnableDepthReads(false);
            } else {
                renderer->GetDepthRange(zmin, zmax);
                renderer->SetDepthRange(zmax, zmax);
                renderer->EnableDepthReads(true);
            }

            renderer->EnableDepthWrites(false);
            renderer->EnableBackfaceCulling(false);
            renderer->EnableTexture2D(false);
            renderer->EnableLighting(false);
            renderer->DisableBlending();
            renderer->DisableTexture(0);
            renderer->DisableTexture(1);

            Matrix4 modelview, proj, modelviewProj;

            renderer->GetModelviewMatrix(&modelview);

            // Blow away any translation
            for (int row = 0; row < 3; row++) {
                modelview.elem[row][3] = 0;
            }
            modelview.elem[3][3] = 1;

            Matrix4 scale;
            // Now scale if we have a certain size we're after
            if (skyboxSize > 0) {
                double s = skyboxSize / cubeDimension;
                scale.elem[0][0] = s;
                scale.elem[1][1] = s;
                scale.elem[2][2] = s;
            }

            modelview = modelview * scale;

            Matrix4 geocentricModelview = modelview;

            // Account for which way is up...
            modelview = modelview * renderer->GetInverseBasis4x4();

            if (!geocentricMode) {
                Matrix4 billboardMatrix = renderer->GetBillboardMatrix();
                billboardMatrix = renderer->GetBasis4x4() * billboardMatrix;
                Billboard::OverrideBillboardMatrix(billboardMatrix);
            }

            renderer->GetProjectionMatrix(&proj);

            modelviewProj = proj * modelview;

            renderer->SetModelviewMatrix(modelview);

            ShaderHandle currentShader = 0;
            if (skyModel == PREETHAM) {
                currentShader = disableToneMapping ? skyShaderHDR : skyShader;
            } else {
                currentShader = disableToneMapping ? skyShaderHDRHW : skyShaderHW;
            }

            renderer->SetConstantMatrix(currentShader, "sl_modelViewProj", modelviewProj);
            renderer->SetConstantMatrix(currentShader, "sl_modelView", modelview);

            if (infraRed) {
                renderer->ClearScreen(Color(0,0,0));
            } else {
                renderer->BindShader(currentShader, VERTEX_PROGRAM);

                for (int face = 0; face < NUM_FACES; face++) {
                    renderer->DrawStrip(cubeFace[face]->GetHandle(), indexBuffer->GetHandle(),
                                        0, nIndices, cubeFace[face]->GetNumVertices());
                }

                renderer->UnbindShader(VERTEX_PROGRAM);
            }

            double originalFog = Billboard::GetFogDensity();
            Billboard::SetFogDensity(1E-9);

            Matrix4 savedModelview = modelview;
            if (geocentricMode) {
                renderer->SetModelviewMatrix(geocentricModelview);
            }

            if (drawStars && stars && (YZenith < duskZenithLuminance) &&
                    !(isOvercast && overcastBlend > 0) && !infraRed ) {
                stars->Draw(this);
            }

            if (drawSunAndMoon) {
                renderer->EnableBlending(SRCCOLOR, ONE);
                DrawSun(infraRed);

                if (!infraRed) {
                    renderer->EnableBlending(SRCCOLOR, ONE);
                    DrawMoon();
                }
            }

            renderer->SetModelviewMatrix(savedModelview);

            Billboard::SetFogDensity(originalFog);

            if( clearDepth ) {
                renderer->EnableDepthReads(true);
            } else {
                renderer->SetDepthRange( zmin, zmax );
            }

            renderer->EnableDepthWrites(true);
            //renderer->EnableBackfaceCulling(true);
            renderer->EnableLighting(false);

            renderer->PopTransforms();

            if (!geocentricMode) {
                Billboard::RestoreBillboardMatrix();
            }

            if (hadFog) renderer->EnableFog(true);
        }

    }

    return true;
}

void Sky::ComputeSun(double altitude)
{
    Vector3 sunPos = ephemeris->GetSunPositionHorizon();
    sunPos.Normalize();
    double cosZenith = sunPos.y;

    if (lastSunT != T || lastSunZenith != cosZenith || lastIsOvercast != isOvercast || lastOvercastBlend != overcastBlend) {
        lastSunT = T;
        lastSunZenith = cosZenith;
        lastIsOvercast = isOvercast;
        lastOvercastBlend = overcastBlend;

        lightingChanged = true;

        double XBoost = 0, YBoost = 0, ZBoost = 0;
        int boostExp = 3;
        double solarAltitudeOffset = 0;
        Configuration::GetDoubleValue("sunset-X-boost", XBoost);
        Configuration::GetDoubleValue("sunset-Y-boost", YBoost);
        Configuration::GetDoubleValue("sunset-Z-boost", ZBoost);
        Configuration::GetIntValue("sunset-boost-exponent", boostExp);
        Configuration::GetDoubleValue("solar-altitude-offset", solarAltitudeOffset);

        double solarAltitude = DEGREES(asin(sunPos.y));

        solarAltitude += solarAltitudeOffset;

        if (solarAltitude > 1.0) {
            perezBlend = 0;
        } else if (solarAltitude < 0) {
            perezBlend = 1.0f;
        } else {
            perezBlend = 1.0f - (float)solarAltitude;
        }

        double zenithAngle = acos(cosZenith) - RADIANS(solarAltitudeOffset);
        const double switchAngle = RADIANS(1.0);

        if (PI * 0.5 - zenithAngle > switchAngle) {
            sunAboveHorizon = true;

            Spectrum solarDirect, solarScattered;
            sunSpectrum.ApplyAtmosphericTransmittance(zenithAngle, cosZenith, T, altitude,
                    solarDirect, solarScattered);

            sunTransmittedLuminance = solarDirect.ToXYZ();
            sunScatteredLuminance = solarScattered.ToXYZ();

            if (altitude < 1000) {
                sunTransmittedLuminanceSeaLevel = sunTransmittedLuminance;
                sunScatteredLuminanceSeaLevel = sunScatteredLuminance;
            } else {
                Spectrum solarDirectSeaLevel, solarScatteredSeaLevel;
                sunSpectrum.ApplyAtmosphericTransmittance(zenithAngle, cosZenith, T, 0,
                        solarDirectSeaLevel, solarScatteredSeaLevel);

                sunTransmittedLuminanceSeaLevel = solarDirectSeaLevel.ToXYZ();
                sunScatteredLuminanceSeaLevel = solarScatteredSeaLevel.ToXYZ();
            }

            // Apply sunset color tweaks
            double alpha = zenithAngle / (PI * 0.5);
            for (int i = 0; i < boostExp; i++) alpha *= alpha;
            sunScatteredLuminance.x *= 1.0 + alpha * XBoost;
            sunScatteredLuminance.y *= 1.0 + alpha * YBoost;
            sunScatteredLuminance.z *= 1.0 + alpha * ZBoost;

            sunScatteredLuminanceSeaLevel.x *= 1.0 + alpha * XBoost;
            sunScatteredLuminanceSeaLevel.y *= 1.0 + alpha * YBoost;
            sunScatteredLuminanceSeaLevel.z *= 1.0 + alpha * ZBoost;
        } else {
            sunAboveHorizon = false;

            // In twilight conditions, we lookup luminance based on experimental results
            // on cloudless nights.

            int lower = (int)floor(solarAltitude);
            int higher = (int)ceil(solarAltitude);

            double alpha = solarAltitude - lower;

            double a = twilightLuminance[lower];
            double b = twilightLuminance[higher];

            // Blend light from sunset
            Spectrum solarDirect, solarScattered;
            double zenithAngle = PI * 0.5 - switchAngle;
            sunSpectrum.ApplyAtmosphericTransmittance(zenithAngle, cos(zenithAngle), T, altitude,
                    solarDirect, solarScattered);
            sunTransmittedLuminance = solarDirect.ToXYZ();
            sunScatteredLuminance = solarScattered.ToXYZ();

            if (altitude < 1000) {
                sunTransmittedLuminanceSeaLevel = sunTransmittedLuminance;
                sunScatteredLuminanceSeaLevel = sunScatteredLuminance;
            } else {
                Spectrum solarDirectSeaLevel, solarScatteredSeaLevel;
                sunSpectrum.ApplyAtmosphericTransmittance(zenithAngle, cos(zenithAngle), T, 0,
                        solarDirectSeaLevel, solarScatteredSeaLevel);
                sunTransmittedLuminanceSeaLevel = solarDirectSeaLevel.ToXYZ();
                sunScatteredLuminanceSeaLevel = solarScatteredSeaLevel.ToXYZ();
            }

            double Y = (1 - alpha) * a + alpha * b; // luminance per lookup table
            double x = 0.25;
            double y = 0.25;
            double minDirectional = 0.0;

            Configuration::GetDoubleValue("twilight-chromaticity-x", x);
            Configuration::GetDoubleValue("twilight-chromaticity-y", y);
            Configuration::GetDoubleValue("twilight-directional-light", minDirectional);

            double X = x * (Y / y);
            double Z = (1.0 - x - y) * (Y / y);

            alpha = -solarAltitude / 2.0;
            if (alpha > 1.0) alpha = 1.0;
            if (alpha < 0) alpha = 0;
            alpha = alpha * alpha;

            sunTransmittedLuminance = sunTransmittedLuminance * Y * minDirectional * alpha
                                      + sunTransmittedLuminance * (1.0 - alpha);
            sunScatteredLuminance = Vector3(X, Y, Z) * alpha + sunScatteredLuminance * (1.0 - alpha);

            sunTransmittedLuminanceSeaLevel = sunTransmittedLuminanceSeaLevel * Y * minDirectional * alpha
                                              + sunTransmittedLuminanceSeaLevel * (1.0 - alpha);
            sunScatteredLuminanceSeaLevel = Vector3(X, Y, Z) * alpha + sunScatteredLuminanceSeaLevel * (1.0 - alpha);

            // Apply sunset color tweaks
            sunScatteredLuminance.x *= 1.0 + XBoost;
            sunScatteredLuminance.y *= 1.0 + YBoost;
            sunScatteredLuminance.z *= 1.0 + ZBoost;

            sunScatteredLuminanceSeaLevel.x *= 1.0 + XBoost;
            sunScatteredLuminanceSeaLevel.y *= 1.0 + YBoost;
            sunScatteredLuminanceSeaLevel.z *= 1.0 + ZBoost;
        }

        if (isOvercast) {
            sunTransmittedLuminance = (sunTransmittedLuminance * (overcastBlend * overcastTransmissionDirect))
                                      + (sunTransmittedLuminance * (1.0 - overcastBlend));
            sunScatteredLuminance = (sunScatteredLuminance * (overcastBlend * overcastTransmissionAmbient))
                                    + (sunScatteredLuminance * (1.0 - overcastBlend));
            sunTransmittedLuminanceSeaLevel = (sunTransmittedLuminanceSeaLevel * (overcastBlend * overcastTransmissionDirect))
                                              + (sunTransmittedLuminanceSeaLevel * (1.0 - overcastBlend));
            sunScatteredLuminanceSeaLevel = (sunScatteredLuminanceSeaLevel * (overcastBlend * overcastTransmissionAmbient))
                                            + (sunScatteredLuminanceSeaLevel * (1.0 - overcastBlend));
        }

        double sunTransmissionScale = 1.0, sunScatteredScale = 1.0;
        Configuration::GetDoubleValue("sun-transmission-scale", sunTransmissionScale);
        Configuration::GetDoubleValue("sun-scattered-scale", sunScatteredScale);
        sunTransmittedLuminance = sunTransmittedLuminance * sunTransmissionScale;
        sunScatteredLuminance = sunScatteredLuminance * sunScatteredScale;
        sunTransmittedLuminanceSeaLevel = sunTransmittedLuminanceSeaLevel * sunTransmissionScale;
        sunScatteredLuminanceSeaLevel = sunScatteredLuminanceSeaLevel * sunScatteredScale;
    }
}

void Sky::ComputeMoon(double altitude)
{
    Vector3 moonPos = ephemeris->GetMoonPositionHorizon();
    moonPos.Normalize();

    double cosZenith = moonPos.y;
    double zenith = acos(cosZenith);

    if (lastMoonT != T || lastMoonZenith != zenith) {
        lastMoonT = T;
        lastMoonZenith = zenith;

        lightingChanged = true;

        Spectrum moonSpectrumEarthDirect, moonSpectrumEarthScattered;
        lunarSpectrum.ApplyAtmosphericTransmittance(zenith, cosZenith, T, altitude,
                moonSpectrumEarthDirect, moonSpectrumEarthScattered);
        double moonLuminance = MoonLuminance();
        moonTransmittedLuminance = moonSpectrumEarthDirect.ToXYZ() * moonLuminance;
        moonScatteredLuminance = moonSpectrumEarthScattered.ToXYZ() * moonLuminance;

        Spectrum moonSpectrumEarthDirectSeaLevel, moonSpectrumEarthScatteredSeaLevel;
        lunarSpectrum.ApplyAtmosphericTransmittance(zenith, cosZenith, T, 0,
                moonSpectrumEarthDirectSeaLevel, moonSpectrumEarthScatteredSeaLevel);

        moonTransmittedLuminance = moonSpectrumEarthDirect.ToXYZ() * moonLuminance;
        moonScatteredLuminance = moonSpectrumEarthScattered.ToXYZ() * moonLuminance;
        moonTransmittedLuminanceSeaLevel = moonSpectrumEarthDirectSeaLevel.ToXYZ() * moonLuminance;
        moonScatteredLuminanceSeaLevel = moonSpectrumEarthScatteredSeaLevel.ToXYZ() * moonLuminance;

        if (isOvercast) {
            moonTransmittedLuminance = (moonTransmittedLuminance * (overcastBlend * overcastTransmissionDirect))
                                       + (moonTransmittedLuminance * (1.0 - overcastBlend));
            moonScatteredLuminance = (moonScatteredLuminance * (overcastBlend * overcastTransmissionAmbient))
                                     + (moonScatteredLuminance * (1.0 - overcastBlend));
            moonTransmittedLuminanceSeaLevel = (moonTransmittedLuminanceSeaLevel * (overcastBlend * overcastTransmissionDirect))
                                               + (moonTransmittedLuminanceSeaLevel * (1.0 - overcastBlend));
            moonScatteredLuminanceSeaLevel = (moonScatteredLuminanceSeaLevel * (overcastBlend * overcastTransmissionAmbient))
                                             + (moonScatteredLuminanceSeaLevel * (1.0 - overcastBlend));
        }

        double moonTransmissionScale = 1.0, moonScatteredScale = 1.0;
        Configuration::GetDoubleValue("moon-transmission-scale", moonTransmissionScale);
        Configuration::GetDoubleValue("moon-scattered-scale", moonScatteredScale);
        moonTransmittedLuminance = moonTransmittedLuminance * moonTransmissionScale;
        moonScatteredLuminance = moonScatteredLuminance * moonScatteredScale;
        moonTransmittedLuminanceSeaLevel = moonTransmittedLuminanceSeaLevel * moonTransmissionScale;
        moonScatteredLuminanceSeaLevel = moonScatteredLuminanceSeaLevel * moonScatteredScale;
    }
}

void Sky::DrawSun(bool infraRed)
{
    Timer t("SUN: Draw sun");

    Configuration::GetDoubleValue("sun-width-degrees", sunWidth);

    Vector3 sunPosHorizon = ephemeris->GetSunPositionHorizon();
    sunPosHorizon.Normalize();

    sunAltitude = DEGREES(asin(sunPosHorizon.y));

    Vector3 sunPosGeo = ephemeris->GetSunPositionGeographic();

    Vector3 sunPos = geocentricMode ? sunPosGeo : sunPosHorizon;

    sunPos.Normalize();
    sunPos = sunPos * halfDim;

    const double discPer = (256.0 - (57.0 * 2.0)) / 256.0;
    const double sunDiscSize = sunWidth * (1.0 / discPer);

    if (!drawSunBelowHorizon && !geocentricMode) {
        if (sunAltitude < (DEGREES(yOffsetAngle) - sunDiscSize)) return;
    }

    if (useGlowSun) {
        if (infraRed) {
            sunGlowBillboard->SetColor(Color(1,1,1));
        }

        Renderer::GetInstance()->EnableBlending(SRCCOLOR, ONE);
        sunGlowBillboard->SetWorldPosition(sunPos);
        sunGlowBillboard->SetAngularSize(infraRed ? glowSizeSunInfrared : glowSizeSun);
        sunGlowBillboard->Draw(glowTexture, 0, sunAlpha, false);
    }

    if (sunGlare && glareMgr) {
        if (glareMgr->EnableShaders()) {
            sunGlare->SetWorldPosition(sunPos);
            // Minimum sun luminance enforced so we get glares on the sun when viewed from space.
            sunGlare->SetIntensity(sunTransmittedLuminance.y > 0.25 ? sunTransmittedLuminance.y : 0.25);
            sunGlare->Draw(0);
            glareMgr->DisableShaders();
        }
    }

    double x = 0.312727, y = 0.329024;
    double dnits = sunTransmittedLuminance.y;
    LuminanceMapper::DurandMapper(&x, &y, &dnits);
    float intensity = (float)dnits * (float)glowIntensitySun;

    if (Atmosphere::GetHDREnabled()) {
        intensity /= 1000.0f;
    } else {
        if (intensity > 1.0) intensity = 1.0;
        if (intensity < 0) intensity = 0;
    }

    if (sunHasColor) {
        Vector3 sunXYZ = sunTransmittedLuminance;
        Vector3 sunColorV = sunXYZ * XYZ2RGB_CIE;
        scaleDownToOne(sunColorV);

        sunColorV.Normalize();

        if (sunColorV.x < 0) sunColorV.x = 0;
        if (sunColorV.y < 0) sunColorV.y = 0;
        if (sunColorV.z < 0) sunColorV.z = 0;

        ApplyGamma(sunColorV);

        sunColorV = sunColorV * isothermalEffect + Vector3(1,1,1) * (1.0 - isothermalEffect);

        Color sunColor(sunColorV.x, sunColorV.y, sunColorV.z, sunAlpha);

        sunBillboard->SetColor(sunColor);
        sunGlowBillboard->SetColor(sunColor * intensity);
    } else {
        Color sunColor(intensity, intensity, intensity, (float)sunAlpha);
        sunGlowBillboard->SetColor(sunColor);
    }

    Renderer::GetInstance()->EnableBlending(SRCCOLOR, ONE);
    sunBillboard->SetWorldPosition(sunPos);
    sunBillboard->SetAngularSize(sunDiscSize);
    sunBillboard->Draw(sunTexture, 0, sunAlpha, false);
}

void Sky::DrawMoon()
{
    Timer t("Draw moon");

    Configuration::GetDoubleValue("moon-width-degrees", moonWidth);

    const double discPer = (256.0 - (3.0 * 2.0)) / 256.0;
    const double moonDiscSize = moonWidth * (1.0 / discPer);

    Vector3 moonPos = ephemeris->GetMoonPositionHorizon();
    if (geocentricMode) moonPos = ephemeris->GetMoonPositionGeographic();

    moonPos.Normalize();
    moonPos = moonPos * halfDim;

    int moonDay = (int)(floor((ephemeris->GetMoonPhaseAngle() / (2.0 * PI)) * 30.0));

    double x = 0.312727, y = 0.329024;
    double dnits = moonTransmittedLuminance.y;
    LuminanceMapper::DurandMapper(&x, &y, &dnits);
    float intensity = (float)dnits * (float)(sin(ephemeris->GetMoonPhaseAngle() * 0.5));

    if (Atmosphere::GetHDREnabled()) {
        intensity /= 1000.0f;
    } else {
        if (intensity > 1.0) intensity = 1.0;
        if (intensity < 0) intensity = 0;
    }

    if (useGlowMoon && !geocentricMode) {
        Renderer::GetInstance()->EnableBlending(SRCCOLOR, ONE);
        moonGlowBillboard->SetWorldPosition(moonPos);
        moonGlowBillboard->SetAngularSize(glowSizeMoon);
        moonGlowBillboard->Draw(glowTexture, 0, moonAlpha, false);
    }

    if (moonGlare && glareMgr) {
        moonGlare->SetIntensity(moonTransmittedLuminance.y > 0.25 ? moonTransmittedLuminance.y : 0.25);

        if (glareMgr->EnableShaders()) {
            moonGlare->SetWorldPosition(moonPos);
            moonGlare->Draw(0);
            glareMgr->DisableShaders();
        }
    }

    Renderer::GetInstance()->EnableBlending(SRCCOLOR, ONE);

    if (moonDay >= 0 && moonDay < 30) {
        if (moonBillboard[moonDay]) {
            moonBillboard[moonDay]->SetWorldPosition(moonPos);

            if (moonHasColor) {
                Vector3 moonXYZ = moonTransmittedLuminance;
                Vector3 moonColor = moonXYZ * XYZ2RGB_CIE;
                scaleDownToOne(moonColor);

                if (moonColor.x < 0) moonColor.x = 0;
                if (moonColor.y < 0) moonColor.y = 0;
                if (moonColor.z < 0) moonColor.z = 0;

                ApplyGamma(moonColor);

                double fakeHDR = sunAltitude >= 0 ? daylightMoonDimming : 1.0;
                if (sunAltitude > -20.0 && sunAltitude < 0) {
                    double alpha = -sunAltitude / 20.0;
                    if (alpha > 1.0) alpha = 1.0;
                    fakeHDR = alpha + daylightMoonDimming * (1.0 - alpha);
                }
                moonColor = moonColor * moonBrightness * fakeHDR;
                moonColor = moonColor * isothermalEffect + Vector3(1,1,1) * (1.0 - isothermalEffect);
                Color scaledMoonColor(moonColor.x, moonColor.y, moonColor.z, moonAlpha);
                moonBillboard[moonDay]->SetColor(scaledMoonColor);
                scaledMoonColor = scaledMoonColor * intensity * (float)glowIntensityMoon;
                moonGlowBillboard->SetColor(scaledMoonColor);

            } else {
                intensity *= (float)glowIntensityMoon;
                moonGlowBillboard->SetColor(Color(intensity, intensity, intensity, (float)moonAlpha));
            }

            moonBillboard[moonDay]->SetAngularSize(moonDiscSize);

            // Use a normal camera-facing billboard.
            Billboard::OverrideBillboardMatrix(ComputeMoonBillboard(moonPos));

            moonBillboard[moonDay]->Draw(moonTexture[moonDay], 0, moonAlpha, false);

            Billboard::RestoreBillboardMatrix();
        }
    }
}

Matrix4 Sky::ComputeMoonBillboard(const Vector3& moonPos)
{
    Vector3 sunPosHorizon = ephemeris->GetSunPositionHorizon();
    Vector3 sunPosGeo = ephemeris->GetSunPositionGeographic();
    Vector3 sunPos = geocentricMode ? sunPosGeo : sunPosHorizon;
    sunPos.Normalize();
    sunPos = sunPos * halfDim;

    Matrix4 billboard;
    Renderer *ren = Renderer::GetInstance();

    Vector3 Normal = moonPos * -1.0;
    Normal.Normalize();

    Vector3 Right =  moonPos - sunPos;
    Right.Normalize();

    if (ephemeris->GetMoonPhaseAngle() > PI) {
        Right = Right * -1.0;
    }

    Vector3 Up = Right.Cross(Normal);
    Up.Normalize();
    Right = Up.Cross(Normal);
    Right.Normalize();

    if (ren->GetIsRightHanded()) {
        billboard = Matrix4(Right.x, Up.x, Normal.x, 0,
                            Right.y, Up.y, Normal.y, 0,
                            Right.z, Up.z, Normal.z, 0,
                            0, 0, 0, 1);
    } else {
        billboard = Matrix4(Right.x, Up.x, -Normal.x, 0,
                            Right.y, Up.y, -Normal.y, 0,
                            Right.z, Up.z, -Normal.z, 0,
                            0, 0, 0, 1);
    }

    return billboard;
}

double Sky::AngleBetween(const Vector3& v1, const Vector3& v2) const
{
    Vector3 a = v1;
    a.Normalize();
    Vector3 b = v2;
    b.Normalize();

    double dot = a.Dot(b);

    return acos(dot);
}

void Sky::UpdateHosekWilkieCoefficients()
{
    unsigned int i;
    double albedo = 0;
    Configuration::GetDoubleValue("ground-albedo-hosek", albedo);

    double hosekWilkieRadianceScale = 1.0;
    Configuration::GetDoubleValue("hosek-wilkie-radiance-scale", hosekWilkieRadianceScale);

    for (int channel = 0; channel < 3; channel++) {
        double *dataset = datasetsXYZ[channel];

        double  * elev_matrix;

        int     int_turbidity = (int)T;
        double  turbidity_rem = T - (double)int_turbidity;

        Vector3 sunPos = ephemeris->GetSunPositionHorizon();
        sunPos.Normalize();

        double solar_elevation = asin(sunPos.y);
        if (solar_elevation < 0) solar_elevation = 0;

        solar_elevation = pow(solar_elevation / (PI / 2.0), (1.0 / 3.0));

        // alb 0 low turb

        elev_matrix = dataset + ( 9 * 6 * (int_turbidity-1) );


        for( i = 0; i < 9; ++i ) {
            //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
            hosekWilkieCoeffs[channel][i] =
                (1.0-albedo) * (1.0 - turbidity_rem)
                * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  +
                    5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
                    10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
                    10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
                    5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
                    pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
        }

        // alb 1 low turb
        elev_matrix = dataset + (9*6*10 + 9*6*(int_turbidity-1));
        for( i = 0; i < 9; ++i) {
            //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
            hosekWilkieCoeffs[channel][i] +=
                (albedo) * (1.0 - turbidity_rem)
                * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  +
                    5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
                    10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
                    10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
                    5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
                    pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
        }

        if(int_turbidity < 10) {
            // alb 0 high turb
            elev_matrix = dataset + (9*6*(int_turbidity));
            for( i = 0; i < 9; ++i) {
                //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
                hosekWilkieCoeffs[channel][i] +=
                    (1.0-albedo) * (turbidity_rem)
                    * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  +
                        5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
                        10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
                        10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
                        5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
                        pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
            }

            // alb 1 high turb
            elev_matrix = dataset + (9*6*10 + 9*6*(int_turbidity));
            for( i = 0; i < 9; ++i) {
                //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
                hosekWilkieCoeffs[channel][i] +=
                    (albedo) * (turbidity_rem)
                    * ( pow(1.0-solar_elevation, 5.0) * elev_matrix[i]  +
                        5.0  * pow(1.0-solar_elevation, 4.0) * solar_elevation * elev_matrix[i+9] +
                        10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[i+18] +
                        10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[i+27] +
                        5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[i+36] +
                        pow(solar_elevation, 5.0)  * elev_matrix[i+45]);
            }
        }

        dataset = datasetsXYZRad[channel];
        double res;

        // alb 0 low turb
        elev_matrix = dataset + (6*(int_turbidity-1));
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        res = (1.0-albedo) * (1.0 - turbidity_rem) *
              ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
                5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
                10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
                10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
                5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
                pow(solar_elevation, 5.0) * elev_matrix[5]);

        // alb 1 low turb
        elev_matrix = dataset + (6*10 + 6*(int_turbidity-1));
        //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
        res += (albedo) * (1.0 - turbidity_rem) *
               ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
                 5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
                 10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
                 10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
                 5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
                 pow(solar_elevation, 5.0) * elev_matrix[5]);

        if(int_turbidity < 10) {
            // alb 0 high turb
            elev_matrix = dataset + (6*(int_turbidity));
            //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
            res += (1.0-albedo) * (turbidity_rem) *
                   ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
                     5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
                     10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
                     10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
                     5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
                     pow(solar_elevation, 5.0) * elev_matrix[5]);

            // alb 1 high turb
            elev_matrix = dataset + (6*10 + 6*(int_turbidity));
            //(1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
            res += (albedo) * (turbidity_rem) *
                   ( pow(1.0-solar_elevation, 5.0) * elev_matrix[0] +
                     5.0*pow(1.0-solar_elevation, 4.0)*solar_elevation * elev_matrix[1] +
                     10.0*pow(1.0-solar_elevation, 3.0)*pow(solar_elevation, 2.0) * elev_matrix[2] +
                     10.0*pow(1.0-solar_elevation, 2.0)*pow(solar_elevation, 3.0) * elev_matrix[3] +
                     5.0*(1.0-solar_elevation)*pow(solar_elevation, 4.0) * elev_matrix[4] +
                     pow(solar_elevation, 5.0) * elev_matrix[5]);
        }

        hosekWilkieRadiances[channel] = res * hosekWilkieRadianceScale;
    }
}

void Sky::UpdatePerezCoefficients()
{
    double aScale = 1.0, bScale = 1.0, cScale = 1.0, dScale = 1.0, eScale = 1.0;
    Configuration::GetDoubleValue("perez-a-scale", aScale);
    Configuration::GetDoubleValue("perez-b-scale", bScale);
    Configuration::GetDoubleValue("perez-c-scale", cScale);
    Configuration::GetDoubleValue("perez-d-scale", dScale);
    Configuration::GetDoubleValue("perez-e-scale", eScale);

    AY = ( 0.1787 * T - 1.4630) * aScale;
    BY = (-0.3554 * T + 0.4275) * bScale;
    CY = (-0.0227 * T + 5.3251) * cScale;
    DY = ( 0.1206 * T - 2.5771) * dScale;
    EY = (-0.0670 * T + 0.3702) * eScale;

    Ax = -0.0193 * T - 0.2592;
    Bx = -0.0665 * T + 0.0008;
    Cx = -0.0004 * T + 0.2125;
    Dx = -0.0641 * T - 0.8989;
    Ex = -0.0033 * T + 0.0452;

    Ay = -0.0167 * T - 0.2608;
    By = -0.0950 * T + 0.0092;
    Cy = -0.0079 * T + 0.2102;
    Dy = -0.0441 * T - 1.6537;
    Ey = -0.0109 * T + 0.0529;
}

void Sky::InitTwilightZenithLuminanceLookup()
{
    twilightLuminance[5] = 2200;
    twilightLuminance[4] = 1800;
    twilightLuminance[3] = 1400;
    twilightLuminance[2] = 1200;
    twilightLuminance[1] = 710;
    twilightLuminance[0] = 400;
    twilightLuminance[-1] = 190;
    twilightLuminance[-2] = 77;
    twilightLuminance[-3] = 28;
    twilightLuminance[-4] = 9.4;
    twilightLuminance[-5] = 2.9;
    twilightLuminance[-6] = 0.9;
    twilightLuminance[-7] = 0.3;
    twilightLuminance[-8] = 0.11;
    twilightLuminance[-9] = 0.047;
    twilightLuminance[-10] = 0.021;
    twilightLuminance[-11] = 0.0092;
    twilightLuminance[-12] = 0.0031;
    twilightLuminance[-13] = 0.0022;
    twilightLuminance[-14] = 0.0019;
    twilightLuminance[-15] = 0.0018;
    twilightLuminance[-16] = 0.0018;

    double twilightScale = 0.1;
    Configuration::GetDoubleValue("twilight-scale", twilightScale);

    // Convert from asb to cd / m2
    SL_MAP(int, double) ::iterator it;
    for (it = twilightLuminance.begin(); it != twilightLuminance.end(); it++) {
        (*it).second = ((*it).second / PI) * twilightScale;
    }
}

void Sky::UpdateZenith(double altitude)
{
    Vector3 sunPos = ephemeris->GetSunPositionHorizon();
    sunPos.Normalize();

    const Vector3 zenithPos(0, 1, 0);
    thetaS = AngleBetween(zenithPos, sunPos);

    Vector3 moonPos = ephemeris->GetMoonPositionHorizon();
    moonPos.Normalize();
    thetaM = AngleBetween(zenithPos, moonPos);

    double den = sunScatteredLuminance.x + sunScatteredLuminance.y + sunScatteredLuminance.z;
    double denMoon = moonScatteredLuminance.x + moonScatteredLuminance.y + moonScatteredLuminance.z;

    xZenith = yZenith = xMoon = yMoon = 0.2;

    if (den) {
        xZenith = sunScatteredLuminance.x / den;
        yZenith = sunScatteredLuminance.y / den;
    }

    /*
        // Zenith chromaticity from "A Practical Analytic Model for Daylight"
        double theta3 = thetaS * thetaS * thetaS;
        double theta2 = thetaS * thetaS;
        double T2 = T * T;
        xZenith =
       ( 0.00165 * theta3 - 0.00375 * theta2 + 0.00209 * thetaS + 0.0)     * T2 +
       (-0.02903 * theta3 + 0.06377 * theta2 - 0.03202 * thetaS + 0.00394) * T +
       ( 0.11693 * theta3 - 0.21196 * theta2 + 0.06052 * thetaS + 0.25886);

         yZenith =
       ( 0.00275 * theta3 - 0.00610 * theta2 + 0.00317 * thetaS + 0.0)     * T2 +
       (-0.04214 * theta3 + 0.08970 * theta2 - 0.04153 * thetaS + 0.00516) * T +
       ( 0.15346 * theta3 - 0.26756 * theta2 + 0.06670 * thetaS + 0.26688);
     */

    if (denMoon) {
        xMoon = moonScatteredLuminance.x / denMoon;
        yMoon = moonScatteredLuminance.y / denMoon;
    }

    static double moonScale = 0.1;
    static bool first1 = true;
    if ( first1 ) {
        first1 = false;
        Configuration::GetDoubleValue("moon-brightness-scale", moonScale);
    }
    YMoon = moonScatteredLuminance.y * 0.001 * moonScale;

    // Assume that our own scattered sunlight calculation is the zenith luminance.
    YZenith = sunScatteredLuminance.y * 0.001 + NightSkyLuminance();

    // Account for high altitude. As you lose atmosphere, less scattering occurs.
    static double H = 8435.0;
    Configuration::GetDoubleValue("atmosphere-scale-height-meters", H);
    H *= Atmosphere::GetUnitScale();

    isothermalEffect = exp(-(altitude / H));
    if (isothermalEffect < 0) isothermalEffect = 0;
    if (isothermalEffect > 1.0) isothermalEffect = 1.0;
    YZenith *= isothermalEffect;
    YMoon *= isothermalEffect;

    // Alternate approaches:

// Zenith luminance from "A Practical Analytic Model for Daylight" (Preeham, Shirley Smits)
// double chiSun = (4.0/9.0 - T / 120.0) * (PI - 2 * thetaS);
// YZenith = (4.053 * T - 4.9710) * tan(chiSun) - 0.2155 * T + 2.4192 + NightSkyLuminance();

// Zenith luminance from "Sky Luminance Distribution Model for Simulation of Daylit
// Environment" (Igawa, Nakamura, Matsuura)
    /*
       double Ys = (PI * 0.5) - thetaS;

       double A = 18.373 * Ys + 9.955;
       double B = -52.013 * Ys - 37.766;
       double C = 46.572 * Ys + 59.352;
       double D = 1.691 * Ys * Ys - 16.498 * Ys - 48.670;
       double E = 1.124 * Ys + 19.738;
       double F = 1.170 * log(Ys) + 6.369;

       const double N = 1.0; // normalized global illuminance
       YZenith = exp(A * N * N * N * N * N +
            B * N * N * N * N +
         C * N * N * N +
         D * N * N +
         E * N +
         F) * 0.001 + NightSkyLuminance(); */

}

static inline void clamp(Vector3 *v)
{
    if (v->x > 1.0) v->x = 1.0;
    if (v->y > 1.0) v->y = 1.0;
    if (v->z > 1.0) v->z = 1.0;
}

Vector3 Sky::GetSunOrMoonPosition() const
{
    if (ephemeris) {
        if (sunTransmittedLuminance.SquaredLength() > moonTransmittedLuminance.SquaredLength()) {
            return ephemeris->GetSunPositionHorizon();
        } else {
            return ephemeris->GetMoonPositionHorizon();
        }
    } else {
        return Vector3(0,1,0);
    }
}

Vector3 Sky::GetSunOrMoonPositionGeographic() const
{
    if (ephemeris) {
        if (sunTransmittedLuminance.SquaredLength() > moonTransmittedLuminance.SquaredLength()) {
            return ephemeris->GetSunPositionGeographic();
        } else {
            return ephemeris->GetMoonPositionGeographic();
        }
    } else {
        return Vector3(0,1,0);
    }
}

Vector3 Sky::GetSunOrMoonPositionEquatorial() const
{
    if (ephemeris) {
        if (sunTransmittedLuminance.SquaredLength() > moonTransmittedLuminance.SquaredLength()) {
            return ephemeris->GetSunPositionEquatorial();
        } else {
            return ephemeris->GetMoonPositionEquatorial();
        }
    } else {
        return Vector3(0,1,0);
    }
}

Vector3 Sky::GetMoonPosition() const
{
    if (ephemeris) {
        return ephemeris->GetMoonPositionHorizon();
    } else {
        return Vector3(0,1,0);
    }
}

Vector3 Sky::GetSunPosition() const
{
    if (ephemeris) {
        return ephemeris->GetSunPositionHorizon();
    } else {
        return Vector3(0,1,0);
    }
}

Vector3 Sky::GetSunPositionGeographic() const
{
    if (ephemeris) {
        return ephemeris->GetSunPositionGeographic();
    } else {
        return Vector3(1,0,0);
    }
}

Vector3 Sky::GetMoonPositionGeographic() const
{
    if (ephemeris) {
        return ephemeris->GetMoonPositionGeographic();
    } else {
        return Vector3(1,0,0);
    }
}

Vector3 Sky::GetSunPositionEquatorial() const
{
    if (ephemeris) {
        return ephemeris->GetSunPositionEquatorial();
    } else {
        return Vector3(1,0,0);
    }
}

Vector3 Sky::GetMoonPositionEquatorial() const
{
    if (ephemeris) {
        return ephemeris->GetMoonPositionEquatorial();
    } else {
        return Vector3(1,0,0);
    }
}

Matrix3 Sky::GetHorizonToEquatorialMatrix() const
{
    if (ephemeris) {
        return ephemeris->GetHorizonToEquatorialMatrix();
    } else {
        return Matrix3();
    }
}

Matrix3 Sky::GetHorizonToGeographicMatrix() const
{
    if (ephemeris) {
        return ephemeris->GetHorizonToGeographicMatrix();
    } else {
        return Matrix3();
    }
}

Color Sky::GetSunOrMoonColor() const
{
    double sunScale = 0.5;
    double moonScale = 0.5;

    Configuration::GetDoubleValue("sun-luminance-scale", sunScale);
    Configuration::GetDoubleValue("moon-luminance-scale", moonScale);

    Vector3 sunXYZ = sunTransmittedLuminance * sunScale;
    Vector3 moonXYZ = moonTransmittedLuminance * moonScale;

    LuminanceMapper::DurandMapperXYZ(&sunXYZ);
    LuminanceMapper::DurandMapperXYZ(&moonXYZ);

    if (Atmosphere::GetHDREnabled()) {
        //Map to kCd for consistency with sky
        sunXYZ = sunXYZ * 0.001;
        moonXYZ = moonXYZ * 0.001;
    }

    Vector3 XYZ = (sunXYZ + moonXYZ);

    Vector3 rgb = XYZ * XYZ2RGB_CIE;

    ApplyGamma(rgb);
    //scaleDownToOne(rgb);

    return Color(rgb.x, rgb.y, rgb.z);
}

Color Sky::GetSunOrMoonColorSeaLevel() const
{
    double sunScale = 0.5;
    double moonScale = 0.5;

    Configuration::GetDoubleValue("sun-luminance-scale", sunScale);
    Configuration::GetDoubleValue("moon-luminance-scale", moonScale);

    Vector3 sunXYZ = sunTransmittedLuminanceSeaLevel * sunScale;
    Vector3 moonXYZ = moonTransmittedLuminanceSeaLevel * moonScale;

    LuminanceMapper::DurandMapperXYZ(&sunXYZ);
    LuminanceMapper::DurandMapperXYZ(&moonXYZ);

    if (Atmosphere::GetHDREnabled()) {
        //Map to kCd for consistency with sky
        sunXYZ = sunXYZ * 0.001;
        moonXYZ = moonXYZ * 0.001;
    }

    Vector3 XYZ = (sunXYZ + moonXYZ);

    Vector3 rgb = XYZ * XYZ2RGB_CIE;

    ApplyGamma(rgb);
    //scaleDownToOne(rgb);

    return Color(rgb.x, rgb.y, rgb.z);
}

Color Sky::GetSunColor() const
{
    double sunScale = 0.5;
    Configuration::GetDoubleValue("sun-luminance-scale", sunScale);

    Vector3 sunXYZ = sunTransmittedLuminance * sunScale;

    LuminanceMapper::DurandMapperXYZ(&sunXYZ);

    if (Atmosphere::GetHDREnabled()) {
        // Convert to kCd
        sunXYZ = sunXYZ * 0.001;
    }

    Vector3 rgb = sunXYZ * XYZ2RGB_CIE;

    ApplyGamma(rgb);
    //scaleDownToOne(rgb);

    return Color(rgb.x, rgb.y, rgb.z);
}

Color Sky::GetMoonColor() const
{
    double moonScale = 0.5;
    Configuration::GetDoubleValue("moon-luminance-scale", moonScale);

    Vector3 moonXYZ = moonTransmittedLuminance * moonScale;

    LuminanceMapper::DurandMapperXYZ(&moonXYZ);

    if (Atmosphere::GetHDREnabled()) {
        moonXYZ = moonXYZ * 0.001;
    }

    Vector3 rgb = moonXYZ * XYZ2RGB_CIE;

    ApplyGamma(rgb);
    //scaleDownToOne(rgb);

    return Color(rgb.x, rgb.y, rgb.z);
}

void Sky::GetZenithLuminance(double *x, double *y, double *Y)
{
    *x = xZenith;
    *y = yZenith;
    *Y = YZenith;
}

Color Sky::GetToneMappedSkyLight()
{
    return skyLight;
}

Color Sky::GetToneMappedSkyLightSeaLevel()
{
    return skyLightSeaLevel;
}

void Sky::ComputeToneMappedSkyLight()
{
    double sunScale = 0.5;
    double moonScale = 0.5;
    Configuration::GetDoubleValue("sun-luminance-scale", sunScale);
    Configuration::GetDoubleValue("moon-luminance-scale", moonScale);

    Vector3 sunXYZ = sunScatteredLuminance * sunScale;
    sunXYZ.y += NightSkyLuminance() * 1000.0;

    Vector3 moonXYZ = moonScatteredLuminance * moonScale;

    Vector3 sunXYZSeaLevel = sunScatteredLuminanceSeaLevel * sunScale;
    sunXYZSeaLevel.y += NightSkyLuminance() * 1000.0;

    Vector3 moonXYZSeaLevel = moonScatteredLuminanceSeaLevel * moonScale;

    if (Atmosphere::GetHDREnabled()) {
        // convert to kCd
        sunXYZ = sunXYZ * 0.001;
        moonXYZ = moonXYZ * 0.001;
        sunXYZSeaLevel = sunXYZSeaLevel * 0.001;
        moonXYZSeaLevel = moonXYZSeaLevel * 0.001;
    } else {
        LuminanceMapper::DurandMapperXYZ(&sunXYZ);
        LuminanceMapper::DurandMapperXYZ(&moonXYZ);
        LuminanceMapper::DurandMapperXYZ(&sunXYZSeaLevel);
        LuminanceMapper::DurandMapperXYZ(&moonXYZSeaLevel);
    }

    Vector3 XYZ = (sunXYZ + moonXYZ);

    if (!Atmosphere::GetHDREnabled()) {
        if (XYZ.x > maxSkylightLuminance) XYZ.x = maxSkylightLuminance;
        if (XYZ.y > maxSkylightLuminance) XYZ.y = maxSkylightLuminance;
        if (XYZ.z > maxSkylightLuminance) XYZ.z = maxSkylightLuminance;
    }

    Vector3 rgb = XYZ * XYZ2RGB_CIE;

    //scaleDownToOne(rgb);
    if (monochromeAmbient) {
        rgb.x = rgb.z = rgb.y;
    }

    ApplyGamma(rgb);

    skyLight = Color(rgb.x, rgb.y, rgb.z);

    XYZ = (sunXYZSeaLevel + moonXYZSeaLevel);

    if (!Atmosphere::GetHDREnabled()) {
        if (XYZ.x > maxSkylightLuminance) XYZ.x = maxSkylightLuminance;
        if (XYZ.y > maxSkylightLuminance) XYZ.y = maxSkylightLuminance;
        if (XYZ.z > maxSkylightLuminance) XYZ.z = maxSkylightLuminance;
    }

    rgb = XYZ * XYZ2RGB_CIE;

    //scaleDownToOne(rgb);
    if (monochromeAmbient) {
        rgb.x = rgb.z = rgb.y;
    }

    ApplyGamma(rgb);

    skyLightSeaLevel = Color(rgb.x, rgb.y, rgb.z);
}
