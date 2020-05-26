// Copyright (c) 2004-2008  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/Glare.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/LuminanceMapper.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Configuration.h>

#include <assert.h>

using namespace SilverLining;

#define TEXSIZE 128

static double glareDegrees;

GlareManager::GlareManager() : mesopic(0), scotopic(0), photopic(0),
    photopicDisc(0), visionType(PHOTOPIC)
{
    Configuration::GetDoubleValue("max-glare-diameter", glareDegrees);

    bool sunGlare = false, moonGlare = false, starGlare = false;
    Configuration::GetBoolValue("disable-sun-glare", sunGlare);
    Configuration::GetBoolValue("disable-moon-glare", moonGlare);
    Configuration::GetBoolValue("disable-star-glare", starGlare);
    sunGlare = !sunGlare;
    moonGlare = !moonGlare;
    starGlare = !starGlare;

    texelSize = glareDegrees / (double)TEXSIZE;

    if (sunGlare || moonGlare || starGlare) {
        MakeTextures();

        fragmentShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/Glare.cg", FRAGMENT_PROGRAM);
    } else {
        fragmentShader = 0;
    }
}

GlareManager::~GlareManager()
{
    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        if (mesopic) {
            ren->ReleaseTexture(mesopic);
        }
        if (scotopic) {
            ren->ReleaseTexture(scotopic);
        }
        if (photopic) {
            ren->ReleaseTexture(photopic);
        }
        if (photopicDisc) {
            ren->ReleaseTexture(photopicDisc);
        }
    }
}

bool GlareManager::BuildFlareTexture(float *&outBuffer)
{
    bool ok = false;
    int i;
    unsigned char *tmp = SL_NEW unsigned char[TEXSIZE * TEXSIZE * 4 * sizeof(unsigned char)];

    Renderer *ren = Renderer::GetInstance();
    Vector3 origin((double)TEXSIZE * 0.5, (double)TEXSIZE * 0.5, 0);
    double radius = (double)TEXSIZE * 0.5; // * sqrt(2.0);

    int x, y, vpW, vpH;

    bool offscreen = false;
    RenderTargetHandle target = 0;
    Configuration::GetBoolValue("render-offscreen", offscreen);
    if (offscreen) {
        offscreen = (ren->InitRenderTarget(TEXSIZE, TEXSIZE, &target));
        if (offscreen) {
            ren->MakeRenderTargetCurrent(target);
        }
    }

    ren->PushTransforms();

    if (ren->GetViewport(x, y, vpW, vpH)) {
        ren->Set2DOrthoMatrix(vpW, vpH);

        ren->EnableDepthReads(false);
        ren->EnableDepthWrites(false);

        ren->ClearScreen(Color(0,0,0,1));

        int flareLines = 360;
        Configuration::GetIntValue("flare-lines", flareLines);
        double flareFalloff = 1.2;
        Configuration::GetDoubleValue("flare-falloff", flareFalloff);

        for (i = 0; i < flareLines; i++) {
            double dangle = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
            double angle = dangle * 2.0 * PI;
            Vector3 random(radius * sin(angle), radius * cos(angle), 0);

            double intensity = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();

            Color c(intensity, intensity, intensity);

            ren->DrawAALine(c, 1.0, origin, origin + random);
        }

        ren->EnableDepthReads(true);
        ren->EnableDepthWrites(true);

        ren->GetPixels(0, 0, TEXSIZE, TEXSIZE, tmp, true);

        if (offscreen) {
            ren->RestoreRenderTarget(target);
            ren->ReleaseRenderTarget(target);
        }

        outBuffer = SL_NEW float[TEXSIZE * TEXSIZE * sizeof(float)];

        float *out = outBuffer;
        float totalIntensity = 0;

        int stride = TEXSIZE * 4;
        float cx = (float)floor(TEXSIZE * 0.5);
        float cy = (float)floor(TEXSIZE * 0.5);
        float unit = (float)(TEXSIZE) * 0.5;

        for (int row = 0; row < TEXSIZE; row++) {
            float ny = (float)(row - cy) / unit;
            for (int col = 0; col < TEXSIZE; col++) {
                float nx = (float)(col - cx) / unit;
                float dist = sqrtf(ny*ny + nx*nx);
                if (dist > 1.0) dist = 1.0;
                float intensity = (float)tmp[row*stride + col*4] / 255.0f;
                intensity *= ::pow(dist, (float)flareFalloff);
                totalIntensity += intensity;
                *out++ = intensity;
            }
        }

        // Make avg 1.0

        if (totalIntensity > 0) {
            float scale = (float)(TEXSIZE * TEXSIZE) / totalIntensity;
            for (i = 0; i < TEXSIZE * TEXSIZE; i++) {
                outBuffer[i] *= scale;
            }
        }

        ok = true;
    }

    ren->PopTransforms();

    SL_DELETE[] tmp;

    return ok;
}

