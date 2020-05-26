// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/TGALoader.h>
#include <string>
#include <assert.h>
#include <FeSilverliningLib/Atmosphere.h>

using namespace SilverLining;
using namespace std;

SL_MAP(Atmosphere *, TextureHandle) Metaball::metaballTextureMap;
SL_MAP(Atmosphere *, TextureHandle) Metaball::wispTextureMap;
SL_MAP(Atmosphere *, TextureHandle) Metaball::atlasTextureMap;

int Metaball::textureDimension = 0;
Color Metaball::ambient(0, 0, 0);

Vector3 Metaball::lightPosition(0, 0, 0);
Vector3 Metaball::cameraPosition(0, 0, 0);
Vector3 Metaball::offsetPosition(0, 0, 0);

int Metaball::phaseFunction = LAMBERTIAN;
double Metaball::anisotropicX = 0.2;
double Metaball::henyeyGreensteinG1 = 0.8;
double Metaball::henyeyGreensteinG2 = -0.1;
double Metaball::henyeyGreensteinF = 0.85;
bool Metaball::inited = false;
float Metaball::maxMetaballColor = 0.9f;

void Metaball::ReadConfig()
{
    if (!inited) {
        inited = true;

        const char * phaseFunctionString;
        Configuration::GetStringValue("phase-function", phaseFunctionString);
        if (strcmp(phaseFunctionString, "isotropic") == 0) {
            phaseFunction = ISOTROPIC;
        } else if (strcmp(phaseFunctionString, "anisotropic") == 0) {
            phaseFunction = ANISOTROPIC;
        } else if (strcmp(phaseFunctionString, "lambertian") == 0) {
            phaseFunction = LAMBERTIAN;
        } else if (strcmp(phaseFunctionString, "rayleigh") == 0) {
            phaseFunction = RAYLEIGH;
        } else if (strcmp(phaseFunctionString, "henyey-greenstein") == 0) {
            phaseFunction = HENYEY_GREENSTEIN;
            Configuration::GetDoubleValue("henyey-greenstein-g1", henyeyGreensteinG1);
            Configuration::GetDoubleValue("henyey-greenstein-g2", henyeyGreensteinG2);
            Configuration::GetDoubleValue("henyey-greenstein-f", henyeyGreensteinF);
        }

        Configuration::GetFloatValue("max-metaball-color", maxMetaballColor);
    }
}

void Metaball::ReleaseTexture(Atmosphere *atm)
{
    Renderer *ren = Renderer::GetInstance();
    if (wispTextureMap[atm]) {
        ren->ReleaseTexture(wispTextureMap[atm]);
    }

    if (atlasTextureMap[atm]) {
        ren->ReleaseTexture(atlasTextureMap[atm]);
    }

    if (metaballTextureMap[atm]) {
        ren->ReleaseTexture(metaballTextureMap[atm]);
    }

    SL_MAP(Atmosphere *, TextureHandle) ::iterator it;
    it = wispTextureMap.find(atm);
    wispTextureMap.erase(it);
    it = atlasTextureMap.find(atm);
    atlasTextureMap.erase(it);
    it = metaballTextureMap.find(atm);
    metaballTextureMap.erase(it);
}

bool Metaball::CreateTexture(Atmosphere *atm)
{
    typedef unsigned char texel_type;

    textureDimension = 64;
    texel_type *tex;

    bool ok;

    TGALoader tga;

    if (tga.Load("wispy-metaball.tga")) {
        textureDimension = tga.GetWidth();
        tex = SL_NEW texel_type[textureDimension * textureDimension * 2];

        unsigned char *src = tga.GetPixels();
        unsigned char *dst = tex;

        for (int i = 0; i < textureDimension * textureDimension; i++) {
            unsigned char p = *src;
            *dst++ = p;
            *dst++ = p;
            src += 4;
        }

        TextureHandle wispTexture;
        ok = Renderer::GetInstance()->LoadTexture(tex, textureDimension, textureDimension,
                &wispTexture, false, false);

        wispTextureMap[atm] = wispTexture;

        SL_DELETE[] tex;
    }

    const char *atlasName;
    if (!Configuration::GetStringValue("cloud-atlas-texture-name", atlasName)) {
        atlasName = "CloudAtlas.tga";
    }
    if (tga.Load(atlasName)) {
        textureDimension = tga.GetWidth();
        tex = SL_NEW texel_type[textureDimension * textureDimension * 2];

        unsigned char *src = tga.GetPixels();
        unsigned char *dst = tex;

        for (int i = 0; i < textureDimension * textureDimension; i++) {
            unsigned char p = *src;
            *dst++ = p;
            *dst++ = p;
            src += 4;
        }

        TextureHandle atlasTexture;
        ok = Renderer::GetInstance()->LoadTexture(tex, textureDimension, textureDimension,
                &atlasTexture, false, false);

        atlasTextureMap[atm] = atlasTexture;

        SL_DELETE[] tex;
    }

    Configuration::GetIntValue("metaball-texture-dimension", textureDimension);

    tex = SL_NEW texel_type[textureDimension * textureDimension * 2];

    double midpoint = (double)textureDimension * 0.5;
    texel_type *tp = tex;

    for (int row = 0; row < textureDimension; row++) {
        double dy = midpoint - (double)row;
        for (int col = 0; col < textureDimension; col++) {
            double dx = midpoint - (double)col;
            double dist = sqrt(dx*dx + dy*dy) / midpoint;

            if (dist <= 1.0) {
                double a2 = dist * dist;
                double a4 = a2 * a2;
                double a6 = a4 * a2;

                double texel = ((-4.0/9.0) * a6 + (17.0/9.0) * a4 - (22.0/9.0) * a2 + 1.0) * 255.0;
                texel_type tt = (texel_type)texel;

                *tp++ = tt;
                *tp++ = tt;
            } else {
                *tp++ = 0;
                *tp++ = 0;
            }
        }
    }

    TextureHandle metaballTexture;
    ok = Renderer::GetInstance()->LoadTexture(tex, textureDimension, textureDimension,
            &metaballTexture, false, false);
    metaballTextureMap[atm] = metaballTexture;

    SL_DELETE[] tex;

    return ok;
}

