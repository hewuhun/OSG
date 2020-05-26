// Copyright (c) 2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/GodRays.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>
#include <FeTriton/Ocean.h>
#include <FeTriton/ProjectedGrid.h>
#include <math.h>

using namespace Triton;

GodRays::GodRays(const Environment *env) : slopeFoamTex(0), displacementTex(0), environment(env), fade(1.0f)
	,mGodRaysExposure(Vector3(0.76,2.46,2.29))
	,mGodRaysIntensity(0.045)
{
    N = 50;
    Configuration::GetIntValue("god-rays-mesh-dimension", N);

    effectDepth = 30.0;
    Configuration::GetFloatValue("god-rays-effect-depth", effectDepth);
    effectDepth /= (float)env->GetWorldUnits();

    // source: http://web.pdx.edu/~sytsmam/limno/Limno09.7.Light.pdf
    float er = 0.4f, eg = 0.008f, eb = 0.005f;
    Configuration::GetFloatValue("god-rays-extinction-coefficient-r", er);
    Configuration::GetFloatValue("god-rays-extinction-coefficient-g", eg);
    Configuration::GetFloatValue("god-rays-extinction-coefficient-b", eb);
    extinction = Vector3(er, eg, eb);

    K = 0.8;
    Configuration::GetDoubleValue("god-rays-adjustment-factor", K);

    Lx = Ly = 1024.0;
    Configuration::GetFloatValue("fft-grid-size-x", Lx);
    Configuration::GetFloatValue("fft-grid-size-y", Ly);
}

GodRays::~GodRays()
{
}

bool GodRays::Initialize()
{
    return SetupMesh();
}

bool GodRays::Draw(const ProjectedGrid *grid)
{
    basis = grid->GetBasis();
    invBasis = grid->GetInverseBasis();

    fade = grid->GetGodRayFade();

    if (ComputeConstants(grid)) 
	{
        return DrawMesh();
    }

    return false;
}

void GodRays::SetTextures(TextureHandle slopeFoam, TextureHandle displacement)
{
    slopeFoamTex = slopeFoam;
    displacementTex = displacement;
}

static Vector3 refract(const Vector3& N, const Vector3& I, double eta)
{
    Vector3 R;
    double k = 1.0 - eta * eta * (1.0 - N.Dot(I) * N.Dot(I));
    if (k < 0.0)
        R = Vector3(0, 0, 0);
    else
        R = I * eta - N * (eta * N.Dot(I) + sqrt(k));

    return R;
}

bool GodRays::ComputeConstants(const ProjectedGrid *grid)
{
    viewer = environment->GetCameraPosition();
    sunDir = environment->GetLightDirection();
    sunColor = environment->GetDirectionalLightColor();

    Vector3 I = sunDir * -1.0;
    Vector3 Nm = environment->GetUpVector();
    Vector3 T = refract(Nm, I, 1.0 / 1.333);

    Vector3 E = viewer;

    Vector3 surfaceAbove;
    if (grid->GetIntersection(viewer, Nm, surfaceAbove)) {
        double depth = (surfaceAbove - viewer).Length();

        // Transform so z is up
        E = E * basis;
        I = I * basis;
        Nm = Nm * basis;
        T = T * basis;

        double h0 = environment->GetSeaLevel();

        Vector3 P = E + T * depth * (1.0/T.z);

        origin = P * invBasis;

        double e = environment->GetProjectionMatrix()[0];

        double S = (2.0 * depth * (1.0 / e)) / (double)N;
        S *= K;

        spacing = (float)S;

        Matrix4 view(environment->GetCameraMatrix());
        viewDir = Vector3(view.elem[0][2], view.elem[1][2], view.elem[2][2]) * -1.0;

        return true;
    }

    return false;
}