bool GlareManager::MakeTextures()
{
    int i;

    Renderer *renderer = Renderer::GetInstance();

    if (!renderer->HasFloatTextures()) {
        return false;
    }

    float *g, *g1;
    if (BuildFlareTexture(g)) {
        if (BuildFlareTexture(g1)) {

            double *f0 = SL_NEW double[TEXSIZE * TEXSIZE];
            double *f1 = SL_NEW double[TEXSIZE * TEXSIZE];
            double *f2 = SL_NEW double[TEXSIZE * TEXSIZE];
            double **f3 = SL_NEW double*[3]; // rgba
            for (i = 0; i < 3; i++) {
                f3[i] = SL_NEW double[TEXSIZE * TEXSIZE];
            }

            double midpoint = (double)TEXSIZE * 0.5;
            // make sure the midpoint falls on a pixel.
            midpoint = floor(midpoint);

            double scale = 1.0;
            Configuration::GetDoubleValue("glare-brightness", scale);

            for (int r = 0; r < TEXSIZE; r++) {
                for (int c = 0; c < TEXSIZE; c++) {
                    int idx = r * TEXSIZE + c;

                    double du = (double)r - midpoint;
                    double dv = (double)c - midpoint;
                    double theta = texelSize * sqrt(du * du + dv * dv);

                    double f0exp = theta / 0.02;
                    f0exp *= f0exp;
                    f0[idx] = 2.61E6 * exp(-f0exp) * scale;

                    double f1den = theta + 0.02;
                    double f2den = f1den * f1den;
                    f1den = f2den * f1den;
                    f1[idx] = (20.91 / f1den) * scale;

                    f2[idx] = (72.37 / f2den) * scale;

                    double wavelengths[3] = {650, 530, 460};
                    for (int i = 0; i < 3; i++) {
                        double f3exp = theta - 3.0 * (wavelengths[i] / 568.0);
                        f3exp *= f3exp;
                        f3[i][idx] = (436.9 * (568.0 / wavelengths[i]) * exp(-f3exp)) * scale;
                    }
                }
            }

            float *mesopicTex = SL_NEW float[TEXSIZE * TEXSIZE * 3];
            float *scotopicTex = SL_NEW float[TEXSIZE * TEXSIZE * 3];
            float *photopicTex = SL_NEW float[TEXSIZE * TEXSIZE * 3];
            float *photopicDiscTex = SL_NEW float[TEXSIZE * TEXSIZE * 3];

            float *ms = mesopicTex;
            float *sc = scotopicTex;
            float *ph = photopicTex;

            bool photopicFlare = true, halos = true;
            Configuration::GetBoolValue("photopic-disc-flare-lines", photopicFlare);
            Configuration::GetBoolValue("lenticular-halos", halos);

            for (int p = 0; p < TEXSIZE * TEXSIZE; p++) {
                const double f0Sco = 0.282;
                const double f1Sco = 0.478;
                const double f2Sco = 0.207;
                const double f3Sco = 0.033;

                const double f0Mes = 0.368;
                const double f1Mes = 0.478;
                const double f2Mes = 0.138;
                const double f3Mes = 0.016;

                if (halos) {
                    *sc++ = (float)(f0Sco * f0[p] + g1[p] * (f1Sco * f1[p] + f2Sco * f2[p]) + f3Sco * g[p] * f3[0][p]);
                    *sc++ = (float)(f0Sco * f0[p] + g1[p] * (f1Sco * f1[p] + f2Sco * f2[p]) + f3Sco * g[p] * f3[1][p]);
                    *sc++ = (float)(f0Sco * f0[p] + g1[p] * (f1Sco * f1[p] + f2Sco * f2[p]) + f3Sco * g[p] * f3[2][p]);

                    *ms++ = (float)(f0Mes * f0[p] + g1[p] * (f1Mes * f1[p] + f2Mes * f2[p]) + f3Mes * g[p] * f3[0][p]);
                    *ms++ = (float)(f0Mes * f0[p] + g1[p] * (f1Mes * f1[p] + f2Mes * f2[p]) + f3Mes * g[p] * f3[1][p]);
                    *ms++ = (float)(f0Mes * f0[p] + g1[p] * (f1Mes * f1[p] + f2Mes * f2[p]) + f3Mes * g[p] * f3[2][p]);
                } else {
                    *sc++ = (float)(f0Sco * f0[p] + g1[p] * (f1Sco * f1[p] + f2Sco * f2[p]));
                    *sc++ = (float)(f0Sco * f0[p] + g1[p] * (f1Sco * f1[p] + f2Sco * f2[p]));
                    *sc++ = (float)(f0Sco * f0[p] + g1[p] * (f1Sco * f1[p] + f2Sco * f2[p]));

                    *ms++ = (float)(f0Mes * f0[p] + g1[p] * (f1Mes * f1[p] + f2Mes * f2[p]));
                    *ms++ = (float)(f0Mes * f0[p] + g1[p] * (f1Mes * f1[p] + f2Mes * f2[p]));
                    *ms++ = (float)(f0Mes * f0[p] + g1[p] * (f1Mes * f1[p] + f2Mes * f2[p]));
                }

                float i;
                if (photopicFlare) {
                    i = (float)(0.384 * f0[p] + g1[p] * (0.478 * f1[p] + 0.138 * f2[p]));
                } else {
                    i = (float)(0.384 * f0[p] + /*g1[p] * */ (0.478 * f1[p] + 0.138 * f2[p]));
                }

                *ph++ = i;
                *ph++ = i;
                *ph++ = i;
            }

            renderer->LoadFloatTextureRGB(mesopicTex, TEXSIZE, TEXSIZE, &mesopic);
            renderer->LoadFloatTextureRGB(scotopicTex, TEXSIZE, TEXSIZE, &scotopic);
            renderer->LoadFloatTextureRGB(photopicTex, TEXSIZE, TEXSIZE, &photopic);

            SL_DELETE[] f0;
            SL_DELETE[] f1;
            SL_DELETE[] f2;
            for (i = 0 ; i < 3; i++) {
                SL_DELETE[] f3[i];
            }
            SL_DELETE[] f3;
            SL_DELETE[] mesopicTex;
            SL_DELETE[] scotopicTex;
            SL_DELETE[] photopicTex;
            SL_DELETE[] photopicDiscTex;

            SL_DELETE[] g1;
            SL_DELETE[] g;
        }
    }

    return false;
}

