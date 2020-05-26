// Copyright (c) 2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CloudGeneratorRandom.h>
#include <FeSilverliningLib/Utils.h>
#if defined(__INTEL_COMPILER)
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

using namespace SilverLining;

CloudGeneratorRandom::CloudGeneratorRandom() : beta(0), nu(1)
{
}

CloudGeneratorRandom::~CloudGeneratorRandom()
{
}

void CloudGeneratorRandom::StartGeneration()
{
    areaCovered = 0;
}

bool CloudGeneratorRandom::GetNextCloud(double& width, double& depth, double& height)
{
    assert(GetMaximumSize() > 0);

    double range = GetMaximumSize() - GetMinimumSize();
    width = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble() * range + GetMinimumSize();
    depth = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble() * range + GetMinimumSize();

    double D = (width + depth) * 0.5;

    double hOverD = nu * pow(D / GetMaximumSize(), beta);
    height = D * hOverD;

    areaCovered += (width * 0.5) * (depth * 0.5) * PI;

    double currentDensity = areaCovered / GetDesiredArea();
    if (currentDensity >= GetDesiredCoverage()) {
        return false;
    } else {
        return true;
    }
}