void Metaball::InitializeMetaball(const float _radius, const float _spinRate, const int atlasIndex, const bool rotate)
{
    ReadConfig();
    SetAtlasIndex(atlasIndex);
    Initialize(rotate, _radius*2.0f, _spinRate);
    SetMetaballColor(Color(1,1,1,1));

    radius = (float)_radius;
    spinRate = (float)_spinRate;
}

Metaball::~Metaball()
{
}

double Metaball::PhaseFunction(double cosa)
{
    switch (phaseFunction) {

    case ISOTROPIC:
        return 1;

    case ANISOTROPIC:
        return (1 + anisotropicX * cosa);

    case LAMBERTIAN: {
        double a = acos(cosa);
        return (8.0 / 3.0) * PI * (sin(a) + (PI - a) * cosa);
    }

    case RAYLEIGH:
        return 0.75 * (1 + cosa * cosa);

    case HENYEY_GREENSTEIN: {
        double g1sq = henyeyGreensteinG1 * henyeyGreensteinG1;
        double g1 = (1 - g1sq) / pow((1 + g1sq - 2 * henyeyGreensteinG1 * cosa), 1.5);
        double g2sq = henyeyGreensteinG2 * henyeyGreensteinG2;
        double g2 = (1 - g2sq) / pow((1 + g2sq - 2 * henyeyGreensteinG2 * cosa), 1.5);
        return (g1 * henyeyGreensteinF) + (g2 * (1-henyeyGreensteinF));
    }

    default:
        return 1;
    }
}

void Metaball::ComputeBillboardColor(int pass, double colorRandomness, float ambientScattering)
{
    if (pass == 0) {
        float phase = (float)PhaseFunction(1.0);

        Color c(color.r * phase, color.g * phase,
                color.b * phase, color.a);

        c.ClampToUnitOrLess();

        SetColor(c);

    } else {
        // Todo: move this to a shader, as this gets cached and the camera moves.
        const Vector3f& b = GetWorldPosition();
        Vector3 w = (Vector3(b.x, b.y, b.z) + offsetPosition - cameraPosition);
        w.Normalize();
        double cosTheta = w.Dot(lightPosition);
        float phase = (float)PhaseFunction(cosTheta);

        double r = fabs(fmod(b.x + b.y + b.z, 100.0f)) * 0.01;
        r = (1.0 - colorRandomness) + (r * colorRandomness);

        // diffuse + ambient. Fog applied in billboard shader.
        Color c((color.r * phase + ambient.r * ambientScattering),
                (color.g * phase + ambient.g * ambientScattering),
                (color.b * phase + ambient.b * ambientScattering), color.a);

        c.r *= (float)r;
        c.g *= (float)r;
        c.b *= (float)r;
        c.a *= (float)r;

        c.ScaleToValueOrLess(maxMetaballColor);

        SetColor(c);

    }
}

void Metaball::SetMetaballColor(const Color& pColor)
{
    color.ClampToUnitOrLess();

    color.r = pColor.r;
    color.g = pColor.g;
    color.b = pColor.b;
}

void Metaball::SetAlpha(double alpha)
{
    color.a = alpha > 1.0 ? 1.0f : (float)alpha;
}


bool Metaball::Cull(const Frustum& f, const Vector3& offset)
{
    const Vector3f& wp = GetWorldPosition();
    Vector3 center = Vector3(wp.x, wp.y, wp.z) + offset;
    for (int i = 0; i < f.GetNumCullingPlanes(); i++) {
        const Plane& p = f.GetPlane(i);
        double origin = (p.GetNormal()).Dot(center) + p.GetDistance();

        if (origin < -radius) {
            return true;
        }
    }

    return false;
}