const TextureHandle GlareManager::GetTexture(int type) const
{
    if (type == AUTOSELECT) type = visionType;

    switch (type) {
    case PHOTOPIC:
        return photopic;
        break;

    case MESOPIC:
        return mesopic;
        break;

    case SCOTOPIC:
        return scotopic;
        break;

    default:
        return 0;
        break;
    }
}

bool GlareManager::EnableShaders()
{
    double rodNits, coneNits;
    LuminanceMapper::GetSceneLogAvg(&rodNits, &coneNits);

    if (coneNits < 0.01) {
        visionType = SCOTOPIC;
    } else if (coneNits < 3.0) {
        visionType = MESOPIC;
    } else {
        visionType = PHOTOPIC;
    }

    if (fragmentShader) {
        if (Renderer::GetInstance()->BindShader(fragmentShader, FRAGMENT_PROGRAM)) {
            return true;
        }
    }

    return false;
}

double GlareManager::GetDiscArea() const
{
    double r = (0.5 * 0.5) / texelSize;
    double area = PI * r * r;

    return area;
}

bool GlareManager::DisableShaders()
{

    Renderer::GetInstance()->UnbindShader(FRAGMENT_PROGRAM);

    return true;
}

Glare::Glare(GlareManager *mgr) : Billboard(), intensity(0)
{
    glareMgr = mgr;
    Initialize(false);
}

Glare::~Glare()
{
}

void Glare::SetIntensity(double nits)
{
    // D65 white point
    double x = 0.312727, y = 0.329024;
    LuminanceMapper::DurandMapper(&x, &y, &nits);

    double maxNits = 1.5;
    Configuration::GetDoubleValue("glare-luminance-cap", maxNits);
    if (nits > maxNits) nits = maxNits;

    nits -= 1.0;

    if (nits < 0) nits = 0;

    SetAbsoluteIntensity(nits);

}

void Glare::SetAbsoluteIntensity(double nits)
{
    intensity = (float)nits;
}

bool Glare::Cull(const Frustum& f)
{
    const Vector3f& fCenter = GetWorldPosition();
    Vector3 center(fCenter.x, fCenter.y, fCenter.z);

    for (int i = 0; i < 6; i++) {
        const Plane& p = f.GetPlane(i);
        Vector3 N = p.GetNormal();
        double origin = N.Dot(center) + p.GetDistance();

        if (origin < 0) {
            return true;
        }
    }

    return false;
}

bool Glare::Draw(int pass)
{
    if (sharedVb && pass == 0 && intensity > 0) {
        TextureHandle tex = glareMgr->GetTexture(AUTOSELECT);

        if (tex) {
            Vector3 v;
            v.x = v.y = v.z = intensity;
            SetAngularSize(glareDegrees);
            Renderer::GetInstance()->SetConstantVector(glareMgr->GetFragmentProgram(), "sl_intensity", v);
            Renderer::GetInstance()->EnableBlending(SRCCOLOR, ONE);
            //Renderer::GetInstance()->DisableBlending();
            bool ok = Billboard::Draw(tex, 0, 1.0, false);
            return ok;
        }

        return true;
    }

    return false;
}
