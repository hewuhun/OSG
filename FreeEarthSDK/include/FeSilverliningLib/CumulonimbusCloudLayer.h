// Copyright (c) 2006-2009 Sundog Software, LLC. All rights reserved worldwide.

/**
    \file CumulonimbusCloudLayer.h
    \brief A cloud layer that contains a single thunderhead.
 */

#ifndef CUMULONIMBUS_CLOUD_LAYER_H
#define CUMULONIMBUS_CLOUD_LAYER_H

#include <FeSilverliningLib/CumulusCloudLayer.h>

namespace SilverLining
{
/** A CloudLayer object that contains a single, large cumulonimbus cloud. */
class CumulonimbusCloudLayer : public CumulusCloudLayer
{
public:
/** Default constructor. */
    CumulonimbusCloudLayer();

/** Instantiates a single cumulonimbus cloud within the volume defined by
   this cloud layer. */
    bool SILVERLINING_API SeedClouds(const Atmosphere& atm);

/** Forces a lightning strike starting with this frame on cloud layers that support lightning. */
    virtual bool SILVERLINING_API ForceLightning(bool value);
    virtual void SILVERLINING_API SetLightningDischargeMode(LightningDischargeMode mode);
    virtual LightningDischargeMode SILVERLINING_API GetLightningDischargeMode();

    // Don't allow infinite cumulonimbus clouds
    virtual void SILVERLINING_API SetIsInfinite(bool inf) {
        isInfinite = false;
    }

    virtual double ComputeSkyCoverage() const {return 0;}

protected:
    CumulusCloud *CreateCloud(CloudLayer *layer);
};
}

#endif
