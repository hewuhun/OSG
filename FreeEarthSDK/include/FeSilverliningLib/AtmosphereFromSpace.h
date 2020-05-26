// Copyright (c) 2007-2009 Sundog Software, LLC. All rights reserved worldwide.

/** \file AtmosphereFromSpace.h
   \brief Creates and maintains a blended ring surrounding the Earth that simulates
     the atmosphere viewed from space.
 */

#ifndef ATMOSPHERE_FROM_SPACE_H
#define ATMOSPHERE_FROM_SPACE_H

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Frustum.h>
#include <FeSilverliningLib/Vector3.h>
#include <FeSilverliningLib/AtmosphereSegment.h>
#include <vector>

namespace SilverLining
{
class AtmosphereFromSpace : public MemObject
{
public:
    AtmosphereFromSpace();
    virtual ~AtmosphereFromSpace();

    void BuildGeometry();

    bool Draw(const Frustum& f, const Vector3& sunPositionGeocentric, const Vector3& earthCenter,
              double altitude);

    float GetAtmosphereThickness() const {
        return atmosphereThickness;
    }

private:
    float earthRadius, atmosphereThickness, atmosphereHeight, atmosphereBrightness;
    float atmosphereFadeDistance;
    int lateralSegments, verticalSegments, numSegments;
    SL_VECTOR(AtmosphereSegment *) segments;
    ShaderHandle atmoShader;
    bool enableShader, doShadow;
};
}

#endif
