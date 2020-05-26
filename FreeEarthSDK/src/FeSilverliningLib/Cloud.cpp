// Copyright (c) 2004-2013  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/Cloud.h>
#include <FeSilverliningLib/CloudImposter.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Utils.h>

using namespace SilverLining;

Vector3 Cloud::sortFromPosition;
SortOrder Cloud::sortOrder = ASCENDING;
Color Cloud::ambientColor;
double Cloud::cosLightChangeAngle = 0.999847;

Cloud::Cloud(CloudLayer *parentLayer) : fade(1.0), alpha(1.0), imposter(0), parentCloudLayer(parentLayer),
    invalid(false), needsRelight(false), culled(false), fadeState(FADED_IN), needsPlacement(true)
{
    double lightChangeAngle = 1.0;
    Configuration::GetDoubleValue("cloud-light-change-angle", lightChangeAngle);

    cosLightChangeAngle = cos(RADIANS(lightChangeAngle));

    coverageThreshold = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
}

Cloud::~Cloud()
{
    if (imposter) SL_DELETE imposter;
}

void Cloud::UpdateLightPos(const Vector3& lightPos)
{
    lastLightPos = lightPos;

    lastCloudPos = centerPos;
}

bool Cloud::LightingChanged(const Vector3& lightPos)
{
    bool changed = lightPos.Dot(lastLightPos) < cosLightChangeAngle;
    return changed;
}

void Cloud::CreateImposter()
{
    imposter = SL_NEW CloudImposter(this);
}
