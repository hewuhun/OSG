// Copyright (c) 2011 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/TritonCommon.h>
#include <FeTriton/WindFetch.h>
#include <math.h>

using namespace Triton;

WindFetch::WindFetch() : isGlobal(true), speed(0), direction(0), fetchLength(0)
{
}

void WindFetch::SetWind(double speed, double direction)
{
    if (fabs(direction) > TRITON_TWOPI) {
        Utils::DebugMsg("You passed in a direction greater than += 2*PI. While this is "
                        "technically OK, are you sure you're passing in radians and not degrees?");
    }
    WindFetch::speed = speed;
    WindFetch::direction = direction;
}


void TRITONAPI Triton::WindFetch::SetWindSpeed( double speed )
{
	WindFetch::speed = speed;
}

void TRITONAPI Triton::WindFetch::SetWindDirection( double direction )
{
	WindFetch::direction =direction;
}

void TRITONAPI Triton::WindFetch::SetWindLength( double fetchLength )
{
	WindFetch::fetchLength = fetchLength;
}

void TRITONAPI Triton::WindFetch::SetWave( double speed, double direction, double fetchLength )
{
	WindFetch::speed = speed;
	WindFetch::direction = direction;
	WindFetch::fetchLength = fetchLength;
}

void WindFetch::AdjustForLeftHanded()
{
    double theta = (0.5 * TRITON_PI) - direction;
    double x = cos(theta);
    double y = -sin(theta);
    direction = (0.5 * TRITON_PI) - atan2(y, x);
}

void WindFetch::SetLocalization(const Vector3& center, const Vector3& radii)
{
    isGlobal = false;
    WindFetch::center = center;
    WindFetch::radii = radii;
}

void WindFetch::ClearLocalization()
{
    isGlobal = true;
}

void WindFetch::SetFetchLength(double fetchLength)
{
    WindFetch::fetchLength = fetchLength;
}

void WindFetch::ClearFetchLength()
{
    fetchLength = 0;
}

void WindFetch::GetWindAtLocation(const Vector3& position, double& windSpeed, double& windDirection, double& fetchLength) const
{
    bool inside = true;

    double computedFetchLength = 0;

    if (!isGlobal) {
        Vector3 relPosition = position - center;
        Vector3 posSq = relPosition * relPosition;
        Vector3 rSq = radii * radii;
        inside = ( posSq.x / rSq.x + posSq.y / rSq.y + posSq.z / rSq.z < 1.0 );

        if (inside) {
            computedFetchLength = relPosition.Length();
        }
    }

    if (inside) {
        windSpeed = speed;
        windDirection = direction;
        fetchLength = WindFetch::fetchLength > 0 ? WindFetch::fetchLength : computedFetchLength;
    } else {
        windSpeed = 0;
        windDirection = 0;
        fetchLength = 0;
    }
}

